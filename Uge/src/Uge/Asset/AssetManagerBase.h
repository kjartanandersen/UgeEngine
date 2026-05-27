#pragma once
#include "Asset.h"

namespace Uge
{
	using AssetMap = std::map<AssetHandle, Ref<Asset>>;


	class AssetManagerBase
	{


	public:
		virtual Ref<Asset> GetAsset(AssetHandle handle) const = 0;
		virtual bool IsAssetHandleValid(AssetHandle handle) const = 0;
		virtual bool IsAssetLoaded(AssetHandle handle) const = 0;

	};

}