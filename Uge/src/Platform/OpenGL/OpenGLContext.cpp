#include <ugpch.h>
#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Uge
{





	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: m_windowHandle(windowHandle)
	{

		UG_CORE_ASSERT(windowHandle, "Window Handle is null!");

	}

	void OpenGLContext::Init()
	{

		glfwMakeContextCurrent(m_windowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		UG_CORE_ASSERT(status, "Failed to initialize GLAD");
		printf("\n");
		UG_CORE_INFO("*********************** OpenGL Info ****************************");
		UG_CORE_INFO("OpenGL Vendor: {0}",   (char*)glGetString(GL_VENDOR));
		UG_CORE_INFO("OpenGL Renderer: {0}", (char*)glGetString(GL_RENDERER));
		UG_CORE_INFO("OpenGL Version: {0}",  (char*)glGetString(GL_VERSION));
		UG_CORE_INFO("");
	}

	void OpenGLContext::SwapBuffers()
	{
		

		glfwSwapBuffers(m_windowHandle);


	}

}



