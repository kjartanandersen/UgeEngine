#include <ugpch.h>
#include "Material.h"

#include "Renderer.h"
#include "Texture.h"
#include <Platform/OpenGL/OpenGLMaterial.h>
#include "Uge/Asset/AssetManager.h"

namespace Uge
{
	// The default material's block. Created on first use, once a GL context exists; a file
	// static so Shutdown() can release it before the context goes.
	static Ref<UniformBuffer> s_defaultPropertiesBuffer;

	Ref<Material> Material::Create(const Ref<Shader>& shader)
	{

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    UG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return CreateRef<OpenGLMaterial>(shader);
		}

		UG_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	void Material::BindDefaultProperties()
	{
		if (!s_defaultPropertiesBuffer)
		{
			MaterialUniformData data{};
			data.AlbedoColor = glm::vec4(1.0f);
			data.EmissiveColor = glm::vec4(0.0f);
			data.Roughness = 1.0f;
			data.Metallic = 0.0f;
			data.EmissiveStrength = 0.0f;
			data.AlphaCutoff = 0.5f;
			data.MapFlags = MaterialMap_None;
			data.BlendMode = static_cast<int32_t>(AlphaMode::Opaque);

			s_defaultPropertiesBuffer = UniformBuffer::Create(sizeof(data), 2);
			s_defaultPropertiesBuffer->SetData(&data, sizeof(data));
		}

		s_defaultPropertiesBuffer->Bind();
	}

	void Material::Shutdown()
	{

		s_defaultPropertiesBuffer.reset();

	}

	MaterialUniformData Material::BuildUniformData() const
	{
		MaterialUniformData data{};

		data.AlbedoColor = m_properties.AlbedoColor;
		data.EmissiveColor = glm::vec4(m_properties.EmissiveColor, 0.0f);
		data.Roughness = m_properties.Roughness;
		data.Metallic = m_properties.Metallic;
		data.EmissiveStrength = m_properties.EmissiveStrength;
		data.AlphaCutoff = m_properties.AlphaCutoff;
		data.BlendMode = static_cast<int32_t>(m_properties.BlendMode);

		int32_t mapFlags = MaterialMap_None;
		if (m_textureMaps.Albedo)			mapFlags |= MaterialMap_Albedo;
		if (m_textureMaps.Normal)			mapFlags |= MaterialMap_Normal;
		if (m_textureMaps.Roughness)		mapFlags |= MaterialMap_Roughness;
		if (m_textureMaps.Metallic)			mapFlags |= MaterialMap_Metallic;
		if (m_textureMaps.AmbientOcclusion)	mapFlags |= MaterialMap_AmbientOcclusion;
		if (m_textureMaps.Emissive)			mapFlags |= MaterialMap_Emissive;

		// The same texture in both slots is a packed glTF metallic-roughness map.
		if (m_textureMaps.Roughness && m_textureMaps.Roughness == m_textureMaps.Metallic)
		{
			mapFlags |= MaterialMap_PackedMetallicRoughness;
		}

		data.MapFlags = mapFlags;


		return data;
	}

	void Material::BindProperties()
	{
		if (!m_propertiesBuffer)
		{
			// Binding 2, MaterialData in assets/shaders/Model.glsl.
			m_propertiesBuffer = UniformBuffer::Create(sizeof(MaterialUniformData), 2);
			m_propertiesDirty = true;
		}

		if (m_propertiesDirty)
		{
			const MaterialUniformData data = BuildUniformData();
			m_propertiesBuffer->SetData(&data, sizeof(data));
			m_propertiesDirty = false;
		}

		m_propertiesBuffer->Bind();
	}

	const std::array<Ref<Texture2D>, Material::TextureMapCount>& Material::ResolveTextures()
	{
		if (!m_texturesResolved)
		{
			const AssetHandle handles[TextureMapCount] =
			{
				m_textureMaps.Albedo, m_textureMaps.Normal, m_textureMaps.Roughness,
				m_textureMaps.Metallic, m_textureMaps.AmbientOcclusion, m_textureMaps.Emissive
			};

			for (uint32_t slot = 0; slot < TextureMapCount; slot++)
			{
				m_textures[slot] = handles[slot] ? AssetManager::GetAsset<Texture2D>(handles[slot]) : nullptr;
			}

			m_texturesResolved = true;
		}

		return m_textures;
	}

}