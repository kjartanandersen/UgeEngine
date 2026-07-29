/**
 * @file EditorAssetManager.h
 * @brief Registry-backed asset manager that imports from disk on demand.
 * @ingroup group_asset
 */

#pragma once

#include "AssetManagerBase.h"
#include "AssetMetadata.h"
#include <map>

namespace Uge
{
	/**
	 * @brief The on-disk registry: handle to type and file path.
	 * @ingroup group_asset
	 */
	using AssetRegistry = std::map<AssetHandle, AssetMetadata>;
	/**
	 * @brief Additional per-model registry data: material handles and dependencies.
	 * @ingroup group_asset
	 */
	using MeshAssetRegistry = std::map<AssetHandle, MeshAssetMetadata>;

	/**
	 * @brief Asset manager for the editor: tracks files and imports them lazily.
	 * @ingroup group_asset
	 *
	 * Keeps three structures. The **asset registry** maps each handle to its type and
	 * path and is serialized as YAML to Uge::Project::GetAssetRegistryPath(). The **mesh
	 * registry** adds the per-material handles and dependency list of imported models. The
	 * **loaded-asset cache** holds what is currently in memory.
	 *
	 * Assets load on first GetAsset() rather than up front, so opening a project is cheap
	 * regardless of how much content it contains.
	 *
	 * @note Memory-only assets — chiefly the materials synthesized during mesh import —
	 * live in the cache but never in the registry, since they have no file to reload from.
	 * @see AddMemoryOnlyAsset
	 */
	class EditorAssetManager : public AssetManagerBase
	{

	public:
		/**
		 * @brief Resolves a handle, importing the asset from disk if it is not cached.
		 * @param handle Asset to fetch.
		 * @return The loaded asset, or null if the handle is unknown or the import failed.
		 */
		virtual Ref<Asset> GetAsset(AssetHandle handle) override;
		
		/**
		 * @brief Whether a handle is present in the registry or the loaded cache.
		 * @param handle Handle to test.
		 * @return `true` if the asset is known.
		 */
		virtual bool IsAssetHandleValid(AssetHandle handle) const override;
		/**
		 * @brief Whether a handle is present in the mesh registry.
		 * @param handle Handle to test.
		 * @return `true` if mesh metadata exists for it.
		 */
		virtual bool IsMeshAssetHandleValid(AssetHandle handle) const override;
		/**
		 * @brief Whether an asset is already in the loaded cache.
		 * @param handle Handle to test.
		 * @return `true` if loaded.
		 */
		virtual bool IsAssetLoaded(AssetHandle handle) const override;
		/**
		 * @brief The registered type of an asset.
		 * @param handle Handle to query.
		 * @return The asset type, or Uge::AssetType::None if unknown.
		 */
		virtual AssetType GetAssetType(AssetHandle handle) const override;


		/**
		 * @brief Registers a file as an asset and imports it immediately.
		 * @param filepath Path relative to the project's asset directory.
		 * @param name Optional display name; the file stem is used when empty.
		 * @return The new handle, or `0` if the type could not be determined or import failed.
		 *
		 * Always creates a **new** handle, so importing the same file twice yields two
		 * distinct assets. Use GetOrImportAsset() to avoid that.
		 */
		AssetHandle ImportAsset(const std::filesystem::path& filepath, const std::string& name = "");
		/**
		 * @brief Returns the handle already registered for a file, importing it if there is none.
		 * @param filepath Path relative to the project's asset directory.
		 * @param name Optional display name used only if the asset is newly imported.
		 * @return The existing or newly created handle.
		 *
		 * The safe choice when the same file may be referenced from several places.
		 */
		AssetHandle GetOrImportAsset(const std::filesystem::path& filepath, const std::string& name = "");

		// Registers an asset that has no backing file (e.g. materials generated during
		// mesh import). Assigns and returns a new handle. Memory-only assets are not
		// serialized to the asset registry.
		/**
		 * @brief Registers an asset that has no backing file.
		 * @param asset Asset to take ownership of; receives a freshly generated handle.
		 * @return The new handle.
		 *
		 * Used for materials generated during mesh import. These are deliberately **not**
		 * written to the asset registry — there is no file to reimport them from, so they are
		 * recreated the next time the model is imported.
		 */
		AssetHandle AddMemoryOnlyAsset(const Ref<Asset>& asset);


		/**
		 * @brief The registry entry for an asset.
		 * @param handle Handle to look up.
		 * @return Const reference to its metadata; a default-constructed entry, which converts
		 *         to `false`, when the handle is unknown.
		 */
		const AssetMetadata& GetMetadata(AssetHandle handle) const;
		/**
		 * @brief The source file of an asset.
		 * @param handle Handle to look up.
		 * @return Const reference to the path, relative to the asset directory; empty for
		 *         memory-only assets.
		 */
		const std::filesystem::path& GetFilePath(AssetHandle handle) const;
		/**
		 * @brief The mesh registry entry for a model.
		 * @param handle Model handle to look up.
		 * @return Const reference to its material handles and dependency list.
		 */
		const MeshAssetMetadata& GetMeshMetadata(AssetHandle handle) const;
		/**
		 * @brief Stores or replaces a model's mesh registry entry.
		 * @param handle Model handle.
		 * @param metadata Material handles and dependencies recorded during import.
		 */
		void SetMeshMetadata(AssetHandle handle, const MeshAssetMetadata& metadata);
		/**
		 * @brief Names of every asset currently in memory.
		 * @return One display name per loaded asset; used by the editor's debug panel.
		 */
		std::vector<std::string> GetLoadedAssetsNames();

		/**
		 * @brief The full asset registry.
		 * @return Const reference to the handle-to-metadata map.
		 */
		const AssetRegistry& GetAssetRegistry() const { return m_assetRegistry; }


		/**
		 * @brief Writes the registry to Uge::Project::GetAssetRegistryPath() as YAML.
		 * @note Memory-only assets are excluded.
		 */
		void SerializeAssetRegistry();
		/**
		 * @brief Reads the registry back from the project's registry file.
		 * @return `true` on success; `false` if the file is missing or malformed.
		 */
		bool DeserializeAssetRegistry();

	private:
		AssetRegistry m_assetRegistry;
		MeshAssetRegistry m_meshAssetRegistry;
		AssetMap  m_loadedAssets;

		// Assets with no backing file (e.g. materials generated during mesh import).

	};

}