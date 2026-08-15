#pragma once

#include "Uge/Physics/PhysicsDebugRenderer.h"

#ifdef JPH_DEBUG_RENDERER

#include "Uge/Physics/Jolt/JoltUtils.h"

#include <Jolt/Jolt.h>
#include <Jolt/Renderer/DebugRendererSimple.h>

namespace Uge
{

    /**
     * @brief Adapts Jolt's debug renderer onto a Uge::PhysicsDebugRenderer line sink.
     *
     * @warning JPH::DebugRenderer keeps a process-wide `sInstance` and asserts if a second
     * one is constructed, so exactly one of these may exist. It is owned by Uge::JoltData,
     * not by a scene: the editor world and the play-mode world are alive at the same time.
     * Uge::JoltScene::DebugDraw points it at a sink for the duration of one call.
     */
    class JoltDebugRenderer : public JPH::DebugRendererSimple
    {
    public:
        /**
         * @brief Points the renderer at the sink to forward into.
         * @param sink Sink for subsequent draw calls, or `nullptr` to discard them.
         */
        void SetSink(PhysicsDebugRenderer* sink) { m_sink = sink; }

        /**
         * @brief Forwards one line to the current sink.
         * @param inFrom Start point.
         * @param inTo End point.
         * @param inColor Line colour.
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
         * @brief Forwards a triangle as its three edges.
         * @param inV1 First vertex.
         * @param inV2 Second vertex.
         * @param inV3 Third vertex.
         * @param inColor Triangle colour.
         * @param inCastShadow Ignored; the sink draws lines only.
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
         * @brief No-op: Uge::PhysicsDebugRenderer draws lines only.
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