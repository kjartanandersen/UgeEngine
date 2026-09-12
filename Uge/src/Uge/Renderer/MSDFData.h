/**
 * @file MSDFData.h
 * @brief Storage for msdf-atlas-gen glyph data.
 * @ingroup group_renderer
 */

#pragma once

#include <vector>

#undef INFINITE
#include <msdf-atlas-gen.h>

namespace Uge
{

	/**
	 * @brief Holds the glyph geometry and font metrics produced by msdf-atlas-gen.
	 * @ingroup group_renderer
	 *
	 * Kept in its own header so Font.h can forward-declare it and callers avoid pulling in
	 * the msdf-atlas-gen headers.
	 */
	struct MSDFData
	{

		std::vector<msdf_atlas::GlyphGeometry> Glyphs; ///< Per-glyph atlas placement and outline data.
		msdf_atlas::FontGeometry Fonts; ///< Font-wide metrics: line height, kerning pairs, glyph lookup.


	};


}