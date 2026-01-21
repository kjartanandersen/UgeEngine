#include <ugpch.h>
#include "Framebuffer.h"

#include "Uge/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"

namespace Uge
{

	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		switch (Renderer::GetAPI())
		{

		case RendererAPI::API::None:
			UG_CORE_ASSERT(false, "Renderer API \"None\" not supported!");
			return nullptr;
			break;
		case RendererAPI::API::OpenGL:

			return std::make_shared<OpenGLFramebuffer>(spec);

			break;

		}

		UG_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;

	}

}

