/**
 * @file EditorCamera.h
 * @brief The orbiting viewport camera used for scene editing.
 * @ingroup group_renderer
 */

#pragma once

#include "Camera.h"
#include "Uge/Core/Timestep.h"
#include "Uge/Events/Event.h"
#include "Uge/Events/MouseEvent.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Uge {

	/**
	 * @brief Orbit camera for the editor viewport, driven by mouse and modifier keys.
	 * @ingroup group_renderer
	 *
	 * Rather than a free-flying position, this camera orbits a focal point at a fixed
	 * distance — the behaviour expected of a modelling tool, where the object of interest
	 * stays centred.
	 *
	 * Controls, all requiring Alt to be held:
	 * - **Alt + left drag** — orbit around the focal point
	 * - **Alt + middle drag** — pan the focal point
	 * - **Alt + right drag** or scroll wheel — zoom, by changing the orbit distance
	 *
	 * Position is derived from the focal point, orientation and distance, so writing to
	 * those is what moves the camera.
	 *
	 * @note Only used in the editor. At runtime the scene renders from the entity holding
	 * the primary Uge::CameraComponent.
	 */
	class EditorCamera : public Camera
	{
	public:
		/** @brief Constructs an unconfigured camera; set a projection before use. */
		EditorCamera() = default;
		/**
		 * @brief Constructs the camera and builds its projection.
		 * @param fov Vertical field of view, in degrees.
		 * @param aspectRatio Viewport width divided by height.
		 * @param nearClip Near plane distance.
		 * @param farClip Far plane distance.
		 */
		EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

		/**
		 * @brief Applies mouse input and recomputes the view matrix.
		 * @param ts Frame delta time.
		 * @note Only responds while Alt is held, so the mouse stays free for selection.
		 */
		void OnUpdate(Timestep ts);
		/**
		 * @brief Handles scroll-wheel zoom.
		 * @param e Event to inspect.
		 */
		void OnEvent(Event& e);

		/**
		 * @brief Orbit radius.
		 * @return Distance from the camera to its focal point.
		 */
		inline float GetDistance() const { return m_distance; }
		/**
		 * @brief Sets the orbit radius.
		 * @param distance New distance from the focal point.
		 */
		inline void SetDistance(float distance) { m_distance = distance; }

		/**
		 * @brief Updates the viewport size and rebuilds the projection.
		 * @param width Viewport width in pixels.
		 * @param height Viewport height in pixels.
		 */
		inline void SetViewportSize(float width, float height) { m_viewportWidth = width; m_viewportHeight = height; UpdateProjection(); }

		/**
		 * @brief The view matrix.
		 * @return Const reference to the world-to-camera transform.
		 */
		const glm::mat4& GetViewMatrix() const { return m_viewMatrix; }
		/**
		 * @brief The combined view-projection matrix.
		 * @return `projection * view`, ready to upload to a shader.
		 */
		glm::mat4 GetViewProjection() const { return m_projection * m_viewMatrix; }

		/**
		 * @brief The camera's up axis.
		 * @return Normalized up vector in world space.
		 */
		glm::vec3 GetUpDirection() const;
		/**
		 * @brief The camera's right axis.
		 * @return Normalized right vector in world space.
		 */
		glm::vec3 GetRightDirection() const;
		/**
		 * @brief The camera's viewing axis.
		 * @return Normalized forward vector in world space.
		 */
		glm::vec3 GetForwardDirection() const;
		/**
		 * @brief The camera's world position.
		 * @return Const reference to the position, derived from the focal point and distance.
		 */
		const glm::vec3& GetPosition() const { return m_position; }
		/**
		 * @brief The camera's orientation.
		 * @return Rotation quaternion built from the current pitch and yaw.
		 */
		glm::quat GetOrientation() const;

		/**
		 * @brief Rotation about the right axis.
		 * @return Pitch in radians.
		 */
		float GetPitch() const { return m_pitch; }
		/**
		 * @brief Rotation about the up axis.
		 * @return Yaw in radians.
		 */
		float GetYaw() const { return m_Yaw; }

		/**
		 * @brief Points the camera along a direction.
		 * @param direction Direction to face, in world space.
		 * @param up Reference up vector resolving the roll.
		 */
		void LookAt(const glm::vec3& direction, const glm::vec3& up);
		/**
		 * @brief Whether an orbit drag is in progress.
		 * @return `true` while the user is rotating the camera.
		 *
		 * The editor checks this to suppress gizmo interaction during a camera drag.
		 */
		bool IsBeingRotated();

	private:
		void UpdateProjection();
		void UpdateView();

		bool OnMouseScroll(MouseScrolledEvent& e);

		void MousePan(const glm::vec2& delta);
		void MouseRotate(const glm::vec2& delta);
		void MouseZoom(float delta);


		glm::vec3 CalculatePosition() const;

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float MovementSpeed() const;
		float ZoomSpeed() const;
	private:
		float m_FOV = 70.0f, m_aspectRatio = 1.778f, m_nearClip = 0.1f, m_farClip = 1000.0f;

		glm::mat4 m_viewMatrix;
		glm::vec3 m_position = { 0.0f, 0.0f, 3.0f };
		glm::vec3 m_focalPoint = { 0.0f, 0.0f, 0.0f };

		glm::quat m_orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		float m_currentPitch = 0.0f;

		glm::vec2 m_initialMousePosition = { 0.0f, 0.0f };

		float m_distance = 10.0f;
		float m_pitch = 0.0f, m_Yaw = 0.0f;

		float m_viewportWidth = 1280, m_viewportHeight = 720;

		bool m_beingRotated = false; 
	};

}