/**
 * @file RendererAPI.h
 * @brief The abstract graphics API and the enumeration identifying which one is active.
 * @ingroup group_renderer
 */

#pragma once

#include "VertexArray.h"

#include <glm/glm.hpp>



namespace Uge
{

	/**
	 * @brief Abstract interface over the underlying graphics API.
	 * @ingroup group_renderer
	 *
	 * The lowest layer of the renderer: raw state and draw calls, with no notion of
	 * cameras, materials or batching. Everything above it goes through Uge::RenderCommand
	 * rather than holding an instance directly.
	 *
	 * GetAPI() also drives every `Create` factory in this module, which is how a call to
	 * Uge::Texture2D::Create ends up returning an Uge::OpenGLTexture2D.
	 *
	 * @see RenderCommand, Uge::OpenGLRendererAPI
	 */
	class RendererAPI
	{

	public:
		/**
		 * @brief Identifies the graphics backend the engine is running on.
		 */
		enum class API
		{
			None = 0, ///< No renderer; headless.
			OpenGL = 1 ///< OpenGL 4.5, the only backend currently implemented.
		};

	public:
		/** @brief Sets up initial device state, such as blending and depth testing. */
		virtual void Init() = 0;

		/**
		 * @brief Sets the colour subsequent Clear() calls write.
		 * @param color RGBA colour, components in `[0, 1]`.
		 */
		virtual void SetClearColor(const glm::vec4& color) = 0;
		/**
		 * @brief Sets the region of the render target that will be drawn into.
		 * @param x Left edge in pixels.
		 * @param y Bottom edge in pixels.
		 * @param width Viewport width in pixels.
		 * @param height Viewport height in pixels.
		 */
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		/** @brief Clears the colour and depth buffers of the bound target. */
		virtual void Clear() = 0;

		/**
		 * @brief Enables or disables writing to the depth buffer.
		 * @param enabled `true` to write depth, `false` to test against it only.
		 *
		 * Turning writes off is what lets blended geometry be drawn back-to-front without
		 * each transparent surface occluding the ones behind it.
		 *
		 * @warning While disabled, Clear() cannot clear the depth buffer either. Restore it
		 * before the end of the pass.
		 */
		virtual void SetDepthWrite(bool enabled) = 0;

		/**
		 * @brief Issues an indexed draw call.
		 * @param vertexArray Geometry to draw; must already have an index buffer set.
		 * @param indexCount Number of indices to draw, or `0` to draw the whole index buffer.
		 *
		 * The partial-draw form is what lets the 2D batcher flush a partly filled buffer.
		 */
		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;

		/**
		 * @brief Returns the graphics API the engine was built against.
		 * @return The active API.
		 * @note Fixed at compile time; it cannot be switched at runtime.
		 */
		inline static API GetAPI() { return m_API; }

	private:
		static API m_API;

	};



}


