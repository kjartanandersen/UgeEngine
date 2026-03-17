#include <ugpch.h>
#include "ScriptGlue.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>

#include <glm/glm.hpp>

namespace Uge
{

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

	void ScriptGlue::RegisterFunctions()
	{
		UG_ADD_INTERNAL_CALL(CppFunction);
		UG_ADD_INTERNAL_CALL(NativeLog);
		UG_ADD_INTERNAL_CALL(NativeLogVector3);
		UG_ADD_INTERNAL_CALL(NativeLogDot);

	}

}