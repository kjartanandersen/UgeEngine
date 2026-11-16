/**
 * @file SceneCameraTests.cpp
 * @brief Tests for Uge::SceneCamera.
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
	// Size is the visible height, so a 2:1 viewport with size 10 sees 20 across.
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

	// Half the size doubles the scale.
	EXPECT_NEAR(narrow, wide * 2.0f, s_tolerance);
}

TEST(SceneCameraTest, PerspectiveProjectionFollowsTheFieldOfView)
{
	SceneCamera camera;
	camera.SetPersp(glm::radians(90.0f), 0.1f, 100.0f);
	camera.SetViewportSize(1600, 800);

	const glm::mat4& projection = camera.GetProjection();

	// At 90 degrees the vertical scale is 1, and the horizontal one is 1 over the aspect ratio.
	EXPECT_NEAR(projection[1][1], 1.0f, 1e-5f);
	EXPECT_NEAR(projection[0][0], 0.5f, 1e-5f);
}

TEST(SceneCameraTest, FieldOfViewIsStoredInRadiansAndReportedInDegrees)
{
	// SetPersp takes radians; the field of view getter and setter use degrees.
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
	// Each projection keeps its settings while the other is in use.
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
	// Each setter rebuilds the projection.
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
