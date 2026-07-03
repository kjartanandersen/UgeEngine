#pragma once

#include "Asset.h"

#include <filesystem>
#include <Uge/Renderer/Material.h>

namespace Uge
{

	struct AssetMetadata
	{
		AssetType Type = AssetType::None;
		std::filesystem::path FilePath;

		operator bool() const { return Type != AssetType::None; }
	};

	enum class MeshTextureType
	{
		None = 0,
		Albedo,
		Normal,
		Roughness,
		Metallic,
		AmbientOcclusion,
		Emissive
	};

	struct MeshMaterialTextureRef
	{
		MeshTextureType Type = MeshTextureType::None;
		AssetHandle Texture = 0;
	};

	struct MeshAssetMetadata
	{
		// One Material asset handle per assimp material index in the source model.
		std::vector<AssetHandle> MaterialHandles;

		// All assets (textures + materials) this mesh depends on.
		std::vector<AssetHandle> Dependencies;

	};

}