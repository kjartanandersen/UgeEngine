#include <Uge.h>
// ************* Entry Point **************
#include "Uge/Core/EntryPoint.h"
// ****************************************

#include "Sandbox2D.h"
#include "Sandbox3D.h"


class Sandbox : public Uge::Application
{
public:
	Sandbox(Uge::ApplicationCommandLineArgs args)
		: Uge::Application(false, "Sandbox App", args)
	{
		if (m_is3D)
			PushLayer( new Sandbox3D());
		else
			PushLayer( new Sandbox2D());
	}
	~Sandbox()
	{

	}

private:
	
};

Uge::Application* Uge::CreateApplication(Uge::ApplicationCommandLineArgs args)
{
	return new Sandbox(args);
}