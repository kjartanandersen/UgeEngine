#include <ugpch.h>

#include "Uge/Project/Project.h"
#include "EditorAssetManager.h"
#include "AssetImporter.h"

#include <fstream>
#include <yaml-cpp/yaml.h>


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

                UG_CORE_ERROR("EditorAssetManager::GetAsset - asset import failed!");

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

    void EditorAssetManager::ImportAsset(const std::filesystem::path& filepath)
    {
        AssetHandle handle; // TODO: Generate new handle
        AssetMetadata metadata;
        metadata.FilePath = filepath;
        metadata.Type = AssetType::Texture2D; // TODO: Grab from file extension
        Ref<Asset> asset = AssetImporter::ImportAsset(handle, metadata);    
        asset->m_handle = handle;

        if (asset)
        {
            m_loadedAssets[handle] = asset;
            m_assetRegistry[handle] = metadata;
            SerializeAssetRegistry();
        }

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

    void EditorAssetManager::SerializeAssetRegistry()
    {
        auto path = Project::GetAssetRegistryPath();

        YAML::Emitter out;

        out << YAML::BeginMap;		// Root
        {
            out << YAML::Key << "AssetRegistry" << YAML::Value;
            
            out << YAML::BeginSeq;
            {
                for (const auto& [handle, metadata] : m_assetRegistry)
                {
                    out << YAML::BeginMap;
                    {
                        out << YAML::Key << "Handle" << YAML::Value << handle;
                        std::string filepathString = metadata.FilePath.generic_string();
                        out << YAML::Key << "FilePath" << YAML::Value << filepathString;
                        out << YAML::Key << "Type" << YAML::Value << AssetTypeToString(metadata.Type);


                    }
                    out << YAML::EndMap;
                }

            }
            out << YAML::EndSeq;	
        }
        out << YAML::EndMap;		// Root

        std::ofstream fout(path);
        fout << out.c_str();


    }

    bool EditorAssetManager::DeserializeAssetRegistry()
    {

        auto path = Project::GetAssetRegistryPath();

        YAML::Node data;
        try
        {
            data = YAML::LoadFile(path.string());
        }
        catch (YAML::ParserException e)
        {
            UG_CORE_ERROR("Failed to load .ugproj file {0}\n	{1}", path.string(), e.what());
            return false;
        }

        auto& projectNode = data["AssetRegistry"];
        if (!projectNode)
        {
            return false;
        }

        for (const auto& node : projectNode)
        {
            AssetHandle handle = node["Handle"].as<uint64_t>();
            auto& metadata = m_assetRegistry[handle];

            metadata.FilePath = node["FilePath"].as<std::string>();
            metadata.Type = AssetTypeFromString(node["Type"].as<std::string>());
            
        }

        return true;

    }

    

}