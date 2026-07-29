/**
 * @file Components.cs
 * @brief Managed wrappers over the native component types.
 * @ingroup group_scripting
 */

namespace Uge
{
    /**
     * @brief Base class for the managed component wrappers.
     * @ingroup group_scripting
     *
     * Wrappers hold no data of their own: each property forwards to the native component
     * through an internal call keyed on the owning entity's UUID.
     */
    public abstract class Component
    {

        /** @brief The entity this component belongs to. */
        public Entity Entity { get; internal set; }

    }

    /**
     * @brief Managed view of the native Uge::TransformComponent.
     * @ingroup group_scripting
     */
    public class TransformComponent : Component
    {
        /** @brief The entity's world position, read and written through the engine. */
        public Vector3 Translation
        {
            get
            {
                InternalCalls.TransformComponent_GetTranslation(Entity.ID, out Vector3 translation);
                return translation;
            }
            set
            {
                InternalCalls.TransformComponent_SetTranslation(Entity.ID, ref value);
            }
        }
    }
}
