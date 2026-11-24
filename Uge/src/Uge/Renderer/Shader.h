/**
 * @file Shader.h
 * @brief Shader programs, and a library of them by name.
 * @ingroup group_renderer
 */

#pragma once

#include <string>
#include <unordered_map>
#include "glm/glm.hpp"

namespace Uge
{

	/**
	 * @brief A shader program.
	 * @ingroup group_renderer
	 *
	 * GLSL is compiled to SPIR-V with shaderc and cached in `assets/cache/shader/`; delete
	 * the cache if a shader edit has no effect. The `Set*` functions look the uniform up by
	 * name on every call.
	 */
	class Shader 
	{
	
	public:
		/** @brief Releases the program. */
		virtual ~Shader() = default;

		/** @brief Binds the program. */
		virtual void Bind() const = 0;
		/** @brief Unbinds the program. */
		virtual void Unbind() const = 0;

		/**
		 * @brief Sets an `int` uniform.
		 * @param name Uniform name.
		 * @param value The value.
		 */
		virtual void SetInt(const std::string name, int value) = 0;
		/**
		 * @brief Sets an `int` array uniform.
		 * @param name Uniform name.
		 * @param values @p count integers.
		 * @param count Number of elements.
		 */
		virtual void SetIntArray(const std::string name, int* values, int count) = 0;
		/**
		 * @brief Sets a `float` uniform.
		 * @param name Uniform name.
		 * @param value The value.
		 */
		virtual void SetFloat(const std::string name, float value) = 0;
		/**
		 * @brief Sets a `vec3` uniform.
		 * @param name Uniform name.
		 * @param values The value.
		 */
		virtual void SetFloat3(const std::string name, const glm::vec3& values) = 0;
		/**
		 * @brief Sets a `vec4` uniform.
		 * @param name Uniform name.
		 * @param values The value.
		 */
		virtual void SetFloat4(const std::string name, const glm::vec4& values) = 0;
		/**
		 * @brief Sets a `mat4` uniform.
		 * @param name Uniform name.
		 * @param values The value.
		 */
		virtual void SetMat4(const std::string name, const glm::mat4& values) = 0;

		/**
		 * @brief The shader's name: the one it was created with, or the file stem.
		 * @return The name.
		 */
		virtual const std::string& GetName() const = 0;

		/**
		 * @brief Compiles a shader from a file.
		 * @param filePath A GLSL file whose stages start with `#type vertex` and `#type fragment`.
		 * @return The shader.
		 */
		static Ref<Shader> Create(const std::string& filePath);
		/**
		 * @brief Compiles a shader from source strings.
		 * @param name The shader's name.
		 * @param vertexSource Vertex stage GLSL.
		 * @param fragmentSource Fragment stage GLSL.
		 * @return The shader.
		 */
		static Ref<Shader> Create(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource);

	};


	/**
	 * @brief Shaders by name.
	 * @ingroup group_renderer
	 */
	class ShaderLibrary
	{

	public:
		/**
		 * @brief Adds a shader. Asserts if the name is taken.
		 * @param name The name.
		 * @param shader The shader.
		 */
		void Add(const std::string& name, const Ref<Shader>& shader);
		/**
		 * @brief Adds a shader under Shader::GetName(). Asserts if the name is taken.
		 * @param shader The shader.
		 */
		void Add(const Ref<Shader>& shader);
		/**
		 * @brief Loads a shader and adds it under its file stem.
		 * @param filePath The shader file.
		 * @return The shader.
		 */
		Ref<Shader> Load(const std::string& filePath);
		/**
		 * @brief Loads a shader and adds it under a name.
		 * @param name The name.
		 * @param filePath The shader file.
		 * @return The shader.
		 */
		Ref<Shader> Load(const std::string& name, const std::string& filePath);

		/**
		 * @brief A shader by name. Asserts if there is none.
		 * @param name The name.
		 * @return The shader.
		 */
		Ref<Shader> Get(const std::string& name);

		/**
		 * @brief Whether a shader has a name.
		 * @param name The name.
		 * @return `true` if one does.
		 */
		bool Exists(const std::string& name) const;

	private:
		std::unordered_map<std::string, Ref<Shader>> m_shaders;



	};


}



