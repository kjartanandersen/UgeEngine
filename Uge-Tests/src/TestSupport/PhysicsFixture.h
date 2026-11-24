#pragma once

#include "Uge/Physics/Physics.h"
#include "Uge/Physics/PhysicsTypes.h"
#include "Uge/Physics/PhysicsScene.h"
#include <Uge/Core/Core.h>

#include <gtest/gtest.h>

namespace UgeTests
{
    /**
     * @brief Starts physics for one test. Physics needs no project or GL context.
     */
    class PhysicsFixture : public ::testing::Test
    {
    protected:
        void SetUp() override { Uge::Physics::Init(); }
        void TearDown() override { Uge::Physics::Shutdown(); }

        /// A physics scene, with default limits unless desc sets others.
        Uge::Scope<Uge::PhysicsScene> MakeScene( const Uge::PhysicsSceneDesc& desc = {} )
        {
            return Uge::Physics::CreateScene(desc);
        }
    };
}