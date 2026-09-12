/**
 * @file WindowsWindow.h
 * @brief GLFW-backed implementation of Uge::Window.
 * @ingroup group_platform
 */

#pragma once

#include "Uge/Core/Window.h"
#include "Uge/Renderer/GraphicsContext.h"

#include "GLFW/glfw3.h"
#include <Uge/Core/Core.h>


namespace Uge
{

	/**
	 * @brief The desktop window, its graphics context, and the source of every input event.
	 * @ingroup group_platform
	 *
	 * Wraps a `GLFWwindow` and owns the Uge::OpenGLContext rendering into it. During Init()
	 * it installs GLFW callbacks that translate platform events into Uge::Event objects and
	 * hand them to the callback set by Uge::Window::SetEventCallback — which is how input
	 * reaches the layer stack.
	 *
	 * A `WindowData` struct is stored as the GLFW user pointer so the static callbacks can
	 * reach the event sink and the cached window size.
	 */
	class WindowsWindow : public Window
	{

	public:

		/**
		 * @brief Creates the window and its graphics context.
		 * @param props Title and initial dimensions.
		 */
		WindowsWindow(const WindowProps& props);
		/** @brief Destroys the window and shuts GLFW down. */
		virtual ~WindowsWindow();

		void OnUpdate() override;

		inline unsigned int GetWidth() const override { return m_data.m_width; };
		inline unsigned int GetHeight() const override { return m_data.m_height; };

		// Window Attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) override 
		{ 
			m_data.m_eventCallback = callback;
		};

		virtual void SetVSync(bool enabled) override;
		virtual bool IsVSync() const override;

		inline virtual void* GetNativeWindow() const { return m_window; }

	private:
		/**
		 * @brief Creates the GLFW window, the context, and installs the event callbacks.
		 * @param props Title and initial dimensions.
		 */
		virtual void Init(const WindowProps& props);
		/** @brief Destroys the GLFW window. */
		virtual void Shutdown();

	private:
		GLFWwindow* m_window;
		Scope<GraphicsContext> m_context;

		struct WindowData
		{

			std::string m_title;
			unsigned int m_width, m_height;
			bool m_vSync;

			EventCallbackFn m_eventCallback;

		};

		WindowData m_data;
		

	};



}

