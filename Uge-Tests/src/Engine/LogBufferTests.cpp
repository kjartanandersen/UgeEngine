/**
 * @file LogBufferTests.cpp
 * @brief Tests for Uge::LogBuffer, the ring behind the editor's console panel.
 */

#include <ugpch.h>

#include "Uge/Debug/LogBuffer.h"

#include <gtest/gtest.h>

using Uge::LogBuffer;
using Uge::LogEntry;

namespace
{

	LogEntry MakeEntry(spdlog::level::level_enum level, const std::string& message)
	{
		return LogEntry{ level, "12:00:00", "UGE", message };
	}

	/** @brief Clears the process-wide buffer around each test so they cannot interfere. */
	class LogBufferTest : public ::testing::Test
	{
	protected:
		void SetUp() override { LogBuffer::Clear(); }
		void TearDown() override { LogBuffer::Clear(); }
	};

}

TEST_F(LogBufferTest, StartsEmpty)
{
	EXPECT_TRUE(LogBuffer::Snapshot().empty());
	EXPECT_EQ(LogBuffer::GetCount(spdlog::level::info), 0u);
}

TEST_F(LogBufferTest, SnapshotReturnsRecordsOldestFirst)
{
	LogBuffer::Push(MakeEntry(spdlog::level::info, "first"));
	LogBuffer::Push(MakeEntry(spdlog::level::warn, "second"));
	LogBuffer::Push(MakeEntry(spdlog::level::err, "third"));

	const std::vector<LogEntry> entries = LogBuffer::Snapshot();

	ASSERT_EQ(entries.size(), 3u);
	EXPECT_EQ(entries[0].Message, "first");
	EXPECT_EQ(entries[1].Message, "second");
	EXPECT_EQ(entries[2].Message, "third");
}

TEST_F(LogBufferTest, CountsAreTrackedPerLevel)
{
	LogBuffer::Push(MakeEntry(spdlog::level::info, "a"));
	LogBuffer::Push(MakeEntry(spdlog::level::info, "b"));
	LogBuffer::Push(MakeEntry(spdlog::level::err, "c"));

	EXPECT_EQ(LogBuffer::GetCount(spdlog::level::info), 2u);
	EXPECT_EQ(LogBuffer::GetCount(spdlog::level::err), 1u);
	EXPECT_EQ(LogBuffer::GetCount(spdlog::level::warn), 0u);
}

TEST_F(LogBufferTest, GetCountRejectsOutOfRangeLevels)
{
	EXPECT_EQ(LogBuffer::GetCount((spdlog::level::level_enum)spdlog::level::n_levels), 0u);
	EXPECT_EQ(LogBuffer::GetCount((spdlog::level::level_enum)-1), 0u);
}

TEST_F(LogBufferTest, VersionChangesOnPushAndClear)
{
	const uint64_t initial = LogBuffer::GetVersion();

	LogBuffer::Push(MakeEntry(spdlog::level::info, "a"));
	const uint64_t afterPush = LogBuffer::GetVersion();
	EXPECT_NE(afterPush, initial);

	// The console panel only re-copies the buffer when this changes, so a Clear() that
	// left the version alone would leave the panel showing records that are gone.
	LogBuffer::Clear();
	EXPECT_NE(LogBuffer::GetVersion(), afterPush);
}

TEST_F(LogBufferTest, ClearDropsRecordsAndCounts)
{
	LogBuffer::Push(MakeEntry(spdlog::level::warn, "a"));
	LogBuffer::Clear();

	EXPECT_TRUE(LogBuffer::Snapshot().empty());
	EXPECT_EQ(LogBuffer::GetCount(spdlog::level::warn), 0u);
}

TEST_F(LogBufferTest, EvictsOldestOnceFull)
{
	constexpr size_t overflow = 10;

	for (size_t i = 0; i < LogBuffer::s_capacity + overflow; i++)
	{
		LogBuffer::Push(MakeEntry(spdlog::level::info, std::to_string(i)));
	}

	const std::vector<LogEntry> entries = LogBuffer::Snapshot();

	ASSERT_EQ(entries.size(), LogBuffer::s_capacity);
	EXPECT_EQ(entries.front().Message, std::to_string(overflow));
	EXPECT_EQ(entries.back().Message, std::to_string(LogBuffer::s_capacity + overflow - 1));
}

TEST_F(LogBufferTest, EvictionDecrementsTheEvictedLevelsCount)
{
	// Fill with warnings, then push enough errors to evict some of them. If eviction did
	// not decrement, the console's "Warn (n)" button would keep counting records that the
	// ring has already dropped.
	for (size_t i = 0; i < LogBuffer::s_capacity; i++)
	{
		LogBuffer::Push(MakeEntry(spdlog::level::warn, "warn"));
	}
	ASSERT_EQ(LogBuffer::GetCount(spdlog::level::warn), (uint32_t)LogBuffer::s_capacity);

	constexpr uint32_t errors = 32;
	for (uint32_t i = 0; i < errors; i++)
	{
		LogBuffer::Push(MakeEntry(spdlog::level::err, "err"));
	}

	EXPECT_EQ(LogBuffer::GetCount(spdlog::level::err), errors);
	EXPECT_EQ(LogBuffer::GetCount(spdlog::level::warn), (uint32_t)LogBuffer::s_capacity - errors);
	EXPECT_EQ(LogBuffer::Snapshot().size(), LogBuffer::s_capacity);
}

TEST_F(LogBufferTest, PushIsSafeFromMultipleThreads)
{
	constexpr int threads = 4;
	constexpr int perThread = 500;

	std::vector<std::thread> workers;
	workers.reserve(threads);

	for (int t = 0; t < threads; t++)
	{
		workers.emplace_back([perThread]()
			{
				for (int i = 0; i < perThread; i++)
				{
					LogBuffer::Push(MakeEntry(spdlog::level::info, "concurrent"));
				}
			});
	}

	for (std::thread& worker : workers)
	{
		worker.join();
	}

	EXPECT_EQ(LogBuffer::GetCount(spdlog::level::info), (uint32_t)(threads * perThread));
	EXPECT_EQ(LogBuffer::Snapshot().size(), (size_t)(threads * perThread));
}
