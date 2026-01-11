#include "ugpch.h"
#include "PerspectiveCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Uge
{


    PerspectiveCamera::PerspectiveCamera(float fovYRadians, float aspect, float nearClip, float farClip)
    {
        m_usingQuat = true;
        SetProjection(fovYRadians, aspect, nearClip, farClip);
    }

    void PerspectiveCamera::SetProjection(float fovYRadians, float aspect, float nearClip, float farClip)
    {
        m_fovY = fovYRadians;
        m_aspect = aspect;
        m_nearClip = nearClip;
        m_farClip = farClip;

        SetProjectionMatrix(glm::perspective(fovYRadians, aspect, nearClip, farClip));
    }



}