#pragma once
#include "Texture.h"

#include <glm/glm.hpp>

namespace Uge
{

	class SubTexture2D
	{

	public:
		SubTexture2D(const Ref<Texture2D>& texture, const glm::vec2& min, const glm::vec2& max);

		const Ref<Texture2D> GetTexture() const { return m_texture; }
		const glm::vec2* GetTextCoords() const { return m_textureCoords; }

		static Ref<SubTexture2D> CreateFromCoords(const Ref<Texture2D>& texture, const glm::vec2& coords, const glm::vec2& cellSize, const glm::vec2& spriteSize = {1, 1});

	private:
		Ref<Texture2D> m_texture;

		glm::vec2 m_textureCoords[4];


	};

}

