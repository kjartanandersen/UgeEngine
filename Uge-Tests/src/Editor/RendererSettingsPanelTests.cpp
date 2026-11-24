/**
 * @file RendererSettingsPanelTests.cpp
 * @brief Tests for the renderer settings panel.
 */

#include <ugpch.h>

#include "Panels/RendererSettingsPanel.h"

#include "TestSupport/HeadlessImGui.h"

#include "Uge/Renderer/PostProcess.h"

#include <gtest/gtest.h>

using Uge::PostProcess;
using Uge::RenderSettings;
using Uge::RendererSettingsPanel;
using Uge::TonemapMode;

namespace
{

	/** @brief Restores the render settings after each test. */
	class RendererSettingsPanelTest : public UgeTests::HeadlessImGuiTest
	{
	protected:
		void SetUp() override
		{
			UgeTests::HeadlessImGuiTest::SetUp();
			m_saved = PostProcess::GetSettings();
		}

		void TearDown() override
		{
			PostProcess::SetSettings(m_saved);
			UgeTests::HeadlessImGuiTest::TearDown();
		}

		RendererSettingsPanel m_panel;

	private:
		RenderSettings m_saved;
	};

}

TEST_F(RendererSettingsPanelTest, RendersWithDefaultSettings)
{
	PostProcess::SetSettings(RenderSettings{});

	DrawFrames(2, [this]() { m_panel.OnImGuiRender(); });
}

TEST_F(RendererSettingsPanelTest, LeavesSettingsUntouchedWhenNothingIsEdited)
{
	// The panel writes the whole struct back every frame, so every field must survive it.
	RenderSettings settings;
	settings.Exposure = 2.5f;
	settings.Tonemap = TonemapMode::Reinhard;
	settings.BloomEnabled = false;
	settings.BloomThreshold = 3.0f;
	settings.BloomKnee = 0.75f;
	settings.BloomIntensity = 0.125f;

	PostProcess::SetSettings(settings);

	DrawFrames(2, [this]() { m_panel.OnImGuiRender(); });

	const RenderSettings& after = PostProcess::GetSettings();
	EXPECT_FLOAT_EQ(after.Exposure, 2.5f);
	EXPECT_EQ(after.Tonemap, TonemapMode::Reinhard);
	EXPECT_FALSE(after.BloomEnabled);
	EXPECT_FLOAT_EQ(after.BloomThreshold, 3.0f);
	EXPECT_FLOAT_EQ(after.BloomKnee, 0.75f);
	EXPECT_FLOAT_EQ(after.BloomIntensity, 0.125f);
}

TEST_F(RendererSettingsPanelTest, RendersEveryTonemapMode)
{
	// None draws an extra paragraph.
	for (TonemapMode mode : { TonemapMode::None, TonemapMode::Reinhard, TonemapMode::ACES })
	{
		RenderSettings settings;
		settings.Tonemap = mode;
		PostProcess::SetSettings(settings);

		DrawFrames(2, [this]() { m_panel.OnImGuiRender(); });

		EXPECT_EQ(PostProcess::GetSettings().Tonemap, mode);
	}
}

TEST_F(RendererSettingsPanelTest, RendersWithBloomEnabledAndDisabled)
{
	// With bloom off, its widgets are inside BeginDisabled/EndDisabled.
	for (bool enabled : { true, false })
	{
		RenderSettings settings;
		settings.BloomEnabled = enabled;
		PostProcess::SetSettings(settings);

		DrawFrames(2, [this]() { m_panel.OnImGuiRender(); });

		EXPECT_EQ(PostProcess::GetSettings().BloomEnabled, enabled);
	}
}
