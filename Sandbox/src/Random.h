/**
 * @file Random.h
 * @brief Small random-number helper used by the Sandbox particle sample.
 */

#pragma once

#include <random>

/**
 * @brief A shared Mersenne Twister engine producing normalized floats.
 *
 * @warning Call Init() once at startup to seed the engine, and note that the shared
 * engine is not thread-safe.
 */
class Random
{
public:
	/** @brief Seeds the engine from `std::random_device`. */
	static void Init()
	{
		s_RandomEngine.seed(std::random_device()());
	}

	/**
	 * @brief Draws a uniform random float.
	 * @return A value in `[0, 1]`.
	 */
	static float Float()
	{
		return (float)s_Distribution(s_RandomEngine) / (float)std::numeric_limits<uint32_t>::max();
	}

private:
	static std::mt19937 s_RandomEngine;
	static std::uniform_int_distribution<std::mt19937::result_type> s_Distribution;
};