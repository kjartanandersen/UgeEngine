/**
 * @file OrthographicCamera.h
 * @brief Orthographic projection camera.
 * @ingroup group_renderer
 */

#pragma once

#include <glm/glm.hpp>

#include "Camera.h"

namespace Uge
{


	/**
	 * @brief A camera with a box-shaped view volume and no perspective foreshortening.
	 * @ingroup group_renderer
	 *
	 * The projection for 2D rendering: object size on screen does not vary with distance.
	 * Bounds are given in world units, so the aspect ratio has to be folded into them by
	 * the caller — Uge::OrthographicCameraController does that from a zoom level.
	 */
	class OrthographicCamera : public Camera
	{


	public:
		/**
		 * @brief Constructs the camera from its view-volume bounds.
		 * @param left Left plane, in world units.
		 * @param right Right plane, in world units.
		 * @param bottom Bottom plane, in world units.
		 * @param top Top plane, in world units.
		 */
		OrthographicCamera(float left, float right, float bottom, float top);

		/**
		 * @brief Rebuilds the projection from new bounds.
		 * @param left Left plane, in world units.
		 * @param right Right plane, in world units.
		 * @param bottom Bottom plane, in world units.
		 * @param top Top plane, in world units.
		 */
		void SetProjection(float left, float right, float bottom, float top);



	private:
		float m_left = -1.0f;
		float m_right = 1.0f;
		float m_bottom = -1.0f;
		float m_top = 1.0f;

	};


}

