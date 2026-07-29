/**
 * @file GraphicsContext.h
 * @brief Abstract graphics context owned by the platform window.
 * @ingroup group_renderer
 */

#pragma once



namespace Uge
{

	/**
	 * @brief The rendering context bound to a window's surface.
	 * @ingroup group_renderer
	 *
	 * Created and owned by the platform window, not by the renderer: Uge::WindowsWindow
	 * builds an Uge::OpenGLContext for its `GLFWwindow` and calls SwapBuffers() once per
	 * frame from Uge::Window::OnUpdate.
	 */
	class GraphicsContext
	{

	public:
		/**
		 * @brief Makes the context current and loads the API entry points.
		 *
		 * Must complete before any other rendering call.
		 */
		virtual void Init() = 0;
		/** @brief Presents the back buffer to the window. */
		virtual void SwapBuffers() = 0;


	};



}
