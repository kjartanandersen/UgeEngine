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
			const std::filesystem::path& modelDirectory, const std::filesystem::path& modelPath,
			const aiScene* scene, aiMaterial* material, int assimpTextureType, MeshTextureType meshTextureType
		);

		// Writes an embedded ("*N") texture out to a file next to the model so it can be
		// imported through the normal file-based asset pipeline. Returns the absolute path,
		// or an empty path on failure.
		static std::filesystem::path ExtractEmbeddedTexture(
			const std::filesystem::path& modelDirectory, const std::filesystem::path& modelPath,
			const aiScene* scene, const std::string& reference
		);

	};

}
