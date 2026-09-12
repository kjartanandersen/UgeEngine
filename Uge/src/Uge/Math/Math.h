/**
 * @file Math.h
 * @brief Math helpers layered on GLM.
 * @ingroup group_math
 */

#pragma once

#include <glm/glm.hpp>

namespace Uge
{
	/**
	 * @brief Math utilities that GLM does not provide directly.
	 * @ingroup group_math
	 */
	namespace Math
	{


		/**
		 * @brief Splits a transform matrix into translation, rotation and scale.
		 * @param[in]  transform Matrix to decompose.
		 * @param[out] outTranslation Extracted position.
		 * @param[out] outRotation Extracted Euler angles, in **radians**.
		 * @param[out] outScale Extracted scale factors.
		 * @return `true` if the matrix could be decomposed.
		 * @ingroup group_math
		 *
		 * Used when a gizmo writes back a manipulated matrix and the resulting values have to
		 * be stored in a Uge::TransformComponent, which keeps the three parts separately.
		 *
		 * @note A simplified version of `glm::decompose` that skips perspective and skew.
		 * Output values are undefined when it returns `false`.
		 */
		bool DecomposeTransform(const glm::mat4& transform, 
			glm::vec3& outTranslation, glm::vec3& outRotation, glm::vec3& outScale);



	}

}