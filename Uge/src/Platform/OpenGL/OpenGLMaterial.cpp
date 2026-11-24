#include <ugpch.h>
#include "OpenGLMaterial.h"

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

		BindProperties();

		// Slots match the samplers in assets/shaders/Model.glsl. An empty slot keeps the last
		// material's texture; the shader reads only the maps u_MapFlags marks.
		const auto& textures = ResolveTextures();
		for (uint32_t slot = 0; slot < TextureMapCount; slot++)
		{
			if (textures[slot])
			{
				textures[slot]->Bind(slot);
			}
		}

	}
	Ref<Shader> OpenGLMaterial::GetShader() const
	{
		return m_shader;
	}
	void OpenGLMaterial::SetShader(const Ref<Shader>& shader)
	{
		m_shader = shader;

	}
}