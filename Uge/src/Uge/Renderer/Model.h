#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "Uge/Core/Core.h"
#include "Uge/Renderer/Mesh.h"
#include "Uge/Renderer/Shader.h"
#include "Uge/Renderer/UniformBuffer.h"
#include "Uge/Renderer/Texture.h"

struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;

namespace Uge
{

	class Model
	{
	public:
		explicit Model(const std::string& path);

		void Draw(const glm::mat4& transform, int entityID = -1) const;

		const std::string& GetPath() const { return m_path; }
		bool IsLoaded() const { return !m_meshes.empty(); }

		static void BeginScene(const glm::mat4& viewProjection);
		static void EndScene();

	private:
		void LoadModel(const std::string& path);
		void ProcessNode(aiNode* node, const aiScene* scene);
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<Ref<Texture2D>> LoadMaterialTextures(aiMaterial* material, int textureType, const std::string& typeName);

		static void EnsureSceneResources();

	private:
		std::vector<Mesh> m_meshes;
		std::vector<Ref<Texture2D>> m_loadedTextures;

		std::string m_directory;
		std::string m_path;

		struct SceneData
		{
			Ref<Shader> ModelShader;
			Ref<UniformBuffer> CameraUniformBuffer;
			Ref<UniformBuffer> ModelUniformBuffer;
			bool Initialized = false;
		};

		static SceneData s_sceneData;
	};

}
