#include <ugpch.h>
#include "ScriptEngine.h"

#include "ScriptGlue.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>
#include <mono/metadata/attrdefs.h>

#include <glm/glm.hpp>

namespace Uge
{

	static std::unordered_map<std::string, ScriptFieldType> s_scriptFieldTypeMap =
	{
		{ "System.Single", ScriptFieldType::Float },
		{ "System.Double", ScriptFieldType::Double },

		{ "Uge.Vector2", ScriptFieldType::Vector2 },
		{ "Uge.Vector3", ScriptFieldType::Vector3 },
		{ "Uge.Vector4", ScriptFieldType::Vector4 },

		{ "System.Int64", ScriptFieldType::Long },
		{ "System.Int", ScriptFieldType::Int },
		{ "System.UInt32", ScriptFieldType::UInt },
		{ "System.Int16", ScriptFieldType::Short },

		{ "System.Boolean", ScriptFieldType::Bool },
		{ "System.Byte", ScriptFieldType::Byte },
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

		const char* ScriptFieldTypeToString(ScriptFieldType fieldtype)
		{

			switch (fieldtype)
			{
				case Uge::ScriptFieldType::None:
				{
					return "<invalid>";

				}
				case Uge::ScriptFieldType::Float:
				{
					return "Float";
				}
				case Uge::ScriptFieldType::Double:
				{
					return "Double";
				}
				case Uge::ScriptFieldType::Vector2:
				{
					return "Vector2";
				}
				case Uge::ScriptFieldType::Vector3:
				{
					return "Vector3";
				}
				case Uge::ScriptFieldType::Vector4:
				{
					return "Vector4";
				}
				case Uge::ScriptFieldType::Long:
				{
					return "Long";
				}
				case Uge::ScriptFieldType::Int:
				{
					return "Int";
				}
				case Uge::ScriptFieldType::UInt:
				{
					return "UInt";
				}
				case Uge::ScriptFieldType::Bool:
				{
					return "Bool";
				}
				case Uge::ScriptFieldType::Short:
				{
					return "Short";
				}
				case Uge::ScriptFieldType::Byte:
				{
					return "Byte";
				}
				case Uge::ScriptFieldType::Char:
				{
					return "Char";
				}
				case Uge::ScriptFieldType::String:
				{
					return "String";
				}
				case Uge::ScriptFieldType::Entity:
				{
					return "Entity";
				}
				default:
				{
					return "<invalid>";
				}
			}

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
		std::unordered_map<UUID, ScriptFieldMap> EntityScriptFields;


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

	Ref<ScriptClass> ScriptEngine::GetEntityClass(const std::string& name)
	{

		auto it = s_data->EntityClasses.find(name);
		if (it == s_data->EntityClasses.end())
		{
			nullptr;
		}

		return it->second;


		return Ref<ScriptClass>();
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
		UG_CORE_ASSERT(s_data->EntityInstances.find(entityUUID) != s_data->EntityInstances.end());

		Ref<ScriptInstance> instance = s_data->EntityInstances[entityUUID];
		instance->InvokeOnUpdate((float)ts);

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

}