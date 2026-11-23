/**

    @file      PhysicsTests.cpp
    @brief     Tests for the physics system.


**/

#include <ugpch.h>

#include <Uge/Physics/PhysicsTypes.h>

#include "TestSupport/PhysicsFixture.h"
#include <gtest/gtest.h>

namespace
{

	/** @brief Physics, started for each test. */
	class PhysicsTest : public UgeTests::PhysicsFixture
	{


	};

    TEST_F(PhysicsTest, MaxBodiesTest)
    {

		auto physicsScene = MakeScene();


    }

	// A query without the static layer doesn't report a static body.
	TEST_F(PhysicsTest, OverlapSphereHonoursTheLayerMask)
	{
		auto scene = MakeScene();

		Uge::BodyDesc floor;
		floor.Type = Uge::BodyType::Static;
		floor.Layer = Uge::PhysicsLayer::Static;
		Uge::ColliderDesc floorCollider;
		floorCollider.Shape = Uge::BoxShapeDesc{ glm::vec3(1.0f) };
		floor.Colliders.push_back(floorCollider);
		const Uge::PhysicsBodyID staticBody = scene->CreateBody(floor);

		Uge::BodyDesc ball;
		ball.Position = { 0.5f, 0.0f, 0.0f };
		Uge::ColliderDesc ballCollider;
		ballCollider.Shape = Uge::SphereShapeDesc{ 0.5f };
		ball.Colliders.push_back(ballCollider);
		const Uge::PhysicsBodyID movingBody = scene->CreateBody(ball);

		ASSERT_TRUE(staticBody.IsValid());
		ASSERT_TRUE(movingBody.IsValid());

		std::vector<Uge::PhysicsBodyID> hits;
		scene->OverlapSphere(glm::vec3(0.0f), 1.0f, 1u << (uint32_t)Uge::PhysicsLayer::Moving, hits);

		ASSERT_EQ(hits.size(), 1u);
		EXPECT_EQ(hits[0], movingBody);
	}

	// Triggers aren't reported, as with CastRay.
	TEST_F(PhysicsTest, OverlapSphereSkipsTriggers)
	{
		auto scene = MakeScene();

		Uge::BodyDesc trigger;
		trigger.Type = Uge::BodyType::Static;
		trigger.Layer = Uge::PhysicsLayer::Static;
		Uge::ColliderDesc collider;
		collider.Shape = Uge::BoxShapeDesc{ glm::vec3(1.0f) };
		collider.IsTrigger = true;
		trigger.Colliders.push_back(collider);
		ASSERT_TRUE(scene->CreateBody(trigger).IsValid());

		std::vector<Uge::PhysicsBodyID> hits;
		scene->OverlapSphere(glm::vec3(0.0f), 0.5f, Uge::PhysicsLayerMaskAll, hits);

		EXPECT_TRUE(hits.empty());
	}

	// A body touched through two colliders is reported once.
	TEST_F(PhysicsTest, OverlapSphereReportsEachBodyOnce)
	{
		auto scene = MakeScene();

		Uge::BodyDesc body;
		Uge::ColliderDesc left;
		left.Shape = Uge::BoxShapeDesc{ glm::vec3(0.5f) };
		left.Offset = { -0.6f, 0.0f, 0.0f };
		Uge::ColliderDesc right = left;
		right.Offset = { 0.6f, 0.0f, 0.0f };
		body.Colliders = { left, right };
		const Uge::PhysicsBodyID id = scene->CreateBody(body);
		ASSERT_TRUE(id.IsValid());

		std::vector<Uge::PhysicsBodyID> hits;
		scene->OverlapSphere(glm::vec3(0.0f), 1.0f, Uge::PhysicsLayerMaskAll, hits);

		ASSERT_EQ(hits.size(), 1u);
		EXPECT_EQ(hits[0], id);
	}

	// Points on one line can't make a hull, so there is no body, and no crash or assert.
	TEST_F(PhysicsTest, AConvexHullFromPointsOnALineCreatesNoBody)
	{
		auto scene = MakeScene();

		Uge::MeshShapeDesc line;
		line.Vertices = { { 0.0f, 0.0f, 0.0f },{ 1.0f, 0.0f, 0.0f },{ 2.0f, 0.0f, 0.0f },{ 3.0f, 0.0f, 0.0f } };
		line.Indices = { 0, 1, 2, 1, 2, 3 };
		line.Convex = true;

		Uge::BodyDesc body;
		Uge::ColliderDesc collider;
		collider.Shape = line;
		body.Colliders.push_back(collider);

		EXPECT_FALSE(scene->CreateBody(body).IsValid());
	}


	// Jolt doesn't wake a body resting on one that is removed, so DestroyBody does.
	TEST_F(PhysicsTest, DestroyingABodyWakesTheBodiesRestingOnIt)
	{
		auto scene = MakeScene();

		Uge::BodyDesc floor;
		floor.Type = Uge::BodyType::Static;
		floor.Layer = Uge::PhysicsLayer::Static;
		floor.Position = { 0.0f, -0.5f, 0.0f };
		Uge::ColliderDesc floorCollider;
		floorCollider.Shape = Uge::BoxShapeDesc{ glm::vec3(5.0f, 0.5f, 5.0f) };
		floor.Colliders.push_back(floorCollider);
		const Uge::PhysicsBodyID floorBody = scene->CreateBody(floor);

		Uge::BodyDesc box;
		box.Position = { 0.0f, 0.5f, 0.0f };
		Uge::ColliderDesc boxCollider;
		boxCollider.Shape = Uge::BoxShapeDesc{ glm::vec3(0.5f) };
		box.Colliders.push_back(boxCollider);
		const Uge::PhysicsBodyID boxBody = scene->CreateBody(box);

		// Two seconds, well past the half second before Jolt puts a resting body to sleep.
		for (int i = 0; i < 120; i++)
		{
			scene->Step(1.0f / 60.0f);
		}

		glm::vec3 restingPosition;
		glm::quat rotation;
		scene->GetTransform(boxBody, restingPosition, rotation);

		scene->DestroyBody(floorBody);
		for (int i = 0; i < 30; i++)
		{
			scene->Step(1.0f / 60.0f);
		}

		// Half a second of free fall is over a metre.
		glm::vec3 position;
		scene->GetTransform(boxBody, position, rotation);
		EXPECT_LT(position.y, restingPosition.y - 0.5f);
	}

}