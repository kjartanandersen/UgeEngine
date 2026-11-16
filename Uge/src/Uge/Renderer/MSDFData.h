/**
 * @file MSDFData.h
 * @brief A font's msdf-atlas-gen data.
 * @ingroup group_renderer
 */

#pragma once

#include <vector>

#undef INFINITE
#include <msdf-atlas-gen.h>

namespace Uge
{

	/**
	 * @brief Glyph geometry and font metrics from msdf-atlas-gen.
	 * @ingroup group_renderer
	 */
	struct MSDFData
	{

		std::vector<msdf_atlas::GlyphGeometry> Glyphs; ///< Each glyph's outline and place in the atlas.
		msdf_atlas::FontGeometry Fonts; ///< Line height, kerning and glyph lookup.


	};


}