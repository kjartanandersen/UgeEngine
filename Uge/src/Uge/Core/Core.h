#pragma once

#include <memory>


// Platform detection using predefined macros
#ifdef _WIN32
	#ifdef _WIN64
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


#ifdef UG_DEBUG
	#if defined(UG_PLATFORM_WINDOWS)
		#define UG_DEBUGBREAK() __debugbreak()
	#elif defined(UG_PLATFORM_LINUX)
		#include <signal.h>
		#define UG_DEBUGBREAK() raise(SIGTRAP)
	#else
		#error "Platform does not support debugbreak yet!"
	#endif
	#define UG_ENABLE_ASSERTS
#endif // UG_DEBUG


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


#define BIT(x) (1 << x)

//#define UG_BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)
#define UG_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }


namespace Uge
{

	template <typename T>
	using Scope = std::unique_ptr<T>;
	template <typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}


	template <typename T>
	using Ref = std::shared_ptr<T>;
	template <typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

}