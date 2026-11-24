#pragma once

#include "Uge/Physics/PhysicsDebugRenderer.h"

#ifdef JPH_DEBUG_RENDERER

#include "Uge/Physics/Jolt/JoltUtils.h"

#include <Jolt/Jolt.h>
#include <Jolt/Renderer/DebugRendererSimple.h>

namespace Uge
{

    /**
     * @brief Sends Jolt's debug drawing to a Uge::PhysicsDebugRenderer.
     *
     * Jolt asserts on a second debug renderer, so exactly one exists. Uge::JoltData owns it,
     * since two scenes can be alive at once.
     */
    class JoltDebugRenderer : public JPH::DebugRendererSimple
    {
    public:
        /**
         * @brief Sets where lines go.
         * @param sink The sink, or `nullptr` to discard them.
         */
        void SetSink(PhysicsDebugRenderer* sink) { m_sink = sink; }

        /**
         * @brief Draws a line.
         * @param inFrom Start.
         * @param inTo End.
         * @param inColor Colour.
         */
        void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override
        {
            if (m_sink == nullptr)
            {
                return;

            }

            m_sink->DrawLine(FromJolt(inFrom), FromJolt(inTo), FromJolt(inColor.ToVec4()));
        }

        /**
         * @brief Draws a triangle's edges.
         * @param inV1 First vertex.
         * @param inV2 Second vertex.
         * @param inV3 Third vertex.
         * @param inColor Colour.
         * @param inCastShadow Ignored.
         */
        void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3,
            JPH::ColorArg inColor, ECastShadow inCastShadow) override
        {
            if (m_sink == nullptr)
                return;

            const glm::vec4 color = FromJolt(inColor.ToVec4());
            const glm::vec3 v1 = FromJolt(inV1), v2 = FromJolt(inV2), v3 = FromJolt(inV3);

            m_sink->DrawLine(v1, v2, color);
            m_sink->DrawLine(v2, v3, color);
            m_sink->DrawLine(v3, v1, color);
        }

        /**
         * @brief Does nothing; only lines are drawn.
         * @param inPosition Ignored.
         * @param inString Ignored.
         * @param inColor Ignored.
         * @param inHeight Ignored.
         */
        void DrawText3D(JPH::RVec3Arg inPosition, const std::string_view& inString,
            JPH::ColorArg inColor, float inHeight) override
        {
        }

    private:
        PhysicsDebugRenderer* m_sink = nullptr;
    };

}

#endif // JPH_DEBUG_RENDERER