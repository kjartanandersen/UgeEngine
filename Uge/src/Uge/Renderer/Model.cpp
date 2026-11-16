#include <ugpch.h>
#include "Model.h"

#include "Uge/Asset/AssetManager.h"
#include "Uge/Renderer/Material.h"
#include "Uge/Renderer/RenderCommand.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>

#include <assimp/Importer.hpp>
#include <assimp/config.h>
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
			// A vec4, as std140 pads a vec3 to 16 bytes anyway.
			glm::vec4 Position;
		};

		struct ModelData
		{
			glm::mat4 ModelTransform;
			int EntityData;
		};

		// Matches LightData in assets/shaders/Model.glsl.
		struct LightData
		{
			glm::vec4 Direction;
			glm::vec4 Radiance;
		};

		// Matches SkyboxData in assets/shaders/Skybox.glsl.
		struct SkyboxData
		{
			glm::mat4 ViewProjection;
			float Intensity;
			float Padding[3];
		};

		// Positions only; the skybox shader samples in the direction of each vertex.
		static Ref<VertexArray> CreateSkyboxCube()
		{
			constexpr float vertices[] =
			{
				-1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,   1.0f,  1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,
				-1.0f, -1.0f,  1.0f,   1.0f, -1.0f,  1.0f,   1.0f,  1.0f,  1.0f,  -1.0f,  1.0f,  1.0f
			};

			constexpr uint32_t indices[] =
			{
				0, 2, 1,  0, 3, 2,
				4, 5, 6,  4, 6, 7,
				0, 7, 3,  0, 4, 7,
				1, 2, 6,  1, 6, 5,
				3, 6, 2,  3, 7, 6,
				0, 1, 5,  0, 5, 4
			};

			Ref<VertexArray> vertexArray = VertexArray::Create();

			Ref<VertexBuffer> vertexBuffer =
				VertexBuffer::Create(const_cast<float*>(vertices), sizeof(vertices));
			vertexBuffer->SetLayout({ { ShaderDataType::Float3, "a_Position" } });
			vertexArray->AddVertexBuffer(vertexBuffer);

			vertexArray->SetIndexBuffer(
				IndexBuffer::Create(const_cast<uint32_t*>(indices), sizeof(indices) / sizeof(uint32_t)));

			return vertexArray;
		}

		static glm::mat4 AssimpToGlm(const aiMatrix4x4t<float>& matrix)
		{
			glm::mat4 result;
			result[0][0] = matrix.a1; result[1][0] = matrix.a2; result[2][0] = matrix.a3; result[3][0] = matrix.a4;
			result[0][1] = matrix.b1; result[1][1] = matrix.b2; result[2][1] = matrix.b3; result[3][1] = matrix.b4;
			result[0][2] = matrix.c1; result[1][2] = matrix.c2; result[2][2] = matrix.c3; result[3][2] = matrix.c4;
			result[0][3] = matrix.d1; result[1][3] = matrix.d2; result[2][3] = matrix.d3; result[3][3] = matrix.d4;
			return result;
		}

		static Ref<Material> ResolveMaterial(AssetHandle materialHandle)
		{
			if (!materialHandle)
			{
				return nullptr;
			}

			return AssetManager::GetAsset<Material>(materialHandle);
		}

		static CullMode CullModeFor(const Ref<Material>& material)
		{
			return material && material->IsDoubleSided() ? CullMode::None : CullMode::Back;
		}
	}

	Model::SceneData Model::s_sceneData;

	Model::Model(const std::string& path, const MeshAssetMetadata& metadata)
		: m_meshMetadata(metadata)
	{
		LoadModel(path);
	}

	void Model::Shutdown()
	{
		
		// Initialized goes back to false, so the next BeginScene() recreates the resources.
		s_sceneData = SceneData();
	
	}

	void Model::EnsureSceneResources()
	{
		if (s_sceneData.Initialized)
		{
			return;
		}

		s_sceneData.ModelShader = Shader::Create("assets/shaders/Model.glsl");
		// Binding 3, as Renderer2D's smaller camera block is at 0.
		s_sceneData.CameraUniformBuffer = UniformBuffer::Create(sizeof(CameraData), 3);
		s_sceneData.ModelUniformBuffer = UniformBuffer::Create(sizeof(ModelData), 1);

		s_sceneData.LightUniformBuffer = UniformBuffer::Create(sizeof(LightData), 4);

		s_sceneData.SkyboxShader = Shader::Create("assets/shaders/Skybox.glsl");
		s_sceneData.SkyboxUniformBuffer = UniformBuffer::Create(sizeof(SkyboxData), 7);
		s_sceneData.SkyboxCube = CreateSkyboxCube();

		s_sceneData.Initialized = true;
	}

	void Model::ApplyCullMode(CullMode mode)
	{
		// Skips the GL call when the mode is already set. BeginScene() sets the mode directly,
		// as code outside the pass can change it.
		if (s_sceneData.CurrentCullMode == mode)
		{
			return;
		}

		RenderCommand::SetCullMode(mode);
		s_sceneData.CurrentCullMode = mode;

	}

	void Model::SetEnvironment(const Ref<Environment>& environment, float intensity)
	{
		s_sceneData.SceneEnvironment = (environment && environment->IsValid()) ? environment : nullptr;
		s_sceneData.EnvironmentIntensity = intensity;
	}

	void Model::SetDirectionalLight(const glm::vec3& direction, const glm::vec3& radiance)
	{
		// A zero direction can't be normalized, so it turns the light off.
		const float lengthSquared = glm::dot(direction, direction);

		s_sceneData.LightDirection = lengthSquared > 0.0f
			? direction * glm::inversesqrt(lengthSquared)
			: glm::vec3(0.0f);

		s_sceneData.LightRadiance = lengthSquared > 0.0f ? radiance : glm::vec3(0.0f);
	}

	void Model::DrawSkybox(const glm::mat4& viewProjection)
	{
		UG_PROFILE_FUNCTION();

		if (!s_sceneData.Initialized || !s_sceneData.SceneEnvironment)
		{
			return;
		}

		SkyboxData skyboxData{};
		skyboxData.ViewProjection = viewProjection;
		skyboxData.Intensity = s_sceneData.EnvironmentIntensity;
		s_sceneData.SkyboxUniformBuffer->SetData(&skyboxData, sizeof(SkyboxData));

		s_sceneData.SkyboxShader->Bind();
		s_sceneData.SceneEnvironment->Skybox->Bind(0);

		ApplyCullMode(CullMode::None); // The cube is seen from inside.

		// The sky is at the far plane (z == w), which GL_LESS would reject.
		RenderCommand::SetDepthFunc(DepthCompare::LessEqual);
		RenderCommand::DrawIndexed(s_sceneData.SkyboxCube);
		RenderCommand::SetDepthFunc(DepthCompare::Less);

		ApplyCullMode(CullMode::Back);
	}

	void Model::BeginScene(const glm::mat4& viewProjection, const glm::vec3& cameraPosition)
	{
		EnsureSceneResources();

		CameraData cameraData{};
		cameraData.ViewProjection = viewProjection;
		cameraData.Position = glm::vec4(cameraPosition, 1.0f);
		s_sceneData.CameraUniformBuffer->SetData(&cameraData, sizeof(CameraData));

		s_sceneData.CameraPosition = cameraPosition;

		// Culling
		s_sceneData.ViewFrustum = Math::Frustum::FromViewProjection(viewProjection);
		// Set directly rather than through ApplyCullMode(), as code outside the pass may have
		// changed it.
		RenderCommand::SetCullMode(CullMode::Back);
		s_sceneData.CurrentCullMode = CullMode::Back;

		// The shader takes the direction towards the light.
		LightData lightData{};
		lightData.Direction = glm::vec4(-s_sceneData.LightDirection, 0.0f);
		lightData.Radiance = glm::vec4(s_sceneData.LightRadiance, 0.0f);
		s_sceneData.LightUniformBuffer->SetData(&lightData, sizeof(LightData));

		// Slots 6 to 8, above the material textures.
		if (s_sceneData.SceneEnvironment)
		{
			s_sceneData.SceneEnvironment->Irradiance->Bind(6);
			s_sceneData.SceneEnvironment->Prefiltered->Bind(7);
			s_sceneData.SceneEnvironment->BrdfLut->Bind(8);
		}

		Material::SetEnvironmentState(s_sceneData.SceneEnvironment != nullptr,
			s_sceneData.SceneEnvironment ? s_sceneData.SceneEnvironment->GetPrefilteredMipCount() : 1,
			s_sceneData.EnvironmentIntensity);

		// Entries left by a pass that wasn't ended point at submeshes that may be gone.
		s_sceneData.BlendedQueue.clear();
	}

	void Model::EndScene()
	{
		if (!s_sceneData.Initialized)
		{
			return;
		}

		if (!s_sceneData.BlendedQueue.empty())
		{
			// Farthest first.
			std::sort(s_sceneData.BlendedQueue.begin(), s_sceneData.BlendedQueue.end(),
				[](const BlendedDraw& lhs, const BlendedDraw& rhs)
				{
					return lhs.SortKey > rhs.SortKey;
				});

			// Depth test on, depth writes off, so blended surfaces show through each other.
			RenderCommand::SetDepthWrite(false);

			s_sceneData.ModelShader->Bind();
			for (const BlendedDraw& blendedDraw : s_sceneData.BlendedQueue)
			{
				ModelData modelData{};
				modelData.ModelTransform = blendedDraw.Transform;
				modelData.EntityData = blendedDraw.EntityID;
				s_sceneData.ModelUniformBuffer->SetData(&modelData, sizeof(ModelData));

				ApplyCullMode(CullModeFor(ResolveMaterial(blendedDraw.SubMesh->GetMaterial())));
				blendedDraw.SubMesh->Draw(s_sceneData.ModelShader, blendedDraw.EntityID);
			}

			RenderCommand::SetDepthWrite(true);

			s_sceneData.BlendedQueue.clear();
		}

		// What draws after this pass expects culling off.
		ApplyCullMode(CullMode::None);
	}

	void Model::Draw(const glm::mat4& transform, int entityID) const
	{
		if (!s_sceneData.Initialized || m_meshes.empty())
		{
			return;
		}

		const bool cull = s_sceneData.FrustumCullingEnabled;

		if (cull && !s_sceneData.ViewFrustum.Intersects(m_bounds.Transform(transform)))
		{
			RenderStats::Get().MeshCulledCount += (uint32_t)m_meshes.size();
			return;
		}

		s_sceneData.ModelShader->Bind();
		ModelData modelData{};
		modelData.ModelTransform = transform;
		modelData.EntityData = entityID;
		s_sceneData.ModelUniformBuffer->SetData(&modelData, sizeof(ModelData));

		for (const auto& mesh : m_meshes)
		{

			if (cull && !s_sceneData.ViewFrustum.Intersects(mesh.GetBounds().Transform(transform)))
			{
				RenderStats::Get().MeshCulledCount++;
				continue;
			}

			const Ref<Material> material = ResolveMaterial(mesh.GetMaterial());
			if (material && material->GetBlendMode() == AlphaMode::Blend)
			{
				// Sorted in EndScene() with the blended submeshes of every model.
				const glm::vec3 worldCenter = glm::vec3(transform * glm::vec4(mesh.GetCenter(), 1.0f));
				const glm::vec3 toCamera = worldCenter - s_sceneData.CameraPosition;

				s_sceneData.BlendedQueue.push_back({ &mesh, transform, entityID, glm::dot(toCamera, toCamera) });
				continue;
			}

			ApplyCullMode(CullModeFor(material));
			mesh.Draw(s_sceneData.ModelShader, entityID);
		}
	}

	bool Model::BuildCollisionGeometry(AssetHandle mesh, std::vector<glm::vec3>& outVertices, std::vector<uint32_t>& outIndices)
	{
		outVertices.clear();
		outIndices.clear();

		if (!mesh)
		{
			return false;

		}

		Ref<Model> model = AssetManager::GetAsset<Model>(mesh);
		if (!model || !model->IsLoaded())
		{
			return false;

		}

		size_t vertexCount = 0, indexCount = 0;
		for (const Mesh& sub : model->GetMeshes())
		{
			vertexCount += sub.GetVertices().size();
			indexCount += sub.GetIndices().size();
		}
		outVertices.reserve(vertexCount);
		outIndices.reserve(indexCount);

		// Submesh positions are already in the model's space, so they only need an index
		// offset.
		for (const Mesh& sub : model->GetMeshes())
		{
			const uint32_t base = (uint32_t)outVertices.size();

			for (const MeshVertex& v : sub.GetVertices())
			{
				outVertices.push_back(v.Position);

			}
			for (uint32_t i : sub.GetIndices())
			{
				outIndices.push_back(base + i);

			}
		}

		return !outVertices.empty() && outIndices.size() >= 3;
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

		// Drop points and lines: ProcessMesh() builds a triangle index buffer.
		importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);

		// No aiProcess_CalcTangentSpace: MeshVertex has no tangents.
		const aiScene* scene = importer.ReadFile(path,
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

		m_bounds = Math::AABB{};
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
			}
			else
			{
				vertex.TexCoord = { 0.0f, 0.0f };
			}

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

		Mesh retMesh = Mesh(vertices, indices, materialHandle, mesh->mName.C_Str());

		m_bounds.Grow(retMesh.GetBounds());

		return retMesh;
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
