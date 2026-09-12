#include <ugpch.h>
#include "Texture.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Uge
{

	uint32_t ImageFormatBytesPerPixel(ImageFormat format)
	{
		switch (format)
		{
			case ImageFormat::R8:		return 1;
			case ImageFormat::RGB8:		return 3;
			case ImageFormat::RGBA8:	return 4;
			case ImageFormat::RG16F:	return 2 * 2;
			case ImageFormat::RGBA16F:	return 4 * 2;
			case ImageFormat::RGBA32F:	return 4 * 4;
		}

		UG_CORE_ASSERT(false, "Unknown ImageFormat!");
		return 0;
	}

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



