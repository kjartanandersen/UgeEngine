#include <ugpch.h>
#include "RenderCommand.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Uge
{

	RendererAPI* RenderCommand::m_rendererAPI = new OpenGLRendererAPI;



}