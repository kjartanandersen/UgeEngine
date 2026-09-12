#pragma once

#include <glm/glm.hpp>

namespace Uge
{

    /**
     * @brief Line sink that Uge::PhysicsScene::DebugDraw writes into.
     * @ingroup group_physics
     *
     * Keeps the physics module free of any renderer dependency; the editor supplies an
     * implementation that forwards to Uge::Renderer2D::DrawLine.
     */
    class PhysicsDebugRenderer
    {
    public:
        virtual ~PhysicsDebugRenderer() = default;

        /**
         * @brief Draws one world-space line segment.
         * @param from Start point.
         * @param to End point.
         * @param color RGBA colour, components in `[0, 1]`.
         */
        virtual void DrawLine(const glm::vec3& from, const glm::vec3& to,
            const glm::vec4& color) = 0;
    };

}