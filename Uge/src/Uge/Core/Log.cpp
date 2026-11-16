#include "ugpch.h"
#include "Log.h"

#include "Uge/Debug/LogBuffer.h"

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Uge
{
	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

	void Log::Init()
	{
		std::vector<spdlog::sink_ptr> sinks;

		// Console: time, logger and message.
		sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		sinks.back()->set_pattern("%^[%T] %n: %v%$");

		// File: truncated each run, with the date and level on every line.
		sinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(s_logFilePath, true));
		sinks.back()->set_pattern("[%Y-%m-%d %T.%e] [%n] [%l] %v");

		// In memory: the message only, since level and logger are stored separately.
		sinks.emplace_back(LogBuffer::CreateSink());
		sinks.back()->set_pattern("%v");

		s_CoreLogger = std::make_shared<spdlog::logger>("UGE", sinks.begin(), sinks.end());
		s_CoreLogger->set_level(spdlog::level::trace);
		s_CoreLogger->flush_on(spdlog::level::warn);
		spdlog::register_logger(s_CoreLogger);

		s_ClientLogger = std::make_shared<spdlog::logger>("APP", sinks.begin(), sinks.end());
		s_ClientLogger->set_level(spdlog::level::trace);
		s_ClientLogger->flush_on(spdlog::level::warn);
		spdlog::register_logger(s_ClientLogger);
	}

	void Log::Shutdown()
	{
		if (s_CoreLogger)
		{
			s_CoreLogger->flush();
		}
		if (s_ClientLogger)
		{
			s_ClientLogger->flush();
		}
	}
}
