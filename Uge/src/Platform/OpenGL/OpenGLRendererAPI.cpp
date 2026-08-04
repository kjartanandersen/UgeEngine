#include <ugpch.h>
#include "OpenGLRendererAPI.h"

#include <glad/glad.h>

namespace Uge
{
	void OpenGLRendererAPI::Init()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);

		// Lets a filter kernel straddling a cube face edge read across into the neighbouring
		// face instead of clamping. Without it, every convolved environment map shows a bright
		// seam along all twelve edges - worst on the low-resolution irradiance map, where a
		// single texel covers a large solid angle.
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	}

	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
	{

		glClearColor(color.r, color.g, color.b, color.a);

	}

	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{

		glViewport(x, y, width, height);


	}

	void OpenGLRendererAPI::Clear()
	{

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	}

	void OpenGLRendererAPI::SetDepthWrite(bool enabled)
	{

		glDepthMask(enabled ? GL_TRUE : GL_FALSE);

	}

	void OpenGLRendererAPI::SetDepthTest(bool enabled)
	{

		if (enabled)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);

	}

	void OpenGLRendererAPI::SetDepthFunc(DepthCompare compare)
	{

		glDepthFunc(compare == DepthCompare::LessEqual ? GL_LEQUAL : GL_LESS);

	}

	void OpenGLRendererAPI::SetBlendMode(BlendMode mode)
	{

		switch (mode)
		{
		case BlendMode::Alpha:
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case BlendMode::Additive:
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
			break;
		case BlendMode::None:
			glDisable(GL_BLEND);
			break;
		}

	}

	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		vertexArray->Bind();

		uint32_t count = indexCount == 0 ? vertexArray->GetIndexBuffers()->GetCount() : indexCount;
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);

		// Deliberately does not unbind anything. A `glBindTexture(GL_TEXTURE_2D, 0)` used to sit
		// here, which cleared whichever texture unit happened to be active after every draw in
		// the engine. Any pass that binds a texture once and then issues several draws lost it
		// after the first one - which is exactly what left five of the six faces of every
		// environment cubemap black. Leaving a texture bound is harmless; every draw path binds
		// what it needs before drawing.

	}

}