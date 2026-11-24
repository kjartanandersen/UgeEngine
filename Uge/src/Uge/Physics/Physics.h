#pragma once

#include "Uge/Physics/PhysicsAPI.h"

namespace Uge
{
	class PhysicsScene;
	struct PhysicsSceneDesc;

	/**
     * @brief Static access to the physics backend, which creates the worlds.
     * @ingroup group_physics
     */
	class Physics
	{

	public:
		/** @brief Starts the backend. Call once. */
		static void Init();

		/** @brief Shuts the backend down. Call after every Uge::PhysicsScene is destroyed. */
		static void Shutdown();

		/**
		 * @brief Creates a physics world.
		 * @param desc Limits and gravity.
		 * @return The world.
		 */
		static Scope<PhysicsScene> CreateScene(const PhysicsSceneDesc& desc);

	private:
		static Scope<PhysicsAPI> s_physicsAPI;

	};

}