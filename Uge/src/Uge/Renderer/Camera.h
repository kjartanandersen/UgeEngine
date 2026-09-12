/**
 * @file Camera.h
 * @brief Base camera holding a projection matrix.
 * @ingroup group_renderer
 */

#pragma once

#include <glm/glm.hpp>


namespace Uge
{

	/**
	 * @brief Base class for anything that supplies a projection matrix.
	 * @ingroup group_renderer
	 *
	 * Deliberately minimal: it holds the projection only, leaving the view matrix to
	 * subclasses and callers. Uge::SceneCamera pairs it with an entity's transform, while
	 * Uge::EditorCamera manages its own orbit view.
	 */
	class Camera
	{

	public:
		/** @brief Constructs a camera with an identity projection. */
		Camera() = default;
		/**
		 * @brief Constructs a camera with an explicit projection.
		 * @param projection Projection matrix to use.
		 */
		Camera(const glm::mat4& projection) 
			: m_projection(projection) {}

		/** @brief Virtual destructor. */
		virtual ~Camera() = default;

		/**
		 * @brief The projection matrix.
		 * @return Const reference to the projection.
		 */
		const glm::mat4& GetProjection() const { return m_projection; };

	protected:
		glm::mat4 m_projection = glm::mat4(1.0f); ///< Projection matrix; subclasses rebuild it.

		

	};




}


