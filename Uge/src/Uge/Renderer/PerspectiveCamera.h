#pragma once

#include "Camera.h"

namespace Uge
{

	class PerspectiveCamera : public Camera
	{


    public:
        PerspectiveCamera(float fovYRadians, float aspect, float nearClip, float farClip);

        void SetProjection(float fovYRadians, float aspect, float nearClip, float farClip);

    private:
        float m_fovY = 0.785398f; // 45 deg in radians
        float m_aspect = 16.0f / 9.0f;
        float m_nearClip = 0.1f;
        float m_farClip = 1000.0f;



	};



}



