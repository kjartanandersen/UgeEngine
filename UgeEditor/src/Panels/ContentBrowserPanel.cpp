#include <ugpch.h>
#include "ContentBrowserPanel.h"

#include <imgui.h>


namespace Uge
{

	// TODO: To be changed later
	const std::filesystem::path g_assetPath = "assets";



	ContentBrowserPanel::ContentBrowserPanel()
		: m_currentDirectory(g_assetPath)
	{
		m_directoryIcon = Texture2D::Create("Resources/Icons/ContentBrowser/DirectoryIcon.png");
		m_fileIcon = Texture2D::Create("Resources/Icons/ContentBrowser/FileIcon.png");

	}

	void ContentBrowserPanel::OnImGuiRender()
	{

		ImGui::Begin("Content Browser");
		{
			if (m_currentDirectory != std::filesystem::path(g_assetPath))
			{

				if (ImGui::Button("<-"))
				{
					m_currentDirectory = m_currentDirectory.parent_path();
				}

			}

			static float padding = 16.0f;
			static float thumbnailSize = 128.0f;
			float cellSize = thumbnailSize + padding;

			float panelWidth = ImGui::GetContentRegionAvail().x;
			int columnCount = (int)(panelWidth / cellSize);
			if (columnCount < 1)
				columnCount = 1;

			ImGui::Columns(columnCount, 0, false);

			
			for (auto& directoryEntry : std::filesystem::directory_iterator(m_currentDirectory))
			{
				const auto& path = directoryEntry.path();
				auto relativePath = std::filesystem::relative(path, g_assetPath);
				std::string filenameString = relativePath.filename().string();
				ImGui::PushID(filenameString.c_str());
				{

					Ref<Texture2D> icon = directoryEntry.is_directory() ? m_directoryIcon : m_fileIcon;
					ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
					{
						ImGui::ImageButton("CBP_IMG", (ImTextureID)icon->GetRendererID(), 
							{thumbnailSize, thumbnailSize}, {0, 1}, {1, 0});
					
						if (ImGui::BeginDragDropSource())
						{
							const wchar_t* itemPath = relativePath.c_str();

							ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, 
								(wcslen(itemPath) + 1) * sizeof(wchar_t), ImGuiCond_Once);

							ImGui::EndDragDropSource();
						}
					}
					ImGui::PopStyleColor();

					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						if (directoryEntry.is_directory())
						{
							m_currentDirectory /= path.filename();
						}


					}
					ImGui::TextWrapped(filenameString.c_str());

					ImGui::NextColumn();
					ImGui::PopID();

				}

			}

		}
		ImGui::End();

	}

}