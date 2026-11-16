/**
 * @file AssetImporter.h
 * @brief Imports an asset with the importer for its type.
 * @ingroup group_asset
 */

#pragma once

#include "Asset.h"
#include "AssetMetadata.h"



namespace Uge
{

	/**
	 * @brief Imports an asset with the importer for its type.
	 * @ingroup group_asset
	 */
	class AssetImporter
	{

	public:
		/**
		 * @brief Imports an asset.
		 * @param handle Handle to give the asset.
		 * @param metadata The asset's type and file.
		 * @return The asset, or null if the type has no importer or the import failed.
		 */
		static Ref<Asset> ImportAsset(AssetHandle handle, const AssetMetadata& metadata);

	};

}