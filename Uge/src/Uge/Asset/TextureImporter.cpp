#include <ugpch.h>

#include "TextureImporter.h"

#include <stb_image.h>


namespace Uge
{
	Ref<Texture2D> TextureImporter::ImportTexture2D(AssetHandle handle, const AssetMetadata& metadata)
	{

		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		Buffer data;

		std::string pathString = metadata.FilePath.string();
		{
			UG_PROFILE_SCOPE("stbi_load - TextureImporter::ImportTexture2D");

			data.Data = stbi_load(pathString.c_str(), &width, &height, &channels, STBI_rgb_alpha);

		}

		if (data.Data == nullptr)
		{
			UG_CORE_ERROR("TextureImporter::ImportTexture2D - Could not load texture from filepath: {0} ", pathString);
		}

		TextureSpecification spec;
		spec.Width = width;
		spec.Height = height;

		// TODO: Not ideal for HDR
		data.Size = static_cast<uint64_t>(width) * height * channels;

		switch (channels)
		{
			case 3:
			{
				spec.Format = ImageFormat::RGB8;

				break;
			}
			case 4:
			{

				spec.Format = ImageFormat::RGBA8;

				break;
			}
		}

		return Texture2D::Create(spec, data);

		
	}
}