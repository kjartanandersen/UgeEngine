#include "ugpch.h"
#include "Application.h"


#include "Uge/Log.h"
#include "Renderer/Renderer.h"

#include "Uge/Input.h"
#include "Uge/KeyCodes.h"

#include <GLFW/glfw3.h>

namespace Uge
{
	
	
	Application* Application::s_instance = nullptr;

	


	Application::Application()
	{
		

		UG_CORE_ASSERT(!s_instance, "Application Already Exists!");
		s_instance = this;

		m_window = Scope<Window>(Window::Create());
		m_window->SetEventCallback(UG_BIND_EVENT_FN(Application::OnEvent));

		m_window->SetVSync(true);

		Renderer::Init();

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

			float time = (float)glfwGetTime();			// Platform::GetTime
			Timestep timestep = time - m_lastFrameTime;
			m_lastFrameTime = time;




			if (Uge::Input::IsKeyPressed(UG_KEY_ESCAPE))
				CloseProgram();

			if (!m_minimized)
			{

				for (auto layer : m_layerStack)
					layer->OnUpdate(timestep);

				

			}
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
		dispatcher.Dispatch<WindowResizeEvent>(UG_BIND_EVENT_FN(Application::OnWindowResize));

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

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{

		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_minimized = true;
			return false;

		}

		m_minimized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());
		return false;
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

