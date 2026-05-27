#include <ugpch.h>
#include "Texture.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Uge
{

	Ref<Texture2D> Texture2D::Create(const TextureSpecification& specification, Buffer data)
	{


		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    UG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return CreateRef<OpenGLTexture2D>(specification, data);
		}

		UG_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}



