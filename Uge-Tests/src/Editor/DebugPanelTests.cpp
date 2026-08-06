/**
 * @file DebugPanelTests.cpp
 * @brief Tests for the editor's diagnostics panel, drawn headlessly.
 */

#include <ugpch.h>

#include "Panels/DebugPanel.h"

#include "TestSupport/HeadlessImGui.h"

#include "Uge/Debug/FrameProfiler.h"
#include "Uge/Scene/Components.h"
#include "Uge/Scene/Entity.h"
#include "Uge/Scene/Scene.h"

#include <gtest/gtest.h>

using Uge::DebugPanel;
using Uge::Entity;
using Uge::FrameProfiler;
using Uge::Scene;

namespace
{

	class DebugPanelTest : public UgeTests::HeadlessImGuiTest
	{
	protected:
		DebugPanel m_panel;
	};

}

TEST_F(DebugPanelTest, RendersWithNoSceneAndNoProject)
{
	// This is the editor's state before a project finishes loading. Every section has to
	// tolerate it: no scene, no asset manager, no device info from a driver.
	DrawFrames(2, [this]() { m_panel.OnImGuiRender(); });
}

TEST_F(DebugPanelTest, RendersSceneStatisticsForAPopulatedScene)
{
	Uge::Ref<Scene> scene = Uge::CreateRef<Scene>();
	scene->CreateEntity("A");
	scene->CreateEntity("B");

	m_panel.SetContext(scene);

	DrawFrames(2, [this]() { m_panel.OnImGuiRender(); });

	EXPECT_EQ(scene->GetAllEntitiesWith<Uge::IDComponent>().size(), 2u);
}

TEST_F(DebugPanelTest, RendersWithAHoveredEntity)
{
	Uge::Ref<Scene> scene = Uge::CreateRef<Scene>();
	Entity hovered = scene->CreateEntity("Hovered");

	m_panel.SetContext(scene);
	m_panel.SetHoveredEntity(hovered);

	DrawFrames(2, [this]() { m_panel.OnImGuiRender(); });
}

TEST_F(DebugPanelTest, ClearingTheContextDropsTheHoveredEntity)
{
	Uge::Ref<Scene> scene = Uge::CreateRef<Scene>();
	m_panel.SetContext(scene);
	m_panel.SetHoveredEntity(scene->CreateEntity("Hovered"));

	// Opening a different scene must not leave the panel holding a handle into the old
	// registry; dereferencing one after the scene is gone is undefined.
	m_panel.SetContext(nullptr);

	DrawFrames(2, [this]() { m_panel.OnImGuiRender(); });
}

TEST_F(DebugPanelTest, RendersProfilerTimings)
{
	// Close a frame so GetLastFrame() has scopes in it and the profiler table draws rows
	// rather than the "no timings" placeholder.
	{
		UG_PROFILE_SCOPE("DebugPanelTests::SyntheticScope");
	}
	FrameProfiler::BeginFrame(16.6f);

	DrawFrames(2, [this]() { m_panel.OnImGuiRender(); });
}

TEST_F(DebugPanelTest, OnUpdateIsANoOpWhenNoCaptureIsRunning)
{
	for (int i = 0; i < 5; i++)
	{
		m_panel.OnUpdate();
	}
}
