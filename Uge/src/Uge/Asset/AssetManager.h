/**
 * @file AssetManager.h
 * @brief Static access to the active project's asset manager.
 * @ingroup group_asset
 */

#pragma once

#include "AssetManagerBase.h"

#include "Uge/Project/Project.h"

namespace Uge
{

	/**
	 * @brief Static access to the active project's asset manager.
	 * @ingroup group_asset
	 *
	 * Every call asserts when no project is loaded.
	 */
	class AssetManager
	{

	public:

		/**
		 * @brief Returns an asset, loading it if needed.
		 * @tparam T Asset class. It must define a static `GetStaticType()`.
		 * @param handle Asset to fetch.
		 * @return The asset, or null if the handle is unknown, the import failed or the
		 * asset is not a `T`.
		 *
		 * A handle of another type still loads that asset.
		 */
		template<typename T>
		static Ref<T> GetAsset(AssetHandle handle)
		{
			Ref<Asset> asset = Project::GetActive()->GetAssetManager()->GetAsset(handle);
			if (!asset || asset->GetType() != T::GetStaticType())
			{
				return nullptr;
			}

			return std::static_pointer_cast<T>(asset);
		}

		/**
		 * @brief Whether a handle names a known asset.
		 * @param handle Handle to test.
		 * @return `true` if the asset is registered or in memory.
		 */
		static bool IsAssetHandleValid(AssetHandle handle) 
		{

			return Project::GetActive()->GetAssetManager()->IsAssetHandleValid(handle);

		}
		/**
		 * @brief Whether an asset is in memory.
		 * @param handle Asset to test.
		 * @return `true` if it is loaded.
		 */
		static bool IsAssetLoaded(AssetHandle handle) 
		{

			return Project::GetActive()->GetAssetManager()->IsAssetLoaded(handle);


		}
		/**
		 * @brief An asset's type, without loading it.
		 * @param handle Asset to query.
		 * @return The type, or Uge::AssetType::None for an unknown handle.
		 */
		static AssetType GetAssetType(AssetHandle handle)
		{

			return Project::GetActive()->GetAssetManager()->GetAssetType(handle);


		}

		/**
		 * @brief Unloads an asset, and any dependency no other model uses.
		 * @param handle Asset to unload.
		 *
		 * The handle stays valid, and the next GetAsset() imports the asset again. A `Ref`
		 * held elsewhere keeps the asset alive.
		 */
		static void DeleteAsset(AssetHandle handle)
		{

			Project::GetActive()->GetAssetManager()->DeleteAsset(handle);

		}
	};

}