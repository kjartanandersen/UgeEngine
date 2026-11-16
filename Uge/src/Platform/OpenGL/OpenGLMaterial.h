/**
 * @file OpenGLMaterial.h
 * @brief OpenGL implementation of Uge::Material.
 * @ingroup group_platform
 */

#pragma once

#include "Uge/Renderer/Material.h"
#include <string>

namespace Uge
{

	/**
	 * @brief The OpenGL material.
	 * @ingroup group_platform
	 */
	class OpenGLMaterial : public Material
	{

	public:
		/**
		 * @brief A material with a shader.
		 * @param shader The shader, or null.
		 */
		OpenGLMaterial(const Ref<Shader>& shader);

		virtual void Bind() override;

		virtual Ref<Shader> GetShader() const override;
		virtual void SetShader(const Ref<Shader>& shader) override;

		/**
		 * @brief Binds a texture to a slot. Binds nothing for `0` or a texture that doesn't
		 * resolve.
		 * @param handle The texture's handle.
		 * @param slot Texture unit.
		 */
		void BindTexture(AssetHandle handle, uint32_t slot);

	
	};

}