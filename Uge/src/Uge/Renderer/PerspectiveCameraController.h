#pragma once

#include "Uge/Renderer/PerspectiveCamera.h"
#include "Uge/Core/Timestep.h"

#include "Uge/Events/ApplicationEvent.h"
#include "Uge/Events/MouseEvent.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Uge
{

	class PerspectiveCameraController
	{

	public:
		PerspectiveCameraController(float fovYRadians, float aspectRatio, bool rotation, float nearClip = 0.1f, float farClip = 1000.0f);

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);

		PerspectiveCamera& GetCamera() { return m_camera; }
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
		glm::vec3 m_cameraPosition = { 0.0f, 0.0f, 0.0f };
		glm::quat m_qRotation = { 1.0f, 0.0f, 0.0f, 0.0f };
		float m_cameraTranslationSpeed = 0.3f, m_cameraRotationSpeed = 180.0f;
		float m_mouseSpeed = 1.0f;

	};


}


