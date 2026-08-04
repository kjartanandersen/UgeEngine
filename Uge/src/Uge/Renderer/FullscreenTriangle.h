/**
 * @file FullscreenTriangle.h
 * @brief The geometry every fullscreen pass draws.
 * @ingroup group_renderer
 */

#pragma once

#include "Uge/Core/Core.h"
#include "Uge/Renderer/VertexArray.h"

namespace Uge
{

	/**
	 * @brief The shared fullscreen triangle, created on first use.
	 * @return Vertex array with `a_Position` and `a_TexCoord`, and a three-index buffer.
	 * @ingroup group_renderer
	 *
	 * A triangle rather than a quad: one primitive instead of two, and no seam along the
	 * diagonal where a quad's halves meet. Its vertices deliberately overshoot the screen —
	 * positions reach 3.0 and texture coordinates 2.0 — so the visible region is the
	 * inscribed part of a triangle twice its size.
	 *
	 * @warning Requires a live graphics context on the first call.
	 */
	const Ref<VertexArray>& GetFullscreenTriangle();

}
