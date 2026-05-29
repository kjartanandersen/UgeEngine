#include <ugpch.h>

#include "Asset.h"

namespace Uge
{
    std::string_view AssetTypeToString(AssetType type)
    {

        switch (type)
        {
            case Uge::AssetType::None:
            {
                return "AssetType::None";
                
            }
            case Uge::AssetType::Scene:
            {

                return "AssetType::Scene";
            }
            case Uge::AssetType::Texture2D:
            {

                return "AssetType::Texture2D";
            }

        }

        return "AssetType::<Invalid>";

    }

    AssetType AssetTypeFromString(std::string_view type)
    {

        if (type == "AssetType::None")
        {
            return AssetType::None;
        }
        if (type == "AssetType::Scene")
        {
            return AssetType::Scene;
        }
        if (type == "AssetType::Texture2D")
        {
            return AssetType::Texture2D;
        }

        return AssetType::None;
        
    }
}