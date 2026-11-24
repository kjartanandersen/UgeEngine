/**
 * @file Entity.cs
 * @brief The base class of entity scripts.
 * @ingroup group_scripting
 */
using System;

namespace Uge
{
    /**
     * @brief A native entity, and the base class of entity scripts.
     * @ingroup group_scripting
     */
    public class Entity
    {

        /** @brief An entity with ID 0. */
        protected Entity() { ID = 0; }

        /**
         * @brief Wraps a native entity.
         * @param id The entity's UUID.
         */
        internal Entity(ulong id) { ID = id; }


        /** @brief The entity's UUID. */
        public readonly ulong ID;

        /**
         * @brief The Uge::TransformComponent's translation, read and written directly.
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
         * @brief Whether the entity has a component.
         * @tparam T The component type.
         * @return `true` if it has one.
         */
        public bool HasComponent<T>() where T : Component, new()
        {
            Type componentType = typeof(T);
            return InternalCalls.Entity_HasComponent(ID, componentType);
        }

        /**
         * @brief A component of the entity.
         * @tparam T The component type.
         * @return The component, or `null` if the entity has none.
         */
        public T GetComponent<T>() where T : Component, new()
        {
            if (!HasComponent<T>())
                return null;

            T component = new T() { Entity = this };
            return component;
        }

        /**
         * @brief An entity in the scene with a given tag.
         * @param name The tag.
         * @return The entity, or `null` if there is none.
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
         * @brief The entity's script instance, as a `T`.
         * @tparam T The script class.
         * @return The instance, or `null` if it isn't a `T`.
         */
        public T As<T>() where T : Entity, new()
        {

            object instance = InternalCalls.GetScriptInstance(ID);

            return instance as T;
        }

    }
    
}
