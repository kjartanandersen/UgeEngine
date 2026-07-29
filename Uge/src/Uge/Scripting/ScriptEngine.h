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
	/** @brief Opaque Mono type handle; forward-declared to avoid including the Mono headers. */
	typedef struct _MonoClass MonoClass;
	/** @brief Opaque handle to a managed object instance. */
	typedef struct _MonoObject MonoObject;
	/** @brief Opaque handle to a managed method. */
	typedef struct _MonoMethod MonoMethod;
	/** @brief Opaque handle to a loaded managed assembly. */
	typedef struct _MonoAssembly MonoAssembly;
	/** @brief Opaque handle to an assembly's metadata image. */
	typedef struct _MonoImage MonoImage;
	/** @brief Opaque handle to a field on a managed class. */
	typedef struct _MonoClassField MonoClassField;
}

namespace Uge
{

	/**
	 * @brief The C# field types the editor can inspect and serialize.
	 * @ingroup group_scripting
	 *
	 * Anything not listed here is invisible to the property panel and is not persisted,
	 * even if the managed field exists.
	 */
	enum class ScriptFieldType
	{
		None = 0, ///< Unset / unsupported field type.
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
	 * @brief Describes a public field on a C# class.
	 * @ingroup group_scripting
	 *
	 * Reflected out of the assembly when it is loaded; carries the type information but
	 * not a value. @see ScriptFieldInstance
	 */
	struct ScriptField
	{
		std::string Name; ///< Field name as declared in C#.
		ScriptFieldType Type; ///< Field type.
		MonoClassField* ClassField = nullptr; ///< Mono handle used to read and write the field.


	};


	// Script Field + data storage
	/**
	 * @brief A script field together with a value, stored outside the managed heap.
	 * @ingroup group_scripting
	 *
	 * Field values set in the editor have to survive when no managed object exists — before
	 * play begins, and across an assembly reload. They are kept here in a fixed 16-byte
	 * buffer and pushed into the managed instance when the scene starts.
	 *
	 * @warning The buffer is 16 bytes; GetValue() and SetValue() static-assert on larger
	 * types.
	 */
	struct ScriptFieldInstance
	{
		ScriptField Field; ///< Which field this value belongs to.

		/** @brief Constructs the instance with a zeroed value buffer. */
		ScriptFieldInstance()
		{
			memset(m_dataBuffer, 0, sizeof(m_dataBuffer));
		}

		/**
		 * @brief Reads the stored value.
		 * @tparam T Type to read as; must be at most 16 bytes.
		 * @return The stored value reinterpreted as `T`.
		 * @warning No type checking: reading as the wrong `T` reinterprets the raw bytes.
		 */
		template<typename T>
		T GetValue()
		{
			static_assert(sizeof(T) <= 16, "Type too large!");
			return *(T*)m_dataBuffer;
			

		}

		/**
		 * @brief Stores a value.
		 * @tparam T Type to store; must be at most 16 bytes.
		 * @param value Value to copy into the buffer.
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
	 * @brief Field name to stored value, for one entity's script.
	 * @ingroup group_scripting
	 */
	using ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;


	/**
	 * @brief A C# class reflected out of a loaded assembly.
	 * @ingroup group_scripting
	 *
	 * Wraps a `MonoClass` and caches its public fields, so the editor can show them without
	 * re-reflecting each frame. One of these exists per script class; each entity using it
	 * gets its own Uge::ScriptInstance.
	 */
	class ScriptClass
	{
	public:
		/** @brief Constructs an empty wrapper bound to no class. */
		ScriptClass() = default;

		/**
		 * @brief Looks a class up in a loaded assembly.
		 * @param classNamespace C# namespace, e.g. `Sandbox`.
		 * @param className Class name without the namespace.
		 * @param isCore `true` to search the engine's `Uge-ScriptCore` assembly instead of the
		 *        project's script assembly.
		 */
		ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore = false);

		/**
		 * @brief Allocates a managed object of this class.
		 * @return The new managed object; its constructor has not run yet.
		 */
		MonoObject* Instantiate();

		/**
		 * @brief Looks up a method by name and arity.
		 * @param name Method name.
		 * @param parameterCount Number of parameters, used to pick between overloads.
		 * @return The method, or null if no match exists.
		 */
		MonoMethod* GetMethod(const std::string& name, int parameterCount);
		/**
		 * @brief Calls a managed method.
		 * @param instance Object to call on; null for a static method.
		 * @param method Method to invoke.
		 * @param params Array of pointers to the arguments, or null when there are none.
		 * @return The boxed return value, or null for a `void` method.
		 */
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);

		/**
		 * @brief The class's public fields.
		 * @return Const reference to the field map, keyed by field name.
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
	 * @brief A live managed object bound to one entity.
	 * @ingroup group_scripting
	 *
	 * Created by Uge::ScriptEngine::OnCreateEntity when the scene starts running. It caches
	 * the `OnCreate` and `OnUpdate` method pointers up front so the per-frame call does not
	 * pay for a method lookup.
	 */
	class ScriptInstance
	{

	public:
		/**
		 * @brief Instantiates the class and runs its constructor with the entity's UUID.
		 * @param scriptClass Class to instantiate.
		 * @param entity Entity this script is attached to.
		 */
		ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity);

		/** @brief Calls the script's `OnCreate`, once, when the scene starts. */
		void InvokeOnCreate();
		/**
		 * @brief Calls the script's `OnUpdate`.
		 * @param ts Frame delta time in seconds.
		 */
		void InvokeOnUpdate(float ts);

		/**
		 * @brief The class this instance was created from.
		 * @return The script class.
		 */
		Ref<ScriptClass> GetScriptClass() { return m_scriptClass; }

		/**
		 * @brief Reads a field from the managed object.
		 * @tparam T Field type; must be at most 8 bytes.
		 * @param name Field name.
		 * @return The field's value, or a default-constructed `T` if the field does not exist.
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
		 * @brief Writes a field on the managed object.
		 * @tparam T Field type; must be at most 8 bytes.
		 * @param name Field name.
		 * @param value Value to write.
		 */
		template<typename T>
		void SetFieldValue(const std::string& name, T value)
		{
			static_assert(sizeof(T) <= 8, "Type too large!");

			SetFieldValueInternal(name, &value);
			
		}

		/**
		 * @brief The underlying managed object.
		 * @return The `MonoObject` this instance wraps.
		 */
		MonoObject* GetManagedMonoObject() { return m_instance; }
	
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

	/**
	 * @brief Hosts the Mono runtime and drives every entity script.
	 * @ingroup group_scripting
	 *
	 * Loads two assemblies: the engine's `Uge-ScriptCore`, which defines the `Entity` base
	 * class and the internal-call declarations, and the project's script assembly named by
	 * Uge::ProjectConfig::ScriptModulePath.
	 *
	 * At runtime start it creates a Uge::ScriptInstance for every entity carrying a
	 * Uge::ScriptComponent, then calls `OnCreate` on each and `OnUpdate` every frame.
	 *
	 * @note Native functions callable from C# are registered by Uge::ScriptGlue. Adding one
	 * requires matching changes in `ScriptGlue.cpp` and `InternalCalls.cs`; a mismatch
	 * fails at runtime, not at compile time.
	 */
	class ScriptEngine
	{

	public:
		/**
		 * @brief Starts the Mono runtime, loads the core assembly and registers the glue.
		 */
		static void Init();
		/** @brief Tears down the Mono runtime. Called from ~Application. */
		static void Shutdown();

		/**
		 * @brief Loads the engine's `Uge-ScriptCore` assembly.
		 * @param filePath Path to the core assembly DLL.
		 * @return `true` on success.
		 */
		static bool LoadAssembly(const std::filesystem::path& filePath);
		/**
		 * @brief Loads the project's script assembly and reflects its entity classes.
		 * @param filePath Path to the project's script DLL.
		 * @return `true` on success.
		 */
		static bool LoadAppAssembly(const std::filesystem::path& filePath);

		/**
		 * @brief Unloads and reloads both assemblies, picking up a recompiled script DLL.
		 *
		 * Lets scripts be rebuilt without restarting the editor. Field values stored in the
		 * script field maps survive the reload; live managed objects do not.
		 */
		static void ReloadAssembly();

		/**
		 * @brief Looks up a reflected entity class.
		 * @param name Fully qualified class name, e.g. `Sandbox.Player`.
		 * @return The class, or null if it is not in the loaded assembly.
		 */
		static Ref<ScriptClass> GetEntityClass(const std::string& name);
		/**
		 * @brief Every class deriving from the script core's `Entity`.
		 * @return Map from fully qualified name to class; populates the editor's script picker.
		 */
		static std::unordered_map<std::string, Ref<ScriptClass>> GetEntityClasses();
		/**
		 * @brief The stored field values for an entity's script.
		 * @param entityID Entity to look up.
		 * @return Mutable reference to its field map, created empty if it does not exist.
		 */
		static ScriptFieldMap& GetScriptFieldMap(UUID entityID);

		/**
		 * @brief Binds the engine to a scene entering play mode.
		 * @param scene Scene that is starting; borrowed, not owned.
		 */
		static void OnRuntimeStart(Scene* scene);
		/** @brief Destroys every script instance and clears the scene context. */
		static void OnRuntimeStop();

		/**
		 * @brief Instantiates an entity's script and calls its `OnCreate`.
		 * @param entity Entity carrying a Uge::ScriptComponent.
		 */
		static void OnCreateEntity(Entity entity);

		/**
		 * @brief Calls an entity script's `OnUpdate`.
		 * @param entity Entity whose script should tick.
		 * @param ts Frame delta time.
		 */
		static void OnUpdateEntity(Entity entity, Timestep ts);

		/**
		 * @brief The scene currently running scripts.
		 * @return The scene, or null outside play mode. Used by the internal calls to resolve
		 *         entity handles.
		 */
		static Scene* GetSceneContext();
		/**
		 * @brief The live script instance for an entity.
		 * @param entityID Entity to look up.
		 * @return The instance, or null if the entity has no running script.
		 */
		static Ref<ScriptInstance> GetEntityScriptInstance(UUID entityID);

		/**
		 * @brief The core assembly's Mono image.
		 * @return The image, used to look up the script core's built-in types.
		 */
		static MonoImage* GetCoreAssemblyImage();

		/**
		 * @brief The managed object backing an entity's script.
		 * @param entityID Entity to look up.
		 * @return The `MonoObject`, or null if the entity has no running script.
		 */
		static MonoObject* GetManagedInstance(UUID entityID);

		/**
		 * @brief Whether a script class is present in the loaded assembly.
		 * @param fullClassName Fully qualified class name.
		 * @return `true` if the class exists.
		 *
		 * The editor uses this to flag a Uge::ScriptComponent naming a class that no longer
		 * exists after a reload.
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
	 * @brief Conversions between Uge::ScriptFieldType and its serialized name.
	 * @ingroup group_scripting
	 */
	namespace Utils
	{

		/**
		 * @brief Parses a field type from its serialized name.
		 * @param fieldType String produced by Uge::Utils::ScriptFieldTypeToString.
		 * @return The matching type; asserts in Debug and returns `None` if unrecognized.
		 */
		inline ScriptFieldType ScriptFieldTypeFromString(std::string_view fieldType)
		{
			if (fieldType == "None")    return ScriptFieldType::None;
			if (fieldType == "Float")   return ScriptFieldType::Float;
			if (fieldType == "Double")  return ScriptFieldType::Double;
			if (fieldType == "Bool")    return ScriptFieldType::Bool;
			if (fieldType == "Char")    return ScriptFieldType::Char;
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

			UG_CORE_ASSERT(false, "Unknown ScriptFieldType");
			return ScriptFieldType::None;
		}

		/**
		 * @brief Converts a field type to its serialized name.
		 * @param fieldtype Type to convert.
		 * @return A static string such as `"Vector3"`, or `"<invalid>"` if unrecognized.
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
