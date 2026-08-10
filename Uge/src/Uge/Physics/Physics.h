#pragma once

#include "Uge/Physics/PhysicsAPI.h"

namespace Uge
{
	class PhysicsScene;
	class PhysicsSceneDesc;

	/**
     * @brief Static facade over the active Uge::PhysicsAPI.
     * @ingroup group_physics
     *
     * Owns the process-wide backend: allocators, the job system and type registration.
     * Individual worlds are Uge::PhysicsScene objects created through CreateScene().
     *
     * @see PhysicsScene, Uge::JoltAPI
     */
	class Physics
	{

	public:
		/** @brief Boots the backend. Call once, from Uge::Application's constructor. */
		static void Init();

		/** @brief Tears the backend down. Call after every Uge::PhysicsScene is destroyed. */
		static void ShutDown();

		/**
		 * @brief Creates an independent physics world.
		 * @param desc Tuning limits and initial gravity for the world.
		 * @return An owning handle to the new world.
		 */
		static Scope<PhysicsScene> CreateScene(const PhysicsSceneDesc& desc);

	private:
		static Scope<PhysicsAPI> s_physicsAPI;

	};

}