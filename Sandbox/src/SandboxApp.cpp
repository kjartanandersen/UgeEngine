#include <Uge.h>

class ExampleLayer : public Uge::Layer
{

public:
	ExampleLayer()
		: Layer("Example")
	{



	}

	void OnUpdate() override
	{

		UG_INFO("ExampleLayer::Update");


	}

	void OnEvent(Uge::Event& event) override
	{

		UG_TRACE("{0}", event.ToString());

	}


};


class Sandbox : public Uge::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
		PushOverlay(new Uge::ImGuiLayer());
	}
	~Sandbox()
	{

	}
};

Uge::Application* Uge::CreateApplication()
{
	return new Sandbox();
}