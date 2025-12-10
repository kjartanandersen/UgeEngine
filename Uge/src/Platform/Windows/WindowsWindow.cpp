#include "ugpch.h"
#include "WindowsWindow.h"


#include "Uge/Events/ApplicationEvent.h"
#include "Uge/Events/MouseEvent.h"
#include "Uge/Events/KeyEvent.h"



namespace Uge
{


	static bool s_glfwInitialized = false;

	static void GLFWErrorCallback(int error , const char* description)
	{

		UG_CORE_ERROR("GLFW Error ({0}): {1}", error, description);

	}

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

			glfwSetErrorCallback(GLFWErrorCallback);

			s_glfwInitialized = true;

		}

		m_window = glfwCreateWindow((int)props.m_width, (int)props.m_height,
			m_data.m_title.c_str(), nullptr, nullptr);

		glfwMakeContextCurrent(m_window);
		glfwSetWindowUserPointer(m_window, &m_data);
		SetVSync(true);


		// Set GLFW Callbacks
		glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height)
		{
				
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				data.m_width = width;
				data.m_height = height;

				WindowResizeEvent event(width, height);
				data.m_eventCallback(event);


		});

		glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window) 
		{

				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				WindowCloseEvent event;

				data.m_eventCallback(event);

		});

		glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {

			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{

				case GLFW_PRESS:

				{

					KeyPressedEvent event(key, 0);
					data.m_eventCallback(event);

					break;
				}



				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(key);
					data.m_eventCallback(event);

					break;
				}

				case GLFW_REPEAT:
				{
					KeyPressedEvent event(key, 1);
					data.m_eventCallback(event);

					break;
				}


				default:
					break;
			}

		});

		glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods)
		{

				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);


				switch (action)
				{

					case GLFW_PRESS:
					{

						MouseButtonReleasedEvent event(button);
						data.m_eventCallback(event);

						break;

					}
					case GLFW_RELEASE:
					{



					}



						default:
							break;
				}



		});

		glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xoffset, double yoffset)
		{

				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				MouseScrolledEvent event((float)xoffset, (float)yoffset);

				data.m_eventCallback(event);



		});

		glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xpos, double ypos)
		{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				MouseMovedEvent event((float) xpos, (float) ypos);

				data.m_eventCallback(event);


		});


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