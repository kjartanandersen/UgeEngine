#include <ugpch.h>
#include "OpenGLMaterial.h"

#include "Uge/Asset/AssetManager.h"
#include <Uge/Renderer/Texture.h>

namespace Uge
{
	OpenGLMaterial::OpenGLMaterial(const Ref<Shader>& shader)
	{

		m_shader = shader;

	}
	void OpenGLMaterial::Bind()
	{
		// Imported materials have no shader; the caller binds Model's.
		if (m_shader)
		{
			m_shader->Bind();
		}

		const MaterialUniformData uniformData = BuildUniformData();
		PropertiesUniformBuffer()->SetData(&uniformData, sizeof(uniformData));

		// Slots match the samplers in assets/shaders/Model.glsl. An empty slot keeps the last
		// material's texture; the shader reads only the maps u_MapFlags marks.
		BindTexture(m_textureMaps.Albedo, 0);
		BindTexture(m_textureMaps.Normal, 1);
		BindTexture(m_textureMaps.Roughness, 2);
		BindTexture(m_textureMaps.Metallic, 3);
		BindTexture(m_textureMaps.AmbientOcclusion, 4);
		BindTexture(m_textureMaps.Emissive, 5);

	}
	Ref<Shader> OpenGLMaterial::GetShader() const
	{
		return m_shader;
	}
	void OpenGLMaterial::SetShader(const Ref<Shader>& shader)
	{
		m_shader = shader;

	}
	void OpenGLMaterial::BindTexture(AssetHandle handle, uint32_t slot)
	{
		if (!handle)
		{
			return;
		}

		Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(handle);
		if (texture)
		{
			texture->Bind(slot);
		}

	}
}