#pragma once

#include "Entity.h"
#include <Uge/Core/Timestep.h>

namespace Uge
{

	
	class ScriptableEntity
	{

	public:
		virtual ~ScriptableEntity() {}

		template<typename T>
		T& GetComponent()
		{
			return m_entity.GetComponent<T>();
		}

	protected:
		virtual void OnCreate() {}
		virtual void OnDestroy() {}
		virtual void OnUpdate(Timestep ts) {}


	private:
		Entity m_entity;

		friend class Scene;

	};

}