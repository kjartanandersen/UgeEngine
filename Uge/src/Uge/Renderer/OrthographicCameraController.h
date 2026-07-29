/**
 * @file OrthographicCameraController.h
 * @brief Keyboard and scroll-wheel controller for an orthographic camera.
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
	 * @brief The camera's visible rectangle in world units.
	 * @ingroup group_renderer
	 */
	struct OrthographicCameraBounds
	{
		/** @brief Horizontal extent in world units. */
		float Left, Right; ///< Horizontal extent in world units.
		/** @brief Vertical extent in world units. */
		float Bottom, Top; ///< Vertical extent in world units.

		/** @brief Width of the visible area. @return `Right - Left`, in world units. */
		float GetWidth() { return Right - Left; }
		/** @brief Height of the visible area. @return `Top - Bottom`, in world units. */
		float GetHeight() { return Top - Bottom; }
	};


	/**
	 * @brief Drives an Uge::OrthographicCamera from keyboard and mouse-wheel input.
	 * @ingroup group_renderer
	 *
	 * Owns its camera and keeps the projection consistent with the zoom level and aspect
	 * ratio. Forward both callbacks from the layer that owns it:
	 *
	 * @code
	 * void Sandbox2D::OnUpdate(Timestep ts) { m_cameraController.OnUpdate(ts); }
	 * void Sandbox2D::OnEvent(Event& e)     { m_cameraController.OnEvent(e); }
	 * @endcode
	 *
	 * @note Zooming changes the visible world area, so movement speed is scaled by the zoom
	 * level to keep panning feeling consistent.
	 */
	class OrthographicCameraController
	{

	public:
		/**
		 * @brief Constructs the controller and its camera.
		 * @param aspectRatio Viewport width divided by height.
		 * @param rotation `true` to enable Q/E roll control.
		 */
		OrthographicCameraController(float aspectRatio, bool rotation = false);

		/**
		 * @brief Applies held-key movement for this frame.
		 * @param ts Frame delta time, so movement is frame-rate independent.
		 */
		void OnUpdate(Timestep ts);
		/**
		 * @brief Handles scroll-wheel zoom and window resize.
		 * @param e Event to inspect.
		 */
		void OnEvent(Event& e);

		/**
		 * @brief Updates the aspect ratio and rebuilds the projection.
		 * @param width New viewport width in pixels.
		 * @param height New viewport height in pixels.
		 */
		void OnResize(float width, float height);

		/**
		 * @brief The controlled camera.
		 * @return Mutable reference to the camera.
		 */
		OrthographicCamera& GetCamera() { return m_camera; }
		/**
		 * @brief The controlled camera.
		 * @return Const reference to the camera.
		 */
		const OrthographicCamera& GetCamera() const { return m_camera; }

		/**
		 * @brief Sets the zoom level and rebuilds the projection.
		 * @param zLevel Half-height of the visible area in world units; larger zooms out.
		 */
		void SetZoomLevel(float zLevel) { m_zoomLevel = zLevel; CalculateView(); }
		/**
		 * @brief The current zoom level.
		 * @return Half-height of the visible area in world units.
		 */
		float GetZoomLevel() const { return m_zoomLevel; }


		/**
		 * @brief The visible world-space rectangle.
		 * @return Const reference to the current bounds.
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
		float m_cameraRotation = 0.0f; //In degrees, in the anti-clockwise direction
		float m_cameraTranslationSpeed = 5.0f, m_cameraRotationSpeed = 180.0f;

	};


}


