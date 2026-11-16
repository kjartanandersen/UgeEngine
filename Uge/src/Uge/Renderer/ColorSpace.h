/**
 * @file ColorSpace.h
 * @brief Conversions between sRGB and linear colour.
 * @ingroup group_renderer
 */

#pragma once

#include <cmath>

#include <glm/glm.hpp>

namespace Uge
{

	/**
	 * @brief Converts an sRGB channel to linear.
	 * @param channel sRGB value in `[0, 1]`.
	 * @return The linear value.
	 * @ingroup group_renderer
	 */
	inline float SrgbToLinear(float channel)
	{
		return channel <= 0.04045f
			? channel / 12.92f
			: std::pow((channel + 0.055f) / 1.055f, 2.4f);
	}

	/**
	 * @brief Converts an sRGB colour to linear.
	 * @param color sRGB colour in `[0, 1]`.
	 * @return The linear colour.
	 * @ingroup group_renderer
	 *
	 * Colours picked on screen are sRGB; convert them before giving them to the renderer.
	 */
	inline glm::vec3 SrgbToLinear(const glm::vec3& color)
	{
		return { SrgbToLinear(color.r), SrgbToLinear(color.g), SrgbToLinear(color.b) };
	}

	/**
	 * @brief Converts an sRGB colour to linear, keeping alpha as it is.
	 * @param color sRGB colour.
	 * @return The linear colour.
	 * @ingroup group_renderer
	 */
	inline glm::vec4 SrgbToLinear(const glm::vec4& color)
	{
		return { SrgbToLinear(glm::vec3(color)), color.a };
	}

	/**
	 * @brief Converts a linear channel to sRGB.
	 * @param channel Linear value in `[0, 1]`.
	 * @return The sRGB value.
	 * @ingroup group_renderer
	 */
	inline float LinearToSrgb(float channel)
	{
		return channel <= 0.0031308f
			? channel * 12.92f
			: 1.055f * std::pow(channel, 1.0f / 2.4f) - 0.055f;
	}

	/**
	 * @brief Converts a linear colour to sRGB.
	 * @param color Linear colour in `[0, 1]`.
	 * @return The sRGB colour.
	 * @ingroup group_renderer
	 */
	inline glm::vec3 LinearToSrgb(const glm::vec3& color)
	{
		return { LinearToSrgb(color.r), LinearToSrgb(color.g), LinearToSrgb(color.b) };
	}

}
