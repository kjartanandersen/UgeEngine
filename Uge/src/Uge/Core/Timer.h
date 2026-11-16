/**
 * @file Timer.h
 * @brief A stopwatch.
 * @ingroup group_core
 */

#pragma once

#include <chrono>

namespace Uge
{

	/**
	 * @brief A high-resolution stopwatch that starts when it is created.
	 * @ingroup group_core
	 */
	class Timer
	{
	public:
		/** @brief Creates and starts the timer. */
		Timer()
		{
			Reset();
		}

		/** @brief Restarts the timer. */
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