/**
 * @file TextureCube.h
 * @brief Cubemap textures.
 * @ingroup group_renderer
 */

#pragma once

#include "Uge/Core/Core.h"
#include "Uge/Renderer/Texture.h"

namespace Uge
{

	/**
	 * @brief The settings a cubemap is created from.
	 * @ingroup group_renderer
	 */
	struct TextureCubeSpecification
	{
		uint32_t Size = 1; ///< Width and height of a face, in pixels.
		ImageFormat Format = ImageFormat::RGBA16F; ///< Pixel format.
		bool GenerateMips = false; ///< Whether to allocate mip levels.

		/**
		 * @brief Mip levels to allocate, if #GenerateMips is set; `0` for a full chain.
		 *
		 * A map that writes each level itself should allocate only the levels it writes.
		 */
		uint32_t MipLevels = 0;
	};

	/**
	 * @brief A cubemap, created empty and filled by rendering into its faces.
	 * @ingroup group_renderer
	 */
	class TextureCube
	{
	public:
		/** @brief Releases the texture. */
		virtual ~TextureCube() = default;

		/**
		 * @brief The specification.
		 * @return The specification.
		 */
		virtual const TextureCubeSpecification& GetSpecification() const = 0;

		/** @brief The face size. @return Size in pixels. */
		virtual uint32_t GetSize() const = 0;
		/**
		 * @brief The number of mip levels.
		 * @return The count; `1` without mipmaps.
		 */
		virtual uint32_t GetMipLevelCount() const = 0;
		/**
		 * @brief The OpenGL texture name.
		 * @return The name.
		 */
		virtual uint32_t GetRendererID() const = 0;

		/**
		 * @brief Binds the cubemap to a slot.
		 * @param slot Texture unit.
		 */
		virtual void Bind(uint32_t slot = 0) const = 0;

		/**
		 * @brief Generates the mip levels from level 0. Does nothing without mipmaps.
		 *
		 * Don't call it on a map whose levels were rendered separately: it overwrites them.
		 */
		virtual void GenerateMips() = 0;

		/**
		 * @brief Creates a cubemap.
		 * @param specification Size, format and mipmaps.
		 * @return The cubemap.
		 */
		static Ref<TextureCube> Create(const TextureCubeSpecification& specification);
	};

}
