#pragma once

#include "Uge/Renderer/Camera.h"

namespace Uge
{

	class SceneCamera : public Camera
	{

	public:
		enum class ProjectionType { Perspective = 0, Orthographic = 1};

	public:
		SceneCamera();
		virtual ~SceneCamera() = default;

		void SetOrtho(float size, float nearClip, float farClip);
		void SetPersp(float fovy, float nearClip, float farClip);

		void SetViewportSize(uint32_t width, uint32_t height);


		/************************
		 * Orthographic Functions
		 ************************/

		float GetOrthoSize() const { return m_orthographicSize; };
		void SetOrthoSize(float size) 
		{
			m_orthographicSize = size; 
			RecalculateProjection();
		};

		float GetOrthoNearClip() const { return m_orthographicNear; };
		void SetOrthoNearClip(float nearClip) 
		{  
			m_orthographicNear = nearClip;
			RecalculateProjection();
		}

		float GetOrthoFarClip() const { return m_orthographicFar; };
		void SetOrthoFarClip(float farClip)
		{
			m_orthographicFar = farClip;
			RecalculateProjection();
		}

		/************************
		 * Perspective Functions
		 ************************/

		float GetPerspVerticalFOV() const { return glm::degrees(m_perspectiveFOV); };
		void SetPerspVerticalFOV(float fovy)
		{
			m_perspectiveFOV = glm::radians(fovy);
			RecalculateProjection();
		};

		float GetPerspNearClip() const { return m_perspectivecNear; };
		void SetPerspNearClip(float nearClip)
		{
			m_perspectivecNear = nearClip;
			RecalculateProjection();
		}

		float GetPerspFarClip() const { return m_perspectivecFar; };
		void SetPerspFarClip(float farClip)
		{
			m_perspectivecFar = farClip;
			RecalculateProjection();
		}



		
		ProjectionType GetProjectionType() const { return m_projectionType; }
		void SetProjectionType(ProjectionType type) 
		{ 
			m_projectionType = type; RecalculateProjection();
		}

	private:
		void RecalculateProjection();

	private:
		ProjectionType m_projectionType = ProjectionType::Orthographic;

		float m_orthographicSize =  10.0f;
		float m_orthographicNear = -1.0f, m_orthographicFar =  1.0f;

		float m_perspectiveFOV = glm::radians(45.0f);
		float m_perspectivecNear = 0.01f, m_perspectivecFar = 1000.0f;

		float m_aspectRatio = 0.0f;


	};


}