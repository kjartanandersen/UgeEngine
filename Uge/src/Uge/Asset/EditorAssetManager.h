/**
 * @file EditorAssetManager.h
 * @brief The editor's asset manager.
 * @ingroup group_asset
 */

#pragma once

#include "AssetManagerBase.h"
#include "AssetMetadata.h"
#include <map>
#include <unordered_set>

namespace Uge
{
	/**
	 * @brief Each asset's type and file, by handle.
	 * @ingroup group_asset
	 */
	using AssetRegistry = std::map<AssetHandle, AssetMetadata>;

	/**
	 * @brief Each imported model's material handles and dependencies, by handle.
	 * @ingroup group_asset
	 */
	using MeshAssetRegistry = std::map<AssetHandle, MeshAssetMetadata>;

	/**
	 * @brief Asset manager that imports each file on first use.
	 * @ingroup group_asset
	 *
	 * The registry is saved to Uge::Project::GetAssetRegistryPath(). Memory-only assets
	 * are never written to it.
	 */
	class EditorAssetManager : public AssetManagerBase
	{

	public:
		/**
		 * @brief Returns an asset, importing it on first use.
		 * @param handle Asset to fetch.
		 * @return The asset, or null if the handle is unknown or the import failed. A failed
		 * import is not retried.
		 */
		virtual Ref<Asset> GetAsset(AssetHandle handle) override;

		/**
		 * @brief Whether a handle names a known asset.
		 * @param handle Handle to test.
		 * @return `true` if the asset is registered or in memory.
		 */
		virtual bool IsAssetHandleValid(AssetHandle handle) const override;

		/**
		 * @brief Whether a handle is in the mesh registry.
		 * @param handle Handle to test.
		 * @return `true` if it is.
		 */
		virtual bool IsMeshAssetHandleValid(AssetHandle handle) const override;

		/**
		 * @brief Whether an asset is in memory.
		 * @param handle Asset to test.
		 * @return `true` if it is loaded.
		 */
		virtual bool IsAssetLoaded(AssetHandle handle) const override;

		/**
		 * @brief An asset's type, without loading it.
		 * @param handle Asset to query.
		 * @return The type, or Uge::AssetType::None for an unknown handle.
		 */
		virtual AssetType GetAssetType(AssetHandle handle) const override;

		/**
		 * @brief Registers a file and imports it.
		 * @param filepath Path relative to the asset directory.
		 * @param name Display name.
		 * @return A new handle. The asset is registered only if the import succeeds.
		 *
		 * Asserts on an unknown file extension. Each call creates a new handle, even for a
		 * file that is already registered.
		 */
		AssetHandle ImportAsset(const std::filesystem::path& filepath, const std::string& name = "");

		/**
		 * @brief Returns the handle registered for a file, importing the file if there is none.
		 * @param filepath Path relative to the asset directory.
		 * @param name Display name, used only for a new import.
		 * @return The handle.
		 */
		AssetHandle GetOrImportAsset(const std::filesystem::path& filepath, const std::string& name = "");

		/**
		 * @brief Adds an asset that has no file. It is never written to the registry.
		 * @param asset Asset to add; it receives the new handle.
		 * @return The new handle.
		 */
		AssetHandle AddMemoryOnlyAsset(const Ref<Asset>& asset);

		/**
		 * @brief An asset's registry entry.
		 * @param handle Asset to look up.
		 * @return The entry, or an empty one (which converts to `false`) for an unknown handle.
		 */
		const AssetMetadata& GetMetadata(AssetHandle handle) const;

		/**
		 * @brief An asset's file.
		 * @param handle Asset to look up.
		 * @return Path relative to the asset directory; empty for a memory-only asset.
		 */
		const std::filesystem::path& GetFilePath(AssetHandle handle) const;

		/**
		 * @brief A model's mesh registry entry.
		 * @param handle Model to look up.
		 * @return The entry, or an empty one for an unknown handle.
		 */
		const MeshAssetMetadata& GetMeshMetadata(AssetHandle handle) const;

		/**
		 * @brief Sets a model's mesh registry entry.
		 * @param handle Model.
		 * @param metadata Its material handles and dependencies.
		 */
		void SetMeshMetadata(AssetHandle handle, const MeshAssetMetadata& metadata);

		/**
		 * @brief Names of the assets in memory.
		 * @return One display name per loaded asset.
		 */
		std::vector<std::string> GetLoadedAssetsNames();

		/**
		 * @brief The asset registry.
		 * @return Every registered asset's entry.
		 */
		const AssetRegistry& GetAssetRegistry() const { return m_assetRegistry; }

		/**
		 * @brief Writes the registry file.
		 */
		void SerializeAssetRegistry();

		/**
		 * @brief Reads the registry file.
		 * @return `false` if the file is missing, unreadable or has no registry. A missing
		 * file is created empty.
		 */
		bool DeserializeAssetRegistry();

		/**
		 * @brief Unloads an asset, and any dependency no other model uses.
		 * @param handle Asset to unload.
		 *
		 * The handle stays registered, and the next GetAsset() imports the asset again.
		 */
		virtual void DeleteAsset(AssetHandle handle) override;

		/**
		 * @brief Unloads a model, and the materials and textures no other model uses.
		 * @param handle Model to unload.
		 *
		 * Removes its mesh registry entry; a reimport recreates it.
		 */
		void DeleteModel(AssetHandle handle);

		/**
		 * @brief Unloads a texture.
		 * @param handle Texture to unload.
		 */
		void DeleteTexture(AssetHandle handle);

		/**
		 * @brief Unloads a material.
		 * @param handle Material to unload.
		 *
		 * A memory-only material can't be loaded again; reimporting its model recreates it.
		 */
		void DeleteMaterial(AssetHandle handle);

		/**
		 * @brief Unloads a scene, and lets a failed import of it be retried.
		 * @param handle Scene to unload.
		 */
		void DeleteScene(AssetHandle handle);

	private:
		/**
		 * @brief Whether any model in the mesh registry lists an asset as a dependency.
		 * @param dependency Material or texture.
		 * @return `true` if one does.
		 */
		bool IsDependencyOfAnyModel(AssetHandle dependency) const;

	private:

		AssetRegistry m_assetRegistry;
		MeshAssetRegistry m_meshAssetRegistry;
		AssetMap  m_loadedAssets;
		std::unordered_set<AssetHandle> m_failedImports; ///< Handles whose import failed.


	};

}