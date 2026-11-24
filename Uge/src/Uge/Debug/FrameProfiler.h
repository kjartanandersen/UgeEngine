/**
 * @file FrameProfiler.h
 * @brief Per-frame timings of the profiling scopes.
 * @ingroup group_debug
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Uge
{

	/**
	 * @brief One scope's timings over a frame.
	 * @ingroup group_debug
	 */
	struct ProfileEntry
	{
		/** @brief The scope label; the qualified function name for `UG_PROFILE_FUNCTION`. */
		std::string Name;
		float TotalMs = 0.0f;  ///< Total time in the frame.
		uint32_t Calls = 0;    ///< Times the scope ran in the frame.
	};

	/**
	 * @brief Sums the `UG_PROFILE_*` scope timings per frame and keeps recent frame times.
	 * @ingroup group_debug
	 *
	 * Always on. Submit() does no string work; label text is read once per frame, in
	 * BeginFrame().
	 */
	class FrameProfiler
	{
	public:
		/** @brief Number of frame times kept. */
		static constexpr size_t s_historySize = 256;

		/**
		 * @brief Ends the frame being recorded and starts the next.
		 * @param frameMs Length of the frame that ended, in milliseconds.
		 *
		 * Scopes whose labels have the same text become one entry.
		 */
		static void BeginFrame(float frameMs);

		/**
		 * @brief Adds a scope's timing to the current frame.
		 * @param name Scope label. It must stay valid until the next BeginFrame(), so it must
		 * be a string literal or `__FUNCSIG__`.
		 * @param milliseconds How long the scope took.
		 */
		static void Submit(const char* name, float milliseconds);

		/**
		 * @brief The timings of the last finished frame.
		 * @return Entries, slowest first.
		 */
		static std::vector<ProfileEntry> GetLastFrame();

		/**
		 * @brief Recent frame times.
		 * @return Up to #s_historySize values in milliseconds, oldest first.
		 */
		static std::vector<float> GetFrameTimeHistory();

		/**
		 * @brief The mean of the kept frame times.
		 * @return Milliseconds, or `0` before the first frame.
		 */
		static float GetAverageFrameMs();

		/**
		 * @brief The length of the last finished frame.
		 * @return Milliseconds, or `0` before the first frame.
		 */
		static float GetLastFrameMs();

		/** @brief Clears the frame times and the last frame's timings. */
		static void Reset();
	};

}
