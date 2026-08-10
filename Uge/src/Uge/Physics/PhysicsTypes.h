#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Uge
{
	/**
		@enum  Uge::BodyType
		@brief How the simulation moves a body.
	**/
	enum class BodyType
	{

		Static		= 0,	///< Never moves
		Kinematic	= 1,	///< Can be moved by game code, can push dynamic bodies, ignores force
		Dynamic		= 2		///< Fully simulated. Gravity, forces and collision response

	};

	/** @brief Broad collision category, used to build the backend's filter tables. */
	enum class PhysicsLayer : uint8_t
	{
		Static = 0, ///< Level geometry. Static-vs-Static pairs are never tested.
		Moving = 1, ///< Anything that can move.
		Count  = 2
	};

	/** @brief Bitmask over Uge::PhysicsLayer, for filtering queries. */
	using PhysicsLayerMask = uint32_t;
	/** @brief A mask matching every layer. */
	constexpr PhysicsLayerMask PhysicsLayerMaskAll = ~0u;

	/**
	 * @brief Opaque handle to a body inside one Uge::PhysicsScene.
	 *
	 * Deliberately a POD rather than a polymorphic object: bodies are touched once per
	 * entity per frame, and a vtable dispatch there buys nothing. Operations go through
	 * Uge::PhysicsScene, which is where the backend lives.
	 *
	 * @warning Only meaningful in the world that produced it, and only until that body is
	 * destroyed. Never serialize one.
	 */
	struct PhysicsBodyID
	{

		static constexpr uint32_t Invalid = ~0u; ///< Sentinel for "no body".

		uint32_t Value = Invalid; ///< Backend-defined body index.

		/** @brief Whether the handle refers to a body. */
		bool IsValid() const { return Value != Invalid; }
		bool operator==(const PhysicsBodyID& other) const { return Value == other.Value; }
		bool operator!=(const PhysicsBodyID& other) const { return Value != other.Value; }

	};

	/** @brief Surface response properties of a collider. */
	struct PhysicsMaterial
	{
		float Friction = 0.2f;    ///< Coulomb friction coefficient; `0` is frictionless.
		float Restitution = 0.0f; ///< Bounciness in `[0, 1]`; `0` absorbs all energy.
		float Density = 1000.0f;  ///< kg/m^3, used to derive mass from shape volume.
	};

	/** @brief Result of a successful Uge::PhysicsScene::CastRay. */
	struct RayHit
	{
		PhysicsBodyID Body;                        ///< Body that was hit.
		glm::vec3 Position = { 0.0f, 0.0f, 0.0f }; ///< World-space point of impact.
		glm::vec3 Normal = { 0.0f, 0.0f, 0.0f };   ///< Surface normal at #Position.
		float Distance = 0.0f;                     ///< Distance along the ray to #Position.
	};

	/** @brief What happened between two bodies during a step. */
	enum class ContactType
	{
		Begin = 0, ///< First step in which the pair touched.
		End = 1    ///< First step in which the pair stopped touching.
	};

	/**
	 * @brief One collision or trigger event produced by a step.
	 * @see PhysicsScene::ConsumeContactEvents
	 */
	struct ContactEvent
	{
		PhysicsBodyID BodyA;					///< First body in the pair.
		PhysicsBodyID BodyB;					///< Second body in the pair.
		ContactType Type = ContactType::Begin;	///< Whether contact began or ended.
		bool IsTrigger = false;					///< `true` if either body is a sensor.
	};

	/** @brief Construction limits and initial state for a Uge::PhysicsScene. */
	struct PhysicsSceneDesc
	{
		glm::vec3 Gravity = { 0.0f, -9.81f, 0.0f }; ///< Constant acceleration, m/s^2.
		uint32_t MaxBodies = 10240;					///< Hard cap on simultaneous bodies.
		uint32_t MaxBodyPairs = 10240;				///< Broad-phase pair buffer size.
		uint32_t MaxContactConstraints = 4096;		///< Narrow-phase constraint buffer size.
		uint32_t NumBodyMutexes = 0;				///< `0` lets the backend choose.
	};

}