#pragma once

#include "Uge/Scene/Scene.h"
#include "Uge/Scene/Entity.h"

#include <filesystem>
#include <string>
#include <unordered_map>

extern "C"
{
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoAssembly MonoAssembly;
	typedef struct _MonoImage MonoImage;
	typedef struct _MonoClassField MonoClassField;
}

namespace Uge
{

	enum class ScriptFieldType
	{
		None = 0,
		Float, 
		Double, 

		Vector2, 
		Vector3, 
		Vector4,

		Int, 
		UInt, 
		Long, 
		Short, 

		Bool, 

		Byte,
		Char,

		String,

		Entity
	};

	struct ScriptField
	{
		std::string Name;
		ScriptFieldType Type;
		MonoClassField* ClassField = nullptr;


	};


	// Script Field + data storage
	struct ScriptFieldInstance
	{
		ScriptField Field;

		ScriptFieldInstance()
		{
			memset(m_dataBuffer, 0, sizeof(m_dataBuffer));
		}

		template<typename T>
		T GetValue()
		{
			static_assert(sizeof(T) <= 8, "Type too large!");
			return *(T*)m_dataBuffer;
			

		}

		template<typename T>
		void SetValue(T value)
		{
			static_assert(sizeof(T) <= 8, "Type too large!");
			memcpy(m_dataBuffer, &value, sizeof(T));
			
		}

	private:
		char m_dataBuffer[8];

		friend class ScriptEngine;
		friend class ScriptInstance;

	};

	using ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;


	class ScriptClass
	{
	public:
		ScriptClass() = default;

		ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore = false);

		MonoObject* Instantiate();

		MonoMethod* GetMethod(const std::string& name, int parameterCount);
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);

		const std::unordered_map<std::string, ScriptField>& GetFields() const { return m_fields; }

	private:
		std::string m_classNamspace;
		std::string m_className;

		std::unordered_map<std::string, ScriptField> m_fields;

		MonoClass* m_monoClass = nullptr;

		friend class ScriptEngine;

	};


	class ScriptInstance
	{

	public:
		ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity);

		void InvokeOnCreate();
		void InvokeOnUpdate(float ts);

		Ref<ScriptClass> GetScriptClass() { return m_scriptClass; }

		template<typename T>
		T GetFieldValue(const std::string& name)
		{
			static_assert(sizeof(T) <= 8, "Type too large!");

			bool success = GetFieldValueInternal(name, s_fieldValueBuffer);
			if (!success)
			{
				return T();
			}

			return *(T*)s_fieldValueBuffer;
		}

		template<typename T>
		void SetFieldValue(const std::string& name, T value)
		{
			static_assert(sizeof(T) <= 8, "Type too large!");

			SetFieldValueInternal(name, &value);
			
		}
	
	private:

		
		bool GetFieldValueInternal(const std::string& name, void* buf);
		bool SetFieldValueInternal(const std::string& name, const void* val);

	private:
		Ref<ScriptClass> m_scriptClass;
		MonoObject* m_instance = nullptr;
		MonoMethod* m_constructor = nullptr;
		MonoMethod* m_onCreateMethod = nullptr;
		MonoMethod* m_onUpdateMethod = nullptr;

		inline static char s_fieldValueBuffer[8];

		friend class ScriptEngine;
		friend struct ScriptFieldInstance;

	};

	class ScriptEngine
	{

	public:
		static void Init();
		static void Shutdown();

		static void LoadAssembly(const std::filesystem::path& filePath);
		static void LoadAppAssembly(const std::filesystem::path& filePath);

		static Ref<ScriptClass> GetEntityClass(const std::string& name);
		static std::unordered_map<std::string, Ref<ScriptClass>> GetEntityClasses();
		static ScriptFieldMap& GetScriptFieldMap(UUID entityID);

		static void OnRuntimeStart(Scene* scene);
		static void OnRuntimeStop();

		static void OnCreateEntity(Entity entity);

		static void OnUpdateEntity(Entity entity, Timestep ts);

		static Scene* GetSceneContext();
		static Ref<ScriptInstance> GetEntityScriptInstance(UUID entityID);

		static MonoImage* GetCoreAssemblyImage();

		static bool EntityClassExists(const std::string& fullClassName);

	private:
		static void InitMono();
		static void ShutdownMono();

		static MonoObject* InstantiateClass(MonoClass* monoClass);
		static void LoadAssemblyClasses();

		friend class ScriptClass;

	};

}
