#include <ugpch.h>
#include "VertexArray.h"

#include "Uge/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace Uge
{

	Ref<VertexArray> VertexArray::Create()
	{


		switch (Renderer::GetAPI())
		{

			case RendererAPI::API::None:
				UG_CORE_ASSERT(false, "Renderer API \"None\" not supported!");
				return nullptr;
				break;
			case RendererAPI::API::OpenGL:

				return std::make_shared<OpenGLVertexArray>();

				break;

		}


		UG_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}



}


