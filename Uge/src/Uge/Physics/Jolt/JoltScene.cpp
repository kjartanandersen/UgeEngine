#include <ugpch.h>
#include "JoltScene.h"

#include "Uge/Physics/Jolt/JoltUtils.h"
#include "Uge/Physics/Jolt/JoltData.h"
#include "Uge/Physics/Jolt/JoltPhysicsDebugRenderer.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyFilter.h>
#include <Jolt/Physics/Body/BodyLock.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/NarrowPhaseQuery.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/TransformedShape.h>

#include <mutex>
#include <unordered_set>

namespace Uge
{

    /// Builds one collider's shape. Returns an empty ref on failure.
    static JPH::ShapeRefC BuildColliderShape(const ColliderDesc& collider, BodyType bodyType)
    {
        JPH::Ref<JPH::ShapeSettings> settings;

        if (const auto* box = std::get_if<BoxShapeDesc>(&collider.Shape))
        {
            auto* s = new JPH::BoxShapeSettings(ToJolt(box->HalfExtents));
            s->SetDensity(collider.Material.Density);
            settings = s;
        }
        else if (const auto* sphere = std::get_if<SphereShapeDesc>(&collider.Shape))
        {
            auto* s = new JPH::SphereShapeSettings(sphere->Radius);
            s->SetDensity(collider.Material.Density);
            settings = s;
        }
        else if (const auto* capsule = std::get_if<CapsuleShapeDesc>(&collider.Shape))
        {
            // Jolt takes (halfHeightOfCylinder, radius), the reverse of our order.
            auto* s = new JPH::CapsuleShapeSettings(capsule->HalfHeight, capsule->Radius);
            s->SetDensity(collider.Material.Density);
            settings = s;
        }
        else if (const auto* mesh = std::get_if<MeshShapeDesc>(&collider.Shape))
        {
            if (mesh->Vertices.empty() || mesh->Indices.size() < 3)
            {
                UG_CORE_WARN("Physics: mesh collider has no geometry; collider skipped.");
                return {};
            }

            // A triangle mesh makes the whole body static-only (MustBeStatic carries through
            // compound and decorated shapes) and has no volume to derive mass from. A moving
            // body falls back to a convex hull instead of losing the collider.
            bool convex = mesh->Convex;
            if (!convex && bodyType != BodyType::Static)
            {
                UG_CORE_WARN("Physics: a triangle-mesh collider can only back a static body; "
                    "falling back to a convex hull.");
                convex = true;
            }

            if (convex)
            {
                JPH::Array<JPH::Vec3> points;
                points.reserve(mesh->Vertices.size());
                for (const glm::vec3& v : mesh->Vertices)
                    points.push_back(ToJolt(v));

                auto* s = new JPH::ConvexHullShapeSettings(points);
                s->SetDensity(collider.Material.Density);
                JPH::ShapeSettings::ShapeResult hull = s->Create();
                if (hull.HasError())
                {
                    UG_CORE_WARN("Physics: convex hull failed ({0}); retrying with a looser tolerance.", hull.GetError());
                    s->mHullTolerance = 0.05f;
                    s->ClearCachedResult();
                }
                settings = s;
            }
            else
            {
                JPH::VertexList vertices;
                vertices.reserve(mesh->Vertices.size());
                for (const glm::vec3& v : mesh->Vertices)
                    vertices.push_back(JPH::Float3(v.x, v.y, v.z));

                JPH::IndexedTriangleList triangles;
                triangles.reserve(mesh->Indices.size() / 3);
                for (size_t i = 0; i + 2 < mesh->Indices.size(); i += 3)
                    triangles.push_back(JPH::IndexedTriangle(
                        mesh->Indices[i], mesh->Indices[i + 1], mesh->Indices[i + 2], 0));

                // A triangle soup has no volume, so there is no SetDensity. The constructor
                // drops duplicate and degenerate triangles.
                settings = new JPH::MeshShapeSettings(std::move(vertices), std::move(triangles));
            }
        }
        else
        {
            UG_CORE_ERROR("Physics: unhandled shape description; collider skipped.");
            return {};
        }

        JPH::ShapeSettings::ShapeResult result = settings->Create();
        if (result.HasError())
        {
            UG_CORE_ERROR("Physics: failed to create collider shape: {0}", result.GetError());
            return {};
        }

        return result.Get();
    }

    namespace
    {
        /// Skips sensors, so a trigger doesn't block a ray.
        class IgnoreSensorsBodyFilter : public JPH::BodyFilter
        {
        public:
            bool ShouldCollideLocked(const JPH::Body& body) const override
            {
                return !body.IsSensor();
            }
        };
    }

    /**
     * @brief Buffers Jolt's contact callbacks for the main thread.
     *
     * The callbacks run on Jolt's worker threads during Update, with every body locked, so
     * they only buffer. Nothing here may touch the registry, assets or Mono.
     */
    class JoltContactListener : public JPH::ContactListener
    {
    public:
        /**
         * @brief Creates the listener.
         * @param physicsSystem System asked whether two bodies were already touching.
         */
        explicit JoltContactListener(JPH::PhysicsSystem& physicsSystem)
            : m_physicsSystem(physicsSystem)
        {
        }

        /**
         * @brief Buffers a ContactType::Begin when two bodies first touch.
         * @param body1 First body.
         * @param body2 Second body.
         * @param manifold Unused.
         * @param settings Left unchanged.
         */
        void OnContactAdded(const JPH::Body& body1, const JPH::Body& body2,
            const JPH::ContactManifold& manifold,
            JPH::ContactSettings& settings) override
        {
            // Called per sub-shape pair. False means the bodies weren't touching before.
            if (m_physicsSystem.WereBodiesInContact(body1.GetID(), body2.GetID()))
            {
                return;

            }

            Push(ContactEvent{ FromJoltBodyID(body1.GetID()),
                               FromJoltBodyID(body2.GetID()),
                               ContactType::Begin,
                               body1.IsSensor() || body2.IsSensor() });
        }

        /**
         * @brief Buffers a ContactType::End when the last sub-shape contact ends.
         * @param pair Sub-shape pair that separated.
         */
        void OnContactRemoved(const JPH::SubShapeIDPair& pair) override
        {
            // True means other sub-shape contacts remain, so the bodies still touch.
            if (m_physicsSystem.WereBodiesInContact(pair.GetBody1ID(), pair.GetBody2ID()))
            {
                return;

            }

            const PhysicsBodyID a = FromJoltBodyID(pair.GetBody1ID());
            const PhysicsBodyID b = FromJoltBodyID(pair.GetBody2ID());

            // The bodies may already be destroyed, so the sensor flag comes from the set.
            Push(ContactEvent{ a, b, ContactType::End,
                               IsSensorBody(a) || IsSensorBody(b) });
        }

        /**
         * @brief Moves the buffered events into @p out.
         * @param out Receives the events; its contents are replaced.
         */
        void Drain(std::vector<ContactEvent>& out)
        {
            std::scoped_lock lock(m_mutex);
            out.swap(m_pending);
            m_pending.clear();
        }

        /** @brief Records a sensor body. Main thread, outside Step(). @param body Sensor body. */
        void MarkSensor(PhysicsBodyID body) { m_sensorBodies.insert(body.Value); }
        /** @brief Forgets a destroyed body. Main thread, outside Step(). @param body The body. */
        void ForgetBody(PhysicsBodyID body) { m_sensorBodies.erase(body.Value); }

    private:
        void Push(const ContactEvent& event)
        {
            std::scoped_lock lock(m_mutex);
            m_pending.push_back(event);
        }

        bool IsSensorBody(PhysicsBodyID body) const
        {
            return m_sensorBodies.count(body.Value) != 0;
        }

        JPH::PhysicsSystem& m_physicsSystem;

        std::mutex m_mutex;
        std::vector<ContactEvent> m_pending;

        // Written on the main thread outside Update, so the callbacks read it without a lock.
        std::unordered_set<uint32_t> m_sensorBodies;
    };


	JoltScene::JoltScene(const PhysicsSceneDesc& desc)
	{

		m_physicsSystem.Init(desc.MaxBodies, desc.NumBodyMutexes, desc.MaxBodyPairs, desc.MaxContactConstraints, 
			m_broadPhaseLayerInterface, m_objectVsBroadPhaseFilter, m_objectLayerPairFilter);

		SetGravity(desc.Gravity);

        m_contactListener = CreateScope<JoltContactListener>(m_physicsSystem);
        m_physicsSystem.SetContactListener(m_contactListener.get());

        m_maxBodies = desc.MaxBodies;
	}

    JoltScene::~JoltScene()
    {

        // The listener is destroyed before the system, so it is detached first.
        m_physicsSystem.SetContactListener(nullptr);

    }

    void JoltScene::Step(float fixedDeltaTime, int collisionSteps)
    {
        if (fixedDeltaTime <= 0.0f || collisionSteps < 1)
        {
            return;
        }

        JoltData& joltData = GetJoltData();

        const JPH::EPhysicsUpdateError error = 
            m_physicsSystem.Update( fixedDeltaTime, 
                                    collisionSteps,
                                    joltData.TempAllocator.get(), 
                                    joltData.JobThreadPool.get());

        if (error != JPH::EPhysicsUpdateError::None)
        {
            UG_CORE_WARN(   "Physics: Jolt update reported error flags 0x{0:x} — raise "
                            "MaxBodyPairs / MaxContactConstraints in PhysicsSceneDesc.",
                            (uint32_t)error);


        }

    }

	PhysicsBodyID JoltScene::CreateBody(const BodyDesc& desc)
	{
        if (desc.Colliders.empty())
        {
            UG_CORE_WARN("Physics: CreateBody called with no colliders; body not created.");
            return {};
        }

        // 1. Build each collider's shape, then combine them.

        JPH::ShapeRefC shape;

        if (desc.Colliders.size() == 1)
        {
            const ColliderDesc& collider = desc.Colliders[0];

            shape = BuildColliderShape(collider, desc.Type);
            if (shape == nullptr)
                return {};

            // A single collider's offset goes in a decorator; no compound is needed.
            if (collider.Offset != glm::vec3(0.0f))
            {
                JPH::Ref<JPH::ShapeSettings> offsetSettings =
                    new JPH::RotatedTranslatedShapeSettings(ToJolt(collider.Offset),
                        JPH::Quat::sIdentity(),
                        shape.GetPtr());

                JPH::ShapeSettings::ShapeResult result = offsetSettings->Create();
                if (result.HasError())
                {
                    UG_CORE_ERROR("Physics: failed to offset collider: {0}", result.GetError());
                    return {};
                }
                shape = result.Get();
            }
        }
        else
        {
            // AddShape takes each collider's offset directly.

            JPH::Ref<JPH::StaticCompoundShapeSettings> compound =
                new JPH::StaticCompoundShapeSettings();

            uint32_t added = 0;
            for (const ColliderDesc& collider : desc.Colliders)
            {
                JPH::ShapeRefC sub = BuildColliderShape(collider, desc.Type);
                if (sub == nullptr)
                    continue;

                compound->AddShape(ToJolt(collider.Offset), JPH::Quat::sIdentity(), sub.GetPtr());
                ++added;
            }

            if (added == 0)
            {
                UG_CORE_WARN("Physics: every collider failed to build; body not created.");
                return {};
            }

            JPH::ShapeSettings::ShapeResult result = compound->Create();
            if (result.HasError())
            {
                UG_CORE_ERROR("Physics: failed to build compound shape: {0}", result.GetError());
                return {};
            }
            shape = result.Get();
        }

        if (shape->MustBeStatic() && desc.Type != BodyType::Static)
        {
            UG_CORE_ERROR("Physics: body has a static-only shape but is not static; body not created.");
            return {};
        }

        // 2. Apply the entity's scale. Jolt bodies have no scale, so it is fixed at creation.
        if (desc.Scale != glm::vec3(1.0f))
        {

            JPH::Shape::ShapeResult result = shape->ScaleShape(ToJolt(desc.Scale));
            if (result.HasError())
            {
                UG_CORE_ERROR("Physics: failed to scale shape: {0}", result.GetError());
                return {};
            }
            shape = result.Get();
        }

        // 3. Describe the body.

        JPH::EMotionType motionType = JPH::EMotionType::Dynamic;
        switch (desc.Type)
        {
        case BodyType::Static:    motionType = JPH::EMotionType::Static;    break;
        case BodyType::Kinematic: motionType = JPH::EMotionType::Kinematic; break;
        case BodyType::Dynamic:   motionType = JPH::EMotionType::Dynamic;   break;
        }

        const JPH::ObjectLayer objectLayer = (JPH::ObjectLayer)desc.Layer;

        JPH::BodyCreationSettings settings(shape.GetPtr(),
            ToJolt(desc.Position),
            ToJolt(desc.Rotation),
            motionType,
            objectLayer);

        // Friction, restitution and the sensor flag are per body in Jolt: the first
        // collider's apply.
        settings.mFriction = desc.Colliders[0].Material.Friction;
        settings.mRestitution = desc.Colliders[0].Material.Restitution;
        settings.mIsSensor = desc.Colliders[0].IsTrigger;

        settings.mLinearDamping = desc.LinearDamping;
        settings.mAngularDamping = desc.AngularDamping;
        settings.mGravityFactor = desc.GravityFactor;
        settings.mUserData = desc.UserData;

        if (desc.FixedRotation && motionType != JPH::EMotionType::Static)
        {
            settings.mAllowedDOFs = JPH::EAllowedDOFs::TranslationX
                | JPH::EAllowedDOFs::TranslationY
                | JPH::EAllowedDOFs::TranslationZ;
        }

        // Mass 0 leaves Jolt to derive it from volume and density.
        if (desc.Mass > 0.0f && motionType == JPH::EMotionType::Dynamic)
        {
            settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
            settings.mMassPropertiesOverride.mMass = desc.Mass;
        }

        // 4. Create the body and add it.

        JPH::BodyInterface& bodyInterface = m_physicsSystem.GetBodyInterface();

        JPH::BodyID bodyID = bodyInterface.CreateAndAddBody(settings,
            motionType == JPH::EMotionType::Static ? JPH::EActivation::DontActivate : JPH::EActivation::Activate);

        if (bodyID.IsInvalid())
        {
            UG_CORE_ERROR("Physics: CreateAndAddBody failed — MaxBodies ({0}) may be exhausted.",
                m_maxBodies);
            return {};
        }

        const PhysicsBodyID handle = FromJoltBodyID(bodyID);

        if (settings.mIsSensor)
        {
            m_contactListener->MarkSensor(handle);

        }

        return handle;

	}

    void JoltScene::DestroyBody(PhysicsBodyID body)
    {

        m_contactListener->ForgetBody(body);

        JPH::BodyInterface& bodyInterface = m_physicsSystem.GetBodyInterface();

        const JPH::BodyID joltBody = ToJoltBodyID(body);

        // Jolt doesn't wake the bodies around a removed one, so one resting on it would stay
        // asleep in the air. The bounds are read before the body goes, and grown a little so
        // a body only touching it is included.
        JPH::AABox bounds = bodyInterface.GetTransformedShape(joltBody).GetWorldSpaceBounds();
        bounds.ExpandBy(JPH::Vec3::sReplicate(0.1f));

        bodyInterface.RemoveBody(joltBody);
        bodyInterface.DestroyBody(joltBody);

        bodyInterface.ActivateBodiesInAABox(bounds, JPH::BroadPhaseLayerFilter(), JPH::ObjectLayerFilter());


    }

    uint64_t JoltScene::GetUserData(PhysicsBodyID body) const
    {
        const JPH::BodyInterface& bodyInterface = m_physicsSystem.GetBodyInterface();

        return bodyInterface.GetUserData(ToJoltBodyID(body));

    }

    void JoltScene::SetTransform(PhysicsBodyID body, const glm::vec3& position, const glm::quat& rotation)
    {

        JPH::BodyInterface& bodyInterface = m_physicsSystem.GetBodyInterface();
        bodyInterface.SetPositionAndRotation(ToJoltBodyID(body), ToJolt(position), ToJolt(rotation), JPH::EActivation::Activate);

    }

    void JoltScene::GetTransform(PhysicsBodyID body, glm::vec3& outPosition, glm::quat& outRotation) const
    {

        const JPH::BodyInterface& bodyInterface = m_physicsSystem.GetBodyInterface();

        JPH::Vec3 pos;
        JPH::Quat rot;
        bodyInterface.GetPositionAndRotation(ToJoltBodyID(body), pos, rot);

        outPosition = FromJolt(pos);
        outRotation = FromJolt(rot);

    }

    void JoltScene::SetLinearVelocity(PhysicsBodyID body, const glm::vec3& velocity)
    {
        JPH::BodyInterface& bodyInterface = m_physicsSystem.GetBodyInterface();

        bodyInterface.SetLinearVelocity(ToJoltBodyID(body), ToJolt(velocity));

    }

    glm::vec3 JoltScene::GetLinearVelocity(PhysicsBodyID body) const
    {
        const JPH::BodyInterface& bodyInterface = m_physicsSystem.GetBodyInterface();

        return FromJolt(bodyInterface.GetLinearVelocity(ToJoltBodyID(body)));
    }

    void JoltScene::SetAngularVelocity(PhysicsBodyID body, const glm::vec3& velocity)
    {

        JPH::BodyInterface& bodyInterface = m_physicsSystem.GetBodyInterface();

        bodyInterface.SetAngularVelocity(ToJoltBodyID(body), ToJolt(velocity));
    }

    glm::vec3 JoltScene::GetAngularVelocity(PhysicsBodyID body) const
    {
        const JPH::BodyInterface& bodyInterface = m_physicsSystem.GetBodyInterface();

        return FromJolt(bodyInterface.GetAngularVelocity(ToJoltBodyID(body)));
    }

    void JoltScene::AddForce(PhysicsBodyID body, const glm::vec3& force)
    {

        JPH::BodyInterface& bodyInterface = m_physicsSystem.GetBodyInterface();

        bodyInterface.AddForce(ToJoltBodyID(body), ToJolt(force));
    }

    void JoltScene::AddImpulse(PhysicsBodyID body, const glm::vec3& impulse)
    {
        JPH::BodyInterface& bodyInterface = m_physicsSystem.GetBodyInterface();

        bodyInterface.AddImpulse(ToJoltBodyID(body), ToJolt(impulse));
    }

    void JoltScene::AddTorqueImpulse(PhysicsBodyID body, const glm::vec3& impulse)
    {
        JPH::BodyInterface& bodyInterface = m_physicsSystem.GetBodyInterface();

        bodyInterface.AddAngularImpulse(ToJoltBodyID(body), ToJolt(impulse));
    }

    bool JoltScene::CastRay(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, 
        PhysicsLayerMask mask, RayHit& outHit) const
    {
        const float directionLength = glm::length(direction);
        if (directionLength <= 0.0f || maxDistance <= 0.0f)
        {
            return false;
        }

        // RRayCast's direction includes the length: the ray ends at mOrigin + mDirection.

        JPH::RRayCast ray(
            ToJolt(origin),
            ToJolt(direction * (maxDistance / directionLength))
        );

        // mFraction starts just above 1, so the whole ray is considered.

        JPH::RayCastResult hit;

        const JoltLayers::MaskObjectLayerFilter layerFilters(mask);
        const IgnoreSensorsBodyFilter bodyFilter;

        if (!m_physicsSystem.GetNarrowPhaseQuery().CastRay(ray, hit, {}, layerFilters, bodyFilter))
        {
            return false;
        }
        
        // mFraction is a fraction of maxDistance.

        const JPH::RVec3 hitPosition = ray.GetPointOnRay(hit.mFraction);

        outHit.Body = FromJoltBodyID(hit.mBodyID);
        outHit.Position = FromJolt(hitPosition);
        outHit.Distance = hit.mFraction * maxDistance;
        outHit.Normal = glm::vec3(0.0f);

        // RayCastResult has no normal; the hit sub-shape gives it.
        const JPH::BodyLockRead lock(m_physicsSystem.GetBodyLockInterface(), hit.mBodyID);
        if (lock.Succeeded())
        {
            outHit.Normal =
                FromJolt(lock.GetBody().GetWorldSpaceSurfaceNormal(hit.mSubShapeID2, hitPosition));
        }

        return true;
    }

    void JoltScene::OverlapSphere(const glm::vec3& center, float radius, PhysicsLayerMask mask, 
        std::vector<PhysicsBodyID>& outBodies) const
    {

        outBodies.clear();

        if (radius <= 0.0f)
        {
            return;
        }

        JPH::SphereShape shape(radius);
        shape.SetEmbedded();


        JPH::CollideShapeSettings settings;
        const JoltLayers::MaskObjectLayerFilter layerFilters(mask);
        const IgnoreSensorsBodyFilter bodyFilter;
        JPH::AllHitCollisionCollector<JPH::CollideShapeCollector> collector;

        // {} is the broad-phase filter; the layer and sensor filters follow it.
        m_physicsSystem.GetNarrowPhaseQuery().CollideShape(&shape, JPH::Vec3::sOne(),
            JPH::Mat44::sTranslation(ToJolt(center)), settings, JPH::Vec3::sZero(), collector,
            {}, layerFilters, bodyFilter);

        // One hit per sub-shape, so a compound body can appear more than once. Sorting
        // removes the repeats.
        std::vector<JPH::BodyID> ids;
        ids.reserve(collector.mHits.size());
        for (const auto& hit : collector.mHits)
        {
            ids.push_back(hit.mBodyID2);
        }

        std::sort(ids.begin(), ids.end());
        ids.erase(std::unique(ids.begin(), ids.end()), ids.end());

        outBodies.reserve(ids.size());
        for (const JPH::BodyID& id : ids)
        {
            outBodies.push_back(FromJoltBodyID(id));
        }

    }

    void JoltScene::ConsumeContactEvents(std::vector<ContactEvent>& out)
    {

        m_contactListener->Drain(out);

    }

    void JoltScene::SetGravity(const glm::vec3& gravity)
    {

        m_physicsSystem.SetGravity(ToJolt(gravity));

    }

    glm::vec3 JoltScene::GetGravity() const
    {
        
        return FromJolt(m_physicsSystem.GetGravity());

    }

    void JoltScene::DebugDraw(PhysicsDebugRenderer& renderer)
    {
#ifdef JPH_DEBUG_RENDERER 

        JoltDebugRenderer* joltRenderer = GetJoltData().DebugRenderer.get();
        if (!joltRenderer)
        {
            return;
        }

        JPH::BodyManager::DrawSettings settings;
        settings.mDrawShapeWireframe = true;

        joltRenderer->SetSink(&renderer);
        m_physicsSystem.DrawBodies(settings, joltRenderer);
        joltRenderer->SetSink(nullptr);

#else
        (void)renderer;
#endif
    }

    void JoltScene::OptimizeBroadPhase()
    {

        m_physicsSystem.OptimizeBroadPhase();

    }

}