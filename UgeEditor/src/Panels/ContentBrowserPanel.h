/**
 * @file ContentBrowserPanel.h
 * @brief Browses the project's asset directory.
 * @ingroup group_editor
 */

#pragma once


#include <filesystem>
#include <map>

#include "Uge.h"

namespace Uge
{

	/**
	 * @brief Browses the project's content, as either registered assets or raw files.
	 * @ingroup group_editor
	 *
	 * Two modes. **Asset** mode shows a tree built from the asset registry, so only
	 * imported content appears. **FileSystem** mode walks the asset directory directly,
	 * showing files that have not been imported yet.
	 *
	 * Entries can be dragged onto the viewport or into the properties panel to assign the
	 * asset, which is carried as an Uge::AssetHandle in the ImGui drag-and-drop payload.
	 */
	class ContentBrowserPanel
	{

	public:
		/** @brief Constructs the panel, loads its icons and builds the asset tree. */
		ContentBrowserPanel();

		/** @brief Draws the breadcrumb bar and the item grid for the current directory. */
		void OnImGuiRender();

	private:
		/** @brief Rebuilds the directory tree from the current asset registry. */
		void RefreshAssetTree();

	private:
		std::filesystem::path m_currentDirectory;
		std::filesystem::path m_baseDirectory;

		Ref<Texture2D> m_directoryIcon;
		Ref<Texture2D> m_fileIcon;

		/**
		 * @brief One directory or asset in the browser's tree.
		 *
		 * Nodes live in a flat vector and refer to each other by index rather than by pointer,
		 * so rebuilding the tree cannot leave dangling references.
		 */
		struct TreeNode
		{
			std::filesystem::path Path; ///< Path this node represents.
			AssetHandle Handle = 0; ///< Asset handle; `0` for a directory.

			uint32_t Parent = (uint32_t)-1; ///< Index of the parent node; `-1` at the root.
			std::map<std::filesystem::path, uint32_t> Children; ///< Child nodes, by name, as indices into the node vector.

			/** @brief Constructs an empty node. */
			TreeNode() = default;

			/**
			 * @brief Constructs a node for a path.
			 * @param path Path this node represents.
			 * @param handle Asset handle, or `0` for a directory.
			 */
			TreeNode(const std::filesystem::path& path, AssetHandle handle)
				: Path(path), Handle(handle) { }
		};


		std::vector<TreeNode> m_treeNodes;

		std::map<std::filesystem::path, std::vector<std::filesystem::path>> m_assetTree;

		/** @brief What the browser lists. */
		enum class Mode
		{
			Asset = 0, FileSystem = 1 ///< Registered assets only, or every file on disk.
		};

		Mode m_mode = Mode::Asset;

	};


}

