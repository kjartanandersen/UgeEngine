/**
 * @file LayerStackTests.cpp
 * @brief Tests for Uge::LayerStack's ordering and ownership rules.
 */

#include <ugpch.h>

#include "Uge/Core/Layer.h"
#include "Uge/Core/LayerStack.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vector>

using Uge::Layer;
using Uge::LayerStack;

namespace
{

	/**
	 * @brief A Layer whose lifecycle calls can be asserted on.
	 *
	 * The stack owns and deletes what it is given, so these are always heap-allocated and
	 * either left to the stack or deleted by the test after being popped.
	 */
	class MockLayer : public Layer
	{
	public:
		explicit MockLayer(const std::string& name = "MockLayer")
			: Layer(name) { }

		MOCK_METHOD(void, OnAttach, (), (override));
		MOCK_METHOD(void, OnDetach, (), (override));
		MOCK_METHOD(void, OnUpdate, (Uge::Timestep), (override));
		MOCK_METHOD(void, OnImGuiRender, (), (override));
		MOCK_METHOD(void, OnEvent, (Uge::Event&), (override));
	};

	using NiceMockLayer = ::testing::NiceMock<MockLayer>;

	/** @brief The layers in iteration order, bottom first. */
	std::vector<Layer*> Order(LayerStack& stack)
	{
		return std::vector<Layer*>(stack.begin(), stack.end());
	}

}

TEST(LayerStackTest, StartsEmpty)
{
	LayerStack stack;

	EXPECT_EQ(stack.begin(), stack.end());
}

TEST(LayerStackTest, LayersIterateInPushOrder)
{
	LayerStack stack;

	Layer* first = new NiceMockLayer("First");
	Layer* second = new NiceMockLayer("Second");

	stack.PushLayer(first);
	stack.PushLayer(second);

	EXPECT_EQ(Order(stack), (std::vector<Layer*>{ first, second }));
}

TEST(LayerStackTest, OverlaysStayAboveLayersWhicheverOrderTheyArePushedIn)
{
	// This is the stack's whole reason for existing: the ImGui overlay is pushed during
	// Application's constructor, before any client layer, and still has to end up on top.
	LayerStack stack;

	Layer* overlay = new NiceMockLayer("Overlay");
	Layer* layer = new NiceMockLayer("Layer");

	stack.PushOverlay(overlay);
	stack.PushLayer(layer);

	EXPECT_EQ(Order(stack), (std::vector<Layer*>{ layer, overlay }));
}

TEST(LayerStackTest, NewLayersGoAboveOlderLayersButBelowEveryOverlay)
{
	LayerStack stack;

	Layer* bottom = new NiceMockLayer("Bottom");
	Layer* overlayA = new NiceMockLayer("OverlayA");
	Layer* middle = new NiceMockLayer("Middle");
	Layer* overlayB = new NiceMockLayer("OverlayB");

	stack.PushLayer(bottom);
	stack.PushOverlay(overlayA);
	stack.PushLayer(middle);
	stack.PushOverlay(overlayB);

	EXPECT_EQ(Order(stack), (std::vector<Layer*>{ bottom, middle, overlayA, overlayB }));
}

TEST(LayerStackTest, PopLayerDetachesRemovesAndReturnsOwnership)
{
	LayerStack stack;

	NiceMockLayer* layer = new NiceMockLayer("Layer");
	Layer* keep = new NiceMockLayer("Keep");

	stack.PushLayer(layer);
	stack.PushLayer(keep);

	EXPECT_CALL(*layer, OnDetach()).Times(1);

	stack.PopLayer(layer);

	EXPECT_EQ(Order(stack), (std::vector<Layer*>{ keep }));

	// Ownership came back to us, so the stack must not delete it a second time.
	delete layer;
}

TEST(LayerStackTest, PoppingALayerKeepsTheOverlayPartitionIntact)
{
	// PopLayer decrements the insert index; if it did not, a layer pushed afterwards
	// would land on the wrong side of the partition and render over the UI.
	LayerStack stack;

	Layer* first = new NiceMockLayer("First");
	Layer* overlay = new NiceMockLayer("Overlay");

	stack.PushLayer(first);
	stack.PushOverlay(overlay);
	stack.PopLayer(first);

	Layer* second = new NiceMockLayer("Second");
	stack.PushLayer(second);

	EXPECT_EQ(Order(stack), (std::vector<Layer*>{ second, overlay }));

	delete first;
}

TEST(LayerStackTest, PopLayerIgnoresALayerThatWasNeverPushed)
{
	LayerStack stack;

	Layer* pushed = new NiceMockLayer("Pushed");
	stack.PushLayer(pushed);

	NiceMockLayer stranger("Stranger");
	EXPECT_CALL(stranger, OnDetach()).Times(0);

	stack.PopLayer(&stranger);

	EXPECT_EQ(Order(stack), (std::vector<Layer*>{ pushed }));
}

TEST(LayerStackTest, PopLayerWillNotRemoveAnOverlay)
{
	// The two partitions are searched separately, so an overlay is only removable through
	// PopOverlay.
	LayerStack stack;

	NiceMockLayer* overlay = new NiceMockLayer("Overlay");
	stack.PushOverlay(overlay);

	EXPECT_CALL(*overlay, OnDetach()).Times(1); // From the destructor, not from PopLayer.

	stack.PopLayer(overlay);

	EXPECT_EQ(Order(stack), (std::vector<Layer*>{ overlay }));
}

TEST(LayerStackTest, PopOverlayRemovesTheOverlay)
{
	LayerStack stack;

	NiceMockLayer* overlay = new NiceMockLayer("Overlay");
	Layer* layer = new NiceMockLayer("Layer");

	stack.PushLayer(layer);
	stack.PushOverlay(overlay);

	stack.PopOverlay(overlay);

	EXPECT_EQ(Order(stack), (std::vector<Layer*>{ layer }));

	delete overlay;
}

TEST(LayerStackTest, PopOverlayDoesNotCallOnDetach)
{
	// Asymmetric with PopLayer, which does detach. Pinned here so the difference is a
	// deliberate decision rather than something a refactor changes by accident: nothing
	// else calls OnDetach for a popped overlay, so as it stands an overlay removed this
	// way never gets one.
	LayerStack stack;

	NiceMockLayer* overlay = new NiceMockLayer("Overlay");
	stack.PushOverlay(overlay);

	EXPECT_CALL(*overlay, OnDetach()).Times(0);

	stack.PopOverlay(overlay);

	delete overlay;
}

TEST(LayerStackTest, PopOverlayIgnoresAnOverlayThatWasNeverPushed)
{
	LayerStack stack;

	Layer* overlay = new NiceMockLayer("Overlay");
	stack.PushOverlay(overlay);

	NiceMockLayer stranger("Stranger");
	stack.PopOverlay(&stranger);

	EXPECT_EQ(Order(stack), (std::vector<Layer*>{ overlay }));
}

TEST(LayerStackTest, DestructorDetachesAndDeletesEverythingLeft)
{
	NiceMockLayer* layer = new NiceMockLayer("Layer");
	NiceMockLayer* overlay = new NiceMockLayer("Overlay");

	{
		LayerStack stack;
		stack.PushLayer(layer);
		stack.PushOverlay(overlay);

		EXPECT_CALL(*layer, OnDetach()).Times(1);
		EXPECT_CALL(*overlay, OnDetach()).Times(1);
	}

	// Both mocks verified their expectations as the stack deleted them; reaching here
	// without a leak report or a double free is the rest of the assertion.
}

TEST(LayerStackTest, PushingDoesNotAttach)
{
	// Application::PushLayer calls OnAttach itself, right after handing the layer over.
	// If the stack did it too, every layer would attach twice.
	LayerStack stack;

	NiceMockLayer* layer = new NiceMockLayer("Layer");
	EXPECT_CALL(*layer, OnAttach()).Times(0);
	EXPECT_CALL(*layer, OnDetach()).Times(1);

	stack.PushLayer(layer);
}

TEST(LayerStackTest, LayerKeepsItsDebugName)
{
	MockLayer layer("Diagnostics");

	EXPECT_EQ(layer.GetName(), "Diagnostics");
}
