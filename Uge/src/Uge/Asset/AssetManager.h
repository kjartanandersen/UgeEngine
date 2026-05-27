#pragma once

#include "AssetManagerBase.h"

#include "Uge/Project/Project.h"

namespace Uge
{

	class AssetManager
	{

	public:

		template<typename T>
		static Ref<T> GetAsset(AssetHandle handle)
		{
			Ref<Asset> asset = Project::GetActive()->GetAssetManager()->GetAsset(handle);
			return std::static_pointer_cast<T>(asset);

		}
	};

}