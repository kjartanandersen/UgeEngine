/**
 * @file Timestep.h
 * @brief Frame delta-time wrapper passed to every layer update.
 * @ingroup group_core
 */

#pragma once


namespace Uge
{

	/**
	 * @brief A frame duration, stored in seconds.
	 * @ingroup group_core
	 *
	 * Wrapping the delta in a type rather than passing a bare `float` makes the unit
	 * explicit at call sites. It converts implicitly back to `float` (seconds), so it can
	 * be used directly in arithmetic:
	 *
	 * @code
	 * void OnUpdate(Timestep ts) override
	 * {
	 *     m_position += m_velocity * ts;          // implicit conversion, seconds
	 *     UG_TRACE("frame: {0}ms", ts.GetMilliseconds());
	 * }
	 * @endcode
	 */
	class Timestep
	{

	public:
		/**
		 * @brief Constructs a timestep.
		 * @param time Elapsed time in seconds.
		 */
		Timestep(float time = 0.0f)
			: m_time(time) {}


		/**
		 * @brief Implicit conversion to the elapsed time in seconds.
		 * @return Seconds elapsed.
		 */
		operator float() const { return m_time; }

		/**
		 * @brief Elapsed time in seconds.
		 * @return Seconds elapsed.
		 */
		float GetSeconds() const { return m_time; }
		/**
		 * @brief Elapsed time in milliseconds.
		 * @return Milliseconds elapsed.
		 */
		float GetMilliseconds() const { return m_time * 1000; }


	private:
		float m_time;

	};



}

