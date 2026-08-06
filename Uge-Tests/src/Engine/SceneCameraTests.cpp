/**
 * @file SceneCameraTests.cpp
 * @brief Tests for Uge::SceneCamera's projection maintenance.
 */

#include <ugpch.h>

#include "Uge/Scene/SceneCamera.h"

#include <gtest/gtest.h>

using Uge::SceneCamera;

namespace
{
	constexpr float s_tolerance = 1e-5f;
}

TEST(SceneCameraTest, DefaultsToOrthographic)
{
	SceneCamera camera;

	EXPECT_EQ(camera.GetProjectionType(), SceneCamera::ProjectionType::Orthographic);
	EXPECT_FLOAT_EQ(camera.GetOrthoSize(), 10.0f);
}

TEST(SceneCameraTest, OrthographicProjectionMatchesTheViewportExtents)
{
	// Size is the vertical extent and the horizontal one follows the aspect ratio, so a
	// 2:1 viewport with size 10 sees 20 units across and 10 down.
	SceneCamera camera;
	camera.SetOrtho(10.0f, -1.0f, 1.0f);
	camera.SetViewportSize(1600, 800);

	const glm::mat4& projection = camera.GetProjection();

	EXPECT_NEAR(projection[0][0], 2.0f / 20.0f, s_tolerance);
	EXPECT_NEAR(projection[1][1], 2.0f / 10.0f, s_tolerance);
}

TEST(SceneCameraTest, OrthographicSizeScalesTheProjection)
{
	SceneCamera camera;
	camera.SetViewportSize(800, 800);

	camera.SetOrthoSize(10.0f);
	const float wide = camera.GetProjection()[1][1];

	camera.SetOrthoSize(5.0f);
	const float narrow = camera.GetProjection()[1][1];

	// Halving the visible extent doubles the scale factor: zooming in, not out.
	EXPECT_NEAR(narrow, wide * 2.0f, s_tolerance);
}

TEST(SceneCameraTest, PerspectiveProjectionFollowsTheFieldOfView)
{
	SceneCamera camera;
	camera.SetPersp(glm::radians(90.0f), 0.1f, 100.0f);
	camera.SetViewportSize(1600, 800);

	const glm::mat4& projection = camera.GetProjection();

	// At 90 degrees vertical, the vertical scale is exactly 1, and the horizontal one is
	// that divided by the aspect ratio.
	EXPECT_NEAR(projection[1][1], 1.0f, 1e-5f);
	EXPECT_NEAR(projection[0][0], 0.5f, 1e-5f);
}

TEST(SceneCameraTest, FieldOfViewIsStoredInRadiansAndReportedInDegrees)
{
	// SetPersp takes radians while the getter and the property panel's setter use degrees;
	// mixing them up gives a camera that looks either fully zoomed in or inside-out.
	SceneCamera camera;
	camera.SetPersp(glm::radians(60.0f), 0.1f, 100.0f);

	EXPECT_NEAR(camera.GetPerspVerticalFOV(), 60.0f, 1e-3f);

	camera.SetPerspVerticalFOV(45.0f);
	EXPECT_NEAR(camera.GetPerspVerticalFOV(), 45.0f, 1e-3f);
}

TEST(SceneCameraTest, SettersRecordTheClipPlanes)
{
	SceneCamera camera;

	camera.SetOrtho(4.0f, -2.0f, 6.0f);
	EXPECT_FLOAT_EQ(camera.GetOrthoSize(), 4.0f);
	EXPECT_FLOAT_EQ(camera.GetOrthoNearClip(), -2.0f);
	EXPECT_FLOAT_EQ(camera.GetOrthoFarClip(), 6.0f);

	camera.SetPersp(glm::radians(30.0f), 0.5f, 250.0f);
	EXPECT_FLOAT_EQ(camera.GetPerspNearClip(), 0.5f);
	EXPECT_FLOAT_EQ(camera.GetPerspFarClip(), 250.0f);
}

TEST(SceneCameraTest, SetOrthoAndSetPerspSwitchTheProjectionType)
{
	SceneCamera camera;

	camera.SetPersp(glm::radians(45.0f), 0.1f, 100.0f);
	EXPECT_EQ(camera.GetProjectionType(), SceneCamera::ProjectionType::Perspective);

	camera.SetOrtho(10.0f, -1.0f, 1.0f);
	EXPECT_EQ(camera.GetProjectionType(), SceneCamera::ProjectionType::Orthographic);
}

TEST(SceneCameraTest, SwitchingTypeKeepsBothParameterSets)
{
	// Documented behaviour, and what makes the projection dropdown in the properties panel
	// safe to flip back and forth while experimenting.
	SceneCamera camera;
	camera.SetOrtho(7.0f, -3.0f, 9.0f);
	camera.SetPersp(glm::radians(70.0f), 0.25f, 500.0f);

	camera.SetProjectionType(SceneCamera::ProjectionType::Orthographic);

	EXPECT_FLOAT_EQ(camera.GetOrthoSize(), 7.0f);
	EXPECT_FLOAT_EQ(camera.GetOrthoNearClip(), -3.0f);
	EXPECT_FLOAT_EQ(camera.GetOrthoFarClip(), 9.0f);

	camera.SetProjectionType(SceneCamera::ProjectionType::Perspective);

	EXPECT_NEAR(camera.GetPerspVerticalFOV(), 70.0f, 1e-3f);
	EXPECT_FLOAT_EQ(camera.GetPerspNearClip(), 0.25f);
	EXPECT_FLOAT_EQ(camera.GetPerspFarClip(), 500.0f);
}

TEST(SceneCameraTest, EverySetterRebuildsTheProjectionImmediately)
{
	// The properties panel edits these live and never asks for a recalculation, so a
	// setter that only stored its value would show a stale viewport.
	SceneCamera camera;
	camera.SetViewportSize(800, 600);

	const glm::mat4 before = camera.GetProjection();

	camera.SetOrthoSize(camera.GetOrthoSize() * 2.0f);
	EXPECT_NE(camera.GetProjection(), before);
}

TEST(SceneCameraTest, ResizingTheViewportUpdatesTheAspectRatio)
{
	SceneCamera camera;
	camera.SetViewportSize(800, 600);
	const glm::mat4 fourThree = camera.GetProjection();

	camera.SetViewportSize(1920, 1080);
	EXPECT_NE(camera.GetProjection(), fourThree);
}

TEST(SceneCameraTest, SquareViewportGivesEqualScales)
{
	SceneCamera camera;
	camera.SetOrtho(10.0f, -1.0f, 1.0f);
	camera.SetViewportSize(512, 512);

	const glm::mat4& projection = camera.GetProjection();

	EXPECT_NEAR(projection[0][0], projection[1][1], s_tolerance);
}
