/**
 * @file TextureImporter.h
 * @brief Loads image files into Uge::Texture2D assets.
 * @ingroup group_asset
 */

#pragma once

#include "Asset.h"
#include "AssetMetadata.h"
#include <Uge/Renderer/Texture.h>

namespace Uge
{

	/**
	 * @brief Imports images from disk using stb_image.
	 * @ingroup group_asset
	 *
	 * Handles the formats stb_image supports — PNG, JPEG, TGA, BMP and others. Images are
	 * flipped vertically on load so that OpenGL's bottom-left texture origin matches the
	 * top-left origin the files use.
	 */
	class TextureImporter
	{

	public:
		/**
		 * @brief Imports a texture described by registry metadata.
		 * @param handle Handle to assign to the imported texture.
		 * @param metadata Metadata whose file path is resolved against the asset directory.
		 * @return The loaded texture, or null on failure.
		 */
		static Ref<Texture2D> ImportTexture2D(AssetHandle handle, const AssetMetadata& metadata);

		/**
		 * @brief Loads a texture straight from a path, bypassing the registry.
		 * @param path Filesystem path to the image.
		 * @return The loaded texture, or null if the file is missing or undecodable.
		 */
		static Ref<Texture2D> LoadTexture2D(const std::filesystem::path& path);


	};

}