/**
 * @file TextureCube.h
 * @brief Cubemap textures, used for environment lighting and the skybox.
 * @ingroup group_renderer
 */

#pragma once

#include "Uge/Core/Core.h"
#include "Uge/Renderer/Texture.h"

namespace Uge
{

	/**
	 * @brief Describes the cubemap to create.
	 * @ingroup group_renderer
	 *
	 * Cube faces are square, so one #Size describes both dimensions.
	 */
	struct TextureCubeSpecification
	{
		uint32_t Size = 1; ///< Edge length of each face, in pixels.
		ImageFormat Format = ImageFormat::RGBA16F; ///< Pixel format; environment data is HDR.
		bool GenerateMips = false; ///< Whether to allocate a mip chain.

		/**
		 * @brief Levels to allocate; `0` means a full chain down to 1x1.
		 *
		 * Only meaningful when #GenerateMips is set. A prefiltered map writes each level
		 * explicitly, so it must allocate exactly as many as it fills — allocating a full
		 * chain leaves the tail undefined, and a rough surface indexing into it samples
		 * whatever the driver left in the allocation.
		 */
		uint32_t MipLevels = 0;
	};

	/**
	 * @brief A six-faced cubemap texture.
	 * @ingroup group_renderer
	 *
	 * Created empty and filled by rendering into its faces rather than by uploading pixels:
	 * the three maps image-based lighting needs — the environment itself, its diffuse
	 * irradiance and its roughness-prefiltered reflection — are all derived on the GPU from a
	 * single equirectangular source image. @see Uge::Environment
	 *
	 * @note A prefiltered map stores roughness across its mip chain, so it must be created
	 * with TextureCubeSpecification::GenerateMips set; the level count is what the shader's
	 * roughness lookup is scaled against.
	 */
	class TextureCube
	{
	public:
		/** @brief Releases the GPU texture. */
		virtual ~TextureCube() = default;

		/**
		 * @brief The specification this cubemap was created from.
		 * @return Const reference to the specification.
		 */
		virtual const TextureCubeSpecification& GetSpecification() const = 0;

		/** @brief Edge length of one face. @return Size in pixels. */
		virtual uint32_t GetSize() const = 0;
		/**
		 * @brief Number of mip levels allocated.
		 * @return Level count; `1` when the cubemap has no mip chain.
		 */
		virtual uint32_t GetMipLevelCount() const = 0;
		/**
		 * @brief The backend's native handle.
		 * @return The OpenGL texture name.
		 */
		virtual uint32_t GetRendererID() const = 0;

		/**
		 * @brief Binds the cubemap to a sampler slot.
		 * @param slot Texture unit to bind to.
		 */
		virtual void Bind(uint32_t slot = 0) const = 0;

		/**
		 * @brief Fills the mip chain from level 0.
		 *
		 * Called after rendering into the base level. Does nothing when the cubemap has no
		 * mip chain.
		 *
		 * @warning Not for prefiltered maps: those write each level deliberately, and
		 * generating mips afterwards would overwrite that work with plain downsamples.
		 */
		virtual void GenerateMips() = 0;

		/**
		 * @brief Creates a cubemap for the active graphics API.
		 * @param specification Face size, format and mip settings.
		 * @return The backend's cubemap implementation.
		 */
		static Ref<TextureCube> Create(const TextureCubeSpecification& specification);
	};

}
