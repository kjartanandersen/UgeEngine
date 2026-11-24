/**
 * @file Entity.h
 * @brief A handle to an entity in a scene.
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
	 * @brief A handle to one entity in a scene, with component access. Pass it by value.
	 * @ingroup group_scene
	 *
	 * The handle doesn't own the entity; IsValid() tells whether the entity still exists.
	 * It is only meaningful in its own scene, so save a Uge::UUID instead.
	 */
	class Entity
	{

	public:
		/** @brief An invalid handle. */
		Entity() = default;
		/**
		 * @brief Wraps an EnTT entity.
		 * @param handle The EnTT entity.
		 * @param scene Its scene; not owned.
		 */
		Entity(entt::entity handle, Scene* scene);
		/**
		 * @brief Copies the handle, not the entity.
		 * @param other Handle to copy.
		 */
		Entity(const Entity& other) = default;

		/**
		 * @brief Adds a component.
		 * @tparam T Component type.
		 * @tparam Args Constructor argument types.
		 * @param args Constructor arguments.
		 * @return The component. Adding more components can invalidate the reference.
		 *
		 * Asserts if the entity already has a `T`.
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
		 * @brief Adds a component, replacing one of the same type.
		 * @tparam T Component type.
		 * @tparam Args Constructor argument types.
		 * @param args Constructor arguments.
		 * @return The component.
		 */
		template<typename T, typename... Args>
		T& AddOrReplaceComponent(Args&&... args)
		{
			T& component = m_scene->m_registry.emplace_or_replace<T>(m_entityHandle, std::forward<Args>(args)...);
			m_scene->OnComponentAdded<T>(*this, component);
			return component;
		}

		/**
		 * @brief A component. Asserts if the entity has no `T`.
		 * @tparam T Component type.
		 * @return The component.
		 */
		template<typename T>
		T& GetComponent()
		{
			bool hasC = HasComponent<T>();
			UG_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");

			return m_scene->m_registry.get<T>(m_entityHandle);
		}

		/**
		 * @brief Whether the entity has a component.
		 * @tparam T Component type.
		 * @return `true` if it has a `T`.
		 */
		template<typename T>
		bool HasComponent() { return m_scene->m_registry.any_of<T>(m_entityHandle); }

		/**
		 * @brief Removes a component. Asserts if the entity has no `T`.
		 * @tparam T Component type.
		 */
		template<typename T>
		void RemoveComponent()
		{
			UG_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");

			m_scene->m_registry.remove<T>(m_entityHandle);
		}

		/**
		 * @brief Whether the entity exists.
		 * @return `false` for a default handle, or once the entity is destroyed.
		 */
		bool IsValid() const
		{
			return m_scene && m_entityHandle != entt::null && m_scene->m_registry.valid(m_entityHandle);
		}

		/**
		 * @brief Whether the entity exists.
		 * @return IsValid().
		 */
		operator bool() const { return IsValid(); }
		/**
		 * @brief The entity index.
		 * @return The `entt::entity` value, used as the picking ID.
		 */
		operator uint32_t() const { return (uint32_t)m_entityHandle; }
		/**
		 * @brief The EnTT entity.
		 * @return The `entt::entity`.
		 */
		operator entt::entity() const { return m_entityHandle; }

		/**
		 * @brief The entity's scene.
		 * @return The scene, or null for a default handle.
		 */
		Scene* GetScene() const { return m_scene; }

		/**
		 * @brief The entity's ID.
		 * @return The UUID from its Uge::IDComponent.
		 */
		UUID GetUUID() { return GetComponent<IDComponent>().ID; }
		/**
		 * @brief The entity's name.
		 * @return The tag from its Uge::TagComponent.
		 */
		const std::string& GetName() { return GetComponent<TagComponent>().Tag; }

		/**
		 * @brief Equality.
		 * @param other Handle to compare.
		 * @return `true` for the same entity in the same scene.
		 */
		bool operator==(const Entity& other) const
		{
			return m_entityHandle == other.m_entityHandle && m_scene == other.m_scene;
		}

		/**
		 * @brief Inequality.
		 * @param other Handle to compare.
		 * @return `true` for a different entity or scene.
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
