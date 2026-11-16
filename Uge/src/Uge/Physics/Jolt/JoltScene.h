#pragma once

#include "Uge/Physics/PhysicsScene.h"
#include "JoltLayers.h"


#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/PhysicsSystem.h>



namespace Uge
{

    /**
     * @brief Converts a Jolt body ID to the engine's body handle.
     * @param id Jolt body ID.
     * @return The handle.
     */
    inline PhysicsBodyID FromJoltBodyID(const JPH::BodyID& id)
    {
        return PhysicsBodyID{ id.GetIndexAndSequenceNumber() };
    }

    /**
     * @brief Converts an engine body handle to a Jolt body ID.
     * @param id The handle.
     * @return The Jolt body ID.
     */
    inline JPH::BodyID ToJoltBodyID(PhysicsBodyID id)
    {
        return JPH::BodyID(id.Value);
    }

    /**
     * @brief The Jolt physics world.
     * @ingroup group_physics
     */
	class JoltScene : public PhysicsScene
	{

    public:

        /**
         * @brief Creates the world.
         * @param desc World settings.
         */
        JoltScene(const PhysicsSceneDesc& desc);

        /** @brief Destroys the world. */
        ~JoltScene() override;
        virtual void Step(float fixedDeltaTime, int collisionSteps = 1) override;

        // Jolt keeps friction, restitution and the trigger flag per body, so a body with
        // several colliders uses the first collider's. A static trigger only detects bodies
        // that are awake; make it kinematic to detect resting ones. Scale is applied at
        // creation only.
        virtual PhysicsBodyID CreateBody(const BodyDesc& desc) override;
        virtual void DestroyBody(PhysicsBodyID body) override;

        virtual uint64_t GetUserData(PhysicsBodyID body) const override;

        virtual void SetTransform(PhysicsBodyID body, const glm::vec3& position, const glm::quat& rotation) override;

        virtual void GetTransform(PhysicsBodyID body, glm::vec3& outPosition, glm::quat& outRotation) const override;

        virtual void SetLinearVelocity(PhysicsBodyID body, const glm::vec3& velocity) override;
        virtual glm::vec3 GetLinearVelocity(PhysicsBodyID body) const override;
        virtual void SetAngularVelocity(PhysicsBodyID body, const glm::vec3& velocity) override;
        virtual glm::vec3 GetAngularVelocity(PhysicsBodyID body) const override;

        virtual void AddForce(PhysicsBodyID body, const glm::vec3& force) override;
        virtual void AddImpulse(PhysicsBodyID body, const glm::vec3& impulse) override;
        virtual void AddTorqueImpulse(PhysicsBodyID body, const glm::vec3& impulse) override;

        virtual bool CastRay(const glm::vec3& origin, const glm::vec3& direction, float maxDistance,
            PhysicsLayerMask mask, RayHit& outHit) const override;

        virtual void OverlapSphere(const glm::vec3& center, float radius, PhysicsLayerMask mask,
            std::vector<PhysicsBodyID>& outBodies) const override;

        virtual void ConsumeContactEvents(std::vector<ContactEvent>& out) override;

        virtual void SetGravity(const glm::vec3& gravity) override;
        virtual glm::vec3 GetGravity() const override;

        virtual void DebugDraw(PhysicsDebugRenderer& renderer) override;

        virtual void OptimizeBroadPhase() override;

    private:
        // Declared before m_physicsSystem, which refers to them, so they outlive it.

        uint32_t m_maxBodies = 0;  ///< Most bodies the world can hold.

        JoltLayers::BroadPhaseLayerInterfaceImpl      m_broadPhaseLayerInterface;
        JoltLayers::ObjectVsBroadPhaseLayerFilterImpl m_objectVsBroadPhaseFilter;
        JoltLayers::ObjectLayerPairFilterImpl         m_objectLayerPairFilter;

        JPH::PhysicsSystem m_physicsSystem;

        Scope<class JoltContactListener> m_contactListener;


	};

}