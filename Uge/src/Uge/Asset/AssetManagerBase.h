/**
 * @file AssetManagerBase.h
 * @brief Interface of the asset managers.
 * @ingroup group_asset
 */

#pragma once
#include "Asset.h"

#include <map>

namespace Uge
{
	/**
	 * @brief Loaded assets by handle.
	 * @ingroup group_asset
	 */
	using AssetMap = std::map<AssetHandle, Ref<Asset>>;


	/**
	 * @brief Resolves asset handles to assets.
	 * @ingroup group_asset
	 */
	class AssetManagerBase
	{


	public:
		/**
		 * @brief Returns an asset, loading it if needed.
		 * @param handle Asset to fetch.
		 * @return The asset, or null if the handle is unknown or the import failed.
		 */
		virtual Ref<Asset> GetAsset(AssetHandle handle) = 0;
		/**
		 * @brief Whether a handle names a known asset.
		 * @param handle Handle to test.
		 * @return `true` if the asset is registered or in memory.
		 */
		virtual bool IsAssetHandleValid(AssetHandle handle) const = 0;
		/**
		 * @brief Whether a handle is in the mesh registry.
		 * @param handle Handle to test.
		 * @return `true` if it is.
		 */
		virtual bool IsMeshAssetHandleValid(AssetHandle handle) const = 0;
		/**
		 * @brief Whether an asset is in memory.
		 * @param handle Asset to test.
		 * @return `true` if it is loaded.
		 */
		virtual bool IsAssetLoaded(AssetHandle handle) const = 0;
		/**
		 * @brief An asset's type, without loading it.
		 * @param handle Asset to query.
		 * @return The type, or Uge::AssetType::None for an unknown handle.
		 */
		virtual AssetType GetAssetType(AssetHandle handle) const = 0;

		/**
		 * @brief Unloads an asset, and any dependency no other model uses.
		 * @param handle Asset to unload.
		 *
		 * The handle stays valid, and the next GetAsset() imports the asset again. A `Ref`
		 * held elsewhere keeps the asset alive.
		 */
		virtual void DeleteAsset(AssetHandle handle) = 0;

	};

}