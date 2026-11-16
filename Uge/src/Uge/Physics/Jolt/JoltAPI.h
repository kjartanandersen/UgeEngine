#pragma once

#include "Uge/Physics/PhysicsAPI.h"

namespace Uge
{
	/**
	 * @brief Uge::PhysicsAPI backed by Jolt.
	 * @ingroup group_physics
	 *
	 * Owns the process-wide Jolt state. No Jolt type appears in this header.
	 */
	class JoltAPI : public PhysicsAPI
	{

	public:
		/** @brief Creates the backend; Init() starts it. */
		JoltAPI();
		/** @brief Shuts the backend down if it is still running. */
		~JoltAPI() override;

		virtual void Init() override;
		virtual void Shutdown() override;

		Scope<PhysicsScene> CreateScene(const PhysicsSceneDesc& desc) override;


	};

}