/**
 * @file GraphicsContext.h
 * @brief Abstract graphics context owned by the platform window.
 * @ingroup group_renderer
 */

#pragma once

#include <string>

namespace Uge
{

	/**
	 * @brief Human-readable identification of the graphics device and driver.
	 * @ingroup group_renderer
	 *
	 * Filled in by the concrete context during Uge::GraphicsContext::Init and surfaced in
	 * the editor's Debug panel, so a bug report can say which driver produced it.
	 */
	struct GraphicsDeviceInfo
	{
		std::string Vendor;       ///< Driver vendor, e.g. `NVIDIA Corporation`.
		std::string Renderer;     ///< Device name, e.g. `NVIDIA GeForce RTX 4070`.
		std::string Version;      ///< API version string reported by the driver.
		std::string ShadingLanguage; ///< Shading language version string.
		bool DebugOutputEnabled = false; ///< Whether a debug context with message callbacks is active.
	};

	/**
	 * @brief The rendering context bound to a window's surface.
	 * @ingroup group_renderer
	 *
	 * Created and owned by the platform window, not by the renderer: Uge::WindowsWindow
	 * builds an Uge::OpenGLContext for its `GLFWwindow` and calls SwapBuffers() once per
	 * frame from Uge::Window::OnUpdate.
	 */
	class GraphicsContext
	{

	public:
		/**
		 * @brief Makes the context current and loads the API entry points.
		 *
		 * Must complete before any other rendering call.
		 */
		virtual void Init() = 0;
		/** @brief Presents the back buffer to the window. */
		virtual void SwapBuffers() = 0;

		/**
		 * @brief Returns what the driver reported about the device.
		 * @return The info recorded during Init(); empty strings before then.
		 */
		static const GraphicsDeviceInfo& GetDeviceInfo();

		/**
		 * @brief Records the device description; called by the concrete context.
		 * @param info Values read back from the driver.
		 */
		static void SetDeviceInfo(const GraphicsDeviceInfo& info);

	private:
		static GraphicsDeviceInfo s_deviceInfo;
	};



}
