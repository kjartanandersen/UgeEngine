#pragma once

#ifdef UG_PLATFORM_WINDOWS
	#ifdef UG_BUILD_DLL
		#define UG_API __declspec(dllexport)
	#else
		#define UG_API __declspec(dllimport)
	#endif // UG_BUILD_DLL
#else
	#error Uge only supports Windows!
#endif // UG_PLATFORM_WINDOWS

#ifdef UG_ENABLE_ASSERTS
	#define UG_ASSERT(x, ...) { if (!(x)) { UG_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define UG_CORE_ASSERT(x, ...) { if (!(x)) { UG_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define UG_ASSERT(x, ...)
	#define UG_CORE_ASSERT(x, ...)
#endif // UG_ENABLE_ASSERTS


#define BIT(x) (1 << x)
