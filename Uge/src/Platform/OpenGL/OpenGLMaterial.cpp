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
		// Imported materials share the Model's shader, which is bound by the caller
		// (Mesh::Draw). A material only owns a shader when one is explicitly assigned.
		if (m_shader)
			m_shader->Bind();

		// Uploaded per material rather than per frame: without it the shader has no way to
		// tell a black plastic from a silver paint, since neither has a base colour map.
		const MaterialUniformData uniformData = BuildUniformData();
		PropertiesUniformBuffer()->SetData(&uniformData, sizeof(uniformData));

		// Slot order matches the sampler bindings in assets/shaders/Model.glsl. A slot whose
		// handle is 0 keeps whatever the previous material left there, which is safe only
		// because the shader gates every read on MaterialData::u_MapFlags.
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


	}
	void OpenGLMaterial::BindTexture(AssetHandle handle, uint32_t slot)
	{
		if (!handle || !AssetManager::IsAssetHandleValid(handle))
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