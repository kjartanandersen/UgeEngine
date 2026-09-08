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

        // Corner index bit 0 = x, bit 1 = y, bit 2 = z. Two corners share an edge exactly when
        // their indices differ in a single bit, which is what the inner loop tests; the j > i
        // guard emits each of the 12 edges once rather than twice.
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

        // All three circles start on an axis at angle 0, so the XY and XZ circles share a
        // starting point and the YZ circle begins a quarter turn round.
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
        segments += segments & 1u;              // force even, so each cap arc splits cleanly

        const float step = glm::two_pi<float>() / (float)segments;

        // --- the two seam circles, in the XZ plane at each end of the cylinder ---
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

        // --- four verticals joining the seams, on the ±x and ±z axes ---
        const glm::vec2 axes[4] = { {  radius, 0.0f }, { -radius, 0.0f },
                                    { 0.0f,  radius }, { 0.0f, -radius } };
        for (const glm::vec2& a : axes)
        {
            out.DrawLine(ToWorld(transform, { a.x,  halfHeight, a.y }),
                ToWorld(transform, { a.x, -halfHeight, a.y }), color);
        }

        // --- the caps: a half-arc in XY and one in ZY at each end ---
        const uint32_t half = segments / 2;
        for (int side = 0; side < 2; side++)
        {
            const float y = (side == 0) ? halfHeight : -halfHeight;
            const float bulge = (side == 0) ? 1.0f : -1.0f;   // which way the dome points

            glm::vec3 prevXY = ToWorld(transform, { radius, y, 0.0f });
            glm::vec3 prevZY = ToWorld(transform, { 0.0f,   y, radius });

            for (uint32_t i = 1; i <= half; i++)
            {
                // 0 .. pi: sweeps the seam diameter while the sine lifts it over the pole.
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

}