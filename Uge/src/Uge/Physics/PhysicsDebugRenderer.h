#pragma once

#include <glm/glm.hpp>

namespace Uge
{

    /**
     * @brief Receives the lines of physics debug drawing, so physics needs no renderer.
     * @ingroup group_physics
     */
    class PhysicsDebugRenderer
    {
    public:
        virtual ~PhysicsDebugRenderer() = default;

        /**
         * @brief Draws a world-space line.
         * @param from Start.
         * @param to End.
         * @param color RGBA colour.
         */
        virtual void DrawLine(const glm::vec3& from, const glm::vec3& to,
            const glm::vec4& color) = 0;
    };

}