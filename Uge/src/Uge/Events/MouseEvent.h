/**
 * @file MouseEvent.h
 * @brief Mouse movement, wheel and button events.
 * @ingroup group_events
 */

#pragma once

#include "Event.h"


namespace Uge
{
	/**
	 * @brief The cursor moved.
	 * @ingroup group_events
	 */
	class MouseMovedEvent : public Event
	{

	public:
		/**
		 * @brief Creates the event.
		 * @param x Pixels from the window's left edge.
		 * @param y Pixels from the window's top edge.
		 */
		MouseMovedEvent(float x, float y)
			: m_mouseX(x), m_mouseY(y) {}

		/**
		 * @brief The cursor's x position.
		 * @return Pixels from the window's left edge.
		 */
		inline float GetX() const { return m_mouseX; };
		/**
		 * @brief The cursor's y position.
		 * @return Pixels from the window's top edge.
		 */
		inline float GetY() const { return m_mouseY; };

		/**
		 * @brief A description for logging.
		 * @return The cursor position.
		 */
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: ( x: " << m_mouseX << ", y: " << m_mouseY << " )";
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		float m_mouseX;
		float m_mouseY;
	};


	/**
	 * @brief The mouse wheel scrolled.
	 * @ingroup group_events
	 */
	class MouseScrolledEvent : public Event
	{

	public:
		/**
		 * @brief Creates the event.
		 * @param xOffset Horizontal scroll since the last event.
		 * @param yOffset Vertical scroll since the last event; positive is up.
		 */
		MouseScrolledEvent(float xOffset, float yOffset)
			: m_xOffset(xOffset), m_yOffset(yOffset) {}

		/**
		 * @brief Horizontal scroll since the last event.
		 * @return The offset.
		 */
		inline float GetXOffset() const { return m_xOffset; };
		/**
		 * @brief Vertical scroll since the last event.
		 * @return The offset; positive is up.
		 */
		inline float GetYOffset() const { return m_yOffset; };

		/**
		 * @brief A description for logging.
		 * @return Both offsets.
		 */
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: ( X Offset: " << m_xOffset << ", Y Offset: " << m_yOffset << " )";
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		float m_xOffset;
		float m_yOffset;
	};


	/**
	 * @brief Base class of the mouse button events.
	 * @ingroup group_events
	 */
	class MouseButtonEvent : public Event
	{

	public:

		/**
		 * @brief The button.
		 * @return A `UG_MOUSE_BUTTON_*` code.
		 */
		inline int GetMouseButton() const { return m_button; };


		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	protected:
		/**
		 * @brief Creates the event.
		 * @param button Button code.
		 */
		MouseButtonEvent(int button)
			: m_button(button) { }

		int m_button; ///< Button code.
	};



	/**
	 * @brief A mouse button went down.
	 * @ingroup group_events
	 */
	class MouseButtonPressedEvent : public MouseButtonEvent
	{

	public:
		/**
		 * @brief Creates the event.
		 * @param button Button code.
		 */
		MouseButtonPressedEvent(int button)
			: MouseButtonEvent(button)  {}

		/**
		 * @brief A description for logging.
		 * @return The button code.
		 */
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << m_button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
	};



	/**
	 * @brief A mouse button went up.
	 * @ingroup group_events
	 */
	class MouseButtonReleasedEvent : public MouseButtonEvent
	{

	public:
		/**
		 * @brief Creates the event.
		 * @param button Button code.
		 */
		MouseButtonReleasedEvent(int button)
			: MouseButtonEvent(button) {}

		/**
		 * @brief A description for logging.
		 * @return The button code.
		 */
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << m_button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)
	};




}
