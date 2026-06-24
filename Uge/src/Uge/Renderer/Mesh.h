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
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		float TexIndex;
		float TilingFactor;

		// Editor Only
		int EntityID;
	};

	struct MeshVertex
	{
		glm::vec3	Position;
		glm::vec3	Normal;
		glm::vec2	TexCoord;
		int			HasDiffuseMap;
		float		TexIndex;

		// Editor Only
		int EntityID;
	};

	/*
	struct MeshTexture
	{
		Ref<Texture2D> Texture;
		std::string Type;
		std::string Path;
	};
	*/

	class Mesh
	{
	public:
		Mesh() = default;
		Mesh(const std::vector<MeshVertex>& vertices, 
			 const std::vector<uint32_t>& indices, 
			 const AssetHandle material,
			 const std::string& name = ""
		);

		void Draw(const Ref<Shader>& shader, int entityID = -1) const;

		AssetHandle GetMaterial() const { return m_material; }
		void SetMaterial(AssetHandle material) { m_material = material; }

	private:
		void SetupMesh();

	private:
		std::string m_name;

		std::vector<MeshVertex> m_vertices;
		std::vector<uint32_t> m_indices;
		AssetHandle m_material = 0;

		Ref<UniformBuffer> m_diffuseMap;

		Ref<VertexArray> m_VAO;
		Ref<VertexBuffer> m_VBO;
		Ref<IndexBuffer> m_IBO;
	};

}
