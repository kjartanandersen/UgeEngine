/**
 * @file InternalCalls.cs
 * @brief The native functions C# can call.
 * @ingroup group_scripting
 */
using System;
using System.Runtime.CompilerServices;


namespace Uge
{
    /**
     * @brief Internal calls, bound to the functions Uge::ScriptGlue registers.
     * @ingroup group_scripting
     *
     * Names and signatures must match the native side; a mismatch throws a missing-method
     * exception at runtime.
     */
    public static class InternalCalls
    {

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_HasComponent(ulong entityID, Type componentType);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]

        internal extern static ulong Entity_FindEntityByName(string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static object GetScriptInstance(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetTranslation(ulong entityID, out Vector3 translation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetTranslation(ulong entityID, ref Vector3 translation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsKeyDown(KeyCode keycode);

    }
}
