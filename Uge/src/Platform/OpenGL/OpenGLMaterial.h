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
	 * @brief Binds a shader together with its texture maps and property uniforms.
	 * @ingroup group_platform
	 *
	 * Each texture handle is resolved through Uge::AssetManager at bind time, so a map
	 * that is missing or not yet loaded falls back rather than failing the draw.
	 */
	class OpenGLMaterial : public Material
	{

	public:
		/**
		 * @brief Creates a material for a shader.
		 * @param shader Shader this material parameterizes.
		 */
		OpenGLMaterial(const Ref<Shader>& shader);

		virtual void Bind() override;

		virtual Ref<Shader> GetShader() const override;
		virtual void SetShader(const Ref<Shader>& shader) override;

		/**
		 * @brief Resolves a texture handle and binds it to a sampler slot.
		 * @param handle Texture asset to bind; a handle of `0` binds the white fallback.
		 * @param slot Texture unit to bind to.
		 */
		void BindTexture(AssetHandle handle, uint32_t slot);

	
	};

}