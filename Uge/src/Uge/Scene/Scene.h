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
		Entity FindEntityByName(std::string_view name);

		void DestroyEntity(Entity entity);

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnUpdateRuntime(Timestep ts);
		
		void OnUpdateEditor(Timestep ts, EditorCamera& camera);

		void OnViewportResize(uint32_t width, uint32_t height);

		Entity GetPrimaryCameraEntity();

		bool IsRunning() const { return m_isRunning; }
		bool IsPaused() const { return m_isPaused; }

		void SetPaused(bool paused) { m_isPaused = paused; }

		void Step(int frames = 1);

		Entity DuplicateEntity(Entity entity);

	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);


		void OnRenderUpdateRuntime(Timestep ts);



	private:
		entt::registry m_registry;

		std::unordered_map<UUID, entt::entity> m_entityMap;

		uint32_t m_viewportWidth = 0, m_viewportHeight = 0;

		bool m_isRunning = false;
		bool m_isPaused = false;

		int m_stepFrames = 0;


		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;

	};



}