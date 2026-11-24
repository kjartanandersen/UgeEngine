#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Uge
{
	/**
	 * @brief How the simulation moves a body.
	 * @ingroup group_physics
	 */
	enum class BodyType
	{

		Static		= 0,	///< Never moves.
		Kinematic	= 1,	///< Moved by code; pushes dynamic bodies and ignores forces.
		Dynamic		= 2		///< Moved by the simulation: gravity, forces and collisions.

	};

	/** @brief Collision layers. */
	enum class PhysicsLayer : uint8_t
	{
		Static = 0, ///< Level geometry. Two static bodies are never tested.
		Moving = 1, ///< Anything that can move.
		Count  = 2  ///< Number of layers.
	};

	/** @brief Bit mask of Uge::PhysicsLayer values, for filtering queries. */
	using PhysicsLayerMask = uint32_t;
	/** @brief A mask with every layer. */
	constexpr PhysicsLayerMask PhysicsLayerMaskAll = ~0u;

	/**
	 * @brief A body in one Uge::PhysicsScene.
	 *
	 * Valid only in the world that created it, until the body is destroyed. Never save one.
	 */
	struct PhysicsBodyID
	{

		static constexpr uint32_t Invalid = ~0u; ///< No body.

		uint32_t Value = Invalid; ///< The backend's body index.

		/** @brief Whether the handle names a body. @return `false` for #Invalid. */
		bool IsValid() const { return Value != Invalid; }
		/** @brief Equality. @param other Handle to compare. @return `true` for the same body. */
		bool operator==(const PhysicsBodyID& other) const { return Value == other.Value; }
		/** @brief Inequality. @param other Handle to compare. @return `true` for different bodies. */
		bool operator!=(const PhysicsBodyID& other) const { return Value != other.Value; }

	};

	/** @brief A collider's surface properties. */
	struct PhysicsMaterial
	{
		float Friction = 0.2f;    ///< Friction coefficient; `0` is frictionless.
		float Restitution = 0.0f; ///< Bounciness from `0` to `1`.
		float Density = 1000.0f;  ///< In kg/m^3, for deriving mass from volume.
	};

	/** @brief A ray hit from Uge::PhysicsScene::CastRay. */
	struct RayHit
	{
		PhysicsBodyID Body;                        ///< The body hit.
		glm::vec3 Position = { 0.0f, 0.0f, 0.0f }; ///< World-space hit point.
		glm::vec3 Normal = { 0.0f, 0.0f, 0.0f };   ///< Surface normal at the hit.
		float Distance = 0.0f;                     ///< Distance along the ray.
	};

	/** @brief Whether two bodies started or stopped touching. */
	enum class ContactType
	{
		Begin = 0, ///< The step in which they first touched.
		End = 1    ///< The step in which they stopped touching.
	};

	/** @brief A collision or trigger event from a step. */
	struct ContactEvent
	{
		PhysicsBodyID BodyA;					///< First body.
		PhysicsBodyID BodyB;					///< Second body.
		ContactType Type = ContactType::Begin;	///< Began or ended.
		bool IsTrigger = false;					///< `true` if either body is a sensor.
	};

	/** @brief Limits and gravity for a Uge::PhysicsScene. */
	struct PhysicsSceneDesc
	{
		glm::vec3 Gravity = { 0.0f, -9.81f, 0.0f }; ///< In m/s^2.
		uint32_t MaxBodies = 10240;					///< Most bodies at once.
		uint32_t MaxBodyPairs = 10240;				///< Size of the broad-phase pair buffer.
		uint32_t MaxContactConstraints = 4096;		///< Size of the contact constraint buffer.
		uint32_t NumBodyMutexes = 0;				///< `0` lets the backend choose.
	};

}