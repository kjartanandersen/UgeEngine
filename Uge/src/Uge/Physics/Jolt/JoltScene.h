#pragma once

#include "Uge/Physics/PhysicsScene.h"
#include "JoltLayers.h"
#include "JoltPhysicsDebugRenderer.h"


#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/PhysicsSystem.h>



namespace Uge
{

    /** 
     * @brief Packs a Jolt body id into the engine's opaque handle. 
     * @param id Jolt body ID
     * 
     */
    inline PhysicsBodyID FromJoltBodyID(const JPH::BodyID& id)
    {
        return PhysicsBodyID{ id.GetIndexAndSequenceNumber() };
    }

    /** 
     * @brief Unpacks an engine handle back into a Jolt body id. 
     * @param id Engine body ID
     * 
     */
    inline JPH::BodyID ToJoltBodyID(PhysicsBodyID id)
    {
        return JPH::BodyID(id.Value);
    }

	class JoltScene : public PhysicsScene
	{

    public:
        JoltScene(const PhysicsSceneDesc& desc);
        ~JoltScene() override;
        /**
         * @brief Advances the simulation by a fixed interval.
         * @param fixedDeltaTime Seconds to advance; keep this constant across calls.
         * @param collisionSteps Sub-steps within the interval; more is more stable.
         *
         * @warning Must be a fixed step. Feeding a variable frame delta makes contacts
         * jitter and stacks explode. @see Uge::Scene::OnUpdateRuntime for the accumulator.
         */
        virtual void Step(float fixedDeltaTime, int collisionSteps = 1) override;

        /**
         * @brief Creates a body and adds it to the world.
         * @param desc Body parameters, including at least one collider.
         * @return A handle to the new body, or an invalid handle on failure.
         * 
         * @note Friction, restitution and the trigger flag are per-body in Jolt, per-collider in BodyDesc.
         *  A multi-collider body uses Colliders[0]'s values. Jolt's own PhysicsMaterial is only a
         *  debug name/colour in the base library — it carries no friction. Density is per-shape and is
         *  honoured per collider.
         * 
         * @note Static sensors only detect active dynamic/kinematic bodies (Body.h:73-75). A trigger
         * volume built as a BodyType::Static body will stop reporting a resting object once it sleeps.
         * Make trigger volumes Kinematic if that matters.
         * 
         * @note Scale is baked at creation. Changing TransformComponent::Scale at runtime does nothing
         * until the body is destroyed and rebuilt. (Already noted for Phase 2; it originates here.)
         */
        virtual PhysicsBodyID CreateBody(const BodyDesc& desc) override;
        /**
         * @brief Removes a body from the world and destroys it.
         * @param body Body to destroy; an invalid handle is ignored.
         */
        virtual void DestroyBody(PhysicsBodyID body) override;

        /**
         * @brief Value stored in Uge::BodyDesc::UserData at creation.
         * @param body Body to query.
         * @return The stored value, or `0` for an invalid handle.
         */
        virtual uint64_t GetUserData(PhysicsBodyID body) const override;

        /**
         * @brief Teleports a body, bypassing collision resolution.
         * @param body Body to move.
         * @param position New world position.
         * @param rotation New world orientation.
         *
         * The correct way to drive a Uge::BodyType::Kinematic body from game code.
         * @warning On a Dynamic body this can push it through walls.
         */
        virtual void SetTransform(PhysicsBodyID body, const glm::vec3& position, const glm::quat& rotation) override;

        /**
         * @brief Reads a body's current world transform.
         * @param body Body to query.
         * @param outPosition Receives the world position.
         * @param outRotation Receives the world orientation.
         */
        virtual void GetTransform(PhysicsBodyID body, glm::vec3& outPosition, glm::quat& outRotation) const override;

        /** @brief Sets a body's linear velocity in m/s. @param body Body. @param velocity Velocity. */
        virtual void SetLinearVelocity(PhysicsBodyID body, const glm::vec3& velocity) override;
        /** @brief Reads a body's linear velocity. @param body Body. @return Velocity in m/s. */
        virtual glm::vec3 GetLinearVelocity(PhysicsBodyID body) const override;
        /** @brief Sets a body's angular velocity. @param body Body. @param velocity Radians/s. */
        virtual void SetAngularVelocity(PhysicsBodyID body, const glm::vec3& velocity) override;
        /** @brief Reads a body's angular velocity. @param body Body. @return Radians/s. */
        virtual glm::vec3 GetAngularVelocity(PhysicsBodyID body) const override;

        /**
         * @brief Applies a continuous force for the next step.
         * @param body Body to push.
         * @param force Force in newtons.
         * @note Cleared after each Step(); apply every frame for a sustained push.
         */
        virtual void AddForce(PhysicsBodyID body, const glm::vec3& force) override;
        /**
         * @brief Applies an instantaneous change in momentum.
         * @param body Body to push.
         * @param impulse Impulse in newton-seconds.
         */
        virtual void AddImpulse(PhysicsBodyID body, const glm::vec3& impulse) override;
        /** @brief Applies an instantaneous angular impulse. @param body Body. @param impulse N·m·s. */
        virtual void AddTorqueImpulse(PhysicsBodyID body, const glm::vec3& impulse) override;

        /**
         * @brief Finds the nearest body along a ray.
         * @param origin World-space start of the ray.
         * @param direction Ray direction; need not be normalized.
         * @param maxDistance Longest distance to consider.
         * @param mask Layers to test against.
         * @param outHit Receives the hit, untouched if nothing was hit.
         * @return `true` if a body was hit.
         */
        virtual bool CastRay(const glm::vec3& origin, const glm::vec3& direction, float maxDistance,
            PhysicsLayerMask mask, RayHit& outHit) const override;

        /**
         * @brief Collects every body overlapping a sphere.
         * @param center World-space centre.
         * @param radius Sphere radius.
         * @param mask Layers to test against.
         * @param outBodies Cleared, then filled with the overlapping bodies.
         */
        virtual void OverlapSphere(const glm::vec3& center, float radius, PhysicsLayerMask mask,
            std::vector<PhysicsBodyID>& outBodies) const override;

        /**
         * @brief Moves the events produced since the last call into @p out.
         * @param out Cleared, then filled with the pending events.
         *
         * @warning The backend generates these on worker threads during Step(). Call this
         * on the main thread **after** Step() returns; never dispatch from inside the
         * backend's own callback.
         */
        virtual void ConsumeContactEvents(std::vector<ContactEvent>& out) override;

        /** @brief Sets the world's gravity vector. @param gravity Acceleration in m/s^2. */
        virtual void SetGravity(const glm::vec3& gravity) override;
        /** @brief Reads the world's gravity vector. @return Acceleration in m/s^2. */
        virtual glm::vec3 GetGravity() const override;

        /**
         * @brief Emits wireframes for every body into @p renderer.
         * @param renderer Line sink to draw into.
         * @note Compiled to a no-op in Dist.
         */
        virtual void DebugDraw(PhysicsDebugRenderer& renderer) override;

        /**
         * @brief Rebuilds the broad-phase acceleration structure.
         *
         * Call once after populating a world, before the first Step(). Too slow for
         * per-frame use.
         */
        virtual void OptimizeBroadPhase() override;

    private:
        // Declared before m_physicsSystem so they are destroyed after it:
        // JPH::PhysicsSystem holds references to all three for its whole lifetime.

        JoltLayers::BroadPhaseLayerInterfaceImpl      m_broadPhaseLayerInterface;
        JoltLayers::ObjectVsBroadPhaseLayerFilterImpl m_objectVsBroadPhaseFilter;
        JoltLayers::ObjectLayerPairFilterImpl         m_objectLayerPairFilter;

        JPH::PhysicsSystem m_physicsSystem;

        Scope<class JoltContactListener> m_contactListener;
        Scope<DebugRendererImpl> m_debugRenderer;


	};

}