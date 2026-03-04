#pragma once


#include "Uge/Renderer/Shader.h"

#include <glm/glm.hpp>

// TODO: Remove
typedef unsigned int GLenum;
typedef int GLint;

namespace Uge
{

	class OpenGLShader : public Shader
	{

	public:
		OpenGLShader(const std::string& filePath);
		OpenGLShader(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource);
		virtual ~OpenGLShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetInt(const std::string name, int value) override;
		virtual void SetIntArray(const std::string name, int* values, int count) override;
		virtual void SetFloat(const std::string name, float value) override;
		virtual void SetFloat3(const std::string name, const glm::vec3& values) override;
		virtual void SetFloat4(const std::string name, const glm::vec4& values) override;
		virtual void SetMat4(const std::string name, const glm::mat4& values) override;

		virtual const std::string& GetName() const override { return m_name; };

		void UploadUniformInt(const std::string& name, int value);
		void UploadUniformIntArray(const std::string& name, int* values, int count);

		void UploadUniformFloat(const std::string& name, float value);
		void UploadUniformFloat2(const std::string& name, const glm::vec2& values);
		void UploadUniformFloat3(const std::string& name, const glm::vec3& values);
		void UploadUniformFloat4(const std::string& name, const glm::vec4& values);
		
		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);
		
	private:
		GLint GetUniformLocation(const std::string name) const;
		std::string ReadFile(const std::string& filePath);
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);
		void CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources);
		void CompileOrGetOpenGLBinaries();
		void CreateProgram();
		void Reflect(GLenum stage, const std::vector<uint32_t>& shaderData);

	private:
		uint32_t m_rendererID;
		std::string m_filePath;
		mutable std::unordered_map<std::string, GLint> m_uniformLocCache;

		std::unordered_map<GLenum, std::vector<uint32_t>> m_vulkanSPIRV;
		std::unordered_map<GLenum, std::vector<uint32_t>> m_openGLSPIRV;

		std::unordered_map<GLenum, std::string> m_openGLSourceCode;
		std::string m_name;


	};




}

