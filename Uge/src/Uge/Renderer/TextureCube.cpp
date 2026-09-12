#include <ugpch.h>
#include "TextureCube.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLTextureCube.h"

namespace Uge
{

	Ref<TextureCube> TextureCube::Create(const TextureCubeSpecification& specification)
	{

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    UG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return CreateRef<OpenGLTextureCube>(specification);
		}

		UG_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}
