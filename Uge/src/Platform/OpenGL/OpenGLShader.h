/**
 * @file OpenGLShader.h
 * @brief The OpenGL shader.
 * @ingroup group_platform
 */

#pragma once


#include "Uge/Renderer/Shader.h"

#include <glm/glm.hpp>

// TODO: Remove
/** @brief GL's enum type, declared here to avoid including GL. */
typedef unsigned int GLenum;
/** @brief GL's int type, declared here to avoid including GL. */
typedef int GLint;

namespace Uge
{

	/**
	 * @brief An OpenGL shader program.
	 * @ingroup group_platform
	 *
	 * GLSL is compiled to Vulkan SPIR-V with shaderc, and SPIRV-Cross turns that back into
	 * GLSL for the driver. Both are cached in `assets/cache/shader/`.
	 */
	class OpenGLShader : public Shader
	{

	public:
		/**
		 * @brief Compiles a shader from a file.
		 * @param filePath A GLSL file whose stages start with `#type vertex` and `#type fragment`.
		 */
		OpenGLShader(const std::string& filePath);
		/**
		 * @brief Compiles a shader from source strings.
		 * @param name The shader's name.
		 * @param vertexSource Vertex stage GLSL.
		 * @param fragmentSource Fragment stage GLSL.
		 */
		OpenGLShader(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource);
		/** @brief Deletes the program. */
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

		/**
		 * @brief Sets an `int` uniform on the bound shader.
		 * @param name Uniform name.
		 * @param value The value.
		 */
		void UploadUniformInt(const std::string& name, int value);
		/**
		 * @brief Sets an `int` array uniform on the bound shader.
		 * @param name Uniform name.
		 * @param values @p count integers.
		 * @param count Number of elements.
		 */
		void UploadUniformIntArray(const std::string& name, int* values, int count);

		/**
		 * @brief Sets a `float` uniform on the bound shader.
		 * @param name Uniform name.
		 * @param value The value.
		 */
		void UploadUniformFloat(const std::string& name, float value);
		/**
		 * @brief Sets a `vec2` uniform on the bound shader.
		 * @param name Uniform name.
		 * @param values The value.
		 */
		void UploadUniformFloat2(const std::string& name, const glm::vec2& values);
		/**
		 * @brief Sets a `vec3` uniform on the bound shader.
		 * @param name Uniform name.
		 * @param values The value.
		 */
		void UploadUniformFloat3(const std::string& name, const glm::vec3& values);
		/**
		 * @brief Sets a `vec4` uniform on the bound shader.
		 * @param name Uniform name.
		 * @param values The value.
		 */
		void UploadUniformFloat4(const std::string& name, const glm::vec4& values);
		
		/**
		 * @brief Sets a `mat3` uniform on the bound shader.
		 * @param name Uniform name.
		 * @param matrix The value.
		 */
		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
		/**
		 * @brief Sets a `mat4` uniform on the bound shader.
		 * @param name Uniform name.
		 * @param matrix The value.
		 */
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);
		
	private:
		/**
		 * @brief A uniform's location, cached by name.
		 * @param name Uniform name.
		 * @return The location, or `-1` if there is no such active uniform.
		 */
		GLint GetUniformLocation(const std::string name) const;
		std::string ReadFile(const std::string& filePath);
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);
		void CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources);
		// Cross-compiles the Vulkan SPIR-V to GLSL 450 and caches the text.
		void CompileOrGetOpenGLSource();
		void CreateProgram();
		void Reflect(GLenum stage, const std::vector<uint32_t>& shaderData);

	private:
		uint32_t m_rendererID;
		std::string m_filePath;
		mutable std::unordered_map<std::string, GLint> m_uniformLocCache;

		std::unordered_map<GLenum, std::vector<uint32_t>> m_vulkanSPIRV;

		// The GLSL given to the driver, cross-compiled from m_vulkanSPIRV.
		std::unordered_map<GLenum, std::string> m_openGLSourceCode;
		std::string m_name;


	};




}

