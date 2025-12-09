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


#define BIT(x) (1 << x)
