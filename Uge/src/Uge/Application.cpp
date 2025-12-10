#include "ugpch.h"
#include "Application.h"


#include "Uge/Events/ApplicationEvent.h"
#include "Uge/Log.h"

#include <GLFW/glfw3.h>

namespace Uge
{
	Application::Application()
	{
		m_window = std::unique_ptr<Window>(Window::Create());
	}

	Application::~Application()
	{

	}

	void Application::Run()
	{

		

		while (m_running)
		{
			glClearColor(1, 0, 1, 1);
			glClear(GL_COLOR_BUFFER_BIT);
			m_window->OnUpdate();

		}
	}

}

