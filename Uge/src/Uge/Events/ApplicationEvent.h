/**
 * @file ApplicationEvent.h
 * @brief Window and application lifecycle events.
 * @ingroup group_events
 */

#pragma once

#include "Event.h"

namespace Uge
{


	/**
	 * @brief Raised when the window's client area changes size.
	 * @ingroup group_events
	 *
	 * Uge::Application handles this before the layers see it: a zero width or height means
	 * the window was minimized, which pauses layer updates, and otherwise the renderer's
	 * viewport is resized. The application deliberately leaves the event unhandled so
	 * layers can react too.
	 */
	class WindowResizeEvent : public Event
	{

	public:
		/**
		 * @brief Constructs the event.
		 * @param width New client-area width in pixels.
		 * @param height New client-area height in pixels.
		 */
		WindowResizeEvent(unsigned int width, unsigned int height)
			: m_width(width), m_height(height) { }


		/**
		 * @brief New width.
		 * @return Width in pixels; `0` when minimized.
		 */
		inline unsigned int GetWidth() const { return m_width; }
		/**
		 * @brief New height.
		 * @return Height in pixels; `0` when minimized.
		 */
		inline unsigned int GetHeight() const { return m_height; }

		/**
		 * @brief Describes the event.
		 * @return A string containing the new dimensions.
		 */
		std::string ToString() const override
		{

			std::stringstream ss;
			ss << "WindowResizeEvent: (X: " << m_width << ", Y: " << m_height << ")";
			return ss.str();

		}

		EVENT_CLASS_TYPE(WindowResize)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	private:
		unsigned int m_width;
		unsigned int m_height;

	};



	/**
	 * @brief Raised when the user closes the window.
	 * @ingroup group_events
	 *
	 * Uge::Application handles this by ending the frame loop, so layers do not normally
	 * see it.
	 */
	class WindowCloseEvent : public Event
	{

	public:
		/** @brief Constructs the event; it carries no payload. */
		WindowCloseEvent() {}

		EVENT_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	};



	/**
	 * @brief Fixed-rate application tick.
	 * @ingroup group_events
	 * @note Declared for completeness; nothing raises it yet.
	 */
	class AppTickEvent : public Event
	{

	public:
		/** @brief Constructs the event; it carries no payload. */
		AppTickEvent() {}

		EVENT_CLASS_TYPE(AppTick)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	};



	/**
	 * @brief Application update notification.
	 * @ingroup group_events
	 * @note Declared for completeness; nothing raises it yet. Layers are updated through
	 * Uge::Layer::OnUpdate instead.
	 */
	class AppUpdateEvent : public Event
	{

	public:
		/** @brief Constructs the event; it carries no payload. */
		AppUpdateEvent() {}

		EVENT_CLASS_TYPE(AppUpdate)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	};




	/**
	 * @brief Application render notification.
	 * @ingroup group_events
	 * @note Declared for completeness; nothing raises it yet.
	 */
	class AppRenderEvent : public Event
	{

	public:
		/** @brief Constructs the event; it carries no payload. */
		AppRenderEvent() {}

		EVENT_CLASS_TYPE(AppRender)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	};


}