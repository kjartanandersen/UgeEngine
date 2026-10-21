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

	// A query that excludes the static layer must not report a static body.
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

	// Triggers aren't reported, matching CastRay.
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

	// A compound body touched through two colliders is still reported once.
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

	// Points on one line can't make a hull at any tolerance. That has to come back as "no
	// body", not a crash or an assert.
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

}