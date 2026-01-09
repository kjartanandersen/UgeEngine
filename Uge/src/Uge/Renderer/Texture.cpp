#include <ugpch.h>
#include "Texture.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Uge
{


	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height)
	{
		switch (Renderer::GetAPI())
		{

		case RendererAPI::API::None:
			UG_CORE_ASSERT(false, "Renderer API \"None\" not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLTexture2D>(width, height);


		}


		UG_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;

	}


	Ref<Texture2D> Texture2D::Create(const std::string& path)
	{

		switch (Renderer::GetAPI())
		{

		case RendererAPI::API::None:
			UG_CORE_ASSERT(false, "Renderer API \"None\" not supported!");
			return nullptr;
			break;
		case RendererAPI::API::OpenGL:

			return CreateRef<OpenGLTexture2D>(path);

			break;

		}


		UG_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;


		return nullptr;



	}

	

}



