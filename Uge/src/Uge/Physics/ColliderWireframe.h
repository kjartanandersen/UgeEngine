#pragma once

#include "PhysicsDebugRenderer.h"

#include <glm/glm.hpp>

namespace Uge::ColliderWireframe
{

    void DrawBox(PhysicsDebugRenderer& out, const glm::mat4& transform,
        const glm::vec3& halfExtents, const glm::vec4& color);

    void DrawSphere(PhysicsDebugRenderer& out, const glm::mat4& transform,
        float radius, const glm::vec4& color, uint32_t segments = 24);

    void DrawCapsule(PhysicsDebugRenderer& out, const glm::mat4& transform,
        float radius, float halfHeight, const glm::vec4& color, uint32_t segments = 24);


}