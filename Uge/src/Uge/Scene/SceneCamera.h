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
	 * @brief A camera owned by an entity, switchable between perspective and orthographic.
	 * @ingroup group_scene
	 *
	 * Holds only the projection; the view matrix comes from the owning entity's
	 * Uge::TransformComponent. The parameters of both projection types are kept
	 * independently, so toggling the type back and forth does not lose settings.
	 *
	 * Every setter recalculates the projection immediately, which is what lets the
	 * property panel edit these values live.
	 */
	class SceneCamera : public Camera
	{

	public:
		/** @brief Which projection the camera builds. */
		enum class ProjectionType { Perspective = 0, Orthographic = 1};

	public:
		/** @brief Constructs an orthographic camera with default settings. */
		SceneCamera();
		/** @brief Virtual destructor. */
		virtual ~SceneCamera() = default;

		/**
		 * @brief Switches to an orthographic projection and sets its parameters.
		 * @param size Vertical extent of the visible area, in world units.
		 * @param nearClip Near plane distance.
		 * @param farClip Far plane distance.
		 */
		void SetOrtho(float size, float nearClip, float farClip);
		/**
		 * @brief Switches to a perspective projection and sets its parameters.
		 * @param fovy Vertical field of view, in radians.
		 * @param nearClip Near plane distance; must be greater than zero.
		 * @param farClip Far plane distance.
		 */
		void SetPersp(float fovy, float nearClip, float farClip);

		/**
		 * @brief Updates the aspect ratio from a viewport size.
		 * @param width Viewport width in pixels.
		 * @param height Viewport height in pixels; ignored if zero.
		 */
		void SetViewportSize(uint32_t width, uint32_t height);


		/************************
		 * Orthographic Functions
		 ************************/

		/**
		 * @brief Orthographic vertical extent.
		 * @return Size in world units.
		 */
		float GetOrthoSize() const { return m_orthographicSize; };
		/** @brief Sets the orthographic vertical extent. @param size Size in world units. */
		void SetOrthoSize(float size) 
		{
			m_orthographicSize = size; 
			RecalculateProjection();
		};

		/** @brief Orthographic near plane. @return Distance to the near plane. */
		float GetOrthoNearClip() const { return m_orthographicNear; };
		/** @brief Sets the orthographic near plane. @param nearClip New distance. */
		void SetOrthoNearClip(float nearClip) 
		{  
			m_orthographicNear = nearClip;
			RecalculateProjection();
		}

		/** @brief Orthographic far plane. @return Distance to the far plane. */
		float GetOrthoFarClip() const { return m_orthographicFar; };
		/** @brief Sets the orthographic far plane. @param farClip New distance. */
		void SetOrthoFarClip(float farClip)
		{
			m_orthographicFar = farClip;
			RecalculateProjection();
		}

		/************************
		 * Perspective Functions
		 ************************/

		/**
		 * @brief Perspective field of view.
		 * @return Vertical FOV in **degrees**.
		 */
		float GetPerspVerticalFOV() const { return glm::degrees(m_perspectiveFOV); };
		/**
		 * @brief Sets the perspective field of view.
		 * @param fovy Vertical FOV in **degrees**; stored internally as radians.
		 */
		void SetPerspVerticalFOV(float fovy)
		{
			m_perspectiveFOV = glm::radians(fovy);
			RecalculateProjection();
		};

		/** @brief Perspective near plane. @return Distance to the near plane. */
		float GetPerspNearClip() const { return m_perspectivecNear; };
		/** @brief Sets the perspective near plane. @param nearClip New distance. */
		void SetPerspNearClip(float nearClip)
		{
			m_perspectivecNear = nearClip;
			RecalculateProjection();
		}

		/** @brief Perspective far plane. @return Distance to the far plane. */
		float GetPerspFarClip() const { return m_perspectivecFar; };
		/** @brief Sets the perspective far plane. @param farClip New distance. */
		void SetPerspFarClip(float farClip)
		{
			m_perspectivecFar = farClip;
			RecalculateProjection();
		}



		
		/** @brief The active projection type. @return Perspective or orthographic. */
		ProjectionType GetProjectionType() const { return m_projectionType; }
		/**
		 * @brief Switches projection type, keeping both parameter sets intact.
		 * @param type Projection to use.
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