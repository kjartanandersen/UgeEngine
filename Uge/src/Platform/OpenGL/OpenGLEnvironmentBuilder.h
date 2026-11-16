/**
 * @file OpenGLEnvironmentBuilder.h
 * @brief Builds an environment's lighting maps.
 * @ingroup group_platform
 */

#pragma once

#include "Uge/Renderer/Environment.h"

namespace Uge
{

	/**
	 * @brief Renders an equirectangular image into an Uge::Environment's maps.
	 * @ingroup group_platform
	 *
	 * Uses GL directly, as Uge::Framebuffer can't target a cubemap face or mip level.
	 */
	class OpenGLEnvironmentBuilder
	{
	public:
		/**
		 * @brief Builds the skybox, irradiance, prefiltered and BRDF maps.
		 *
		 * Restores the bound framebuffer and viewport, and leaves face culling off.
		 *
		 * @param equirectangular The HDR source image.
		 * @return The environment, or null if @p equirectangular is null.
		 */
		static Ref<Environment> Build(const Ref<Texture2D>& equirectangular);
	};

}
