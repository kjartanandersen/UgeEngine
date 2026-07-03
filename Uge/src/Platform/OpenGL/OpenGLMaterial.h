#pragma once

#include "Uge/Renderer/Material.h"
#include <string>

namespace Uge
{

	class OpenGLMaterial : public Material
	{

	public:
		OpenGLMaterial(const Ref<Shader>& shader);

		virtual void Bind() override;

		virtual Ref<Shader> GetShader() const override;
		virtual void SetShader(const Ref<Shader>& shader) override;

		void BindTexture(AssetHandle handle, uint32_t slot);

	
	};

}