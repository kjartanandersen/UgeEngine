/**
 * @file AssetMetadata.h
 * @brief Registry metadata describing where an asset lives and what it depends on.
 * @ingroup group_asset
 */

#pragma once

#include "Asset.h"

#include <filesystem>
#include <Uge/Renderer/Material.h>

namespace Uge
{

	/**
	 * @brief What the registry stores about an asset: its type and its file.
	 * @ingroup group_asset
	 *
	 * Converts to `false` when the type is Uge::AssetType::None, which is how a lookup
	 * miss is reported:
	 * @code
	 * const auto& metadata = manager->GetMetadata(handle);
	 * if (!metadata)
	 *     UG_CORE_WARN("Unknown asset handle");
	 * @endcode
	 */
	struct AssetMetadata
	{
		AssetType Type = AssetType::None; ///< What kind of asset this is.
		std::filesystem::path FilePath; ///< Path relative to the project's asset directory.

		/**
		 * @brief Whether this metadata describes a real asset.
		 * @return `true` if the type is set.
		 */
		operator bool() const { return Type != AssetType::None; }
	};

	/**
	 * @brief The texture slots a mesh material can populate during import.
	 * @ingroup group_asset
	 *
	 * Mirrors the maps in Uge::MaterialTextureMap; the importer maps assimp's texture
	 * types onto these.
	 */
	enum class MeshTextureType
	{
		None = 0, ///< Unset / invalid.
		Albedo, ///< Base colour map.
		Normal, ///< Tangent-space normal map.
		Roughness, ///< Surface roughness map.
		Metallic, ///< Metalness map.
		AmbientOcclusion, ///< Baked ambient occlusion map.
		Emissive ///< Emission map.
	};

	/**
	 * @brief One texture discovered during mesh import, tagged with the slot it fills.
	 * @ingroup group_asset
	 */
	struct MeshMaterialTextureRef
	{
		MeshTextureType Type = MeshTextureType::None; ///< Which material slot this texture fills.
		AssetHandle Texture = 0; ///< Handle of the imported texture.
	};

	/**
	 * @brief Extra registry data for an imported model.
	 * @ingroup group_asset
	 *
	 * Persisting the per-material handles is what makes reimporting a model stable: the
	 * regenerated materials keep the handles the scene already refers to, instead of
	 * getting fresh ones and breaking every reference.
	 *
	 * #Dependencies lists everything the model pulled in, so the assets it brought with it
	 * can be released when it is unloaded.
	 */
	struct MeshAssetMetadata
	{
		// One Material asset handle per assimp material index in the source model.
		std::vector<AssetHandle> MaterialHandles; ///< One material handle per assimp material index in the source model.

		// All assets (textures + materials) this mesh depends on.
		std::vector<AssetHandle> Dependencies; ///< Every asset — textures and materials — this model depends on.

	};

}