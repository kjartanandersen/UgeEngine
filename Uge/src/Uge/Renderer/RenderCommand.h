/**
 * @file RenderCommand.h
 * @brief Static entry point for issuing raw draw and state commands.
 * @ingroup group_renderer
 */

#pragma once

#include "RendererAPI.h"


namespace Uge
{


	/**
	 * @brief Thin static facade forwarding to the active Uge::RendererAPI.
	 * @ingroup group_renderer
	 *
	 * Saves every call site from having to resolve the backend instance. All three draw
	 * paths — Uge::Renderer2D, Uge::Renderer3D and Uge::Model — bottom out here.
	 *
	 * @note Calls are issued immediately; there is no command buffer or deferred queue.
	 * They must therefore run on the thread that owns the graphics context.
	 */
	class RenderCommand
	{

	public :

		/** @brief Initializes the backend's device state. @see RendererAPI::Init */
		inline static void Init()
		{
			m_rendererAPI->Init();
		}

		/**
		 * @brief Sets the drawable region of the current render target.
		 * @param x Left edge in pixels.
		 * @param y Bottom edge in pixels.
		 * @param width Viewport width in pixels.
		 * @param height Viewport height in pixels.
		 */
		inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{

			m_rendererAPI->SetViewport(x, y, width, height);
		}

		/**
		 * @brief Sets the clear colour.
		 * @param color RGBA colour, components in `[0, 1]`.
		 */
		inline static void SetClearColor(const glm::vec4& color)
		{
			m_rendererAPI->SetClearColor(color);
		}
		/** @brief Clears the bound render target's colour and depth buffers. */
		inline static void Clear()
		{
			m_rendererAPI->Clear();
		}

		/**
		 * @brief Enables or disables depth buffer writes.
		 * @param enabled `true` to write depth, `false` to test against it only.
		 * @see RendererAPI::SetDepthWrite
		 */
		inline static void SetDepthWrite(bool enabled)
		{
			m_rendererAPI->SetDepthWrite(enabled);
		}


		/**
		 * @brief Issues an indexed draw call.
		 * @param vertexArray Geometry to draw, with its index buffer set.
		 * @param indexCount Indices to draw, or `0` for the entire index buffer.
		 */
		inline static void DrawIndexed(const Ref<VertexArray> vertexArray, uint32_t indexCount = 0)
		{
			m_rendererAPI->DrawIndexed(vertexArray, indexCount);
		}


	private:
		static RendererAPI* m_rendererAPI;



	};


}


