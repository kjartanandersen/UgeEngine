/**
 * @file EntryPoint.h
 * @brief Defines `main`. Include it in exactly one client source file.
 * @ingroup group_core
 *
 * The client implements Uge::CreateApplication.
 */

#pragma once

#include "Uge/Core/Core.h"
#include "Uge/Core/Application.h"
#include "Uge/Debug/CrashHandler.h"

#ifdef UG_PLATFORM_WINDOWS

extern Uge::Application* Uge::CreateApplication(ApplicationCommandLineArgs args);

/**
 * @brief The program entry point.
 * @param argc Argument count.
 * @param argv Arguments.
 * @return `0`.
 * @ingroup group_core
 *
 * With profiling on, writes `UgeProfile-Startup.json` and `UgeProfile-Shutdown.json`.
 */
int main(int argc, char** argv)
{

	Uge::Log::Init();

	// After the logger, which the handler reports through.
	Uge::CrashHandler::Init();

	UG_PROFILE_BEGIN_SESSION("Startup", "UgeProfile-Startup.json");
	auto app = Uge::CreateApplication({ argc, argv });
	UG_PROFILE_END_SESSION();

	// No trace session for the frame loop: tracing every frame is too slow. Traces are
	// captured on demand instead.
	app->Run();

	UG_PROFILE_BEGIN_SESSION("Shutdown", "UgeProfile-Shutdown.json");
	delete app;
	UG_PROFILE_END_SESSION();

	Uge::Log::Shutdown();

	return 0;
}

#endif // UG_PLATFORM_WINDOWS


