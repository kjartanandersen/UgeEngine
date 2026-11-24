/**
 * @file Input.cs
 * @brief Keyboard input for scripts.
 * @ingroup group_scripting
 */
using Uge;

namespace Source.Uge
{
    /**
     * @brief Keyboard state, from the native Uge::Input.
     * @ingroup group_scripting
     */
    public class Input
    {
        /**
         * @brief Whether a key is down.
         * @param keycode The key.
         * @return `true` if it is.
         */
        public static bool IsKeyDown(KeyCode keycode)
        {
            return InternalCalls.Input_IsKeyDown(keycode);
        }
    }
}
