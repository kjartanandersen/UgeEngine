/**
 * @file EditorCamera.h
 * @brief The editor's viewport camera.
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
	 * @brief A fly camera for the editor viewport.
	 * @ingroup group_renderer
	 *
	 * Hold the right mouse button to look around, and W, A, S and D to move while it is held.
	 */
	class EditorCamera : public Camera
	{
	public:
		/** @brief A camera with an identity projection. */
		EditorCamera() = default;
		/**
		 * @brief A camera at `(0, 0, 3)` looking at the origin.
		 * @param fov Vertical field of view in degrees.
		 * @param aspectRatio Width divided by height.
		 * @param nearClip Near plane distance.
		 * @param farClip Far plane distance.
		 */
		EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

		/**
		 * @brief Applies mouse and keyboard input and updates the view matrix.
		 * @param ts Frame time.
		 */
		void OnUpdate(Timestep ts);
		/**
		 * @brief Handles an event. Scrolling does nothing yet.
		 * @param e The event.
		 */
		void OnEvent(Event& e);

		/**
		 * @brief The orbit distance, which the fly controls don't use.
		 * @return The distance.
		 */
		inline float GetDistance() const { return m_distance; }
		/**
		 * @brief Sets the orbit distance.
		 * @param distance The distance.
		 */
		inline void SetDistance(float distance) { m_distance = distance; }

		/**
		 * @brief Sets the viewport size and rebuilds the projection.
		 * @param width Width in pixels.
		 * @param height Height in pixels.
		 */
		inline void SetViewportSize(float width, float height) { m_viewportWidth = width; m_viewportHeight = height; UpdateProjection(); }

		/**
		 * @brief The view matrix.
		 * @return The view matrix.
		 */
		const glm::mat4& GetViewMatrix() const { return m_viewMatrix; }
		/**
		 * @brief The view-projection matrix.
		 * @return `projection * view`.
		 */
		glm::mat4 GetViewProjection() const { return m_projection * m_viewMatrix; }

		/**
		 * @brief The camera's up direction.
		 * @return Unit vector in world space.
		 */
		glm::vec3 GetUpDirection() const;
		/**
		 * @brief The camera's right direction.
		 * @return Unit vector in world space.
		 */
		glm::vec3 GetRightDirection() const;
		/**
		 * @brief The direction the camera looks in.
		 * @return Unit vector in world space.
		 */
		glm::vec3 GetForwardDirection() const;
		/**
		 * @brief The camera's position.
		 * @return World-space position.
		 */
		const glm::vec3& GetPosition() const { return m_position; }
		/**
		 * @brief A rotation built from GetPitch() and GetYaw().
		 * @return The rotation.
		 */
		glm::quat GetOrientation() const;

		/**
		 * @brief Pitch, which mouse look doesn't update.
		 * @return Pitch in radians.
		 */
		float GetPitch() const { return m_pitch; }
		/**
		 * @brief Yaw, which mouse look doesn't update.
		 * @return Yaw in radians.
		 */
		float GetYaw() const { return m_Yaw; }

		/**
		 * @brief Turns the camera to face a direction.
		 * @param direction World-space direction.
		 * @param up Up vector.
		 */
		void LookAt(const glm::vec3& direction, const glm::vec3& up);
		/**
		 * @brief Whether the right mouse button was held at the last OnUpdate().
		 * @return `true` if it was.
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