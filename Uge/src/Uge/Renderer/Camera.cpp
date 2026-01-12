#include "ugpch.h"
#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Uge
{



    void Camera::RecalculateViewMatrix()
    {
        UG_PROFILE_FUNCTION();

        glm::mat4 transform;

        if (m_usingQuat)
        {
            transform = glm::translate(glm::mat4(1.0f), m_position) *
                glm::mat4_cast(m_qRotation);



        }
        else
        {
            transform = glm::translate(glm::mat4(1.0f), m_position) *
                glm::rotate(glm::mat4(1.0f), glm::radians(m_rotation), glm::vec3(0, 0, 1));



        }
        m_viewMatrix = glm::inverse(transform);
        m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;

    }

}