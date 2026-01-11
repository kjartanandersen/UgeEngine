#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


namespace Uge
{

	class Camera
	{

	public:
		virtual ~Camera() = default;

		const glm::vec3& GetPosition() const { return m_position; }
		float GetRotation() const { return m_rotation; }
		
		void SetPosition(const glm::vec3& position)
		{
			m_position = position;
			RecalculateViewMatrix();
		}
		void SetRotation(float rotation)
		{
			m_rotation = rotation;
			RecalculateViewMatrix();
		}

		void SetQuatRotation(glm::quat qRotation)
		{
			m_qRotation = qRotation;
			RecalculateViewMatrix();

		}

		void SetPositionAndRotation(const glm::vec3& position, float rotation)
		{
			m_position = position;
			m_rotation = rotation;
			RecalculateViewMatrix();
		}

		const glm::mat4& GetProjectionMatrix() const { return m_projectionMatrix; }
		const glm::mat4& GetViewMatrix() const { return m_viewMatrix; }
		const glm::mat4& GetViewProjectionMatrix() const { return m_viewProjectionMatrix; }



	protected:
		void RecalculateViewMatrix();

		void SetProjectionMatrix(const glm::mat4& projection)
		{
			m_projectionMatrix = projection;
			m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
		}


	protected:
		glm::mat4 m_projectionMatrix;
		glm::mat4 m_viewMatrix;
		glm::mat4 m_viewProjectionMatrix;

		glm::vec3 m_position = { 0.0f, 0.0f, 0.0f };
		glm::quat m_qRotation = { 1.0f, 0.0f, 0.0f, 0.0f };
		float m_rotation = 0.0f;
		bool m_usingQuat;

	private:

		

	};




}


