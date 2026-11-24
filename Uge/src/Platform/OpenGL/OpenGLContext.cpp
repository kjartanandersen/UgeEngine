#include <ugpch.h>
#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Uge
{

	namespace
	{
		/** @brief Decodes `GL_DEBUG_SOURCE_*` for the log message. */
		const char* DebugSourceToString(GLenum source)
		{
			switch (source)
			{
				case GL_DEBUG_SOURCE_API:             return "API";
				case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   return "Window System";
				case GL_DEBUG_SOURCE_SHADER_COMPILER: return "Shader Compiler";
				case GL_DEBUG_SOURCE_THIRD_PARTY:     return "Third Party";
				case GL_DEBUG_SOURCE_APPLICATION:     return "Application";
				default:                              return "Other";
			}
		}

		/** @brief Decodes `GL_DEBUG_TYPE_*` for the log message. */
		const char* DebugTypeToString(GLenum type)
		{
			switch (type)
			{
				case GL_DEBUG_TYPE_ERROR:               return "Error";
				case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "Deprecated Behaviour";
				case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "Undefined Behaviour";
				case GL_DEBUG_TYPE_PORTABILITY:         return "Portability";
				case GL_DEBUG_TYPE_PERFORMANCE:         return "Performance";
				case GL_DEBUG_TYPE_MARKER:              return "Marker";
				case GL_DEBUG_TYPE_PUSH_GROUP:          return "Push Group";
				case GL_DEBUG_TYPE_POP_GROUP:           return "Pop Group";
				default:                                return "Other";
			}
		}

		// A message id is logged this many times, then muted for the rest of the run.
		constexpr uint32_t s_maxRepeatsPerMessage = 4;
		std::unordered_map<GLuint, uint32_t> s_messageCounts;

		/**
		 * @brief Logs a driver debug message. A high-severity one breaks into an attached
		 * debugger, on the call that caused it as output is synchronous.
		 */
		void APIENTRY OnGLDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity,
			GLsizei length, const GLchar* message, const void* userParam)
		{
			// Required by the callback signature.
			(void)length;
			(void)userParam;

			// High severity is never muted.
			if (severity != GL_DEBUG_SEVERITY_HIGH)
			{
				const uint32_t seen = ++s_messageCounts[id];
				if (seen > s_maxRepeatsPerMessage)
				{
					return;
				}

				if (seen == s_maxRepeatsPerMessage)
				{
					UG_CORE_WARN("[OpenGL] Message {0} has repeated {1} times; muting it for the rest of the run.",
						id, s_maxRepeatsPerMessage);
				}
			}

			switch (severity)
			{
				case GL_DEBUG_SEVERITY_HIGH:
					UG_CORE_FATAL("[OpenGL] {0} / {1} ({2}): {3}",
						DebugSourceToString(source), DebugTypeToString(type), id, message);
					// UG_DEBUGBREAK without a debugger ends the process.
					if (IsDebuggerPresent())
					{
						UG_DEBUGBREAK();
					}
					break;
				case GL_DEBUG_SEVERITY_MEDIUM:
					UG_CORE_ERROR("[OpenGL] {0} / {1} ({2}): {3}",
						DebugSourceToString(source), DebugTypeToString(type), id, message);
					break;
				case GL_DEBUG_SEVERITY_LOW:
					UG_CORE_WARN("[OpenGL] {0} / {1} ({2}): {3}",
						DebugSourceToString(source), DebugTypeToString(type), id, message);
					break;
				default:
					UG_CORE_TRACE("[OpenGL] {0} / {1} ({2}): {3}",
						DebugSourceToString(source), DebugTypeToString(type), id, message);
					break;
			}
		}

		/**
		 * @brief Installs the debug callback, if this is a debug context.
		 * @return `true` if it was installed.
		 */
		bool TryEnableDebugOutput()
		{
			GLint flags = 0;
			glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

			if ((flags & GL_CONTEXT_FLAG_DEBUG_BIT) == 0 || glDebugMessageCallback == nullptr)
			{
				return false;
			}

			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(OnGLDebugMessage, nullptr);

			// Everything except notifications.
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION,
				0, nullptr, GL_FALSE);

			return true;
		}

		/** @brief A GL string, or `<unknown>` if the driver returns null. */
		std::string GetGLString(GLenum name)
		{
			const GLubyte* value = glGetString(name);
			return value ? reinterpret_cast<const char*>(value) : "<unknown>";
		}
	}

	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: m_windowHandle(windowHandle)
	{

		UG_CORE_ASSERT(windowHandle, "Window Handle is null!");

	}

	void OpenGLContext::Init()
	{
		UG_PROFILE_FUNCTION();

		glfwMakeContextCurrent(m_windowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
#ifdef UG_ENABLE_ASSERTS
		UG_CORE_ASSERT(status, "Failed to initialize GLAD");
#endif

		GraphicsDeviceInfo info;
		info.Vendor = GetGLString(GL_VENDOR);
		info.Renderer = GetGLString(GL_RENDERER);
		info.Version = GetGLString(GL_VERSION);
		info.ShadingLanguage = GetGLString(GL_SHADING_LANGUAGE_VERSION);

#ifdef UG_DEBUG
		// WindowsWindow asks GLFW for a debug context in Debug builds.
		info.DebugOutputEnabled = TryEnableDebugOutput();
#endif

		GraphicsContext::SetDeviceInfo(info);

		printf("\n");
		UG_CORE_INFO("*********************** OpenGL Info ****************************");
		UG_CORE_INFO("OpenGL Vendor: {0}",   info.Vendor);
		UG_CORE_INFO("OpenGL Renderer: {0}", info.Renderer);
		UG_CORE_INFO("OpenGL Version: {0}",  info.Version);
		UG_CORE_INFO("GLSL Version: {0}",    info.ShadingLanguage);
		UG_CORE_INFO("Debug Output: {0}",    info.DebugOutputEnabled ? "enabled" : "disabled");
		UG_CORE_INFO("");

#ifdef UG_ENABLE_ASSERTS
		int versionMajor;
		int versionMinor;
		glGetIntegerv(GL_MAJOR_VERSION, &versionMajor);
		glGetIntegerv(GL_MINOR_VERSION, &versionMinor);

		UG_CORE_ASSERT(versionMajor > 4 || (versionMajor == 4 && versionMinor >= 5), "Uge requires at least OpenGL Version 4.5");

#endif


	}

	void OpenGLContext::SwapBuffers()
	{

		UG_PROFILE_FUNCTION();

		glfwSwapBuffers(m_windowHandle);


	}

}
