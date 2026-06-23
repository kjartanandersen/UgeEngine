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

	struct MeshMaterialImportData
	{
		std::string Name;
		MaterialTextureMap TextureMaps;
		MaterialProperties Properties;
	};

	struct MeshAssetMetadata
	{
		std::vector<MeshMaterialImportData> Materials;
		std::vector<AssetHandle> Dependencies;

	};

}