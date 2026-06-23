#pragma once

#include "Asset.h"
#include "AssetMetadata.h"

#include "Uge/Renderer/Model.h"
#include "Uge/Asset/AssetMetadata.h"

#include <filesystem>
#include <vector>

struct aiScene;
struct aiMaterial;


namespace Uge
{

	class MeshImporter
	{

	public:
		static Ref<Model> ImportMesh(AssetHandle handle, const AssetMetadata& metadata);

	private:
		static MeshAssetMetadata ImportMaterialData(const std::filesystem::path& modelPath, const aiScene* scene);
		static std::vector<MeshMaterialTextureRef> ImportMaterialTextures(
			const std::filesystem::path& modelDirectory, aiMaterial* material, int assimpTextureType, MeshTextureType meshTextureType
		);

	};

}
