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
		#define UG_API __declspec(dllimport)
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

// DLL Support
#ifdef UG_PLATFORM_WINDOWS
	#if UG_DYNAMIC_LINK
		#ifdef UG_BUILD_DLL
			#define UG_API __declspec(dllexport)
		#else
			#define UG_API __declspec(dllimport)
		#endif // UG_BUILD_DLL
	#else
		#define UG_API
	#endif
#else
	#error "Uge only supports Windows!"
	
#endif // UG_PLATFORM_WINDOWS

#ifdef UG_DEBUG
		#define UG_ENABLE_ASSERTS
#endif // UG_DEBUG


#ifdef UG_ENABLE_ASSERTS
	#define UG_ASSERT(x, ...) { if (!(x)) { UG_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define UG_CORE_ASSERT(x, ...) { if (!(x)) { UG_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define UG_ASSERT(x, ...)
	#define UG_CORE_ASSERT(x, ...)
#endif // UG_ENABLE_ASSERTS


#define BIT(x) (1 << x)

#define UG_BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)


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