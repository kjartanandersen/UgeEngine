/**
 * @file Input.cs
 * @brief Managed keyboard input, forwarded to the engine through an internal call.
 * @ingroup group_scripting
 */
using Uge;

namespace Source.Uge
{
    /**
     * @brief Polled keyboard state, as seen from a C# script.
     * @ingroup group_scripting
     *
     * The managed mirror of the native Uge::Input. Calls cross into the engine through
     * Uge::ScriptGlue.
     */
    public class Input
    {
        /**
         * @brief Tests whether a key is currently held down.
         * @param keycode Key to test.
         * @return `true` while the key is down.
         */
        public static bool IsKeyDown(KeyCode keycode)
        {
            return InternalCalls.Input_IsKeyDown(keycode);
        }
    }
}
