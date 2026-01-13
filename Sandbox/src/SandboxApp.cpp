#include <Uge.h>
// ************* Entry Point **************
#include "Uge/Core/EntryPoint.h"
// ****************************************

#include "Sandbox2D.h"
#include "Sandbox3D.h"


class Sandbox : public Uge::Application
{
public:
	Sandbox()
		: Application(true)
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

Uge::Application* Uge::CreateApplication()
{
	return new Sandbox();
}