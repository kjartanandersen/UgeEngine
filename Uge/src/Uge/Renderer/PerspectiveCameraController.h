/**
 * @file PerspectiveCameraController.h
 * @brief Input controller for a perspective camera.
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
	 * @brief Owns an Uge::PerspectiveCamera and tracks keyboard movement and mouse look.
	 * @ingroup group_renderer
	 */
	class PerspectiveCameraController
	{

	public:
		/**
		 * @brief A controller for the application's window, which must be a Uge::WindowsWindow.
		 * @param fovYRadians Vertical field of view in radians.
		 * @param aspectRatio Width divided by height.
		 * @param rotation Whether the mouse turns the camera.
		 * @param nearClip Near plane distance.
		 * @param farClip Far plane distance.
		 */
		PerspectiveCameraController(float fovYRadians, float aspectRatio, bool rotation, float nearClip = 0.1f, float farClip = 1000.0f);

		/**
		 * @brief Moves with W, A, S and D, and turns with the mouse while the right button is
		 * held.
		 *
		 * The position and rotation aren't passed to the camera, which has only a projection.
		 *
		 * @param ts Frame time.
		 */
		void OnUpdate(Timestep ts);
		/**
		 * @brief Rebuilds the projection on a resize. Scrolling does nothing.
		 * @param e The event.
		 */
		void OnEvent(Event& e);

		/**
		 * @brief The camera.
		 * @return The camera.
		 */
		PerspectiveCamera& GetCamera() { return m_camera; }
		/**
		 * @brief The camera.
		 * @return The camera.
		 */
		const PerspectiveCamera& GetCamera() const { return m_camera; }

	private:
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);

	private:
		float m_fovY = 0.785398f; // 45 degrees
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


