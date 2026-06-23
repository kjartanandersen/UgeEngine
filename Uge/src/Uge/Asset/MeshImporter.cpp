#include <ugpch.h>

#include "MeshImporter.h"


#include "Uge/Project/Project.h"
#include "Uge/Asset/EditorAssetManager.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <assimp/scene.h>

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
		(void)meshMetadata;

		auto model = CreateRef<Model>(modelPath.string());
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
					materialData.Textures.emplace_back(textureRef);

					if (std::find(meshMetadata.Dependencies.begin(), meshMetadata.Dependencies.end(), textureRef.Texture)
						== meshMetadata.Dependencies.end())
					{
						meshMetadata.Dependencies.emplace_back(textureRef.Texture);
					}
				}
			};

			appendTextures(ImportMaterialTextures(modelDirectory, material, aiTextureType_DIFFUSE, MeshTextureType::Albedo));
			appendTextures(ImportMaterialTextures(modelDirectory, material, aiTextureType_NORMALS, MeshTextureType::Normal));
			appendTextures(ImportMaterialTextures(modelDirectory, material, aiTextureType_DIFFUSE_ROUGHNESS, MeshTextureType::Roughness));
			appendTextures(ImportMaterialTextures(modelDirectory, material, aiTextureType_METALNESS, MeshTextureType::Metallic));

			meshMetadata.Materials.emplace_back(materialData);
		}

		return meshMetadata;
	}

	std::vector<MeshMaterialTextureRef> MeshImporter::ImportMaterialTextures(
		const std::filesystem::path& modelDirectory, aiMaterial* material, int assimpTextureType, MeshTextureType meshTextureType)
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
			if (!texturePathString.empty() && texturePathString[0] == '*')
			{
				UG_CORE_WARN("Embedded mesh texture '{0}' is not supported yet", texturePath.string());
				continue;
			}

			std::filesystem::path absoluteTexturePath = texturePath.is_absolute()
				? texturePath
				: modelDirectory / texturePath;

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



	
}
