#include <ugpch.h>
#include "Mesh.h"

#include "Uge/Renderer/RenderCommand.h"

namespace Uge
{

	Mesh::Mesh(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices,
		const std::vector<Ref<Texture2D>>& textures, const std::string& name)
		: m_vertices(vertices), m_indices(indices), m_textures(textures), m_name(name)
	{
	
		SetupMesh();
	}

	void Mesh::SetupMesh()
	{
		if (m_vertices.empty() || m_indices.empty())
		{
			return;
		}

		m_VAO = VertexArray::Create();
		m_VBO = VertexBuffer::Create(static_cast<uint32_t>(m_vertices.size() * sizeof(MeshVertex)));
		m_VBO->SetData((void*)m_vertices.data(), static_cast<uint32_t>(m_vertices.size() * sizeof(MeshVertex)));

		BufferLayout vbLayout =
		{
			{ ShaderDataType::Float3,	"a_Position" },
			{ ShaderDataType::Float3,	"a_Normal" },
			{ ShaderDataType::Float2,	"a_TexCoord" },
			{ ShaderDataType::Int,		"a_HasDiffuseMap" },
			{ ShaderDataType::Float,	"a_TexIndex" },
			{ ShaderDataType::Int,		"a_EntityID" }
		};

		m_VBO->SetLayout(vbLayout);
		m_VAO->AddVertexBuffer(m_VBO);

		m_IBO = IndexBuffer::Create((uint32_t*)m_indices.data(), static_cast<uint32_t>(m_indices.size()));
		m_VAO->SetIndexBuffer(m_IBO);
	}

	void Mesh::Draw(const Ref<Shader>& shader, int /*entityID*/) const
	{
		if (!m_VAO || m_indices.empty() || !shader)
		{
			return;
		}
		shader->Bind();

		for (uint32_t i = 0; i < m_textures.size(); i++)
		{
			const auto& texture = m_textures[i];
			if (!texture)
			{
				continue;
			}

			if (texture->m_name == "texture_diffuse")
			{
				texture->Bind(0);
				break;
			}
		}

		m_VAO->Bind();
		RenderCommand::DrawIndexed(m_VAO);
		shader->Unbind();
	}

}
