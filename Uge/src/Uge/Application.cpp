#include "ugpch.h"
#include "Application.h"


#include "Uge/Log.h"

#include <GLFW/glfw3.h>

namespace Uge
{

#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

	Application::Application()
	{
		m_window = std::unique_ptr<Window>(Window::Create());
		m_window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
	}

	Application::~Application()
	{

	}

	void Application::Run()
	{

		auto start_time = std::chrono::high_resolution_clock::now();

		unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
		std::default_random_engine generator(seed);

		std::uniform_real_distribution<float> distribution(0.0f, 1.0f);

		float r = distribution(generator);
		float b = distribution(generator);
		float g = distribution(generator);

		while (m_running)
		{
			auto current_time = std::chrono::high_resolution_clock::now();

			auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time);

			if (elapsed_time.count() >= 1)
			{

				r = distribution(generator);
				g = distribution(generator);
				b = distribution(generator);

				start_time = std::chrono::high_resolution_clock::now();
			}


			glClearColor(r, g, b, 1);
			glClear(GL_COLOR_BUFFER_BIT);
			m_window->OnUpdate();

		}
	}

	void Application::OnEvent(Event& e)
	{

		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));

		UG_CORE_TRACE("{0}", e.ToString());


	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		
		m_running = false;



		return true;

	}

}

