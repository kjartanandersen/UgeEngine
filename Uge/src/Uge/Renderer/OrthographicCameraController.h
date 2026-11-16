/**
 * @file OrthographicCameraController.h
 * @brief Input controller for an orthographic camera.
 * @ingroup group_renderer
 */

#pragma once

#include "Uge/Renderer/OrthographicCamera.h"
#include "Uge/Core/Timestep.h"

#include "Uge/Events/ApplicationEvent.h"
#include "Uge/Events/MouseEvent.h"

namespace Uge
{

	/**
	 * @brief The visible rectangle, in world units.
	 * @ingroup group_renderer
	 */
	struct OrthographicCameraBounds
	{
		/** @brief Horizontal edges. */
		float Left, Right; ///< Horizontal edges.
		/** @brief Vertical edges. */
		float Bottom, Top; ///< Vertical edges.

		/** @brief The width. @return `Right - Left`. */
		float GetWidth() { return Right - Left; }
		/** @brief The height. @return `Top - Bottom`. */
		float GetHeight() { return Top - Bottom; }
	};


	/**
	 * @brief Owns an Uge::OrthographicCamera and fits its projection to the zoom level and
	 * aspect ratio.
	 * @ingroup group_renderer
	 */
	class OrthographicCameraController
	{

	public:
		/**
		 * @brief A controller with a zoom level of 1.
		 * @param aspectRatio Width divided by height.
		 * @param rotation Whether Q and E rotate.
		 */
		OrthographicCameraController(float aspectRatio, bool rotation = false);

		/**
		 * @brief Moves with W, A, S and D, and rotates with Q and E.
		 *
		 * The position and rotation aren't passed to the camera, which has only a projection.
		 *
		 * @param ts Frame time.
		 */
		void OnUpdate(Timestep ts);
		/**
		 * @brief Zooms on scroll, in steps of 0.2 between 0.1 and 10, and handles resizes.
		 * @param e The event.
		 */
		void OnEvent(Event& e);

		/**
		 * @brief Sets the aspect ratio and rebuilds the projection.
		 * @param width Width in pixels.
		 * @param height Height in pixels.
		 */
		void OnResize(float width, float height);

		/**
		 * @brief The camera.
		 * @return The camera.
		 */
		OrthographicCamera& GetCamera() { return m_camera; }
		/**
		 * @brief The camera.
		 * @return The camera.
		 */
		const OrthographicCamera& GetCamera() const { return m_camera; }

		/**
		 * @brief Sets the zoom level and rebuilds the projection.
		 * @param zLevel Half the visible height, in world units.
		 */
		void SetZoomLevel(float zLevel) { m_zoomLevel = zLevel; CalculateView(); }
		/**
		 * @brief The zoom level.
		 * @return Half the visible height, in world units.
		 */
		float GetZoomLevel() const { return m_zoomLevel; }


		/**
		 * @brief The visible rectangle.
		 * @return The bounds.
		 */
		const OrthographicCameraBounds& GetBounds() const { return m_bounds; }

	private:
		void CalculateView();
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);

	private:

		float m_aspectRatio;
		float m_zoomLevel = 1.0f;
		bool m_rotation;

		OrthographicCameraBounds m_bounds;
		OrthographicCamera m_camera;
		glm::vec3 m_cameraPosition = { 0.0f, 0.0f, 0.0f };
		float m_cameraRotation = 0.0f; // Degrees, anticlockwise.
		float m_cameraTranslationSpeed = 5.0f, m_cameraRotationSpeed = 180.0f;

	};


}


