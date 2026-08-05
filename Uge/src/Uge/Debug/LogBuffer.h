/**
 * @file LogBuffer.h
 * @brief In-memory ring of recent log records, and the spdlog sink that fills it.
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
	 * @brief One captured log record, already formatted.
	 * @ingroup group_debug
	 */
	struct LogEntry
	{
		spdlog::level::level_enum Level = spdlog::level::info; ///< Severity of the record.
		std::string Time;    ///< Wall-clock time the record was emitted, as `HH:MM:SS`.
		std::string Logger;  ///< Name of the logger that emitted it, `UGE` or `APP`.
		std::string Message; ///< The formatted message text.
	};

	/**
	 * @brief Process-wide ring buffer holding the most recent log records.
	 * @ingroup group_debug
	 *
	 * Fed automatically by the sink Uge::Log::Init installs, so nothing needs to call
	 * Push() by hand. It exists so the editor's console panel — and any other client —
	 * can display engine output without a terminal attached.
	 *
	 * All members are thread-safe. Rather than locking while drawing, consumers should
	 * poll GetVersion() and only re-copy via Snapshot() when it changes:
	 *
	 * @code
	 * if (LogBuffer::GetVersion() != m_cachedVersion)
	 * {
	 *     m_cachedVersion = LogBuffer::GetVersion();
	 *     m_cached = LogBuffer::Snapshot();
	 * }
	 * @endcode
	 */
	class LogBuffer
	{
	public:
		/** @brief Maximum number of records retained; the oldest are dropped first. */
		static constexpr size_t s_capacity = 8192;

		/**
		 * @brief Appends a record, evicting the oldest one once the ring is full.
		 * @param entry The record to store; moved from.
		 */
		static void Push(LogEntry&& entry);

		/**
		 * @brief Copies every retained record.
		 * @return The records, oldest first.
		 */
		static std::vector<LogEntry> Snapshot();

		/** @brief Drops every retained record and resets the per-level counts. */
		static void Clear();

		/**
		 * @brief Returns how many retained records have a given severity.
		 * @param level Severity to count.
		 * @return Number of retained records at that level.
		 */
		static uint32_t GetCount(spdlog::level::level_enum level);

		/**
		 * @brief Returns a counter that changes whenever the contents change.
		 * @return Monotonically increasing version stamp.
		 *
		 * Bumped by both Push() and Clear(), so a consumer that caches a Snapshot() can
		 * detect staleness without copying.
		 */
		static uint64_t GetVersion();

		/**
		 * @brief Creates the spdlog sink that forwards records into this buffer.
		 * @return A sink ready to be attached to a logger.
		 */
		static spdlog::sink_ptr CreateSink();

	private:
		static std::mutex s_mutex;
		static std::deque<LogEntry> s_entries;
		static uint32_t s_counts[spdlog::level::n_levels];
		static uint64_t s_version;
	};

}
