/**
 * @file MathTests.cpp
 * @brief Tests for Uge::Math::DecomposeTransform, the gizmo's write-back path.
 */

#include <ugpch.h>

#include "Uge/Math/Math.h"
#include "Uge/Scene/Components.h"

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
