/**
 * @file Scene.h
 * @brief The scene: entities, updates and rendering.
 * @ingroup group_scene
 */

#pragma once

#include <entt.hpp>

#include "Uge/Renderer/EditorCamera.h"
#include "Uge/Asset/Asset.h"
#include "Uge/Core/UUID.h"
#include "Uge/Core/Timestep.h"
#include "Uge/Physics/PhysicsScene.h"
namespace Uge
{
	class Entity;

	/**
	 * @brief A world of entities and components, stored as a `.uge` asset.
	 * @ingroup group_scene
	 *
	 * OnUpdateEditor() only renders, from the editor camera. OnUpdateRuntime() runs scripts
	 * and physics and renders from the primary camera.
	 */
	class Scene : public Asset
	{

	public:
		/** @brief An empty scene. */
		Scene();
		/** @brief Destroys the scene and its entities. */
		~Scene();

		/**
		 * @brief Copies a scene, keeping every entity's UUID.
		 * @param other Scene to copy.
		 * @return The copy.
		 *
		 * Play mode runs on a copy. Only the types in Uge::AllComponents are copied.
		 */
		static Ref<Scene> Copy(Ref<Scene> other);

		/**
		 * @brief Creates an entity with a new UUID, a tag and a transform.
		 * @param name Its name.
		 * @return The entity.
		 */
		Entity CreateEntity(const std::string& name = std::string());
		/**
		 * @brief Creates an entity with a given UUID.
		 * @param uuid The UUID; must not be in use.
		 * @param name Its name.
		 * @return The entity.
		 */
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
		/**
		 * @brief An entity by UUID.
		 * @param uuid The UUID.
		 * @return The entity, or an invalid handle.
		 */
		Entity GetEntityByUUID(UUID uuid);
		/**
		 * @brief An entity by name.
		 * @param name The name.
		 * @return A matching entity, or an invalid handle. With duplicate names, any match.
		 */
		Entity FindEntityByName(std::string_view name);

		/**
		 * @brief Destroys an entity.
		 * @param entity The entity; handles to it become invalid.
		 */
		void DestroyEntity(Entity entity);

		/**
		 * @brief Unloads a mesh asset if no entity in the scene uses it.
		 * @param mesh The mesh; `0` is ignored.
		 *
		 * Call it after removing a mesh reference; DestroyEntity() already does. Does nothing
		 * while the scene runs, since the editor scene still uses the same meshes.
		 */
		void ReleaseMeshIfUnused(AssetHandle mesh);

		/** @brief Starts play mode: physics bodies, then scripts. */
		void OnRuntimeStart();
		/** @brief Ends play mode. */
		void OnRuntimeStop();

		/**
		 * @brief Runs one frame of play mode and renders from the primary camera.
		 * @param ts Frame time.
		 *
		 * While paused, only stepped frames run. @see Step
		 */
		void OnUpdateRuntime(Timestep ts);

		/**
		 * @brief Renders the scene from the editor camera.
		 * @param ts Frame time.
		 * @param camera The editor camera.
		 */
		void OnUpdateEditor(Timestep ts, EditorCamera& camera);

		/**
		 * @brief Resizes every camera without a fixed aspect ratio.
		 * @param width Viewport width in pixels.
		 * @param height Viewport height in pixels.
		 */
		void OnViewportResize(uint32_t width, uint32_t height);

		/**
		 * @brief The camera play mode renders through.
		 * @return The first entity with a primary Uge::CameraComponent, or an invalid handle.
		 */
		Entity GetPrimaryCameraEntity();

		/**
		 * @brief Whether the scene is in play mode.
		 * @return `true` between OnRuntimeStart() and OnRuntimeStop().
		 */
		bool IsRunning() const { return m_isRunning; }

		/**
		 * @brief The physics world.
		 * @return The world, or null outside play mode.
		 */
		PhysicsScene* GetPhysicsScene() { return m_physicsScene.get(); }

		/**
		 * @brief Whether play mode is paused.
		 * @return `true` if paused.
		 */
		bool IsPaused() const { return m_isPaused; }

		/**
		 * @brief Pauses or resumes play mode. Rendering continues while paused.
		 * @param paused `true` to pause.
		 */
		void SetPaused(bool paused) { m_isPaused = paused; }

		/**
		 * @brief Runs a paused scene for a number of frames.
		 * @param frames Frames to run.
		 */
		void Step(int frames = 1);

		/**
		 * @brief Copies an entity and its components, with a new UUID.
		 * @param entity Entity to copy.
		 * @return The copy.
		 */
		Entity DuplicateEntity(Entity entity);

		/**
		 * @brief Every entity with all of the given components.
		 * @tparam Components Component types.
		 * @return An `entt::view`. Don't create or destroy entities while iterating it.
		 */
		template <typename... Components>
		auto GetAllEntitiesWith() { return m_registry.view<Components...>(); }

		/**
		 * @brief The EnTT registry.
		 * @return The registry.
		 */
		const entt::registry& GetRegistry() const { return m_registry; }

		/**
		 * @brief The asset type.
		 * @return Uge::AssetType::Scene.
		 */
		static AssetType GetStaticType() { return AssetType::Scene; }
		/**
		 * @brief The asset type.
		 * @return Uge::AssetType::Scene.
		 */
		virtual AssetType GetType() const override { return GetStaticType(); }

	private:
		/**
		 * @brief Called when a component is added to an entity.
		 * @tparam T Component type.
		 * @param entity The entity.
		 * @param component The component.
		 *
		 * Specialized for each component type in `Scene.cpp`; a missing specialization fails
		 * to link.
		 */
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);


		void OnRenderUpdateRuntime(Timestep ts);

		void CreatePhysicsBody(Entity entity);



	private:
		entt::registry m_registry;

		std::unordered_map<UUID, entt::entity> m_entityMap;

		uint32_t m_viewportWidth = 0, m_viewportHeight = 0;

		bool m_isRunning = false;
		bool m_isPaused = false;

		int m_stepFrames = 0;

		Scope<PhysicsScene> m_physicsScene;
		float m_physicsAccumulator = 0.0f;
		std::vector<ContactEvent> m_contactEvents;


		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;

	};



}