#include <ugpch.h>
#include "Texture.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Uge
{





	Ref<Texture2D> Texture2D::Create(const std::string& path)
	{

		switch (Renderer::GetAPI())
		{

		case RendererAPI::API::None:
			UG_CORE_ASSERT(false, "Renderer API \"None\" not supported!");
			return nullptr;
			break;
		case RendererAPI::API::OpenGL:

			return std::make_shared<OpenGLTexture2D>(path);

			break;

		}


		UG_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;


		return nullptr;



	}

}



