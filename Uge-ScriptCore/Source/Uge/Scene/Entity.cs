/**
 * @file Entity.cs
 * @brief The managed base class every C# entity script derives from.
 * @ingroup group_scripting
 */
using System;

namespace Uge
{
    /**
     * @brief The base class for entity scripts; the managed view of a native entity.
     * @ingroup group_scripting
     *
     * Uge::ScriptEngine instantiates a subclass of this for every entity carrying a
     * Uge::ScriptComponent, passing the entity's UUID to the internal constructor.
     *
     * @code
     * public class Player : Entity
     * {
     *     void OnUpdate(float ts)
     *     {
     *         if (Input.IsKeyDown(KeyCode.W))
     *             Translation += new Vector3(0.0f, ts, 0.0f);
     *     }
     * }
     * @endcode
     */
    public class Entity
    {

        /** @brief Constructor used when a script subclass is created by the engine. */
        protected Entity() { ID = 0; }

        /**
         * @brief Wraps an existing native entity.
         * @param id The entity's UUID.
         */
        internal Entity(ulong id) { ID = id; }


        /** @brief The entity's UUID, matching its native Uge::IDComponent. */
        public readonly ulong ID;

        /**
         * @brief The entity's world position.
         *
         * Reads and writes go straight through to the native Uge::TransformComponent, so
         * assigning to this moves the entity immediately.
         */
        public Vector3 Translation
        {

            get
            {

                InternalCalls.TransformComponent_GetTranslation(ID, out Vector3 result);
                return result;

            }

            set
            {
                InternalCalls.TransformComponent_SetTranslation(ID, ref value);
            }
        }

        /**
         * @brief Tests whether the entity carries a component.
         * @tparam T Component type to test for.
         * @return `true` if the native entity has the corresponding component.
         */
        public bool HasComponent<T>() where T : Component, new()
        {
            Type componentType = typeof(T);
            return InternalCalls.Entity_HasComponent(ID, componentType);
        }

        /**
         * @brief Retrieves a component wrapper bound to this entity.
         * @tparam T Component type to fetch.
         * @return The component, or `null` if the entity does not have one.
         */
        public T GetComponent<T>() where T : Component, new()
        {
            if (!HasComponent<T>())
                return null;

            T component = new T() { Entity = this };
            return component;
        }

        /**
         * @brief Finds another entity in the scene by name.
         * @param name Tag to search for.
         * @return The entity, or `null` if no entity has that name.
         */
        public Entity FindEntityByName(string name)
        {
            ulong entityID = InternalCalls.Entity_FindEntityByName(name);
            if (entityID == 0)
            {
                return null;
            }
            return new Entity(entityID);
        }

        /**
         * @brief Casts this entity to its concrete script class.
         * @tparam T The script type expected.
         * @return The script instance, or `null` if the entity's script is not a `T`.
         */
        public T As<T>() where T : Entity, new()
        {

            object instance = InternalCalls.GetScriptInstance(ID);

            return instance as T;
        }

    }
    
}
