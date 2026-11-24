/**
 * @file Log.h
 * @brief Logging through spdlog, and the `UG_*` log macros.
 * @ingroup group_core
 */

#pragma once

#include "Core.h"

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"


namespace Uge
{
	/**
	 * @brief Owns the engine's and the client's loggers.
	 * @ingroup group_core
	 *
	 * Both write to the console, to the file at GetLogFilePath() (truncated each run) and to
	 * Uge::LogBuffer. Messages use fmt syntax (`{0}`), not printf.
	 */
	class Log
	{

	public:
		/** @brief Creates both loggers. Must run before any log macro. */
		static void Init();

		/** @brief Flushes both loggers to the file. */
		static void Shutdown();

		/**
		 * @brief The log file, relative to the working directory.
		 * @return `logs/Uge.log`.
		 */
		inline static const char* GetLogFilePath() { return s_logFilePath; }

		/**
		 * @brief The engine's logger.
		 * @return The "UGE" logger.
		 */
		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; };
		/**
		 * @brief The client's logger.
		 * @return The "APP" logger.
		 */
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; };
	
	private:
		static constexpr const char* s_logFilePath = "logs/Uge.log";

		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};

}

// TODO: Add checks for release builds to disable logging

/**
 * @name Engine log macros
 * @brief For engine code; log through the core logger.
 * @ingroup group_core
 * @{
 */
/** @brief Logs at trace level. @param ... fmt format string and arguments. */
#define UG_CORE_TRACE(...)	::Uge::Log::GetCoreLogger()->trace(__VA_ARGS__)
/** @brief Logs at info level. @param ... fmt format string and arguments. */
#define UG_CORE_INFO(...)	::Uge::Log::GetCoreLogger()->info(__VA_ARGS__)
/** @brief Logs a warning. @param ... fmt format string and arguments. */
#define UG_CORE_WARN(...)	::Uge::Log::GetCoreLogger()->warn(__VA_ARGS__)
/** @brief Logs an error. @param ... fmt format string and arguments. */
#define UG_CORE_ERROR(...)	::Uge::Log::GetCoreLogger()->error(__VA_ARGS__)
/** @brief Logs at critical level. @param ... fmt format string and arguments. */
#define UG_CORE_FATAL(...)	::Uge::Log::GetCoreLogger()->critical(__VA_ARGS__)

/** @} */

/**
 * @name Client log macros
 * @brief For editor and game code; log through the client logger.
 * @ingroup group_core
 * @{
 */
/** @brief Logs at trace level. @param ... fmt format string and arguments. */
#define UG_TRACE(...)		::Uge::Log::GetClientLogger()->trace(__VA_ARGS__)
/** @brief Logs at info level. @param ... fmt format string and arguments. */
#define UG_INFO(...)		::Uge::Log::GetClientLogger()->info(__VA_ARGS__)
/** @brief Logs a warning. @param ... fmt format string and arguments. */
#define UG_WARN(...)		::Uge::Log::GetClientLogger()->warn(__VA_ARGS__)
/** @brief Logs an error. @param ... fmt format string and arguments. */
#define UG_ERROR(...)		::Uge::Log::GetClientLogger()->error(__VA_ARGS__)
/** @brief Logs at critical level. @param ... fmt format string and arguments. */
#define UG_FATAL(...)		::Uge::Log::GetClientLogger()->critical(__VA_ARGS__)
/** @} */
