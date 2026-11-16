/**
 * @file SceneCamera.h
 * @brief The camera stored in an entity's Uge::CameraComponent.
 * @ingroup group_scene
 */

#pragma once

#include "Uge/Renderer/Camera.h"

namespace Uge
{

	/**
	 * @brief An entity's camera projection, perspective or orthographic.
	 * @ingroup group_scene
	 *
	 * The view comes from the entity's transform. Both projections keep their settings, so
	 * switching between them loses nothing.
	 */
	class SceneCamera : public Camera
	{

	public:
		/** @brief The projection type. */
		enum class ProjectionType { Perspective = 0, Orthographic = 1};

	public:
		/** @brief An orthographic camera with default settings. */
		SceneCamera();
		/** @brief Virtual destructor. */
		virtual ~SceneCamera() = default;

		/**
		 * @brief Switches to orthographic and sets it up.
		 * @param size Visible height, in world units.
		 * @param nearClip Near plane.
		 * @param farClip Far plane.
		 */
		void SetOrtho(float size, float nearClip, float farClip);
		/**
		 * @brief Switches to perspective and sets it up.
		 * @param fovy Vertical field of view, in radians.
		 * @param nearClip Near plane; above zero.
		 * @param farClip Far plane.
		 */
		void SetPersp(float fovy, float nearClip, float farClip);

		/**
		 * @brief Sets the aspect ratio from a viewport size.
		 * @param width Viewport width in pixels.
		 * @param height Viewport height in pixels; ignored if zero.
		 */
		void SetViewportSize(uint32_t width, uint32_t height);


		/************************
		 * Orthographic Functions
		 ************************/

		/**
		 * @brief The orthographic visible height.
		 * @return Height in world units.
		 */
		float GetOrthoSize() const { return m_orthographicSize; };
		/** @brief Sets the orthographic visible height. @param size Height in world units. */
		void SetOrthoSize(float size) 
		{
			m_orthographicSize = size; 
			RecalculateProjection();
		};

		/** @brief The orthographic near plane. @return Its distance. */
		float GetOrthoNearClip() const { return m_orthographicNear; };
		/** @brief Sets the orthographic near plane. @param nearClip Its distance. */
		void SetOrthoNearClip(float nearClip) 
		{  
			m_orthographicNear = nearClip;
			RecalculateProjection();
		}

		/** @brief The orthographic far plane. @return Its distance. */
		float GetOrthoFarClip() const { return m_orthographicFar; };
		/** @brief Sets the orthographic far plane. @param farClip Its distance. */
		void SetOrthoFarClip(float farClip)
		{
			m_orthographicFar = farClip;
			RecalculateProjection();
		}

		/************************
		 * Perspective Functions
		 ************************/

		/**
		 * @brief The perspective vertical field of view.
		 * @return Degrees.
		 */
		float GetPerspVerticalFOV() const { return glm::degrees(m_perspectiveFOV); };
		/**
		 * @brief Sets the perspective vertical field of view.
		 * @param fovy In degrees.
		 */
		void SetPerspVerticalFOV(float fovy)
		{
			m_perspectiveFOV = glm::radians(fovy);
			RecalculateProjection();
		};

		/** @brief The perspective near plane. @return Its distance. */
		float GetPerspNearClip() const { return m_perspectivecNear; };
		/** @brief Sets the perspective near plane. @param nearClip Its distance. */
		void SetPerspNearClip(float nearClip)
		{
			m_perspectivecNear = nearClip;
			RecalculateProjection();
		}

		/** @brief The perspective far plane. @return Its distance. */
		float GetPerspFarClip() const { return m_perspectivecFar; };
		/** @brief Sets the perspective far plane. @param farClip Its distance. */
		void SetPerspFarClip(float farClip)
		{
			m_perspectivecFar = farClip;
			RecalculateProjection();
		}




		/** @brief The projection type. @return The type. */
		ProjectionType GetProjectionType() const { return m_projectionType; }
		/**
		 * @brief Switches the projection type.
		 * @param type The type.
		 */
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