/**
 * @file Timestep.h
 * @brief Frame time.
 * @ingroup group_core
 */

#pragma once


namespace Uge
{

	/**
	 * @brief A frame's duration, in seconds. Converts to `float`.
	 * @ingroup group_core
	 */
	class Timestep
	{

	public:
		/**
		 * @brief Creates a timestep.
		 * @param time Duration in seconds.
		 */
		Timestep(float time = 0.0f)
			: m_time(time) {}


		/**
		 * @brief The duration in seconds.
		 * @return Seconds.
		 */
		operator float() const { return m_time; }

		/**
		 * @brief The duration in seconds.
		 * @return Seconds.
		 */
		float GetSeconds() const { return m_time; }
		/**
		 * @brief The duration in milliseconds.
		 * @return Milliseconds.
		 */
		float GetMilliseconds() const { return m_time * 1000; }


	private:
		float m_time;

	};



}

