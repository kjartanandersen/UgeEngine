#include <ugpch.h>

#include "Uge/Project/Project.h"
#include "EditorAssetManager.h"
#include "AssetImporter.h"


#include <fstream>
#include <yaml-cpp/yaml.h>


namespace Uge
{

    static std::map<std::filesystem::path, AssetType> s_assetExtentionMap = {
        { ".png",  AssetType::Texture2D },
        { ".jpg",  AssetType::Texture2D },
        { ".jpeg", AssetType::Texture2D },

        { ".uge",  AssetType::Scene     },
        
        { ".obj",  AssetType::Mesh      },
        { ".fbx",  AssetType::Mesh      },
        { ".gltf", AssetType::Mesh      },
        { ".glb",  AssetType::Mesh      }
    };

    static AssetType GetAssetTypeFromFileExtention(const std::filesystem::path& extention)
    {

        if (s_assetExtentionMap.find(extention) == s_assetExtentionMap.end())
        {
            UG_CORE_WARN("Could not find AssetType for extention: {0}", extention.string());
            return AssetType::None;

        }

        return s_assetExtentionMap.at(extention);

    }

    Ref<Asset> EditorAssetManager::GetAsset(AssetHandle handle) 
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
            m_loadedAssets[handle] = asset;
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

    AssetType EditorAssetManager::GetAssetType(AssetHandle handle) const
    {

        if (!IsAssetHandleValid(handle))
        {
            return AssetType::None;
        }

        return m_assetRegistry.at(handle).Type;

    }

    AssetHandle EditorAssetManager::ImportAsset(const std::filesystem::path& filepath)
    {
        AssetHandle handle; // TODO: Generate new handle
        AssetMetadata metadata;
        metadata.FilePath = filepath;


        metadata.Type = GetAssetTypeFromFileExtention(filepath.extension()); 
        UG_CORE_ASSERT(metadata.Type != AssetType::None);
        Ref<Asset> asset = AssetImporter::ImportAsset(handle, metadata);    

        if (asset)
        {
            asset->m_handle = handle;
            m_loadedAssets[handle] = asset;
            m_assetRegistry[handle] = metadata;
            SerializeAssetRegistry();
        }

        return handle;

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

    const std::filesystem::path& EditorAssetManager::GetFilePath(AssetHandle handle) const
    {

        return GetMetadata(handle).FilePath;
        
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