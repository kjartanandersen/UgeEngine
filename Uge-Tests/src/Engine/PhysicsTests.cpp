/**

    @file      PhysicsTests.cpp
    @brief     Tests for the physics System


**/

#include <ugpch.h>

#include <Uge/Physics/PhysicsTypes.h>

#include "TestSupport/PhysicsFixture.h"
#include <gtest/gtest.h>

namespace
{

	/** @brief Physics initialized for the duration of each test. */
	class PhysicsTest : public UgeTests::PhysicsFixture
	{


	};

    TEST_F(PhysicsTest, MaxBodiesTest)
    {

		auto physicsScene = MakeScene();


    }

}