#include "ugpch.h"
#include "Application.h"


#include "Uge/Log.h"
#include "Renderer/Renderer.h"

#include "Uge/Input.h"
#include "Uge/KeyCodes.h"

namespace Uge
{
	
	
	Application* Application::s_instance = nullptr;

	


	Application::Application()
	{
		

		UG_CORE_ASSERT(!s_instance, "Application Already Exists!");
		s_instance = this;

		m_window = std::unique_ptr<Window>(Window::Create());
		m_window->SetEventCallback(UG_BIND_EVENT_FN(Application::OnEvent));

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);


	}

	Application::~Application()
	{

	}

	void Application::Run()
	{

		while (m_running)
		{

			if (Uge::Input::IsKeyPressed(UG_KEY_ESCAPE))
				CloseProgram();

			for (auto layer : m_layerStack)
				layer->OnUpdate();

			m_ImGuiLayer->Begin();

			for (auto layer : m_layerStack)
				layer->OnImGuiRender();

			m_ImGuiLayer->End();

			m_window->OnUpdate();

		}
	}

	void Application::OnEvent(Event& e)
	{

		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<WindowCloseEvent>(UG_BIND_EVENT_FN(Application::OnWindowClose));

		for ( auto it = m_layerStack.end(); it != m_layerStack.begin(); )
		{

			(*--it)->OnEvent(e);
			if (e.m_handled)
				break;


		}


	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		
		m_running = false;



		return true;

	}

	void Application::PushLayer(Layer* layer)
	{

		m_layerStack.PushLayer(layer);
		layer->OnAttach();


	}

	void Application::PushOverlay(Layer* overlay)
	{

		m_layerStack.PushOverlay(overlay);
		overlay->OnAttach();


	}

}

