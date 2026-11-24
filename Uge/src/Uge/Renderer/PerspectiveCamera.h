/**
 * @file PerspectiveCamera.h
 * @brief Perspective camera.
 * @ingroup group_renderer
 */

#pragma once

#include "Camera.h"

namespace Uge
{

	/**
	 * @brief A camera with a perspective projection. The field of view is in radians.
	 * @ingroup group_renderer
	 */
	class PerspectiveCamera : public Camera
	{


    public:
        /**
         * @brief A camera with the given projection.
         * @param fovYRadians Vertical field of view in radians.
         * @param aspect Width divided by height.
         * @param nearClip Near plane distance, above zero.
         * @param farClip Far plane distance.
         */
        PerspectiveCamera(float fovYRadians, float aspect, float nearClip, float farClip);

        /**
         * @brief Rebuilds the projection.
         * @param fovYRadians Vertical field of view in radians.
         * @param aspect Width divided by height.
         * @param nearClip Near plane distance, above zero.
         * @param farClip Far plane distance.
         */
        void SetProjection(float fovYRadians, float aspect, float nearClip, float farClip);

    private:
        float m_fovY = 0.785398f; // 45 degrees
        float m_aspect = 16.0f / 9.0f;
        float m_nearClip = 0.1f;
        float m_farClip = 1000.0f;



	};



}



