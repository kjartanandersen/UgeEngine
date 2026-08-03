#include <ugpch.h>

#include "MeshImporter.h"


#include "Uge/Project/Project.h"
#include "Uge/Asset/EditorAssetManager.h"
#include "Uge/Renderer/Material.h"

#include <assimp/Importer.hpp>
#include <assimp/config.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <assimp/scene.h>
#include <assimp/texture.h>
#include <assimp/GltfMaterial.h>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iterator>

namespace Uge
{
	namespace
	{
		constexpr uint32_t k_glbMagic = 0x46546C67;		// 'glTF'
		constexpr uint32_t k_glbVersion = 2;
		constexpr uint32_t k_glbChunkJson = 0x4E4F534A;	// 'JSON'
		constexpr uint32_t k_glbChunkBinary = 0x004E4942;	// 'BIN\0'

		struct GltfContents
		{
			std::string Json;
			std::vector<char> Binary;
			bool IsGlb = false;
		};

		static std::string LowercaseExtension(const std::filesystem::path& path)
		{
			std::string extension = path.extension().string();
			std::transform(extension.begin(), extension.end(), extension.begin(),
				[](unsigned char character) { return static_cast<char>(std::tolower(character)); });
			return extension;
		}

		// Pulls a glTF file apart into its JSON document and, for .glb, the binary chunk.
		static bool ReadGltfContents(const std::filesystem::path& path, GltfContents& contents)
		{
			std::ifstream in(path, std::ios::binary);
			if (!in)
				return false;

			contents.IsGlb = LowercaseExtension(path) == ".glb";

			if (!contents.IsGlb)
			{
				contents.Json.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
				return !contents.Json.empty();
			}

			uint32_t header[3] = {};	// magic, version, total length
			in.read(reinterpret_cast<char*>(header), sizeof(header));
			if (!in || header[0] != k_glbMagic)
				return false;

			// Chunks run to the end of the file. The spec allows chunk types we do not know
			// about, which are skipped here - only JSON and BIN carry anything we can use.
			const uint32_t fileLength = header[2];
			uint32_t offset = sizeof(header);
			while (offset + 8 <= fileLength)
			{
				uint32_t chunkHeader[2] = {};	// length, type
				in.read(reinterpret_cast<char*>(chunkHeader), sizeof(chunkHeader));
				if (!in)
					return false;

				const uint32_t chunkLength = chunkHeader[0];
				const uint32_t chunkType = chunkHeader[1];

				if (chunkType == k_glbChunkJson)
				{
					contents.Json.resize(chunkLength);
					in.read(contents.Json.data(), chunkLength);
				}
				else if (chunkType == k_glbChunkBinary)
				{
					contents.Binary.resize(chunkLength);
					in.read(contents.Binary.data(), chunkLength);
				}
				else
				{
					in.seekg(chunkLength, std::ios::cur);
				}

				if (!in)
					return false;

				offset += 8 + chunkLength;
			}

			return !contents.Json.empty();
		}

		// Writes `json` back out in the format `source` came from, reusing its binary chunk.
		static bool WriteGltfContents(const std::filesystem::path& path, const std::string& json, const GltfContents& source)
		{
			std::ofstream out(path, std::ios::binary);
			if (!out)
				return false;

			if (!source.IsGlb)
			{
				out.write(json.data(), static_cast<std::streamsize>(json.size()));
				return out.good();
			}

			// Both chunks are padded to a 4 byte boundary: JSON with spaces, BIN with zeroes.
			const uint32_t jsonPadding = static_cast<uint32_t>((4 - (json.size() % 4)) % 4);
			const uint32_t jsonChunkLength = static_cast<uint32_t>(json.size()) + jsonPadding;

			const bool hasBinary = !source.Binary.empty();
			const uint32_t binaryPadding = hasBinary ? static_cast<uint32_t>((4 - (source.Binary.size() % 4)) % 4) : 0;
			const uint32_t binaryChunkLength = hasBinary ? static_cast<uint32_t>(source.Binary.size()) + binaryPadding : 0;

			uint32_t totalLength = 12 + 8 + jsonChunkLength;
			if (hasBinary)
				totalLength += 8 + binaryChunkLength;

			auto writeUint32 = [&out](uint32_t value)
			{
				out.write(reinterpret_cast<const char*>(&value), sizeof(value));
			};

			writeUint32(k_glbMagic);
			writeUint32(k_glbVersion);
			writeUint32(totalLength);

			writeUint32(jsonChunkLength);
			writeUint32(k_glbChunkJson);
			out.write(json.data(), static_cast<std::streamsize>(json.size()));
			out.write("   ", jsonPadding);

			if (hasBinary)
			{
				const char zeroes[3] = {};

				writeUint32(binaryChunkLength);
				writeUint32(k_glbChunkBinary);
				out.write(source.Binary.data(), static_cast<std::streamsize>(source.Binary.size()));
				out.write(zeroes, binaryPadding);
			}

			return out.good();
		}
	}

	Ref<Model> MeshImporter::ImportMesh(AssetHandle handle, const AssetMetadata& metadata)
	{
		UG_PROFILE_FUNCTION();

		std::filesystem::path modelPath = Project::GetAssetDirectory() / metadata.FilePath;
		auto assetManager = Project::GetActive()->GetEditorAssetManager();

		// A glTF file can hold more than one scene and assimp only ever walks the default
		// one, so the others would import as geometry no node reaches. Split them into a
		// file per scene, register the extra scenes as assets of their own, and import
		// scene 0 under the handle we were asked for.
		std::vector<std::filesystem::path> scenePaths = SplitGltfScenes(modelPath);
		if (!scenePaths.empty())
		{
			for (size_t i = 1; i < scenePaths.size(); i++)
			{
				std::filesystem::path relativeScenePath =
					std::filesystem::relative(scenePaths[i], Project::GetAssetDirectory());
				assetManager->GetOrImportAsset(relativeScenePath, relativeScenePath.stem().string());
			}

			modelPath = scenePaths[0];
		}

		Assimp::Importer importer;

		// The mesh path only knows how to build triangle index buffers, so have
		// aiProcess_SortByPType discard line/point primitives instead of splitting
		// them out into meshes we would then draw as garbage triangles.
		importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);

		// No aiProcess_CalcTangentSpace: MeshVertex carries no tangent/bitangent, so the
		// result is discarded. It is also by far the most expensive step on dense meshes
		// (430s -> 6s on a 5.7M vertex FBX in Debug). Re-add it alongside tangent fields
		// on MeshVertex if normal mapping is ever wired up.
		const aiScene* scene = importer.ReadFile(
			modelPath.string(),
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

		assetManager->SetMeshMetadata(handle, meshMetadata);

		auto model = CreateRef<Model>(modelPath.string(), meshMetadata);
		model->m_handle = handle;
		return model;
	}

	std::vector<std::filesystem::path> MeshImporter::SplitGltfScenes(const std::filesystem::path& modelPath)
	{
		UG_PROFILE_FUNCTION();

		const std::string extension = LowercaseExtension(modelPath);
		if (extension != ".glb" && extension != ".gltf")
			return {};

		GltfContents contents;
		if (!ReadGltfContents(modelPath, contents))
		{
			UG_CORE_WARN("Could not read the glTF contents of '{0}'", modelPath.string());
			return {};
		}

		rapidjson::Document document;
		document.Parse(contents.Json.c_str(), contents.Json.size());
		if (document.HasParseError() || !document.IsObject())
		{
			UG_CORE_WARN("Failed to parse the glTF JSON of '{0}'", modelPath.string());
			return {};
		}

		auto scenesMember = document.FindMember("scenes");
		if (scenesMember == document.MemberEnd() || !scenesMember->value.IsArray())
			return {};

		const rapidjson::SizeType sceneCount = scenesMember->value.Size();
		if (sceneCount < 2)
			return {};	// assimp already reaches everything through the single scene

		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

		// Deep copy the scenes out before touching the array: each output file keeps exactly
		// one of them and the array is rewritten in place below.
		std::vector<rapidjson::Value> scenes;
		scenes.reserve(sceneCount);
		for (rapidjson::SizeType i = 0; i < sceneCount; i++)
			scenes.emplace_back(scenesMember->value[i], allocator);

		std::vector<std::filesystem::path> scenePaths;
		scenePaths.reserve(sceneCount);

		for (rapidjson::SizeType i = 0; i < sceneCount; i++)
		{
			std::filesystem::path scenePath = modelPath.parent_path() /
				(modelPath.stem().string() + ".scene" + std::to_string(i) + extension);
			scenePaths.emplace_back(scenePath);

			if (std::filesystem::exists(scenePath))
				continue;

			// Only "scenes" and "scene" change. Every node, mesh, accessor and bufferView
			// index in the rest of the document stays valid and the binary chunk is copied
			// through untouched, so the nodes the dropped scenes used simply become
			// unreachable - which is exactly what makes assimp ignore them.
			document["scenes"].SetArray();
			document["scenes"].PushBack(scenes[i], allocator);

			// A one element "scenes" array makes 0 the right default, so an absent "scene"
			// member needs no fixing up.
			auto sceneMember = document.FindMember("scene");
			if (sceneMember != document.MemberEnd())
				sceneMember->value.SetInt(0);

			rapidjson::StringBuffer buffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
			document.Accept(writer);

			if (!WriteGltfContents(scenePath, std::string(buffer.GetString(), buffer.GetSize()), contents))
			{
				// Leave the caller to import the original file rather than a half written set.
				UG_CORE_ERROR("Failed to write split glTF scene to '{0}'", scenePath.string());
				return {};
			}

			UG_CORE_INFO("Split scene {0} of '{1}' into '{2}'",
				i, modelPath.filename().string(), scenePath.filename().string());
		}

		return scenePaths;
	}

	MeshAssetMetadata MeshImporter::ImportMaterialData(const std::filesystem::path& modelPath, const aiScene* scene)
	{
		MeshAssetMetadata meshMetadata;
		std::filesystem::path modelDirectory = modelPath.parent_path();

		Ref<EditorAssetManager> assetManager = Project::GetActive()->GetEditorAssetManager();

		for (uint32_t i = 0; i < scene->mNumMaterials; i++)
		{
			aiMaterial* aiMat = scene->mMaterials[i];

			// Imported materials share the Model's shader (bound during Mesh::Draw), so
			// they are created without one of their own.
			Ref<Material> material = Material::Create(nullptr);

			aiString materialName;
			if (aiMat->Get(AI_MATKEY_NAME, materialName) == AI_SUCCESS)
			{
				material->SetName(materialName.C_Str());

			}

			auto assignTextures = [&](int assimpTextureType, MeshTextureType meshTextureType)
			{
				std::vector<MeshMaterialTextureRef> textureRefs =
					ImportMaterialTextures(modelDirectory, modelPath, scene, aiMat, assimpTextureType, meshTextureType);

				for (const auto& textureRef : textureRefs)
				{
					switch (textureRef.Type)
					{
					case MeshTextureType::Albedo:    material->SetAlbedoMap(textureRef.Texture); break;
					case MeshTextureType::Normal:    material->SetNormalMap(textureRef.Texture); break;
					case MeshTextureType::Roughness: material->SetRoughnessMap(textureRef.Texture); break;
					case MeshTextureType::Metallic:  material->SetMetallicMap(textureRef.Texture); break;
					default: break;
					}

					if (std::find(meshMetadata.Dependencies.begin(), meshMetadata.Dependencies.end(), textureRef.Texture)
						== meshMetadata.Dependencies.end())
					{
						meshMetadata.Dependencies.emplace_back(textureRef.Texture);
					}
				}
			};

			assignTextures(aiTextureType_DIFFUSE, MeshTextureType::Albedo);
			assignTextures(aiTextureType_NORMALS, MeshTextureType::Normal);
			assignTextures(aiTextureType_DIFFUSE_ROUGHNESS, MeshTextureType::Roughness);
			assignTextures(aiTextureType_METALNESS, MeshTextureType::Metallic);

			MaterialProperties properties;

			aiColor4D albedoColor;
			if (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, albedoColor) == AI_SUCCESS)
			{
				properties.AlbedoColor = glm::vec4(albedoColor.r, albedoColor.g, albedoColor.b, albedoColor.a);

			}

			ai_real roughnessFactor;
			if (aiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughnessFactor) == AI_SUCCESS)
			{
				properties.Roughness = roughnessFactor;

			}

			ai_real metallicFactor;
			if (aiMat->Get(AI_MATKEY_METALLIC_FACTOR, metallicFactor) == AI_SUCCESS)
			{
				properties.Metallic = metallicFactor;

			}

			ImportAlphaMode(aiMat, properties);

			material->SetProperties(properties);

			AssetHandle materialHandle = assetManager->AddMemoryOnlyAsset(material);
			meshMetadata.MaterialHandles.emplace_back(materialHandle);

			if (std::find(meshMetadata.Dependencies.begin(), meshMetadata.Dependencies.end(), materialHandle)
				== meshMetadata.Dependencies.end())
			{
				meshMetadata.Dependencies.emplace_back(materialHandle);
			}
		}

		return meshMetadata;
	}

	void MeshImporter::ImportAlphaMode(aiMaterial* material, MaterialProperties& properties)
	{
		aiString alphaMode;
		if (material->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode) == AI_SUCCESS)
		{
			const std::string mode = alphaMode.C_Str();

			if (mode == "BLEND")
				properties.BlendMode = AlphaMode::Blend;
			else if (mode == "MASK")
				properties.BlendMode = AlphaMode::Mask;
			else
				properties.BlendMode = AlphaMode::Opaque;
		}
		else
		{
			// Formats other than glTF have no alphaMode, so fall back to the opacity factor.
			// FBX in particular expresses a transparent material only this way.
			ai_real opacity;
			if (material->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS && opacity < 1.0f)
			{
				properties.BlendMode = AlphaMode::Blend;
				properties.AlbedoColor.a = opacity;
			}
		}

		ai_real alphaCutoff;
		if (material->Get(AI_MATKEY_GLTF_ALPHACUTOFF, alphaCutoff) == AI_SUCCESS)
		{
			properties.AlphaCutoff = alphaCutoff;
		}
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

			AssetHandle textureHandle = assetManager->GetOrImportAsset(relativeTexturePath, (relativeTexturePath.string()));

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
