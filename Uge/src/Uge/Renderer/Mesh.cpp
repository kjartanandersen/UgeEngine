#include <ugpch.h>
#include "Mesh.h"

#include "Uge/Renderer/RenderCommand.h"

namespace Uge
{

	Mesh::Mesh(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices,
		AssetHandle material, const std::string& name)
		: m_vertices(vertices), m_indices(indices), m_material(material), m_name(name)
	{
	
		SetupMesh();
	}

	void Mesh::SetupMesh()
	{
		if (m_vertices.empty() || m_indices.empty())
		{
			return;
		}

		glm::vec3 accumulated(0.0f);
		for (const MeshVertex& vertex : m_vertices)
		{
			accumulated += vertex.Position;
			m_bounds.Grow(vertex.Position);
		}
		m_center = accumulated / static_cast<float>(m_vertices.size());

		m_VAO = VertexArray::Create();
		m_VBO = VertexBuffer::Create(static_cast<uint32_t>(m_vertices.size() * sizeof(MeshVertex)));
		m_VBO->SetData((void*)m_vertices.data(), static_cast<uint32_t>(m_vertices.size() * sizeof(MeshVertex)));

		BufferLayout vbLayout =
		{
			{ ShaderDataType::Float3,	"a_Position" },
			{ ShaderDataType::Float3,	"a_Normal" },
			{ ShaderDataType::Float2,	"a_TexCoord" }
		};

		m_VBO->SetLayout(vbLayout);
		m_VAO->AddVertexBuffer(m_VBO);

		m_IBO = IndexBuffer::Create((uint32_t*)m_indices.data(), static_cast<uint32_t>(m_indices.size()));
		m_VAO->SetIndexBuffer(m_IBO);
	}

	void Mesh::Draw() const
	{
		if (!m_VAO || m_indices.empty())
		{
			return;
		}

		RenderCommand::DrawIndexed(m_VAO);
		RenderStats::Get().MeshDrawCount++;
	}

}
