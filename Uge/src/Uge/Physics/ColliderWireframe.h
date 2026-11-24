/**
 * @file ColliderWireframe.h
 * @brief Wireframes of the collider shapes.
 * @ingroup group_physics
 */

#pragma once

#include "PhysicsDebugRenderer.h"

#include <vector>
#include <cstdint>

#include <glm/glm.hpp>


/**
 * @brief Draws the collider shapes as line segments. Needs no GL context.
 * @ingroup group_physics
 *
 * Vertices are built in shape-local space and transformed by the caller's matrix. Compose it
 * as `tc.GetTransform() * glm::translate(glm::mat4(1.0f), collider.Offset)`, so the offset
 * scales with the entity; the other order drifts once the entity is scaled.
 */
namespace Uge::ColliderWireframe
{

    /**
     * @brief Draws a box's twelve edges.
     * @param out Receives the lines.
     * @param transform Shape-local to world.
     * @param halfExtents Half the size on each axis.
     * @param color RGBA colour.
     * @ingroup group_physics
     */
    void DrawBox(PhysicsDebugRenderer& out, const glm::mat4& transform,
        const glm::vec3& halfExtents, const glm::vec4& color);

    /**
     * @brief Draws a sphere as a circle in each of the local XY, XZ and YZ planes.
     * @param out Receives the lines.
     * @param transform Shape-local to world.
     * @param radius Radius.
     * @param color RGBA colour.
     * @param segments Segments per circle; at least `3`.
     * @ingroup group_physics
     */
    void DrawSphere(PhysicsDebugRenderer& out, const glm::mat4& transform,
        float radius, const glm::vec4& color, uint32_t segments = 24);

    /**
     * @brief Draws a capsule along the local Y axis.
     * @param out Receives the lines.
     * @param transform Shape-local to world.
     * @param radius Cap radius.
     * @param halfHeight Half the length of the cylinder, without the caps.
     * @param color RGBA colour.
     * @param segments Segments per circle; rounded up to an even number, at least `4`.
     * @ingroup group_physics
     */
    void DrawCapsule(PhysicsDebugRenderer& out, const glm::mat4& transform,
        float radius, float halfHeight, const glm::vec4& color, uint32_t segments = 24);


    /**
     * @brief Collects each triangle edge of a mesh once, in shape-local space.
     * @param vertices Vertex positions.
     * @param indices Three per triangle.
     * @param outEdges Receives two endpoints per edge; cleared first.
     * @param maxEdges Most edges to collect.
     * @return `false` if the mesh has more than @p maxEdges edges.
     *
     * Edges are matched by index, so vertices split along a seam give that seam twice.
     */
    bool BuildEdges(const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices,
        std::vector<glm::vec3>& outEdges, uint32_t maxEdges = 20000);

    /**
     * @brief Draws an edge list from BuildEdges().
     * @param out Receives the lines.
     * @param transform Shape-local to world.
     * @param edges Endpoint pairs.
     * @param color RGBA colour.
     */
    void DrawEdges(PhysicsDebugRenderer& out, const glm::mat4& transform,
        const std::vector<glm::vec3>& edges, const glm::vec4& color);

}
