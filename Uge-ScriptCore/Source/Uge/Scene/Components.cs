/**
 * @file Components.cs
 * @brief Components for scripts.
 * @ingroup group_scripting
 */

namespace Uge
{
    /**
     * @brief The base class of components. They hold no data; each property calls the engine.
     * @ingroup group_scripting
     */
    public abstract class Component
    {

        /** @brief The component's entity. */
        public Entity Entity { get; internal set; }

    }

    /**
     * @brief The native Uge::TransformComponent.
     * @ingroup group_scripting
     */
    public class TransformComponent : Component
    {
        /** @brief The translation. */
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
