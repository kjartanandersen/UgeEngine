/**
 * @file Window.h
 * @brief Platform-agnostic window interface and its creation properties.
 * @ingroup group_core
 */

#pragma once

#include "ugpch.h"

#include "Uge/Core/Core.h"
#include "Uge/Events/Event.h"

namespace Uge
{

	/**
	 * @brief Title and initial dimensions used to create a window.
	 * @ingroup group_core
	 */
	struct WindowProps
	{

		std::string m_title; ///< Window title bar text.
		uint32_t m_width; ///< Initial client-area width in pixels.
		uint32_t m_height; ///< Initial client-area height in pixels.

		/**
		 * @brief Constructs window properties.
		 * @param title Window title bar text.
		 * @param width Initial client-area width in pixels.
		 * @param height Initial client-area height in pixels.
		 */
		WindowProps( const std::string& title = "Uge Engine",
				uint32_t width = 1600, uint32_t height = 900) 
				: m_title(title), m_width(width), m_height(height)  {}
	};

	/**
	 * @brief Abstract desktop window that owns the graphics context and emits input events.
	 * @ingroup group_core
	 *
	 * Create windows with Create(), which returns the backend implementation for the
	 * current platform (Uge::WindowsWindow, a GLFW wrapper). The application owns exactly
	 * one window and drives it once per frame through OnUpdate().
	 *
	 * @see WindowProps, Uge::WindowsWindow
	 */
	class Window
	{

	public:
		/** @brief Signature of the sink that receives every event the window produces. */
		using EventCallbackFn = std::function<void(Event&)>;

		/** @brief Virtual destructor; windows are held by Uge::Scope. */
		virtual ~Window() {}

		/**
		 * @brief Polls platform events and presents the back buffer.
		 *
		 * Called once per frame, last, by Application::Run. Event callbacks fire from inside
		 * this call.
		 */
		virtual void OnUpdate() = 0;

		/**
		 * @brief Current client-area width.
		 * @return Width in pixels.
		 */
		virtual uint32_t GetWidth() const = 0;
		/**
		 * @brief Current client-area height.
		 * @return Height in pixels.
		 */
		virtual uint32_t GetHeight() const = 0;
		

		// Window Attributes
		/**
		 * @brief Installs the sink that receives all window and input events.
		 * @param callback Invoked from inside OnUpdate() for each event produced.
		 *
		 * The application installs Application::OnEvent here during construction.
		 */
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		/**
		 * @brief Enables or disables vertical synchronization.
		 * @param enabled `true` to cap presentation to the display refresh rate.
		 */
		virtual void SetVSync(bool enabled) = 0;
		/**
		 * @brief Reports whether vertical synchronization is enabled.
		 * @return `true` if VSync is on.
		 */
		virtual bool IsVSync() const = 0;

		/**
		 * @brief Returns the underlying platform handle.
		 * @return An opaque pointer; a `GLFWwindow*` in the current backend.
		 *
		 * Needed by ImGui and by the native file dialogs. Cast it only in platform code.
		 */
		virtual void* GetNativeWindow() const = 0;

		/**
		 * @brief Creates the window implementation for the current platform.
		 * @param props Title and initial size.
		 * @return An owning pointer to the new window.
		 */
		static Scope<Window> Create(const WindowProps& props = WindowProps());



	};



}