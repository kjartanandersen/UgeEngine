#pragma once

#include "Uge/Math/AABB.h"

#include <glm/glm.hpp>

namespace Uge::Math
{

	/**
	 * @brief A plane, `dot(Normal, p) + Distance == 0`.
	 * @ingroup group_math
	 *
	 * A zeroed plane counts every point as on it, which reads as inside.
	 */
	struct Plane
	{

		glm::vec3 Normal = glm::vec3(0.0f); ///< Unit normal; frustum planes face inward.
		float Distance = 0.0f;				///< Signed distance from the origin along Normal.

		/**
		 * @brief Signed distance to a point.
		 * @param point The point.
		 * @return Positive in front of the plane, negative behind, zero on it.
		 */
		float SignedDistance(const glm::vec3& point) const
		{

			return glm::dot(Normal, point) + Distance;

		}

	};

	/**
	 * @brief A camera's six clipping planes.
	 * @ingroup group_math
	 *
	 * A default frustum accepts everything.
	 */
	class Frustum
	{
	public:
		/** @brief Plane indices. */
		enum Side
		{
			Left = 0,
			Right,
			Bottom,
			Top,
			Near,
			Far,
			Count     ///< Number of planes.
		};

		/**
		 * @brief Extracts the world-space planes from a view-projection matrix.
		 * @param viewProjection `projection * view`.
		 * @return The frustum, with planes facing inward.
		 */
		static Frustum FromViewProjection(const glm::mat4& viewProjection);

		/**
		 * @brief Whether a box may be visible.
		 * @param bounds The box, in world space.
		 * @return `false` only if the box is entirely behind some plane. An empty box gives
		 * `true`.
		 */
		bool Intersects(const AABB& bounds) const;

		/**
		 * @brief One plane.
		 * @param side Which plane.
		 * @return The plane.
		 */
		const Plane& GetPlane(Side side) const { return m_planes[side]; }

	private:
		Plane m_planes[Count]; ///< The planes, facing inward.
	};


}