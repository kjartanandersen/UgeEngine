/**
 * @file WindowsWindow.h
 * @brief The GLFW window.
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
	 * @brief A GLFW window with an OpenGL context.
	 * @ingroup group_platform
	 *
	 * Turns GLFW callbacks into Uge::Event objects for the event callback.
	 */
	class WindowsWindow : public Window
	{

	public:

		/**
		 * @brief Creates the window and its context, starting GLFW on first use.
		 * @param props Title and size.
		 */
		WindowsWindow(const WindowProps& props);
		/** @brief Destroys the window. GLFW keeps running. */
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
		 * @brief Creates the window and context and sets the GLFW callbacks.
		 * @param props Title and size.
		 */
		virtual void Init(const WindowProps& props);
		/** @brief Destroys the window. */
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

