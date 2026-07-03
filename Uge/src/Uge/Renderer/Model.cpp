#include <ugpch.h>
#include "Model.h"

#include "Uge/Asset/AssetManager.h"
#include "Uge/Renderer/Material.h"

#include <cstdlib>
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
			int EntityData;
		};

		static glm::mat4 AssimpToGlm(const aiMatrix4x4t<float>& matrix)
		{
			glm::mat4 result;
			result[0][0] = matrix.a1; result[1][0] = matrix.a2; result[2][0] = matrix.a3; result[3][0] = matrix.a4;
			result[0][1] = matrix.b1; result[1][1] = matrix.b2; result[2][1] = matrix.b3; result[3][1] = matrix.b4;
			result[0][2] = matrix.c1; result[1][2] = matrix.c2; result[2][2] = matrix.c3; result[3][2] = matrix.c4;
			result[0][3] = matrix.d1; result[1][3] = matrix.d2; result[2][3] = matrix.d3; result[3][3] = matrix.d4;
			return result;
		}
	}

	Model::SceneData Model::s_sceneData;

	Model::Model(const std::string& path, const MeshAssetMetadata& metadata)
		: m_meshMetadata(metadata)
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
		modelData.EntityData = entityID;
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
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType);

		if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
		{
			UG_CORE_ERROR("Failed to load model '{0}': {1}", path, importer.GetErrorString());
			return;
		}

		std::filesystem::path modelPath(path);
		m_directory = modelPath.has_parent_path() ? modelPath.parent_path().string() : std::string();

		SetName(scene->mName.C_Str());

		ProcessNode(scene->mRootNode, scene, aiMatrix4x4t<float>());
	}

	void Model::ProcessNode(aiNode* node, const aiScene* scene, const aiMatrix4x4t<float>& parentTransform)
	{
		aiMatrix4x4t<float> nodeTransform = parentTransform * node->mTransformation;

		for (uint32_t i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			m_meshes.emplace_back(ProcessMesh(mesh, scene, nodeTransform));
		}

		for (uint32_t i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene, nodeTransform);
		}
	}

	Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene, const aiMatrix4x4t<float>& transform)
	{
		std::vector<MeshVertex> vertices;
		std::vector<uint32_t> indices;

		const glm::mat4 meshTransform = AssimpToGlm(transform);
		const glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(meshTransform)));

		AssetHandle materialHandle = 0;
		if (mesh->mMaterialIndex < m_meshMetadata.MaterialHandles.size())
		{
			materialHandle = m_meshMetadata.MaterialHandles[mesh->mMaterialIndex];

		}

		bool hasAlbedoMap = false;
		if (materialHandle)
		{
			Ref<Material> material = AssetManager::GetAsset<Material>(materialHandle);
			hasAlbedoMap = material && material->GetAlbedoMap() != 0;
		}

		vertices.reserve(mesh->mNumVertices);
		for (uint32_t i = 0; i < mesh->mNumVertices; i++)
		{
			MeshVertex vertex{};

			glm::vec4 transformedPosition = meshTransform * glm::vec4(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f);
			vertex.Position = glm::vec3(transformedPosition);

			if (mesh->HasNormals())
			{
				glm::vec3 transformedNormal = normalMatrix * glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
				vertex.Normal = glm::normalize(transformedNormal);
			}
			else
			{
				vertex.Normal = { 0.0f, 0.0f, 1.0f };
			}

			if (mesh->mTextureCoords[0])
			{
				vertex.TexCoord.x = mesh->mTextureCoords[0][i].x;
				vertex.TexCoord.y = mesh->mTextureCoords[0][i].y;
				vertex.HasDiffuseMap = hasAlbedoMap ? 1 : 0;
				vertex.TexIndex = hasAlbedoMap ? 0.0f : -1.0f;
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

		return Mesh(vertices, indices, materialHandle, mesh->mName.C_Str());
	}

	std::vector<Ref<Texture2D>> Model::LoadMaterialTextures(aiMaterial* material, const aiScene* scene, int textureType, const std::string& typeName)
	{
		std::vector<Ref<Texture2D>> textures;
		aiTextureType type = (aiTextureType)textureType;

		for (uint32_t i = 0; i < material->GetTextureCount(type); i++)
		{
			aiString str;
			material->GetTexture(type, i, &str);
			std::string relativePath = str.C_Str();

			if (relativePath.empty())
			{
				continue;
			}

			auto cachedIt = std::find_if(m_loadedTextures.begin(), m_loadedTextures.end(),
				[&relativePath](const AssetHandle& texture)
				{
					
					return AssetManager::GetAsset<Texture2D>(texture)->m_path == relativePath;
				});

			if (cachedIt != m_loadedTextures.end())
			{
				// textures.emplace_back(*cachedIt);
				continue;
			}

			Ref<Texture2D> meshTexture;
			if (relativePath[0] == '*')
			{
				if (!scene)
				{
					UG_CORE_WARN("Embedded texture reference '{0}' has no scene context", relativePath);
					continue;
				}

				char* end = nullptr;
				long textureIndex = std::strtol(relativePath.c_str() + 1, &end, 10);
				if (end == relativePath.c_str() + 1 || *end != '\0' || textureIndex < 0 || textureIndex >= (long)scene->mNumTextures)
				{
					UG_CORE_WARN("Invalid embedded texture reference '{0}'", relativePath);
					continue;
				}

				const aiTexture* embeddedTexture = scene->mTextures[textureIndex];
				if (!embeddedTexture || !embeddedTexture->pcData)
				{
					UG_CORE_WARN("Embedded texture '{0}' has no texture data", relativePath);
					continue;
				}

				if (embeddedTexture->mHeight == 0)
				{

					TextureSpecification spec;

					spec.Format = ImageFormat::RGBA8;
					spec.Height = 1;
					spec.Width = embeddedTexture->mWidth;
					std::vector<unsigned char> rgbaPixels;

					rgbaPixels.resize(static_cast<size_t>(embeddedTexture->mWidth) * 4);

					for (uint32_t texelIndex = 0; texelIndex < embeddedTexture->mWidth; texelIndex++)
					{
						const aiTexel& src = embeddedTexture->pcData[texelIndex];
						const size_t dstOffset = static_cast<size_t>(texelIndex) * 4;
						rgbaPixels[dstOffset + 0] = src.r;
						rgbaPixels[dstOffset + 1] = src.g;
						rgbaPixels[dstOffset + 2] = src.b;
						rgbaPixels[dstOffset + 3] = src.a;
					}


					Buffer data;
					data.Data = reinterpret_cast<uint8_t*>(rgbaPixels.data());
					data.Size = static_cast<uint64_t>(rgbaPixels.size());
					
					
					// meshTexture = Texture2D::Create(reinterpret_cast<const unsigned char*>(embeddedTexture->pcData), embeddedTexture->mWidth);
					meshTexture = Texture2D::Create(spec, data);
					meshTexture->SetName(embeddedTexture->mFilename.C_Str());
				}
				else
				{
					const uint32_t width = embeddedTexture->mWidth;
					const uint32_t height = embeddedTexture->mHeight;
					std::vector<unsigned char> rgbaPixels;
					rgbaPixels.resize(static_cast<size_t>(width) * static_cast<size_t>(height) * 4);

					for (uint32_t texelIndex = 0; texelIndex < width * height; texelIndex++)
					{
						const aiTexel& src = embeddedTexture->pcData[texelIndex];
						const size_t dstOffset = static_cast<size_t>(texelIndex) * 4;
						rgbaPixels[dstOffset + 0] = src.r;
						rgbaPixels[dstOffset + 1] = src.g;
						rgbaPixels[dstOffset + 2] = src.b;
						rgbaPixels[dstOffset + 3] = src.a;
					}

					TextureSpecification spec;



					spec.Height = height;
					spec.Width = width;

					Buffer data;
					data.Data = reinterpret_cast<uint8_t*>(rgbaPixels.data());
					data.Size = static_cast<uint64_t>(rgbaPixels.size());

					meshTexture = Texture2D::Create(spec, data);
					meshTexture->SetName(embeddedTexture->mFilename.C_Str());
					// meshTexture->SetData(rgbaPixels.data(), static_cast<uint32_t>(rgbaPixels.size()));
				}
			}
			else
			{
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

				//meshTexture = Texture2D::Create(fullPath.string());
			}

			if (!meshTexture)
			{
				UG_CORE_WARN("Failed to create model texture '{0}'", relativePath);
				continue;
			}

			meshTexture->SetName(typeName);
			meshTexture->m_path = relativePath;

			textures.emplace_back(meshTexture);
			// m_loadedTextures.emplace_back(meshTexture);
		}

		return textures;
	}

}
