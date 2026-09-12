/**
 * @file FrameProfiler.h
 * @brief Live, in-memory per-frame timing aggregation for the editor's Debug panel.
 * @ingroup group_debug
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Uge
{

	/**
	 * @brief Accumulated timings for one named scope over a single frame.
	 * @ingroup group_debug
	 */
	struct ProfileEntry
	{
		/**
		 * @brief Display label: the literal passed to `UG_PROFILE_SCOPE`, or for
		 *        `UG_PROFILE_FUNCTION` the qualified function name with its return type
		 *        and parameter list stripped.
		 */
		std::string Name;
		float TotalMs = 0.0f;  ///< Summed wall time across every call in the frame.
		uint32_t Calls = 0;    ///< Number of times the scope was entered during the frame.
	};

	/**
	 * @brief Aggregates `UG_PROFILE_*` scope timings per frame and keeps a frame-time history.
	 * @ingroup group_debug
	 *
	 * The Chrome-tracing output of Uge::Instrumentor answers "what happened during those
	 * three seconds"; this answers "what is costing me *right now*", which is the question
	 * a live panel needs. Both are fed from the same Uge::InstrumentationTimer, but this
	 * one is always on and allocates nothing per event beyond a map lookup.
	 *
	 * Uge::Application::Run calls BeginFrame() once per frame; the editor's Debug panel
	 * reads GetLastFrame() and GetFrameTimeHistory().
	 */
	class FrameProfiler
	{
	public:
		/** @brief Number of frame times retained for the frame-time graph. */
		static constexpr size_t s_historySize = 256;

		/**
		 * @brief Closes the previous frame and starts a new one.
		 * @param frameMs Wall time of the frame that just finished, in milliseconds.
		 *
		 * Snapshots the accumulated scopes so GetLastFrame() stays stable for the whole
		 * frame, then clears the accumulator and records @p frameMs in the history ring.
		 */
		static void BeginFrame(float frameMs);

		/**
		 * @brief Records one completed scope timing into the frame being accumulated.
		 * @param name Scope label; must outlive the call, so pass a string literal.
		 * @param milliseconds Duration of the scope.
		 *
		 * Called by Uge::InstrumentationTimer; there is rarely a reason to call it directly.
		 */
		static void Submit(const char* name, float milliseconds);

		/**
		 * @brief Returns the scope timings of the most recently completed frame.
		 * @return Entries sorted by descending total time.
		 */
		static std::vector<ProfileEntry> GetLastFrame();

		/**
		 * @brief Returns recent frame times for plotting.
		 * @return Up to #s_historySize milliseconds values, oldest first.
		 */
		static std::vector<float> GetFrameTimeHistory();

		/**
		 * @brief Returns the mean of the retained frame times.
		 * @return Average frame time in milliseconds, or `0` before the first frame.
		 */
		static float GetAverageFrameMs();

		/**
		 * @brief Returns the duration of the most recently completed frame.
		 * @return Frame time in milliseconds, or `0` before the first frame.
		 */
		static float GetLastFrameMs();

		/** @brief Drops the frame-time history and the last frame's scope timings. */
		static void Reset();
	};

}
