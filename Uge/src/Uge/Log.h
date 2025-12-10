#pragma once

#include "Core.h"

// spdlog
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"


namespace Uge
{
	class UG_API Log
	{

	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; };
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; };
	
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};

}

// TODO: Add checks for release builds to disable logging

// Core Log Macros
#define UG_CORE_TRACE(...)	::Uge::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define UG_CORE_INFO(...)	::Uge::Log::GetCoreLogger()->info(__VA_ARGS__)
#define UG_CORE_WARN(...)	::Uge::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define UG_CORE_ERROR(...)	::Uge::Log::GetCoreLogger()->error(__VA_ARGS__)
#define UG_CORE_FATAL(...)	::Uge::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client Log Macros
#define UG_TRACE(...)		::Uge::Log::GetClientLogger()->trace(__VA_ARGS__)
#define UG_INFO(...)		::Uge::Log::GetClientLogger()->info(__VA_ARGS__)
#define UG_WARN(...)		::Uge::Log::GetClientLogger()->warn(__VA_ARGS__)
#define UG_ERROR(...)		::Uge::Log::GetClientLogger()->error(__VA_ARGS__)
#define UG_FATAL(...)		::Uge::Log::GetClientLogger()->critical(__VA_ARGS__)