/**
 * @file RenderStats.h
 * @brief Per-frame draw counters.
 * @ingroup group_renderer
 */

#pragma once

#include <cstdint>

namespace Uge
{

	/**
	 * @brief What a frame drew. Not thread-safe.
	 * @ingroup group_renderer
	 *
	 * Call Reset() once per frame, before the first pass.
	 */
	struct RenderStats
	{
		uint32_t DrawCalls = 0;			///< Indexed draw calls.
		uint32_t LinesDrawCalls = 0;		///< Line draw calls.
		
		uint32_t IndexCount = 0;		///< Indices drawn.
		uint32_t TriangleCount = 0;		///< Indices drawn, divided by 3.
		uint32_t LinesCount = 0;		///< Line vertices drawn.

		uint32_t Quad2DCount = 0;		///< Uge::Renderer2D quads.
		uint32_t Text2DQuadCount = 0;	///< Uge::Renderer2D glyph quads.

		uint32_t MeshDrawCount = 0;		///< Submeshes drawn.

		uint32_t MeshCulledCount = 0;	///< Submeshes skipped by frustum culling.


		/**
		 * @brief This frame's counters.
		 * @return The counters.
		 */
		static RenderStats& Get();

		/** @brief Zeroes the counters. */
		static void Reset();
	};

}
