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
			Ref<Asset> asset = Manager().GetAsset(handle);
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

			return Manager().IsAssetHandleValid(handle);

		}
		/**
		 * @brief Whether an asset is in memory.
		 * @param handle Asset to test.
		 * @return `true` if it is loaded.
		 */
		static bool IsAssetLoaded(AssetHandle handle) 
		{

			return Manager().IsAssetLoaded(handle);


		}
		/**
		 * @brief An asset's type, without loading it.
		 * @param handle Asset to query.
		 * @return The type, or Uge::AssetType::None for an unknown handle.
		 */
		static AssetType GetAssetType(AssetHandle handle)
		{

			return Manager().GetAssetType(handle);


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

			Manager().DeleteAsset(handle);

		}

	private:

		/** @brief The active project's asset manager. Asserts if there is none. */
		static AssetManagerBase& Manager()
		{
			const Ref<Project>& project = Project::GetActive();
			UG_CORE_ASSERT(project && project->GetAssetManager(), "No active project with an asset manager is loaded");
			return *project->GetAssetManager();
		}

	};

}