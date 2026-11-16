/**
 * @file ScriptableEntity.h
 * @brief Base class for native C++ entity behaviours.
 * @ingroup group_scene
 */

#pragma once

#include "Entity.h"
#include <Uge/Core/Timestep.h>

namespace Uge
{

	
	/**
	 * @brief Base class of a C++ script on an entity, run by Uge::NativeScriptComponent.
	 * @ingroup group_scene
	 *
	 * The entity is set before OnCreate() runs.
	 */
	class ScriptableEntity
	{

	public:
		/** @brief Virtual destructor. */
		virtual ~ScriptableEntity() {}

		/**
		 * @brief A component of the entity. Asserts if it has no `T`.
		 * @tparam T Component type.
		 * @return The component.
		 */
		template<typename T>
		T& GetComponent()
		{
			return m_entity.GetComponent<T>();
		}

	protected:
		/** @brief Called once, when the script is created. */
		virtual void OnCreate() {}
		/** @brief Called once, before the script is destroyed. */
		virtual void OnDestroy() {}
		/**
		 * @brief Called every frame in play mode.
		 * @param ts Frame time.
		 */
		virtual void OnUpdate(Timestep ts) {}


	private:
		Entity m_entity;

		friend class Scene;

	};

}