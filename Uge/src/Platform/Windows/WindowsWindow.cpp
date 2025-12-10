#include "ugpch.h"
#include "WindowsWindow.h"



namespace Uge
{


	static bool s_glfwInitialized = false;

	Window* Window::Create(const WindowProps& props)
	{

		return new WindowsWindow(props);

	}

	WindowsWindow::WindowsWindow(const WindowProps& props)
	{

		Init(props);

	}

	WindowsWindow::~WindowsWindow() 
	{ 
		Shutdown();
	}

	void WindowsWindow::Init(const WindowProps& props)
	{

		m_data.m_title = props.m_title;
		m_data.m_width = props.m_width;
		m_data.m_height = props.m_height;

		UG_CORE_INFO("Creating Window {0} (Width: {1}, Height: {2})", 
					  props.m_title, props.m_width, props.m_height);


		if (!s_glfwInitialized)
		{

			// TODO: GLFW Terminate on system shutdown
			int success = glfwInit();
			UG_CORE_ASSERT(success, "Could not initialize GLFW!");

			s_glfwInitialized = true;

		}

		m_window = glfwCreateWindow((int)props.m_width, (int)props.m_height,
			m_data.m_title.c_str(), nullptr, nullptr);

		glfwMakeContextCurrent(m_window);
		glfwSetWindowUserPointer(m_window, &m_data);
		SetVSync(true);


	}

	void WindowsWindow::Shutdown()
	{
		glfwDestroyWindow(m_window);


	}

	void WindowsWindow::OnUpdate()
	{

		glfwPollEvents();
		glfwSwapBuffers(m_window);

	}

	void WindowsWindow::SetVSync(bool enabled)
	{


		if (enabled)
		{

			glfwSwapInterval(1);

		}
		else
		{

			glfwSwapInterval(0);

		}

		m_data.m_vSync = enabled;


	}

	bool WindowsWindow::IsVSync() const
	{
		return m_data.m_vSync;
	}


}