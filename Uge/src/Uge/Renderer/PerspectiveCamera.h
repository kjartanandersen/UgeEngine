/**
 * @file PerspectiveCamera.h
 * @brief Perspective projection camera.
 * @ingroup group_renderer
 */

#pragma once

#include "Camera.h"

namespace Uge
{

	/**
	 * @brief A camera with a frustum-shaped view volume, giving depth foreshortening.
	 * @ingroup group_renderer
	 *
	 * @note The field of view is in **radians**, not degrees.
	 */
	class PerspectiveCamera : public Camera
	{


    public:
        /**
         * @brief Constructs the camera.
         * @param fovYRadians Vertical field of view, in radians.
         * @param aspect Width divided by height.
         * @param nearClip Near plane distance; must be greater than zero.
         * @param farClip Far plane distance.
         */
        PerspectiveCamera(float fovYRadians, float aspect, float nearClip, float farClip);

        /**
         * @brief Rebuilds the projection.
         * @param fovYRadians Vertical field of view, in radians.
         * @param aspect Width divided by height.
         * @param nearClip Near plane distance; must be greater than zero.
         * @param farClip Far plane distance.
         */
        void SetProjection(float fovYRadians, float aspect, float nearClip, float farClip);

    private:
        float m_fovY = 0.785398f; // 45 deg in radians
        float m_aspect = 16.0f / 9.0f;
        float m_nearClip = 0.1f;
        float m_farClip = 1000.0f;



	};



}



