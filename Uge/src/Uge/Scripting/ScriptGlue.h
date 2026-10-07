/**
 * @file ScriptGlue.h
 * @brief Registers the native functions that C# scripts call into.
 * @ingroup group_scripting
 */

#pragma once

namespace Uge
{

	/**
	 * @brief Binds the engine's native functions to their C# `InternalCall` declarations.
	 * @ingroup group_scripting
	 *
	 * The bridge in the script-to-engine direction: C# code calls a method marked
	 * `[MethodImpl(MethodImplOptions.InternalCall)]`, and Mono dispatches it to the native
	 * function registered here.
	 *
	 * @warning Every internal call needs matching changes on both sides — the registration
	 * in `ScriptGlue.cpp` and the declaration in
	 * `Uge-ScriptCore/Source/Uge/InternalCalls.cs`. The name and signature must agree
	 * exactly; a mismatch fails at runtime with a missing-method exception, not at compile
	 * time.
	 */
	class ScriptGlue
	{

	public:
		/**
		 * @brief Registers every native function C# can call.
		 *
		 * Called once during Uge::ScriptEngine::Init, and again after each assembly reload.
		 */
		static void RegisterFunctions();
		/**
		 * @brief Registers the component types so C# can query `HasComponent<T>()`.
		 *
		 * Only components with a C# class in Uge-ScriptCore are registered; each is listed by name in ScriptGlue.cpp.
		 */
		static void RegisterComponents();

	private:


	};


}