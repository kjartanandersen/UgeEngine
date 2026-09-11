/**
 * @file AABB.h
 * @brief File detailing the implementation of Axis Aligned Bounding Box.
 * @ingroup group_math
 */

#pragma once

#include <glm/glm.hpp>

namespace Uge::Math
{
	/**
	 * @brief Implementation of Axis Aligned Bounding Box, currently used for frustrum culling. 
	 * Based on James Arvo's algorithm
	 * @ingroup group_math
	 *
	 * On initialization Min is set to floating point maximum and Max is set to floating point minimum so that Grow() can accumulate
	 */
	struct AABB
	{

		glm::vec3 Min = glm::vec3( FLT_MAX); ///< The Minimum, XYZ set to floating point maximum on start
		glm::vec3 Max = glm::vec3(-FLT_MAX); ///< The Maximum, XYZ set to floating point minimum on start

  /**
      @brief  Checks if the AABB minimums and maximums are valid
      @retval bool - True if valid, otherwise false
  **/
		bool IsValid() const { return Min.x <= Max.x && Min.y <= Max.y && Min.z <= Max.z; }

  /**
      @brief Grow the AABB minimums and maximums so that the point fits into the AABB
      @param point - The point added into the AABB
  **/
		void Grow(const glm::vec3& point)
		{

			Min = glm::min(Min, point);
			Max = glm::max(Max, point);

		}

  /**
      @brief Grow the AABB minimums and maximums so that the AABB fits into the AABB
      @param other - The AABB added into the AABB
  **/
		void Grow(const AABB& other)
		{
			if (!other.IsValid())
			{
				return;
			}

			Min = glm::min(Min, other.Min);	
			Max = glm::max(Max, other.Max);
		}

  /**
      @brief  Get the center of the AABB
      @retval  - The center
  **/
		glm::vec3 GetCenter()  const { return (Min + Max) * 0.5f; }
  /**
      @brief  Get the extents of the AABB
      @retval  - The extents
  **/
		glm::vec3 GetExtents() const { return (Max - Min) * 0.5f; }

  /**
      @brief  James Arvo's implementation for transforming on AABB
      @param  matrix - The transformation matrix
      @retval	- A transformed AABB
  **/
		AABB AABB::Transform(const glm::mat4& matrix) const
		{
			if (!IsValid())
			{
				return *this;
			}

			const glm::vec3 center = glm::vec3(matrix * glm::vec4(GetCenter(), 1.0f));

			const glm::vec3 e = GetExtents();
			const glm::vec3 rotatedExtents = glm::vec3(
				std::abs(matrix[0][0]) * e.x + std::abs(matrix[1][0]) * e.y + std::abs(matrix[2][0]) * e.z,
				std::abs(matrix[0][1]) * e.x + std::abs(matrix[1][1]) * e.y + std::abs(matrix[2][1]) * e.z,
				std::abs(matrix[0][2]) * e.x + std::abs(matrix[1][2]) * e.y + std::abs(matrix[2][2]) * e.z);

			AABB result;
			result.Min = center - rotatedExtents;
			result.Max = center + rotatedExtents;
			return result;
		}

	};

}