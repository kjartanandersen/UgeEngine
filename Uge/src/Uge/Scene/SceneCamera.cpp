#include <ugpch.h>
#include "SceneCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Uge
{
	SceneCamera::SceneCamera()
	{
		RecalculateProjection();


	}

	void SceneCamera::SetOrtho(float size, float nearClip, float farClip)
	{
		m_projectionType = ProjectionType::Orthographic;
		m_orthographicSize = size;
		m_orthographicNear = nearClip;
		m_orthographicFar = farClip;

		RecalculateProjection();



	}

	void Uge::SceneCamera::SetPersp(float fovy, float nearClip, float farClip)
	{
		m_projectionType = ProjectionType::Perspective;
		m_perspectiveFOV = fovy;
		m_perspectivecNear = nearClip;
		m_perspectivecFar = farClip;

		RecalculateProjection();


	}

	void SceneCamera::SetViewportSize(uint32_t width, uint32_t height)
	{
		UG_CORE_ASSERT(width > 0 && height > 0);

		m_aspectRatio = (float)width / (float)height;
		RecalculateProjection();
		


	}

	void SceneCamera::RecalculateProjection()
	{

		switch (m_projectionType)
		{
		case Uge::SceneCamera::ProjectionType::Perspective:
			m_projection = glm::perspective(m_perspectiveFOV, m_aspectRatio, m_perspectivecNear, m_perspectivecFar);
			break;

		case Uge::SceneCamera::ProjectionType::Orthographic:
			float orthoLeft = -m_orthographicSize * m_aspectRatio * 0.5f;
			float orthoRight = m_orthographicSize * m_aspectRatio * 0.5f;
			float orthoBottom = -m_orthographicSize * 0.5f;
			float orthoTop = m_orthographicSize * 0.5f;


			m_projection = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop,
				m_orthographicNear, m_orthographicFar);
			break;


		}




	}

}