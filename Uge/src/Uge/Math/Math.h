/**
 * @file Math.h
 * @brief Math helpers.
 * @ingroup group_math
 */

#pragma once

#include <glm/glm.hpp>

namespace Uge
{
	/**
	 * @brief Math helpers missing from GLM.
	 * @ingroup group_math
	 */
	namespace Math
	{


		/**
		 * @brief Splits a transform into translation, rotation and scale.
		 * @param[in]  transform The transform.
		 * @param[out] outTranslation Translation.
		 * @param[out] outRotation Euler angles, in radians.
		 * @param[out] outScale Scale.
		 * @return `false` if the matrix can't be split; the outputs are then undefined.
		 * @ingroup group_math
		 *
		 * Perspective and skew are ignored.
		 */
		bool DecomposeTransform(const glm::mat4& transform, 
			glm::vec3& outTranslation, glm::vec3& outRotation, glm::vec3& outScale);



	}

}