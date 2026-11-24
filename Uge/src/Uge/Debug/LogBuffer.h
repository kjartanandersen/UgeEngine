/**
 * @file LogBuffer.h
 * @brief Recent log records in memory, and the spdlog sink that fills them.
 * @ingroup group_debug
 */

#pragma once

#include <cstdint>
#include <deque>
#include <mutex>
#include <string>
#include <vector>

#include "spdlog/common.h"

namespace Uge
{

	/**
	 * @brief One formatted log record.
	 * @ingroup group_debug
	 */
	struct LogEntry
	{
		spdlog::level::level_enum Level = spdlog::level::info; ///< Severity.
		std::string Time;    ///< When it was logged, as `HH:MM:SS`.
		std::string Logger;  ///< `UGE` or `APP`.
		std::string Message; ///< The message.
	};

	/**
	 * @brief The most recent log records, filled by the sink Uge::Log::Init installs.
	 * @ingroup group_debug
	 *
	 * Thread-safe. Compare GetVersion() before copying with Snapshot().
	 */
	class LogBuffer
	{
	public:
		/** @brief Records kept; the oldest are dropped first. */
		static constexpr size_t s_capacity = 8192;

		/**
		 * @brief Adds a record, dropping the oldest when full.
		 * @param entry The record.
		 */
		static void Push(LogEntry&& entry);

		/**
		 * @brief Copies the records.
		 * @return The records, oldest first.
		 */
		static std::vector<LogEntry> Snapshot();

		/** @brief Removes every record and resets the counts. */
		static void Clear();

		/**
		 * @brief How many records have a severity.
		 * @param level Severity to count.
		 * @return The count.
		 */
		static uint32_t GetCount(spdlog::level::level_enum level);

		/**
		 * @brief A counter that changes whenever the records do.
		 * @return The version.
		 */
		static uint64_t GetVersion();

		/**
		 * @brief Creates the spdlog sink that adds records to the buffer.
		 * @return The sink.
		 */
		static spdlog::sink_ptr CreateSink();

	private:
		static std::mutex s_mutex;
		static std::deque<LogEntry> s_entries;
		static uint32_t s_counts[spdlog::level::n_levels];
		static uint64_t s_version;
	};

}
