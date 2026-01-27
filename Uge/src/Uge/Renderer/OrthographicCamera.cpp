#include <ugpch.h>
#include "OrthographicCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Uge
{

	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
		: Camera({1.0f}), m_left(left), m_right(right), m_bottom(bottom), m_top(top)
	{

		//m_usingQuat = false;
		//SetProjectionMatrix(glm::ortho(left, right, bottom, top, -1.0f, 1.0f));

	}

	void OrthographicCamera::SetProjection(float left, float right, float bottom, float top)
	{
		m_left = left;
		m_right = right;
		m_bottom = bottom;
		m_top = top;

		//SetProjectionMatrix(glm::ortho(left, right, bottom, top, -1.0f, 1.0f));
	}

	

}