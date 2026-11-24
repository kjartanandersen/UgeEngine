/**
 * @file Mesh.h
 * @brief Vertex formats and meshes.
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

#include "Uge/Math/AABB.h"

namespace Uge
{

	/**
	 * @brief A vertex of the batched 2D renderer.
	 * @ingroup group_renderer
	 */
	struct Vertex
	{
		glm::vec3 Position; ///< Position.
		glm::vec4 Color; ///< RGBA colour.
		glm::vec2 TexCoord; ///< Texture coordinates.
		float TexIndex; ///< Which of the batch's texture slots to sample.
		float TilingFactor; ///< How many times the texture repeats.

		// Editor Only
		int EntityID; ///< The entity, for mouse picking.
	};

	/**
	 * @brief A vertex of a 3D mesh.
	 * @ingroup group_renderer
	 */
	struct MeshVertex
	{
		glm::vec3	Position; ///< Position in model space.
		glm::vec3	Normal; ///< Normal in model space.
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
	 * @brief A submesh: GPU geometry and a material handle.
	 * @ingroup group_renderer
	 */
	class Mesh
	{
	public:
		/** @brief An empty mesh. */
		Mesh() = default;
		/**
		 * @brief Builds a mesh and uploads it to the GPU.
		 * @param vertices The vertices.
		 * @param indices Triangle indices into @p vertices.
		 * @param material The material's handle.
		 * @param name Debug name.
		 */
		Mesh(const std::vector<MeshVertex>& vertices, 
			 const std::vector<uint32_t>& indices, 
			 const AssetHandle material,
			 const std::string& name = ""
		);

		/**
		 * @brief Draws the mesh with the shader, material and cull mode that are bound.
		 */
		void Draw() const;

		/**
		 * @brief The material.
		 * @return The material's handle, or `0`.
		 */
		AssetHandle GetMaterial() const { return m_material; }
		/**
		 * @brief Sets the material.
		 * @param material The material's handle.
		 */
		void SetMaterial(AssetHandle material) { m_material = material; }

		/**
		 * @brief The average vertex position, in model space.
		 * @return The centre; the origin for an empty mesh.
		 */
		const glm::vec3& GetCenter() const { return m_center; }

		/**
		 * @brief The vertices, as uploaded.
		 * @return The vertices, in model space.
		 */
		const std::vector<MeshVertex>& GetVertices() const { return m_vertices; }

		/**
		 * @brief The triangle indices.
		 * @return Indices into GetVertices(), three per triangle.
		 */
		const std::vector<uint32_t>& GetIndices() const { return m_indices; }

		/**
		* @brief The box around the vertices.
		* @return The bounds, in model space; invalid for an empty mesh.
		*/
		const Math::AABB& GetBounds() const { return m_bounds; }


	private:
		void SetupMesh();

	private:
		std::string m_name;

		std::vector<MeshVertex> m_vertices;
		std::vector<uint32_t> m_indices;
		AssetHandle m_material = 0;
		glm::vec3 m_center = glm::vec3(0.0f);
		Math::AABB m_bounds;

		Ref<UniformBuffer> m_diffuseMap;

		Ref<VertexArray> m_VAO;
		Ref<VertexBuffer> m_VBO;
		Ref<IndexBuffer> m_IBO;
	};

}
