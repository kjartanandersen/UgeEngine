/**
 * @file Mesh.h
 * @brief Vertex formats and the drawable mesh submitted by Uge::Model.
 * @ingroup group_renderer
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "Uge/Asset/Asset.h"

#include "Uge/Core/Core.h"
#include "Uge/Renderer/Shader.h"
#include "Uge/Renderer/Texture.h"
#include "Uge/Renderer/VertexArray.h"
#include "Uge/Renderer/UniformBuffer.h"

namespace Uge
{

	// Batched 2D renderer vertex format.
	/**
	 * @brief Vertex format used by the batched 2D renderer.
	 * @ingroup group_renderer
	 *
	 * #TexIndex selects among the texture slots bound for the current batch, which is what
	 * lets many differently textured quads share one draw call.
	 */
	struct Vertex
	{
		glm::vec3 Position; ///< Position in model space.
		glm::vec4 Color; ///< Vertex colour, RGBA.
		glm::vec2 TexCoord; ///< Texture coordinates.
		float TexIndex; ///< Index of the bound texture slot to sample.
		float TilingFactor; ///< Texture repeat count across the quad.

		// Editor Only
		int EntityID; ///< Owning entity, written to the picking attachment. Editor only.
	};

	/**
	 * @brief Vertex format used by the 3D mesh path.
	 * @ingroup group_renderer
	 *
	 * @note Carries geometry only. Material inputs come from the `MaterialData` uniform
	 * block instead, so changing a material does not mean rebuilding the vertex buffer,
	 * and the picked entity ID comes from the per-draw `ModelData` block.
	 */
	struct MeshVertex
	{
		glm::vec3	Position; ///< Position in model space.
		glm::vec3	Normal; ///< Surface normal in model space.
		glm::vec2	TexCoord; ///< Texture coordinates.
	};

	/*
	struct MeshTexture
	{
		Ref<Texture2D> Texture;
		std::string Type;
		std::string Path;
	};
	*/

	/**
	 * @brief One drawable submesh: geometry plus the handle of its material.
	 * @ingroup group_renderer
	 *
	 * A Uge::Model owns a list of these, one per assimp mesh in the imported file. The
	 * GPU buffers are built once at construction and reused for every draw.
	 *
	 * @note The material is referenced by Uge::AssetHandle, not by pointer, and resolved
	 * through Uge::AssetManager at draw time. @see group_asset
	 */
	class Mesh
	{
	public:
		/** @brief Constructs an empty mesh with no geometry. */
		Mesh() = default;
		/**
		 * @brief Builds a mesh and uploads its geometry to the GPU.
		 * @param vertices Vertex data.
		 * @param indices Triangle indices into @p vertices.
		 * @param material Handle of the material to shade this mesh with.
		 * @param name Optional debug name, usually the name from the source file.
		 */
		Mesh(const std::vector<MeshVertex>& vertices, 
			 const std::vector<uint32_t>& indices, 
			 const AssetHandle material,
			 const std::string& name = ""
		);

		/**
		 * @brief Binds the material and issues the draw call.
		 * @param shader Shader to render with; supplied by Uge::Model.
		 * @param entityID ID written to the picking attachment; `-1` for none.
		 */
		void Draw(const Ref<Shader>& shader, int entityID = -1) const;

		/**
		 * @brief The material used to shade this mesh.
		 * @return The material's asset handle; `0` if none is assigned.
		 */
		AssetHandle GetMaterial() const { return m_material; }
		/**
		 * @brief Assigns a different material.
		 * @param material Handle of the material to use.
		 */
		void SetMaterial(AssetHandle material) { m_material = material; }

		/**
		 * @brief The average of the mesh's vertex positions, in model space.
		 * @return The centroid; the origin for an empty mesh.
		 *
		 * Used as the sort key for blended submeshes. A centroid is a coarse
		 * approximation — it breaks down for large or interpenetrating transparent
		 * surfaces — but it is what makes car windows composite in a sensible order
		 * without a per-fragment technique.
		 */
		const glm::vec3& GetCenter() const { return m_center; }


	private:
		void SetupMesh();

	private:
		std::string m_name;

		std::vector<MeshVertex> m_vertices;
		std::vector<uint32_t> m_indices;
		AssetHandle m_material = 0;
		glm::vec3 m_center = glm::vec3(0.0f);

		Ref<UniformBuffer> m_diffuseMap;

		Ref<VertexArray> m_VAO;
		Ref<VertexBuffer> m_VBO;
		Ref<IndexBuffer> m_IBO;
	};

}
