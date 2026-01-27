#pragma once

#include <glm/glm.hpp>


namespace Uge
{

	class Camera
	{

	public:
		Camera(const glm::mat4& projection) 
			: m_projection(projection) {}

		const glm::mat4& GetProjection() const { return m_projection; };

		// TODO:
		// SetPerspective
		// SetOrtho
	private:
		glm::mat4 m_projection;

		

	};




}


