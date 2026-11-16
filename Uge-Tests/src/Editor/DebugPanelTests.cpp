/**
 * @file DebugPanelTests.cpp
 * @brief Tests for the diagnostics panel.
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
	// As before a project loads: no scene, no asset manager and no device info.
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

	// A new scene clears the hovered entity, which belongs to the old one.
	m_panel.SetContext(nullptr);

	DrawFrames(2, [this]() { m_panel.OnImGuiRender(); });
}

TEST_F(DebugPanelTest, RendersProfilerTimings)
{
	// A finished frame, so the profiler table has rows.
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
