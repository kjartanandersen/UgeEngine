/**
 * @file TextureImporter.h
 * @brief Loads images into Uge::Texture2D assets.
 * @ingroup group_asset
 */

#pragma once

#include "Asset.h"
#include "AssetMetadata.h"
#include <Uge/Renderer/Texture.h>

namespace Uge
{

	/**
	 * @brief Loads images with stb_image, flipped vertically for OpenGL.
	 * @ingroup group_asset
	 */
	class TextureImporter
	{

	public:
		/**
		 * @brief Imports a texture.
		 * @param handle Handle to give the texture.
		 * @param metadata Its registry entry.
		 * @return The texture, or null on failure.
		 */
		static Ref<Texture2D> ImportTexture2D(AssetHandle handle, const AssetMetadata& metadata);

		/**
		 * @brief Loads a texture from a file, without the registry.
		 * @param path The image file.
		 * @return The texture, or null if the file is missing or can't be decoded.
		 */
		static Ref<Texture2D> LoadTexture2D(const std::filesystem::path& path);

		/**
		 * @brief Loads an HDR image, such as a `.hdr` file, as floats.
		 * @param path The image file.
		 * @return An Uge::ImageFormat::RGBA32F texture without mips, or null on failure.
		 */
		static Ref<Texture2D> LoadTextureHDR(const std::filesystem::path& path);


	};

}