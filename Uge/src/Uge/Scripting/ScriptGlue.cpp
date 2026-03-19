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

	static void CppFunction()
	{

		std::cout << "This is written in C++\n";

	}

	static void NativeLog(MonoString* text, int parameter)
	{
		char* msString = mono_string_to_utf8(text);

		std::cout << "Text: " << msString << std::endl;
		std::cout << "Parameter: " << parameter << std::endl;

		mono_free(msString);

	}

	static void NativeLogVector3(glm::vec3* parameter, glm::vec3* outResult)
	{
		UG_CORE_WARN("NativeLogVec3");
		UG_CORE_WARN("Value: X: {0}, Y: {1}, Z: {2}", parameter->x, parameter->y, parameter->z);

		*outResult = glm::cross(*parameter, glm::vec3(parameter->x, parameter->y, -parameter->z));



	}

	static float NativeLogDot(glm::vec3* vec1, glm::vec3* vec2)
	{
		UG_CORE_WARN("NativeLogDot");
		UG_CORE_WARN("Value of vector 1: X: {0}, Y: {1}, Z: {2}", vec1->x, vec1->y, vec1->z);
		UG_CORE_WARN("Value of vector 2: X: {0}, Y: {1}, Z: {2}", vec2->x, vec2->y, vec2->z);

		return glm::dot(*vec1, *vec2);



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
		UG_ADD_INTERNAL_CALL(CppFunction);
		UG_ADD_INTERNAL_CALL(NativeLog);
		UG_ADD_INTERNAL_CALL(NativeLogVector3);
		UG_ADD_INTERNAL_CALL(NativeLogDot);

		UG_ADD_INTERNAL_CALL(Entity_HasComponent);
		UG_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		UG_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
		
		
		UG_ADD_INTERNAL_CALL(Input_IsKeyDown);

	}

	template<typename... Component>
	static void RegisterComponent()
	{
		([]()
			{
				std::string_view typeName = typeid(Component).name();
				size_t pos = typeName.find_last_of(':');
				std::string_view structName = typeName.substr(pos + 1);
				std::string managedTypename = fmt::format("Uge.{}", structName);

				MonoType* managedType = mono_reflection_type_from_name(managedTypename.data(), ScriptEngine::GetCoreAssemblyImage());
				if (!managedType)
				{
					UG_CORE_ERROR("Could not find component type {}", managedTypename);
					return;
				}
				UG_CORE_TRACE("Found component type {}", managedTypename);
				s_entityHasComponentFuncs[managedType] = [](Entity entity) { return entity.HasComponent<Component>(); };
			}(), ...);
	}

	template<typename... Component>
	static void RegisterComponent(ComponentGroup<Component...>)
	{
		RegisterComponent<Component...>();
	}

	void ScriptGlue::RegisterComponents()
	{
		RegisterComponent(AllComponents{});
	}

}