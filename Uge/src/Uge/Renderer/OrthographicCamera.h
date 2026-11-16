/**
 * @file OrthographicCamera.h
 * @brief Orthographic camera.
 * @ingroup group_renderer
 */

#pragma once

#include <glm/glm.hpp>

#include "Camera.h"

namespace Uge
{


	/**
	 * @brief A camera with an orthographic projection.
	 * @ingroup group_renderer
	 *
	 * The bounds are in world units, so they have to include the aspect ratio.
	 */
	class OrthographicCamera : public Camera
	{


	public:
		/**
		 * @brief A camera with the given bounds.
		 * @param left Left edge.
		 * @param right Right edge.
		 * @param bottom Bottom edge.
		 * @param top Top edge.
		 */
		OrthographicCamera(float left, float right, float bottom, float top);

		/**
		 * @brief Rebuilds the projection with new bounds.
		 * @param left Left edge.
		 * @param right Right edge.
		 * @param bottom Bottom edge.
		 * @param top Top edge.
		 */
		void SetProjection(float left, float right, float bottom, float top);



	private:
		float m_left = -1.0f;
		float m_right = 1.0f;
		float m_bottom = -1.0f;
		float m_top = 1.0f;

	};


}

