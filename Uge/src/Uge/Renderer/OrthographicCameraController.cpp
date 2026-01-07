#include "ugpch.h"
#include "OrthographicCameraController.h"

#include "Uge/Core/Input.h"
#include "Uge/Core/KeyCodes.h"


namespace Uge
{


	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation)
		: m_aspectRatio(aspectRatio), m_camera(-m_aspectRatio * m_zoomLevel, m_aspectRatio* m_zoomLevel, -m_zoomLevel, m_zoomLevel), m_rotation(rotation)
	{



	}

	void OrthographicCameraController::OnUpdate(Timestep ts)
	{

		// Camera Position
		if (Input::IsKeyPressed(UG_KEY_A))
		{
			m_cameraPosition.x -= cos(glm::radians(m_cameraRotation)) * m_cameraTranslationSpeed * ts;
			m_cameraPosition.y -= sin(glm::radians(m_cameraRotation)) * m_cameraTranslationSpeed * ts;
		}

		else if (Input::IsKeyPressed(UG_KEY_D))
		{
			m_cameraPosition.x += cos(glm::radians(m_cameraRotation)) * m_cameraTranslationSpeed * ts;
			m_cameraPosition.y += sin(glm::radians(m_cameraRotation)) * m_cameraTranslationSpeed * ts;
		}

		if (Input::IsKeyPressed(UG_KEY_W))
		{
			m_cameraPosition.x += -sin(glm::radians(m_cameraRotation)) * m_cameraTranslationSpeed * ts;
			m_cameraPosition.y += cos(glm::radians(m_cameraRotation)) * m_cameraTranslationSpeed * ts;
		}

		else if (Input::IsKeyPressed(UG_KEY_S))
		{
			m_cameraPosition.x -= -sin(glm::radians(m_cameraRotation)) * m_cameraTranslationSpeed * ts;
			m_cameraPosition.y -= cos(glm::radians(m_cameraRotation)) * m_cameraTranslationSpeed * ts;
		}


		if (m_rotation)
		{
			// Camera Rotation
			if (Input::IsKeyPressed(UG_KEY_Q))
				m_cameraRotation += m_cameraRotationSpeed * ts;

			if (Input::IsKeyPressed(UG_KEY_E))
				m_cameraRotation -= m_cameraRotationSpeed * ts;

			m_camera.SetRotation(m_cameraRotation);

		}

		m_camera.SetPosition(m_cameraPosition);

	}
	void OrthographicCameraController::OnEvent(Event& e)
	{

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(UG_BIND_EVENT_FN(OrthographicCameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(UG_BIND_EVENT_FN(OrthographicCameraController::OnWindowResized));



	}


	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{

		m_zoomLevel -= e.GetYOffset() * 0.2f;
		m_zoomLevel = std::clamp(m_zoomLevel, 0.1f, 5.0f);
		m_camera.SetProjection(-m_aspectRatio * m_zoomLevel, m_aspectRatio * m_zoomLevel, 
			-m_zoomLevel, m_zoomLevel);



		return false;
	}
	bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& e)
	{

		m_aspectRatio = (float)e.GetWidth() / (float)e.GetHeight();
		m_camera.SetProjection(-m_aspectRatio * m_zoomLevel, m_aspectRatio * m_zoomLevel,
			-m_zoomLevel, m_zoomLevel);


		return false;
	}


}