#include <ugpch.h>
#include "Material.h"

#include "Renderer.h"
#include <Platform/OpenGL/OpenGLMaterial.h>

namespace Uge
{



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

	const Ref<UniformBuffer>& Material::PropertiesUniformBuffer()
	{
		// Created on first Bind() rather than at startup: the graphics context does not
		// exist yet when the first materials are constructed during model import.
		static Ref<UniformBuffer> s_propertiesUniformBuffer =
			UniformBuffer::Create(sizeof(MaterialUniformData), 2);

		return s_propertiesUniformBuffer;
	}

	void Material::BindDefaultProperties()
	{
		MaterialUniformData data{};

		data.AlbedoColor = glm::vec4(1.0f);
		data.Roughness = 1.0f;
		data.Metallic = 0.0f;
		data.EmissiveStrength = 0.0f;
		data.AlphaCutoff = 0.5f;
		data.MapFlags = MaterialMap_None;
		data.BlendMode = static_cast<int32_t>(AlphaMode::Opaque);

		PropertiesUniformBuffer()->SetData(&data, sizeof(data));
	}

	MaterialUniformData Material::BuildUniformData() const
	{
		MaterialUniformData data{};

		data.AlbedoColor = m_properties.AlbedoColor;
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
		data.MapFlags = mapFlags;

		return data;
	}

}