#include <ugpch.h>
#include "Buffer.h"

#include "Uge/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLBuffer.h"

namespace Uge
{





	VertexBuffer* VertexBuffer::Create(float* vertices, uint32_t size)
	{

		switch (Renderer::GetAPI())
		{

		case RendererAPI::None:
			UG_CORE_ASSERT(false, "Renderer API \"None\" not supported!");
			return nullptr;
			break;
		case RendererAPI::OpenGL:

			return new OpenGLVertexBuffer(vertices, size);

			break;

		}

		
		UG_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}

	IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t size)
	{

		switch (Renderer::GetAPI())
		{

		case RendererAPI::None:
			UG_CORE_ASSERT(false, "Renderer API \"None\" not supported!");
			return nullptr;
			break;
		case RendererAPI::OpenGL:

			return new OpenGLIndexBuffer(indices, size);

			break;

		}


		UG_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;



		return nullptr;
	}

}