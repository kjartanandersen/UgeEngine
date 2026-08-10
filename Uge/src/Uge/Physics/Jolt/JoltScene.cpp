#include <ugpch.h>
#include "JoltScene.h"

#include "Uge/Physics/Jolt/JoltAPI.h"
#include "Uge/Physics/Jolt/JoltUtils.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>

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


	JoltScene::JoltScene(const PhysicsSceneDesc& desc)
	{

		m_physicsSystem.Init(desc.MaxBodies, desc.NumBodyMutexes, desc.MaxBodyPairs, desc.MaxContactConstraints, 
			m_broadPhaseLayerInterface, m_objectVsBroadPhaseFilter, m_objectLayerPairFilter);

		SetGravity(desc.Gravity);

	}

    void JoltScene::Step(float fixedDeltaTime, int collisionSteps)
    {

        const JoltData* joltData = JoltAPI::GetJoltData();

        m_physicsSystem.Update(fixedDeltaTime, collisionSteps);

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

        return FromJoltBodyID(bodyID);

	}

}