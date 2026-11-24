/**
 * @file FrameProfilerTests.cpp
 * @brief Tests for Uge::FrameProfiler.
 */

#include <ugpch.h>

#include "Uge/Debug/FrameProfiler.h"

#include <gtest/gtest.h>

using Uge::FrameProfiler;
using Uge::ProfileEntry;

namespace
{

	/** @brief Resets the profiler before and after each test. */
	class FrameProfilerTest : public ::testing::Test
	{
	protected:
		void SetUp() override { FrameProfiler::Reset(); }
		void TearDown() override { FrameProfiler::Reset(); }
	};

	const ProfileEntry* Find(const std::vector<ProfileEntry>& entries, const std::string& name)
	{
		for (const ProfileEntry& entry : entries)
		{
			if (entry.Name == name)
			{
				return &entry;
			}
		}
		return nullptr;
	}

}

TEST_F(FrameProfilerTest, StartsEmpty)
{
	EXPECT_TRUE(FrameProfiler::GetLastFrame().empty());
	EXPECT_TRUE(FrameProfiler::GetFrameTimeHistory().empty());
	EXPECT_FLOAT_EQ(FrameProfiler::GetLastFrameMs(), 0.0f);
	EXPECT_FLOAT_EQ(FrameProfiler::GetAverageFrameMs(), 0.0f);
}

TEST_F(FrameProfilerTest, SubmissionsOnlyBecomeVisibleWhenTheFrameCloses)
{
	// A scope submitted this frame appears only after BeginFrame.
	FrameProfiler::Submit("Scope", 1.0f);
	EXPECT_TRUE(FrameProfiler::GetLastFrame().empty());

	FrameProfiler::BeginFrame(16.0f);
	EXPECT_EQ(FrameProfiler::GetLastFrame().size(), 1u);
}

TEST_F(FrameProfilerTest, RepeatedScopesAccumulateIntoOneRow)
{
	FrameProfiler::Submit("Renderer2D::DrawQuad", 0.5f);
	FrameProfiler::Submit("Renderer2D::DrawQuad", 0.25f);
	FrameProfiler::Submit("Renderer2D::DrawQuad", 0.25f);

	FrameProfiler::BeginFrame(16.0f);

	const std::vector<ProfileEntry> entries = FrameProfiler::GetLastFrame();
	ASSERT_EQ(entries.size(), 1u);
	EXPECT_EQ(entries[0].Calls, 3u);
	EXPECT_NEAR(entries[0].TotalMs, 1.0f, 1e-5f);
}

TEST_F(FrameProfilerTest, EntriesAreSortedByDescendingCost)
{
	// The panel doesn't sort, so the most expensive scope comes first.
	FrameProfiler::Submit("Cheap", 0.1f);
	FrameProfiler::Submit("Expensive", 8.0f);
	FrameProfiler::Submit("Middling", 2.0f);

	FrameProfiler::BeginFrame(16.0f);

	const std::vector<ProfileEntry> entries = FrameProfiler::GetLastFrame();
	ASSERT_EQ(entries.size(), 3u);
	EXPECT_EQ(entries[0].Name, "Expensive");
	EXPECT_EQ(entries[1].Name, "Middling");
	EXPECT_EQ(entries[2].Name, "Cheap");
}

TEST_F(FrameProfilerTest, ClosingAFrameClearsTheAccumulator)
{
	FrameProfiler::Submit("Scope", 1.0f);
	FrameProfiler::BeginFrame(16.0f);
	FrameProfiler::BeginFrame(16.0f);

	// A scope from an earlier frame is gone.
	EXPECT_TRUE(FrameProfiler::GetLastFrame().empty());
}

TEST_F(FrameProfilerTest, ShortensAFunctionSignatureToAQualifiedName)
{
	// UG_PROFILE_FUNCTION's label is __FUNCSIG__.
	FrameProfiler::Submit("void __cdecl Uge::EditorLayer::OnUpdate(class Uge::Timestep)", 1.0f);
	FrameProfiler::BeginFrame(16.0f);

	const std::vector<ProfileEntry> entries = FrameProfiler::GetLastFrame();
	ASSERT_EQ(entries.size(), 1u);
	EXPECT_EQ(entries[0].Name, "Uge::EditorLayer::OnUpdate");
}

TEST_F(FrameProfilerTest, LeavesALiteralScopeNameUnchanged)
{
	FrameProfiler::Submit("Renderer2D Flush", 1.0f);
	FrameProfiler::BeginFrame(16.0f);

	const std::vector<ProfileEntry> entries = FrameProfiler::GetLastFrame();
	ASSERT_EQ(entries.size(), 1u);
	EXPECT_EQ(entries[0].Name, "Renderer2D Flush");
}

TEST_F(FrameProfilerTest, IdenticalLabelsFromDifferentCallSitesShareARow)
{
	FrameProfiler::Submit("void __cdecl Uge::Scene::OnUpdate(class Uge::Timestep)", 1.0f);
	FrameProfiler::Submit("void __cdecl Uge::Scene::OnUpdate(class Uge::Timestep)", 2.0f);

	FrameProfiler::BeginFrame(16.0f);

	const std::vector<ProfileEntry> entries = FrameProfiler::GetLastFrame();
	ASSERT_EQ(entries.size(), 1u);
	EXPECT_EQ(entries[0].Calls, 2u);
}

TEST_F(FrameProfilerTest, RecordsTheFrameTime)
{
	FrameProfiler::BeginFrame(12.5f);

	EXPECT_FLOAT_EQ(FrameProfiler::GetLastFrameMs(), 12.5f);
}

TEST_F(FrameProfilerTest, HistoryIsOldestFirstWhileItIsFillingUp)
{
	FrameProfiler::BeginFrame(1.0f);
	FrameProfiler::BeginFrame(2.0f);
	FrameProfiler::BeginFrame(3.0f);

	const std::vector<float> history = FrameProfiler::GetFrameTimeHistory();

	ASSERT_EQ(history.size(), 3u);
	EXPECT_FLOAT_EQ(history[0], 1.0f);
	EXPECT_FLOAT_EQ(history[2], 3.0f);
}

TEST_F(FrameProfilerTest, HistoryUnrollsTheRingOnceItWraps)
{
	// The history comes oldest first once the ring has wrapped.
	constexpr size_t frames = FrameProfiler::s_historySize + 10;

	for (size_t i = 0; i < frames; i++)
	{
		FrameProfiler::BeginFrame((float)i);
	}

	const std::vector<float> history = FrameProfiler::GetFrameTimeHistory();

	ASSERT_EQ(history.size(), FrameProfiler::s_historySize);
	EXPECT_FLOAT_EQ(history.front(), (float)(frames - FrameProfiler::s_historySize));
	EXPECT_FLOAT_EQ(history.back(), (float)(frames - 1));

	for (size_t i = 1; i < history.size(); i++)
	{
		ASSERT_FLOAT_EQ(history[i], history[i - 1] + 1.0f) << "at index " << i;
	}
}

TEST_F(FrameProfilerTest, AverageIsOverTheRetainedHistory)
{
	FrameProfiler::BeginFrame(10.0f);
	FrameProfiler::BeginFrame(20.0f);
	FrameProfiler::BeginFrame(30.0f);

	EXPECT_NEAR(FrameProfiler::GetAverageFrameMs(), 20.0f, 1e-5f);
}

TEST_F(FrameProfilerTest, ResetClearsEverything)
{
	FrameProfiler::Submit("Scope", 1.0f);
	FrameProfiler::BeginFrame(16.0f);

	FrameProfiler::Reset();

	EXPECT_TRUE(FrameProfiler::GetLastFrame().empty());
	EXPECT_TRUE(FrameProfiler::GetFrameTimeHistory().empty());
	EXPECT_FLOAT_EQ(FrameProfiler::GetLastFrameMs(), 0.0f);
	EXPECT_FLOAT_EQ(FrameProfiler::GetAverageFrameMs(), 0.0f);
}

TEST_F(FrameProfilerTest, SubmitIsSafeFromMultipleThreads)
{
	constexpr int threads = 4;
	constexpr int perThread = 250;

	std::vector<std::thread> workers;
	workers.reserve(threads);

	for (int t = 0; t < threads; t++)
	{
		workers.emplace_back([perThread]()
			{
				for (int i = 0; i < perThread; i++)
				{
					FrameProfiler::Submit("Concurrent", 1.0f);
				}
			});
	}

	for (std::thread& worker : workers)
	{
		worker.join();
	}

	FrameProfiler::BeginFrame(16.0f);

	const std::vector<ProfileEntry> entries = FrameProfiler::GetLastFrame();
	ASSERT_EQ(entries.size(), 1u);

	const ProfileEntry* entry = Find(entries, "Concurrent");
	ASSERT_NE(entry, nullptr);
	EXPECT_EQ(entry->Calls, (uint32_t)(threads * perThread));
	EXPECT_NEAR(entry->TotalMs, (float)(threads * perThread), 1.0f);
}

TEST_F(FrameProfilerTest, ALabelAtTwoAddressesIsOneRow)
{
	// The same text at two addresses, as a literal in two translation units can be, makes
	// one row.
	const char first[] = "Scene Physics";
	const char second[] = "Scene Physics";
	ASSERT_NE(static_cast<const void*>(first), static_cast<const void*>(second));

	FrameProfiler::Submit(first, 1.0f);
	FrameProfiler::Submit(second, 2.0f);
	FrameProfiler::BeginFrame(16.0f);

	const std::vector<ProfileEntry> entries = FrameProfiler::GetLastFrame();
	ASSERT_EQ(entries.size(), 1u);
	EXPECT_EQ(entries[0].Name, "Scene Physics");
	EXPECT_EQ(entries[0].Calls, 2u);
	EXPECT_NEAR(entries[0].TotalMs, 3.0f, 1e-5f);
}

