#pragma once

#include "Asset.h"
#include "AssetMetadata.h"



namespace Uge
{

	class AssetImporter
	{

	public:
		static Ref<Asset> ImportAsset(AssetHandle handle, const AssetMetadata& metadata);

	};

}