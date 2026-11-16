#include "ugpch.h"
#include "Frustum.h"

namespace Uge::Math
{
    namespace
    {
        // GLM indexes [column][row].
        glm::vec4 MatrixRow(const glm::mat4& matrix, int row)
        {
            return glm::vec4(matrix[0][row], matrix[1][row], matrix[2][row], matrix[3][row]);
        }

        // Normalized, so SignedDistance is in world units. A degenerate row leaves the plane
        // zeroed, which accepts everything.
        Plane MakePlane(const glm::vec4& coefficients)
        {
            Plane plane;

            const glm::vec3 normal(coefficients);
            const float length = glm::length(normal);

            if (length <= 0.0f)
            {
                return plane;
            }

            plane.Normal = normal / length;
            plane.Distance = coefficients.w / length;
            return plane;
        }
    }

    Frustum Frustum::FromViewProjection(const glm::mat4& viewProjection)
    {
        const glm::vec4 rowX = MatrixRow(viewProjection, 0);
        const glm::vec4 rowY = MatrixRow(viewProjection, 1);
        const glm::vec4 rowZ = MatrixRow(viewProjection, 2);
        const glm::vec4 rowW = MatrixRow(viewProjection, 3);

        Frustum frustum;
        frustum.m_planes[Left]      = MakePlane(rowW + rowX);
        frustum.m_planes[Right]     = MakePlane(rowW - rowX);
        frustum.m_planes[Bottom]    = MakePlane(rowW + rowY);
        frustum.m_planes[Top]       = MakePlane(rowW - rowY);
        frustum.m_planes[Near]      = MakePlane(rowW + rowZ); // OpenGL clip space, depth [-1, 1].
        frustum.m_planes[Far]       = MakePlane(rowW - rowZ);

        return frustum;
    }

    bool Frustum::Intersects(const AABB& bounds) const
    {
        // Empty bounds count as visible: a missing mesh is harder to spot than an extra draw.
        if (!bounds.IsValid())
        {
            return true;
        }

        for (const Plane& plane : m_planes)
        {
            // The corner furthest along the normal. If it is behind the plane, so is the box.
            const glm::vec3 positiveVertex(
                plane.Normal.x >= 0.0f ? bounds.Max.x : bounds.Min.x,
                plane.Normal.y >= 0.0f ? bounds.Max.y : bounds.Min.y,
                plane.Normal.z >= 0.0f ? bounds.Max.z : bounds.Min.z);

            if (plane.SignedDistance(positiveVertex) < 0.0f)
            {
                return false;
            }
        }

        return true;
    }
}
