#include <ugpch.h>
#include "ContentBrowserPanel.h"

#include <imgui.h>


namespace Uge
{

	// TODO: To be changed later
	static const std::filesystem::path s_assetPath = "assets";



	ContentBrowserPanel::ContentBrowserPanel()
		: m_currentDirectory(s_assetPath)
	{


	}

	void ContentBrowserPanel::OnImGuiRender()
	{

		ImGui::Begin("Content Browser");
		{
			if (m_currentDirectory != std::filesystem::path(s_assetPath))
			{

				if (ImGui::Button("<-"))
				{
					m_currentDirectory = m_currentDirectory.parent_path();
				}

			}

			for (auto& directoryEntry : std::filesystem::directory_iterator(m_currentDirectory))
			{
				const auto& path = directoryEntry.path();
				auto relativePath = std::filesystem::relative(path, s_assetPath);
				std::string filenameString = relativePath.filename().string();
				if (directoryEntry.is_directory())
				{
					if (ImGui::Button(filenameString.c_str()))
					{
						m_currentDirectory /= path.filename();

					}
				}
				else
				{
					if (ImGui::Button(filenameString.c_str()))
					{
						

					}
				}




			}

		}
		ImGui::End();

	}

}