#include <ugpch.h>
#include "OpenGLShader.h"

#include <filesystem>
#include <fstream>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#include "Uge/Core/Timer.h"

namespace Uge
{
	namespace Utils
	{
		static GLenum ShaderTypeFromString(const std::string& type)
		{
			if (type == "vertex")
			{
				return GL_VERTEX_SHADER;
			}

			if (type == "fragment" || type == "pixel")
			{
				return GL_FRAGMENT_SHADER;
			}

			UG_CORE_ASSERT(false, "Unknown shader type!");
			return 0;
		}

		static shaderc_shader_kind GLShaderStageToShaderC(GLenum stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER:   return shaderc_glsl_vertex_shader;
				case GL_FRAGMENT_SHADER: return shaderc_glsl_fragment_shader;
			}
			UG_CORE_ASSERT(false);
			return (shaderc_shader_kind)0;
		}

		static const char* GLShaderStageToString(GLenum stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER:   return "GL_VERTEX_SHADER";
				case GL_FRAGMENT_SHADER: return "GL_FRAGMENT_SHADER";
			}
			UG_CORE_ASSERT(false);
			return nullptr;
		}

		static const char* GetCacheDirectory()
		{
			// TODO: make sure the assets directory is valid
			return "assets/cache/shader/opengl";
		}

		static void CreateCacheDirectoryIfNeeded()
		{
			std::string cacheDirectory = GetCacheDirectory();
			if (!std::filesystem::exists(cacheDirectory))
			{
				std::filesystem::create_directories(cacheDirectory);
			}
		}

		// v3 caches GLSL source rather than SPIR-V. The version bump matters: a leftover v2
		// blob read back as text would be handed to the driver as garbage source.
		static const char* GLShaderStageCachedOpenGLFileExtension(uint32_t stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER:    return ".v3.cached_opengl.vert";
				case GL_FRAGMENT_SHADER:  return ".v3.cached_opengl.frag";
			}
			UG_CORE_ASSERT(false);
			return "";
		}

		static const char* GLShaderStageCachedVulkanFileExtension(uint32_t stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER:    return ".v2.cached_vulkan.vert";
				case GL_FRAGMENT_SHADER:  return ".v2.cached_vulkan.frag";
			}
			UG_CORE_ASSERT(false);
			return "";
		}

		static bool IsCacheFileValid(const std::filesystem::path& shaderFilePath, const std::filesystem::path& cachePath)
		{
			if (shaderFilePath.empty() || cachePath.empty())
			{
				return false;
			}

			std::error_code ec;
			if (!std::filesystem::exists(shaderFilePath, ec) || !std::filesystem::exists(cachePath, ec))
			{
				return false;
			}

			const auto cacheTime = std::filesystem::last_write_time(cachePath, ec);
			if (ec)
			{
				return false;
			}

			const auto shaderTime = std::filesystem::last_write_time(shaderFilePath, ec);
			if (ec)
			{
				return false;
			}

			return cacheTime >= shaderTime;
		}

	}

	OpenGLShader::OpenGLShader(const std::string& filePath)
		: m_filePath(filePath)
	{
		UG_PROFILE_FUNCTION();

		Utils::CreateCacheDirectoryIfNeeded();

		std::string source = ReadFile(filePath);
		auto shaderSources = PreProcess(source);
		
		{
			Timer timer;
			CompileOrGetVulkanBinaries(shaderSources);
			CompileOrGetOpenGLSource();
			CreateProgram();
			UG_CORE_WARN("Shader creation took {0} ms", timer.ElapsedMillis());
		}

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
		
		CompileOrGetVulkanBinaries(sources);
		CompileOrGetOpenGLSource();
		CreateProgram();
		

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
			UG_CORE_ASSERT(Utils::ShaderTypeFromString(type), "Invalid shader type specification!");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			pos = source.find(typeToken, nextLinePos);

			shaderSources[Utils::ShaderTypeFromString(type)] = source.substr(nextLinePos, 
				pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));


		}
		return shaderSources;

	}

	void OpenGLShader::CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		UG_PROFILE_FUNCTION();

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		// Keep the intermediate SPIR-V conservative so the bundled SPIRV-Cross can
		// translate fragment control-flow such as discard/kill reliably.
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_0);
		options.SetTargetSpirv(shaderc_spirv_version_1_0);
		const bool optimize = true;
		if (optimize)
		{
			options.SetOptimizationLevel(shaderc_optimization_level_performance);
		}

		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

		auto& shaderData = m_vulkanSPIRV;
		shaderData.clear();

		for (auto&& [stage, source] : shaderSources)
		{
			std::filesystem::path shaderFilePath = m_filePath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::GLShaderStageCachedVulkanFileExtension(stage));

			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
			if (in.is_open() && Utils::IsCacheFileValid(shaderFilePath, cachedPath))
			{
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else
			{
				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_filePath.c_str(), options);
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					UG_CORE_ERROR(module.GetErrorMessage());
					UG_CORE_ASSERT(false);
				}

				shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = shaderData[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}

		for (auto&& [stage, data] : shaderData)
		{
			Reflect(stage, data);
		}

	}

	void OpenGLShader::CompileOrGetOpenGLSource()
	{
		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

		m_openGLSourceCode.clear();
		for (auto&& [stage, spirv] : m_vulkanSPIRV)
		{
			std::filesystem::path shaderFilePath = m_filePath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::GLShaderStageCachedOpenGLFileExtension(stage));

			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
			if (in.is_open() && Utils::IsCacheFileValid(shaderFilePath, cachedPath))
			{
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				std::string& source = m_openGLSourceCode[stage];
				source.resize(static_cast<size_t>(size));
				in.read(source.data(), size);
			}
			else
			{
				try
				{
					spirv_cross::CompilerGLSL glslCompiler(spirv);
					spirv_cross::CompilerGLSL::Options glslOptions = glslCompiler.get_common_options();
					glslOptions.version = 450;
					glslOptions.es = false;
					glslCompiler.set_common_options(glslOptions);

					m_openGLSourceCode[stage] = glslCompiler.compile();
				}
				catch (const spirv_cross::CompilerError& e)
				{
					UG_CORE_ERROR("SPIRV-Cross failed to compile {0} ({1}): {2}", m_filePath, Utils::GLShaderStageToString(stage), e.what());
					UG_CORE_ASSERT(false, "SPIRV-Cross shader compilation failed");
				}

				const std::string& source = m_openGLSourceCode[stage];

				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					out.write(source.data(), static_cast<std::streamsize>(source.size()));
					out.flush();
					out.close();
				}
			}
		}
	}

	namespace
	{
		// Drivers report an info log length of 0 when they have no message. Handing the
		// resulting empty vector's data() to the logger passes a null pointer, which spdlog
		// rejects with "string pointer is null" - swallowing the very error being reported.
		static std::string GetProgramInfoLog(GLuint program)
		{
			GLint length = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
			if (length <= 0)
			{
				return "<no message from driver>";
			}

			std::vector<GLchar> infoLog(length);
			glGetProgramInfoLog(program, length, &length, infoLog.data());
			return std::string(infoLog.data(), length > 0 ? length : 0);
		}

		static std::string GetShaderInfoLog(GLuint shader)
		{
			GLint length = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
			if (length <= 0)
			{
				return "<no message from driver>";
			}

			std::vector<GLchar> infoLog(length);
			glGetShaderInfoLog(shader, length, &length, infoLog.data());
			return std::string(infoLog.data(), length > 0 ? length : 0);
		}
	}

	void OpenGLShader::CreateProgram()
	{
		GLuint program = glCreateProgram();

		std::vector<GLuint> shaderIDs;
		for (auto&& [stage, source] : m_openGLSourceCode)
		{
			// Compiled from GLSL source rather than loaded as a SPIR-V binary. The driver's
			// GLSL front end is far better exercised than its GL_ARB_gl_spirv path, which on
			// Intel rejects valid SPIR-V - spirv-val passes the same modules - and reports an
			// empty info log when it does. @see CompileOrGetOpenGLSource
			GLuint shaderID = shaderIDs.emplace_back(glCreateShader(stage));

			const char* sourceData = source.c_str();
			glShaderSource(shaderID, 1, &sourceData, nullptr);
			glCompileShader(shaderID);

			// A stage that fails to compile produces a link error carrying no message of its
			// own, so report it here where the cause is still known.
			GLint isCompiled;
			glGetShaderiv(shaderID, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				UG_CORE_ERROR("Shader compilation failed ({0}, {1}):\n{2}",
					m_filePath, Utils::GLShaderStageToString(stage), GetShaderInfoLog(shaderID));
			}

			glAttachShader(program, shaderID);
		}

		glLinkProgram(program);

		GLint isLinked;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE)
		{
			UG_CORE_ERROR("Shader linking failed ({0}):\n{1}", m_filePath, GetProgramInfoLog(program));

			glDeleteProgram(program);

			for (auto id : shaderIDs)
			{
				glDeleteShader(id);
			}

			// Deliberately not storing the deleted program. Doing so leaves every uniform
			// block reading as zero, so geometry draws in raw clip space - looking like a
			// transform bug rather than a shader that never linked.
			m_rendererID = 0;
			return;
		}

		for (auto id : shaderIDs)
		{
			glDetachShader(program, id);
			glDeleteShader(id);
		}

		m_rendererID = program;
	}

	void OpenGLShader::Reflect(GLenum stage, const std::vector<uint32_t>& shaderData)
	{
		spirv_cross::Compiler compiler(shaderData);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		UG_CORE_TRACE("OpenGLShader::Reflect - {0} {1}", Utils::GLShaderStageToString(stage), m_filePath);
		UG_CORE_TRACE("    {0} uniform buffers", resources.uniform_buffers.size());
		UG_CORE_TRACE("    {0} resources", resources.sampled_images.size());

		UG_CORE_TRACE("Uniform buffers:");
		for (const auto& resource : resources.uniform_buffers)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			uint32_t bufferSize = compiler.get_declared_struct_size(bufferType);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			int memberCount = bufferType.member_types.size();

			UG_CORE_TRACE("  {0}", resource.name);
			UG_CORE_TRACE("    Size = {0}", bufferSize);
			UG_CORE_TRACE("    Binding = {0}", binding);
			UG_CORE_TRACE("    Members = {0}", memberCount);
		}
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
