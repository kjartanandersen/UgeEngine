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
	 * @brief Raised when the cursor moves over the window.
	 * @ingroup group_events
	 *
	 * Coordinates are in pixels relative to the window's top-left corner, with y growing
	 * downward.
	 */
	class MouseMovedEvent : public Event
	{

	public:
		/**
		 * @brief Constructs the event.
		 * @param x Cursor x position in pixels.
		 * @param y Cursor y position in pixels.
		 */
		MouseMovedEvent(float x, float y)
			: m_mouseX(x), m_mouseY(y) {}

		/**
		 * @brief Cursor x position.
		 * @return Pixels from the left edge of the window.
		 */
		inline float GetX() const { return m_mouseX; };
		/**
		 * @brief Cursor y position.
		 * @return Pixels from the top edge of the window.
		 */
		inline float GetY() const { return m_mouseY; };

		/**
		 * @brief Describes the event.
		 * @return A string with the cursor position.
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
	 * @brief Raised when the mouse wheel is scrolled.
	 * @ingroup group_events
	 *
	 * Offsets are deltas since the last event, not absolute positions. Most mice only
	 * report the vertical axis.
	 */
	class MouseScrolledEvent : public Event
	{

	public:
		/**
		 * @brief Constructs the event.
		 * @param xOffset Horizontal scroll delta.
		 * @param yOffset Vertical scroll delta; positive is scrolling up.
		 */
		MouseScrolledEvent(float xOffset, float yOffset)
			: m_xOffset(xOffset), m_yOffset(yOffset) {}

		/**
		 * @brief Horizontal scroll delta.
		 * @return Units scrolled since the previous event.
		 */
		inline float GetXOffset() const { return m_xOffset; };
		/**
		 * @brief Vertical scroll delta.
		 * @return Units scrolled since the previous event; positive is up.
		 */
		inline float GetYOffset() const { return m_yOffset; };

		/**
		 * @brief Describes the event.
		 * @return A string with both scroll offsets.
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
	 * @brief Common base for mouse button events.
	 * @ingroup group_events
	 *
	 * Abstract in practice: the constructor is protected. Dispatch against
	 * Uge::MouseButtonPressedEvent or Uge::MouseButtonReleasedEvent instead.
	 */
	class MouseButtonEvent : public Event
	{

	public:

		/**
		 * @brief The button involved.
		 * @return A `UG_MOUSE_BUTTON_*` value from MouseButtonCodes.h.
		 */
		inline int GetMouseButton() const { return m_button; };


		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	protected:
		/**
		 * @brief Constructs the base event.
		 * @param button Button code from MouseButtonCodes.h.
		 */
		MouseButtonEvent(int button)
			: m_button(button) { }

		int m_button; ///< Button code from MouseButtonCodes.h.
	};



	/**
	 * @brief Raised when a mouse button goes down.
	 * @ingroup group_events
	 */
	class MouseButtonPressedEvent : public MouseButtonEvent
	{

	public:
		/**
		 * @brief Constructs the event.
		 * @param button Button code from MouseButtonCodes.h.
		 */
		MouseButtonPressedEvent(int button)
			: MouseButtonEvent(button)  {}

		/**
		 * @brief Describes the event.
		 * @return A string with the button code.
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
	 * @brief Raised when a mouse button goes up.
	 * @ingroup group_events
	 */
	class MouseButtonReleasedEvent : public MouseButtonEvent
	{

	public:
		/**
		 * @brief Constructs the event.
		 * @param button Button code from MouseButtonCodes.h.
		 */
		MouseButtonReleasedEvent(int button)
			: MouseButtonEvent(button) {}

		/**
		 * @brief Describes the event.
		 * @return A string with the button code.
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
