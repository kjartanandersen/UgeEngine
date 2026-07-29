/**
 * @file Model.h
 * @brief The 3D mesh draw path: imports a model file and renders its submeshes.
 * @ingroup group_renderer
 */

#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "Uge/Core/Core.h"

#include "Uge/Renderer/Mesh.h"
#include "Uge/Renderer/Shader.h"
#include "Uge/Renderer/UniformBuffer.h"
#include "Uge/Renderer/Texture.h"

#include "Uge/Asset/Asset.h"
#include "Uge/Asset/AssetMetadata.h"


struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
template<typename TReal>
class aiMatrix4x4t;

namespace Uge
{

	/**
	 * @brief An imported 3D model: a set of Uge::Mesh submeshes and their materials.
	 * @ingroup group_renderer
	 *
	 * The real 3D rendering path, and an Uge::Asset in its own right. Loading uses assimp,
	 * so any format it supports works; FBX and glTF/GLB are the ones exercised.
	 *
	 * Unlike the other draw paths, this one owns its scene state as a class-level
	 * `SceneData` — its own shader and camera/model uniform buffers — and so has its own
	 * BeginScene()/EndScene() pair:
	 *
	 * @code
	 * Model::BeginScene(camera.GetViewProjection());
	 * for (auto entity : meshView)
	 *     model->Draw(transform, (int)entity);
	 * Model::EndScene();
	 * @endcode
	 *
	 * @warning That state is shared by all instances and is separate from
	 * Uge::Renderer2D's. Uge::Scene calls this pair separately from
	 * Uge::Renderer2D::BeginScene; do not nest them.
	 *
	 * @note Materials found during import have no backing file, so they are registered as
	 * memory-only assets and are not written to the asset registry. @see group_asset
	 */
	class Model : public Asset
	{
	public:
		/**
		 * @brief Imports a model file and builds its GPU resources.
		 * @param path Path to the model file, relative to the project's asset directory.
		 * @param metadata Previously imported material handles and dependencies, so a reimport
		 *        reuses the same asset handles instead of generating new ones.
		 *
		 * @note Blocking and potentially slow: it parses the file, uploads every mesh and loads
		 * the referenced textures. Check IsLoaded() afterwards.
		 */
		explicit Model(const std::string& path, const MeshAssetMetadata& metadata = {});

		/**
		 * @brief Draws every submesh at the given transform.
		 * @param transform World transform to render at.
		 * @param entityID ID written to the picking attachment; `-1` for none.
		 * @pre BeginScene() must have been called for this frame.
		 */
		void Draw(const glm::mat4& transform, int entityID = -1) const;

		/**
		 * @brief The file this model was imported from.
		 * @return Const reference to the source path.
		 */
		const std::string& GetPath() const { return m_path; }
		/**
		 * @brief Whether the import produced any geometry.
		 * @return `true` if at least one submesh was loaded.
		 */
		bool IsLoaded() const { return !m_meshes.empty(); }

		/**
		 * @brief The asset type this class represents.
		 * @return Uge::AssetType::Mesh.
		 */
		static AssetType GetStaticType() { return AssetType::Mesh; }
		/**
		 * @brief This instance's asset type.
		 * @return Uge::AssetType::Mesh.
		 */
		virtual AssetType GetType() const override { return GetStaticType(); }



		/**
		 * @brief Begins the mesh pass, uploading the camera matrix to the shared uniform buffer.
		 * @param viewProjection Combined view-projection matrix.
		 *
		 * Creates the shared shader and uniform buffers on first use.
		 */
		static void BeginScene(const glm::mat4& viewProjection);
		/** @brief Ends the mesh pass opened by BeginScene(). */
		static void EndScene();

	private:
		void LoadModel(const std::string& path);
		void ProcessNode(aiNode* node, const aiScene* scene, const aiMatrix4x4t<float>& parentTransform);
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene, const aiMatrix4x4t<float>& transform);
		std::vector<Ref<Texture2D>> LoadMaterialTextures(aiMaterial* material, const aiScene* scene, int textureType, const std::string& typeName);

		static void EnsureSceneResources();

	private:

		std::vector<Mesh> m_meshes;
		std::vector<AssetHandle> m_loadedTextures;

		std::string m_directory;
		std::string m_path;

		MeshAssetMetadata m_meshMetadata;

		/** @brief Shader and uniform buffers shared by every model draw. */
		struct SceneData
		{
			Ref<Shader> ModelShader; ///< Shader used for every mesh draw.
			Ref<UniformBuffer> CameraUniformBuffer; ///< Per-frame camera matrices.
			Ref<UniformBuffer> ModelUniformBuffer; ///< Per-draw model transform.
			bool Initialized = false; ///< Whether the shared resources have been created.
		};

		static SceneData s_sceneData;
	};

}
