#include <Uge.h>

class Sandbox : public Uge::Application
{
public:
	Sandbox()
	{

	}
	~Sandbox()
	{

	}
};

Uge::Application* Uge::CreateApplication()
{
	return new Sandbox();
}