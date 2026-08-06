/**
 * @file EventTests.cpp
 * @brief Tests for the event hierarchy and Uge::EventDispatcher.
 */

#include <ugpch.h>

#include "Uge/Events/ApplicationEvent.h"
#include "Uge/Events/Event.h"
#include "Uge/Events/KeyEvent.h"
#include "Uge/Events/MouseEvent.h"

#include <gtest/gtest.h>

using namespace Uge;

TEST(EventTest, WindowResizeCarriesItsSizeAndIdentity)
{
	WindowResizeEvent event(1280, 720);

	EXPECT_EQ(event.GetWidth(), 1280u);
	EXPECT_EQ(event.GetHeight(), 720u);
	EXPECT_EQ(event.GetEventType(), EventType::WindowResize);
	EXPECT_EQ(WindowResizeEvent::GetStaticType(), EventType::WindowResize);
	EXPECT_STREQ(event.GetName(), "WindowResize");
}

TEST(EventTest, KeyPressedCarriesItsKeyAndRepeatCount)
{
	KeyPressedEvent event(65, 30, 3);

	EXPECT_EQ(event.GetKeyCode(), 65);
	EXPECT_EQ(event.GetRepeatCount(), 3);
	EXPECT_EQ(event.GetEventType(), EventType::KeyPressed);
}

TEST(EventTest, MouseEventsCarryTheirPayload)
{
	MouseMovedEvent moved(12.5f, -3.0f);
	EXPECT_FLOAT_EQ(moved.GetX(), 12.5f);
	EXPECT_FLOAT_EQ(moved.GetY(), -3.0f);

	MouseScrolledEvent scrolled(0.0f, 1.5f);
	EXPECT_FLOAT_EQ(scrolled.GetXOffset(), 0.0f);
	EXPECT_FLOAT_EQ(scrolled.GetYOffset(), 1.5f);

	MouseButtonPressedEvent pressed(1);
	EXPECT_EQ(pressed.GetMouseButton(), 1);
}

TEST(EventTest, EventsReportEveryCategoryTheyBelongTo)
{
	// A layer filters on categories, so an event that under-reports them silently stops
	// reaching handlers that ask for the family rather than the exact type.
	KeyPressedEvent key(65, 30, 0);
	EXPECT_TRUE(key.IsInCategory(EventCategoryKeyboard));
	EXPECT_TRUE(key.IsInCategory(EventCategoryInput));
	EXPECT_FALSE(key.IsInCategory(EventCategoryMouse));
	EXPECT_FALSE(key.IsInCategory(EventCategoryApplication));

	MouseButtonPressedEvent button(0);
	EXPECT_TRUE(button.IsInCategory(EventCategoryMouse));
	EXPECT_TRUE(button.IsInCategory(EventCategoryInput));
	EXPECT_FALSE(button.IsInCategory(EventCategoryApplication));

	WindowCloseEvent close;
	EXPECT_TRUE(close.IsInCategory(EventCategoryApplication));
	EXPECT_FALSE(close.IsInCategory(EventCategoryInput));
}

TEST(EventTest, EventsStartUnhandled)
{
	WindowCloseEvent event;

	EXPECT_FALSE(event.m_handled);
}

TEST(EventTest, ToStringIncludesThePayload)
{
	WindowResizeEvent event(800, 600);

	const std::string text = event.ToString();
	EXPECT_NE(text.find("800"), std::string::npos);
	EXPECT_NE(text.find("600"), std::string::npos);
}

TEST(EventDispatcherTest, RunsTheHandlerWhenTheTypeMatches)
{
	WindowResizeEvent event(1920, 1080);
	EventDispatcher dispatcher(event);

	unsigned int seenWidth = 0;
	const bool dispatched = dispatcher.Dispatch<WindowResizeEvent>(
		[&seenWidth](WindowResizeEvent& e)
		{
			seenWidth = e.GetWidth();
			return true;
		});

	EXPECT_TRUE(dispatched);
	EXPECT_EQ(seenWidth, 1920u);
	EXPECT_TRUE(event.m_handled);
}

TEST(EventDispatcherTest, SkipsTheHandlerWhenTheTypeDiffers)
{
	WindowResizeEvent event(1920, 1080);
	EventDispatcher dispatcher(event);

	bool ran = false;
	const bool dispatched = dispatcher.Dispatch<WindowCloseEvent>(
		[&ran](WindowCloseEvent&)
		{
			ran = true;
			return true;
		});

	EXPECT_FALSE(dispatched);
	EXPECT_FALSE(ran);
	EXPECT_FALSE(event.m_handled);
}

TEST(EventDispatcherTest, HandlerReturnValueDecidesWhetherPropagationStops)
{
	// Returning false from a handler is how a layer says "I looked at this, let it keep
	// going". If Dispatch forced m_handled to true, no lower layer would ever see it.
	KeyPressedEvent event(65, 30, 0);
	EventDispatcher dispatcher(event);

	dispatcher.Dispatch<KeyPressedEvent>([](KeyPressedEvent&) { return false; });

	EXPECT_FALSE(event.m_handled);
}

TEST(EventDispatcherTest, ReturnValueReportsThatTheHandlerRanNotThatItHandled)
{
	// The documented quirk: Dispatch returns true for a type match even when the handler
	// declined the event.
	KeyPressedEvent event(65, 30, 0);
	EventDispatcher dispatcher(event);

	const bool dispatched = dispatcher.Dispatch<KeyPressedEvent>(
		[](KeyPressedEvent&) { return false; });

	EXPECT_TRUE(dispatched);
	EXPECT_FALSE(event.m_handled);
}

TEST(EventDispatcherTest, OnlyTheMatchingHandlerOfSeveralRuns)
{
	// The shape of a real OnEvent: a short list of dispatches, exactly one of which fires.
	MouseScrolledEvent event(0.0f, 2.0f);
	EventDispatcher dispatcher(event);

	bool keyRan = false;
	bool resizeRan = false;
	bool scrollRan = false;

	dispatcher.Dispatch<KeyPressedEvent>([&](KeyPressedEvent&) { keyRan = true; return true; });
	dispatcher.Dispatch<WindowResizeEvent>([&](WindowResizeEvent&) { resizeRan = true; return true; });
	dispatcher.Dispatch<MouseScrolledEvent>([&](MouseScrolledEvent& e)
		{
			scrollRan = true;
			EXPECT_FLOAT_EQ(e.GetYOffset(), 2.0f);
			return true;
		});

	EXPECT_FALSE(keyRan);
	EXPECT_FALSE(resizeRan);
	EXPECT_TRUE(scrollRan);
}

TEST(EventDispatcherTest, DispatchingThroughABaseReferenceStillMatchesTheConcreteType)
{
	// Layers receive Event&, never the concrete type; the match has to be on the runtime
	// type rather than the static one.
	KeyReleasedEvent concrete(70, 33);
	Event& event = concrete;

	EventDispatcher dispatcher(event);

	int seenKey = 0;
	const bool dispatched = dispatcher.Dispatch<KeyReleasedEvent>(
		[&seenKey](KeyReleasedEvent& e)
		{
			seenKey = e.GetKeyCode();
			return true;
		});

	EXPECT_TRUE(dispatched);
	EXPECT_EQ(seenKey, 70);
}
