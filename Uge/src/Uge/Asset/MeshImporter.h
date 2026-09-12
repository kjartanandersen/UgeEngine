/**
 * @file MeshImporter.h
 * @brief Imports 3D model files through assimp, including their materials and textures.
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
	 * The most involved of the importers, because a model file is really a bundle: geometry,
	 * a material per assimp material index, and textures that may be separate files or
	 * embedded in the model itself. The importer resolves all of it and records the result
	 * in Uge::MeshAssetMetadata so a later reimport reuses the same handles.
	 *
	 * Two format quirks are handled explicitly: embedded textures are extracted to disk so
	 * they can go through the normal file-based pipeline, and multi-scene glTF files are
	 * split. @see SplitGltfScenes
	 */
	class MeshImporter
	{

	public:
		/**
		 * @brief Imports a model and every material and texture it references.
		 * @param handle Handle to assign to the imported model.
		 * @param metadata Metadata whose file path is resolved against the asset directory.
		 * @return The loaded model, or null on failure.
		 *
		 * Materials created here have no backing file and are registered with
		 * Uge::EditorAssetManager::AddMemoryOnlyAsset.
		 */
		static Ref<Model> ImportMesh(AssetHandle handle, const AssetMetadata& metadata);

		// A glTF file can declare several scenes, but assimp only ever builds its node graph
		// from the default one, which leaves every other scene's meshes unreachable. This
		// rewrites such a file into one single-scene file per scene, next to the original,
		// and returns their paths in scene order. Returns an empty vector when the file is
		// not glTF, already has a single scene, or could not be read.
		/**
		 * @brief Splits a multi-scene glTF file into one single-scene file per scene.
		 * @param modelPath Path to the glTF or GLB file to inspect.
		 * @return Paths of the generated files, in scene order. Empty when the file is not
		 *         glTF, already contains a single scene, or could not be read.
		 *
		 * A glTF file may declare several scenes, but assimp only ever builds its node graph
		 * from the default one, leaving every other scene's meshes unreachable. Rewriting the
		 * file into one scene per file makes all of them importable.
		 *
		 * @note Writes the new files next to the original.
		 */
		static std::vector<std::filesystem::path> SplitGltfScenes(const std::filesystem::path& modelPath);

	private:
		static MeshAssetMetadata ImportMaterialData(const std::filesystem::path& modelPath, const aiScene* scene);

		// Reads the material's alpha mode and cutoff. glTF states them outright; other formats
		// only have an opacity factor, which is treated as a request for blending.
		static void ImportAlphaMode(aiMaterial* material, MaterialProperties& properties);

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
