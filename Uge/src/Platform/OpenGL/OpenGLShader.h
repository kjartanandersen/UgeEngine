#pragma once


#include "Uge/Renderer/Shader.h"

#include <glm/glm.hpp>
#include <glad/glad.h>

namespace Uge
{

	class OpenGLShader : public Shader
	{

	public:
		OpenGLShader(const std::string& vertexSource, const std::string& fragmentSource);
		virtual ~OpenGLShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		void UploadUniformInt(const std::string& name, int value);

		void UploadUniformFloat(const std::string& name, float value);
		void UploadUniformFloat2(const std::string& name, const glm::vec2& values);
		void UploadUniformFloat3(const std::string& name, const glm::vec3& values);
		void UploadUniformFloat4(const std::string& name, const glm::vec4& values);
		
		void UploadUniformMat3(const std::string& name, const glm::mat4& matrix);
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);
		
	private:
		GLint GetUniformLocation(const std::string name) const;

	private:
		uint32_t m_rendererID;
		mutable std::unordered_map<std::string, GLint> m_uniformLocCache;


	};




}

