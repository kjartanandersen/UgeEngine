/**
 * @file ColorSpaceTests.cpp
 * @brief Tests for the sRGB/linear conversions in Uge/Renderer/ColorSpace.h.
 */

#include <ugpch.h>

#include "Uge/Renderer/ColorSpace.h"

#include <gtest/gtest.h>

using Uge::LinearToSrgb;
using Uge::SrgbToLinear;

namespace
{
	constexpr float s_tolerance = 1e-5f;
}

TEST(ColorSpaceTest, BlackAndWhiteAreFixedPoints)
{
	EXPECT_NEAR(SrgbToLinear(0.0f), 0.0f, s_tolerance);
	EXPECT_NEAR(SrgbToLinear(1.0f), 1.0f, s_tolerance);
	EXPECT_NEAR(LinearToSrgb(0.0f), 0.0f, s_tolerance);
	EXPECT_NEAR(LinearToSrgb(1.0f), 1.0f, s_tolerance);
}

TEST(ColorSpaceTest, UsesTheLinearSegmentBelowTheKnee)
{
	// Below 0.04045 the curve is a plain division, not a power. Getting this branch wrong
	// only shows up in near-black values, which is exactly where it is hardest to see.
	EXPECT_NEAR(SrgbToLinear(0.02f), 0.02f / 12.92f, s_tolerance);
	EXPECT_NEAR(SrgbToLinear(0.04045f), 0.04045f / 12.92f, s_tolerance);
}

TEST(ColorSpaceTest, UsesThePowerSegmentAboveTheKnee)
{
	EXPECT_NEAR(SrgbToLinear(0.5f), 0.21404114f, 1e-5f);
	EXPECT_NEAR(SrgbToLinear(0.75f), 0.52252054f, 1e-5f);
}

TEST(ColorSpaceTest, LinearToSrgbIsTheInverse)
{
	for (float value : { 0.0f, 0.001f, 0.0031308f, 0.05f, 0.25f, 0.5f, 0.75f, 1.0f })
	{
		EXPECT_NEAR(LinearToSrgb(SrgbToLinear(value)), value, 1e-5f)
			<< "round trip failed for " << value;
	}
}

TEST(ColorSpaceTest, DecodingDarkensAndEncodingBrightens)
{
	// The direction matters more than the exact number: a value decoded twice, or encoded
	// when it should have been decoded, is the classic "washed out" bug.
	EXPECT_LT(SrgbToLinear(0.5f), 0.5f);
	EXPECT_GT(LinearToSrgb(0.5f), 0.5f);
}

TEST(ColorSpaceTest, ConvertsEachChannelOfAVec3)
{
	const glm::vec3 encoded(0.5f, 0.25f, 0.75f);
	const glm::vec3 linear = SrgbToLinear(encoded);

	EXPECT_NEAR(linear.r, SrgbToLinear(0.5f), s_tolerance);
	EXPECT_NEAR(linear.g, SrgbToLinear(0.25f), s_tolerance);
	EXPECT_NEAR(linear.b, SrgbToLinear(0.75f), s_tolerance);
}

TEST(ColorSpaceTest, LeavesAlphaAlone)
{
	// Alpha is coverage, not light, so gamma does not apply to it.
	const glm::vec4 encoded(0.5f, 0.5f, 0.5f, 0.33f);
	const glm::vec4 linear = SrgbToLinear(encoded);

	EXPECT_FLOAT_EQ(linear.a, 0.33f);
	EXPECT_NEAR(linear.r, SrgbToLinear(0.5f), s_tolerance);
}

TEST(ColorSpaceTest, Vec3RoundTrips)
{
	const glm::vec3 original(0.2f, 0.6f, 0.9f);
	const glm::vec3 roundTripped = LinearToSrgb(SrgbToLinear(original));

	EXPECT_NEAR(roundTripped.r, original.r, 1e-5f);
	EXPECT_NEAR(roundTripped.g, original.g, 1e-5f);
	EXPECT_NEAR(roundTripped.b, original.b, 1e-5f);
}

TEST(ColorSpaceTest, IsMonotonic)
{
	float previous = SrgbToLinear(0.0f);

	for (int step = 1; step <= 100; step++)
	{
		const float current = SrgbToLinear((float)step / 100.0f);
		EXPECT_GT(current, previous) << "at step " << step;
		previous = current;
	}
}
