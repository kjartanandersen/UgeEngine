#include <Uge.h>
// ************* Entry Point **************
#include "Uge/Core/EntryPoint.h"
// ****************************************

#include "Sandbox2D.h"
#include "Sandbox3D.h"


class Sandbox : public Uge::Application
{
public:
	Sandbox(Uge::ApplicationSpecification spec)
		: Uge::Application(spec)
	{
		PushLayer( new Sandbox2D());
	}
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