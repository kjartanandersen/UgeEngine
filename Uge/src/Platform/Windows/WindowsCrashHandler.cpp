#include <ugpch.h>
#include "Uge/Debug/CrashHandler.h"

#ifdef UG_PLATFORM_WINDOWS

#include <cstdio>
#include <ctime>
#include <exception>
#include <filesystem>
#include <mutex>

// dbghelp.lib is linked by premake, alongside the other Windows system libraries.
#include <DbgHelp.h>

namespace Uge
{

	namespace
	{
		constexpr const char* s_dumpDirectory = "logs";
		constexpr uint32_t s_maxStackFrames = 62; // CaptureStackBackTrace's documented cap.

		bool s_installed = false;
		bool s_symbolsInitialized = false;
		std::mutex s_symbolMutex; // DbgHelp is single-threaded; serialise every entry point.

		/** @brief Loads debug symbols for the process once, on first use. */
		void EnsureSymbols()
		{
			if (s_symbolsInitialized)
			{
				return;
			}

			SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME);
			s_symbolsInitialized = SymInitialize(GetCurrentProcess(), nullptr, TRUE) != FALSE;
		}

		/** @brief Builds `logs/Uge-YYYYMMDD-HHMMSS.dmp`. */
		std::string MakeDumpPath()
		{
			const std::time_t now = std::time(nullptr);
			std::tm local{};
			localtime_s(&local, &now);

			char path[128];
			std::snprintf(path, sizeof(path), "%s/Uge-%04d%02d%02d-%02d%02d%02d.dmp",
				s_dumpDirectory,
				local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,
				local.tm_hour, local.tm_min, local.tm_sec);

			return path;
		}

		/**
		 * @brief Writes a minidump for the faulting thread.
		 * @param exceptionInfo The exception record, or `nullptr` for a terminate handler.
		 * @return Path written, or an empty string on failure.
		 */
		std::string WriteMiniDump(EXCEPTION_POINTERS* exceptionInfo)
		{
			std::filesystem::create_directories(s_dumpDirectory);

			const std::string path = MakeDumpPath();
			HANDLE file = CreateFileA(path.c_str(), GENERIC_WRITE, 0, nullptr,
				CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

			if (file == INVALID_HANDLE_VALUE)
			{
				return {};
			}

			MINIDUMP_EXCEPTION_INFORMATION info{};
			info.ThreadId = GetCurrentThreadId();
			info.ExceptionPointers = exceptionInfo;
			info.ClientPointers = FALSE;

			const BOOL written = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
				file, MiniDumpWithIndirectlyReferencedMemory,
				exceptionInfo ? &info : nullptr, nullptr, nullptr);

			CloseHandle(file);

			return written ? path : std::string{};
		}

		/** @brief Maps the common `EXCEPTION_*` codes to readable names. */
		const char* ExceptionCodeToString(DWORD code)
		{
			switch (code)
			{
				case EXCEPTION_ACCESS_VIOLATION:         return "Access violation";
				// Reached when an assertion's UG_DEBUGBREAK fires with no debugger attached.
				case EXCEPTION_BREAKPOINT:               return "Breakpoint (assertion or debug break)";
				case EXCEPTION_SINGLE_STEP:              return "Single step";
				case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    return "Array bounds exceeded";
				case EXCEPTION_DATATYPE_MISALIGNMENT:    return "Datatype misalignment";
				case EXCEPTION_FLT_DENORMAL_OPERAND:     return "Float denormal operand";
				case EXCEPTION_FLT_DIVIDE_BY_ZERO:       return "Float divide by zero";
				case EXCEPTION_FLT_INVALID_OPERATION:    return "Float invalid operation";
				case EXCEPTION_ILLEGAL_INSTRUCTION:      return "Illegal instruction";
				case EXCEPTION_INT_DIVIDE_BY_ZERO:       return "Integer divide by zero";
				case EXCEPTION_PRIV_INSTRUCTION:         return "Privileged instruction";
				case EXCEPTION_STACK_OVERFLOW:           return "Stack overflow";
				case EXCEPTION_IN_PAGE_ERROR:            return "In-page error";
				case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "Non-continuable exception";
				default:                                 return "Unknown exception";
			}
		}

		/**
		 * @brief Shared reporting path: logs the reason, the stack and the dump location.
		 * @param reason One-line description of what went wrong.
		 * @param exceptionInfo The exception record, or `nullptr`.
		 * @param skipFrames Innermost frames to omit from the trace.
		 */
		void Report(const std::string& reason, EXCEPTION_POINTERS* exceptionInfo, uint32_t skipFrames)
		{
			UG_CORE_FATAL("================ Uge crashed ================");
			UG_CORE_FATAL("{0}", reason);

			UG_CORE_FATAL("Stack trace:");
			for (const std::string& frame : CrashHandler::CaptureStackTrace(skipFrames))
			{
				UG_CORE_FATAL("    {0}", frame);
			}

			const std::string dump = WriteMiniDump(exceptionInfo);
			if (dump.empty())
			{
				UG_CORE_FATAL("Failed to write a minidump.");
			}
			else
			{
				UG_CORE_FATAL("Minidump written to {0}", dump);
			}

			UG_CORE_FATAL("=============================================");

			// The process is about to die, so push the file sink out before it does.
			Log::Shutdown();
		}

		/** @brief `SetUnhandledExceptionFilter` callback. */
		LONG WINAPI OnUnhandledException(EXCEPTION_POINTERS* exceptionInfo)
		{
			const EXCEPTION_RECORD* record = exceptionInfo ? exceptionInfo->ExceptionRecord : nullptr;
			const DWORD code = record ? record->ExceptionCode : 0;

			char reason[256];
			std::snprintf(reason, sizeof(reason), "%s (0x%08lX) at address 0x%p",
				ExceptionCodeToString(code), static_cast<unsigned long>(code),
				record ? record->ExceptionAddress : nullptr);

			// Skip this filter itself; the frames below it are the real fault site.
			Report(reason, exceptionInfo, 1);

			return EXCEPTION_EXECUTE_HANDLER;
		}

		/** @brief `std::set_terminate` callback, for uncaught C++ exceptions. */
		void OnTerminate()
		{
			std::string reason = "std::terminate called";

			if (std::exception_ptr current = std::current_exception())
			{
				try
				{
					std::rethrow_exception(current);
				}
				catch (const std::exception& e)
				{
					reason += std::string(" - uncaught exception: ") + e.what();
				}
				catch (...)
				{
					reason += " - uncaught exception of unknown type";
				}
			}

			Report(reason, nullptr, 1);

			// Hand back to the CRT so the debugger still gets its chance.
			std::abort();
		}
	}

	void CrashHandler::Init()
	{
		if (s_installed)
		{
			return;
		}
		s_installed = true;

		std::filesystem::create_directories(s_dumpDirectory);

		SetUnhandledExceptionFilter(OnUnhandledException);
		std::set_terminate(OnTerminate);

		UG_CORE_TRACE("Crash handler installed; dumps go to '{0}'", s_dumpDirectory);
	}

	const char* CrashHandler::GetDumpDirectory()
	{
		return s_dumpDirectory;
	}

	std::vector<std::string> CrashHandler::CaptureStackTrace(uint32_t skipFrames)
	{
		std::lock_guard<std::mutex> lock(s_symbolMutex);
		EnsureSymbols();

		void* frames[s_maxStackFrames] = {};
		const USHORT captured = CaptureStackBackTrace(
			static_cast<ULONG>(skipFrames + 1), s_maxStackFrames, frames, nullptr);

		std::vector<std::string> trace;
		trace.reserve(captured);

		const HANDLE process = GetCurrentProcess();

		// SYMBOL_INFO is variable-length: the name is written past the end of the struct.
		alignas(SYMBOL_INFO) char symbolStorage[sizeof(SYMBOL_INFO) + MAX_SYM_NAME] = {};
		SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>(symbolStorage);
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		symbol->MaxNameLen = MAX_SYM_NAME;

		for (USHORT i = 0; i < captured; i++)
		{
			const DWORD64 address = reinterpret_cast<DWORD64>(frames[i]);

			DWORD64 displacement = 0;
			const bool haveSymbol = s_symbolsInitialized
				&& SymFromAddr(process, address, &displacement, symbol) != FALSE;

			std::string line;
			if (haveSymbol)
			{
				line = symbol->Name;
				line += " + " + std::to_string(displacement);
			}
			else
			{
				char raw[32];
				std::snprintf(raw, sizeof(raw), "0x%llX", static_cast<unsigned long long>(address));
				line = raw;
			}

			IMAGEHLP_LINE64 lineInfo{};
			lineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
			DWORD lineDisplacement = 0;

			if (s_symbolsInitialized
				&& SymGetLineFromAddr64(process, address, &lineDisplacement, &lineInfo) != FALSE)
			{
				line += " (";
				line += lineInfo.FileName;
				line += ":" + std::to_string(lineInfo.LineNumber);
				line += ")";
			}

			trace.emplace_back(std::move(line));
		}

		return trace;
	}

}

#endif // UG_PLATFORM_WINDOWS
