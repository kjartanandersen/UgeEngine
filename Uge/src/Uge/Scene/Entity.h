/**
 * @file Entity.h
 * @brief Lightweight handle pairing an `entt::entity` with its owning scene.
 * @ingroup group_scene
 */

#pragma once

#include "Scene.h"
#include "Uge/Core/UUID.h"
#include <Uge/Scene/Components.h>


#include <entt.hpp>

namespace Uge
{

	/**
	 * @brief A handle to one entity in a scene, with typed component access.
	 * @ingroup group_scene
	 *
	 * Two words wide — an `entt::entity` and a scene pointer — so pass it by value. It is
	 * a handle, not an owner: destroying the entity through Uge::Scene::DestroyEntity
	 * leaves any copies dangling, which IsValid() detects.
	 *
	 * @code
	 * Entity camera = scene->CreateEntity("Camera");
	 * camera.AddComponent<CameraComponent>();
	 * auto& transform = camera.GetComponent<TransformComponent>();
	 * transform.Translation = { 0.0f, 0.0f, 10.0f };
	 * @endcode
	 *
	 * @warning Entity handles are not stable across serialization; the `entt::entity` value
	 * is only meaningful within one registry. Store a Uge::UUID for persistent references
	 * and resolve it with Uge::Scene::GetEntityByUUID.
	 */
	class Entity
	{

	public:
		/** @brief Constructs an invalid handle referring to no entity. */
		Entity() = default;
		/**
		 * @brief Wraps a registry handle together with its scene.
		 * @param handle Underlying EnTT entity.
		 * @param scene Scene owning the entity; not itself owned.
		 */
		Entity(entt::entity handle, Scene* scene);
		/**
		 * @brief Copy constructor; copies the handle, not the entity.
		 * @param other Handle to copy.
		 */
		Entity(const Entity& other) = default;

		/**
		 * @brief Attaches a new component to the entity.
		 * @tparam T Component type to add.
		 * @tparam Args Constructor argument types, deduced.
		 * @param args Arguments forwarded to the component's constructor.
		 * @return Reference to the newly added component.
		 *
		 * Runs Uge::Scene::OnComponentAdded for `T` afterwards.
		 *
		 * @warning Asserts if the entity already has a `T`. Use AddOrReplaceComponent() when
		 * that is a possibility.
		 * @note The returned reference can be invalidated by later component additions; do not
		 * hold on to it across frames.
		 */
		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			UG_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
			T& component = m_scene->m_registry.emplace<T>(m_entityHandle, std::forward<Args>(args)...);
			m_scene->OnComponentAdded<T>(*this, component);
			return component;
		}

		/**
		 * @brief Attaches a component, replacing any existing one of the same type.
		 * @tparam T Component type to add or replace.
		 * @tparam Args Constructor argument types, deduced.
		 * @param args Arguments forwarded to the component's constructor.
		 * @return Reference to the component.
		 */
		template<typename T, typename... Args>
		T& AddOrReplaceComponent(Args&&... args)
		{
			T& component = m_scene->m_registry.emplace_or_replace<T>(m_entityHandle, std::forward<Args>(args)...);
			m_scene->OnComponentAdded<T>(*this, component);
			return component;
		}

		/**
		 * @brief Retrieves a component.
		 * @tparam T Component type to fetch.
		 * @return Mutable reference to the component.
		 * @warning Asserts if the entity does not have a `T`; test with HasComponent() first.
		 */
		template<typename T>
		T& GetComponent()
		{
			bool hasC = HasComponent<T>();
			UG_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");

			return m_scene->m_registry.get<T>(m_entityHandle);
		}

		/**
		 * @brief Tests for the presence of a component.
		 * @tparam T Component type to test for.
		 * @return `true` if the entity has a `T`.
		 */
		template<typename T>
		bool HasComponent() { return m_scene->m_registry.any_of<T>(m_entityHandle); }

		/**
		 * @brief Detaches a component.
		 * @tparam T Component type to remove.
		 * @warning Asserts if the entity does not have a `T`.
		 */
		template<typename T>
		void RemoveComponent()
		{
			UG_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");

			m_scene->m_registry.remove<T>(m_entityHandle);
		}

		/**
		 * @brief Whether this handle still refers to a live entity.
		 * @return `false` for a default-constructed handle, or after the entity was destroyed.
		 */
		bool IsValid() const
		{
			return m_scene && m_entityHandle != entt::null && m_scene->m_registry.valid(m_entityHandle);
		}

		/**
		 * @brief Implicit validity test, so a handle can be used in a condition.
		 * @return `true` if the handle is valid.
		 */
		operator bool() const { return IsValid(); }
		/**
		 * @brief Conversion to the raw entity index.
		 * @return The `entt::entity` value, used as the picking ID in draw calls.
		 */
		operator uint32_t() const { return (uint32_t)m_entityHandle; }
		/**
		 * @brief Conversion to the underlying EnTT handle.
		 * @return The wrapped `entt::entity`.
		 */
		operator entt::entity() const { return m_entityHandle; }

		/**
		 * @brief The entity's persistent identifier.
		 * @return The UUID from its Uge::IDComponent.
		 */
		UUID GetUUID() { return GetComponent<IDComponent>().ID; }
		/**
		 * @brief The entity's display name.
		 * @return Const reference to the tag from its Uge::TagComponent.
		 */
		const std::string& GetName() { return GetComponent<TagComponent>().Tag; }

		/**
		 * @brief Compares two handles.
		 * @param other Handle to compare against.
		 * @return `true` if both refer to the same entity in the same scene.
		 */
		bool operator==(const Entity& other) const 
		{ 
			return m_entityHandle == other.m_entityHandle && m_scene == other.m_scene; 
		}

		/**
		 * @brief Compares two handles for inequality.
		 * @param other Handle to compare against.
		 * @return `true` if they refer to different entities or different scenes.
		 */
		bool operator!=(const Entity& other) const
		{
			return !(*this == other);
		}


	private:
		entt::entity m_entityHandle{ entt::null };
		Scene* m_scene = nullptr;

	};

	


}
