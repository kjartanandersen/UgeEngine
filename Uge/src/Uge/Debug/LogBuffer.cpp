#include <ugpch.h>
#include "LogBuffer.h"

#include <cstdio>
#include <ctime>

#include "spdlog/details/os.h"
#include "spdlog/sinks/base_sink.h"

namespace Uge
{

	std::mutex LogBuffer::s_mutex;
	std::deque<LogEntry> LogBuffer::s_entries;
	uint32_t LogBuffer::s_counts[spdlog::level::n_levels] = {};
	uint64_t LogBuffer::s_version = 0;

	namespace
	{
		/**
		 * @brief spdlog sink that forwards every record into Uge::LogBuffer.
		 *
		 * Locking is handled by `base_sink<std::mutex>`; LogBuffer takes its own lock as
		 * well so that direct Push() calls stay safe.
		 */
		class LogBufferSink : public spdlog::sinks::base_sink<std::mutex>
		{
		protected:
			void sink_it_(const spdlog::details::log_msg& msg) override
			{
				const std::time_t timeT = std::chrono::system_clock::to_time_t(msg.time);
				const std::tm local = spdlog::details::os::localtime(timeT);

				char time[16];
				std::snprintf(time, sizeof(time), "%02d:%02d:%02d",
					local.tm_hour, local.tm_min, local.tm_sec);

				LogBuffer::Push({
					msg.level,
					time,
					std::string(msg.logger_name.data(), msg.logger_name.size()),
					std::string(msg.payload.data(), msg.payload.size())
				});
			}

			void flush_() override {}
		};
	}

	void LogBuffer::Push(LogEntry&& entry)
	{
		std::lock_guard<std::mutex> lock(s_mutex);

		if (s_entries.size() == s_capacity)
		{
			const spdlog::level::level_enum evicted = s_entries.front().Level;
			if (s_counts[evicted] > 0)
			{
				s_counts[evicted]--;
			}
			s_entries.pop_front();
		}

		s_counts[entry.Level]++;
		s_entries.push_back(std::move(entry));
		s_version++;
	}

	std::vector<LogEntry> LogBuffer::Snapshot()
	{
		std::lock_guard<std::mutex> lock(s_mutex);
		return std::vector<LogEntry>(s_entries.begin(), s_entries.end());
	}

	void LogBuffer::Clear()
	{
		std::lock_guard<std::mutex> lock(s_mutex);
		s_entries.clear();
		std::fill(std::begin(s_counts), std::end(s_counts), 0u);
		s_version++;
	}

	uint32_t LogBuffer::GetCount(spdlog::level::level_enum level)
	{
		if (level < 0 || level >= spdlog::level::n_levels)
		{
			return 0;
		}

		std::lock_guard<std::mutex> lock(s_mutex);
		return s_counts[level];
	}

	uint64_t LogBuffer::GetVersion()
	{
		std::lock_guard<std::mutex> lock(s_mutex);
		return s_version;
	}

	spdlog::sink_ptr LogBuffer::CreateSink()
	{
		return std::make_shared<LogBufferSink>();
	}

}
