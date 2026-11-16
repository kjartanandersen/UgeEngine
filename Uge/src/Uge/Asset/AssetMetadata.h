/**
 * @file AssetMetadata.h
 * @brief Registry entries for assets and imported models.
 * @ingroup group_asset
 */

#pragma once

#include "Asset.h"

#include <filesystem>
#include <Uge/Renderer/Material.h>

namespace Uge
{

	/**
	 * @brief An asset's registry entry: its type and file.
	 * @ingroup group_asset
	 */
	struct AssetMetadata
	{
		AssetType Type = AssetType::None; ///< The asset's type.
		std::filesystem::path FilePath; ///< Path relative to the project's asset directory.

		/**
		 * @brief Whether the entry is set.
		 * @return `true` unless the type is Uge::AssetType::None.
		 */
		operator bool() const { return Type != AssetType::None; }
	};

	/**
	 * @brief Material texture slots filled by mesh import.
	 * @ingroup group_asset
	 */
	enum class MeshTextureType
	{
		None = 0, ///< No slot.
		Albedo, ///< Base colour.
		Normal, ///< Tangent-space normals.
		Roughness,
		Metallic,
		AmbientOcclusion,
		Emissive
	};

	/**
	 * @brief A texture found by mesh import, and the slot it fills.
	 * @ingroup group_asset
	 */
	struct MeshMaterialTextureRef
	{
		MeshTextureType Type = MeshTextureType::None; ///< The slot it fills.
		AssetHandle Texture = 0; ///< The texture.
	};

	/**
	 * @brief Registry data for an imported model.
	 * @ingroup group_asset
	 */
	struct MeshAssetMetadata
	{
		std::vector<AssetHandle> MaterialHandles; ///< One material per assimp material index, kept so a reimport reuses the handles.

		std::vector<AssetHandle> Dependencies; ///< Every texture and material the model imported.

	};

}