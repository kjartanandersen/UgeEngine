#include <ugpch.h>
#include "ScriptEngine.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/mono-gc.h>

#include <glm/glm.hpp>
#include <FileWatch.hpp>

#include "ScriptGlue.h"
#include "Uge/Core/Application.h"
#include "Uge/Core/Timer.h"
#include "Uge/Core/Buffer.h"
#include "Uge/Core/FileSystem.h"
#include "Uge/Project/Project.h"

namespace Uge
{

	// Relative to the working directory, like every engine resource; for the editor that is
	// UgeEditor/.
	static constexpr const char* s_monoAssembliesPath = "mono/lib";
	static constexpr const char* s_coreAssemblyPath = "Resources/Scripts/Uge-ScriptCore.dll";

	static std::unordered_map<std::string, ScriptFieldType> s_scriptFieldTypeMap =
	{
		{ "System.Single", ScriptFieldType::Float },
		{ "System.Double", ScriptFieldType::Double },

		{ "Uge.Vector2", ScriptFieldType::Vector2 },
		{ "Uge.Vector3", ScriptFieldType::Vector3 },
		{ "Uge.Vector4", ScriptFieldType::Vector4 },

		{ "System.Int32", ScriptFieldType::Int },
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
		/**
		 * @brief Logs every type in an assembly.
		 * @param assembly The assembly.
		 */
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

		ScriptFieldType ScriptFieldTypeFromManagedName(const std::string& typeName)
		{
			auto it = s_scriptFieldTypeMap.find(typeName);
			if (it == s_scriptFieldTypeMap.end())
			{
				UG_CORE_WARN("Script fields of type {0} aren't supported: scripts can use them, but the editor can't set or save their values", typeName);
				return ScriptFieldType::None;
			}
			return it->second;
		}

		/**
		 * @brief The field type of a managed type.
		 * @param monoType The managed type.
		 * @return The type, or `None` if unsupported.
		 */
		ScriptFieldType MonoTypeToScriptFieldType(MonoType* monoType)
		{
			// mono_type_get_name allocates the string; the caller frees it.
			char* typeName = mono_type_get_name(monoType);
			ScriptFieldType type = ScriptFieldTypeFromManagedName(typeName);
			mono_free(typeName);
			return type;
		}

	}

	/**
	 * @brief The script engine's Mono state.
	 */
	struct ScriptEngineData
	{

		MonoDomain* RootDomain = nullptr; ///< Created once at startup.
		MonoDomain* AppDomain = nullptr; ///< Holds the assemblies; recreated on reload.

		MonoAssembly* CoreAssembly = nullptr; ///< `Uge-ScriptCore`.
		MonoImage* CoreAssemblyImage = nullptr; ///< #CoreAssembly's image.

		MonoAssembly* AppAssembly = nullptr; ///< The project's script assembly.
		MonoImage* AppAssemblyImage = nullptr; ///< #AppAssembly's image.

		std::filesystem::path CoreAssemblyFilePath; ///< Where #CoreAssembly was loaded from.
		std::filesystem::path AppAssemblyFilePath; ///< Where #AppAssembly was loaded from; watched for changes.

		ScriptClass EntityMonoClass; ///< The `Entity` base class.

		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses; ///< Script classes, by full name.
		std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances; ///< Script objects, by entity UUID.
		std::unordered_map<UUID, ScriptFieldMap> EntityScriptFields; ///< Stored field values; kept across reloads.

		Scope<filewatch::FileWatch<std::string>> AppAssemblyFileWatcher = nullptr; ///< Reloads the script assembly when it is rebuilt.
		bool AppAssemblyReloadPending = false; ///< Set by the watcher; the reload runs on the main thread.

		Timer ReloadTimer; ///< Times the reload.


		Scene* SceneContext = nullptr; ///< Scene running scripts; null outside play mode.
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

		// mono_jit_init ends the process without a log line if it can't find mscorlib, so
		// check first and run without scripts.
		if (!std::filesystem::exists(s_monoAssembliesPath) || !std::filesystem::exists(s_coreAssemblyPath))
		{
			UG_CORE_ERROR("Scripting disabled: '{0}' or '{1}' not found in the working directory '{2}'. Run the editor from UgeEditor/.",
				s_monoAssembliesPath, s_coreAssemblyPath, std::filesystem::current_path().string());
			return;
		}

		InitMono();
		ScriptGlue::RegisterFunctions();
	}

	void ScriptEngine::LoadProject()
	{
		// No root domain means Init() disabled scripting.
		if (!s_data || !s_data->RootDomain)
		{
			return;
		}

		if (!LoadAssembly(s_coreAssemblyPath))
		{
			UG_CORE_ERROR("ScriptEngine Could not load Uge-ScriptCore assembly!");
			return;
		}

		const std::filesystem::path scriptModulePath =
			Project::GetAssetDirectory() / Project::GetActive()->GetConfig().ScriptModulePath;
		if (!LoadAppAssembly(scriptModulePath))
		{
			UG_CORE_ERROR("ScriptEngine Could not load app assembly!");
			return;
		}

		LoadAssemblyClasses();
		ScriptGlue::RegisterComponents();
		s_data->EntityMonoClass = ScriptClass("Uge", "Entity", true);
	}

	void ScriptEngine::UnloadProject()
	{
		if (!s_data)
		{
			return;
		}

		// Destroying the watcher joins its threads, so no new reload gets queued.
		s_data->AppAssemblyFileWatcher.reset();
		s_data->AppAssemblyReloadPending = false;

		// GC handles belong to the app domain, so the instances go before it does.
		s_data->EntityInstances.clear();
		s_data->EntityClasses.clear();
		s_data->EntityScriptFields.clear();
		s_data->EntityMonoClass = ScriptClass();
		s_data->SceneContext = nullptr;

		if (s_data->AppDomain)
		{
			mono_domain_set(mono_get_root_domain(), false);
			mono_domain_unload(s_data->AppDomain);
			s_data->AppDomain = nullptr;
		}

		s_data->CoreAssembly = nullptr;
		s_data->CoreAssemblyImage = nullptr;
		s_data->AppAssembly = nullptr;
		s_data->AppAssemblyImage = nullptr;
	}

	void ScriptEngine::Shutdown()
	{
		if (!s_data)
		{
			return;
		}

		UnloadProject();

		ShutdownMono();
		delete s_data;
		s_data = nullptr;
	}



	void ScriptEngine::InitMono()
	{
	
		mono_set_assemblies_path(s_monoAssembliesPath);

		MonoDomain* rootDomain = mono_jit_init("UgeJITRuntime");
		UG_CORE_ASSERT(rootDomain);


		s_data->RootDomain = rootDomain;
	}

	void ScriptEngine::ShutdownMono()
	{
		// Null when Init didn't start Mono, for lack of the runtime files.
		if (!s_data->RootDomain)
		{
			return;
		}

		mono_domain_set(mono_get_root_domain(), false);

		if (s_data->AppDomain)
		{
			mono_domain_unload(s_data->AppDomain);
			s_data->AppDomain = nullptr;
		}

		mono_jit_cleanup(s_data->RootDomain);
		s_data->RootDomain = nullptr;
	}

	bool ScriptEngine::LoadAssembly(const std::filesystem::path& filePath)
	{

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
		
		UG_CORE_INFO("Loaded App Assembly At: {0}", filePath.string());
		return true;
		
	}

	void ScriptEngine::ReloadAssembly()
	{
		if (!s_data || !s_data->RootDomain || !s_data->AppDomain)
		{
			UG_CORE_WARN("Scripting isn't running; nothing to reload.");
			return;
		}

		// GC handles belong to the domain being unloaded, so they are freed first. Scripts
		// running in Play stop until the next Play.
		if (!s_data->EntityInstances.empty())
		{
			UG_CORE_WARN("Script assembly reloaded during Play; scripts stop until the next Play.");
			s_data->EntityInstances.clear();
		}

		UG_CORE_INFO("Reloading Took {0}ms", s_data->ReloadTimer.ElapsedMillis());

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
		// Null before Init() and after Shutdown().
		if (!s_data)
		{
			UG_CORE_WARN("Script engine data is null");
			return nullptr;
		}

		auto it = s_data->EntityClasses.find(name);
		if (it == s_data->EntityClasses.end())
		{
			UG_CORE_WARN("Entity class not found: {0}", name);
			return nullptr;
		}

		return it->second;
	}

	std::unordered_map<std::string, Ref<ScriptClass>> ScriptEngine::GetEntityClasses()
	{
		return s_data->EntityClasses;
	}

	ScriptFieldMap* ScriptEngine::GetScriptFieldMap(UUID entityID)
	{
		// Null before Init() and after Shutdown().
		if (!s_data)
		{
			return nullptr;
		}

		return &s_data->EntityScriptFields[entityID];
	}

	bool ScriptEngine::EntityClassExists(const std::string& fullClassName)
	{

		return s_data->EntityClasses.find(fullClassName) != s_data->EntityClasses.end();
	}

	MonoObject* ScriptEngine::InstantiateClass(MonoClass* monoClass)
	{

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

			UG_CORE_TRACE("{0} has {1} Fields: ", className, fieldCount);
			void* it = nullptr;
			MonoClassField* field;
			while ((field = mono_class_get_fields(monoClass, &it)) != nullptr)
			{
				const char* fieldName = mono_field_get_name(field);
				uint32_t flags = mono_field_get_flags(field);
				UG_CORE_TRACE("  {0} flags = {1}", fieldName, flags);
				if (flags & MONO_FIELD_ATTR_PUBLIC)
				{
					MonoType* type = mono_field_get_type(field);
					const char* typeName = mono_type_get_name(type);

					ScriptFieldType fieldType = Utils::MonoTypeToScriptFieldType(type);

					UG_CORE_TRACE("   {0} ({1}) is public", fieldName, Utils::ScriptFieldTypeToString(fieldType));

					scriptClass->m_fields[fieldName] = { fieldName, fieldType, field };

				}
			}
		}
	}

	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity)
		: m_scriptClass(scriptClass)
	{

		// Held by a GC handle at once, or the next collection could free or move the object.
		m_gcHandle = mono_gchandle_new(scriptClass->Instantiate(), false);

		m_constructor = s_data->EntityMonoClass.GetMethod(".ctor", 1);
		m_onCreateMethod = scriptClass->GetMethod("OnCreate", 0);
		m_onUpdateMethod = scriptClass->GetMethod("OnUpdate", 1);

		// The Entity constructor takes the entity's UUID.
		{
			UUID entityID = entity.GetUUID();
			void* param = &entityID;
			m_scriptClass->InvokeMethod(GetManagedMonoObject(), m_constructor, &param);
		}
	}

	ScriptInstance::~ScriptInstance()
	{
		// Must run before the object's domain is unloaded.
		if (m_gcHandle)
		{
			mono_gchandle_free(m_gcHandle);
		}


	}

	MonoObject* ScriptInstance::GetManagedMonoObject()
	{
		return mono_gchandle_get_target(m_gcHandle);
	}

	void ScriptInstance::InvokeOnCreate()
	{

		m_scriptClass->InvokeMethod(GetManagedMonoObject(), m_onCreateMethod);
	}

	void ScriptInstance::InvokeOnUpdate(float ts)
	{

		void* param = &ts;

		m_scriptClass->InvokeMethod(GetManagedMonoObject(), m_onUpdateMethod, &param);
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
		mono_field_get_value(GetManagedMonoObject(), field.ClassField, buf);

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
		mono_field_set_value(GetManagedMonoObject(), field.ClassField, (void*)val);

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

			if (s_data->EntityScriptFields.find(entityID) != s_data->EntityScriptFields.end())
			{
				const ScriptFieldMap& fieldMap = s_data->EntityScriptFields.at(entityID);
				const auto& classFields = instance->GetScriptClass()->GetFields();
				for (const auto& [name, fieldInstance] : fieldMap)
				{
					// Copied only into a field of the same type. If a reload changed the type, the
					// field keeps its default and the stored value is kept.
					auto it = classFields.find(name);
					if (it != classFields.end() && it->second.Type != fieldInstance.Field.Type)
					{
						UG_CORE_WARN("{0}.{1} is now {2} but its value was set as {3}, so the field starts from its default",
							sc.ClassName, name, Utils::ScriptFieldTypeToString(it->second.Type),
							Utils::ScriptFieldTypeToString(fieldInstance.Field.Type));
						continue;
					}

					instance->SetFieldValueInternal(name, fieldInstance.m_dataBuffer);
				}


			}

			instance->InvokeOnCreate();
		}
		else
		{
			// Warned once here; OnUpdateEntity skips the entity silently.
			if (sc.ClassName.empty())
			{
				UG_CORE_WARN("Entity '{0}' ({1}) has a Script component with no class; it won't run.",
					entity.GetName(), (uint64_t)entity.GetUUID());
			}
			else
			{
				UG_CORE_WARN("Entity '{0}' ({1}) has script class '{2}', which isn't in the script assembly; it won't run.",
					entity.GetName(), (uint64_t)entity.GetUUID(), sc.ClassName);
			}
		}
	}

	void ScriptEngine::OnUpdateEntity(Entity entity, Timestep ts)
	{
		// Without an instance, OnCreateEntity has already warned; don't repeat it every frame.
		auto it = s_data->EntityInstances.find(entity.GetUUID());
		if (it != s_data->EntityInstances.end())
		{
			it->second->InvokeOnUpdate((float)ts);
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