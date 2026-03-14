#include <ugpch.h>
#include "ScriptEngine.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>

namespace Uge
{

	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
	};

	static ScriptEngineData* s_data = nullptr;

	void ScriptEngine::Init()
	{
		s_data = new ScriptEngineData();

		InitMono();

	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMono();
		delete s_data;
	}

	char* ReadBytes(const std::string& filepath, uint32_t* outSize)
	{

		std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

		if (!stream)
		{
			// Failed to open the file
			return nullptr;
		}

		std::streampos end = stream.tellg();
		stream.seekg(0, std::ios::beg);
		uint32_t size = end - stream.tellg();
		if (size == 0)
		{
			// File is empty
			return nullptr;
		}

		char* buffer = new char[size];
		stream.read((char*)buffer ,size);
		stream.close();

		*outSize = size;
		return buffer;


	}

	MonoAssembly* LoadCSharpAssembly(const std::string& assemblyPath)
	{

		uint32_t fileSize = 0;
		char* fileData = ReadBytes(assemblyPath, &fileSize);

		MonoImageOpenStatus status;
		MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

		if (status != MONO_IMAGE_OK)
		{
			const char* errorMsg = mono_image_strerror(status);
			UG_CORE_ERROR("Error: {0}", errorMsg);

			return nullptr;
		}

		MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, 0);
		mono_image_close(image);

		delete[] fileData;

		return assembly;

	}

	void PrintAssemblyTypes(MonoAssembly* assembly)
	{

		MonoImage* image = mono_assembly_get_image(assembly);
		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

		for (int32_t i = 0; i < numTypes; i++)
		{

			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

			UG_CORE_TRACE("{0}.{1}", nameSpace, name);

		}
		printf("\n");

	}

	void ScriptEngine::InitMono()
	{
	
		mono_set_assemblies_path("mono/lib");

		MonoDomain* rootDomain = mono_jit_init("UgeJITRuntime");
		UG_CORE_ASSERT(rootDomain);
		

		// Store the root domain pointer
		s_data->RootDomain = rootDomain;

		// Create an App Domain
		s_data->AppDomain = mono_domain_create_appdomain("UgeScriptRuntime", nullptr);
		mono_domain_set(s_data->AppDomain, true);

		s_data->CoreAssembly = LoadCSharpAssembly("Resources/Scripts/Uge-ScriptCore.dll");
		PrintAssemblyTypes(s_data->CoreAssembly);

		MonoImage* AssemblyImg = mono_assembly_get_image(s_data->CoreAssembly);
		MonoClass* monoClass = mono_class_from_name(AssemblyImg, "Uge", "Main");

		// Create object and call constructor
		MonoObject* instance = mono_object_new(s_data->AppDomain, monoClass);
		mono_runtime_object_init(instance);

		// Call function
		{
			MonoMethod* printMessageMethod = mono_class_get_method_from_name(monoClass, "PrintMessage", 0);
			mono_runtime_invoke(printMessageMethod, instance, nullptr, nullptr);
		}

		// Call function with parameter
		{
			MonoMethod* printIntMethod = mono_class_get_method_from_name(monoClass, "PrintInt", 1);

			int value = 5;
			void* param = &value;

			mono_runtime_invoke(printIntMethod, instance, &param, nullptr);
		
		}

		// Call ints function with parameter
		{
		
			MonoMethod* printIntsMethod = mono_class_get_method_from_name(monoClass, "PrintInts", 2);

			int value1 = 5;
			int value2 = 6;
			void* params[2] =
			{
				&value1,
				&value2
			};

			mono_runtime_invoke(printIntsMethod, instance, params, nullptr);

		}


		// Call string message method
		{
		
			MonoMethod* printCustomMessageMethod = mono_class_get_method_from_name(monoClass, "PrintCustomMessage", 1);

			MonoString* str = mono_string_new(s_data->AppDomain, "Hello!!");
			void* param[1] =
			{
				str
			};

			mono_runtime_invoke(printCustomMessageMethod, instance, param, nullptr);

		
		}
	}

	void ScriptEngine::ShutdownMono()
	{
		
		// Might need fixing

		// mono_domain_unload(s_data->AppDomain);
		s_data->AppDomain = nullptr;
		
		// mono_jit_cleanup(s_data->RootDomain);
		s_data->RootDomain = nullptr;
		


	}


	

}