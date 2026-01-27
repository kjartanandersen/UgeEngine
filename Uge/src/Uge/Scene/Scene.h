#pragma once

#include <entt.hpp>


#include "Uge/Core/Timestep.h"

namespace Uge
{
	class Entity;

	class Scene
	{

	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = std::string());


		void OnUpdate(Timestep ts);

	private:
		entt::registry m_registry;

		friend class Entity;

	};


}