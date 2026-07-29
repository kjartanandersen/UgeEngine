/**
 * @file SubTexture2D.h
 * @brief A rectangular region of a texture atlas.
 * @ingroup group_renderer
 */

#pragma once
#include "Texture.h"

#include <glm/glm.hpp>

namespace Uge
{

	/**
	 * @brief A sub-rectangle of a texture, addressed by its four corner UVs.
	 * @ingroup group_renderer
	 *
	 * Lets many sprites share one atlas texture, which keeps the 2D batcher from flushing
	 * on every sprite change.
	 *
	 * @code
	 * auto atlas = Texture2D::Create(spec, pixels);
	 * // the tile at column 2, row 3 of a 128x128 grid
	 * auto tile = SubTexture2D::CreateFromCoords(atlas, { 2, 3 }, { 128, 128 });
	 * @endcode
	 */
	class SubTexture2D
	{

	public:
		/**
		 * @brief Creates a sub-texture from explicit UV bounds.
		 * @param texture Atlas texture to sample from.
		 * @param min Lower-left UV corner, in `[0, 1]`.
		 * @param max Upper-right UV corner, in `[0, 1]`.
		 */
		SubTexture2D(const Ref<Texture2D>& texture, const glm::vec2& min, const glm::vec2& max);

		/**
		 * @brief The atlas texture.
		 * @return The texture this region belongs to.
		 */
		const Ref<Texture2D> GetTexture() const { return m_texture; }
		/**
		 * @brief The region's texture coordinates.
		 * @return Pointer to four UVs, counter-clockwise from the lower-left corner.
		 */
		const glm::vec2* GetTextCoords() const { return m_textureCoords; }

		/**
		 * @brief Creates a sub-texture from grid cell coordinates rather than raw UVs.
		 * @param texture Atlas texture to sample from.
		 * @param coords Cell index as `(column, row)`, counting from the lower-left.
		 * @param cellSize Size of one grid cell in pixels.
		 * @param spriteSize How many cells the sprite spans, for multi-cell sprites.
		 * @return The new sub-texture.
		 */
		static Ref<SubTexture2D> CreateFromCoords(const Ref<Texture2D>& texture, const glm::vec2& coords, const glm::vec2& cellSize, const glm::vec2& spriteSize = {1, 1});

	private:
		Ref<Texture2D> m_texture;

		glm::vec2 m_textureCoords[4];


	};

}

