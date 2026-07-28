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

		// A glTF file can declare several scenes, but assimp only ever builds its node graph
		// from the default one, which leaves every other scene's meshes unreachable. This
		// rewrites such a file into one single-scene file per scene, next to the original,
		// and returns their paths in scene order. Returns an empty vector when the file is
		// not glTF, already has a single scene, or could not be read.
		static std::vector<std::filesystem::path> SplitGltfScenes(const std::filesystem::path& modelPath);

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
