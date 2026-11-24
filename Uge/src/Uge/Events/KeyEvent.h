/**
 * @file KeyEvent.h
 * @brief Keyboard events.
 * @ingroup group_events
 */

#pragma once

#include "ugpch.h"

#include "Event.h"

namespace Uge
{

	/**
	 * @brief Base class of the keyboard events.
	 * @ingroup group_events
	 */
	class KeyEvent : public Event
	{

	public:
		/**
		 * @brief The key.
		 * @return A `UG_KEY_*` code; depends on the keyboard layout.
		 */
		inline int GetKeyCode() const { return m_keyCode; };
		/**
		 * @brief The physical key.
		 * @return A platform scan code, independent of the layout.
		 */
		inline int GetScanCode() const { return m_scanCode; };

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

	protected:
		/**
		 * @brief Creates the event.
		 * @param keyCode Key code.
		 * @param scancode Scan code.
		 */
		KeyEvent(int keyCode, int scancode)
			: m_keyCode(keyCode), m_scanCode(scancode) { }

		int m_keyCode; ///< Key code.
		int m_scanCode; ///< Scan code.

	};

	/**
	 * @brief A key went down, or repeated while held.
	 * @ingroup group_events
	 */
	class  KeyPressedEvent : public KeyEvent
	{

	public:
		/**
		 * @brief Creates the event.
		 * @param keyCode Key code.
		 * @param scancode Scan code.
		 * @param repeatCount `0` for the first press.
		 */
		KeyPressedEvent(int keyCode, int scancode, int repeatCount)
			: KeyEvent(keyCode, scancode), m_repeatCount(repeatCount) { }

		/**
		 * @brief Whether this is a repeat.
		 * @return `0` for the first press, non-zero for repeats.
		 */
		inline int GetRepeatCount() const { return m_repeatCount; }

		/**
		 * @brief A description for logging.
		 * @return The key code and repeat count.
		 */
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_keyCode << " (" << m_repeatCount << " repeats)";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)

	private:
		int m_repeatCount;

	};

	/**
	 * @brief A key went up.
	 * @ingroup group_events
	 */
	class KeyReleasedEvent : public KeyEvent
	{

	public:
		/**
		 * @brief Creates the event.
		 * @param keyCode Key code.
		 * @param scancode Scan code.
		 */
		KeyReleasedEvent(int keyCode, int scancode)
			: KeyEvent(keyCode, scancode) {
		}

		/**
		 * @brief A description for logging.
		 * @return The key code.
		 */
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_keyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyReleased)

	};


	/**
	 * @brief A key press produced a character, after layout and modifiers.
	 * @ingroup group_events
	 */
	class KeyTypedEvent : public KeyEvent
	{

	public:
		/**
		 * @brief Creates the event.
		 * @param keyCode The character.
		 * @param scancode Scan code.
		 */
		KeyTypedEvent(int keyCode, int scancode)
			: KeyEvent(keyCode, scancode) {  }
		/**
		 * @brief Creates the event without a scan code.
		 * @param keyCode The character.
		 */
		KeyTypedEvent(int keyCode)
			: KeyEvent(keyCode, 0) { }


		/**
		 * @brief A description for logging.
		 * @return The character code.
		 */
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent: " << m_keyCode ;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyTyped)


	};



}

