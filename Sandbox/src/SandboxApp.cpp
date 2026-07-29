/**
 * @file SandboxApp.cpp
 * @brief Entry point of the Sandbox sample application.
 */

#include <Uge.h>
// ************* Entry Point **************
#include "Uge/Core/EntryPoint.h"
// ****************************************

#include "Sandbox2D.h"
#include "Sandbox3D.h"


/**
 * @brief The Sandbox client application.
 *
 * Pushes the sample layer that exercises the engine's renderers.
 * @see Uge::CreateApplication
 */
class Sandbox : public Uge::Application
{
public:
	/**
	 * @brief Constructs the application and pushes the sample layer.
	 * @param spec Startup configuration from Uge::CreateApplication.
	 */
	Sandbox(Uge::ApplicationSpecification spec)
		: Uge::Application(spec)
	{
		PushLayer( new Sandbox2D());
	}
	/** @brief Destroys the application. */
	~Sandbox()
	{

	}

private:
	
};

Uge::Application* Uge::CreateApplication( Uge::ApplicationCommandLineArgs args)
{
	ApplicationSpecification spec;
	spec.Name = "Sandbox";
	spec.CommandLineArgs = args;

	return new Sandbox(spec);
}