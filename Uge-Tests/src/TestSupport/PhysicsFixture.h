#pragma once

#include "Uge/Physics/Physics.h"
#include "Uge/Physics/PhysicsTypes.h"
#include "Uge/Physics/PhysicsScene.h"
#include <Uge/Core/Core.h>

#include <gtest/gtest.h>

namespace UgeTests
{
    /**
     * @brief Boots the physics backend for the duration of one test.
     *
     * Uge::Physics::Init registers Jolt's types process-wide and asserts if called twice,
     * so a test that creates a Uge::PhysicsScene has to bracket itself. No project and no
     * GL context are needed — physics touches neither.
     */
    class PhysicsFixture : public ::testing::Test
    {
    protected:
        void SetUp() override { Uge::Physics::Init(); }
        void TearDown() override { Uge::Physics::Shutdown(); }

        /// A scene with default limits; override desc fields before calling for specific ones.
        Uge::Scope<Uge::PhysicsScene> MakeScene( const Uge::PhysicsSceneDesc& desc = {} )
        {
            return Uge::Physics::CreateScene(desc);
        }
    };
}