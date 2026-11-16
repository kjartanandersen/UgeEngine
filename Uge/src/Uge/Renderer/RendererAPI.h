/**
 * @file RendererAPI.h
 * @brief The graphics API interface.
 * @ingroup group_renderer
 */

#pragma once

#include "VertexArray.h"

#include <glm/glm.hpp>



namespace Uge
{

	/**
	 * @brief Depth comparisons.
	 * @ingroup group_renderer
	 */
	enum class DepthCompare
	{
		Less = 0, ///< The default.
		LessEqual = 1 ///< Needed to draw at the far plane.
	};

	/**
	 * @brief Blend modes.
	 * @ingroup group_renderer
	 */
	enum class BlendMode
	{
		Alpha = 0, ///< Source alpha over destination; the default.
		Additive = 1, ///< Source added to destination.
		None = 2 ///< Source replaces destination.
	};

	/**
	* @brief Which faces are culled. Front faces wind anticlockwise.
	* @ingroup group_renderer
	*/
	enum class CullMode
	{
		None	= 0,
		Front	= 1,
		Back	= 2
	};

	/**
	 * @brief The graphics API: GPU state and draw calls. Use it through Uge::RenderCommand.
	 * @ingroup group_renderer
	 *
	 * Blend, depth and cull settings are global; restore the defaults after changing them.
	 */
	class RendererAPI
	{

	public:
		/**
		 * @brief Graphics APIs.
		 */
		enum class API
		{
			None = 0, ///< No renderer.
			OpenGL = 1 ///< OpenGL 4.5, the only one implemented.
		};

	public:
		/** @brief Sets the initial blend and depth state. */
		virtual void Init() = 0;

		/**
		 * @brief Sets the clear colour.
		 * @param color RGBA colour.
		 */
		virtual void SetClearColor(const glm::vec4& color) = 0;
		/**
		 * @brief Sets the viewport.
		 * @param x Left edge in pixels.
		 * @param y Bottom edge in pixels.
		 * @param width Width in pixels.
		 * @param height Height in pixels.
		 */
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		/** @brief Clears the colour and depth buffers. */
		virtual void Clear() = 0;

		/**
		 * @brief Turns depth writes on or off. While they are off, Clear() can't clear depth.
		 * @param enabled Whether to write depth.
		 */
		virtual void SetDepthWrite(bool enabled) = 0;

		/**
		 * @brief Turns the depth test on or off. With it off, depth isn't written either.
		 * @param enabled Whether to test depth.
		 */
		virtual void SetDepthTest(bool enabled) = 0;

		/**
		 * @brief Sets the depth comparison.
		 * @param compare The comparison.
		 */
		virtual void SetDepthFunc(DepthCompare compare) = 0;

		/**
		 * @brief Sets the blend mode.
		 * @param mode The mode.
		 */
		virtual void SetBlendMode(BlendMode mode) = 0;

		/**
		* @brief Sets which faces are culled.
		* @param mode The faces to cull.
		*/
		virtual void SetCullMode(CullMode mode) = 0;

		/**
		 * @brief Draws indexed triangles.
		 * @param vertexArray The geometry, with an index buffer.
		 * @param indexCount Indices to draw, or `0` for all of them.
		 */
		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;

		/**
		 * @brief Draws line segments.
		 * @param vertexArray The lines, two vertices each.
		 * @param vertexCount Vertices to draw.
		 */
		virtual void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) = 0;

		/**
		 * @brief Sets the line width.
		 * @param width Width in pixels.
		 */
		virtual void SetLineWidth(float width) = 0;

		/**
		 * @brief The graphics API, fixed at compile time.
		 * @return The API.
		 */
		inline static API GetAPI() { return m_API; }

	private:
		static API m_API;

	};



}


