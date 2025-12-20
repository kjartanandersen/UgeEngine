#pragma once

#include "ugpch.h"

#include "Event.h"

namespace Uge
{

	class KeyEvent : public Event
	{

	public:
		inline int GetKeyCode() const { return m_keyCode; };
		inline int GetScanCode() const { return m_scanCode; };

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

	protected:
		KeyEvent(int keyCode, int scancode)
			: m_keyCode(keyCode), m_scanCode(scancode) { }

		int m_keyCode;
		int m_scanCode;

	};

	class  KeyPressedEvent : public KeyEvent
	{

	public:
		KeyPressedEvent(int keyCode, int scancode, int repeatCount)
			: KeyEvent(keyCode, scancode), m_repeatCount(repeatCount) { }

		inline int GetRepeatCount() const { return m_repeatCount; }

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

	class KeyReleasedEvent : public KeyEvent
	{

	public:
		KeyReleasedEvent(int keyCode, int scancode)
			: KeyEvent(keyCode, scancode) {
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_keyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyReleased)

	};


	class KeyTypedEvent : public KeyEvent
	{

	public:
		KeyTypedEvent(int keyCode, int scancode)
			: KeyEvent(keyCode, scancode) {  }
		KeyTypedEvent(int keyCode)
			: KeyEvent(keyCode, 0) { }


		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent: " << m_keyCode ;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyTyped)


	};



}

