/**
 * @file RenderCommand.h
 * @brief Draw and state commands.
 * @ingroup group_renderer
 */

#pragma once

#include "RendererAPI.h"
#include "RenderStats.h"


namespace Uge
{


	/**
	 * @brief Static functions that forward to the Uge::RendererAPI.
	 * @ingroup group_renderer
	 *
	 * Commands run immediately, so call them on the thread that owns the GL context.
	 */
	class RenderCommand
	{

	public :

		/** @brief Sets the initial GPU state. */
		inline static void Init()
		{
			m_rendererAPI->Init();
		}

		/**
		 * @brief Sets the viewport.
		 * @param x Left edge in pixels.
		 * @param y Bottom edge in pixels.
		 * @param width Width in pixels.
		 * @param height Height in pixels.
		 */
		inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{

			m_rendererAPI->SetViewport(x, y, width, height);
		}

		/**
		 * @brief Sets the clear colour.
		 * @param color RGBA colour.
		 */
		inline static void SetClearColor(const glm::vec4& color)
		{
			m_rendererAPI->SetClearColor(color);
		}
		/** @brief Clears the colour and depth buffers. */
		inline static void Clear()
		{
			m_rendererAPI->Clear();
		}

		/**
		 * @brief Turns depth writes on or off.
		 * @param enabled Whether to write depth.
		 */
		inline static void SetDepthWrite(bool enabled)
		{
			m_rendererAPI->SetDepthWrite(enabled);
		}

		/**
		 * @brief Turns the depth test on or off.
		 * @param enabled Whether to test depth.
		 */
		inline static void SetDepthTest(bool enabled)
		{
			m_rendererAPI->SetDepthTest(enabled);
		}

		/**
		 * @brief Sets the depth comparison.
		 * @param compare The comparison.
		 */
		inline static void SetDepthFunc(DepthCompare compare)
		{
			m_rendererAPI->SetDepthFunc(compare);
		}

		/**
		 * @brief Sets the blend mode.
		 * @param mode The mode.
		 */
		inline static void SetBlendMode(BlendMode mode)
		{
			m_rendererAPI->SetBlendMode(mode);
		}

		/**
		* @brief Sets which faces are culled.
		* @param mode The faces to cull.
		*/
		inline static void SetCullMode(CullMode mode)
		{
			m_rendererAPI->SetCullMode(mode);
		}

		/**
		 * @brief Sets the line width.
		 * @param width Width in pixels.
		 */
		inline static void SetLineWidth(float width)
		{
			m_rendererAPI->SetLineWidth(width);
		}


		/**
		 * @brief Draws indexed triangles and adds them to Uge::RenderStats.
		 * @param vertexArray The geometry, with an index buffer.
		 * @param indexCount Indices to draw, or `0` for all of them.
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

		/**
		 * @brief Draws line segments and adds them to Uge::RenderStats.
		 * @param vertexArray The lines, two vertices each.
		 * @param vertexCount Vertices to draw.
		 */
		inline static void DrawLines(const Ref<VertexArray> vertexArray, uint32_t vertexCount = 0)
		{
			const uint32_t drawn = vertexCount != 0
				? vertexCount
				: vertexArray->GetVertexBuffers().size();

			RenderStats& stats = RenderStats::Get();
			stats.LinesDrawCalls++;
			stats.LinesCount += drawn;

			m_rendererAPI->DrawLines(vertexArray, vertexCount);
		}


	private:
		static RendererAPI* m_rendererAPI;



	};


}


