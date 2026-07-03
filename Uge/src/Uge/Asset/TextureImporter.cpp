#include <ugpch.h>

#include "TextureImporter.h"

#include "Uge/Project/Project.h"

#include <stb_image.h>


namespace Uge
{
	Ref<Texture2D> TextureImporter::ImportTexture2D(AssetHandle handle, const AssetMetadata& metadata)
	{

		UG_PROFILE_FUNCTION();

		return LoadTexture2D(Project::GetAssetDirectory() / metadata.FilePath);

		
	}

	Ref<Texture2D> TextureImporter::LoadTexture2D(const std::filesystem::path& path)
	{

		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		Buffer data;

		std::string pathString = path.string();
		{
			UG_PROFILE_SCOPE("stbi_load - TextureImporter::LoadTexture2D");

			data.Data = stbi_load(pathString.c_str(), &width, &height, &channels, STBI_rgb_alpha);

		}

		if (data.Data == nullptr)
		{
			UG_CORE_ERROR("TextureImporter::LoadTexture2D - Could not load texture from filepath: {0} ", pathString);
			return nullptr;
		}

		TextureSpecification spec;


		spec.Width = width;
		spec.Height = height;

		// We force STBI_rgb_alpha above, so stb always hands back 4-channel RGBA data
		// regardless of the source image's channel count.
		spec.Format = ImageFormat::RGBA8;
		data.Size = static_cast<uint64_t>(width) * height * 4;

		Ref<Texture2D> texture = Texture2D::Create(spec, data);
		texture->SetName(Project::GetActive()->GetRelativePathString(pathString));

		data.Release();
		return texture;

	}
}