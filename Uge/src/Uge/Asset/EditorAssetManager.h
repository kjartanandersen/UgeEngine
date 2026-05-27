#pragma once

#include "AssetManagerBase.h"
#include "AssetMetadata.h"
#include <map>

namespace Uge
{
	using AssetRegistry = std::map<AssetHandle, AssetMetadata>;

	class EditorAssetManager : public AssetManagerBase
	{

	public:
		virtual Ref<Asset> GetAsset(AssetHandle handle) const override;
		
		virtual bool IsAssetHandleValid(AssetHandle handle) const override;
		virtual bool IsAssetLoaded(AssetHandle handle) const override;

		const AssetMetadata& GetMetadata(AssetHandle handle) const;

	private:
		AssetRegistry m_assetRegistry;
		AssetMap  m_loadedAssets;

		// TODO: memory only assets

	};

}