#pragma once
#include "Uge/Physics/PhysicsTypes.h"
#include "Uge/Physics/PhysicsShapes.h"
#include "Uge/Physics/PhysicsDebugRenderer.h"

#include <vector>

namespace Uge
{

    /**
     * @brief One physics world.
     * @ingroup group_physics
     *
     * Each Uge::Scene has its own, so the editor scene and its play-mode copy can exist at
     * once. Not thread-safe: use it from the main thread.
     */
    class PhysicsScene
    {
    public:
        virtual ~PhysicsScene() = default;

        /**
         * @brief Advances the simulation.
         * @param fixedDeltaTime Seconds to advance. It must be the same on every call; a
         * variable step makes contacts jitter.
         * @param collisionSteps Collision steps within the interval; more is more stable.
         */
        virtual void Step(float fixedDeltaTime, int collisionSteps = 1) = 0;

        /**
         * @brief Creates a body and adds it to the world.
         * @param desc Body settings, with at least one collider.
         * @return The body, or an invalid handle on failure.
         */
        virtual PhysicsBodyID CreateBody(const BodyDesc& desc) = 0;
        /**
         * @brief Removes and destroys a body.
         * @param body The body; an invalid handle is ignored.
         */
        virtual void DestroyBody(PhysicsBodyID body) = 0;

        /**
         * @brief The Uge::BodyDesc::UserData a body was created with.
         * @param body The body.
         * @return The value, or `0` for an invalid handle.
         */
        virtual uint64_t GetUserData(PhysicsBodyID body) const = 0;

        /**
         * @brief Moves a body without resolving collisions.
         * @param body The body.
         * @param position World position.
         * @param rotation World rotation.
         *
         * For kinematic bodies; it can push a dynamic body into a wall.
         */
        virtual void SetTransform(PhysicsBodyID body, const glm::vec3& position, const glm::quat& rotation) = 0;

        /**
         * @brief A body's world transform.
         * @param body The body.
         * @param outPosition Receives the position.
         * @param outRotation Receives the rotation.
         */
        virtual void GetTransform(PhysicsBodyID body, glm::vec3& outPosition, glm::quat& outRotation) const = 0;

        /** @brief Sets a body's linear velocity. @param body Body. @param velocity In m/s. */
        virtual void SetLinearVelocity(PhysicsBodyID body, const glm::vec3& velocity) = 0;
        /** @brief A body's linear velocity. @param body Body. @return In m/s. */
        virtual glm::vec3 GetLinearVelocity(PhysicsBodyID body) const = 0;
        /** @brief Sets a body's angular velocity. @param body Body. @param velocity In radians/s. */
        virtual void SetAngularVelocity(PhysicsBodyID body, const glm::vec3& velocity) = 0;
        /** @brief A body's angular velocity. @param body Body. @return In radians/s. */
        virtual glm::vec3 GetAngularVelocity(PhysicsBodyID body) const = 0;

        /**
         * @brief Applies a force for the next step only.
         * @param body The body.
         * @param force In newtons.
         */
        virtual void AddForce(PhysicsBodyID body, const glm::vec3& force) = 0;
        /**
         * @brief Applies an impulse.
         * @param body The body.
         * @param impulse In newton-seconds.
         */
        virtual void AddImpulse(PhysicsBodyID body, const glm::vec3& impulse) = 0;
        /** @brief Applies an angular impulse. @param body Body. @param impulse In N*m*s. */
        virtual void AddTorqueImpulse(PhysicsBodyID body, const glm::vec3& impulse) = 0;

        /**
         * @brief The nearest body along a ray. Triggers are skipped.
         * @param origin World-space start.
         * @param direction Direction; need not be normalized.
         * @param maxDistance Longest distance.
         * @param mask Layers to test.
         * @param outHit Receives the hit; unchanged on a miss.
         * @return `true` on a hit.
         */
        virtual bool CastRay(const glm::vec3& origin, const glm::vec3& direction, float maxDistance,
            PhysicsLayerMask mask, RayHit& outHit) const = 0;

        /**
         * @brief The bodies overlapping a sphere, each once. Triggers are skipped.
         * @param center World-space center.
         * @param radius Radius.
         * @param mask Layers to test.
         * @param outBodies Cleared, then filled.
         */
        virtual void OverlapSphere(const glm::vec3& center, float radius, PhysicsLayerMask mask,
            std::vector<PhysicsBodyID>& outBodies) const = 0;

        /**
         * @brief Moves the contact events since the last call into @p out.
         * @param out Cleared, then filled.
         *
         * Events are produced on worker threads during Step(). Call this on the main thread,
         * after Step() returns.
         */
        virtual void ConsumeContactEvents(std::vector<ContactEvent>& out) = 0;

        /** @brief Sets gravity. @param gravity In m/s^2. */
        virtual void SetGravity(const glm::vec3& gravity) = 0;
        /** @brief The gravity. @return In m/s^2. */
        virtual glm::vec3 GetGravity() const = 0;

        /**
         * @brief Draws every body's wireframe. Does nothing in Dist.
         * @param renderer Receives the lines.
         */
        virtual void DebugDraw(PhysicsDebugRenderer& renderer) = 0;

        /**
         * @brief Rebuilds the broad phase.
         *
         * Call once after adding the bodies, before the first Step(). Too slow for every
         * frame.
         */
        virtual void OptimizeBroadPhase() = 0;





    };


}