#pragma once

#include "Uge/Core/Core.h"
#include "Uge/Renderer/Texture.h"


#include <filesystem>
#include <memory>

namespace Uge
{
	struct MSDFData;

	class Font
	{

	public:
		Font(const std::filesystem::path& filepath);
		~Font();

		const MSDFData* GetMSDFData() const { return m_data; }

		Ref<Texture2D> GetAtlasTexture() const { return m_atlasTexture; }

		static Ref<Font> GetDefault();

	private:
		MSDFData* m_data;
		Ref<Texture2D> m_atlasTexture;
	};

}