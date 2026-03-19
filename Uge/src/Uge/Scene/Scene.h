#pragma once

#include <entt.hpp>

#include "Uge/Renderer/EditorCamera.h"
#include "Uge/Core/UUID.h"
#include "Uge/Core/Timestep.h"

namespace Uge
{
	class Entity;

	class Scene
	{

	public:
		Scene();
		~Scene();

		static Ref<Scene> Copy(Ref<Scene> other);

		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
		Entity GetEntityByUUID(UUID uuid);

		void DestroyEntity(Entity entity);

		void OnRuntimeStart();
		void OnRuntimeStop();
		void OnUpdateRuntime(Timestep ts);


		
		void OnUpdateEditor(Timestep ts, EditorCamera& camera);

		void OnViewportResize(uint32_t width, uint32_t height);

		Entity GetPrimaryCameraEntity();

	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);


	private:
		entt::registry m_registry;

		std::unordered_map<UUID, entt::entity> m_entityMap;

		uint32_t m_viewportWidth = 0, m_viewportHeight = 0;


		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;

	};



}