#pragma once


#include <filesystem>

namespace Uge
{

	class ContentBrowserPanel
	{

	public:
		ContentBrowserPanel();

		void OnImGuiRender();

	private:
		std::filesystem::path m_currentDirectory;

	};


}

