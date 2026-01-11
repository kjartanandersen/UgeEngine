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
	{
		//PushLayer( new Sandbox2D());
		PushLayer( new Sandbox3D());
		
	}
	~Sandbox()
	{

	}
};

Uge::Application* Uge::CreateApplication()
{
	return new Sandbox();
}