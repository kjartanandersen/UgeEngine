/**
 * @file OpenGLRendererAPI.h
 * @brief OpenGL implementation of Uge::RendererAPI.
 * @ingroup group_platform
 */

#pragma once

#include "Uge/Renderer/RendererAPI.h"

namespace Uge
{

	/**
	 * @brief Issues OpenGL state changes and draw calls.
	 * @ingroup group_platform
	 *
	 * Init() enables blending, depth testing and, in Debug builds, the OpenGL debug
	 * message callback that routes driver diagnostics into the engine log.
	 *
	 * Reached through Uge::RenderCommand rather than directly.
	 */
	class OpenGLRendererAPI : public RendererAPI
	{

	public:
		virtual void Init() override;

		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		virtual void Clear() override;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCound = 0) override;



	};



}
