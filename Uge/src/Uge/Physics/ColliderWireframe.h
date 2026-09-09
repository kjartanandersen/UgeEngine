/**
 * @file ColliderWireframe.h
 * @brief Line-segment wireframes for the collider components.
 * @ingroup group_physics
 */

#pragma once

#include "PhysicsDebugRenderer.h"

#include <vector>
#include <cstdint>

#include <glm/glm.hpp>


/**
 * @brief Builds wireframe outlines for the collider shapes and emits them as line segments.
 * @ingroup group_physics
 *
 * Pure geometry. It depends only on GLM and Uge::PhysicsDebugRenderer, never on the renderer,
 * which is what lets it live inside the physics module. The editor supplies a sink that
 * forwards into Uge::Renderer2D; a test supplies one that records segments into a vector, so
 * none of this needs a GL context.
 *
 * Every function generates its vertices in **shape-local space**, centred on the origin, and
 * transforms each one by the caller's matrix. Compose that matrix as
 * `tc.GetTransform() * glm::translate(glm::mat4(1.0f), collider.Offset)`, which is how
 * Uge::JoltScene builds a body: the collider offset is applied inside the shape and the entity
 * scale is baked over the whole assembly afterwards, so the offset scales with the entity.
 * Composing it the other way round looks correct at a scale of `1` and drifts as soon as an
 * entity is scaled.
 */
namespace Uge::ColliderWireframe
{

    /**
     * @brief Emits the twelve edges of a box.
     * @param out Sink receiving the line segments.
     * @param transform Maps shape-local space to world space.
     * @param halfExtents Half-size on each local axis, as in Uge::BoxColliderComponent.
     * @param color RGBA colour, components in `[0, 1]`.
     * @ingroup group_physics
     */
    void DrawBox(PhysicsDebugRenderer& out, const glm::mat4& transform,
        const glm::vec3& halfExtents, const glm::vec4& color);

    /**
     * @brief Emits three great circles approximating a sphere.
     * @param out Sink receiving the line segments.
     * @param transform Maps shape-local space to world space.
     * @param radius Sphere radius, before @p transform is applied.
     * @param color RGBA colour, components in `[0, 1]`.
     * @param segments Segments per circle; clamped to a minimum of `3`.
     * @ingroup group_physics
     *
     * Emits `3 * segments` segments — one ring in each of the local XY, XZ and YZ planes.
     *
     * @note A non-uniformly scaled @p transform skews the rings into ellipses. That is
     * accurate rather than a defect: Jolt rejects a non-uniformly scaled sphere outright, so
     * such a collider produces no body at all.
     */
    void DrawSphere(PhysicsDebugRenderer& out, const glm::mat4& transform,
        float radius, const glm::vec4& color, uint32_t segments = 24);

    /**
     * @brief Emits the outline of a capsule aligned to the local Y axis.
     * @param out Sink receiving the line segments.
     * @param transform Maps shape-local space to world space.
     * @param radius Radius of the hemispherical caps.
     * @param halfHeight Half the length of the **cylindrical section only**, excluding the
     *        caps; the total height is `2 * (halfHeight + radius)`.
     * @param color RGBA colour, components in `[0, 1]`.
     * @param segments Segments per full circle; rounded up to an even number and clamped to a
     *        minimum of `4`.
     * @ingroup group_physics
     *
     * Emits `4 * segments + 4` segments: a seam circle at each end of the cylinder, four
     * verticals joining them, and two half-arcs over each cap.
     *
     * @note A @p halfHeight of `0` collapses to a sphere-like figure rather than
     * misbehaving — the seams coincide and the zero-length verticals draw nothing.
     */
    void DrawCapsule(PhysicsDebugRenderer& out, const glm::mat4& transform,
        float radius, float halfHeight, const glm::vec4& color, uint32_t segments = 24);


    /**
     * @brief Collects each unique triangle edge of a mesh, in shape-local space.
     * @param vertices Triangle vertex positions.
     * @param indices Three per triangle, indexing @p vertices.
     * @param outEdges Receives endpoint pairs — two entries per edge. Cleared first.
     * @param maxEdges Budget; collection stops once this many edges have been emitted.
     * @return `true` if every edge fit within @p maxEdges.
     *
     * Edges are deduplicated by index pair, so a mesh with vertices split along UV or
     * smoothing seams still emits its seam edges twice. Build once and cache; the result
     * depends only on the geometry, not on the transform.
     */
    bool BuildEdges(const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices,
        std::vector<glm::vec3>& outEdges, uint32_t maxEdges = 20000);

    /**
     * @brief Emits a prebuilt edge list.
     * @param out Sink receiving the line segments.
     * @param transform Maps shape-local space to world space.
     * @param edges Endpoint pairs from BuildEdges().
     * @param color RGBA colour, components in `[0, 1]`.
     */
    void DrawEdges(PhysicsDebugRenderer& out, const glm::mat4& transform,
        const std::vector<glm::vec3>& edges, const glm::vec4& color);

}
