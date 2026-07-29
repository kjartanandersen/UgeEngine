/**
 * @file SceneImporter.h
 * @brief Loads and saves Uge::Scene assets.
 * @ingroup group_asset
 */

#pragma once

#include "Asset.h"
#include "AssetMetadata.h"
#include "Uge/Scene/Scene.h"
namespace Uge
{

	/**
	 * @brief Bridges the asset system and Uge::SceneSerializer.
	 * @ingroup group_asset
	 *
	 * Scenes are assets like any other, so they load through the same importer dispatch;
	 * the actual YAML work is done by Uge::SceneSerializer.
	 */
	class SceneImporter
	{

	public:
		/**
		 * @brief Imports a scene described by registry metadata.
		 * @param handle Handle to assign to the imported scene.
		 * @param metadata Metadata whose file path is resolved against the asset directory.
		 * @return The loaded scene, or null on failure.
		 */
		static Ref<Scene> ImportScene(AssetHandle handle, const AssetMetadata& metadata);

		/**
		 * @brief Loads a scene straight from a path, bypassing the registry.
		 * @param path Filesystem path to the `.uge` file.
		 * @return The loaded scene, or null if the file is missing or malformed.
		 */
		static Ref<Scene> LoadScene(const std::filesystem::path& path);

		/**
		 * @brief Writes a scene to disk as YAML.
		 * @param scene Scene to save.
		 * @param path Destination `.uge` path; overwritten if it exists.
		 */
		static void SaveScene(Ref<Scene> scene, const std::filesystem::path& path);

	};

}