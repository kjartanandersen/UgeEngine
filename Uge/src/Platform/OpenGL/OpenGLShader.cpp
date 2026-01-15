#include <ugpch.h>
#include "OpenGLShader.h"

#include <fstream>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace Uge
{

	static GLenum ShaderTypeFromString(const std::string& type)
	{

		if (type == "vertex")							
			return GL_VERTEX_SHADER;
		if (type == "fragment" || type == "pixel")		
			return GL_FRAGMENT_SHADER;

		UG_CORE_ASSERT(false, "Uknown shader type!");
		return 0;
	}

	OpenGLShader::OpenGLShader(const std::string& filePath)
	{
		UG_PROFILE_FUNCTION();

		std::string source = ReadFile(filePath);
		auto shaderSources = PreProcess(source);
		Compile(shaderSources);

		// Extract name from filepath
		// assets/shaders/Texture.glsl
		auto lastSlash = filePath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filePath.rfind(".");
		auto count = lastDot == std::string::npos ? filePath.size() - lastSlash : lastDot - lastSlash;

		m_name = filePath.substr(lastSlash, count);

	}

	OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource)
		: m_name(name)
	{
		UG_PROFILE_FUNCTION();

		std::unordered_map<GLenum, std::string> sources;
		sources[GL_VERTEX_SHADER] = vertexSource;
		sources[GL_FRAGMENT_SHADER] = fragmentSource;
		Compile(sources);
		

	}

	OpenGLShader::~OpenGLShader()
	{
		UG_PROFILE_FUNCTION();

		glDeleteProgram(m_rendererID);

	}

	std::string OpenGLShader::ReadFile(const std::string& filePath)
	{
		UG_PROFILE_FUNCTION();

		std::string result;
		std::ifstream in(filePath, std::ios::in | std::ios::binary);

		if (in)
		{

			in.seekg(0, std::ios::end);
			result.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&result[0], result.size());
			in.close();


		}
		else
		{
			UG_CORE_ERROR("Could not open file: {0}", filePath);
		}

		return result;
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source)
	{
		UG_PROFILE_FUNCTION();

		std::unordered_map<GLenum, std::string> shaderSources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);
		while (pos != std::string::npos)
		{

			size_t eol = source.find_first_of("\r\n", pos);
			UG_CORE_ASSERT(eol != std::string::npos, "Syntax Error!");
			size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);
			UG_CORE_ASSERT(ShaderTypeFromString(type), "Invalid shader type specification!");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			pos = source.find(typeToken, nextLinePos);

			shaderSources[ShaderTypeFromString(type)] = source.substr(nextLinePos, 
				pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));


		}
		return shaderSources;

	}

	void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		UG_PROFILE_FUNCTION();

		// Now time to link them together into a program.
		// Get a program object.
		GLuint program = glCreateProgram();
		UG_CORE_ASSERT(shaderSources.size() <= 2, "Only 2 shaders are supported for now!");
		std::array<GLenum, 2> glShaderIDs;
		int glShaderIDIdx = 0;

		for (auto& key : shaderSources)
		{

			GLenum shaderType = key.first;
			const std::string& source = key.second;

			// Create an empty vertex shader handle
			GLuint shader = glCreateShader(shaderType);

			// Send the vertex shader source code to GL
			// Note that std::string's .c_str is NULL character terminated.
			const GLchar* sourceCstr = (const GLchar*)source.c_str();
			glShaderSource(shader, 1, &sourceCstr, 0);

			// Compile the vertex shader
			glCompileShader(shader);

			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				// The maxLength includes the NULL character
				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

				// We don't need the shader anymore.
				glDeleteShader(shader);

				UG_CORE_ERROR("{0}", (char*)infoLog.data());
				UG_CORE_ASSERT(false, "Shader compilation Failure!");
				break;


			}

			glAttachShader(program, shader);
			glShaderIDs[glShaderIDIdx++] = shader;

		}

		// Attach our shaders to our program

		// Link our program
		glLinkProgram(program);

		// Note the different functions here: glGetProgram* instead of glGetShader*.
		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			// We don't need the program anymore.
			glDeleteProgram(program);
			// Don't leak shaders either.

			for (auto id : glShaderIDs)
				glDeleteShader(id);



			
			

			UG_CORE_ERROR("{0}", (char*)infoLog.data());
			UG_CORE_ASSERT(false, "Shader Link Failure!");
			return;
		}

		// Always detach shaders after a successful link.
		for (auto id : glShaderIDs)
			glDetachShader(program, id);


		m_rendererID = program;

		

	

	}

	

	void OpenGLShader::Bind() const
	{
		UG_PROFILE_FUNCTION();

		glUseProgram(m_rendererID);

	}

	void OpenGLShader::Unbind() const
	{
		UG_PROFILE_FUNCTION();

		glUseProgram(0);


	}

	void OpenGLShader::SetInt(const std::string name, int value)
	{
		UG_PROFILE_FUNCTION();

		UploadUniformInt(name, value);


	}

	void OpenGLShader::SetIntArray(const std::string name, int* values, int count)
	{


		UG_PROFILE_FUNCTION();

		UploadUniformIntArray(name, values, count);

	}

	void OpenGLShader::SetFloat(const std::string name, float value)
	{

		UG_PROFILE_FUNCTION();

		UploadUniformFloat(name, value);
	}

	void OpenGLShader::SetFloat3(const std::string name, const glm::vec3& values)
	{
		UG_PROFILE_FUNCTION();

		UploadUniformFloat3(name, values);


	}

	void OpenGLShader::SetFloat4(const std::string name, const glm::vec4& values)
	{
		UG_PROFILE_FUNCTION();

		UploadUniformFloat4(name, values);


	}

	void OpenGLShader::SetMat4(const std::string name, const glm::mat4& values)
	{
		UG_PROFILE_FUNCTION();

		UploadUniformMat4(name, values);


	}

	// Uniform Uploads

	
	void OpenGLShader::UploadUniformInt(const std::string& name, int value)
	{

		GLint location = GetUniformLocation(name);
		glUniform1i(location, value);


	}

	void OpenGLShader::UploadUniformIntArray(const std::string& name, int* values, int count)
	{
		GLint location = GetUniformLocation(name);
		glUniform1iv(location, count, values);


	}

	void OpenGLShader::UploadUniformFloat(const std::string& name, float value)
	{

		GLint location = GetUniformLocation(name);
		glUniform1f(location, value);

	}

	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& values)
	{

		GLint location = GetUniformLocation(name);
		glUniform2f(location, values.x, values.y);

	}

	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& values)
	{

		GLint location = GetUniformLocation(name);
		glUniform3f(location, values.x, values.y, values.z);


	}

	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& values)
	{


		GLint location = GetUniformLocation(name);
		glUniform4f(location, values.x, values.y, values.z, values.w);

	}

	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix)
	{

		GLint location = GetUniformLocation(name);
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));


	}

	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{

		GLint location = GetUniformLocation(name);
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));


	} 

	GLint OpenGLShader::GetUniformLocation(const std::string name) const
	{

		if (m_uniformLocCache.find(name) != m_uniformLocCache.end())
			return m_uniformLocCache[name];

		GLint location = glGetUniformLocation(m_rendererID, name.c_str());
		m_uniformLocCache[name] = location;

		return location;
	}


}
