/**
 * @file OpenGLContext.h
 * @brief OpenGL implementation of Uge::GraphicsContext.
 * @ingroup group_platform
 */

#pragma once
#include "Uge/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace Uge
{


	/**
	 * @brief A GLFW window's OpenGL context. Init() loads GL through GLAD.
	 * @ingroup group_platform
	 */
	class OpenGLContext : public GraphicsContext
	{

	public:
		/**
		 * @brief A context for a window. Asserts if the handle is null.
		 * @param windowHandle The window, not owned.
		 */
		OpenGLContext(GLFWwindow* windowHandle);

		virtual void Init() override;
		virtual void SwapBuffers() override;

	private:
		GLFWwindow* m_windowHandle;





	};




}

