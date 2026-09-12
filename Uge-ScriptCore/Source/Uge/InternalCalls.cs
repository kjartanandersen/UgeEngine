/**
 * @file InternalCalls.cs
 * @brief Declarations of the native engine functions callable from C#.
 * @ingroup group_scripting
 */
using System;
using System.Runtime.CompilerServices;


namespace Uge
{
    /**
     * @brief The managed half of the script bridge: `extern` declarations bound by the engine.
     * @ingroup group_scripting
     *
     * Each method is marked `InternalCall` and has no managed body — Mono dispatches it to
     * the native function registered by Uge::ScriptGlue::RegisterFunctions.
     *
     * @warning Names and signatures must match the native registrations exactly. A mismatch
     * surfaces at runtime as a missing-method exception, not as a compile error.
     *
     * @note Internal to the script core. Scripts use the wrappers — Uge.Entity,
     * Uge.Component, Source.Uge.Input — rather than calling these directly.
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

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CppFunction();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NativeLog(string text, int parameter);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NativeLogVector3(ref Vector3 parameter, out Vector3 outResult);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float NativeLogDot(ref Vector3 vector1, ref Vector3 vector2);

    }
}
