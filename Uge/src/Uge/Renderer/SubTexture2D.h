/**
 * @file SubTexture2D.h
 * @brief A region of a texture atlas.
 * @ingroup group_renderer
 */

#pragma once
#include "Texture.h"

#include <glm/glm.hpp>

namespace Uge
{

	/**
	 * @brief A rectangle of a texture, as four corner texture coordinates.
	 * @ingroup group_renderer
	 */
	class SubTexture2D
	{

	public:
		/**
		 * @brief A region between two texture coordinates.
		 * @param texture The atlas.
		 * @param min Lower-left corner, in `[0, 1]`.
		 * @param max Upper-right corner, in `[0, 1]`.
		 */
		SubTexture2D(const Ref<Texture2D>& texture, const glm::vec2& min, const glm::vec2& max);

		/**
		 * @brief The atlas.
		 * @return The texture.
		 */
		const Ref<Texture2D> GetTexture() const { return m_texture; }
		/**
		 * @brief The corner texture coordinates.
		 * @return Four coordinates, anticlockwise from the lower left.
		 */
		const glm::vec2* GetTextCoords() const { return m_textureCoords; }

		/**
		 * @brief A region of a grid of cells.
		 * @param texture The atlas.
		 * @param coords The cell's column and row, from the lower left.
		 * @param cellSize Cell size in pixels.
		 * @param spriteSize How many cells the region covers.
		 * @return The region.
		 */
		static Ref<SubTexture2D> CreateFromCoords(const Ref<Texture2D>& texture, const glm::vec2& coords, const glm::vec2& cellSize, const glm::vec2& spriteSize = {1, 1});

	private:
		Ref<Texture2D> m_texture;

		glm::vec2 m_textureCoords[4];


	};

}

