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
	 * @brief Common base for keyboard events, carrying the key and scan codes.
	 * @ingroup group_events
	 *
	 * Abstract in practice: the constructor is protected, so only the concrete subclasses
	 * can be created. Dispatch against those, not against this class.
	 *
	 * Belongs to both #EventCategoryKeyboard and #EventCategoryInput.
	 */
	class KeyEvent : public Event
	{

	public:
		/**
		 * @brief The key involved.
		 * @return A `UG_KEY_*` value from KeyCodes.h; layout-dependent.
		 */
		inline int GetKeyCode() const { return m_keyCode; };
		/**
		 * @brief The physical key involved.
		 * @return A platform-specific scan code, independent of keyboard layout.
		 */
		inline int GetScanCode() const { return m_scanCode; };

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

	protected:
		/**
		 * @brief Constructs the base event.
		 * @param keyCode Layout-dependent key code.
		 * @param scancode Platform-specific physical scan code.
		 */
		KeyEvent(int keyCode, int scancode)
			: m_keyCode(keyCode), m_scanCode(scancode) { }

		int m_keyCode; ///< Layout-dependent key code from KeyCodes.h.
		int m_scanCode; ///< Platform-specific physical scan code.

	};

	/**
	 * @brief Raised when a key goes down, and again while it auto-repeats.
	 * @ingroup group_events
	 *
	 * Check GetRepeatCount() to distinguish the initial press from auto-repeat: shortcuts
	 * usually want to act only when it is `0`.
	 */
	class  KeyPressedEvent : public KeyEvent
	{

	public:
		/**
		 * @brief Constructs the event.
		 * @param keyCode Layout-dependent key code.
		 * @param scancode Platform-specific physical scan code.
		 * @param repeatCount `0` for the initial press, non-zero while auto-repeating.
		 */
		KeyPressedEvent(int keyCode, int scancode, int repeatCount)
			: KeyEvent(keyCode, scancode), m_repeatCount(repeatCount) { }

		/**
		 * @brief Auto-repeat state of this press.
		 * @return `0` on the first press, non-zero for repeats.
		 */
		inline int GetRepeatCount() const { return m_repeatCount; }

		/**
		 * @brief Describes the event.
		 * @return A string with the key code and repeat count.
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
	 * @brief Raised when a key goes up.
	 * @ingroup group_events
	 */
	class KeyReleasedEvent : public KeyEvent
	{

	public:
		/**
		 * @brief Constructs the event.
		 * @param keyCode Layout-dependent key code.
		 * @param scancode Platform-specific physical scan code.
		 */
		KeyReleasedEvent(int keyCode, int scancode)
			: KeyEvent(keyCode, scancode) {
		}

		/**
		 * @brief Describes the event.
		 * @return A string with the key code.
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
	 * @brief Raised when a key press produces a character.
	 * @ingroup group_events
	 *
	 * Unlike Uge::KeyPressedEvent this is text input: it accounts for keyboard layout and
	 * modifiers, so it is what text fields should consume. Non-printing keys such as
	 * Shift or the arrow keys produce no typed event.
	 */
	class KeyTypedEvent : public KeyEvent
	{

	public:
		/**
		 * @brief Constructs the event.
		 * @param keyCode Character code produced.
		 * @param scancode Platform-specific physical scan code.
		 */
		KeyTypedEvent(int keyCode, int scancode)
			: KeyEvent(keyCode, scancode) {  }
		/**
		 * @brief Constructs the event with no scan code.
		 * @param keyCode Character code produced.
		 */
		KeyTypedEvent(int keyCode)
			: KeyEvent(keyCode, 0) { }


		/**
		 * @brief Describes the event.
		 * @return A string with the character code.
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

