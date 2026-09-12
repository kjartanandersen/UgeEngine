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

#include <mutex>
#include <unordered_set>

namespace Uge
{

    /// Builds the primitive shape for one collider. Returns an empty ref on failure.
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
            // Jolt's argument order is (halfHeightOfCylinder, radius) — the reverse of ours.
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

            // JPH::MeshShape::MustBeStatic() is true, and both CompoundShape and DecoratedShape
            // propagate it, so one triangle mesh makes the whole body static-only. A triangle
            // soup also has no volume, so Jolt cannot derive mass from it. Downgrade rather than
            // drop the collider: a moving body with a hull is far more useful than no body.
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
                    // Almost always "too many points in hull". Interior points are already discarded,
                    // so the only remaining knob is how far a point may sit outside the hull.
                    s->mHullTolerance = 0.05f;
                    hull = s->Create();
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

                // MeshShapeSettings derives from ShapeSettings, not ConvexShapeSettings: there is
                // no SetDensity, because a triangle soup has no volume. Density is ignored here.
                // The constructor calls Sanitize(), dropping duplicate and degenerate triangles.
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
        /// Excludes sensors from queries: a trigger volume should not block a line-of-sight ray.
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
     * @brief Buffers Jolt contact callbacks for the main thread to drain.
     *
     * @warning Every method except the sensor bookkeeping runs on Jolt's worker threads,
     * concurrently, while PhysicsSystem::Update is in flight and all bodies are locked.
     * Nothing here may touch entt::registry, resolve an asset, or call into Mono — buffer
     * only, and dispatch after Step() returns.
     */
    class JoltContactListener : public JPH::ContactListener
    {
    public:
        /**
         * @brief Binds the listener to the system whose contacts it buffers.
         * @param physicsSystem System queried for WereBodiesInContact during callbacks.
         */
        explicit JoltContactListener(JPH::PhysicsSystem& physicsSystem)
            : m_physicsSystem(physicsSystem)
        {
        }

        /**
         * @brief Buffers a ContactType::Begin the first step two bodies touch.
         * @param body1 First body of the pair, in Jolt's sorted order.
         * @param body2 Second body of the pair, in Jolt's sorted order.
         * @param manifold Contact manifold for this sub-shape pair; unused.
         * @param settings Mutable contact response; deliberately left untouched.
         */
        void OnContactAdded(const JPH::Body& body1, const JPH::Body& body2,
            const JPH::ContactManifold& manifold,
            JPH::ContactSettings& settings) override
        {
            // One callback per sub-shape pair: a compound body fires several for a single
            // body pair. False here means this is the step the two bodies first touched.
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
         * @brief Buffers a ContactType::End once the last sub-shape contact disappears.
         * @param pair Sub-shape pair that stopped touching.
         */
        void OnContactRemoved(const JPH::SubShapeIDPair& pair) override
        {
            // Symmetric: true means other sub-shape contacts survive, so the bodies have
            // not actually separated yet.
            if (m_physicsSystem.WereBodiesInContact(pair.GetBody1ID(), pair.GetBody2ID()))
            {
                return;

            }

            const PhysicsBodyID a = FromJoltBodyID(pair.GetBody1ID());
            const PhysicsBodyID b = FromJoltBodyID(pair.GetBody2ID());

            // The bodies cannot be read here — they may already be destroyed — so the
            // sensor flag comes from the set instead.
            Push(ContactEvent{ a, b, ContactType::End,
                               IsSensorBody(a) || IsSensorBody(b) });
        }

        /**
         * @brief Moves the buffered events into @p out, keeping the buffer's capacity.
         * @param out Receives the pending events; its previous contents are discarded.
         */
        void Drain(std::vector<ContactEvent>& out)
        {
            std::scoped_lock lock(m_mutex);
            out.swap(m_pending);
            m_pending.clear();
        }

        /** @brief Records a sensor body. Main thread only; never during Step(). @param body Sensor body. */
        void MarkSensor(PhysicsBodyID body) { m_sensorBodies.insert(body.Value); }
        /** @brief Drops a destroyed body. Main thread only; never during Step(). @param body Body being destroyed. */
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

        // Written only by CreateBody/DestroyBody on the main thread and read lock-free from
        // the callbacks. Sound because body creation never overlaps PhysicsSystem::Update.
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

        // m_contactListener is declared after m_physicsSystem, so it is destroyed first.
        // Detaching explicitly means the PhysicsSystem never holds a dangling listener.
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

        // --- 1. Bake each collider, then combine ------------------------------------

        JPH::ShapeRefC shape;

        if (desc.Colliders.size() == 1)
        {
            const ColliderDesc& collider = desc.Colliders[0];

            shape = BuildColliderShape(collider, desc.Type);
            if (shape == nullptr)
                return {};

            // A single collider carries its offset in a decorator; a compound would
            // add a level of indirection for nothing.
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
            // CompoundShapeSettings::AddShape already takes a local transform, so the
            // per-collider offset goes there — no RotatedTranslated wrapper needed.

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

        // --- 2. Bake the entity's scale into the shape ------------------------------
        // Jolt bodies have no scale of their own; this is baked once, at creation.
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

        // --- 3. Describe the body ---------------------------------------------------

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

        // Jolt stores friction/restitution on the body, not the shape, so a multi-collider
        // body gets one surface response: the first collider's. Same for the sensor flag.
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

        // Mass == 0 means "derive it from shape volume and density", which is Jolt's default.
        if (desc.Mass > 0.0f && motionType == JPH::EMotionType::Dynamic)
        {
            settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
            settings.mMassPropertiesOverride.mMass = desc.Mass;
        }

        // --- 4. Create and add ------------------------------------------------------

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
        bodyInterface.RemoveBody(joltBody);
        bodyInterface.DestroyBody(joltBody);

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

        // JPH::RRayCast stores direction and length in one vector: the ray ends at
        // mOrigin + mDirection. Our contract keeps them separate, so combine here.

        JPH::RRayCast ray(
            ToJolt(origin),
            ToJolt(direction * (maxDistance / directionLength))
        );

        // mFraction defaults to 1.0f + FLT_EPSILON, i.e. "consider the whole ray".

        JPH::RayCastResult hit;

        const JoltLayers::MaskObjectLayerFilter layerFilters(mask);
        const IgnoreSensorsBodyFilter bodyFilter;

        if (!m_physicsSystem.GetNarrowPhaseQuery().CastRay(ray, hit, {}, layerFilters, bodyFilter))
        {
            return false;
        }
        
        // mFraction is along the scaled direction vector, so it is already normalized
        // against maxDistance.

        const JPH::RVec3 hitPosition = ray.GetPointOnRay(hit.mFraction);

        outHit.Body = FromJoltBodyID(hit.mBodyID);
        outHit.Position = FromJolt(hitPosition);
        outHit.Distance = hit.mFraction * maxDistance;
        outHit.Normal = glm::vec3(0.0f);

        // RayCastResult has no normal; deriving it needs the body and the sub-shape it hit.
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
        JPH::AllHitCollisionCollector<JPH::CollideShapeCollector> collector;

        m_physicsSystem.GetNarrowPhaseQuery().CollideShape(&shape, JPH::Vec3::sOne(), JPH::Mat44::sTranslation(ToJolt(center)), 
                                                            settings, JPH::Vec3::sZero(), collector, {});

        outBodies.reserve(collector.mHits.size());

        for (const auto& hit : collector.mHits)
        {
            const PhysicsBodyID id = FromJoltBodyID(hit.mBodyID2);
            if (std::find(outBodies.begin(), outBodies.end(), id) == outBodies.end())
            {
                outBodies.push_back(FromJoltBodyID(hit.mBodyID2));

            }

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