/**
 * @file GraphicsContext.h
 * @brief The graphics context.
 * @ingroup group_renderer
 */

#pragma once

#include <string>

namespace Uge
{

	/**
	 * @brief What the driver reports about the GPU.
	 * @ingroup group_renderer
	 */
	struct GraphicsDeviceInfo
	{
		std::string Vendor;       ///< Such as `NVIDIA Corporation`.
		std::string Renderer;     ///< The GPU's name.
		std::string Version;      ///< API version.
		std::string ShadingLanguage; ///< Shading language version.
		bool DebugOutputEnabled = false; ///< Whether debug output is on.
	};

	/**
	 * @brief A window's rendering context, owned by the window.
	 * @ingroup group_renderer
	 */
	class GraphicsContext
	{

	public:
		/**
		 * @brief Makes the context current and loads the API. Call before any rendering.
		 */
		virtual void Init() = 0;
		/** @brief Presents the back buffer to the window. */
		virtual void SwapBuffers() = 0;

		/**
		 * @brief What the driver reported, as recorded by Init().
		 * @return The info; empty before Init().
		 */
		static const GraphicsDeviceInfo& GetDeviceInfo();

		/**
		 * @brief Stores the device info.
		 * @param info The info.
		 */
		static void SetDeviceInfo(const GraphicsDeviceInfo& info);

	private:
		static GraphicsDeviceInfo s_deviceInfo;
	};



}
