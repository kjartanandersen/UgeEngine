#include <ugpch.h>
#include "JoltScene.h"

#include "Uge/Physics/Jolt/JoltUtils.h"
#include "Uge/Physics/Jolt/JoltData.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/Body/BodyFilter.h>
#include <Jolt/Physics/Body/BodyLock.h>
#include <Jolt/Physics/Collision/NarrowPhaseQuery.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>

#include <mutex>
#include <unordered_set>

namespace Uge
{

    /// Builds the primitive shape for one collider. Returns an empty ref on failure.
    static JPH::ShapeRefC BuildColliderShape(const ColliderDesc& collider)
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
        else
        {
            // MeshShapeDesc — not yet supported, see JoltScene.h.
            UG_CORE_WARN("Physics: mesh colliders are not implemented yet; collider skipped.");
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
        explicit JoltContactListener(JPH::PhysicsSystem& physicsSystem)
            : m_physicsSystem(physicsSystem)
        {
        }

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

        /** @brief Moves the buffered events into @p out, keeping the buffer's capacity. */
        void Drain(std::vector<ContactEvent>& out)
        {
            std::scoped_lock lock(m_mutex);
            out.swap(m_pending);
            m_pending.clear();
        }

        /** @brief Records a sensor body. Main thread only; never during Step(). */
        void MarkSensor(PhysicsBodyID body) { m_sensorBodies.insert(body.Value); }
        /** @brief Drops a destroyed body. Main thread only; never during Step(). */
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

        m_debugRenderer = CreateScope<DebugRendererImpl>();

	}

    JoltScene::~JoltScene()
    {

        // m_contactListener is declared after m_physicsSystem, so it is destroyed first.
        // Detaching explicitly means the PhysicsSystem never holds a dangling listener.
        m_physicsSystem.SetContactListener(nullptr);

    }

    void JoltScene::Step(float fixedDeltaTime, int collisionSteps)
    {

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

            shape = BuildColliderShape(collider);
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
                JPH::ShapeRefC sub = BuildColliderShape(collider);
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

        // --- 2. Bake the entity's scale into the shape ------------------------------
        // Jolt bodies have no scale of their own; this is baked once, at creation.
        if (desc.Scale != glm::vec3(1.0f))
        {
            JPH::Ref<JPH::ShapeSettings> scaled =
                new JPH::ScaledShapeSettings(shape.GetPtr(), ToJolt(desc.Scale));

            JPH::ShapeSettings::ShapeResult result = scaled->Create();
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
                desc.Colliders.size());
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

        bodyInterface.RemoveBody(ToJoltBodyID(body));

    }

    uint64_t JoltScene::GetUserData(PhysicsBodyID body) const
    {
        return 0;
    }

    void JoltScene::SetTransform(PhysicsBodyID body, const glm::vec3& position, const glm::quat& rotation)
    {
    }

    void JoltScene::GetTransform(PhysicsBodyID body, glm::vec3& outPosition, glm::quat& outRotation) const
    {
    }

    void JoltScene::SetLinearVelocity(PhysicsBodyID body, const glm::vec3& velocity)
    {
    }

    glm::vec3 JoltScene::GetLinearVelocity(PhysicsBodyID body) const
    {
        return glm::vec3();
    }

    void JoltScene::SetAngularVelocity(PhysicsBodyID body, const glm::vec3& velocity)
    {
    }

    glm::vec3 JoltScene::GetAngularVelocity(PhysicsBodyID body) const
    {
        return glm::vec3();
    }

    void JoltScene::AddForce(PhysicsBodyID body, const glm::vec3& force)
    {
    }

    void JoltScene::AddImpulse(PhysicsBodyID body, const glm::vec3& impulse)
    {
    }

    void JoltScene::AddTorqueImpulse(PhysicsBodyID body, const glm::vec3& impulse)
    {
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

    void JoltScene::OverlapSphere(const glm::vec3& center, float radius, PhysicsLayerMask mask, std::vector<PhysicsBodyID>& outBodies) const
    {
    }

    void JoltScene::ConsumeContactEvents(std::vector<ContactEvent>& out)
    {

        m_contactListener->Drain(out);

    }

    void JoltScene::SetGravity(const glm::vec3& gravity)
    {
    }

    glm::vec3 JoltScene::GetGravity() const
    {
        return glm::vec3();
    }

    void JoltScene::DebugDraw(PhysicsDebugRenderer& renderer)
    {
#ifdef JPH_DEBUG_RENDERER 

        JPH::BodyManager::DrawSettings settings;

        m_physicsSystem.DrawBodies(settings, m_debugRenderer.get());


#endif
    }

    void JoltScene::OptimizeBroadPhase()
    {
    }

}