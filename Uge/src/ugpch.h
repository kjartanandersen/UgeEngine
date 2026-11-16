/**
 * @file ugpch.h
 * @brief The engine's precompiled header.
 * @ingroup group_core
 *
 * Every `.cpp` under `Uge/src/` includes it first, except `thirdparty/ImGuizmo/**.cpp`.
 * It has the common STL headers, Log.h, Core.h, Buffer.h, Instrumentor.h and
 * `<Windows.h>`.
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

