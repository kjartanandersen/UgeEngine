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
	 * @brief Loads and saves scenes.
	 * @ingroup group_asset
	 */
	class SceneImporter
	{

	public:
		/**
		 * @brief Imports a scene.
		 * @param handle Handle to give the scene.
		 * @param metadata Its registry entry.
		 * @return The scene, or null on failure.
		 */
		static Ref<Scene> ImportScene(AssetHandle handle, const AssetMetadata& metadata);

		/**
		 * @brief Loads a scene from a file, without the registry.
		 * @param path The `.uge` file.
		 * @return The scene, or null if the file is missing or malformed.
		 */
		static Ref<Scene> LoadScene(const std::filesystem::path& path);

		/**
		 * @brief Saves a scene.
		 * @param scene Scene to save.
		 * @param path The `.uge` file; replaced if it exists.
		 * @return `true` on success. On failure the reason is logged and the file is unchanged.
		 */
		static bool SaveScene(Ref<Scene> scene, const std::filesystem::path& path);

	};

}