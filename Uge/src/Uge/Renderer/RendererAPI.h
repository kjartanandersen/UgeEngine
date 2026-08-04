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
	 * @brief How a fragment's depth is compared against the depth buffer.
	 * @ingroup group_renderer
	 */
	enum class DepthCompare
	{
		Less = 0, ///< Passes when strictly nearer; the default.
		LessEqual = 1 ///< Passes when nearer or equal; required to draw at the far plane.
	};

	/**
	 * @brief How a fragment's colour is combined with what is already in the target.
	 * @ingroup group_renderer
	 */
	enum class BlendMode
	{
		Alpha = 0, ///< Source alpha over destination; the default, used for transparency.
		Additive = 1, ///< Source added to destination, ignoring alpha.
		None = 2 ///< Source replaces destination.
	};

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
		 * @brief Enables or disables depth testing.
		 * @param enabled `true` to test fragments against the depth buffer, `false` to draw
		 *        every fragment regardless of what is already there.
		 *
		 * Fullscreen passes disable it: they cover the whole target deliberately and have no
		 * meaningful depth of their own.
		 *
		 * @note Independent of SetDepthWrite(). Disabling the test also stops depth being
		 * written, whatever the write mask says.
		 */
		virtual void SetDepthTest(bool enabled) = 0;

		/**
		 * @brief Sets the depth comparison function.
		 * @param compare Comparison to use for subsequent draws.
		 *
		 * The skybox needs Uge::DepthCompare::LessEqual: it is emitted at exactly the far
		 * plane, so a strict comparison rejects every one of its fragments.
		 *
		 * @warning Global state. Restore Uge::DepthCompare::Less after the pass that needed
		 * otherwise, or unrelated geometry will start z-fighting with itself.
		 */
		virtual void SetDepthFunc(DepthCompare compare) = 0;

		/**
		 * @brief Sets how subsequent draws blend with the render target.
		 * @param mode Blend equation to use.
		 *
		 * The bloom upsample chain needs Uge::BlendMode::Additive: each level is accumulated
		 * onto the one below it, and alpha blending would replace rather than add.
		 *
		 * @warning Global state. Restore Uge::BlendMode::Alpha afterwards, or transparent
		 * geometry in the next pass stops compositing correctly.
		 */
		virtual void SetBlendMode(BlendMode mode) = 0;

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


