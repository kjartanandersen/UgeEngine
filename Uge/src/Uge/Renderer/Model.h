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

#include "Uge/Renderer/Environment.h"
#include "Uge/Renderer/Mesh.h"
#include "Uge/Renderer/Shader.h"
#include "Uge/Renderer/UniformBuffer.h"
#include "Uge/Renderer/Texture.h"
#include "Uge/Renderer/VertexArray.h"

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
	 * Model::BeginScene(camera.GetViewProjection(), camera.GetPosition());
	 * for (auto entity : meshView)
	 *     model->Draw(transform, (int)entity);
	 * Model::EndScene();
	 * @endcode
	 *
	 * Draw() only issues the opaque submeshes. Blended ones are queued and drawn by
	 * EndScene(), back to front across every model in the pass — calling EndScene() is
	 * therefore not optional if anything in the scene is transparent.
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
		 * @brief Submits every submesh at the given transform.
		 * @param transform World transform to render at.
		 * @param entityID ID written to the picking attachment; `-1` for none.
		 * @pre BeginScene() must have been called for this frame.
		 *
		 * Opaque and alpha-masked submeshes are drawn immediately. Submeshes whose material
		 * is Uge::AlphaMode::Blend are queued for EndScene() instead, so they can be sorted
		 * against the transparent geometry of every other model in the pass.
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
		 * @param cameraPosition Camera position in world space, used to sort blended submeshes.
		 *
		 * Creates the shared shader and uniform buffers on first use, and discards anything
		 * left in the blended queue by a pass that was never ended.
		 */
		static void BeginScene(const glm::mat4& viewProjection, const glm::vec3& cameraPosition);

		/**
		 * @brief Sets the environment every subsequent mesh draw is lit by.
		 * @param environment Built environment maps, or null to light with the flat fallback.
		 * @param intensity Multiplier applied to the skybox; `1.0f` leaves it as captured.
		 *
		 * Must be called before BeginScene(), which binds the maps and tells Uge::Material
		 * whether they are available. Persists until changed, so a scene with no sky light
		 * has to clear it explicitly rather than simply not setting it.
		 */
		static void SetEnvironment(const Ref<Environment>& environment, float intensity = 1.0f);

		/**
		 * @brief Sets the single directional light every subsequent mesh draw is lit by.
		 * @param direction World-space direction the light travels **in**; normalized
		 *        internally. A zero vector disables the light.
		 * @param radiance Linear colour multiplied by intensity. Zero disables the light.
		 *
		 * Must be called before BeginScene(), which uploads it. Persists until changed, so a
		 * scene with no light entity has to clear it rather than simply not setting it.
		 *
		 * @note Radiance, not a `[0, 1]` brightness — the diffuse term is divided by pi, so
		 * plausible sunlight is well above 1.
		 */
		static void SetDirectionalLight(const glm::vec3& direction, const glm::vec3& radiance);

		/**
		 * @brief Draws the environment cubemap behind everything already rendered.
		 * @param viewProjection Combined view-projection matrix; its translation is stripped
		 *        internally so the sky never moves relative to the camera.
		 *
		 * Does nothing when no environment is set. Call after the opaque geometry so the
		 * depth buffer rejects sky fragments the scene already covers, and before
		 * EndScene(), so transparent surfaces can blend over it.
		 */
		static void DrawSkybox(const glm::mat4& viewProjection);
		/**
		 * @brief Ends the mesh pass opened by BeginScene(), flushing the blended queue.
		 *
		 * Draws every queued transparent submesh back to front with depth writes disabled,
		 * then restores them. Skipping this call leaves transparent geometry undrawn.
		 */
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

		/**
		 * @brief One blended submesh deferred to EndScene().
		 *
		 * @warning #SubMesh points into the owning model's submesh vector, so an entry is
		 * only valid until the end of the pass that queued it.
		 */
		struct BlendedDraw
		{
			const Mesh* SubMesh; ///< Submesh to draw.
			glm::mat4 Transform; ///< World transform it was submitted with.
			int EntityID; ///< ID for the picking attachment.
			float SortKey; ///< Squared distance from the camera to the world-space centroid.
		};

		/** @brief Shader, uniform buffers and blended queue shared by every model draw. */
		struct SceneData
		{
			Ref<Shader> ModelShader; ///< Shader used for every mesh draw.
			Ref<UniformBuffer> CameraUniformBuffer; ///< Per-frame camera matrices.
			Ref<UniformBuffer> ModelUniformBuffer; ///< Per-draw model transform.
			glm::vec3 CameraPosition = glm::vec3(0.0f); ///< Sort origin for the blended queue.
			std::vector<BlendedDraw> BlendedQueue; ///< Transparent submeshes awaiting EndScene().
			bool Initialized = false; ///< Whether the shared resources have been created.

			Ref<Environment> SceneEnvironment; ///< Environment lighting the pass; may be null.
			float EnvironmentIntensity = 1.0f; ///< Skybox brightness multiplier.

			glm::vec3 LightDirection = glm::vec3(0.0f); ///< Direction the light travels in.
			glm::vec3 LightRadiance = glm::vec3(0.0f); ///< Linear radiance; zero means no light.
			Ref<UniformBuffer> LightUniformBuffer; ///< Per-frame directional light block.

			Ref<Shader> SkyboxShader; ///< Shader used to draw the environment cubemap.
			Ref<VertexArray> SkyboxCube; ///< Unit cube the skybox is rendered on.
			Ref<UniformBuffer> SkyboxUniformBuffer; ///< Per-frame skybox camera and intensity.
		};

		static SceneData s_sceneData;
	};

}
