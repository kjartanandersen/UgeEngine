/**
 * @file ColliderWireframeTests.cpp
 * @brief Tests for the collider outlines the editor draws over the viewport.
 */

#include <ugpch.h>

#include "Uge/Physics/ColliderWireframe.h"
#include "Uge/Physics/PhysicsDebugRenderer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <gtest/gtest.h>

#include <utility>
#include <vector>

namespace
{
	/** @brief Records every line segment it is given. */
	class LineRecorder : public Uge::PhysicsDebugRenderer
	{
	public:
		void DrawLine(const glm::vec3& from, const glm::vec3& to, const glm::vec4&) override
		{
			Lines.emplace_back(from, to);
		}

		std::vector<std::pair<glm::vec3, glm::vec3>> Lines;
	};

	const glm::vec4 k_white(1.0f);

	/** @brief Distance from a point to the line segment between @p a and @p b. */
	float DistanceToSegment(const glm::vec3& point, const glm::vec3& a, const glm::vec3& b)
	{
		const glm::vec3 ab = b - a;
		const float t = glm::clamp(glm::dot(point - a, ab) / glm::dot(ab, ab), 0.0f, 1.0f);
		return glm::length(point - (a + t * ab));
	}

	/** @brief A unit square made of two triangles that share a diagonal. */
	const std::vector<glm::vec3> k_squareVertices = {
		{ 0.0f, 0.0f, 0.0f }, 
		{ 1.0f, 0.0f, 0.0f },
		{ 1.0f, 1.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f } 
	};

	const std::vector<uint32_t> k_squareIndices = { 0, 1, 2, 2, 3, 0 };
}

TEST(ColliderWireframeTest, ABoxIsTwelveEdgesBetweenItsCorners)
{
	LineRecorder lines;
	const glm::vec3 halfExtents(1.0f, 2.0f, 3.0f);
	Uge::ColliderWireframe::DrawBox(lines, glm::mat4(1.0f), halfExtents, k_white);

	ASSERT_EQ(lines.Lines.size(), 12u);
	for (const auto& [from, to] : lines.Lines)
	{
		for (const glm::vec3& point : { from, to })
		{
			EXPECT_NEAR(std::abs(point.x), halfExtents.x, 1e-5f);
			EXPECT_NEAR(std::abs(point.y), halfExtents.y, 1e-5f);
			EXPECT_NEAR(std::abs(point.z), halfExtents.z, 1e-5f);
		}
	}
}

TEST(ColliderWireframeTest, TheTransformPlacesTheOutline)
{
	LineRecorder lines;
	const glm::vec3 position(10.0f, -5.0f, 2.0f);
	Uge::ColliderWireframe::DrawBox(lines, glm::translate(glm::mat4(1.0f), position), glm::vec3(1.0f), k_white);

	ASSERT_EQ(lines.Lines.size(), 12u);
	for (const auto& [from, to] : lines.Lines)
	{
		for (const glm::vec3& point : { from, to })
		{
			EXPECT_NEAR(std::abs(point.x - position.x), 1.0f, 1e-5f);
			EXPECT_NEAR(std::abs(point.y - position.y), 1.0f, 1e-5f);
			EXPECT_NEAR(std::abs(point.z - position.z), 1.0f, 1e-5f);
		}
	}
}

TEST(ColliderWireframeTest, ASphereIsThreeRingsOnItsSurface)
{
	LineRecorder lines;
	const float radius = 2.5f;
	Uge::ColliderWireframe::DrawSphere(lines, glm::mat4(1.0f), radius, k_white, 16);

	ASSERT_EQ(lines.Lines.size(), 3u * 16u);
	for (const auto& [from, to] : lines.Lines)
	{
		EXPECT_NEAR(glm::length(from), radius, 1e-4f);
		EXPECT_NEAR(glm::length(to), radius, 1e-4f);
	}
}

TEST(ColliderWireframeTest, ACapsuleOutlineLiesOnItsSurface)
{
	// Every point on a capsule's surface is exactly one radius from its core: the segment
	// between the centres of its two caps.
	LineRecorder lines;
	const float radius = 0.5f;
	const float halfHeight = 1.5f;
	Uge::ColliderWireframe::DrawCapsule(lines, glm::mat4(1.0f), radius, halfHeight, k_white, 16);

	ASSERT_EQ(lines.Lines.size(), 4u * 16u + 4u);
	const glm::vec3 top(0.0f, halfHeight, 0.0f);
	const glm::vec3 bottom(0.0f, -halfHeight, 0.0f);
	for (const auto& [from, to] : lines.Lines)
	{
		EXPECT_NEAR(DistanceToSegment(from, bottom, top), radius, 1e-4f);
		EXPECT_NEAR(DistanceToSegment(to, bottom, top), radius, 1e-4f);
	}
}

TEST(ColliderWireframeTest, SegmentCountsAreRaisedToUsableValues)
{
	// A ring needs at least three segments. A capsule uses an even count of at least four,
	// so each cap's half-arc is symmetric.
	LineRecorder sphere;
	Uge::ColliderWireframe::DrawSphere(sphere, glm::mat4(1.0f), 1.0f, k_white, 1);
	EXPECT_EQ(sphere.Lines.size(), 3u * 3u);

	LineRecorder capsule;
	Uge::ColliderWireframe::DrawCapsule(capsule, glm::mat4(1.0f), 0.5f, 1.0f, k_white, 5);
	EXPECT_EQ(capsule.Lines.size(), 4u * 6u + 4u);
}

TEST(ColliderWireframeTest, AnEdgeSharedByTwoTrianglesIsCollectedOnce)
{
	// Two triangles forming a square: four sides and one shared diagonal.
	std::vector<glm::vec3> edges;
	EXPECT_TRUE(Uge::ColliderWireframe::BuildEdges(k_squareVertices, k_squareIndices, edges));
	EXPECT_EQ(edges.size(), 5u * 2u);
}

TEST(ColliderWireframeTest, EdgeCollectionStopsAtTheBudget)
{
	std::vector<glm::vec3> edges;
	EXPECT_FALSE(Uge::ColliderWireframe::BuildEdges(k_squareVertices, k_squareIndices, edges, 3));
	EXPECT_EQ(edges.size(), 3u * 2u);
}