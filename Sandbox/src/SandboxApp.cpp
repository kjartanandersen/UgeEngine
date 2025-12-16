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

		//UG_INFO("ExampleLayer::Update");

		

		


	}

	void OnEvent(Uge::Event& event) override
	{

		//UG_TRACE("{0}", event.ToString());

		if (event.GetEventType() == Uge::EventType::KeyPressed)
		{

			Uge::KeyPressedEvent& e = static_cast<Uge::KeyPressedEvent&>(event);
			UG_TRACE("{0}", (char)e.GetKeyCode());


		}

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