#include <ugpch.h>
#include "ContentBrowserPanel.h"

#include "Uge/Project/Project.h"

#include "Uge/Asset/TextureImporter.h"

#include <imgui.h>


namespace Uge
{

	ContentBrowserPanel::ContentBrowserPanel()
		//: m_baseDirectory(Project::GetAssetDirectory()), m_currentDirectory(m_baseDirectory)
	{
		m_baseDirectory = Project::GetAssetDirectory();
		m_currentDirectory = m_baseDirectory;
		m_directoryIcon = TextureImporter::LoadTexture2D("Resources/Icons/ContentBrowser/DirectoryIcon.png");
		m_fileIcon = TextureImporter::LoadTexture2D("Resources/Icons/ContentBrowser/FileIcon.png");
		m_treeNodes.push_back(TreeNode("."));
		RefreshAssetTree();
		m_mode = Mode::Asset;

	}

	void ContentBrowserPanel::OnImGuiRender()
	{

		ImGui::Begin("Content Browser");
		{
			const char* label = m_mode == Mode::Asset ? "Asset" : "File";

			if (ImGui::Button(label))
			{
				m_mode = m_mode == Mode::Asset ? Mode::FileSystem : Mode::Asset;
			}
			
			if (m_currentDirectory != std::filesystem::path(m_baseDirectory))
			{

				ImGui::SameLine();
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

			if (m_mode == Mode::Asset)
			{

				TreeNode* node = &m_treeNodes[0];

				auto currentDir = std::filesystem::relative(m_currentDirectory, Project::GetAssetDirectory());

				for (const auto& p : currentDir)
				{

					if (node->Path == currentDir)
					{
						break;
					}

					if (node->Children.find(p) != node->Children.end())
					{

						node = &m_treeNodes[node->Children[p]];
						continue;
					}
					else
					{
						UG_CORE_ASSERT(false);

					}


				}

				for (const auto& [item, treeNodeIndex] : node->Children)
				{

					std::string itemStr = item.generic_string();
					bool isDirectory = std::filesystem::is_directory(Project::GetAssetDirectory() / itemStr);


					ImGui::PushID(itemStr.c_str());
					{

						Ref<Texture2D> icon = isDirectory ? m_directoryIcon : m_fileIcon;
						ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
						{
							ImGui::ImageButton("CBP_IMG", (ImTextureID)icon->GetRendererID(),
								{ thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });

							if (ImGui::BeginPopupContextItem())
							{
								if (ImGui::MenuItem("Import"))
								{
									auto relPath = std::filesystem::relative(itemStr, Project::GetAssetDirectory());

									Project::GetActive()->GetEditorAssetManager()->ImportAsset(relPath);
								}

								ImGui::EndPopup();
							}

						}
						ImGui::PopStyleColor();

						if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
						{

							if (isDirectory)
							{
								m_currentDirectory /= item.filename();
							}


						}



						ImGui::TextWrapped(itemStr.c_str());

						ImGui::NextColumn();

					}
					ImGui::PopID();

				}

			}
			else
			{

				for (auto& directoryEntry : std::filesystem::directory_iterator(m_currentDirectory))
				{
					const auto& path = directoryEntry.path();
					std::filesystem::path relativePath(path);
					std::string filenameString = relativePath.filename().string();
					ImGui::PushID(filenameString.c_str());
					{

						Ref<Texture2D> icon = directoryEntry.is_directory() ? m_directoryIcon : m_fileIcon;
						ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
						{
							ImGui::ImageButton("CBP_IMG", (ImTextureID)icon->GetRendererID(),
								{ thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });

							if (ImGui::BeginPopupContextItem())
							{
								if (ImGui::MenuItem("Import"))
								{
									auto relPath = std::filesystem::relative(path, Project::GetAssetDirectory());

									Project::GetActive()->GetEditorAssetManager()->ImportAsset(relPath);
								}

								ImGui::EndPopup();
							}

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

					}
					ImGui::PopID();

				}

			}

			
			

		}
		ImGui::End();

	}

	void ContentBrowserPanel::RefreshAssetTree()
	{

		const auto& assetReg = Project::GetActive()->GetEditorAssetManager()->GetAssetRegistry();

		for (const auto&[handle, metadata] : assetReg)
		{
			uint32_t currentNodeIdx = 0;

			for (const auto& p : metadata.FilePath)
			{
				auto it = m_treeNodes[currentNodeIdx].Children.find(p.generic_string());
				if (it != m_treeNodes[currentNodeIdx].Children.end())
				{
					currentNodeIdx = it->second;

				}
				else
				{
					TreeNode newNode(p);
					newNode.Parent = currentNodeIdx;
					m_treeNodes.push_back(newNode);

					m_treeNodes[currentNodeIdx].Children[p] = m_treeNodes.size() - 1;
					currentNodeIdx = m_treeNodes.size() - 1;

				}
			}

		}

	}

}