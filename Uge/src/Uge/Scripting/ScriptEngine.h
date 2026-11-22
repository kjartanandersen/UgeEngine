/**
 * @file ScriptEngine.h
 * @brief The embedded Mono runtime, and the C# class, instance and field wrappers.
 * @ingroup group_scripting
 */

#pragma once

#include "Uge/Scene/Scene.h"
#include "Uge/Scene/Entity.h"

#include <filesystem>
#include <string>
#include <unordered_map>

extern "C"
{
	/** @brief A managed class. */
	typedef struct _MonoClass MonoClass;
	/** @brief A managed object. */
	typedef struct _MonoObject MonoObject;
	/** @brief A managed method. */
	typedef struct _MonoMethod MonoMethod;
	/** @brief A loaded assembly. */
	typedef struct _MonoAssembly MonoAssembly;
	/** @brief An assembly's metadata image. */
	typedef struct _MonoImage MonoImage;
	/** @brief A field of a managed class. */
	typedef struct _MonoClassField MonoClassField;
}

namespace Uge
{

	/**
	 * @brief The C# field types the engine can show and save. Other fields are ignored.
	 * @ingroup group_scripting
	 */
	enum class ScriptFieldType
	{
		None = 0, ///< Unsupported.
		Float, ///< `float`
		Double, ///< `double`

		Vector2, ///< `Uge.Vector2`
		Vector3, ///< `Uge.Vector3`
		Vector4, ///< `Uge.Vector4`

		Int, ///< `int`
		UInt, ///< `uint`
		Long, ///< `long`
		ULong, ///< `ulong`
		Short, ///< `short`
		UShort, ///< `ushort`

		Bool, ///< `bool`

		Byte, ///< `sbyte`
		UByte, ///< `byte`
		Char, ///< `char`

		String, ///< `string`

		Entity ///< `Uge.Entity`, stored as a UUID.
	};

	/**
	 * @brief A public field of a C# class.
	 * @ingroup group_scripting
	 */
	struct ScriptField
	{
		std::string Name; ///< Name in C#.
		ScriptFieldType Type; ///< Type.
		MonoClassField* ClassField = nullptr; ///< Mono handle for reading and writing it.


	};


	/**
	 * @brief A script field and its value, kept outside the managed heap.
	 * @ingroup group_scripting
	 *
	 * The value survives without a managed object: before play starts and across an
	 * assembly reload. Values of up to 16 bytes fit.
	 */
	struct ScriptFieldInstance
	{
		ScriptField Field; ///< The field.

		/** @brief Creates the instance with a zeroed value. */
		ScriptFieldInstance()
		{
			memset(m_dataBuffer, 0, sizeof(m_dataBuffer));
		}

		/**
		 * @brief The stored value.
		 * @tparam T Type to read as, at most 16 bytes. Not checked against the field type.
		 * @return The value.
		 */
		template<typename T>
		T GetValue() const
		{
			static_assert(sizeof(T) <= 16, "Type too large!");
			T value;
			memcpy(&value, m_dataBuffer, sizeof(T));
			return value;
		}

		/**
		 * @brief Stores a value.
		 * @tparam T Its type, at most 16 bytes.
		 * @param value The value.
		 */
		template<typename T>
		void SetValue(T value)
		{
			static_assert(sizeof(T) <= 16, "Type too large!");
			memcpy(m_dataBuffer, &value, sizeof(T));
			
		}

	private:
		char m_dataBuffer[16];

		friend class ScriptEngine;
		friend class ScriptInstance;

	};

	/**
	 * @brief One entity's stored field values, by field name.
	 * @ingroup group_scripting
	 */
	using ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;


	/**
	 * @brief A C# class from a loaded assembly, with its public fields.
	 * @ingroup group_scripting
	 */
	class ScriptClass
	{
	public:
		/** @brief A wrapper bound to no class. */
		ScriptClass() = default;

		/**
		 * @brief Looks a class up.
		 * @param classNamespace C# namespace, such as `Sandbox`.
		 * @param className Class name.
		 * @param isCore `true` to look in `Uge-ScriptCore` instead of the project's assembly.
		 */
		ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore = false);

		/**
		 * @brief Allocates an object of the class, without running its constructor.
		 * @return The object.
		 */
		MonoObject* Instantiate();

		/**
		 * @brief A method, by name and parameter count.
		 * @param name Method name.
		 * @param parameterCount Number of parameters.
		 * @return The method, or null.
		 */
		MonoMethod* GetMethod(const std::string& name, int parameterCount);
		/**
		 * @brief Calls a method.
		 * @param instance Object to call on; null for a static method.
		 * @param method The method.
		 * @param params Pointers to the arguments, or null.
		 * @return The boxed result, or null for `void`.
		 */
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);

		/**
		 * @brief The public fields.
		 * @return Fields by name.
		 */
		const std::unordered_map<std::string, ScriptField>& GetFields() const { return m_fields; }

	private:
		std::string m_classNamspace;
		std::string m_className;

		std::unordered_map<std::string, ScriptField> m_fields;

		MonoClass* m_monoClass = nullptr;

		friend class ScriptEngine;

	};


	/**
	 * @brief A script object attached to one entity.
	 * @ingroup group_scripting
	 *
	 * Holds the managed object through a strong GC handle. Mono doesn't scan native memory,
	 * so a raw pointer would neither keep the object alive nor follow it when it moves.
	 */
	class ScriptInstance
	{

	public:
		/**
		 * @brief Creates the object and runs its constructor with the entity's UUID.
		 * @param scriptClass Class to create.
		 * @param entity The entity.
		 */
		ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity);

		/** @brief Releases the GC handle, so Mono can collect the object. */
		~ScriptInstance();

		/** @brief Not copyable; both copies would free the GC handle. */
		ScriptInstance(const ScriptInstance&) = delete;
		/** @brief Not copyable; both copies would free the GC handle. */
		ScriptInstance& operator=(const ScriptInstance&) = delete;

		/** @brief Calls the script's `OnCreate`. */
		void InvokeOnCreate();
		/**
		 * @brief Calls the script's `OnUpdate`.
		 * @param ts Seconds since the last frame.
		 */
		void InvokeOnUpdate(float ts);

		/**
		 * @brief The script's class.
		 * @return The class.
		 */
		Ref<ScriptClass> GetScriptClass() { return m_scriptClass; }

		/**
		 * @brief Reads a field of the object.
		 * @tparam T Field type, at most 8 bytes.
		 * @param name Field name.
		 * @return The value, or `T()` if there is no such field.
		 */
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

		/**
		 * @brief Writes a field of the object.
		 * @tparam T Field type, at most 8 bytes.
		 * @param name Field name.
		 * @param value The value.
		 */
		template<typename T>
		void SetFieldValue(const std::string& name, T value)
		{
			static_assert(sizeof(T) <= 8, "Type too large!");

			SetFieldValueInternal(name, &value);
			
		}

		/**
		 * @brief The managed object.
		 * @return The object. Don't keep the pointer: the collector can move the object.
		 */
		MonoObject* GetManagedMonoObject();
	
	private:

		
		bool GetFieldValueInternal(const std::string& name, void* buf);
		bool SetFieldValueInternal(const std::string& name, const void* val);

	private:
		Ref<ScriptClass> m_scriptClass;
		uint32_t m_gcHandle = 0;				///< Strong GC handle that keeps the object alive.
		MonoMethod* m_constructor = nullptr;
		MonoMethod* m_onCreateMethod = nullptr;
		MonoMethod* m_onUpdateMethod = nullptr;

		inline static char s_fieldValueBuffer[8];

		friend class ScriptEngine;
		friend struct ScriptFieldInstance;

	};

	/**
	 * @brief Runs the Mono runtime and the entity scripts.
	 * @ingroup group_scripting
	 *
	 * Loads `Uge-ScriptCore` and the project's script assembly,
	 * Uge::ProjectConfig::ScriptModulePath.
	 */
	class ScriptEngine
	{

	public:
		/**
		 * @brief Starts Mono and registers the internal calls. Call once per process.
		 *
		 * Without the Mono runtime or the core assembly in the working directory, scripting
		 * is disabled, but field values can still be stored.
		 */
		static void Init();
		/** @brief Unloads the project's scripts and shuts Mono down. */
		static void Shutdown();

		/**
		 * @brief Loads the core assembly and the active project's script assembly, finds the
		 * entity classes, and watches the script assembly for rebuilds.
		 *
		 * Does nothing when scripting is disabled. Call UnloadProject() first if another
		 * project's scripts are loaded.
		 */
		static void LoadProject();
		/**
		 * @brief Stops watching the script assembly, destroys the script objects, forgets the
		 * classes and stored field values, and unloads the app domain. Safe when nothing is
		 * loaded.
		 */
		static void UnloadProject();

		/**
		 * @brief Loads `Uge-ScriptCore`.
		 * @param filePath The DLL.
		 * @return `true` on success.
		 */
		static bool LoadAssembly(const std::filesystem::path& filePath);

		/**
		 * @brief Loads the project's script assembly and finds its entity classes.
		 * @param filePath The DLL.
		 * @return `true` on success.
		 */
		static bool LoadAppAssembly(const std::filesystem::path& filePath);

		/**
		 * @brief Reloads both assemblies, to pick up rebuilt scripts.
		 *
		 * Stored field values survive; script objects don't.
		 */
		static void ReloadAssembly();

		/**
		 * @brief An entity class.
		 * @param name Full class name, such as `Sandbox.Player`.
		 * @return The class, or null.
		 */
		static Ref<ScriptClass> GetEntityClass(const std::string& name);

		/**
		 * @brief Every class deriving from `Entity`.
		 * @return Classes by full name.
		 */
		static std::unordered_map<std::string, Ref<ScriptClass>> GetEntityClasses();

		/**
		 * @brief An entity's stored field values.
		 * @param entityID The entity.
		 * @return The values, created empty if needed; null before Init() and after Shutdown().
		 *
		 * Values are kept even while their class isn't loaded.
		 */
		static ScriptFieldMap* GetScriptFieldMap(UUID entityID);

		/**
		 * @brief Starts scripting for a scene entering play mode.
		 * @param scene The scene; not owned.
		 */
		static void OnRuntimeStart(Scene* scene);
		/** @brief Destroys every script object and forgets the scene. */
		static void OnRuntimeStop();

		/**
		 * @brief Creates an entity's script object and calls `OnCreate`.
		 * @param entity An entity with a Uge::ScriptComponent.
		 *
		 * A missing class gets one warning and no object. A stored value whose type no longer
		 * matches its field is skipped, with a warning.
		 */
		static void OnCreateEntity(Entity entity);

		/**
		 * @brief Calls an entity script's `OnUpdate`. Does nothing without a script object.
		 * @param entity The entity.
		 * @param ts Frame time.
		 */
		static void OnUpdateEntity(Entity entity, Timestep ts);

		/**
		 * @brief The scene running scripts.
		 * @return The scene, or null outside play mode.
		 */
		static Scene* GetSceneContext();
		/**
		 * @brief An entity's script object.
		 * @param entityID The entity.
		 * @return The instance, or null.
		 */
		static Ref<ScriptInstance> GetEntityScriptInstance(UUID entityID);

		/**
		 * @brief The core assembly's image.
		 * @return The image.
		 */
		static MonoImage* GetCoreAssemblyImage();

		/**
		 * @brief The managed object of an entity's script.
		 * @param entityID The entity.
		 * @return The object, or null.
		 */
		static MonoObject* GetManagedInstance(UUID entityID);

		/**
		 * @brief Whether a class is in the loaded assembly.
		 * @param fullClassName Full class name.
		 * @return `true` if it is.
		 */
		static bool EntityClassExists(const std::string& fullClassName);

	private:
		static void InitMono();
		static void ShutdownMono();

		static MonoObject* InstantiateClass(MonoClass* monoClass);
		static void LoadAssemblyClasses();

		friend class ScriptClass;

	};

	/**
	 * @brief Script field type conversions.
	 * @ingroup group_scripting
	 */
	namespace Utils
	{

		/**
		 * @brief The field type for a managed type name.
		 * @param typeName A name from `mono_type_get_name`, such as `System.Int32`.
		 * @return The type, or `None` with a warning for an unsupported one.
		 *
		 * The names are .NET's: `int` is `System.Int32` and `float` is `System.Single`.
		 */
		ScriptFieldType ScriptFieldTypeFromManagedName(const std::string& typeName);

		/**
		 * @brief Parses a name written by Uge::Utils::ScriptFieldTypeToString.
		 * @param fieldType The name.
		 * @return The type, or `None` for an unknown name.
		 */
		inline ScriptFieldType ScriptFieldTypeFromString(std::string_view fieldType)
		{
			if (fieldType == "None")    return ScriptFieldType::None;
			if (fieldType == "Float")   return ScriptFieldType::Float;
			if (fieldType == "Double")  return ScriptFieldType::Double;
			if (fieldType == "Bool")    return ScriptFieldType::Bool;
			if (fieldType == "Char")    return ScriptFieldType::Char;
			if (fieldType == "String")  return ScriptFieldType::String;
			if (fieldType == "Byte")    return ScriptFieldType::Byte;
			if (fieldType == "Short")   return ScriptFieldType::Short;
			if (fieldType == "Int")     return ScriptFieldType::Int;
			if (fieldType == "Long")    return ScriptFieldType::Long;
			if (fieldType == "UByte")   return ScriptFieldType::UByte;
			if (fieldType == "UShort")  return ScriptFieldType::UShort;
			if (fieldType == "UInt")    return ScriptFieldType::UInt;
			if (fieldType == "ULong")   return ScriptFieldType::ULong;
			if (fieldType == "Vector2") return ScriptFieldType::Vector2;
			if (fieldType == "Vector3") return ScriptFieldType::Vector3;
			if (fieldType == "Vector4") return ScriptFieldType::Vector4;
			if (fieldType == "Entity")  return ScriptFieldType::Entity;

			return ScriptFieldType::None;
		}

		/**
		 * @brief The serialized name of a field type.
		 * @param fieldtype The type.
		 * @return A name such as `"Vector3"`, or `"<invalid>"`.
		 */
		inline const char* ScriptFieldTypeToString(ScriptFieldType fieldtype)
		{

			switch (fieldtype)
			{
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
				case Uge::ScriptFieldType::ULong:
				{
					return "ULong";
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
				case Uge::ScriptFieldType::UShort:
				{
					return "UShort";
				}
				case Uge::ScriptFieldType::Byte:
				{
					return "Byte";
				}
				case Uge::ScriptFieldType::UByte:
				{
					return "UByte";
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
			
			}

			return "<invalid>";
		}

	}

}
