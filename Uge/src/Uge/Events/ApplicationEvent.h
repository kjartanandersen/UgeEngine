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
	 * @brief The window's client area changed size.
	 * @ingroup group_events
	 */
	class WindowResizeEvent : public Event
	{

	public:
		/**
		 * @brief Creates the event.
		 * @param width New width in pixels.
		 * @param height New height in pixels.
		 */
		WindowResizeEvent(unsigned int width, unsigned int height)
			: m_width(width), m_height(height) { }


		/**
		 * @brief The new width.
		 * @return Pixels; `0` when minimized.
		 */
		inline unsigned int GetWidth() const { return m_width; }
		/**
		 * @brief The new height.
		 * @return Pixels; `0` when minimized.
		 */
		inline unsigned int GetHeight() const { return m_height; }

		/**
		 * @brief A description for logging.
		 * @return The new size.
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
	 * @brief The window was closed.
	 * @ingroup group_events
	 */
	class WindowCloseEvent : public Event
	{

	public:
		/** @brief Creates the event. */
		WindowCloseEvent() {}

		EVENT_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	};



	/**
	 * @brief A fixed-rate tick. Not raised yet.
	 * @ingroup group_events
	 */
	class AppTickEvent : public Event
	{

	public:
		/** @brief Creates the event. */
		AppTickEvent() {}

		EVENT_CLASS_TYPE(AppTick)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	};



	/**
	 * @brief An update. Not raised yet.
	 * @ingroup group_events
	 */
	class AppUpdateEvent : public Event
	{

	public:
		/** @brief Creates the event. */
		AppUpdateEvent() {}

		EVENT_CLASS_TYPE(AppUpdate)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	};




	/**
	 * @brief A render. Not raised yet.
	 * @ingroup group_events
	 */
	class AppRenderEvent : public Event
	{

	public:
		/** @brief Creates the event. */
		AppRenderEvent() {}

		EVENT_CLASS_TYPE(AppRender)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	};


}