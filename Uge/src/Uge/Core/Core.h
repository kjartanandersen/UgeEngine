/**
 * @file Core.h
 * @brief Platform detection, assertion macros and the engine's smart-pointer aliases.
 * @ingroup group_core
 *
 * Included, directly or transitively, by essentially every other engine header. It
 * defines the platform macro for the current target, the debug-break primitive, the
 * assertion and verification macro families, and the Uge::Ref / Uge::Scope aliases.
 */

#pragma once

#include <memory>


// Platform detection using predefined macros
#ifdef _WIN32
	#ifdef _WIN64
		/**
		 * @def UG_PLATFORM_WINDOWS
		 * @brief Defined when building for 64-bit Windows, the only supported target.
		 * @ingroup group_core
		 *
		 * Every other platform branch in this header raises `#error`; 32-bit Windows is
		 * rejected explicitly.
		 */
		#define UG_PLATFORM_WINDOWS
	#else
		#error "x86 Builds are not supported!"
	#endif
#elif defined(__APPLE__) || defined(__MACH__)
	#include <TargetConditionals.h>
	#if TARGET_IPHONE_SIMULATOR == 1
		#error "iOS simulator not supported!"
	#elif TARGET_OS_IPHONE == 1
		#define UG_PLATFORM_IOS
		#error "iOS not supported!"
	#elif TARGET_OS_MAC == 1
		#define UG_PLATFORM_MACOS
		#error "MacOS not supported"
	#else
		#error "Uknown Apple platform!"
	#endif
#elif defined(__ANDROID__)
	#define UG_PLATFORM_ANDROID
	#error "Android is not supported!"
#elif defined(__linux__)
	#define UG_PLATFORM_LINUX
	#error "Linux is not supported!"
#else
	#error "Unknown platform!"
#endif

#if defined(UG_PLATFORM_WINDOWS)
	/**
	 * @def UG_DEBUGBREAK
	 * @brief Traps into the attached debugger.
	 * @ingroup group_core
	 *
	 * Maps to `__debugbreak()` on Windows and `raise(SIGTRAP)` on Linux. Used by the
	 * assertion macros; calling it with no debugger attached terminates the process.
	 */
	#define UG_DEBUGBREAK() __debugbreak()
#elif defined(UG_PLATFORM_LINUX)
	#include <signal.h>
	#define UG_DEBUGBREAK() raise(SIGTRAP)
#else
	#error "Platform does not support debugbreak yet!"
#endif // UG_PLATFORM_WINDOWS

#ifdef UG_DEBUG
	/**
	 * @def UG_ENABLE_ASSERTS
	 * @brief Defined only in the Debug configuration; gates the `UG_ASSERT` family.
	 * @ingroup group_core
	 */
	#define UG_ENABLE_ASSERTS
#endif // UG_DEBUG

#ifndef UG_DIST
	/**
	 * @def UG_ENABLE_VERIFYS
	 * @brief Defined in every configuration except Dist; gates the `UG_VERIFY` family.
	 * @ingroup group_core
	 *
	 * Verifies outlive asserts: use `UG_VERIFY` for conditions worth checking in Release
	 * builds, and `UG_ASSERT` for checks that may be compiled away outside Debug.
	 */
	#define UG_ENABLE_VERIFYS
#endif

/**
 * @def UG_EXPAND_MACRO
 * @brief Forces an extra expansion pass over @p x.
 * @param x Token sequence to expand.
 * @ingroup group_core
 *
 * MSVC's traditional preprocessor treats `__VA_ARGS__` as a single token when it is
 * forwarded to another macro, which breaks argument counting. Wrapping the forwarding
 * call in this macro restores the standard behaviour without needing `/Zc:preprocessor`.
 */
#define UG_EXPAND_MACRO(x) x

/**
 * @def UG_STRINGIFY_MACRO
 * @brief Turns @p x into a string literal.
 * @param x Token sequence to stringify.
 * @ingroup group_core
 */
#define UG_STRINGIFY_MACRO(x) #x

/**
 * @def UG_INTERNAL_CHECK_IMPL
 * @brief Shared body of every assertion and verification macro.
 * @param type Logger infix: `_` for the client logger, `_CORE_` for the engine logger.
 * @param kind Human-readable label, either `Assertion` or `Verification`.
 * @param check Condition that must hold.
 * @param detail Extra text appended to the failure message; may be empty.
 * @ingroup group_core
 * @note Implementation detail — call #UG_ASSERT or #UG_VERIFY instead.
 */
#define UG_INTERNAL_CHECK_IMPL(type, kind, check, detail)                          \
	{                                                                              \
		if (!(check))                                                              \
		{                                                                          \
			UG##type##ERROR("{0} failed: {1}\n    at {2}:{3}{4}", kind,             \
				UG_STRINGIFY_MACRO(check), __FILE__, __LINE__, detail);            \
			UG_DEBUGBREAK();                                                       \
		}                                                                          \
	}

/**
 * @def UG_INTERNAL_CHECK_WITH_MSG
 * @brief Failure path that formats the caller's message.
 * @param type Logger infix.
 * @param kind Human-readable label.
 * @param check Condition that must hold.
 * @param ... fmt format string and arguments describing the failure.
 * @ingroup group_core
 * @note Implementation detail — call #UG_ASSERT or #UG_VERIFY instead.
 */
#define UG_INTERNAL_CHECK_WITH_MSG(type, kind, check, ...) \
	UG_INTERNAL_CHECK_IMPL(type, kind, check, "\n    " + ::spdlog::fmt_lib::format(__VA_ARGS__))

/**
 * @def UG_INTERNAL_CHECK_NO_MSG
 * @brief Failure path used when the caller supplied no message.
 * @param type Logger infix.
 * @param kind Human-readable label.
 * @param check Condition that must hold.
 * @ingroup group_core
 * @note Implementation detail — call #UG_ASSERT or #UG_VERIFY instead.
 */
#define UG_INTERNAL_CHECK_NO_MSG(type, kind, check) \
	UG_INTERNAL_CHECK_IMPL(type, kind, check, "")

/**
 * @def UG_INTERNAL_CHECK_PICK
 * @brief Yields its seventeenth argument, so a padding list can encode an argument count.
 * @param _1 Placeholder for the caller's first argument.
 * @param _2 Placeholder for the caller's second argument.
 * @param _3 Placeholder for the caller's third argument.
 * @param _4 Placeholder for the caller's fourth argument.
 * @param _5 Placeholder for the caller's fifth argument.
 * @param _6 Placeholder for the caller's sixth argument.
 * @param _7 Placeholder for the caller's seventh argument.
 * @param _8 Placeholder for the caller's eighth argument.
 * @param _9 Placeholder for the caller's ninth argument.
 * @param _10 Placeholder for the caller's tenth argument.
 * @param _11 Placeholder for the caller's eleventh argument.
 * @param _12 Placeholder for the caller's twelfth argument.
 * @param _13 Placeholder for the caller's thirteenth argument.
 * @param _14 Placeholder for the caller's fourteenth argument.
 * @param _15 Placeholder for the caller's fifteenth argument.
 * @param _16 Placeholder for the caller's sixteenth argument.
 * @param N The selected overload.
 * @param ... Remaining padding, discarded.
 * @ingroup group_core
 * @note Implementation detail.
 */
#define UG_INTERNAL_CHECK_PICK(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, N, ...) N

/**
 * @def UG_INTERNAL_CHECK_GET_MACRO
 * @brief Resolves to the with-message or no-message overload based on argument count.
 * @param ... The caller's arguments.
 * @ingroup group_core
 *
 * The padding list is fifteen copies of the with-message overload followed by the
 * no-message one, so only a lone condition lands on #UG_INTERNAL_CHECK_NO_MSG. Format
 * arguments beyond sixteen are not supported.
 *
 * @note Implementation detail.
 */
#define UG_INTERNAL_CHECK_GET_MACRO(...)                                                             \
	UG_EXPAND_MACRO(UG_INTERNAL_CHECK_PICK(__VA_ARGS__,                                              \
		UG_INTERNAL_CHECK_WITH_MSG, UG_INTERNAL_CHECK_WITH_MSG, UG_INTERNAL_CHECK_WITH_MSG,          \
		UG_INTERNAL_CHECK_WITH_MSG, UG_INTERNAL_CHECK_WITH_MSG, UG_INTERNAL_CHECK_WITH_MSG,          \
		UG_INTERNAL_CHECK_WITH_MSG, UG_INTERNAL_CHECK_WITH_MSG, UG_INTERNAL_CHECK_WITH_MSG,          \
		UG_INTERNAL_CHECK_WITH_MSG, UG_INTERNAL_CHECK_WITH_MSG, UG_INTERNAL_CHECK_WITH_MSG,          \
		UG_INTERNAL_CHECK_WITH_MSG, UG_INTERNAL_CHECK_WITH_MSG, UG_INTERNAL_CHECK_WITH_MSG,          \
		UG_INTERNAL_CHECK_NO_MSG, ))

/**
 * @def UG_ASSERT
 * @brief Breaks into the debugger when the condition is false; client-side.
 * @param ... The condition, optionally followed by an fmt format string and arguments.
 * @ingroup group_core
 *
 * Compiled out unless #UG_ENABLE_ASSERTS is defined, which happens only in Debug.
 * Use it for conditions that indicate a programming error; use #UG_VERIFY for checks
 * that must survive into Release.
 *
 * Both forms are supported, and the failure always reports the stringified expression
 * along with the source file and line:
 * @code
 * UG_ASSERT(texture);
 * UG_ASSERT(index < count, "index {0} out of range (count {1})", index, count);
 * @endcode
 *
 * @warning The condition is **not** evaluated outside Debug, so it must not have side
 * effects.
 */

/**
 * @def UG_CORE_ASSERT
 * @brief Breaks into the debugger when the condition is false; engine-side.
 * @param ... The condition, optionally followed by an fmt format string and arguments.
 * @ingroup group_core
 *
 * Identical to #UG_ASSERT but logs through the core logger. Use this inside the engine.
 */

#ifdef UG_ENABLE_ASSERTS
	#define UG_ASSERT(...)      UG_EXPAND_MACRO(UG_INTERNAL_CHECK_GET_MACRO(__VA_ARGS__)(_,      "Assertion", __VA_ARGS__))
	#define UG_CORE_ASSERT(...) UG_EXPAND_MACRO(UG_INTERNAL_CHECK_GET_MACRO(__VA_ARGS__)(_CORE_, "Assertion", __VA_ARGS__))
#else
	#define UG_ASSERT(...)
	#define UG_CORE_ASSERT(...)
#endif // UG_ENABLE_ASSERTS

/**
 * @def UG_VERIFY
 * @brief Breaks into the debugger when the condition is false; client-side, kept in Release.
 * @param ... The condition, optionally followed by an fmt format string and arguments.
 * @ingroup group_core
 *
 * Survives into Release and is removed only in Dist, so it suits checks that are worth
 * paying for in a shipping-adjacent build. Otherwise behaves exactly like #UG_ASSERT.
 */

/**
 * @def UG_CORE_VERIFY
 * @brief Breaks into the debugger when the condition is false; engine-side, kept in Release.
 * @param ... The condition, optionally followed by an fmt format string and arguments.
 * @ingroup group_core
 */

#ifdef UG_ENABLE_VERIFYS
	#define UG_VERIFY(...)      UG_EXPAND_MACRO(UG_INTERNAL_CHECK_GET_MACRO(__VA_ARGS__)(_,      "Verification", __VA_ARGS__))
	#define UG_CORE_VERIFY(...) UG_EXPAND_MACRO(UG_INTERNAL_CHECK_GET_MACRO(__VA_ARGS__)(_CORE_, "Verification", __VA_ARGS__))
#else
	#define UG_VERIFY(...)
	#define UG_CORE_VERIFY(...)
#endif // UG_ENABLE_VERIFYS


/**
 * @def BIT
 * @brief Produces the flag value with only bit @p x set.
 * @ingroup group_core
 *
 * Used to build the bit-flag enumerations, most notably Uge::EventCategory.
 */
#define BIT(x) (1 << x)

//#define UG_BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)


/**
 * @def UG_BIND_EVENT_FN
 * @brief Wraps a member function of `this` in a forwarding lambda.
 * @ingroup group_core
 *
 * Used to hand member functions to Uge::EventDispatcher::Dispatch and to
 * Uge::Window::SetEventCallback, for example
 * `dispatcher.Dispatch<WindowCloseEvent>(UG_BIND_EVENT_FN(Application::OnWindowClose))`.
 *
 * @param fn Unqualified or class-qualified member function name.
 */
#define UG_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }


namespace Uge
{

	/**
	 * @brief Exclusive-ownership pointer alias; the engine's spelling of `std::unique_ptr`.
	 * @tparam T Pointee type.
	 * @ingroup group_core
	 */
	template <typename T>
	using Scope = std::unique_ptr<T>;
	/**
	 * @brief Constructs a `T` owned by a Uge::Scope.
	 * @tparam T Type to construct.
	 * @tparam Args Constructor argument types, deduced.
	 * @param args Arguments forwarded to the constructor of `T`.
	 * @return A Uge::Scope owning the new object.
	 * @ingroup group_core
	 */
	template <typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}


	/**
	 * @brief Shared-ownership pointer alias; the engine's spelling of `std::shared_ptr`.
	 * @tparam T Pointee type.
	 * @ingroup group_core
	 *
	 * Assets, renderer resources and scenes are all passed around as `Ref`.
	 */
	template <typename T>
	using Ref = std::shared_ptr<T>;
	/**
	 * @brief Constructs a `T` owned by a Uge::Ref.
	 * @tparam T Type to construct.
	 * @tparam Args Constructor argument types, deduced.
	 * @param args Arguments forwarded to the constructor of `T`.
	 * @return A Uge::Ref owning the new object.
	 * @ingroup group_core
	 */
	template <typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

}