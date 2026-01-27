#include "ugpch.h"
#include "PerspectiveCameraController.h"

#include "Uge/Core/Input.h"
#include "Uge/Core/KeyCodes.h"
#include "Uge/Core/MouseButtonCodes.h"

#include "Uge/Core/Application.h"
#include "Platform/Windows/WindowsWindow.h"


namespace Uge
{


	PerspectiveCameraController::PerspectiveCameraController(float fovYRadians, float aspectRatio, bool rotation, float nearClip, float farClip)
		: m_fovY(glm::radians(fovYRadians)), m_aspectRatio(aspectRatio), m_rotation(rotation), 
		m_nearClip(nearClip), m_farClip(farClip),  
		m_camera(fovYRadians, aspectRatio, nearClip, farClip)
	{

		Application& app = Application::Get();

		// TODO: Needs to be fixed
		auto& window = dynamic_cast<WindowsWindow&>(app.GetWindow());

		m_width = window.GetWidth();
		m_height = window.GetHeight();

		auto [x, y] = Input::GetMousePos();
		m_mouseXPrevPos = x;
		m_mouseYPrevPos = y;
		

	}

	void PerspectiveCameraController::OnUpdate(Timestep ts)
	{
		UG_PROFILE_FUNCTION();

		glm::vec3 forward = m_qRotation * glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 right = m_qRotation * glm::vec3(1.0f, 0.0f, 0.0f);
		glm::vec3 up = m_qRotation * glm::vec3(0.0f, 1.0f, 0.0f);

		glm::vec3 velocity = glm::vec3(0.0f);

		// Camera Position
		if (Input::IsKeyPressed(UG_KEY_A))
			velocity -= right * m_cameraTranslationSpeed * ts.GetMilliseconds();

		else if (Input::IsKeyPressed(UG_KEY_D))
			velocity += right * m_cameraTranslationSpeed * ts.GetMilliseconds();

		if (Input::IsKeyPressed(UG_KEY_W))
			velocity += forward * m_cameraTranslationSpeed * ts.GetMilliseconds();

		else if (Input::IsKeyPressed(UG_KEY_S))
			velocity -= forward * m_cameraTranslationSpeed * ts.GetMilliseconds();
		
		auto [x, y] = Input::GetMousePos();
		m_mouseXDelta = m_mouseXPrevPos - x;
		m_mouseYDelta = m_mouseYPrevPos - y;

		m_mouseXPrevPos = x;
		m_mouseYPrevPos = y;

		if (Input::IsMouseButtonPressed(UG_MOUSE_BUTTON_RIGHT))
		{
			
			if (m_rotation)
			{
				float yaw = (m_mouseXDelta / 1000.0f) * m_cameraRotationSpeed * ts.GetMilliseconds() ;
				float pitch = (m_mouseYDelta / 1000.0f) * m_cameraRotationSpeed * ts.GetMilliseconds();

				glm::quat qYaw = glm::angleAxis(yaw, glm::vec3(0.0f, 1.0f, 0.0f));
				m_qRotation = glm::normalize(qYaw * m_qRotation);

				glm::vec3 rightAxis = m_qRotation * glm::vec3(1.0f, 0.0f, 0.0f);
				glm::quat qPitch = glm::angleAxis(pitch, glm::normalize(rightAxis));
				m_qRotation = glm::normalize(qPitch * m_qRotation);

				// Camera Rotation
				//if (Input::IsKeyPressed(UG_KEY_Q))


				//if (Input::IsKeyPressed(UG_KEY_E))



				//m_camera.SetQuatRotation(m_qRotation);

			}


		}

		
		m_cameraPosition += velocity;
		//m_camera.SetPosition(m_cameraPosition);

	}
	void PerspectiveCameraController::OnEvent(Event& e)
	{
		UG_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(UG_BIND_EVENT_FN(PerspectiveCameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(UG_BIND_EVENT_FN(PerspectiveCameraController::OnWindowResized));



	}


	bool PerspectiveCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{

		UG_PROFILE_FUNCTION();



		return false;
	}
	bool PerspectiveCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		UG_PROFILE_FUNCTION();
		
		m_aspectRatio = (float)e.GetWidth() / (float)e.GetHeight();

		m_width = (float)e.GetWidth();
		m_height = (float)e.GetHeight();

		m_camera.SetProjection(m_fovY, m_aspectRatio, m_nearClip, m_farClip);


		return false;
	}


}