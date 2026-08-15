/**
 * @file Scene.h
 * @brief The EnTT-backed scene: entity lifetime, updates and rendering.
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
	 * @brief A world of entities and components, and the update/render loop over them.
	 * @ingroup group_scene
	 *
	 * Wraps an `entt::registry` and is itself an Uge::Asset, so scenes are loaded and
	 * referenced by handle like any other content and serialized to `.uge` files by
	 * Uge::SceneSerializer.
	 *
	 * A scene runs in one of two modes. OnUpdateEditor() renders from the editor's
	 * camera and does not run gameplay logic, while OnUpdateRuntime() steps scripts and
	 * renders from the entity holding the primary Uge::CameraComponent. The editor's play
	 * button switches between them, working on a copy so edits are not lost on stop.
	 *
	 * Both paths drive Uge::Renderer2D and Uge::Model in separate passes, since those two
	 * do not share state.
	 *
	 * @see Entity, Uge::Components.h
	 */
	class Scene : public Asset
	{

	public:
		/** @brief Constructs an empty scene. */
		Scene();
		/** @brief Destroys the scene and every entity in it. */
		~Scene();

		/**
		 * @brief Deep-copies a scene, preserving every entity's UUID.
		 * @param other Scene to copy.
		 * @return An independent copy.
		 *
		 * How the editor enters play mode: the running scene is a copy, so stopping restores
		 * the original untouched.
		 *
		 * @note Only the component types listed in Uge::ComponentGroup are copied. A new
		 * component type must be added there or it will be silently dropped.
		 */
		static Ref<Scene> Copy(Ref<Scene> other);

		/**
		 * @brief Creates an entity with a fresh UUID.
		 * @param name Display name stored in its Uge::TagComponent.
		 * @return A handle to the new entity.
		 *
		 * The new entity always has an Uge::IDComponent, an Uge::TagComponent and an
		 * Uge::TransformComponent.
		 */
		Entity CreateEntity(const std::string& name = std::string());
		/**
		 * @brief Creates an entity with a specific UUID.
		 * @param uuid Identifier to assign; must not already be in use.
		 * @param name Display name stored in its Uge::TagComponent.
		 * @return A handle to the new entity.
		 *
		 * Used when deserializing, so cross-entity references survive a save/load round-trip.
		 */
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
		/**
		 * @brief Looks an entity up by identifier.
		 * @param uuid Identifier to find.
		 * @return The entity, or an invalid handle if no entity has that UUID.
		 */
		Entity GetEntityByUUID(UUID uuid);
		/**
		 * @brief Finds the first entity with a given name.
		 * @param name Tag to search for.
		 * @return The entity, or an invalid handle if none matches.
		 * @note Names are not unique; with duplicates, which one is returned is unspecified.
		 */
		Entity FindEntityByName(std::string_view name);

		/**
		 * @brief Removes an entity and all of its components from the scene.
		 * @param entity Entity to destroy; handles to it become invalid.
		 */
		void DestroyEntity(Entity entity);

		/**
		 * @brief Releases a mesh asset once nothing in the scene references it any more.
		 * @param mesh Handle a Uge::MeshComponent used to hold; `0` is ignored.
		 *
		 * Call this **after** the reference is gone — after DestroyEntity(), after removing a
		 * Uge::MeshComponent, or after clearing Uge::MeshComponent::Mesh. The scene is
		 * rescanned, and the asset is dropped only if no entity still points at it.
		 *
		 * DestroyEntity() already does this for the entity it destroys; the other two cases
		 * are the caller's job.
		 *
		 * @note Does nothing while the scene is running. A play-mode scene is a copy
		 * (@see Copy), and the editor scene it came from still holds the same handles.
		 */
		void ReleaseMeshIfUnused(AssetHandle mesh);

		/**
		 * @brief Enters play mode: starts the script engine and instantiates entity scripts.
		 */
		void OnRuntimeStart();
		/** @brief Leaves play mode and tears down the script instances. */
		void OnRuntimeStop();

		/**
		 * @brief Advances one frame of gameplay and renders from the primary camera.
		 * @param ts Frame delta time.
		 * @note Does nothing while paused, except when stepping. @see Step
		 */
		void OnUpdateRuntime(Timestep ts);
		
		/**
		 * @brief Renders the scene from the editor camera, without running gameplay logic.
		 * @param ts Frame delta time.
		 * @param camera Editor viewport camera.
		 */
		void OnUpdateEditor(Timestep ts, EditorCamera& camera);

		/**
		 * @brief Propagates a viewport resize to every non-fixed-aspect-ratio scene camera.
		 * @param width New viewport width in pixels.
		 * @param height New viewport height in pixels.
		 */
		void OnViewportResize(uint32_t width, uint32_t height);

		/**
		 * @brief Finds the camera the runtime renders through.
		 * @return The first entity whose Uge::CameraComponent is marked primary, or an invalid
		 *         handle if there is none — in which case nothing is drawn at runtime.
		 */
		Entity GetPrimaryCameraEntity();

		/**
		 * @brief Whether the scene is in play mode.
		 * @return `true` between OnRuntimeStart() and OnRuntimeStop().
		 */
		bool IsRunning() const { return m_isRunning; }

		/**
			@brief  Returns physics scene and is nullptr outside play mode
			@retval  - 
		**/
		PhysicsScene* GetPhysicsScene() { return m_physicsScene.get(); }

		/**
		 * @brief Whether runtime updates are suspended.
		 * @return `true` if paused.
		 */
		bool IsPaused() const { return m_isPaused; }

		/**
		 * @brief Pauses or resumes runtime updates.
		 * @param paused `true` to suspend gameplay while continuing to render.
		 */
		void SetPaused(bool paused) { m_isPaused = paused; }

		/**
		 * @brief Advances a paused scene by a fixed number of frames.
		 * @param frames How many frames to run before pausing again.
		 */
		void Step(int frames = 1);

		/**
		 * @brief Copies an entity and its components into this scene under a new UUID.
		 * @param entity Entity to duplicate.
		 * @return The new entity.
		 */
		Entity DuplicateEntity(Entity entity);

		/**
		 * @brief A view over every entity carrying all of the given components.
		 * @tparam Components Component types an entity must have to be included.
		 * @return An `entt::view`; iterate it for the entity IDs and call `view.get<T>(entity)`
		 *         for the components.
		 *
		 * Defined inline because it is a template: the body must be visible wherever it is
		 * instantiated.
		 *
		 * @code
		 * auto view = scene->GetAllEntitiesWith<MeshComponent>();
		 * for (auto entityID : view)
		 *     UG_INFO(view.get<MeshComponent>(entityID).Mesh);
		 * @endcode
		 *
		 * @warning Do not create or destroy entities while iterating the view.
		 */
		template <typename... Components>
		auto GetAllEntitiesWith() { return m_registry.view<Components...>(); }

		/**
		 * @brief The underlying EnTT registry.
		 * @return Const reference to the registry.
		 */
		const entt::registry& GetRegistry() const { return m_registry; }

		/**
		 * @brief The asset type this class represents.
		 * @return Uge::AssetType::Scene.
		 */
		static AssetType GetStaticType() { return AssetType::Scene; }
		/**
		 * @brief This instance's asset type.
		 * @return Uge::AssetType::Scene.
		 */
		virtual AssetType GetType() const override { return GetStaticType(); }

	private:
		/**
		 * @brief Hook invoked whenever a component of type `T` is attached to an entity.
		 * @tparam T Component type that was added.
		 * @param entity Entity that received the component.
		 * @param component The newly added component.
		 *
		 * @warning Explicitly specialized per component type in `Scene.cpp`. A new component
		 * type **must** have a specialization added there, or the link will fail.
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