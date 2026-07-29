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
	 * @brief An OpenGL context bound to a GLFW window.
	 * @ingroup group_platform
	 *
	 * Created by Uge::WindowsWindow. Init() makes the context current and loads the
	 * function pointers through GLAD, which must happen before any other GL call.
	 */
	class OpenGLContext : public GraphicsContext
	{

	public:
		/**
		 * @brief Binds the context to a window.
		 * @param windowHandle GLFW window to render into; borrowed, not owned.
		 */
		OpenGLContext(GLFWwindow* windowHandle);

		virtual void Init() override;
		virtual void SwapBuffers() override;

	private:
		GLFWwindow* m_windowHandle;





	};




}

