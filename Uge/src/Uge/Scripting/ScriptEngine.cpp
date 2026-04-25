#include <ugpch.h>
#include "ScriptEngine.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>
#include <mono/metadata/attrdefs.h>

#include <glm/glm.hpp>
#include <FileWatch.hpp>

#include "ScriptGlue.h"
#include "Uge/Core/Application.h"
#include "Uge/Core/Timer.h"
#include "Uge/Core/Buffer.h"
#include "Uge/Core/FileSystem.h"

namespace Uge
{

	static std::unordered_map<std::string, ScriptFieldType> s_scriptFieldTypeMap =
	{
		{ "System.Single", ScriptFieldType::Float },
		{ "System.Double", ScriptFieldType::Double },

		{ "Uge.Vector2", ScriptFieldType::Vector2 },
		{ "Uge.Vector3", ScriptFieldType::Vector3 },
		{ "Uge.Vector4", ScriptFieldType::Vector4 },

		{ "System.Int", ScriptFieldType::Int },
		{ "System.UInt32", ScriptFieldType::UInt },
		{ "System.Int64", ScriptFieldType::Long },
		{ "System.UInt64", ScriptFieldType::ULong },
		{ "System.Int16", ScriptFieldType::Short },
		{ "System.UInt16", ScriptFieldType::UShort },

		{ "System.Boolean", ScriptFieldType::Bool },
		{ "System.SByte", ScriptFieldType::Byte },
		{ "System.Byte", ScriptFieldType::UByte },
		{ "System.Char", ScriptFieldType::Char },

		{ "System.String", ScriptFieldType::String },

		{ "Uge.Entity", ScriptFieldType::Entity }
	};

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


		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath)
		{

			ScopedBuffer fileData = FileSystem::ReadFileBinary(assemblyPath);

			MonoImageOpenStatus status;
			MonoImage* image = mono_image_open_from_data_full(fileData.As<char>(), fileData.Size(), 1, &status, 0);


			if (status != MONO_IMAGE_OK)
			{
				const char* errorMsg = mono_image_strerror(status);
				UG_CORE_ERROR("Error: {0}", errorMsg);

				return nullptr;
			}

			std::string assemblyPathStr = assemblyPath.string();
			MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPathStr.c_str(), &status, 0);
			mono_image_close(image);

			return assembly;

		}

		ScriptFieldType MonoTypeToScriptFieldType(MonoType* monoType)
		{
			std::string typeName = mono_type_get_name(monoType);

			auto it = s_scriptFieldTypeMap.find(typeName);
			if (it == s_scriptFieldTypeMap.end())
			{
				UG_CORE_ERROR("Typename is {0}", typeName);
				return ScriptFieldType::None;
			}
			return it->second;

			// if (s_scriptFieldTypeMap.find(typeName) == s_scriptFieldTypeMap.end())
			// {
			// 	return ScriptFieldType::None;
			// }
			// 
			// return s_scriptFieldTypeMap.at(typeName);

			

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

		std::filesystem::path CoreAssemblyFilePath;
		std::filesystem::path AppAssemblyFilePath;

		ScriptClass EntityMonoClass;

		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;
		std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances;
		std::unordered_map<UUID, ScriptFieldMap> EntityScriptFields;

		Scope<filewatch::FileWatch<std::string>> AppAssemblyFileWatcher = nullptr;
		bool AppAssemblyReloadPending = false;

		Timer ReloadTimer;


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
		ScriptGlue::RegisterFunctions();

		bool status = LoadAssembly("Resources/Scripts/Uge-ScriptCore.dll");
		if (!status)
		{
			UG_CORE_ERROR("ScriptEngine Could not load Uge-ScriptCore assembly!");
			return;
		}

		status = LoadAppAssembly("SandboxProject/Assets/Scripts/Binaries/Sandbox.dll");
		if (!status)
		{
			UG_CORE_ERROR("ScriptEngine Could not load app assembly!");
			return;
		}
		LoadAssemblyClasses();

		ScriptGlue::RegisterComponents();
		
		s_data->EntityMonoClass = ScriptClass("Uge", "Entity" ,true);

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

		mono_domain_set(mono_get_root_domain(), false);


		mono_domain_unload(s_data->AppDomain);
		s_data->AppDomain = nullptr;
		
		mono_jit_cleanup(s_data->RootDomain);
		s_data->RootDomain = nullptr;
		
	}

	bool ScriptEngine::LoadAssembly(const std::filesystem::path& filePath)
	{

		// Create an App Domain
		s_data->AppDomain = mono_domain_create_appdomain("UgeScriptRuntime", nullptr);
		mono_domain_set(s_data->AppDomain, true);

		s_data->CoreAssemblyFilePath = filePath;
		s_data->CoreAssembly = Utils::LoadMonoAssembly(filePath);

		if (s_data->CoreAssembly == nullptr)
		{
			return false;
		}

		s_data->CoreAssemblyImage = mono_assembly_get_image(s_data->CoreAssembly);

		// Utils::PrintAssemblyTypes(s_data->CoreAssembly);

		return true;


	}

	static void OnAppAssemblyFileSystemEvent(const std::string& path, const filewatch::Event change_type)
	{
		if (!s_data->AppAssemblyReloadPending && change_type == filewatch::Event::modified)
		{
			s_data->AppAssemblyReloadPending = true;
		
			s_data->ReloadTimer = Timer();

			Application::Get().SubmitToMainThreadQueue([]() 
				{ 
					

					s_data->AppAssemblyFileWatcher.reset();
					ScriptEngine::ReloadAssembly(); 

				}
			);

		}

	}

	bool ScriptEngine::LoadAppAssembly(const std::filesystem::path& filePath)
	{

		s_data->AppAssemblyFilePath = filePath;
		s_data->AppAssembly = Utils::LoadMonoAssembly(filePath);

		if (s_data->AppAssembly == nullptr)
		{
			return false;
		}

		s_data->AppAssemblyImage = mono_assembly_get_image(s_data->AppAssembly);

		// Utils::PrintAssemblyTypes(s_data->CoreAssembly);
		

		s_data->AppAssemblyFileWatcher = CreateScope<filewatch::FileWatch<std::string>>( 
			filePath.string(), OnAppAssemblyFileSystemEvent
		);
		s_data->AppAssemblyReloadPending = false;
		
		return true;
		
	}

	void ScriptEngine::ReloadAssembly()
	{
		UG_CORE_WARN("Reloading Took {0}ms", s_data->ReloadTimer.ElapsedMillis());

		mono_domain_set(mono_get_root_domain(), false);

		// mono_domain_free(s_data->AppDomain, false);
		mono_domain_unload(s_data->AppDomain);

		LoadAssembly(s_data->CoreAssemblyFilePath);
		LoadAppAssembly(s_data->AppAssemblyFilePath);

		LoadAssemblyClasses();

		s_data->EntityMonoClass = ScriptClass("Uge", "Entity", true);

		ScriptGlue::RegisterComponents();


	}

	Ref<ScriptClass> ScriptEngine::GetEntityClass(const std::string& name)
	{

		auto it = s_data->EntityClasses.find(name);
		if (it == s_data->EntityClasses.end())
		{
			nullptr;
		}

		return s_data->EntityClasses.at(name);
	}

	std::unordered_map<std::string, Ref<ScriptClass>> ScriptEngine::GetEntityClasses()
	{
		return s_data->EntityClasses;
	}

	ScriptFieldMap& ScriptEngine::GetScriptFieldMap(UUID entityID)
	{
		
		auto it = s_data->EntityScriptFields.find(entityID);
		// UG_CORE_ASSERT(it != s_data->EntityScriptFields.end());

		return s_data->EntityScriptFields[entityID];

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
			const char* className = mono_metadata_string_heap(s_data->AppAssemblyImage, cols[MONO_TYPEDEF_NAME]);
			std::string fullName;
			if (strlen(nameSpace) != 0)
			{

				fullName = fmt::format("{}.{}", nameSpace, className);
			}
			else
			{

				fullName = className;
			}
			UG_CORE_TRACE("{0}", fullName.c_str());

			MonoClass* monoClass = mono_class_from_name(s_data->AppAssemblyImage, nameSpace, className);

			if (monoClass == entityClass)
			{

				continue;
			}

			bool isEntity = mono_class_is_subclass_of(monoClass, entityClass, false);
			if (!isEntity)
			{
				continue;
			}

			Ref<ScriptClass> scriptClass = CreateRef<ScriptClass>(nameSpace, className);

			s_data->EntityClasses[fullName] = scriptClass;
			
			int fieldCount = mono_class_num_fields(monoClass);

			UG_CORE_WARN("{0} has {1} Fields: ", className, fieldCount);
			void* it = nullptr;
			MonoClassField* field;
			while ((field = mono_class_get_fields(monoClass, &it)) != nullptr)
			{
				const char* fieldName = mono_field_get_name(field);
				uint32_t flags = mono_field_get_flags(field);
				UG_CORE_WARN("  {0} flags = {1}", fieldName, flags);
				if (flags & MONO_FIELD_ATTR_PUBLIC)
				{
					MonoType* type = mono_field_get_type(field);
					const char* typeName = mono_type_get_name(type);

					ScriptFieldType fieldType = Utils::MonoTypeToScriptFieldType(type);

					UG_CORE_WARN("   {0} ({1}) is public", fieldName, Utils::ScriptFieldTypeToString(fieldType));

					scriptClass->m_fields[fieldName] = { fieldName, fieldType, field };

				}


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

	bool ScriptInstance::GetFieldValueInternal(const std::string& name, void* buf)
	{
		const auto& fields = m_scriptClass->GetFields();
		auto it = fields.find(name);

		if (it == fields.end())
		{
			return false;
		}

		
		const ScriptField& field = it->second;
		mono_field_get_value(m_instance, field.ClassField, buf);

		return true;
	}

	bool ScriptInstance::SetFieldValueInternal(const std::string& name, const void* val)
	{
		const auto& fields = m_scriptClass->GetFields();
		auto it = fields.find(name);

		if (it == fields.end())
		{
			return false;
		}


		const ScriptField& field = it->second;
		mono_field_set_value(m_instance, field.ClassField, (void*)val);

		return true;
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
			UUID entityID = entity.GetUUID();
			Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(s_data->EntityClasses[sc.ClassName], entity);
			s_data->EntityInstances[entityID] = instance;

			// Copy field values
			if (s_data->EntityScriptFields.find(entityID) != s_data->EntityScriptFields.end())
			{
				const ScriptFieldMap& fieldMap = s_data->EntityScriptFields.at(entityID);
				for (const auto& [name, fieldInstance] : fieldMap)
				{
					instance->SetFieldValueInternal(name, fieldInstance.m_dataBuffer);

				}


			}

			instance->InvokeOnCreate();
		}
	}

	void ScriptEngine::OnUpdateEntity(Entity entity, Timestep ts)
	{

		UUID entityUUID = entity.GetUUID();
		if (s_data->EntityInstances.find(entityUUID) != s_data->EntityInstances.end())
		{
			Ref<ScriptInstance> instance = s_data->EntityInstances[entityUUID];
			instance->InvokeOnUpdate((float)ts);

		}
		else
		{
			UG_CORE_ERROR("Could not find ScriptInstance for entity {0}!", (uint64_t)entityUUID);
		}


	}

	Scene* ScriptEngine::GetSceneContext()
	{

		return s_data->SceneContext;
	}

	Ref<ScriptInstance> ScriptEngine::GetEntityScriptInstance(UUID entityID)
	{

		auto it = s_data->EntityInstances.find(entityID);

		if (it == s_data->EntityInstances.end())
		{
			return nullptr;
		}

		return it->second;

	}

	MonoImage* ScriptEngine::GetCoreAssemblyImage()
	{



		return s_data->CoreAssemblyImage;
	}

	MonoObject* ScriptEngine::GetManagedInstance(UUID entityID)
	{
		
		UG_CORE_ASSERT(s_data->EntityInstances.find(entityID) != s_data->EntityInstances.end());

		return s_data->EntityInstances.at(entityID)->GetManagedMonoObject();
	}

}