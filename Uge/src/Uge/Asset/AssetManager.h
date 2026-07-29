/**
 * @file AssetManager.h
 * @brief Static facade forwarding to the active project's asset manager.
 * @ingroup group_asset
 */

#pragma once

#include "AssetManagerBase.h"

#include "Uge/Project/Project.h"

namespace Uge
{

	/**
	 * @brief Static entry point for asset lookups.
	 * @ingroup group_asset
	 *
	 * Every call forwards to the manager owned by the active Uge::Project, which keeps
	 * call sites from having to thread a manager reference through everything.
	 *
	 * @warning Requires a loaded project. Each method dereferences
	 * Uge::Project::GetActive(), which asserts when no project is active — so nothing
	 * asset-related can run before a project is opened.
	 *
	 * @code
	 * if (AssetManager::IsAssetHandleValid(component.Mesh))
	 * {
	 *     auto model = AssetManager::GetAsset<Model>(component.Mesh);
	 *     model->Draw(transform, (int)entity);
	 * }
	 * @endcode
	 */
	class AssetManager
	{

	public:

		/**
		 * @brief Resolves a handle to a typed asset, loading it if necessary.
		 * @tparam T Expected asset type, such as Uge::Texture2D or Uge::Model.
		 * @param handle Asset to fetch.
		 * @return The asset cast to `T`, or null if the handle is unknown.
		 *
		 * @warning The cast is unchecked. Passing a `T` that does not match the asset's real
		 * type yields an invalid pointer rather than null — test with GetAssetType() first
		 * when the type is not certain.
		 */
		template<typename T>
		static Ref<T> GetAsset(AssetHandle handle)
		{
			Ref<Asset> asset = Project::GetActive()->GetAssetManager()->GetAsset(handle);
			return std::static_pointer_cast<T>(asset);

		}

		/**
		 * @brief Whether a handle refers to a known asset.
		 * @param handle Handle to test.
		 * @return `true` if the asset is registered.
		 */
		static bool IsAssetHandleValid(AssetHandle handle) 
		{

			return Project::GetActive()->GetAssetManager()->IsAssetHandleValid(handle);

		}
		/**
		 * @brief Whether an asset is currently in memory.
		 * @param handle Handle to test.
		 * @return `true` if the asset is loaded.
		 */
		static bool IsAssetLoaded(AssetHandle handle) 
		{

			return Project::GetActive()->GetAssetManager()->IsAssetLoaded(handle);


		}
		/**
		 * @brief The type of a registered asset.
		 * @param handle Handle to query.
		 * @return The asset type, or Uge::AssetType::None if unknown.
		 */
		static AssetType GetAssetType(AssetHandle handle) 
		{

			return Project::GetActive()->GetAssetManager()->GetAssetType(handle);


		}
	};

}