/**
 * @file Camera.h
 * @brief Base camera.
 * @ingroup group_renderer
 */

#pragma once

#include <glm/glm.hpp>


namespace Uge
{

	/**
	 * @brief A projection matrix. The view matrix is left to subclasses.
	 * @ingroup group_renderer
	 */
	class Camera
	{

	public:
		/** @brief A camera with an identity projection. */
		Camera() = default;
		/**
		 * @brief A camera with a given projection.
		 * @param projection The projection matrix.
		 */
		Camera(const glm::mat4& projection)
			: m_projection(projection) {}

		/** @brief Virtual destructor. */
		virtual ~Camera() = default;

		/**
		 * @brief The projection matrix.
		 * @return The projection.
		 */
		const glm::mat4& GetProjection() const { return m_projection; };

	protected:
		glm::mat4 m_projection = glm::mat4(1.0f); ///< The projection matrix.

		

	};




}


