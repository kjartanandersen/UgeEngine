#pragma once


#include <filesystem>
#include <map>

#include "Uge.h"

namespace Uge
{

	class ContentBrowserPanel
	{

	public:
		ContentBrowserPanel();

		void OnImGuiRender();

	private:
		void RefreshAssetTree();

	private:
		std::filesystem::path m_currentDirectory;
		std::filesystem::path m_baseDirectory;

		Ref<Texture2D> m_directoryIcon;
		Ref<Texture2D> m_fileIcon;

		struct TreeNode
		{
			std::filesystem::path Path;
			uint32_t Parent = (uint32_t)-1;
			std::map<std::filesystem::path, uint32_t> Children;

			TreeNode() = default;

			TreeNode(const std::filesystem::path& path)
				: Path(path) { }
		};


		std::vector<TreeNode> m_treeNodes;

		std::map<std::filesystem::path, std::vector<std::filesystem::path>> m_assetTree;

		enum class Mode
		{
			Asset = 0, FileSystem = 1
		};

		Mode m_mode = Mode::Asset;

	};


}

