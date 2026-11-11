#include <ugpch.h>
#include "ScriptGlue.h"
#include "ScriptEngine.h"

#include "Uge/Core/UUID.h"
#include "Uge/Core/KeyCodes.h"
#include "Uge/Core/Input.h"
		  
#include "Uge/Scene/Scene.h"
#include "Uge/Scene/Entity.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>

#include <glm/glm.hpp>

namespace Uge
{

	static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_entityHasComponentFuncs;

#define UG_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Uge.InternalCalls::" #Name, Name)


	static MonoObject* GetScriptInstance(UUID entityID)
	{

		return ScriptEngine::GetManagedInstance(entityID);

	}

	static bool Entity_HasComponent(UUID entityID, MonoReflectionType* componentType)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		UG_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		UG_CORE_ASSERT(entity);

		MonoType* managedType = mono_reflection_type_get_type(componentType);
		UG_CORE_ASSERT(s_entityHasComponentFuncs.find(managedType) != s_entityHasComponentFuncs.end());
		return s_entityHasComponentFuncs.at(managedType)(entity);
	}

	static uint64_t Entity_FindEntityByName(MonoString* name)
	{

		char* msString = mono_string_to_utf8(name);

		Scene* scene = ScriptEngine::GetSceneContext();
		UG_CORE_ASSERT(scene);
		Entity entity = scene->FindEntityByName(msString);
		mono_free(msString);

		if (!entity)
		{
			return 0;
		}

		return entity.GetUUID();
	}

	static void TransformComponent_GetTranslation(UUID entityID, glm::vec3* outTranslation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		UG_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		UG_CORE_ASSERT(entity);

		*outTranslation = entity.GetComponent<TransformComponent>().Translation;
	}

	static void TransformComponent_SetTranslation(UUID entityID, glm::vec3* translation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		UG_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		UG_CORE_ASSERT(entity);

		entity.GetComponent<TransformComponent>().Translation = *translation;
	}



	static bool Input_IsKeyDown(KeyCode keycode)
	{
		return Input::IsKeyPressed(keycode);
	}

	void ScriptGlue::RegisterFunctions()
	{

		UG_ADD_INTERNAL_CALL(GetScriptInstance);
		UG_ADD_INTERNAL_CALL(Entity_HasComponent);
		UG_ADD_INTERNAL_CALL(Entity_FindEntityByName);

		UG_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		UG_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
		
		
		UG_ADD_INTERNAL_CALL(Input_IsKeyDown);

	}

	// Lets Entity.HasComponent<T>() work for a component that has a C# class in Uge-ScriptCore.
	// Names are explicit: typeid(T).name() is compiler-specific.
	template<typename Component>
	static void RegisterComponent(std::string managedTypeName)
	{
		MonoType* managedType = mono_reflection_type_from_name(managedTypeName.data(), ScriptEngine::GetCoreAssemblyImage());
		if (!managedType)
		{
			UG_CORE_ERROR("Component type {0} is not in Uge-ScriptCore", managedTypeName);
			return;
		}

		s_entityHasComponentFuncs[managedType] = [](Entity entity) { return entity.HasComponent<Component>(); };
	}

	void ScriptGlue::RegisterComponents()
	{
		s_entityHasComponentFuncs.clear();

		// One line per component class in Uge-ScriptCore/Source/Uge/Scene/Components.cs. Most
		// components have no C# counterpart, so they aren't listed.
		RegisterComponent<TransformComponent>("Uge.TransformComponent");
	}

}