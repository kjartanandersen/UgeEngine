/**
 * @file Timer.h
 * @brief Small high-resolution stopwatch.
 * @ingroup group_core
 */

#pragma once

#include <chrono>

namespace Uge
{

	/**
	 * @brief A stopwatch over `std::chrono::high_resolution_clock`.
	 * @ingroup group_core
	 *
	 * Starts on construction; call Reset() to restart it.
	 *
	 * @code
	 * Timer timer;
	 * LoadScene(path);
	 * UG_CORE_INFO("Scene loaded in {0}ms", timer.ElapsedMillis());
	 * @endcode
	 */
	class Timer
	{
	public:
		/** @brief Constructs the timer and starts it. */
		Timer()
		{
			Reset();
		}

		/** @brief Restarts the measurement from now. */
		void Timer::Reset()
		{
			m_Start = std::chrono::high_resolution_clock::now();
		}

		/**
		 * @brief Time since construction or the last Reset().
		 * @return Elapsed seconds.
		 */
		float Timer::Elapsed()
		{
			return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_Start).count() * 0.001f * 0.001f * 0.001f;
		}

		/**
		 * @brief Time since construction or the last Reset().
		 * @return Elapsed milliseconds.
		 */
		float Timer::ElapsedMillis()
		{
			return Elapsed() * 1000.0f;
		}

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
	};


}