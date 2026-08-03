/**
 * @file AssetManagerBase.h
 * @brief The interface both asset managers implement.
 * @ingroup group_asset
 */

#pragma once
#include "Asset.h"

#include <map>

namespace Uge
{
	/**
	 * @brief Handle-to-asset map used for the loaded-asset cache.
	 * @ingroup group_asset
	 */
	using AssetMap = std::map<AssetHandle, Ref<Asset>>;


	/**
	 * @brief Interface for resolving asset handles into loaded assets.
	 * @ingroup group_asset
	 *
	 * Two implementations exist, chosen by the active project:
	 * Uge::EditorAssetManager, which is registry-backed and imports from disk on demand,
	 * and Uge::RuntimeAssetManager, which serves pre-packed assets.
	 *
	 * Call this through the static Uge::AssetManager facade rather than directly.
	 */
	class AssetManagerBase
	{


	public:
		/**
		 * @brief Resolves a handle, loading the asset if it is not in memory yet.
		 * @param handle Asset to fetch.
		 * @return The loaded asset, or null if the handle is unknown or loading failed.
		 * @note May hit the disk, so avoid calling it in a tight per-frame loop.
		 */
		virtual Ref<Asset> GetAsset(AssetHandle handle) = 0;
		/**
		 * @brief Whether a handle refers to a known asset.
		 * @param handle Handle to test.
		 * @return `true` if the asset is registered, whether or not it is loaded.
		 */
		virtual bool IsAssetHandleValid(AssetHandle handle) const = 0;
		/**
		 * @brief Whether a handle refers to a known mesh asset.
		 * @param handle Handle to test.
		 * @return `true` if the handle is present in the mesh registry.
		 */
		virtual bool IsMeshAssetHandleValid(AssetHandle handle) const = 0;
		/**
		 * @brief Whether an asset is currently in memory.
		 * @param handle Handle to test.
		 * @return `true` if the asset is loaded, so GetAsset() will not hit the disk.
		 */
		virtual bool IsAssetLoaded(AssetHandle handle) const = 0;
		/**
		 * @brief The type of a registered asset.
		 * @param handle Handle to query.
		 * @return The asset type, or Uge::AssetType::None if the handle is unknown.
		 */
		virtual AssetType GetAssetType(AssetHandle handle) const = 0;

		/**
		 * @brief Unloads an asset from memory, along with anything only it depended on.
		 * @param handle Asset to release.
		 *
		 * Unloads, it does not unregister: the handle stays valid and GetAsset() reimports
		 * the asset on the next call. Removing an asset from the project is a separate
		 * operation.
		 *
		 * @warning Only drops the manager's own reference. Anything still holding a
		 * `Ref<Asset>` keeps the asset — and its GPU resources — alive.
		 */
		virtual void DeleteAsset(AssetHandle handle) = 0;

	};

}