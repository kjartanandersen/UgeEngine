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

		glm::vec3 accumulated(0.0f);
		for (const MeshVertex& vertex : m_vertices)
		{
			accumulated += vertex.Position;
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

	void Mesh::Draw(const Ref<Shader>& shader, int /*entityID*/) const
	{
		if (!m_VAO || m_indices.empty() || !shader)
		{
			return;
		}
		shader->Bind();

		Ref<Material> material;
		if (m_material && AssetManager::IsAssetHandleValid(m_material) &&
			AssetManager::GetAssetType(m_material) == AssetType::Material)
		{
			material = AssetManager::GetAsset<Material>(m_material);
		}

		if (material)
		{
			material->Bind();
		}
		else
		{
			// The material uniform block survives between draws, so an unmaterialed mesh
			// would otherwise be shaded with whatever bound last.
			Material::BindDefaultProperties();
		}

		m_VAO->Bind();
		RenderCommand::DrawIndexed(m_VAO);
		shader->Unbind();
	}

}
