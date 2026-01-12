#include <ugpch.h>
#include "Buffer.h"

#include "Uge/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLBuffer.h"

namespace Uge
{





	Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size)
	{


		switch (Renderer::GetAPI())
		{

		case RendererAPI::API::None:
			UG_CORE_ASSERT(false, "Renderer API \"None\" not supported!");
			return nullptr;
			break;
		case RendererAPI::API::OpenGL:

			return std::make_shared<OpenGLVertexBuffer>(vertices, size);

			break;

		}

		
		UG_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t size)
	{

		switch (Renderer::GetAPI())
		{

		case RendererAPI::API::None:
			UG_CORE_ASSERT(false, "Renderer API \"None\" not supported!");
			return nullptr;
			break;
		case RendererAPI::API::OpenGL:

			return std::make_shared<OpenGLIndexBuffer>(indices, size);

			break;

		}


		UG_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;



		return nullptr;
	}

}