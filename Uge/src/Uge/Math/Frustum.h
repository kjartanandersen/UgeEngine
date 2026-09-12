#pragma once

#include "Uge/Math/AABB.h"

#include <glm/glm.hpp>

namespace Uge::Math
{

	/**
	   * @brief A plane in the form `dot(Normal, p) + Distance == 0`.
	   * @ingroup group_math
	   *
	   * Frustum planes face **inward**, so a positive signed distance means the point
	   * is on the visible side. A default-constructed plane is all zeroes and reports
	   * every point as lying exactly on it, which reads as inside.
	   */
	struct Plane
	{

		glm::vec3 Normal = glm::vec3(0.0f); ///< Unit normal, pointing into the frustum.
		float Distance = 0.0f;				///< Signed distance from the origin along Normal.

		/**
			   * @brief Signed distance from the plane to a point.
			   * @param point Point to measure, in the same space as the plane.
			   * @return Positive in front of the plane, negative behind it, zero on it.
		*/
		float SignedDistance(const glm::vec3& point) const
		{

			return glm::dot(Normal, point) + Distance;

		}

	};

	/**
	   * @brief The six clipping planes bounding a camera's view volume.
	   * @ingroup group_math
	   *
	   * Built with FromViewProjection() and tested against world-space bounds with
	   * Intersects(). A default-constructed Frustum holds six zeroed planes and
	   * therefore accepts everything, which is the safe state before extraction runs.
	   */
	class Frustum
	{
	public:
		/** @brief Index of each plane within the frustum. */
		enum Side
		{
			Left = 0, ///< Left clipping plane.
			Right,    ///< Right clipping plane.
			Bottom,   ///< Bottom clipping plane.
			Top,      ///< Top clipping plane.
			Near,     ///< Near clipping plane.
			Far,      ///< Far clipping plane.
			Count     ///< Number of planes.
		};

		/**
		 * @brief Extracts the six world-space planes from a combined view-projection matrix.
		 * @param viewProjection The matrix handed to Model::BeginScene, `projection * view`.
		 * @return A frustum whose planes are in world space and face inward.
		 */
		static Frustum FromViewProjection(const glm::mat4& viewProjection);

		/**
		 * @brief Tests whether a world-space box is potentially visible.
		 * @param bounds Box to test, already transformed into world space.
		 * @retval true The box is inside or straddles the frustum, or has invalid bounds.
		 * @retval false The box lies entirely outside at least one plane.
		 */
		bool Intersects(const AABB& bounds) const;

		/**
		 * @brief Reads a single plane, for debugging and visualisation.
		 * @param side Which plane to read.
		 * @return The requested plane.
		 */
		const Plane& GetPlane(Side side) const { return m_planes[side]; }

	private:
		Plane m_planes[Count]; ///< The six inward-facing planes.
	};


}