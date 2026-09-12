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
	 * @brief Base class for a behaviour written in C++ and bound to an entity.
	 * @ingroup group_scene
	 *
	 * The native counterpart to C# scripting: subclass it, override the lifecycle hooks,
	 * and bind it through an entity's Uge::NativeScriptComponent. Uge::Scene sets the
	 * entity reference before OnCreate() runs, so GetComponent() is usable from there on.
	 *
	 * @code
	 * class CameraController : public ScriptableEntity
	 * {
	 *     void OnUpdate(Timestep ts) override
	 *     {
	 *         auto& transform = GetComponent<TransformComponent>();
	 *         if (Input::IsKeyPressed(UG_KEY_A))
	 *             transform.Translation.x -= m_speed * ts;
	 *     }
	 *     float m_speed = 5.0f;
	 * };
	 * entity.AddComponent<NativeScriptComponent>().Bind<CameraController>();
	 * @endcode
	 *
	 * @see Uge::ScriptEngine for the C# alternative.
	 */
	class ScriptableEntity
	{

	public:
		/** @brief Virtual destructor; instances are owned by the component that bound them. */
		virtual ~ScriptableEntity() {}

		/**
		 * @brief Retrieves a component from the entity this script is attached to.
		 * @tparam T Component type to fetch.
		 * @return Mutable reference to the component.
		 * @warning Asserts if the entity does not have a `T`.
		 */
		template<typename T>
		T& GetComponent()
		{
			return m_entity.GetComponent<T>();
		}

	protected:
		/** @brief Called once when the script is instantiated, at runtime start. */
		virtual void OnCreate() {}
		/** @brief Called once before the script is destroyed. */
		virtual void OnDestroy() {}
		/**
		 * @brief Called once per frame while the scene is running.
		 * @param ts Frame delta time.
		 */
		virtual void OnUpdate(Timestep ts) {}


	private:
		Entity m_entity;

		friend class Scene;

	};

}