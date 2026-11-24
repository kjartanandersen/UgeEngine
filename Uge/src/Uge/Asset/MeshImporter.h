/**
 * @file MeshImporter.h
 * @brief Imports models through assimp, with their materials and textures.
 * @ingroup group_asset
 */

#pragma once

#include "Asset.h"
#include "AssetMetadata.h"

#include "Uge/Renderer/Material.h"
#include "Uge/Renderer/Model.h"
#include "Uge/Asset/AssetMetadata.h"

#include <filesystem>
#include <vector>

struct aiScene;
struct aiMaterial;


namespace Uge
{

	/**
	 * @brief Imports model files into Uge::Model assets.
	 * @ingroup group_asset
	 *
	 * A reimport reuses the material handles recorded in Uge::MeshAssetMetadata.
	 */
	class MeshImporter
	{

	public:
		/**
		 * @brief Imports a model with its materials and textures.
		 * @param handle Handle to give the model.
		 * @param metadata Its registry entry.
		 * @return The model, or null on failure.
		 *
		 * The materials are added as memory-only assets.
		 */
		static Ref<Model> ImportMesh(AssetHandle handle, const AssetMetadata& metadata);

		/**
		 * @brief Splits a multi-scene glTF file into one file per scene, next to the original.
		 * @param modelPath The glTF or GLB file.
		 * @return The new files in scene order; empty if the file isn't glTF, has one scene or
		 * can't be read.
		 *
		 * assimp only builds a glTF file's default scene, so the others would be unreachable.
		 */
		static std::vector<std::filesystem::path> SplitGltfScenes(const std::filesystem::path& modelPath);

	private:
		static MeshAssetMetadata ImportMaterialData(const std::filesystem::path& modelPath, const aiScene* scene);

		// Reads the alpha mode and cutoff. Without a glTF alpha mode, an opacity below 1
		// means blending.
		static void ImportAlphaMode(aiMaterial* material, MaterialProperties& properties);

		static std::vector<MeshMaterialTextureRef> ImportMaterialTextures(
			const std::filesystem::path& modelDirectory, const std::filesystem::path& modelPath,
			const aiScene* scene, aiMaterial* material, int assimpTextureType, MeshTextureType meshTextureType
		);

		// Writes an embedded ("*N") texture to a file next to the model. Returns its
		// absolute path, or an empty path on failure.
		static std::filesystem::path ExtractEmbeddedTexture(
			const std::filesystem::path& modelDirectory, const std::filesystem::path& modelPath,
			const aiScene* scene, const std::string& reference
		);

	};

}
