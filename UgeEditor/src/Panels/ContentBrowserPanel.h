/**
 * @file ContentBrowserPanel.h
 * @brief The content browser panel.
 * @ingroup group_editor
 */

#pragma once


#include <filesystem>
#include <map>

#include "Uge.h"

namespace Uge
{

	/**
	 * @brief Lists the project's registered assets, or every file in the asset directory.
	 * @ingroup group_editor
	 *
	 * An asset can be dragged out; the payload is its Uge::AssetHandle.
	 */
	class ContentBrowserPanel
	{

	public:
		/** @brief Loads the icons and builds the asset tree. Needs an active project. */
		ContentBrowserPanel();

		/** @brief Draws the current directory. */
		void OnImGuiRender();
		
		/** @brief Rebuilds the asset tree from the registry. */
		void RefreshAssetTree();

	private:
		std::filesystem::path m_currentDirectory;
		std::filesystem::path m_baseDirectory;

		Ref<Texture2D> m_directoryIcon;
		Ref<Texture2D> m_fileIcon;

		/**
		 * @brief A directory or asset in the tree. Nodes refer to each other by index.
		 */
		struct TreeNode
		{
			std::filesystem::path Path; ///< The path's last component.
			AssetHandle Handle = 0; ///< The asset; `0` for a directory.

			uint32_t Parent = (uint32_t)-1; ///< The parent's index; `-1` at the root.
			std::map<std::filesystem::path, uint32_t> Children; ///< Child indices, by name.

			/** @brief An empty node. */
			TreeNode() = default;

			/**
			 * @brief A node for a path.
			 * @param path The path.
			 * @param handle The asset, or `0` for a directory.
			 */
			TreeNode(const std::filesystem::path& path, AssetHandle handle)
				: Path(path), Handle(handle) { }
		};


		std::vector<TreeNode> m_treeNodes;

		std::map<std::filesystem::path, std::vector<std::filesystem::path>> m_assetTree;

		/** @brief What the browser lists: registered assets, or every file. */
		enum class Mode
		{
			Asset = 0, FileSystem = 1
		};

		Mode m_mode = Mode::Asset;

	};


}

