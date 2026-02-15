#pragma once


#include <filesystem>

#include "Uge.h"

namespace Uge
{

	class ContentBrowserPanel
	{

	public:
		ContentBrowserPanel();

		void OnImGuiRender();

	private:
		std::filesystem::path m_currentDirectory;

		Ref<Texture2D> m_directoryIcon;
		Ref<Texture2D> m_fileIcon;

	};


}

