#include <ugpch.h>
#include "Shader.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace Uge
{

	Shader* Shader::Create(const std::string& filePath)
	{

		switch (Renderer::GetAPI())
		{

		case RendererAPI::API::None:
			UG_CORE_ASSERT(false, "Renderer API \"None\" not supported!");
			return nullptr;
			break;
		case RendererAPI::API::OpenGL:

			return new OpenGLShader(filePath);

			break;

		}


		UG_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;


		return nullptr;



	}


	Shader* Shader::Create(const std::string& vertexSource, const std::string& fragmentSource)
	{


		switch (Renderer::GetAPI())
		{

		case RendererAPI::API::None:
			UG_CORE_ASSERT(false, "Renderer API \"None\" not supported!");
			return nullptr;
			break;
		case RendererAPI::API::OpenGL:

			return new OpenGLShader(vertexSource, fragmentSource);

			break;

		}


		UG_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;


		return nullptr;
	}

	


}


