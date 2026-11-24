/**
 * @file ScriptGlue.h
 * @brief Registers the native functions that C# scripts call into.
 * @ingroup group_scripting
 */

#pragma once

namespace Uge
{

	/**
	 * @brief Binds native functions to their C# `InternalCall` declarations.
	 * @ingroup group_scripting
	 *
	 * Each internal call is registered here and declared in
	 * `Uge-ScriptCore/Source/Uge/InternalCalls.cs`. The name and signature must match
	 * exactly; a mismatch only fails at runtime.
	 */
	class ScriptGlue
	{

	public:
		/** @brief Registers every native function C# can call. */
		static void RegisterFunctions();
		/**
		 * @brief Registers the component types for C#'s `HasComponent<T>()`.
		 *
		 * Only components with a C# class are registered.
		 */
		static void RegisterComponents();

	private:


	};


}