/**
 * @file AssetImporter.h
 * @brief Dispatches an import to the right type-specific importer.
 * @ingroup group_asset
 */

#pragma once

#include "Asset.h"
#include "AssetMetadata.h"



namespace Uge
{

	/**
	 * @brief Routes an asset import to the importer for its type.
	 * @ingroup group_asset
	 *
	 * The single place that knows the mapping from Uge::AssetType to a concrete importer,
	 * so the asset managers never name Uge::TextureImporter or Uge::MeshImporter directly.
	 * Adding an asset type means adding a branch here.
	 */
	class AssetImporter
	{

	public:
		/**
		 * @brief Imports an asset by dispatching on its metadata type.
		 * @param handle Handle to assign to the imported asset.
		 * @param metadata Type and file path of the asset to import.
		 * @return The imported asset, or null if the type has no importer or loading failed.
		 */
		static Ref<Asset> ImportAsset(AssetHandle handle, const AssetMetadata& metadata);

	};

}