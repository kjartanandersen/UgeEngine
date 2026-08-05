/**
 * @file RenderStats.h
 * @brief Per-frame draw counters shared by every rendering path.
 * @ingroup group_renderer
 */

#pragma once

#include <cstdint>

namespace Uge
{

	/**
	 * @brief Counters describing the work one frame submitted to the GPU.
	 * @ingroup group_renderer
	 *
	 * Useful for spotting batch breaks: a 2D quad count far above the draw call count is
	 * healthy batching, while the two converging means something is forcing a flush,
	 * usually too many distinct textures.
	 *
	 * The totals are counted in a single place — Uge::RenderCommand::DrawIndexed — so
	 * every path picks them up without per-call-site plumbing. The path-specific fields
	 * are incremented by the path that owns them:
	 *
	 * @code
	 * RenderStats::Get().MeshDrawCount++;
	 * @endcode
	 *
	 * Reset() must be called once per frame, before any drawing. The editor does this at
	 * the top of `EditorLayer::OnUpdate`; deliberately *not* inside a `BeginScene`,
	 * because a frame opens several begin/end pairs and resetting there would discard the
	 * earlier passes' numbers.
	 *
	 * @note Not synchronised. Rendering must stay on the thread owning the graphics
	 * context, so these counters live on that thread too.
	 */
	struct RenderStats
	{
		uint32_t DrawCalls = 0;     ///< Indexed draw calls issued, across all paths.
		uint32_t IndexCount = 0;    ///< Indices submitted, across all paths.
		uint32_t TriangleCount = 0; ///< Triangles submitted, assuming triangle lists.

		uint32_t Quad2DCount = 0;     ///< Sprite and quad batches submitted by Uge::Renderer2D.
		uint32_t Text2DQuadCount = 0; ///< Glyph quads submitted by Uge::Renderer2D.

		uint32_t MeshDrawCount = 0; ///< Meshes drawn through the Uge::Model path.

		/**
		 * @brief Returns the counters for the frame currently being built.
		 * @return Mutable reference to the process-wide instance.
		 */
		static RenderStats& Get();

		/** @brief Zeroes every counter; call once at the start of each frame. */
		static void Reset();
	};

}
