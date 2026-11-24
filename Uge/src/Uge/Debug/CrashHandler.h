/**
 * @file CrashHandler.h
 * @brief Logs crashes and writes minidumps.
 * @ingroup group_debug
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Uge
{

	/**
	 * @brief On a crash, logs the faulting address and a stack trace, and writes a minidump.
	 * @ingroup group_debug
	 *
	 * Install it right after Uge::Log::Init.
	 */
	class CrashHandler
	{
	public:
		/**
		 * @brief Installs the unhandled-exception filter and the `std::terminate` handler.
		 *
		 * Later calls do nothing.
		 */
		static void Init();

		/**
		 * @brief Where minidumps are written.
		 * @return `logs`, relative to the working directory.
		 */
		static const char* GetDumpDirectory();

		/**
		 * @brief A stack trace of the calling thread.
		 * @param skipFrames Innermost frames to leave out.
		 * @return One line per frame, outermost last.
		 *
		 * Frames show `function + offset (file:line)` when the PDBs are present, and
		 * module-relative addresses otherwise.
		 */
		static std::vector<std::string> CaptureStackTrace(uint32_t skipFrames = 0);
	};

}
