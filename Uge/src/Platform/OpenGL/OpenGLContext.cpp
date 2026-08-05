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

		// Drivers repeat the same complaint every frame, which would bury everything else.
		// Each message id is logged a few times and then muted for the rest of the run.
		constexpr uint32_t s_maxRepeatsPerMessage = 4;
		std::unordered_map<GLuint, uint32_t> s_messageCounts;

		/**
		 * @brief Routes driver diagnostics into the engine log.
		 *
		 * Registered with `glDebugMessageCallback` in Debug builds. The context is created
		 * with `GL_DEBUG_OUTPUT_SYNCHRONOUS`, so the break on a high-severity message lands
		 * on the call stack that caused it rather than somewhere later in the frame.
		 */
		void APIENTRY OnGLDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity,
			GLsizei length, const GLchar* message, const void* userParam)
		{
			// Unused, but required by the GL callback signature.
			(void)length;
			(void)userParam;

			// High severity is never muted: it breaks into the debugger, so it cannot spam.
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
					// Only break when someone is there to catch it. UG_DEBUGBREAK with no
					// debugger attached terminates the process, and a GL error is not worth
					// killing the editor over.
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
		 * @brief Installs the debug message callback if the driver gave us a debug context.
		 * @return `true` when driver diagnostics are now being logged.
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

			// Everything on by default...
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
			// ...except notification-level chatter, which some drivers emit per buffer upload.
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION,
				0, nullptr, GL_FALSE);

			return true;
		}

		/** @brief Reads a GL string, tolerating a null return from a broken driver. */
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
		// Requested by WindowsWindow via GLFW_OPENGL_DEBUG_CONTEXT before window creation.
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
