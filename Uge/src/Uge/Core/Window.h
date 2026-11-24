/**
 * @file Window.h
 * @brief The window interface.
 * @ingroup group_core
 */

#pragma once

#include "ugpch.h"

#include "Uge/Core/Core.h"
#include "Uge/Events/Event.h"

namespace Uge
{

	/**
	 * @brief A window's title and initial size.
	 * @ingroup group_core
	 */
	struct WindowProps
	{

		std::string m_title; ///< Title bar text.
		uint32_t m_width; ///< Width in pixels.
		uint32_t m_height; ///< Height in pixels.

		/**
		 * @brief Creates window properties.
		 * @param title Title bar text.
		 * @param width Width in pixels.
		 * @param height Height in pixels.
		 */
		WindowProps( const std::string& title = "Uge Engine",
				uint32_t width = 1600, uint32_t height = 900)
				: m_title(title), m_width(width), m_height(height)  {}
	};

	/**
	 * @brief A desktop window that owns the graphics context and produces input events.
	 * @ingroup group_core
	 */
	class Window
	{

	public:
		/** @brief Receives every event the window produces. */
		using EventCallbackFn = std::function<void(Event&)>;

		/** @brief Virtual destructor. */
		virtual ~Window() {}

		/**
		 * @brief Polls events and presents the back buffer.
		 *
		 * The event callback runs from inside this call.
		 */
		virtual void OnUpdate() = 0;

		/**
		 * @brief The client-area width.
		 * @return Width in pixels.
		 */
		virtual uint32_t GetWidth() const = 0;
		/**
		 * @brief The client-area height.
		 * @return Height in pixels.
		 */
		virtual uint32_t GetHeight() const = 0;


		/**
		 * @brief Sets the function that receives every window and input event.
		 * @param callback Called from OnUpdate() for each event.
		 */
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		/**
		 * @brief Turns vertical sync on or off.
		 * @param enabled `true` to limit presentation to the display's refresh rate.
		 */
		virtual void SetVSync(bool enabled) = 0;
		/**
		 * @brief Whether vertical sync is on.
		 * @return `true` if it is.
		 */
		virtual bool IsVSync() const = 0;

		/**
		 * @brief The platform window handle.
		 * @return A `GLFWwindow*`, as an opaque pointer.
		 */
		virtual void* GetNativeWindow() const = 0;

		/**
		 * @brief Creates the window for the current platform.
		 * @param props Title and initial size.
		 * @return The window.
		 */
		static Scope<Window> Create(const WindowProps& props = WindowProps());



	};



}