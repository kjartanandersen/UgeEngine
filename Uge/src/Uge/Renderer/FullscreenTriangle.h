/**
 * @file FullscreenTriangle.h
 * @brief The triangle that fullscreen passes draw.
 * @ingroup group_renderer
 */

#pragma once

#include "Uge/Core/Core.h"
#include "Uge/Renderer/VertexArray.h"

namespace Uge
{

	/**
	 * @brief A triangle covering the screen, created on first use.
	 * @return Vertex array with `a_Position` and `a_TexCoord`.
	 * @ingroup group_renderer
	 *
	 * Its corners reach 3 in clip space, so the screen is the part of it from -1 to 1.
	 */
	const Ref<VertexArray>& GetFullscreenTriangle();

	/** @brief Releases the shared triangle. */ 
	void ReleaseFullscreenTriangle();

}
