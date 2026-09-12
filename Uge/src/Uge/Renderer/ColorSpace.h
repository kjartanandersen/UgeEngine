/**
 * @file ColorSpace.h
 * @brief Conversions between display-referred sRGB and the linear values the renderer works in.
 * @ingroup group_renderer
 */

#pragma once

#include <cmath>

#include <glm/glm.hpp>

namespace Uge
{

	/**
	 * @brief Converts one sRGB-encoded channel to linear.
	 * @param channel Encoded value, normally in `[0, 1]`.
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
	 * @brief Converts an sRGB-encoded colour to linear.
	 * @param color Encoded RGB, normally in `[0, 1]`.
	 * @return The linear colour.
	 * @ingroup group_renderer
	 *
	 * Colours picked by eye — a viewport background, a light tint in the property panel — are
	 * chosen for how they look on screen, which makes them sRGB. The scene renders into a
	 * linear target and is encoded once in Uge::PostProcess, so such a value has to be
	 * decoded before it is handed to the renderer, or it is encoded twice and comes out too
	 * bright.
	 *
	 * @note Alpha is never gamma encoded; the four-component overload leaves it alone.
	 */
	inline glm::vec3 SrgbToLinear(const glm::vec3& color)
	{
		return { SrgbToLinear(color.r), SrgbToLinear(color.g), SrgbToLinear(color.b) };
	}

	/**
	 * @brief Converts an sRGB-encoded colour to linear, preserving alpha.
	 * @param color Encoded RGBA; alpha is passed through untouched.
	 * @return The linear colour with the original alpha.
	 * @ingroup group_renderer
	 */
	inline glm::vec4 SrgbToLinear(const glm::vec4& color)
	{
		return { SrgbToLinear(glm::vec3(color)), color.a };
	}

	/**
	 * @brief Converts a linear channel to sRGB encoding.
	 * @param channel Linear value, normally in `[0, 1]`.
	 * @return The encoded value.
	 * @ingroup group_renderer
	 */
	inline float LinearToSrgb(float channel)
	{
		return channel <= 0.0031308f
			? channel * 12.92f
			: 1.055f * std::pow(channel, 1.0f / 2.4f) - 0.055f;
	}

	/**
	 * @brief Converts a linear colour to sRGB encoding.
	 * @param color Linear RGB, normally in `[0, 1]`.
	 * @return The encoded colour.
	 * @ingroup group_renderer
	 *
	 * The inverse of SrgbToLinear(const glm::vec3&), for showing a linear value back to the
	 * user in a colour picker.
	 */
	inline glm::vec3 LinearToSrgb(const glm::vec3& color)
	{
		return { LinearToSrgb(color.r), LinearToSrgb(color.g), LinearToSrgb(color.b) };
	}

}
