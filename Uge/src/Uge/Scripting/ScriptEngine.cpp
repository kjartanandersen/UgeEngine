#include <ugpch.h>
#include "ScriptEngine.h"

#include "ScriptGlue.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>

#include <glm/glm.hpp>

namespace Uge
{


	namespace Utils
	{
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

		// TODO: Move to a Utils or filesystem file, or class
		static char* ReadBytes(const std::filesystem::path& filepath, uint32_t* outSize)
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
			stream.read((char*)buffer, size);
			stream.close();

			*outSize = size;
			return buffer;


		}

		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath)
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

			std::string assemblyPathStr = assemblyPath.string();
			MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPathStr.c_str(), &status, 0);
			mono_image_close(image);

			delete[] fileData;

			return assembly;

		}

	}

	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage = nullptr;

		MonoAssembly* AppAssembly = nullptr;
		MonoImage* AppAssemblyImage = nullptr;

		ScriptClass EntityMonoClass;

		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;
		std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances;


		// Runtime
		Scene* SceneContext = nullptr;
	};

	static ScriptEngineData* s_data = nullptr;

	ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore)
		: m_classNamspace(classNamespace), m_className(className)
	{

		m_monoClass = mono_class_from_name(isCore ? s_data->CoreAssemblyImage : s_data->AppAssemblyImage, 
			classNamespace.c_str(), className.c_str());

	}

	MonoObject* ScriptClass::Instantiate()
	{
		return ScriptEngine::InstantiateClass(m_monoClass);
	}

	MonoMethod* ScriptClass::GetMethod(const std::string& name, int parameterCount)
	{

		return mono_class_get_method_from_name(m_monoClass, name.c_str(), parameterCount);


	}

	MonoObject* ScriptClass::InvokeMethod(MonoObject* instance, MonoMethod* method, void** params)
	{

		return mono_runtime_invoke(method, instance, params, nullptr);

	}
	

	void ScriptEngine::Init()
	{
		s_data = new ScriptEngineData();

		InitMono();

		LoadAssembly("Resources/Scripts/Uge-ScriptCore.dll");
		LoadAppAssembly("SandboxProject/Assets/Scripts/Binaries/Sandbox.dll");
		LoadAssemblyClasses();

		ScriptGlue::RegisterComponents();
		ScriptGlue::RegisterFunctions();
		
		s_data->EntityMonoClass = ScriptClass("Uge", "Entity" ,true);


#if 0
		// Retrieve and instantiate class (w. constructor)
		
		MonoObject* instance = s_data->EntityMonoClass.Instantiate();
		// Call function
		{
			MonoMethod* printMessageMethod = s_data->EntityMonoClass.GetMethod("PrintMessage", 0);
			
			s_data->EntityMonoClass.InvokeMethod(instance, printMessageMethod, nullptr);
		}

		// Call function with parameter
		{
			MonoMethod* printIntMethod = s_data->EntityMonoClass.GetMethod("PrintInt", 1);

			int value = 5;
			void* param = &value;

			s_data->EntityMonoClass.InvokeMethod(instance, printIntMethod, &param);


		}

		// Call ints function with parameter
		{
			MonoMethod* printIntsMethod = s_data->EntityMonoClass.GetMethod("PrintInts", 2);

			int value1 = 5;
			int value2 = 6;
			void* params[2] =
			{
				&value1,
				&value2
			};

			s_data->EntityMonoClass.InvokeMethod(instance, printIntsMethod, params);


		}


		// Call string message method
		{
			MonoMethod* printCustomMessageMethod = s_data->EntityMonoClass.GetMethod("PrintCustomMessage", 1);

			MonoString* str = mono_string_new(s_data->AppDomain, "Hello!!");
			void* param[1] =
			{
				str
			};

			s_data->EntityMonoClass.InvokeMethod(instance, printCustomMessageMethod, param);



		}

		// UG_CORE_ASSERT(false);
#endif

	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMono();
		delete s_data;
	}



	void ScriptEngine::InitMono()
	{
	
		mono_set_assemblies_path("mono/lib");

		MonoDomain* rootDomain = mono_jit_init("UgeJITRuntime");
		UG_CORE_ASSERT(rootDomain);
		

		// Store the root domain pointer
		s_data->RootDomain = rootDomain;
	}

	void ScriptEngine::ShutdownMono()
	{
		
		// Might need fixing

		// mono_domain_unload(s_data->AppDomain);
		s_data->AppDomain = nullptr;
		
		// mono_jit_cleanup(s_data->RootDomain);
		s_data->RootDomain = nullptr;
		
	}

	void ScriptEngine::LoadAssembly(const std::filesystem::path& filePath)
	{

		// Create an App Domain
		s_data->AppDomain = mono_domain_create_appdomain("UgeScriptRuntime", nullptr);
		mono_domain_set(s_data->AppDomain, true);

		s_data->CoreAssembly = Utils::LoadMonoAssembly(filePath);
		s_data->CoreAssemblyImage = mono_assembly_get_image(s_data->CoreAssembly);

		// Utils::PrintAssemblyTypes(s_data->CoreAssembly);

	}

	void ScriptEngine::LoadAppAssembly(const std::filesystem::path& filePath)
	{

		s_data->AppAssembly = Utils::LoadMonoAssembly(filePath);
		s_data->AppAssemblyImage = mono_assembly_get_image(s_data->AppAssembly);

		// Utils::PrintAssemblyTypes(s_data->CoreAssembly);
		
	}

	std::unordered_map<std::string, Ref<ScriptClass>> ScriptEngine::GetEntityClasses()
	{
		return s_data->EntityClasses;
	}

	bool ScriptEngine::EntityClassExists(const std::string& fullClassName)
	{

		return s_data->EntityClasses.find(fullClassName) != s_data->EntityClasses.end();
	}

	MonoObject* ScriptEngine::InstantiateClass(MonoClass* monoClass)
	{

		// Create object and call constructor
		MonoObject* instance = mono_object_new(s_data->AppDomain, monoClass);
		mono_runtime_object_init(instance);

		return instance;

	}

	void ScriptEngine::LoadAssemblyClasses()
	{

		s_data->EntityClasses.clear();

		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(s_data->AppAssemblyImage, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
		MonoClass* entityClass = mono_class_from_name(s_data->CoreAssemblyImage, "Uge", "Entity");

		printf("\n");
		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(s_data->AppAssemblyImage, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(s_data->AppAssemblyImage, cols[MONO_TYPEDEF_NAME]);
			std::string fullName;
			if (strlen(nameSpace) != 0)
			{

				fullName = fmt::format("{}.{}", nameSpace, name);
			}
			else
			{

				fullName = name;
			}
			UG_CORE_TRACE("{0}", fullName.c_str());

			MonoClass* monoClass = mono_class_from_name(s_data->AppAssemblyImage, nameSpace, name);

			if (monoClass == entityClass)
			{

				continue;
			}

			bool isEntity = mono_class_is_subclass_of(monoClass, entityClass, false);
			if (isEntity)
			{
				s_data->EntityClasses[fullName] = CreateRef<ScriptClass>(nameSpace, name);
			}
		}
		printf("\n");

	}

	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity)
		: m_scriptClass(scriptClass)
	{

		m_instance = scriptClass->Instantiate();

		m_constructor = s_data->EntityMonoClass.GetMethod(".ctor", 1);
		m_onCreateMethod = scriptClass->GetMethod("OnCreate", 0);
		m_onUpdateMethod = scriptClass->GetMethod("OnUpdate", 1);

		// Call Entity constructor
		{
			UUID entityID = entity.GetUUID();
			void* param = &entityID;
			m_scriptClass->InvokeMethod(m_instance, m_constructor, &param);
		}
	}

	void ScriptInstance::InvokeOnCreate()
	{

		m_scriptClass->InvokeMethod(m_instance, m_onCreateMethod);
	}

	void ScriptInstance::InvokeOnUpdate(float ts)
	{

		void* param = &ts;

		m_scriptClass->InvokeMethod(m_instance, m_onUpdateMethod, &param);
	}

	void ScriptEngine::OnRuntimeStop()
	{

		s_data->SceneContext = nullptr;

		s_data->EntityInstances.clear();

	}

	void ScriptEngine::OnRuntimeStart(Scene* scene)
	{

		s_data->SceneContext = scene;
	}

	void ScriptEngine::OnCreateEntity(Entity entity)
	{

		const auto& sc = entity.GetComponent<ScriptComponent>();
		if (ScriptEngine::EntityClassExists(sc.ClassName))
		{
			Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(s_data->EntityClasses[sc.ClassName], entity);
			s_data->EntityInstances[entity.GetUUID()] = instance;
			instance->InvokeOnCreate();
		}
	}

	void ScriptEngine::OnUpdateEntity(Entity entity, Timestep ts)
	{

		UUID entityUUID = entity.GetUUID();
		UG_CORE_ASSERT(s_data->EntityInstances.find(entityUUID) != s_data->EntityInstances.end());

		Ref<ScriptInstance> instance = s_data->EntityInstances[entityUUID];
		instance->InvokeOnUpdate((float)ts);

	}

	Scene* ScriptEngine::GetSceneContext()
	{

		return s_data->SceneContext;
	}

	MonoImage* ScriptEngine::GetCoreAssemblyImage()
	{



		return s_data->CoreAssemblyImage;
	}

}