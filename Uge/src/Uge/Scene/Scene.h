#pragma once

#include <entt.hpp>


#include "Uge/Core/Timestep.h"

namespace Uge
{

	class Scene
	{

	public:
		Scene();
		~Scene();

		entt::entity CreateEntity();

		// TODO: TEMP, Remove
		entt::registry& Reg() { return m_registry; }

		void OnUpdate(Timestep ts);

	private:
		entt::registry m_registry;

	};


}