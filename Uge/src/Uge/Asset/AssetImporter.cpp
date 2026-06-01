#include <ugpch.h>

#include "AssetImporter.h"
#include "TextureImporter.h"
#include "SceneImporter.h"

namespace Uge
{
	
	using AssetImportFunction = std::function<Ref<Asset>(AssetHandle, const AssetMetadata&)>;
	static std::map<AssetType, AssetImportFunction> s_assetImportFuncs = {
		{AssetType::Texture2D, TextureImporter::ImportTexture2D },
		{AssetType::Scene, SceneImporter::ImportScene		}
	};

	Ref<Asset> AssetImporter::ImportAsset(AssetHandle handle, const AssetMetadata& metadata)
	{

		if (s_assetImportFuncs.find(metadata.Type) == s_assetImportFuncs.end())
		{
			UG_CORE_ERROR("No importer available for asset type: {}", (uint16_t)metadata.Type);
			return nullptr;
		}

		return s_assetImportFuncs.at(metadata.Type)(handle, metadata);

	}
}