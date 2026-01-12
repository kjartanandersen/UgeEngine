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
		UG_PROFILE_FUNCTION();

		glfwMakeContextCurrent(m_windowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
#ifdef UG_ENABLE_ASSERTS
		UG_CORE_ASSERT(status, "Failed to initialize GLAD");
#endif
		
		printf("\n");
		UG_CORE_INFO("*********************** OpenGL Info ****************************");
		UG_CORE_INFO("OpenGL Vendor: {0}",   (char*)glGetString(GL_VENDOR));
		UG_CORE_INFO("OpenGL Renderer: {0}", (char*)glGetString(GL_RENDERER));
		UG_CORE_INFO("OpenGL Version: {0}",  (char*)glGetString(GL_VERSION));
		UG_CORE_INFO("");

#ifdef UG_ENABLE_ASSERTS
		int versionMajor;
		int versionMinor;
		glGetIntegerv(GL_MAJOR_VERSION, &versionMajor);
		glGetIntegerv(GL_MINOR_VERSION, &versionMinor);

		UG_CORE_ASSERT(versionMajor > 4 || (versionMajor == 4 && versionMinor >= 5), "Uge requires at least OpenGL Version 4.5");

#endif


	}

	void OpenGLContext::SwapBuffers()
	{
		
		UG_PROFILE_FUNCTION();

		glfwSwapBuffers(m_windowHandle);


	}

}



