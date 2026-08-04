#include <ugpch.h>
#include "EditorAssetManager.h"

#include "Uge/Project/Project.h"
#include "AssetImporter.h"
#include "Uge/Core/FileSystem.h"
#include <Uge/Renderer/Model.h>


#include <fstream>
#include <filesystem>
#include <yaml-cpp/yaml.h>


namespace Uge
{

    static std::map<std::filesystem::path, AssetType> s_assetExtentionMap = {
        { ".png",  AssetType::Texture2D },
        { ".jpg",  AssetType::Texture2D },
        { ".jpeg", AssetType::Texture2D },
        { ".JPEG", AssetType::Texture2D },

        { ".uge",  AssetType::Scene     },
        
        { ".obj",  AssetType::Mesh      },
        { ".fbx",  AssetType::Mesh      },
        { ".gltf", AssetType::Mesh      },
        { ".glb",  AssetType::Mesh      },

        { ".hdr",  AssetType::Environment }
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

        auto memoryIt = m_loadedAssets.find(handle);
        if (memoryIt != m_loadedAssets.end())
        {
            return memoryIt->second;
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

        return handle != 0
            && (m_assetRegistry.find(handle) != m_assetRegistry.end()
                || m_loadedAssets.find(handle) != m_loadedAssets.end());

    }

    bool EditorAssetManager::IsMeshAssetHandleValid(AssetHandle handle) const
    {


        return handle != 0 && m_meshAssetRegistry.find(handle) != m_meshAssetRegistry.end();
    }

    bool EditorAssetManager::IsAssetLoaded(AssetHandle handle) const
    {

        return m_loadedAssets.find(handle) != m_loadedAssets.end()
            || m_loadedAssets.find(handle) != m_loadedAssets.end();

    }


    AssetType EditorAssetManager::GetAssetType(AssetHandle handle) const
    {

        if (!IsAssetHandleValid(handle))
        {
            return AssetType::None;
        }

        auto memoryIt = m_loadedAssets.find(handle);
        if (memoryIt != m_loadedAssets.end())
        {
            return memoryIt->second->GetType();
        }

        return m_assetRegistry.at(handle).Type;

    }

    AssetHandle EditorAssetManager::ImportAsset(const std::filesystem::path& filepath, const std::string& name)
    {
        AssetHandle handle; // TODO: Generate new handle
        AssetMetadata metadata;
        metadata.FilePath = filepath;


        metadata.Type = GetAssetTypeFromFileExtention(filepath.extension()); 
        UG_CORE_ASSERT(metadata.Type != AssetType::None);
        Ref<Asset> asset = AssetImporter::ImportAsset(handle, metadata);    

        if (asset)
        {
            asset->SetName(name);
            asset->m_handle = handle;
            m_loadedAssets[handle] = asset;
            m_assetRegistry[handle] = metadata;
            SerializeAssetRegistry();
        }

        return handle;

    }

    AssetHandle EditorAssetManager::GetOrImportAsset(const std::filesystem::path& filepath, const std::string& name)
    {
        for (const auto& [handle, metadata] : m_assetRegistry)
        {
            if (metadata.FilePath == filepath)
                return handle;
        }

        return ImportAsset(filepath, name);
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

    const MeshAssetMetadata& EditorAssetManager::GetMeshMetadata(AssetHandle handle) const
    {

        static MeshAssetMetadata s_nullMetadata;
        auto it = m_meshAssetRegistry.find(handle);

        if (it == m_meshAssetRegistry.end())
        {
            return s_nullMetadata;

        }
        return it->second;

    }

    void EditorAssetManager::SetMeshMetadata(AssetHandle handle, const MeshAssetMetadata& metadata)
    {
        m_meshAssetRegistry[handle] = metadata;
    }

    AssetHandle EditorAssetManager::AddMemoryOnlyAsset(const Ref<Asset>& asset)
    {
        AssetHandle handle; // Generates a new random handle

        if (asset)
        {
            asset->m_handle = handle;
            m_loadedAssets[handle] = asset;
        }

        return handle;
    }

    std::vector<std::string> EditorAssetManager::GetLoadedAssetsNames()
    {
        std::vector<std::string> names;
        names.resize(m_loadedAssets.size());
        
        uint32_t i = 0;
        for (const auto& [k,v] : m_loadedAssets)
        {
            names[i++] = v->GetName();
        }

        return names;
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

        if (!std::filesystem::exists(path))
        {
            UG_CORE_WARN("EditorAssetManager::DeserializeAssetRegistry - Asset registry file does not exist, creating: {0}", path.string());
            FileSystem::CreateEmptyFile(path);
            return false;
        }

        YAML::Node data;
        try
        {
            data = YAML::LoadFile(path.string());
        }
        catch (YAML::ParserException e)
        {
            UG_CORE_ERROR("EditorAssetManager::DeserializeAssetRegistry - Failed to load asset registry {0}\n	{1}", path.string(), e.what());
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

    void EditorAssetManager::DeleteAsset(AssetHandle handle)
    {

        switch (GetAssetType(handle))
        {
        case AssetType::Texture2D:
            DeleteTexture(handle);
            break;
        case AssetType::Material:
            DeleteMaterial(handle);
            break;
        case AssetType::Mesh:
            DeleteModel(handle);
            break;
        case AssetType::Scene:
            break;
        case AssetType::None:
            break;
        default:
            break;
        }

    }

    void EditorAssetManager::DeleteModel(AssetHandle handle)
    {
        auto meshIt = m_meshAssetRegistry.find(handle);
        if (meshIt != m_meshAssetRegistry.end())
        {
            // Copy before erasing; the entry goes first so the shared-use check below
            // does not count the model that is being deleted.
            std::vector<AssetHandle> dependencies = meshIt->second.Dependencies;
            m_meshAssetRegistry.erase(meshIt);

            for (AssetHandle dependency : dependencies)
            {
                if (IsDependencyOfAnyModel(dependency))
                    continue;   // another model still needs it

                DeleteAsset(dependency);
            }
        }

        // Only unload. The registry entry is the file's identity: erasing it would
        // invalidate the handle that scenes, the content browser and the registry file
        // all still refer to, so the same file could never be assigned again.
        m_loadedAssets.erase(handle);

    }

    void EditorAssetManager::DeleteTexture(AssetHandle handle)
    {
        m_loadedAssets.erase(handle);
    }

    void EditorAssetManager::DeleteMaterial(AssetHandle handle)
    {
        m_loadedAssets.erase(handle);
    }

    bool EditorAssetManager::IsDependencyOfAnyModel(AssetHandle dependency) const
    {
        for (const auto& [modelHandle, meshMetadata] : m_meshAssetRegistry)
        {
            if (std::find(meshMetadata.Dependencies.begin(), meshMetadata.Dependencies.end(), dependency)
                != meshMetadata.Dependencies.end())
                return true;
        }
        return false;
    }

    

}