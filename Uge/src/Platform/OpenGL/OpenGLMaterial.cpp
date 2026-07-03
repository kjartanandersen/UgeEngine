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

		BindTexture(m_textureMaps.Albedo, 0);
		BindTexture(m_textureMaps.Normal, 1);
		BindTexture(m_textureMaps.Roughness, 2);
		BindTexture(m_textureMaps.Metallic, 3);

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