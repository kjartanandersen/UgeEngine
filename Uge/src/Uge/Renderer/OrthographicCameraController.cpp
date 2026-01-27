#include "ugpch.h"
#include "OrthographicCameraController.h"

#include "Uge/Core/Input.h"
#include "Uge/Core/KeyCodes.h"


namespace Uge
{


	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation)
		: m_aspectRatio(aspectRatio), m_bounds({ -m_aspectRatio * m_zoomLevel, m_aspectRatio * m_zoomLevel, -m_zoomLevel, m_zoomLevel }), m_camera(m_bounds.Left, m_bounds.Right, m_bounds.Bottom, m_bounds.Top), m_rotation(rotation)
	{



	}

	void OrthographicCameraController::OnUpdate(Timestep ts)
	{
		UG_PROFILE_FUNCTION();

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

			//m_camera.SetRotation(m_cameraRotation);

		}

		//m_camera.SetPosition(m_cameraPosition);

	}
	void OrthographicCameraController::OnEvent(Event& e)
	{
		UG_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(UG_BIND_EVENT_FN(OrthographicCameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(UG_BIND_EVENT_FN(OrthographicCameraController::OnWindowResized));



	}

	void OrthographicCameraController::OnResize(float width, float height)
	{

		m_aspectRatio = width / height;
		CalculateView();


	}

	void OrthographicCameraController::CalculateView()
	{


		m_bounds = { -m_aspectRatio * m_zoomLevel, m_aspectRatio * m_zoomLevel, -m_zoomLevel, m_zoomLevel };
		m_camera.SetProjection(m_bounds.Left, m_bounds.Right, m_bounds.Bottom, m_bounds.Top);

	}


	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		UG_PROFILE_FUNCTION();

		m_zoomLevel -= e.GetYOffset() * 0.2f;
		m_zoomLevel = std::clamp(m_zoomLevel, 0.1f, 10.0f);
		CalculateView();
		return false;
	}
	bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		UG_PROFILE_FUNCTION();

		OnResize((float)e.GetWidth(), (float)e.GetHeight());
		return false;
	}


}