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
 * @def UG_ASSERT
 * @brief Breaks into the debugger when @p x is false; client-side.
 * @param x Condition that must hold.
 * @param ... Message describing the failure, in fmt syntax.
 * @ingroup group_core
 *
 * Compiled out unless #UG_ENABLE_ASSERTS is defined, which happens only in Debug.
 * Use it for conditions that indicate a programming error; use `UG_VERIFY` for checks
 * that must survive into Release.
 *
 * @warning The argument expression is **not** evaluated outside Debug, so it must not
 * have side effects.
 */

/**
 * @def UG_CORE_ASSERT
 * @brief Breaks into the debugger when @p x is false; engine-side.
 * @param x Condition that must hold.
 * @param ... Message describing the failure, in fmt syntax.
 * @ingroup group_core
 *
 * Identical to #UG_ASSERT but logs through the core logger. Use this inside the engine.
 */

#ifdef UG_ENABLE_ASSERTS
	#define UG_ASSERT(x) { if (!(x)) { UG_ERROR("Error!"); UG_DEBUGBREAK(); } }
	#define UG_ASSERT(x, ...) { if (!(x)) { UG_ERROR("Assertion Failed: {0}", __VA_ARGS__); UG_DEBUGBREAK(); } }
	#define UG_CORE_ASSERT(x, ...) { if (!(x)) { UG_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); UG_DEBUGBREAK(); } }
	#define UG_CORE_ASSERT(x) { if (!(x)) { UG_CORE_ERROR("Error!"); UG_DEBUGBREAK(); } }
#else
	#define UG_ASSERT(x, ...)
	#define UG_ASSERT(x)
	#define UG_CORE_ASSERT(x, ...)
	#define UG_CORE_ASSERT(x)
#endif // UG_ENABLE_ASSERTS

/**
 * @def UG_VERIFY
 * @brief Breaks into the debugger when @p x is false; client-side, kept in Release.
 * @param x Condition that must hold.
 * @param ... Message describing the failure, in fmt syntax.
 * @ingroup group_core
 *
 * Survives into Release and is removed only in Dist, so it suits checks that are worth
 * paying for in a shipping-adjacent build.
 */

/**
 * @def UG_CORE_VERIFY
 * @brief Breaks into the debugger when @p x is false; engine-side, kept in Release.
 * @param x Condition that must hold.
 * @param ... Message describing the failure, in fmt syntax.
 * @ingroup group_core
 */

#ifdef UG_ENABLE_VERIFYS
#define UG_VERIFY(x) { if (!(x)) { UG_ERROR("Error!"); UG_DEBUGBREAK(); } }
#define UG_VERIFY(x, ...) { if (!(x)) { UG_ERROR("VERIFYion Failed: {0}", __VA_ARGS__); UG_DEBUGBREAK(); } }
#define UG_CORE_VERIFY(x, ...) { if (!(x)) { UG_CORE_ERROR("VERIFYion Failed: {0}", __VA_ARGS__); UG_DEBUGBREAK(); } }
#define UG_CORE_VERIFY(x) { if (!(x)) { UG_CORE_ERROR("Error!"); UG_DEBUGBREAK(); } }
#else
#define UG_VERIFY(x, ...)
#define UG_VERIFY(x)
#define UG_CORE_VERIFY(x, ...)
#define UG_CORE_VERIFY(x)
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