#include "ugpch.h"
#include "Application.h"


#include "Uge/Core/Log.h"
#include "Uge/Renderer/Renderer.h"
#include "Uge/Scripting/ScriptEngine.h"

#include "Uge/Core/Input.h"
#include "Uge/Core/KeyCodes.h"

#include <GLFW/glfw3.h>

namespace Uge
{
	
	
	Application* Application::s_instance = nullptr;

	Application::Application(const ApplicationSpecification& spec)
		: m_specification(spec)
	{
		UG_PROFILE_FUNCTION();
		UG_CORE_ASSERT(!s_instance, "Application Already Exists!");
		s_instance = this;

		if (!m_specification.WorkingDirectory.empty())
		{
			std::filesystem::current_path(m_specification.WorkingDirectory);
		}

		m_window = Window::Create(WindowProps(m_specification.Name));
		m_window->SetEventCallback(UG_BIND_EVENT_FN(Application::OnEvent));

		m_window->SetVSync(true);

		Renderer::Init();

		ScriptEngine::Init();

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);


	}

	Application::~Application()
	{

		UG_PROFILE_FUNCTION();

		ScriptEngine::Shutdown();
		Renderer::Shutdown();

	}

	void Application::Run()
	{
		UG_PROFILE_FUNCTION();

		while (m_running)
		{
			UG_PROFILE_SCOPE("RunLoop");

			float time = (float)glfwGetTime();			// Platform::GetTime
			Timestep timestep = time - m_lastFrameTime;
			m_lastFrameTime = time;

			ExecuteMainThreadQueue();

			if (!m_minimized)
			{
				{
					UG_PROFILE_SCOPE("LayerStack OnUpdate");
					for (auto layer : m_layerStack)
						layer->OnUpdate(timestep);


				}

				

			}
			m_ImGuiLayer->Begin();
			{
				UG_PROFILE_SCOPE("LayerStack OnImGuiRender");

				for (auto layer : m_layerStack)
					layer->OnImGuiRender();

			}

			m_ImGuiLayer->End();
			

			m_window->OnUpdate();

		}
	}

	void Application::OnEvent(Event& e)
	{
		UG_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<WindowCloseEvent>(UG_BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(UG_BIND_EVENT_FN(Application::OnWindowResize));

		for ( auto it = m_layerStack.end(); it != m_layerStack.begin(); )
		{

			if (e.m_handled)
				break;

			(*--it)->OnEvent(e);


		}


	}

	void Application::SubmitToMainThreadQueue(const std::function<void()> func)
	{
		std::scoped_lock<std::mutex> lock(m_mainThreadQueueMutex);

		m_mainThreadQueue.emplace_back(func);
		
		
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		
		m_running = false;



		return true;

	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		UG_PROFILE_FUNCTION();


		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_minimized = true;
			return false;

		}

		m_minimized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());
		return false;
	}

	void Application::ExecuteMainThreadQueue()
	{

		std::scoped_lock<std::mutex> lock(m_mainThreadQueueMutex);

		for (auto& func : m_mainThreadQueue)
		{
			func();
		}

		m_mainThreadQueue.clear();

	}

	void Application::PushLayer(Layer* layer)
	{
		UG_PROFILE_FUNCTION();

		m_layerStack.PushLayer(layer);
		layer->OnAttach();


	}

	void Application::PushOverlay(Layer* overlay)
	{
		UG_PROFILE_FUNCTION();

		m_layerStack.PushOverlay(overlay);
		overlay->OnAttach();


	}

}

