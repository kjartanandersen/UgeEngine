/**
 * @file RendererSettingsPanelTests.cpp
 * @brief Tests for the editor's renderer settings panel, drawn headlessly.
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

	/** @brief Restores the process-wide render settings after each test. */
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
	// The panel reads the settings into a local, edits it with widgets, then writes the
	// whole struct back every frame. A field the panel forgot to draw would be written
	// back as a default and silently reset itself here.
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
	// The None branch draws an extra explanatory paragraph the others do not.
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
	// Disabled bloom wraps its widgets in BeginDisabled/EndDisabled; an unbalanced pair
	// would assert inside ImGui.
	for (bool enabled : { true, false })
	{
		RenderSettings settings;
		settings.BloomEnabled = enabled;
		PostProcess::SetSettings(settings);

		DrawFrames(2, [this]() { m_panel.OnImGuiRender(); });

		EXPECT_EQ(PostProcess::GetSettings().BloomEnabled, enabled);
	}
}
