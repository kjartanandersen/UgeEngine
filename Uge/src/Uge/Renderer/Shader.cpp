#include <ugpch.h>
#include "Shader.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace Uge
{

	Ref<Shader> Shader::Create(const std::string& filePath)
	{

		switch (Renderer::GetAPI())
		{

		case RendererAPI::API::None:
			UG_CORE_ASSERT(false, "Renderer API \"None\" not supported!");
			return nullptr;
			break;
		case RendererAPI::API::OpenGL:

			return std::make_shared<OpenGLShader>(filePath);

			break;

		}


		UG_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;


		return nullptr;



	}


	Ref<Shader> Shader::Create(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource)
	{


		switch (Renderer::GetAPI())
		{

		case RendererAPI::API::None:
			UG_CORE_ASSERT(false, "Renderer API \"None\" not supported!");
			return nullptr;
			break;
		case RendererAPI::API::OpenGL:
			return std::make_shared<OpenGLShader>(name, vertexSource, fragmentSource);

			break;

		}


		UG_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;


		return nullptr;
	}

	void ShaderLibrary::Add(const std::string& name, const Ref<Shader>& shader)
	{

		UG_CORE_ASSERT(!Exists(name), "Shader already exists!");
		m_shaders[name] = shader;


	}


	void ShaderLibrary::Add(const Ref<Shader>& shader)
	{

		auto& name = shader->GetName();
		Add(name, shader);

	}

	

	Ref<Shader> ShaderLibrary::Load(const std::string& filePath)
	{

		auto shader = Shader::Create(filePath);
		Add(shader);
		return shader;

	}

	Ref<Shader> ShaderLibrary::Load(const std::string& name, const std::string& filePath)
	{

		auto shader = Shader::Create(filePath);
		Add(name, shader);
		return shader;

	}

	Ref<Shader> ShaderLibrary::Get(const std::string& name)
	{
		UG_CORE_ASSERT(Exists(name), "Shader does not exist!");
		return m_shaders[name];

	}

	bool ShaderLibrary::Exists(const std::string& name) const
	{
		return m_shaders.find(name) != m_shaders.end();
	}

}


