/**
 * @file EditorIcons.h
 * @brief Loading the editor's icons.
 * @ingroup group_editor
 */

#pragma once

#include "Uge.h"
#include "Uge/Asset/TextureImporter.h"

#include <cstdint>
#include <filesystem>

namespace Uge
{

	/**
	 * @brief Loads an editor icon, or makes a 1x1 white texture if the file doesn't load.
	 * @param path The image, relative to the working directory.
	 * @return The icon; never null.
	 * @ingroup group_editor
	 *
	 * Needs an open project, as TextureImporter::LoadTexture2D does. That function logs
	 * a file that doesn't load.
	 */
	inline Ref<Texture2D> LoadEditorIcon(const std::filesystem::path& path)
	{
		if (Ref<Texture2D> icon = TextureImporter::LoadTexture2D(path))
		{
			return icon;
		}

		uint32_t white = 0xffffffff;
		return Texture2D::Create(TextureSpecification(), Buffer(&white, sizeof(white)));
	}

}