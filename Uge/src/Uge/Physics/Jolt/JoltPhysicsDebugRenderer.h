#pragma once

#include "Uge/Physics/PhysicsDebugRenderer.h"

#include "Uge/Physics/Jolt/JoltUtils.h"

#include <Jolt/Jolt.h>
#include <Jolt/Renderer/DebugRendererSimple.h>


namespace Uge
{

    class JoltPhysicsDebugRenderer : public PhysicsDebugRenderer
    {
    public:
        /**
         * @brief Draws one world-space line segment.
         * @param from Start point.
         * @param to End point.
         * @param color RGBA colour, components in `[0, 1]`.
         */
        virtual void DrawLine(const glm::vec3& from, const glm::vec3& to,
            const glm::vec4& color) override
        {

            //TODO: Draw Line

        }

    };

    class DebugRendererImpl : public JPH::DebugRendererSimple
    {
    public:
        explicit DebugRendererImpl() = default;
        virtual void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override
        {

            

            m_debugRenderer.DrawLine(FromJolt(inFrom), FromJolt(inTo), FromJolt(inColor.ToVec4()));

        }

    private:
        JoltPhysicsDebugRenderer m_debugRenderer;
    };

	

}