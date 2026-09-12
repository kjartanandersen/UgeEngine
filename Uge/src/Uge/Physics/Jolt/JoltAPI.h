#pragma once

#include "Uge/Physics/PhysicsAPI.h"

namespace Uge
{
	/**
	 * @brief Uge::PhysicsAPI implementation backed by Jolt Physics.
	 * @ingroup group_physics
	 *
	 * Owns the process-wide Jolt state: the allocator, the type factory, the temporary
	 * allocator and the job system thread pool. No Jolt type appears in this header, so
	 * the rest of the engine never needs Jolt's compile defines.
	 */
	class JoltAPI : public PhysicsAPI
	{

	public:
		/** @brief Constructs the backend without booting it; Init() does that. */
		JoltAPI();
		/** @brief Shuts the backend down if Init() ran and Shutdown() did not. */
		~JoltAPI() override;

		virtual void Init() override;
		virtual void Shutdown() override;

		Scope<PhysicsScene> CreateScene(const PhysicsSceneDesc& desc) override;


	};

}