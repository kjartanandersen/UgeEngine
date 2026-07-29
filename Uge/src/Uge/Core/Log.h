/**
 * @file Log.h
 * @brief spdlog-backed logging facade and the `UG_*` logging macros.
 * @ingroup group_core
 */

#pragma once

#include "Core.h"

// spdlog
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"


namespace Uge
{
	/**
	 * @brief Owns the two spdlog loggers used by the engine and by client applications.
	 * @ingroup group_core
	 *
	 * Prefer the macros over calling the loggers directly: `UG_CORE_*` inside the engine
	 * and `UG_*` in client applications, so engine and game output stay distinguishable.
	 *
	 * @code
	 * UG_CORE_INFO("Renderer: {0}", RendererAPI::GetAPI());   // engine code
	 * UG_WARN("Missing texture for entity {0}", entityId);    // client code
	 * @endcode
	 *
	 * Formatting uses the fmt syntax (`{0}`, `{1}`, ...), not printf specifiers.
	 */
	class Log
	{

	public:
		/**
		 * @brief Creates and configures both loggers.
		 *
		 * Must run before any logging macro; `main` in EntryPoint.h calls it first.
		 */
		static void Init();

		/**
		 * @brief Returns the engine-side logger.
		 * @return Reference to the shared "UGE" logger.
		 */
		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; };
		/**
		 * @brief Returns the client-side logger.
		 * @return Reference to the shared application logger.
		 */
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; };
	
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};

}

// TODO: Add checks for release builds to disable logging

/**
 * @name Engine logging macros
 * @brief Log through the core logger; use these inside the engine.
 * @ingroup group_core
 * @{
 */
// Core Log Macros
/** @brief Engine-side trace-level log. @param ... fmt format string and arguments. */
#define UG_CORE_TRACE(...)	::Uge::Log::GetCoreLogger()->trace(__VA_ARGS__)
/** @brief Engine-side info-level log. @param ... fmt format string and arguments. */
#define UG_CORE_INFO(...)	::Uge::Log::GetCoreLogger()->info(__VA_ARGS__)
/** @brief Engine-side warning-level log. @param ... fmt format string and arguments. */
#define UG_CORE_WARN(...)	::Uge::Log::GetCoreLogger()->warn(__VA_ARGS__)
/** @brief Engine-side error-level log. @param ... fmt format string and arguments. */
#define UG_CORE_ERROR(...)	::Uge::Log::GetCoreLogger()->error(__VA_ARGS__)
/** @brief Engine-side critical-level log. @param ... fmt format string and arguments. */
#define UG_CORE_FATAL(...)	::Uge::Log::GetCoreLogger()->critical(__VA_ARGS__)

/** @} */

/**
 * @name Client logging macros
 * @brief Log through the client logger; use these in editor and game code.
 * @ingroup group_core
 * @{
 */
// Client Log Macros
/** @brief Client-side trace-level log. @param ... fmt format string and arguments. */
#define UG_TRACE(...)		::Uge::Log::GetClientLogger()->trace(__VA_ARGS__)
/** @brief Client-side info-level log. @param ... fmt format string and arguments. */
#define UG_INFO(...)		::Uge::Log::GetClientLogger()->info(__VA_ARGS__)
/** @brief Client-side warning-level log. @param ... fmt format string and arguments. */
#define UG_WARN(...)		::Uge::Log::GetClientLogger()->warn(__VA_ARGS__)
/** @brief Client-side error-level log. @param ... fmt format string and arguments. */
#define UG_ERROR(...)		::Uge::Log::GetClientLogger()->error(__VA_ARGS__)
/** @brief Client-side critical-level log. @param ... fmt format string and arguments. */
#define UG_FATAL(...)		::Uge::Log::GetClientLogger()->critical(__VA_ARGS__)
/** @} */
