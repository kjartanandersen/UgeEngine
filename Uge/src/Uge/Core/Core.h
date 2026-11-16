/**
 * @file Core.h
 * @brief Platform detection, asserts and the Ref and Scope pointer aliases.
 * @ingroup group_core
 */

#pragma once

#include <memory>


// Platform detection
#ifdef _WIN32
	#ifdef _WIN64
		/**
		 * @def UG_PLATFORM_WINDOWS
		 * @brief Defined for 64-bit Windows, the only supported platform.
		 * @ingroup group_core
		 *
		 * Defined only here, so a file that tests it must include this header or the
		 * precompiled header first.
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
	 * @brief Breaks into the debugger. Without one attached, the process ends.
	 * @ingroup group_core
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
	 * @brief Defined in Debug only. Enables the `UG_ASSERT` macros.
	 * @ingroup group_core
	 */
	#define UG_ENABLE_ASSERTS
#endif // UG_DEBUG

#ifndef UG_DIST
	/**
	 * @def UG_ENABLE_VERIFYS
	 * @brief Defined in every configuration except Dist. Enables the `UG_VERIFY` macros.
	 * @ingroup group_core
	 */
	#define UG_ENABLE_VERIFYS
#endif

/**
 * @def UG_EXPAND_MACRO
 * @brief Expands @p x once more.
 * @param x Tokens to expand.
 * @ingroup group_core
 *
 * MSVC's traditional preprocessor passes a forwarded `__VA_ARGS__` on as one token; the
 * extra pass splits it again.
 */
#define UG_EXPAND_MACRO(x) x

/**
 * @def UG_STRINGIFY_MACRO
 * @brief Turns @p x into a string literal.
 * @param x Tokens to stringify.
 * @ingroup group_core
 */
#define UG_STRINGIFY_MACRO(x) #x

/**
 * @def UG_INTERNAL_CHECK_IMPL
 * @brief Body of the assert and verify macros.
 * @param type Logger infix: `_` for the client logger, `_CORE_` for the engine's.
 * @param kind `Assertion` or `Verification`.
 * @param check Condition that must hold.
 * @param detail Text appended to the failure message; may be empty.
 * @ingroup group_core
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
 * @brief Check with a formatted message.
 * @param type Logger infix.
 * @param kind `Assertion` or `Verification`.
 * @param check Condition that must hold.
 * @param ... fmt format string and arguments.
 * @ingroup group_core
 */
#define UG_INTERNAL_CHECK_WITH_MSG(type, kind, check, ...) \
	UG_INTERNAL_CHECK_IMPL(type, kind, check, "\n    " + ::spdlog::fmt_lib::format(__VA_ARGS__))

/**
 * @def UG_INTERNAL_CHECK_NO_MSG
 * @brief Check without a message.
 * @param type Logger infix.
 * @param kind `Assertion` or `Verification`.
 * @param check Condition that must hold.
 * @ingroup group_core
 */
#define UG_INTERNAL_CHECK_NO_MSG(type, kind, check) \
	UG_INTERNAL_CHECK_IMPL(type, kind, check, "")

/**
 * @def UG_INTERNAL_CHECK_PICK
 * @brief Yields its seventeenth argument, which selects a check by argument count.
 * @param _1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16 The caller's arguments,
 * then padding.
 * @param N The selected check.
 * @param ... Remaining padding.
 * @ingroup group_core
 */
#define UG_INTERNAL_CHECK_PICK(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, N, ...) N

/**
 * @def UG_INTERNAL_CHECK_GET_MACRO
 * @brief Picks #UG_INTERNAL_CHECK_NO_MSG for a lone condition, #UG_INTERNAL_CHECK_WITH_MSG otherwise.
 * @param ... The caller's arguments, at most 16 including the condition.
 * @ingroup group_core
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
 * @brief Logs and breaks into the debugger when the condition is false. Debug only.
 * @param ... The condition, optionally followed by an fmt format string and arguments.
 * @ingroup group_core
 *
 * Outside Debug the condition is not evaluated, so it must have no side effects.
 */

/**
 * @def UG_CORE_ASSERT
 * @brief #UG_ASSERT for engine code, logging through the core logger.
 * @param ... The condition, optionally followed by an fmt format string and arguments.
 * @ingroup group_core
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
 * @brief Like #UG_ASSERT, but kept in Release and removed only in Dist.
 * @param ... The condition, optionally followed by an fmt format string and arguments.
 * @ingroup group_core
 */

/**
 * @def UG_CORE_VERIFY
 * @brief #UG_VERIFY for engine code, logging through the core logger.
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
 * @brief The value with only bit @p x set.
 * @ingroup group_core
 */
#define BIT(x) (1 << x)

//#define UG_BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)


/**
 * @def UG_BIND_EVENT_FN
 * @brief Wraps a member function of `this` in a forwarding lambda.
 * @ingroup group_core
 * @param fn Member function name, qualified or not.
 */
#define UG_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }


namespace Uge
{

	/**
	 * @brief The engine's name for `std::unique_ptr`.
	 * @tparam T Pointee type.
	 * @ingroup group_core
	 */
	template <typename T>
	using Scope = std::unique_ptr<T>;
	/**
	 * @brief Creates a `T` owned by a Uge::Scope.
	 * @tparam T Type to create.
	 * @tparam Args Constructor argument types.
	 * @param args Constructor arguments.
	 * @return The new object.
	 * @ingroup group_core
	 */
	template <typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}


	/**
	 * @brief The engine's name for `std::shared_ptr`.
	 * @tparam T Pointee type.
	 * @ingroup group_core
	 */
	template <typename T>
	using Ref = std::shared_ptr<T>;
	/**
	 * @brief Creates a `T` owned by a Uge::Ref.
	 * @tparam T Type to create.
	 * @tparam Args Constructor argument types.
	 * @param args Constructor arguments.
	 * @return The new object.
	 * @ingroup group_core
	 */
	template <typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

}