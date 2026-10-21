/**
 * @file MathTests.cpp
 * @brief Tests for Uge::Math::DecomposeTransform, the gizmo's write-back path.
 */

#include <ugpch.h>

#include "Uge/Math/Math.h"
#include "Uge/Scene/Components.h"
#include "Uge/Math/AABB.h"
#include "Uge/Math/Frustum.h"

#include <glm/gtc/matrix_transform.hpp>

#include <gtest/gtest.h>

namespace
{

	constexpr float s_tolerance = 1e-4f;

	void ExpectVec3Near(const glm::vec3& actual, const glm::vec3& expected, float tolerance = s_tolerance)
	{
		EXPECT_NEAR(actual.x, expected.x, tolerance);
		EXPECT_NEAR(actual.y, expected.y, tolerance);
		EXPECT_NEAR(actual.z, expected.z, tolerance);
	}

}

TEST(MathDecomposeTest, RoundTripsATransformComponent)
{
	// ImGuizmo hands the editor a matrix, which EditorLayer decomposes straight back into
	// the entity's TransformComponent. Anything lost here is drift the user sees as the
	// object sliding while it is dragged.
	Uge::TransformComponent source;
	source.Translation = glm::vec3(3.0f, -2.0f, 7.5f);
	source.Rotation = glm::vec3(glm::radians(30.0f), glm::radians(-45.0f), glm::radians(15.0f));
	source.Scale = glm::vec3(2.0f, 0.5f, 1.25f);

	glm::vec3 translation(0.0f);
	glm::vec3 rotation(0.0f);
	glm::vec3 scale(0.0f);

	ASSERT_TRUE(Uge::Math::DecomposeTransform(source.GetTransform(), translation, rotation, scale));

	ExpectVec3Near(translation, source.Translation);
	ExpectVec3Near(rotation, source.Rotation);
	ExpectVec3Near(scale, source.Scale);
}

TEST(MathDecomposeTest, IdentityDecomposesToNoTransform)
{
	glm::vec3 translation(1.0f);
	glm::vec3 rotation(1.0f);
	glm::vec3 scale(0.0f);

	ASSERT_TRUE(Uge::Math::DecomposeTransform(glm::mat4(1.0f), translation, rotation, scale));

	ExpectVec3Near(translation, glm::vec3(0.0f));
	ExpectVec3Near(rotation, glm::vec3(0.0f));
	ExpectVec3Near(scale, glm::vec3(1.0f));
}

TEST(MathDecomposeTest, RejectsADegenerateMatrix)
{
	glm::mat4 degenerate(1.0f);
	degenerate[3][3] = 0.0f;

	glm::vec3 translation(0.0f);
	glm::vec3 rotation(0.0f);
	glm::vec3 scale(0.0f);

	EXPECT_FALSE(Uge::Math::DecomposeTransform(degenerate, translation, rotation, scale));
}

namespace
{
	Uge::Math::AABB MakeBox(const glm::vec3& min, const glm::vec3& max)
	{
		Uge::Math::AABB box;
		box.Min = min;
		box.Max = max;
		return box;
	}
}

TEST(AABBTest, TranslationMovesTheBox)
{
	const Uge::Math::AABB box = MakeBox(glm::vec3(-1.0f), glm::vec3(1.0f));
	const Uge::Math::AABB moved = box.Transform(glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, -2.0f)));

	EXPECT_NEAR(moved.Min.x, 4.0f, 1e-5f);
	EXPECT_NEAR(moved.Max.x, 6.0f, 1e-5f);
	EXPECT_NEAR(moved.Min.z, -3.0f, 1e-5f);
	EXPECT_NEAR(moved.Max.z, -1.0f, 1e-5f);
}

TEST(AABBTest, AQuarterTurnSwapsTheExtents)
{
	// Turned 90 degrees about Y, a box that is long in X becomes long in Z. The box fits
	// its new orientation exactly, so the result is exact rather than merely enclosing.
	const Uge::Math::AABB box = MakeBox(glm::vec3(-2.0f, -1.0f, -0.5f), glm::vec3(2.0f, 1.0f, 0.5f));
	const Uge::Math::AABB turned = box.Transform(
		glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)));

	const glm::vec3 extents = turned.GetExtents();
	EXPECT_NEAR(extents.x, 0.5f, 1e-5f);
	EXPECT_NEAR(extents.y, 1.0f, 1e-5f);
	EXPECT_NEAR(extents.z, 2.0f, 1e-5f);
}

TEST(AABBTest, ATransformedBoxContainsEveryTransformedCorner)
{
	// An arbitrary rotation, a non-uniform scale and a translation together. The result must
	// enclose all eight corners of the original box after the same transform.
	const Uge::Math::AABB box = MakeBox(glm::vec3(-1.0f, -2.0f, -0.5f), glm::vec3(3.0f, 1.0f, 0.5f));
	const glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, -1.0f, 4.0f))
		* glm::rotate(glm::mat4(1.0f), glm::radians(37.0f), glm::normalize(glm::vec3(1.0f, 2.0f, 0.5f)))
		* glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 0.5f, 1.5f));

	const Uge::Math::AABB result = box.Transform(transform);

	for (int corner = 0; corner < 8; corner++)
	{
		const glm::vec3 local(
			(corner & 1) ? box.Max.x : box.Min.x,
			(corner & 2) ? box.Max.y : box.Min.y,
			(corner & 4) ? box.Max.z : box.Min.z);
		const glm::vec3 world = glm::vec3(transform * glm::vec4(local, 1.0f));

		for (int axis = 0; axis < 3; axis++)
		{
			EXPECT_GE(world[axis], result.Min[axis] - 1e-4f);
			EXPECT_LE(world[axis], result.Max[axis] + 1e-4f);
		}
	}
}

TEST(AABBTest, AnEmptyBoxStaysEmptyWhenTransformed)
{
	const Uge::Math::AABB empty;
	EXPECT_FALSE(empty.IsValid());
	EXPECT_FALSE(empty.Transform(glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 2.0f, 3.0f))).IsValid());
}

TEST(FrustumTest, ClassifiesBoxesAgainstAPerspectiveView)
{
	// A camera at the origin looking down -Z, with a 90 degree field of view and a square
	// aspect: at distance d, the view reaches d to either side. The view matrix is the
	// identity, so the projection alone is the view-projection.
	const glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
	const Uge::Math::Frustum frustum = Uge::Math::Frustum::FromViewProjection(projection);

	// Straight ahead.
	EXPECT_TRUE(frustum.Intersects(MakeBox(glm::vec3(-1.0f, -1.0f, -11.0f), glm::vec3(1.0f, 1.0f, -9.0f))));
	// Behind the camera.
	EXPECT_FALSE(frustum.Intersects(MakeBox(glm::vec3(-1.0f, -1.0f, 9.0f), glm::vec3(1.0f, 1.0f, 11.0f))));
	// Past the far plane.
	EXPECT_FALSE(frustum.Intersects(MakeBox(glm::vec3(-1.0f, -1.0f, -210.0f), glm::vec3(1.0f, 1.0f, -200.0f))));
	// Across the left edge: partly visible counts as visible.
	EXPECT_TRUE(frustum.Intersects(MakeBox(glm::vec3(-12.0f, -1.0f, -11.0f), glm::vec3(-8.0f, 1.0f, -9.0f))));
	// Wholly to the left.
	EXPECT_FALSE(frustum.Intersects(MakeBox(glm::vec3(-30.0f, -1.0f, -11.0f), glm::vec3(-20.0f, 1.0f, -9.0f))));
	// Empty bounds are kept rather than culled, so a mesh with no geometry never vanishes silently.
	EXPECT_TRUE(frustum.Intersects(Uge::Math::AABB()));
}
