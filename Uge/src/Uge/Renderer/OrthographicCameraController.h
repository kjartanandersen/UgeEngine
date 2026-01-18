#pragma once

#include "Uge/Renderer/OrthographicCamera.h"
#include "Uge/Core/Timestep.h"

#include "Uge/Events/ApplicationEvent.h"
#include "Uge/Events/MouseEvent.h"

namespace Uge
{

	struct OrthographicCameraBounds
	{
		float Left, Right;
		float Bottom, Top;

		float GetWidth() { return Right - Left; }
		float GetHeight() { return Top - Bottom; }
	};


	class OrthographicCameraController
	{

	public:
		OrthographicCameraController(float aspectRatio, bool rotation = false);

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);

		OrthographicCamera& GetCamera() { return m_camera; }
		const OrthographicCamera& GetCamera() const { return m_camera; }

		void SetZoomLevel(float zLevel) { m_zoomLevel = zLevel; }
		float GetZoomLevel() const { return m_zoomLevel; }


		const OrthographicCameraBounds& GetBounds() const { return m_bounds; }

	private:
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);

	private:

		float m_aspectRatio;
		float m_zoomLevel = 1.0f;
		bool m_rotation;

		OrthographicCameraBounds m_bounds;
		OrthographicCamera m_camera;
		glm::vec3 m_cameraPosition = { 0.0f, 0.0f, 0.0f };
		float m_cameraRotation = 0.0f; //In degrees, in the anti-clockwise direction
		float m_cameraTranslationSpeed = 5.0f, m_cameraRotationSpeed = 180.0f;

	};


}


