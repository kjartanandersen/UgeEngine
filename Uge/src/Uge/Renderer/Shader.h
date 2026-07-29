/**
 * @file Shader.h
 * @brief Shader programs and the name-keyed shader library.
 * @ingroup group_renderer
 */

#pragma once

#include <string>
#include <unordered_map>
#include "glm/glm.hpp"

namespace Uge
{

	/**
	 * @brief A linked shader program with uniform-setting helpers.
	 * @ingroup group_renderer
	 *
	 * Source is written as GLSL and compiled to SPIR-V at runtime with shaderc, then
	 * cross-compiled back for the driver. The results are cached under
	 * `UgeEditor/assets/cache/shader/`.
	 *
	 * A single-file shader uses `#type vertex` and `#type fragment` markers to separate the
	 * stages, which is why Create() can take one path.
	 *
	 * @warning If edits to a shader appear to have no effect, delete the cache directory —
	 * a stale SPIR-V blob is being reused.
	 *
	 * @note Prefer uniform buffers (Uge::UniformBuffer) for per-frame data. The `Set*`
	 * methods look a uniform up by name on every call.
	 */
	class Shader 
	{
	
	public:
		/** @brief Releases the shader program. */
		virtual ~Shader() = default;

		/** @brief Makes this the active shader program. */
		virtual void Bind() const = 0;
		/** @brief Unbinds the shader program. */
		virtual void Unbind() const = 0;

		/**
		 * @brief Sets an `int` uniform.
		 * @param name Uniform name as written in the shader.
		 * @param value Value to upload.
		 */
		virtual void SetInt(const std::string name, int value) = 0;
		/**
		 * @brief Sets an `int` array uniform.
		 * @param name Uniform name as written in the shader.
		 * @param values Pointer to @p count integers.
		 * @param count Number of elements to upload.
		 *
		 * Used to bind the texture-slot table for the batched 2D renderer.
		 */
		virtual void SetIntArray(const std::string name, int* values, int count) = 0;
		/**
		 * @brief Sets a `float` uniform.
		 * @param name Uniform name as written in the shader.
		 * @param value Value to upload.
		 */
		virtual void SetFloat(const std::string name, float value) = 0;
		/**
		 * @brief Sets a `vec3` uniform.
		 * @param name Uniform name as written in the shader.
		 * @param values Value to upload.
		 */
		virtual void SetFloat3(const std::string name, const glm::vec3& values) = 0;
		/**
		 * @brief Sets a `vec4` uniform.
		 * @param name Uniform name as written in the shader.
		 * @param values Value to upload.
		 */
		virtual void SetFloat4(const std::string name, const glm::vec4& values) = 0;
		/**
		 * @brief Sets a `mat4` uniform.
		 * @param name Uniform name as written in the shader.
		 * @param values Value to upload.
		 */
		virtual void SetMat4(const std::string name, const glm::mat4& values) = 0;

		/**
		 * @brief The shader's name.
		 * @return Name given at creation, or derived from the file stem.
		 */
		virtual const std::string& GetName() const = 0;

		/**
		 * @brief Loads and compiles a shader from a single file.
		 * @param filePath Path to a GLSL file using `#type` markers to split the stages.
		 * @return The backend's shader implementation.
		 */
		static Ref<Shader> Create(const std::string& filePath);
		/**
		 * @brief Compiles a shader from in-memory source strings.
		 * @param name Name to register the shader under.
		 * @param vertexSource GLSL vertex stage source.
		 * @param fragmentSource GLSL fragment stage source.
		 * @return The backend's shader implementation.
		 */
		static Ref<Shader> Create(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource);

	};


	/**
	 * @brief A name-keyed cache of shaders, so each is loaded and compiled once.
	 * @ingroup group_renderer
	 *
	 * @code
	 * ShaderLibrary library;
	 * library.Load("assets/shaders/Renderer2D_Quad.glsl");
	 * auto shader = library.Get("Renderer2D_Quad");
	 * @endcode
	 */
	class ShaderLibrary
	{

	public:
		/**
		 * @brief Registers a shader under an explicit name.
		 * @param name Key to store it under; must not already exist.
		 * @param shader Shader to store.
		 */
		void Add(const std::string& name, const Ref<Shader>& shader);
		/**
		 * @brief Registers a shader under its own Shader::GetName().
		 * @param shader Shader to store; its name must not already exist.
		 */
		void Add(const Ref<Shader>& shader);
		/**
		 * @brief Loads a shader from disk and registers it under its file stem.
		 * @param filePath Path to the shader source.
		 * @return The loaded shader.
		 */
		Ref<Shader> Load(const std::string& filePath);
		/**
		 * @brief Loads a shader from disk and registers it under an explicit name.
		 * @param name Key to store it under.
		 * @param filePath Path to the shader source.
		 * @return The loaded shader.
		 */
		Ref<Shader> Load(const std::string& name, const std::string& filePath);

		/**
		 * @brief Retrieves a registered shader.
		 * @param name Key it was stored under.
		 * @return The shader; asserts in Debug if the name is unknown.
		 */
		Ref<Shader> Get(const std::string& name);

		/**
		 * @brief Tests whether a name is registered.
		 * @param name Key to look for.
		 * @return `true` if a shader is stored under @p name.
		 */
		bool Exists(const std::string& name) const;

	private:
		std::unordered_map<std::string, Ref<Shader>> m_shaders;



	};


}



