/**
 * @file AABB.h
 * @brief Axis-aligned bounding box.
 * @ingroup group_math
 */

#pragma once

#include <glm/glm.hpp>

namespace Uge::Math
{
	/**
	 * @brief An axis-aligned bounding box.
	 * @ingroup group_math
	 *
	 * Starts empty, with Min at `FLT_MAX` and Max at `-FLT_MAX`, so Grow() can build it up.
	 */
	struct AABB
	{

		glm::vec3 Min = glm::vec3( FLT_MAX); ///< Smallest corner.
		glm::vec3 Max = glm::vec3(-FLT_MAX); ///< Largest corner.

		/**
		 * @brief Whether the box contains anything.
		 * @return `true` unless Min is above Max on some axis.
		 */
		bool IsValid() const { return Min.x <= Max.x && Min.y <= Max.y && Min.z <= Max.z; }

		/**
		 * @brief Grows the box to contain a point.
		 * @param point The point.
		 */
		void Grow(const glm::vec3& point)
		{

			Min = glm::min(Min, point);
			Max = glm::max(Max, point);

		}

		/**
		 * @brief Grows the box to contain another box. An empty box is ignored.
		 * @param other The other box.
		 */
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
		 * @brief The center.
		 * @return The center.
		 */
		glm::vec3 GetCenter()  const { return (Min + Max) * 0.5f; }
		/**
		 * @brief Half the size on each axis.
		 * @return The extents.
		 */
		glm::vec3 GetExtents() const { return (Max - Min) * 0.5f; }

		/**
		 * @brief The box around this one after a transform, by James Arvo's method.
		 * @param matrix The transform.
		 * @return The new box. An empty box is returned unchanged.
		 */
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