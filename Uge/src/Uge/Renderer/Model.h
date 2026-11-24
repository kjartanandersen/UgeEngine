/**
 * @file Model.h
 * @brief Imported 3D models, and the pass that draws them.
 * @ingroup group_renderer
 */

#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "Uge/Core/Core.h"

#include "Uge/Renderer/RenderCommand.h"
#include "Uge/Renderer/Environment.h"
#include "Uge/Renderer/Mesh.h"
#include "Uge/Renderer/Shader.h"
#include "Uge/Renderer/UniformBuffer.h"
#include "Uge/Renderer/Texture.h"
#include "Uge/Renderer/VertexArray.h"

#include "Uge/Asset/Asset.h"
#include "Uge/Asset/AssetMetadata.h"

#include "Uge/Math/AABB.h"
#include "Uge/Math/Frustum.h"


struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
template<typename TReal>
class aiMatrix4x4t;

namespace Uge
{
	class Material;

	/**
	 * @brief A model imported with assimp: submeshes and their materials.
	 * @ingroup group_renderer
	 *
	 * Models draw in their own pass, with state separate from Uge::Renderer2D's: set the
	 * environment and light, call BeginScene(), Draw() each model, DrawSkybox(), then
	 * EndScene(), which draws the blended submeshes.
	 */
	class Model : public Asset
	{
	public:
		/**
		 * @brief Builds a model from a scene that assimp has read, and uploads it to the GPU.
		 * Check IsLoaded() afterwards.
		 * @param scene The scene; it must have a root node. Only read during the call.
		 * @param path The file the scene was read from.
		 * @param metadata The material handles, indexed by assimp's material index.
		 */
		Model(const aiScene& scene, const std::string& path, const MeshAssetMetadata& metadata);

		/**
		 * @brief Releases the pass's shaders, buffers and environment. The next BeginScene()
		 * recreates the shaders and buffers.
		 */
		static void Shutdown();

		/**
		 * @brief Draws the submeshes that are in view.
		 * @param transform World transform.
		 * @param entityID The entity, for mouse picking; `-1` for none.
		 * @pre BeginScene() has been called.
		 *
		 * Submeshes with a Uge::AlphaMode::Blend material are queued for EndScene().
		 */
		void Draw(const glm::mat4& transform, int entityID = -1) const;

		/**
		 * @brief The file the model was imported from.
		 * @return The path.
		 */
		const std::string& GetPath() const { return m_path; }

		/**
			@brief  The submeshes.
			@retval The submeshes.
		**/
		const std::vector<Mesh>& GetMeshes() const { return m_meshes; }

		/**
		 * @brief Whether the import produced any submeshes.
		 * @return `true` if it did.
		 */
		bool IsLoaded() const { return !m_meshes.empty(); }

		/**
		 * @brief The asset type.
		 * @return Uge::AssetType::Mesh.
		 */
		static AssetType GetStaticType() { return AssetType::Mesh; }
		/**
		 * @brief The asset type.
		 * @return Uge::AssetType::Mesh.
		 */
		virtual AssetType GetType() const override { return GetStaticType(); }



		/**
		 * @brief Starts the pass: uploads the camera and light, binds the environment and
		 * turns back-face culling on.
		 * @param viewProjection View-projection matrix.
		 * @param cameraPosition Camera position, for sorting blended submeshes.
		 *
		 * Empties the blended queue, in case the last pass wasn't ended.
		 */
		static void BeginScene(const glm::mat4& viewProjection, const glm::vec3& cameraPosition);

		/**
		 * @brief Sets the environment for the next BeginScene(). It stays until changed.
		 * @param environment The environment, or null for a flat ambient term. One that
		 *        isn't valid counts as null.
		 * @param intensity Environment light multiplier.
		 */
		static void SetEnvironment(const Ref<Environment>& environment, float intensity = 1.0f);

		/**
		 * @brief Sets the directional light for the next BeginScene(). It stays until changed.
		 * @param direction The direction the light travels in; need not be normalized. Zero
		 *        turns the light off.
		 * @param radiance Linear colour times intensity. Sunlight is well above 1, since the
		 *        diffuse term is divided by pi.
		 */
		static void SetDirectionalLight(const glm::vec3& direction, const glm::vec3& radiance);

		/**
		 * @brief Draws the environment at the far plane. Does nothing without one.
		 * @param viewProjection View-projection matrix, without translation.
		 *
		 * Call it after the opaque models and before EndScene().
		 */
		static void DrawSkybox(const glm::mat4& viewProjection);
		/**
		 * @brief Ends the pass: draws the blended submeshes back to front, then turns culling
		 * off.
		 */
		static void EndScene();

		/**
		 * @brief Joins all of a model's submeshes into one triangle list.
		 * @param mesh The model's handle.
		 * @param outVertices Cleared, then given the positions in model space.
		 * @param outIndices Cleared, then given the triangle indices.
		 * @return `true` if there is at least one triangle.
		 */
		static bool BuildCollisionGeometry(AssetHandle mesh,
			std::vector<glm::vec3>& outVertices,
			std::vector<uint32_t>& outIndices);

	private:
		void ProcessNode(aiNode* node, const aiScene* scene, const aiMatrix4x4t<float>& parentTransform);
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene, const aiMatrix4x4t<float>& transform);
		std::vector<Ref<Texture2D>> LoadMaterialTextures(aiMaterial* material, const aiScene* scene, int textureType, const std::string& typeName);

		static void EnsureSceneResources();

		static void ApplyCullMode(CullMode mode);
		
		// Sets the cull mode, binds the material (or the default block) and draws the submesh.
		static void DrawSubmesh(const Mesh& mesh, Material* material);

	private:

		std::vector<Mesh> m_meshes;
		Math::AABB m_bounds;
		std::vector<AssetHandle> m_loadedTextures;

		std::string m_directory;
		std::string m_path;

		MeshAssetMetadata m_meshMetadata;

		/**
		 * @brief A blended submesh queued for EndScene(). Valid only until the pass ends.
		 */
		struct BlendedDraw
		{
			const Mesh* SubMesh; ///< The submesh.
			Material* SubMeshMaterial; ///< The submesh's material, or null. Valid as long as SubMesh.
			glm::mat4 Transform; ///< World transform.
			int EntityID; ///< The entity, for mouse picking.
			float SortKey; ///< Squared distance from the camera to the submesh's centre.
		};

		/** @brief The pass's state, shared by all models. */
		struct SceneData
		{
			Ref<Shader> ModelShader; ///< The mesh shader.
			Ref<UniformBuffer> CameraUniformBuffer; ///< Camera, per frame.
			Ref<UniformBuffer> ModelUniformBuffer; ///< Model transform, per draw.
			glm::vec3 CameraPosition = glm::vec3(0.0f); ///< Camera position, for sorting.
			std::vector<BlendedDraw> BlendedQueue; ///< Blended submeshes waiting for EndScene().
			bool Initialized = false; ///< Whether the resources exist.

			Ref<Environment> SceneEnvironment; ///< The environment, or null.
			float EnvironmentIntensity = 1.0f; ///< Environment light multiplier.

			glm::vec3 LightDirection = glm::vec3(0.0f); ///< The direction the light travels in.
			glm::vec3 LightRadiance = glm::vec3(0.0f); ///< Linear radiance; zero for no light.
			Ref<UniformBuffer> LightUniformBuffer; ///< Directional light, per frame.

			Ref<Shader> SkyboxShader; ///< The skybox shader.
			Ref<VertexArray> SkyboxCube; ///< The cube the skybox is drawn on.
			Ref<UniformBuffer> SkyboxUniformBuffer; ///< Skybox camera and intensity, per frame.

			// Cull Data
			Math::Frustum ViewFrustum;                      ///< Set by BeginScene(); accepts everything before.
			bool FrustumCullingEnabled = true;				///< Whether Draw() skips what is out of view.
			CullMode CurrentCullMode = CullMode::None;		///< The face culling mode last set.
		};

		static SceneData s_sceneData;
	};

}
