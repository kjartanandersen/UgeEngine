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

	/** @brief Console panel over a buffer that starts and ends empty. */
	class PhysicsTest : public UgeTests::PhysicsFixture
	{
	protected:
		virtual void SetUp() override
		{
			UgeTests::PhysicsFixture::SetUp();
		}

		virtual void TearDown() override
		{
			UgeTests::PhysicsFixture::TearDown();
		}

	};

    TEST_F(PhysicsTest, MaxBodiesTest)
    {
		SetUp();
        
		Uge::PhysicsSceneDesc desc;

		

		auto physicsScene = MakeScene();

		TearDown();

    }

}