/**
 * @file ConsolePanelTests.cpp
 * @brief Tests for the editor's console panel, drawn headlessly.
 */

#include <ugpch.h>

#include "Panels/ConsolePanel.h"

#include "TestSupport/HeadlessImGui.h"

#include "Uge/Debug/LogBuffer.h"

#include <gtest/gtest.h>

using Uge::ConsolePanel;
using Uge::LogBuffer;
using Uge::LogEntry;

namespace
{

	void PushEntry(spdlog::level::level_enum level, const std::string& message)
	{
		LogBuffer::Push(LogEntry{ level, "12:00:00", "UGE", message });
	}

	/** @brief Console panel over a buffer that starts and ends empty. */
	class ConsolePanelTest : public UgeTests::HeadlessImGuiTest
	{
	protected:
		void SetUp() override
		{
			UgeTests::HeadlessImGuiTest::SetUp();
			LogBuffer::Clear();
		}

		void TearDown() override
		{
			LogBuffer::Clear();
			UgeTests::HeadlessImGuiTest::TearDown();
		}

		ConsolePanel m_panel;
	};

}

TEST_F(ConsolePanelTest, RendersWithAnEmptyBuffer)
{
	DrawFrames(2, [this]() { m_panel.OnImGuiRender(); });
}

TEST_F(ConsolePanelTest, RendersEverySeverity)
{
	PushEntry(spdlog::level::trace, "trace record");
	PushEntry(spdlog::level::debug, "debug record");
	PushEntry(spdlog::level::info, "info record");
	PushEntry(spdlog::level::warn, "warn record");
	PushEntry(spdlog::level::err, "error record");
	PushEntry(spdlog::level::critical, "critical record");

	DrawFrames(2, [this]() { m_panel.OnImGuiRender(); });

	// The panel only reads the buffer; drawing must not consume it.
	EXPECT_EQ(LogBuffer::Snapshot().size(), 6u);
}

TEST_F(ConsolePanelTest, PicksUpRecordsPushedBetweenFrames)
{
	// The panel caches a snapshot and re-copies on a version change. A stale cache would
	// show as an empty console while records keep arriving, so drive the transition.
	DrawFrame([this]() { m_panel.OnImGuiRender(); });

	PushEntry(spdlog::level::info, "arrived later");

	DrawFrame([this]() { m_panel.OnImGuiRender(); });

	EXPECT_EQ(LogBuffer::GetCount(spdlog::level::info), 1u);
}

TEST_F(ConsolePanelTest, RendersABufferLargeEnoughToClip)
{
	// The row list is built through an ImGuiListClipper, which behaves differently once
	// the content is taller than the window.
	for (int i = 0; i < 2000; i++)
	{
		PushEntry(spdlog::level::info, "record " + std::to_string(i));
	}

	DrawFrames(2, [this]() { m_panel.OnImGuiRender(); });

	EXPECT_EQ(LogBuffer::Snapshot().size(), 2000u);
}

TEST_F(ConsolePanelTest, RendersRecordsWithFormattingSpecifiersInThem)
{
	// Log text reaches ImGui as user data. Passing it as a format string would crash here.
	PushEntry(spdlog::level::err, "failed to open %s at 0x%p (%d%% done)");
	PushEntry(spdlog::level::warn, "shader has {0} uniforms");

	DrawFrames(2, [this]() { m_panel.OnImGuiRender(); });
}

TEST_F(ConsolePanelTest, SurvivesTheBufferBeingClearedWhileCached)
{
	PushEntry(spdlog::level::info, "before");
	DrawFrame([this]() { m_panel.OnImGuiRender(); });

	LogBuffer::Clear();

	DrawFrame([this]() { m_panel.OnImGuiRender(); });

	EXPECT_TRUE(LogBuffer::Snapshot().empty());
}
