/**
 * @file ugpch.h
 * @brief Precompiled header for the engine library.
 * @ingroup group_core
 *
 * Every `.cpp` under `Uge/src/` must begin with `#include <ugpch.h>`. The one exception
 * is `thirdparty/ImGuizmo/**.cpp`, for which premake disables the PCH.
 *
 * It provides the common STL headers, the engine's Log.h, Core.h, Buffer.h and
 * Instrumentor.h, and `<Windows.h>` on Windows — so those never need including again.
 *
 * @warning Because this is compiled as a PCH, it must be the *first* include in a
 * translation unit; MSVC ignores anything preceding it.
 */

#pragma once

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>


#include <string>
#include <sstream>
#include <array>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include <random>
#include <chrono>
#define _USE_MATH_DEFINES
#include <cmath>


#include "Uge/Core/Log.h"
#include "Uge/Core/Core.h"
#include "Uge/Core/Buffer.h"
#include "Uge/Debug/Instrumentor.h"


#ifdef UG_PLATFORM_WINDOWS
	#include <Windows.h>
#endif // UG_PLATFORM_WINDOWS

