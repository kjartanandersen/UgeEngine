/**
 * @file PerspectiveCameraController.h
 * @brief First-person style controller for a perspective camera.
 * @ingroup group_renderer
 */

#pragma once

#include "Uge/Renderer/PerspectiveCamera.h"
#include "Uge/Core/Timestep.h"

#include "Uge/Events/ApplicationEvent.h"
#include "Uge/Events/MouseEvent.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Uge
{

	/**
	 * @brief Drives an Uge::PerspectiveCamera with mouse-look and keyboard movement.
	 * @ingroup group_renderer
	 *
	 * Orientation is stored as a quaternion, which avoids the gimbal-lock problems of
	 * accumulating Euler angles. For editing rather than in-game movement, use
	 * Uge::EditorCamera, which orbits a focal point instead.
	 */
	class PerspectiveCameraController
	{

	public:
		/**
		 * @brief Constructs the controller and its camera.
		 * @param fovYRadians Vertical field of view, in radians.
		 * @param aspectRatio Viewport width divided by height.
		 * @param rotation `true` to enable mouse-look.
		 * @param nearClip Near plane distance.
		 * @param farClip Far plane distance.
		 */
		PerspectiveCameraController(float fovYRadians, float aspectRatio, bool rotation, float nearClip = 0.1f, float farClip = 1000.0f);

		/**
		 * @brief Applies movement and mouse-look for this frame.
		 * @param ts Frame delta time.
		 */
		void OnUpdate(Timestep ts);
		/**
		 * @brief Handles scroll-wheel and window resize events.
		 * @param e Event to inspect.
		 */
		void OnEvent(Event& e);

		/**
		 * @brief The controlled camera.
		 * @return Mutable reference to the camera.
		 */
		PerspectiveCamera& GetCamera() { return m_camera; }
		/**
		 * @brief The controlled camera.
		 * @return Const reference to the camera.
		 */
		const PerspectiveCamera& GetCamera() const { return m_camera; }

	private:
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);

	private:
		float m_fovY = 0.785398f; // 45 deg in radians
		float m_aspectRatio = 16.0f / 9.0f;
		float m_nearClip = 0.1f;
		float m_farClip = 1000.0f;
		bool m_rotation;

		float m_width, m_height;

		PerspectiveCamera m_camera;
		glm::vec3 m_cameraPosition = { 1.0f, 0.0f, 1.0f };
		glm::quat m_qRotation = { 1.0f, 0.0f, 0.0f, 0.0f };
		float m_cameraTranslationSpeed = 0.03f, m_cameraRotationSpeed = 0.1f;
		float m_mouseSpeed = 1.0f;

		float m_mouseXDelta, m_mouseYDelta;
		float m_mouseXPrevPos, m_mouseYPrevPos;

	};


}


