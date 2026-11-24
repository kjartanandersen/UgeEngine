/**
 * @file Font.h
 * @brief Fonts baked into MSDF atlases.
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
	 * @brief A TrueType font baked into a multi-channel signed distance field atlas.
	 * @ingroup group_renderer
	 *
	 * The atlas is built in the constructor, which is slow; don't create fonts mid-frame.
	 */
	class Font
	{

	public:
		/**
		 * @brief Loads a font and builds its atlas. Logs an error if the file doesn't load.
		 * @param filepath Path to a `.ttf` file.
		 */
		Font(const std::filesystem::path& filepath);
		/** @brief Releases the glyph data and the atlas texture. */
		~Font();

		/**
		 * @brief The glyph geometry and font metrics.
		 * @return The data, owned by the font.
		 */
		const MSDFData* GetMSDFData() const { return m_data; }

		/**
		 * @brief The glyph atlas.
		 * @return The atlas texture.
		 */
		Ref<Texture2D> GetAtlasTexture() const { return m_atlasTexture; }

		/**
		 * @brief The built-in font, loaded on first use.
		 * @return The font.
		 */
		static Ref<Font> GetDefault();

		/** @brief Releases the default font and its atlas texture. */ 
		static void ResetDefault();

	private:
		MSDFData* m_data;
		Ref<Texture2D> m_atlasTexture;
	};

}