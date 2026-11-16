/**
 * @file SandboxApp.cpp
 * @brief The Sandbox application.
 */

#include <Uge.h>
// ************* Entry Point **************
#include "Uge/Core/EntryPoint.h"
// ****************************************

#include "Sandbox2D.h"
#include "Sandbox3D.h"


/**
 * @brief The Sandbox application, which runs Sandbox2D.
 */
class Sandbox : public Uge::Application
{
public:
	/**
	 * @brief Creates the application and pushes Sandbox2D.
	 * @param spec The startup settings.
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