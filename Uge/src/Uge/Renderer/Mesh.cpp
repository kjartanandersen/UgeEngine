#include <ugpch.h>
#include "Mesh.h"

#include "Uge/Renderer/RenderCommand.h"
#include "Material.h"
#include "Uge/Asset/AssetManager.h"

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

		if (m_material && AssetManager::IsAssetHandleValid(m_material))
		{
			AssetType materialType = AssetManager::GetAssetType(m_material);

			if (materialType == AssetType::Material)
			{
				if (Ref<Material> material = AssetManager::GetAsset<Material>(m_material))
					material->Bind();
			}
			else if (materialType == AssetType::Texture2D)
			{
				// No Material asset yet (memory-only assets are not supported); bind the
				// albedo texture directly to the diffuse slot the Model shader samples.
				if (Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(m_material))
					texture->Bind(0);
			}
		}

		m_VAO->Bind();
		RenderCommand::DrawIndexed(m_VAO);
		shader->Unbind();
	}

}
