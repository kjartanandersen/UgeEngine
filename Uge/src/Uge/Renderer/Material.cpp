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

}