/**
 * @file Font.h
 * @brief MSDF font atlas generation and storage.
 * @ingroup group_renderer
 */

#pragma once

#include "Uge/Core/Core.h"
#include "Uge/Renderer/Texture.h"


#include <filesystem>
#include <memory>

namespace Uge
{
	struct MSDFData;

	/**
	 * @brief A font loaded from a TrueType file and baked into an MSDF atlas.
	 * @ingroup group_renderer
	 *
	 * A multi-channel signed distance field encodes each glyph's outline as a distance
	 * rather than as coverage, so text stays sharp at any scale and rotation from a single
	 * modest atlas — no per-size rasterization.
	 *
	 * @warning Constructing a Font generates the atlas, which takes noticeable time. Load
	 * fonts up front rather than during a frame, and reuse them.
	 *
	 * @see Renderer2D::DrawString
	 */
	class Font
	{

	public:
		/**
		 * @brief Loads a font and generates its MSDF atlas.
		 * @param filepath Path to a TrueType (`.ttf`) file.
		 */
		Font(const std::filesystem::path& filepath);
		/** @brief Releases the glyph data and the atlas texture. */
		~Font();

		/**
		 * @brief The glyph geometry and font metrics.
		 * @return Pointer to the internal MSDF data; owned by this font.
		 */
		const MSDFData* GetMSDFData() const { return m_data; }

		/**
		 * @brief The generated glyph atlas.
		 * @return The atlas texture sampled when drawing text.
		 */
		Ref<Texture2D> GetAtlasTexture() const { return m_atlasTexture; }

		/**
		 * @brief The engine's built-in font.
		 * @return A shared default font, generated on first use.
		 */
		static Ref<Font> GetDefault();

	private:
		MSDFData* m_data;
		Ref<Texture2D> m_atlasTexture;
	};

}