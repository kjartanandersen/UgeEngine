#include <ugpch.h>

#include "EditorAssetManager.h"
#include "AssetImporter.h"

namespace Uge
{
    Ref<Asset> EditorAssetManager::GetAsset(AssetHandle handle) const
    {

        if (!IsAssetHandleValid(handle))
        {

            return nullptr;
        
        }
        Ref<Asset> asset;
        if (IsAssetLoaded(handle))
        {

            asset = m_loadedAssets.at(handle);
        
        }
        else
        {

            const AssetMetadata& metadata = GetMetadata(handle);

            asset =  AssetImporter::ImportAsset(handle, metadata);
            if (!asset)
            {



            }
        }

        return asset;



    }
    bool EditorAssetManager::IsAssetHandleValid(AssetHandle handle) const
    {

        return handle != 0 || m_assetRegistry.find(handle) != m_assetRegistry.end();
    
    }
    bool EditorAssetManager::IsAssetLoaded(AssetHandle handle) const
    {

        return m_loadedAssets.find(handle) != m_loadedAssets.end();
        
    }
    const AssetMetadata& EditorAssetManager::GetMetadata(AssetHandle handle) const
    {
        static AssetMetadata s_nullMetadata;
        auto it = m_assetRegistry.find(handle);

        if (it == m_assetRegistry.end())
        {
            return s_nullMetadata;

        }
        return it->second;

    }
}