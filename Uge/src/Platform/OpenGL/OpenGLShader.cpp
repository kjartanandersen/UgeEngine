#include <ugpch.h>
#include "OpenGLShader.h"

#include <glm/gtc/type_ptr.hpp>

namespace Uge
{


	OpenGLShader::OpenGLShader(const std::string& vertexSource, const std::string& fragmentSource)
	{


		// Create an empty vertex shader handle
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);


		// Send the vertex shader source code to GL
		// Note that std::string's .c_str is NULL character terminated.
		const GLchar* source = (const GLchar*)vertexSource.c_str();
		glShaderSource(vertexShader, 1, &source, 0);

		// Compile the vertex shader
		glCompileShader(vertexShader);

		GLint isCompiled = 0;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);

			// We don't need the shader anymore.
			glDeleteShader(vertexShader);

			UG_CORE_ERROR("{0}", (char*)infoLog.data());
			UG_CORE_ASSERT(false, "Vertex Shaader compilation Failure!");
			return;


		}

		// Create an empty fragment shader handle
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		// Send the fragment shader source code to GL
		// Note that std::string's .c_str is NULL character terminated.
		source = (const GLchar*)fragmentSource.c_str();
		glShaderSource(fragmentShader, 1, &source, 0);

		// Compile the fragment shader
		glCompileShader(fragmentShader);

		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);

			// We don't need the shader anymore.
			glDeleteShader(fragmentShader);
			// Either of them. Don't leak shaders.
			glDeleteShader(vertexShader);

			UG_CORE_ERROR("{0}", (char*)infoLog.data());
			UG_CORE_ASSERT(false, "Fragment Shaader compilation Failure!");
			return;
		}

		// Vertex and fragment shaders are successfully compiled.
		// Now time to link them together into a program.
		// Get a program object.
		m_rendererID = glCreateProgram();

		// Attach our shaders to our program
		glAttachShader(m_rendererID, vertexShader);
		glAttachShader(m_rendererID, fragmentShader);

		// Link our program
		glLinkProgram(m_rendererID);

		// Note the different functions here: glGetProgram* instead of glGetShader*.
		GLint isLinked = 0;
		glGetProgramiv(m_rendererID, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(m_rendererID, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(m_rendererID, maxLength, &maxLength, &infoLog[0]);

			// We don't need the program anymore.
			glDeleteProgram(m_rendererID);
			// Don't leak shaders either.
			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);

			UG_CORE_ERROR("{0}", (char*)infoLog.data());
			UG_CORE_ASSERT(false, "Shader Link Failure!");
			return;
		}

		// Always detach shaders after a successful link.
		glDetachShader(m_rendererID, vertexShader);
		glDetachShader(m_rendererID, fragmentShader);

	}

	OpenGLShader::~OpenGLShader()
	{

		glDeleteProgram(m_rendererID);

	}

	void OpenGLShader::Bind() const
	{

		glUseProgram(m_rendererID);

	}

	void OpenGLShader::Unbind() const
	{

		glUseProgram(0);


	}

	// Uniform Uploads

	
	void OpenGLShader::UploadUniformInt(const std::string& name, int value)
	{

		GLint location = GetUniformLocation(name);
		glUniform1i(location, value);


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

	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat4& matrix)
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
