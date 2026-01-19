#include <ugpch.h>
#include "SubTexture2D.h"


namespace Uge
{


	Uge::SubTexture2D::SubTexture2D(const Ref<Texture2D>& texture, 
		const glm::vec2& min, const glm::vec2& max)
		: m_texture(texture)
	{

		m_textureCoords[0] = { min.x, min.y };
		m_textureCoords[1] = { max.x, min.y };
		m_textureCoords[2] = { max.x, max.y };
		m_textureCoords[3] = { min.x, max.y };


	}

	Ref<SubTexture2D> SubTexture2D::CreateFromCoords(const Ref<Texture2D>& texture, const glm::vec2& coords, const glm::vec2& cellSize, const glm::vec2& spriteSize)
	{
		glm::vec2 min = { (coords.x * cellSize.x) / texture->GetWidth() , (coords.y * cellSize.y) / texture->GetHeight() };
		glm::vec2 max = { ((coords.x + spriteSize.x) * cellSize.x) / texture->GetWidth(), ((coords.y + spriteSize.y) * cellSize.y) / texture->GetHeight() };

		return CreateRef<SubTexture2D>(texture, min, max);
	}

}