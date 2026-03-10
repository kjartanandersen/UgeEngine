#include <ugpch.h>
#include "EditorCamera.h"

#include "Uge/Core/Input.h"
#include "Uge/Core/KeyCodes.h"
#include "Uge/Core/MouseButtonCodes.h"


namespace Uge
{

	EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip)
		: m_FOV(fov), m_aspectRatio(aspectRatio), m_nearClip(nearClip), m_farClip(farClip), Camera(glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip))
	{
		glm::vec3 zeroPosDir = glm::normalize(glm::vec3(0, 0, 0) - m_position);
		LookAt(zeroPosDir, glm::vec3(0.0f, 1.0f, 0.0f));
		UpdateView();

	}

	

	void EditorCamera::UpdateProjection()
	{
		m_aspectRatio = m_viewportWidth / m_viewportHeight;
		m_projection = glm::perspective(glm::radians(m_FOV), m_aspectRatio, m_nearClip, m_farClip);
	}

	void EditorCamera::UpdateView()
	{
		// m_Yaw = m_Pitch = 0.0f; // Lock the camera's rotation
		//m_position = CalculatePosition();

		// glm::quat orientation = GetOrientation();

		glm::mat4 rotation = glm::mat4_cast(glm::conjugate(m_orientation));
		glm::mat4 translation = glm::translate(glm::mat4(1.0f), -m_position);

		m_viewMatrix = rotation * translation;

		//m_viewMatrix = glm::translate(glm::mat4(1.0f), m_position) * glm::toMat4(orientation);
		//m_viewMatrix = glm::inverse(m_viewMatrix);

		
	}

	std::pair<float, float> EditorCamera::PanSpeed() const
	{
		float x = std::min(m_viewportWidth / 1000.0f, 2.4f); // max = 2.4f
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		float y = std::min(m_viewportHeight / 1000.0f, 2.4f); // max = 2.4f
		float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float EditorCamera::RotationSpeed() const
	{
		return 20.0f;
	}

	float EditorCamera::MovementSpeed() const
	{
		return 16.0f;
	}

	float EditorCamera::ZoomSpeed() const
	{
		float distance = m_distance * 0.2f;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		speed = std::min(speed, 100.0f); // max speed = 100
		return speed;
	}

	void EditorCamera::OnUpdate(Timestep ts)
	{
		const glm::vec2& mouse{ Input::GetMouseX(), Input::GetMouseY() };
		glm::vec2 delta = (mouse - m_initialMousePosition) * (ts * RotationSpeed());
		m_initialMousePosition = mouse;
		if (Input::IsKeyPressed(KeyCode::UG_KEY_LEFT_ALT))
		{
			

			// if (Input::IsMouseButtonPressed(MouseButton::UG_MOUSE_BUTTON_MIDDLE))
			// 	MousePan(delta);
			
			// else if (Input::IsMouseButtonPressed(MouseButton::UG_MOUSE_BUTTON_RIGHT))
			// 	MouseZoom(delta.y);
		}

		if (Input::IsMouseButtonPressed(MouseButton::UG_MOUSE_BUTTON_RIGHT))
		{
			MouseRotate(delta);
			glm::vec3 moveVec = { 0, 0, 0 };

			if (Input::IsKeyPressed(KeyCode::UG_KEY_W))
			{
				moveVec += GetForwardDirection() * (MovementSpeed() * ts);
			}

			if (Input::IsKeyPressed(KeyCode::UG_KEY_S))
			{
				moveVec -= GetForwardDirection() * (MovementSpeed() * ts);
			}

			if (Input::IsKeyPressed(KeyCode::UG_KEY_A))
			{
				moveVec -= GetRightDirection() * (MovementSpeed() * ts);
			}

			if (Input::IsKeyPressed(KeyCode::UG_KEY_D))
			{
				moveVec += GetRightDirection() * (MovementSpeed() * ts);
			}

			m_position += moveVec;
		}



		UpdateView();
	}

	void EditorCamera::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(UG_BIND_EVENT_FN(EditorCamera::OnMouseScroll));
	}

	bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e)
	{
		// float delta = e.GetYOffset() * 0.1f;
		// MouseZoom(delta);
		// UpdateView();
		return false;
	}

	void EditorCamera::MousePan(const glm::vec2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		m_focalPoint += -GetRightDirection() * delta.x * xSpeed * m_distance;
		m_focalPoint += GetUpDirection() * delta.y * ySpeed * m_distance;
	}

	void EditorCamera::MouseRotate(const glm::vec2& delta)
	{

		// float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		// m_Yaw += yawSign * delta.x * RotationSpeed();
		// m_pitch += delta.y * RotationSpeed();
	
		glm::quat yaw = glm::angleAxis(glm::radians(-delta.x ), glm::vec3(0, 1, 0));

		// 2. Clamp and Handle Pitch (Vertical)
		float newPitch = m_currentPitch + delta.y;
		float newYOffset = delta.y;

		// Lock pitch between -89 and 89 degrees to prevent flipping/singularities
		if (newPitch > 89.0f) 
		{
			newYOffset = (89.0f - m_currentPitch);
			m_currentPitch = 89.0f;
		}
		else if (newPitch < -89.0f) 
		{
			newYOffset = (-89.0f - m_currentPitch);
			m_currentPitch = -89.0f;
		}
		else 
		{
			m_currentPitch = newPitch;
			
		}

		// UG_CORE_INFO("Current Pitch: {0}", m_currentPitch);

		// Vertical rotation around the camera's local Right axis
		glm::quat pitch = glm::angleAxis(glm::radians(-newYOffset), glm::vec3(1, 0, 0));

		// Update orientation: Yaw happens in world space, Pitch in local space
		m_orientation = glm::normalize(yaw * m_orientation * pitch);
	}

	void EditorCamera::MouseZoom(float delta)
	{
		m_distance -= delta * ZoomSpeed();
		if (m_distance < 1.0f)
		{
			m_focalPoint += GetForwardDirection();
			m_distance = 1.0f;
		}
	}

	glm::vec3 EditorCamera::GetRightDirection() const
	{
		return m_orientation * glm::vec3(1.0f, 0.0f, 0.0f);
		// return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
	}

	glm::vec3 EditorCamera::GetUpDirection() const
	{
		return m_orientation * glm::vec3(0.0f, 1.0f, 0.0f);
		// return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::vec3 EditorCamera::GetForwardDirection() const
	{
		return m_orientation * glm::vec3(0.0f, 0.0f, -1.0f);
		// return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
	}

	glm::vec3 EditorCamera::CalculatePosition() const
	{
		return m_focalPoint - GetForwardDirection() * m_distance;
	}

	glm::quat EditorCamera::GetOrientation() const
	{
		return glm::quat(glm::vec3(-m_pitch, -m_Yaw, 0.0f));
	}

	void EditorCamera::LookAt(const glm::vec3& direction, const glm::vec3& up)
	{

		m_orientation = glm::quatLookAt(direction, up);

	}


}