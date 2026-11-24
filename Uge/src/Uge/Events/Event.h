/**
 * @file Event.h
 * @brief Event base class, event types and categories, and the dispatcher.
 * @ingroup group_events
 */

#pragma once

#include "ugpch.h"
#include "Uge/Core/Core.h"

namespace Uge
{

	/**
	 * @brief The type of an Uge::Event.
	 * @ingroup group_events
	 */
	enum class EventType
	{

		None = 0,
		WindowClose,
		WindowResize,
		WindowFocus,
		WindowLostFocus,
		WindowMoved,
		AppTick,
		AppUpdate,
		AppRender,
		KeyPressed, ///< A key went down or repeated.
		KeyReleased,
		KeyTyped, ///< A character was typed.
		MouseButtonPressed,
		MouseButtonReleased,
		MouseMoved,
		MouseScrolled

	};

	/**
	 * @brief Bit flags for families of events. An event can be in several.
	 * @ingroup group_events
	 */
	enum EventCategory
	{

		None = 0,
		EventCategoryApplication	= BIT(0), ///< Window and application events.
		EventCategoryInput			= BIT(1), ///< Any input.
		EventCategoryKeyboard		= BIT(2),
		EventCategoryMouse			= BIT(3), ///< Movement and wheel.
		EventCategoryMouseButton	= BIT(4)


	};

/**
 * @def EVENT_CLASS_TYPE
 * @brief Defines `GetStaticType()`, `GetEventType()` and `GetName()` in an event class.
 * @param type The Uge::EventType enumerator, such as `WindowClose`.
 * @ingroup group_events
 */
#define EVENT_CLASS_TYPE(type)	static EventType GetStaticType() { return EventType::##type; }\
								virtual EventType GetEventType() const override { return GetStaticType(); }\
								virtual const char* GetName() const override { return #type; }
/**
 * @def EVENT_CLASS_CATEGORY
 * @brief Defines `GetCategoryFlags()` in an event class.
 * @param category Uge::EventCategory flags, combined with `|`.
 * @ingroup group_events
 */
#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }


	/**
	 * @brief Base class of every window and input event.
	 * @ingroup group_events
	 *
	 * Events are handled as soon as they occur. Layers get them from top to bottom, until
	 * one sets #m_handled.
	 *
	 * @todo Queue events and handle them in one phase of the frame.
	 */
	class Event
	{
		friend class EventDispatcher;

	public:
		bool m_handled = false; ///< `true` keeps the event from lower layers.


		/**
		 * @brief The event's type.
		 * @return The type.
		 */
		virtual EventType GetEventType() const = 0;
		/**
		 * @brief The class name, for logging.
		 * @return A name such as `"WindowResize"`.
		 */
		virtual const char* GetName() const = 0;
		/**
		 * @brief The event's categories.
		 * @return Uge::EventCategory flags.
		 */
		virtual int GetCategoryFlags() const = 0;
		/**
		 * @brief A description for logging.
		 * @return The description; GetName() by default.
		 */
		virtual std::string ToString() const { return GetName(); };

		/**
		 * @brief Whether the event is in a category.
		 * @param category Category to test.
		 * @return `true` if it is.
		 */
		inline bool IsInCategory(EventCategory category)
		{
			
			return GetCategoryFlags() & category;
		}

	};

	/**
	 * @brief Calls a handler when an event has a given type.
	 * @ingroup group_events
	 */
	class EventDispatcher
	{

		/**
		 * @brief A handler. Returning `true` marks the event handled.
		 * @tparam T Event type.
		 */
		template<typename T>
		using EventFn = std::function<bool(T&)>;

	public:
		/**
		 * @brief Wraps an event.
		 * @param event The event; it must outlive the dispatcher.
		 */
		EventDispatcher(Event& event)
			: m_event(event){ }

		/**
		 * @brief Calls @p func if the event is a `T`.
		 * @tparam T Event type to match.
		 * @param func Handler; its result is stored in Event::m_handled.
		 * @return `true` if the handler ran.
		 */
		template<typename T>
		bool Dispatch(EventFn<T> func)
		{

			if (m_event.GetEventType() == T::GetStaticType())
			{

				m_event.m_handled = func(*(T*)&m_event);
				return true;
			}
			return false;

		}
		
	private:
		Event& m_event;


	};

	/**
	 * @brief Writes an event's description.
	 * @param os Stream.
	 * @param e Event.
	 * @return @p os.
	 * @ingroup group_events
	 */
	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}




}