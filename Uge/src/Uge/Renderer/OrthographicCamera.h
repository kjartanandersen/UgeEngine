#pragma once

#include <glm/glm.hpp>

#include "Camera.h"

namespace Uge
{


	class OrthographicCamera : public Camera
	{


	public:
		OrthographicCamera(float left, float right, float bottom, float top);

		void SetProjection(float left, float right, float bottom, float top);



	private:
		float m_left = -1.0f;
		float m_right = 1.0f;
		float m_bottom = -1.0f;
		float m_top = 1.0f;

	};


}

