#include "ugpch.h"
#include "ColliderWireframe.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

namespace Uge::ColliderWireframe
{

	namespace
	{
		/// Transforms a shape-local point into world space.
		glm::vec3 ToWorld(const glm::mat4& transform, const glm::vec3& point)
		{
			return glm::vec3(transform * glm::vec4(point, 1.0f));
		}
	}


	void DrawBox(PhysicsDebugRenderer& out, const glm::mat4& transform, const glm::vec3& halfExtents, const glm::vec4& color)
	{

        // Corner index bits are x, y and z. Corners one bit apart share an edge, and j > i
        // draws each edge once.
        glm::vec3 corners[8];
        for (int i = 0; i < 8; i++)
        {
            const glm::vec3 sign{ (i & 1) ? 1.0f : -1.0f,
                                  (i & 2) ? 1.0f : -1.0f,
                                  (i & 4) ? 1.0f : -1.0f };
            corners[i] = ToWorld(transform, sign * halfExtents);
        }

        for (int i = 0; i < 8; i++)
        {
            for (int bit = 1; bit < 8; bit <<= 1)
            {
                const int j = i ^ bit;
                if (j > i)
                    out.DrawLine(corners[i], corners[j], color);
            }
        }

	}

	void DrawSphere(PhysicsDebugRenderer& out, const glm::mat4& transform, float radius, const glm::vec4& color, uint32_t segments)
	{
        if (segments < 3)
            segments = 3;

        const float step = glm::two_pi<float>() / (float)segments;

        // The XY and XZ circles start at +x, the YZ circle at +y.
        glm::vec3 prevXY = ToWorld(transform, { radius, 0.0f, 0.0f });
        glm::vec3 prevXZ = prevXY;
        glm::vec3 prevYZ = ToWorld(transform, { 0.0f, radius, 0.0f });

        for (uint32_t i = 1; i <= segments; i++)
        {
            const float angle = step * (float)i;
            const float c = glm::cos(angle) * radius;
            const float s = glm::sin(angle) * radius;

            const glm::vec3 curXY = ToWorld(transform, { c,    s,    0.0f });
            const glm::vec3 curXZ = ToWorld(transform, { c,    0.0f, s });
            const glm::vec3 curYZ = ToWorld(transform, { 0.0f, c,    s });

            out.DrawLine(prevXY, curXY, color);
            out.DrawLine(prevXZ, curXZ, color);
            out.DrawLine(prevYZ, curYZ, color);

            prevXY = curXY;
            prevXZ = curXZ;
            prevYZ = curYZ;
        }
	}

	void DrawCapsule(PhysicsDebugRenderer& out, const glm::mat4& transform, float radius, float halfHeight, const glm::vec4& color, uint32_t segments)
	{
        if (segments < 4)
        {
            segments = 4;
        }
        segments += segments & 1u;              // even, so each cap gets half

        const float step = glm::two_pi<float>() / (float)segments;

        // The seam circles at each end of the cylinder, in the XZ plane.
        for (int side = 0; side < 2; side++)
        {
            const float y = (side == 0) ? halfHeight : -halfHeight;

            glm::vec3 prev = ToWorld(transform, { radius, y, 0.0f });
            for (uint32_t i = 1; i <= segments; i++)
            {
                const float angle = step * (float)i;
                const glm::vec3 cur = ToWorld(transform,
                    { glm::cos(angle) * radius, y, glm::sin(angle) * radius });

                out.DrawLine(prev, cur, color);
                prev = cur;
            }
        }

        // Four lines joining the seams, at +x, -x, +z and -z.
        const glm::vec2 axes[4] = { {  radius, 0.0f }, { -radius, 0.0f },
                                    { 0.0f,  radius }, { 0.0f, -radius } };
        for (const glm::vec2& a : axes)
        {
            out.DrawLine(ToWorld(transform, { a.x,  halfHeight, a.y }),
                ToWorld(transform, { a.x, -halfHeight, a.y }), color);
        }

        // The caps: a half-arc in XY and one in ZY at each end.
        const uint32_t half = segments / 2;
        for (int side = 0; side < 2; side++)
        {
            const float y = (side == 0) ? halfHeight : -halfHeight;
            const float bulge = (side == 0) ? 1.0f : -1.0f;   // which way the dome points

            glm::vec3 prevXY = ToWorld(transform, { radius, y, 0.0f });
            glm::vec3 prevZY = ToWorld(transform, { 0.0f,   y, radius });

            for (uint32_t i = 1; i <= half; i++)
            {
                // 0 to pi: across the seam and over the pole.
                const float angle = glm::pi<float>() * (float)i / (float)half;
                const float c = glm::cos(angle) * radius;
                const float s = glm::sin(angle) * radius * bulge;

                const glm::vec3 curXY = ToWorld(transform, { c,    y + s, 0.0f });
                const glm::vec3 curZY = ToWorld(transform, { 0.0f, y + s, c });

                out.DrawLine(prevXY, curXY, color);
                out.DrawLine(prevZY, curZY, color);

                prevXY = curXY;
                prevZY = curZY;
            }
        }
	}

    bool BuildEdges(const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices, 
        std::vector<glm::vec3>& outEdges, uint32_t maxEdges)
    {
        outEdges.clear();

        if (vertices.empty())
        {
            return true;
        }

        std::unordered_set<uint64_t> seen;
        seen.reserve(indices.size());
        outEdges.reserve((size_t)maxEdges * 2);

        uint32_t edgeCount = 0;
        for (size_t i = 0; i + 2 < indices.size(); i += 3)
        {
            const uint32_t tri[3] = { indices[i], indices[i + 1], indices[i + 2] };
            for (int e = 0; e < 3; e++)
            {
                const uint32_t a = tri[e], b = tri[(e + 1) % 3];
                if (a == b || a >= vertices.size() || b >= vertices.size())
                    continue;                                   // degenerate or out of range

                const uint64_t key = ((uint64_t)std::min(a, b) << 32) | std::max(a, b);
                if (!seen.insert(key).second)
                    continue;                                   // already emitted

                if (edgeCount >= maxEdges)
                    return false;                               // over maxEdges

                outEdges.push_back(vertices[a]);
                outEdges.push_back(vertices[b]);
                edgeCount++;
            }
        }

        return true;
    }

    void DrawEdges(PhysicsDebugRenderer& out, const glm::mat4& transform, 
        const std::vector<glm::vec3>& edges, const glm::vec4& color)
    {
        for (size_t i = 0; i + 1 < edges.size(); i += 2)
        {
            out.DrawLine(ToWorld(transform, edges[i]), ToWorld(transform, edges[i+1]), color);
        }

    }

}