/**
 * @file RenderCommand.h
 * @brief Static entry point for issuing raw draw and state commands.
 * @ingroup group_renderer
 */

#pragma once

#include "RendererAPI.h"
#include "RenderStats.h"


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
		 * @brief Enables or disables depth testing.
		 * @param enabled `true` to test against the depth buffer, `false` to draw regardless.
		 * @see RendererAPI::SetDepthTest
		 */
		inline static void SetDepthTest(bool enabled)
		{
			m_rendererAPI->SetDepthTest(enabled);
		}

		/**
		 * @brief Sets the depth comparison function.
		 * @param compare Comparison to use for subsequent draws.
		 * @see RendererAPI::SetDepthFunc
		 */
		inline static void SetDepthFunc(DepthCompare compare)
		{
			m_rendererAPI->SetDepthFunc(compare);
		}

		/**
		 * @brief Sets how subsequent draws blend with the render target.
		 * @param mode Blend equation to use.
		 * @see RendererAPI::SetBlendMode
		 */
		inline static void SetBlendMode(BlendMode mode)
		{
			m_rendererAPI->SetBlendMode(mode);
		}


		/**
		 * @brief Issues an indexed draw call.
		 * @param vertexArray Geometry to draw, with its index buffer set.
		 * @param indexCount Indices to draw, or `0` for the entire index buffer.
		 *
		 * Every draw path bottoms out here, which is why this is where Uge::RenderStats
		 * accumulates the frame's totals.
		 */
		inline static void DrawIndexed(const Ref<VertexArray> vertexArray, uint32_t indexCount = 0)
		{
			const uint32_t drawn = indexCount != 0
				? indexCount
				: vertexArray->GetIndexBuffers()->GetCount();

			RenderStats& stats = RenderStats::Get();
			stats.DrawCalls++;
			stats.IndexCount += drawn;
			stats.TriangleCount += drawn / 3;

			m_rendererAPI->DrawIndexed(vertexArray, indexCount);
		}


	private:
		static RendererAPI* m_rendererAPI;



	};


}


