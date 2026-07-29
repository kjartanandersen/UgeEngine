/**
 * @file OpenGLShader.h
 * @brief OpenGL implementation of Uge::Shader, with SPIR-V compilation and caching.
 * @ingroup group_platform
 */

#pragma once


#include "Uge/Renderer/Shader.h"

#include <glm/glm.hpp>

// TODO: Remove
/** @brief Local alias for the OpenGL enum type, avoiding a GL header include here. */
typedef unsigned int GLenum;
/** @brief Local alias for the OpenGL integer type, avoiding a GL header include here. */
typedef int GLint;

namespace Uge
{

	/**
	 * @brief An OpenGL shader program compiled through SPIR-V.
	 * @ingroup group_platform
	 *
	 * GLSL source is compiled to SPIR-V with shaderc, then cross-compiled back to GLSL for
	 * the driver by SPIRV-Cross. Compiled binaries are cached under
	 * `assets/cache/shader/`, so only the first run pays the compilation cost.
	 *
	 * Uniform locations are cached by name after the first lookup.
	 *
	 * @warning Delete the cache directory if shader edits appear to have no effect.
	 */
	class OpenGLShader : public Shader
	{

	public:
		/**
		 * @brief Loads, compiles and links a shader from a single file.
		 * @param filePath Path to GLSL source using `#type vertex` / `#type fragment` markers.
		 */
		OpenGLShader(const std::string& filePath);
		/**
		 * @brief Compiles and links a shader from in-memory source.
		 * @param name Name to identify the shader by.
		 * @param vertexSource GLSL vertex stage source.
		 * @param fragmentSource GLSL fragment stage source.
		 */
		OpenGLShader(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource);
		/** @brief Deletes the shader program. */
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
		 * @brief Uploads an `int` uniform directly, skipping the virtual interface.
		 * @param name Uniform name as written in the shader.
		 * @param value Value to upload.
		 * @pre The shader must already be bound.
		 */
		void UploadUniformInt(const std::string& name, int value);
		/**
		 * @brief Uploads an `int` array uniform.
		 * @param name Uniform name as written in the shader.
		 * @param values Pointer to @p count integers.
		 * @param count Number of elements.
		 * @pre The shader must already be bound.
		 */
		void UploadUniformIntArray(const std::string& name, int* values, int count);

		/**
		 * @brief Uploads a `float` uniform.
		 * @param name Uniform name as written in the shader.
		 * @param value Value to upload.
		 * @pre The shader must already be bound.
		 */
		void UploadUniformFloat(const std::string& name, float value);
		/**
		 * @brief Uploads a `vec2` uniform.
		 * @param name Uniform name as written in the shader.
		 * @param values Value to upload.
		 * @pre The shader must already be bound.
		 */
		void UploadUniformFloat2(const std::string& name, const glm::vec2& values);
		/**
		 * @brief Uploads a `vec3` uniform.
		 * @param name Uniform name as written in the shader.
		 * @param values Value to upload.
		 * @pre The shader must already be bound.
		 */
		void UploadUniformFloat3(const std::string& name, const glm::vec3& values);
		/**
		 * @brief Uploads a `vec4` uniform.
		 * @param name Uniform name as written in the shader.
		 * @param values Value to upload.
		 * @pre The shader must already be bound.
		 */
		void UploadUniformFloat4(const std::string& name, const glm::vec4& values);
		
		/**
		 * @brief Uploads a `mat3` uniform.
		 * @param name Uniform name as written in the shader.
		 * @param matrix Value to upload.
		 * @pre The shader must already be bound.
		 */
		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
		/**
		 * @brief Uploads a `mat4` uniform.
		 * @param name Uniform name as written in the shader.
		 * @param matrix Value to upload.
		 * @pre The shader must already be bound.
		 */
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);
		
	private:
		/**
		 * @brief Looks up a uniform's location, caching the result.
		 * @param name Uniform name as written in the shader.
		 * @return The location, or `-1` if the shader has no such active uniform.
		 */
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

