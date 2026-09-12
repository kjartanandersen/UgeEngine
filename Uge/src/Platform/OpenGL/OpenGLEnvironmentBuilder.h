/**
 * @file OpenGLEnvironmentBuilder.h
 * @brief Builds the image-based lighting maps by rendering into cubemap faces.
 * @ingroup group_platform
 */

#pragma once

#include "Uge/Renderer/Environment.h"

namespace Uge
{

	/**
	 * @brief Runs the offline passes that turn an equirectangular image into an Uge::Environment.
	 * @ingroup group_platform
	 *
	 * Deliberately outside the Uge::Framebuffer abstraction. Rendering into one face of a
	 * cubemap needs `glFramebufferTexture2D` with a `GL_TEXTURE_CUBE_MAP_POSITIVE_X + i`
	 * target, and into one mip level needs the level argument; the abstraction expresses
	 * neither, and widening its interface for an operation that runs once per environment at
	 * import would cost far more than it returns. The backend-specific calls are contained
	 * here rather than leaking into `Uge/Renderer/`.
	 *
	 * @warning Leaves no GL state behind that the renderer depends on, but does bind and then
	 * unbind a framebuffer, so it must not run inside another pass.
	 */
	class OpenGLEnvironmentBuilder
	{
	public:
		/**
		 * @brief Projects, convolves and prefilters an equirectangular image.
		 * @param equirectangular The source image, as loaded by
		 *        Uge::TextureImporter::LoadTextureHDR.
		 * @return A fully built environment, or null if @p equirectangular is null.
		 */
		static Ref<Environment> Build(const Ref<Texture2D>& equirectangular);
	};

}
