/**
 * @file Random.h
 * @brief Random floats for the Sandbox.
 */

#pragma once

#include <random>

/**
 * @brief Random floats from a shared Mersenne Twister. Call Init() first. Not thread-safe.
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
	 * @brief A uniform random float.
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