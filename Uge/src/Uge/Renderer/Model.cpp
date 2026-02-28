#include <ugpch.h>
#include "Model.h"

#include <filesystem>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <assimp/scene.h>

namespace Uge
{

	namespace
	{
		struct CameraData
		{
			glm::mat4 ViewProjection;
		};

		struct ModelData
		{
			glm::mat4 ModelTransform;
			glm::ivec4 EntityData;
		};
	}

	Model::SceneData Model::s_sceneData;

	Model::Model(const std::string& path)
	{
		LoadModel(path);
	}

	void Model::EnsureSceneResources()
	{
		if (s_sceneData.Initialized)
		{
			return;
		}

		s_sceneData.ModelShader = Shader::Create("assets/shaders/Model.glsl");
		s_sceneData.CameraUniformBuffer = UniformBuffer::Create(sizeof(CameraData), 0);
		s_sceneData.ModelUniformBuffer = UniformBuffer::Create(sizeof(ModelData), 1);
		s_sceneData.Initialized = true;
	}

	void Model::BeginScene(const glm::mat4& viewProjection)
	{
		EnsureSceneResources();

		CameraData cameraData{};
		cameraData.ViewProjection = viewProjection;
		s_sceneData.CameraUniformBuffer->SetData(&cameraData, sizeof(CameraData));
	}

	void Model::EndScene()
	{
	}

	void Model::Draw(const glm::mat4& transform, int entityID) const
	{
		if (!s_sceneData.Initialized || m_meshes.empty())
		{
			return;
		}

		s_sceneData.ModelShader->Bind();
		ModelData modelData{};
		modelData.ModelTransform = transform;
		modelData.EntityData = glm::ivec4(entityID, 0, 0, 0);
		s_sceneData.ModelUniformBuffer->SetData(&modelData, sizeof(ModelData));

		for (const auto& mesh : m_meshes)
		{
			mesh.Draw(s_sceneData.ModelShader, entityID);
		}
	}

	void Model::LoadModel(const std::string& path)
	{
		m_path = path;
		m_meshes.clear();
		m_loadedTextures.clear();

		if (path.empty())
		{
			UG_CORE_WARN("Model::LoadModel called with an empty path");
			return;
		}

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path,
			aiProcess_Triangulate |
			aiProcess_GenSmoothNormals |
			aiProcess_PreTransformVertices |
			aiProcess_FlipUVs);

		if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
		{
			UG_CORE_ERROR("Failed to load model '{0}': {1}", path, importer.GetErrorString());
			return;
		}

		std::filesystem::path modelPath(path);
		m_directory = modelPath.has_parent_path() ? modelPath.parent_path().string() : std::string();

		ProcessNode(scene->mRootNode, scene);
	}

	void Model::ProcessNode(aiNode* node, const aiScene* scene)
	{
		for (uint32_t i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			m_meshes.emplace_back(ProcessMesh(mesh, scene));
		}

		for (uint32_t i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene);
		}
	}

	Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		std::vector<MeshVertex> vertices;
		std::vector<uint32_t> indices;
		std::vector<Ref<Texture2D>> textures;

		float diffuseTextureIndex = -1.0f;
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			auto diffuseMaps = LoadMaterialTextures(material, (int)aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

			for (uint32_t i = 0; i < textures.size(); i++)
			{
				const auto& texture = textures[i];
				if (texture && texture->m_name == "texture_diffuse")
				{
					diffuseTextureIndex = static_cast<float>(i);
					break;
				}
			}
		}

		vertices.reserve(mesh->mNumVertices);
		for (uint32_t i = 0; i < mesh->mNumVertices; i++)
		{
			MeshVertex vertex{};

			vertex.Position.x = mesh->mVertices[i].x;
			vertex.Position.y = mesh->mVertices[i].y;
			vertex.Position.z = mesh->mVertices[i].z;

			if (mesh->HasNormals())
			{
				vertex.Normal.x = mesh->mNormals[i].x;
				vertex.Normal.y = mesh->mNormals[i].y;
				vertex.Normal.z = mesh->mNormals[i].z;
			}
			else
			{
				vertex.Normal = { 0.0f, 0.0f, 1.0f };
			}

			if (mesh->mTextureCoords[0])
			{
				vertex.TexCoord.x = mesh->mTextureCoords[0][i].x;
				vertex.TexCoord.y = mesh->mTextureCoords[0][i].y;
				vertex.HasDiffuseMap = diffuseTextureIndex >= 0.0f ? 1 : 0;
				vertex.TexIndex = diffuseTextureIndex;
			}
			else
			{
				vertex.TexCoord = { 0.0f, 0.0f };
				vertex.HasDiffuseMap = 0;
				vertex.TexIndex = -1.0f;
			}
			vertex.EntityID = -1;

			vertices.emplace_back(vertex);
		}

		for (uint32_t i = 0; i < mesh->mNumFaces; i++)
		{
			const aiFace& face = mesh->mFaces[i];
			for (uint32_t j = 0; j < face.mNumIndices; j++)
			{
				indices.emplace_back(face.mIndices[j]);
			}
		}

		return Mesh(vertices, indices, textures, scene->mName.C_Str());
	}

	std::vector<Ref<Texture2D>> Model::LoadMaterialTextures(aiMaterial* material, int textureType, const std::string& typeName)
	{
		std::vector<Ref<Texture2D>> textures;
		aiTextureType type = (aiTextureType)textureType;

		for (uint32_t i = 0; i < material->GetTextureCount(type); i++)
		{
			aiString str;
			material->GetTexture(type, i, &str);
			std::string relativePath = str.C_Str();

			if (relativePath.empty() || relativePath[0] == '*')
			{
				continue;
			}

			auto cachedIt = std::find_if(m_loadedTextures.begin(), m_loadedTextures.end(),
				[&relativePath](const Ref<Texture2D>& texture)
				{
					return texture->m_path == relativePath;
				});

			if (cachedIt != m_loadedTextures.end())
			{
				textures.emplace_back(*cachedIt);
				continue;
			}

			std::filesystem::path texturePath(relativePath);
			std::filesystem::path fullPath = texturePath.is_absolute()
				? texturePath
				: std::filesystem::path(m_directory) / texturePath;
			fullPath = fullPath.lexically_normal();

			if (!std::filesystem::exists(fullPath))
			{
				UG_CORE_WARN("Model texture not found: {0}", fullPath.string());
				continue;
			}

			Ref<Texture2D> meshTexture;
			meshTexture = Texture2D::Create(fullPath.string());
			meshTexture->m_name = typeName;
			meshTexture->m_path = relativePath;

			textures.emplace_back(meshTexture);
			m_loadedTextures.emplace_back(meshTexture);
		}

		return textures;
	}

}
