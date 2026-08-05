/**
 * @file CrashHandler.h
 * @brief Last-resort diagnostics for unhandled exceptions and terminate calls.
 * @ingroup group_debug
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Uge
{

	/**
	 * @brief Catches process-fatal failures and records what happened before dying.
	 * @ingroup group_debug
	 *
	 * Without this, an access violation closes the window with nothing to go on. With it,
	 * the faulting address and a symbolised stack trace land in the log file, and a
	 * minidump is written that Visual Studio can open to inspect the state at the crash.
	 *
	 * Install it once, immediately after Uge::Log::Init; `main` in EntryPoint.h does so.
	 */
	class CrashHandler
	{
	public:
		/**
		 * @brief Installs the unhandled-exception filter and the `std::terminate` handler.
		 *
		 * Safe to call more than once; subsequent calls do nothing.
		 */
		static void Init();

		/**
		 * @brief Returns the directory minidumps are written to.
		 * @return `logs`, relative to the working directory.
		 */
		static const char* GetDumpDirectory();

		/**
		 * @brief Captures a symbolised stack trace of the calling thread.
		 * @param skipFrames Number of innermost frames to omit, so this helper and its
		 *        caller do not clutter the output.
		 * @return One formatted line per frame, outermost frame last.
		 *
		 * Frames resolve to `function + offset (file:line)` when PDBs are present, and to
		 * bare module-relative addresses otherwise. Also useful outside a crash — for
		 * instance to log who requested a suspicious asset load.
		 */
		static std::vector<std::string> CaptureStackTrace(uint32_t skipFrames = 0);
	};

}
