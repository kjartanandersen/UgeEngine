/**
 * @file EntryPoint.h
 * @brief Defines the program `main`; include it in exactly one client translation unit.
 * @ingroup group_core
 *
 * The engine, not the client, owns startup. A client application includes this header
 * once and implements Uge::CreateApplication; `main` then initializes logging, builds
 * the application, runs it to completion and destroys it, wrapping each phase in its own
 * profiling session.
 *
 * @code
 * // UgeEditorApp.cpp
 * #include <Uge.h>
 * #include <Uge/Core/EntryPoint.h>
 *
 * Uge::Application* Uge::CreateApplication(ApplicationCommandLineArgs args)
 * {
 *     ApplicationSpecification spec;
 *     spec.Name = "Uge-Editor";
 *     spec.CommandLineArgs = args;
 *     return new UgeEditor(spec);
 * }
 * @endcode
 *
 * @warning Including this header in more than one translation unit produces duplicate
 * definitions of `main`.
 */

#pragma once

#include "Uge/Core/Core.h"
#include "Uge/Core/Application.h"

#ifdef UG_PLATFORM_WINDOWS

extern Uge::Application* Uge::CreateApplication(ApplicationCommandLineArgs args);

/**
 * @brief Program entry point supplied by the engine.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return `0` on normal shutdown.
 * @ingroup group_core
 *
 * Writes `UgeProfile-Startup.json`, `UgeProfile-Runtime.json` and
 * `UgeProfile-Shutdown.json` next to the executable when profiling is enabled.
 */
int main(int argc, char** argv)
{
	
	// Initialize the logger
	Uge::Log::Init();

	UG_PROFILE_BEGIN_SESSION("Startup", "UgeProfile-Startup.json");
	auto app = Uge::CreateApplication({ argc, argv });
	UG_PROFILE_END_SESSION();
	
	UG_PROFILE_BEGIN_SESSION("Runtime", "UgeProfile-Runtime.json");
	app->Run();
	UG_PROFILE_END_SESSION();
	
	UG_PROFILE_BEGIN_SESSION("Shutdown", "UgeProfile-Shutdown.json");
	delete app;
	UG_PROFILE_END_SESSION();

	return 0;
}

#endif // UG_PLATFORM_WINDOWS


