#include <ugpch.h>

#include "MeshImporter.h"


#include "Uge/Project/Project.h"
#include "Uge/Asset/EditorAssetManager.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <assimp/scene.h>
#include <assimp/texture.h>

#include <cstdlib>
#include <fstream>

namespace Uge
{
	Ref<Model> MeshImporter::ImportMesh(AssetHandle handle, const AssetMetadata& metadata)
	{
		UG_PROFILE_FUNCTION();

		std::filesystem::path modelPath = Project::GetAssetDirectory() / metadata.FilePath;

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(
			modelPath.string(),
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType
		);

		if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
		{
			UG_CORE_ERROR("MeshImporter failed to import '{0}': {1}", modelPath.string(), importer.GetErrorString());
			return nullptr;
		}

		MeshAssetMetadata meshMetadata = ImportMaterialData(modelPath, scene);

		auto assetManager = Project::GetActive()->GetEditorAssetManager();
		assetManager->SetMeshMetadata(handle, meshMetadata);

		auto model = CreateRef<Model>(modelPath.string(), meshMetadata);
		model->m_handle = handle;
		return model;
	}

	MeshAssetMetadata MeshImporter::ImportMaterialData(const std::filesystem::path& modelPath, const aiScene* scene)
	{
		MeshAssetMetadata meshMetadata;
		std::filesystem::path modelDirectory = modelPath.parent_path();

		for (uint32_t i = 0; i < scene->mNumMaterials; i++)
		{
			aiMaterial* material = scene->mMaterials[i];

			MeshMaterialImportData materialData;

			aiString materialName;
			if (material->Get(AI_MATKEY_NAME, materialName) == AI_SUCCESS)
				materialData.Name = materialName.C_Str();

			auto appendTextures = [&materialData, &meshMetadata](std::vector<MeshMaterialTextureRef> textureRefs)
			{
				for (const auto& textureRef : textureRefs)
				{
					switch (textureRef.Type)
					{
					case MeshTextureType::Albedo:    materialData.TextureMaps.Albedo = textureRef.Texture; break;
					case MeshTextureType::Normal:    materialData.TextureMaps.Normal = textureRef.Texture; break;
					case MeshTextureType::Roughness: materialData.TextureMaps.Roughness = textureRef.Texture; break;
					case MeshTextureType::Metallic:  materialData.TextureMaps.Metallic = textureRef.Texture; break;
					default: break;
					}

					if (std::find(meshMetadata.Dependencies.begin(), meshMetadata.Dependencies.end(), textureRef.Texture)
						== meshMetadata.Dependencies.end())
					{
						meshMetadata.Dependencies.emplace_back(textureRef.Texture);
					}
				}
			};

			appendTextures(ImportMaterialTextures(modelDirectory, modelPath, scene, material, aiTextureType_DIFFUSE, MeshTextureType::Albedo));
			appendTextures(ImportMaterialTextures(modelDirectory, modelPath, scene, material, aiTextureType_NORMALS, MeshTextureType::Normal));
			appendTextures(ImportMaterialTextures(modelDirectory, modelPath, scene, material, aiTextureType_DIFFUSE_ROUGHNESS, MeshTextureType::Roughness));
			appendTextures(ImportMaterialTextures(modelDirectory, modelPath, scene, material, aiTextureType_METALNESS, MeshTextureType::Metallic));

			meshMetadata.Materials.emplace_back(materialData);
		}

		return meshMetadata;
	}

	std::vector<MeshMaterialTextureRef> MeshImporter::ImportMaterialTextures(
		const std::filesystem::path& modelDirectory, const std::filesystem::path& modelPath,
		const aiScene* scene, aiMaterial* material, int assimpTextureType, MeshTextureType meshTextureType)
	{
		std::vector<MeshMaterialTextureRef> textureRefs;
		aiTextureType textureType = static_cast<aiTextureType>(assimpTextureType);

		for (uint32_t i = 0; i < material->GetTextureCount(textureType); i++)
		{
			aiString assimpPath;
			if (material->GetTexture(textureType, i, &assimpPath) != AI_SUCCESS)
				continue;

			std::filesystem::path texturePath = assimpPath.C_Str();

			if (texturePath.empty())
				continue;

			std::string texturePathString = texturePath.string();

			std::filesystem::path absoluteTexturePath;
			if (!texturePathString.empty() && texturePathString[0] == '*')
			{
				absoluteTexturePath = ExtractEmbeddedTexture(modelDirectory, modelPath, scene, texturePathString);
				if (absoluteTexturePath.empty())
					continue;
			}
			else
			{
				absoluteTexturePath = texturePath.is_absolute()
					? texturePath
					: modelDirectory / texturePath;
			}

			absoluteTexturePath = absoluteTexturePath.lexically_normal();

			if (!std::filesystem::exists(absoluteTexturePath))
			{
				UG_CORE_WARN("Mesh texture not found: {0}", absoluteTexturePath.string());
				continue;
			}

			std::filesystem::path relativeTexturePath =
				std::filesystem::relative(absoluteTexturePath, Project::GetAssetDirectory());

			Ref<EditorAssetManager> assetManager = Project::GetActive()->GetEditorAssetManager();

			AssetHandle textureHandle = assetManager->GetOrImportAsset(relativeTexturePath);

			if (!textureHandle)
				continue;

			textureRefs.push_back({
				meshTextureType,
				textureHandle
				});
		}

		return textureRefs;
	}

	std::filesystem::path MeshImporter::ExtractEmbeddedTexture(
		const std::filesystem::path& modelDirectory, const std::filesystem::path& modelPath,
		const aiScene* scene, const std::string& reference)
	{
		if (!scene)
			return {};

		// reference is of the form "*N" where N is an index into scene->mTextures.
		char* end = nullptr;
		long index = std::strtol(reference.c_str() + 1, &end, 10);
		if (end == reference.c_str() + 1 || *end != '\0' || index < 0 || index >= static_cast<long>(scene->mNumTextures))
		{
			UG_CORE_WARN("Invalid embedded texture reference '{0}'", reference);
			return {};
		}

		const aiTexture* embedded = scene->mTextures[index];
		if (!embedded || !embedded->pcData)
		{
			UG_CORE_WARN("Embedded texture '{0}' has no data", reference);
			return {};
		}

		if (embedded->mHeight != 0)
		{
			// Raw, uncompressed pixel data would need re-encoding before it can be written
			// as a standalone image file. Not handled yet.
			UG_CORE_WARN("Uncompressed embedded texture '{0}' is not supported yet", reference);
			return {};
		}

		// Compressed: pcData holds the raw encoded file bytes (PNG/JPG/...), mWidth = byte count.
		std::string extension = embedded->achFormatHint[0] != '\0' ? embedded->achFormatHint : "png";
		std::string fileName = modelPath.stem().string() + "_embedded_" + std::to_string(index) + "." + extension;
		std::filesystem::path outputPath = (modelDirectory / fileName).lexically_normal();

		if (std::filesystem::exists(outputPath))
			return outputPath;

		std::ofstream out(outputPath, std::ios::binary);
		if (!out)
		{
			UG_CORE_WARN("Failed to write embedded texture to '{0}'", outputPath.string());
			return {};
		}

		out.write(reinterpret_cast<const char*>(embedded->pcData), embedded->mWidth);
		out.close();

		return outputPath;
	}

}
