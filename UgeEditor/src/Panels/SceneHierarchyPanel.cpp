#include <ugpch.h>
#include "SceneHierarchyPanel.h"

#include "Uge/Scripting/ScriptEngine.h"
#include "Uge/Utils/PlatformUtils.h"
#include "Uge/UI/UI.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <string>



namespace Uge
{

	
	static bool DrawVec3Control(const std::string& label, glm::vec3& values,
		float resetValue = 0.0f, float columnWidth = 100.0f)
	{

		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];


		bool hasChanged = false;
		ImGui::PushID(label.c_str());
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();


		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });


		float lineHeight = GImGui->Font->LegacySize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		// X
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
		{
			values.x = resetValue;
		}
		ImGui::PopFont();

		ImGui::SameLine();
		if (ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f"))
		{
			hasChanged = true;
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::PopStyleColor(3);


		// Y
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
		{
			values.y = resetValue;
		}
		ImGui::PopFont();

		ImGui::SameLine();
		if (ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f"))
		{
			hasChanged = true;
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::PopStyleColor(3);


		// Z
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
		{
			values.z = resetValue;
		}
		ImGui::PopFont();

		ImGui::SameLine();
		if (ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f"))
		{
			hasChanged = true;
		}
		ImGui::PopItemWidth();
		ImGui::PopStyleVar();
		ImGui::PopStyleColor(3);

		ImGui::Columns(1);

		ImGui::PopID();
		return hasChanged;

	}

	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, bool canBeDeleted, UIFunction uiFunction)
	{

		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen 
			| ImGuiTreeNodeFlags_Framed 
			| ImGuiTreeNodeFlags_SpanAvailWidth 
			| ImGuiTreeNodeFlags_FramePadding 
			| ImGuiTreeNodeFlags_AllowOverlap;

		if (entity.HasComponent<T>())	
		{
			auto& component = entity.GetComponent<T>();

			ImVec2 contentRegAvail = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = GImGui->Font->LegacySize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar();
			ImGui::SameLine(contentRegAvail.x - lineHeight * 0.5f);
			if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
			{
				ImGui::OpenPopup("ComponentSettings");
			}

			bool removeComponent = false;

			if (canBeDeleted)
			{

				if (ImGui::BeginPopup("ComponentSettings"))
				{

					if (ImGui::MenuItem("Remove Component"))
					{
						removeComponent = true;
					}
					ImGui::EndPopup();
				}


			}

			if (open)
			{

				uiFunction(component);
				ImGui::TreePop();

			}

			if (canBeDeleted)
			{
				if (removeComponent)
				{
					entity.RemoveComponent<T>();
				}

			}
		}


	}

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
	{
		SetContext(context);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_context = context;

		m_selectionContext = {};
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{

		ImGui::Begin("Scene Hierarchy");
		{

			if (m_context)
			{

				for (auto entityID : m_context->m_registry.view<entt::entity>())
				{
					Entity entity{ entityID, m_context.get() };
					DrawEntityNode(entity);

				}

				if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
				{
					m_selectionContext = {};
				}


				if (ImGui::BeginPopupContextWindow(0, 
						ImGuiPopupFlags_NoOpenOverItems |
						ImGuiPopupFlags_MouseButtonRight |
						ImGuiPopupFlags_NoOpenOverExistingPopup))
				{

					if (ImGui::MenuItem("Create Empty Entity"))
					{
						m_context->CreateEntity("Empty Entity");
					}
					ImGui::EndPopup();

				}


			}


		}
		ImGui::End();

		ImGui::Begin("Properties");
		{

			if (m_selectionContext)
			{

				DrawComponents(m_selectionContext);

			}

		}
		ImGui::End();

	}

	void SceneHierarchyPanel::SetSelectedEntity(Entity entity)
	{

		m_selectionContext = entity;


	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{

		auto& tag = entity.GetComponent<TagComponent>().Tag;
		

		ImGuiTreeNodeFlags flags = ((m_selectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());

		if (ImGui::IsItemClicked())
		{

			m_selectionContext = entity;

		}
		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{

			if (ImGui::MenuItem("Delete Entity"))
			{
				entityDeleted = true;
			}
			ImGui::EndPopup();

		}

		if (opened)
		{
			
			ImGui::TreePop();
		}

		if (entityDeleted)
		{
			m_context->DestroyEntity(entity);
			if (m_selectionContext == entity)
			{
				m_selectionContext = {};
			}
		}

	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{

#pragma region TagComponent

		if (entity.HasComponent<TagComponent>())
		{

			auto& tag = entity.GetComponent<TagComponent>().Tag;



			char buffer[256];
			memset(buffer ,0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), tag.c_str());

			if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) 
			{
				
				tag = std::string(buffer);

			}

		}


#pragma endregion

#pragma region Add Component

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);
		if (ImGui::Button("Add Component"))
		{
			ImGui::OpenPopup("AddComponent");
		}
		if (ImGui::BeginPopup("AddComponent"))
		{

			DisplayAddComponentEntry<CameraComponent>("Camera");
			DisplayAddComponentEntry<ScriptComponent>("Script");
			DisplayAddComponentEntry<SpriteRendererComponent>("Sprite Renderer");
			DisplayAddComponentEntry<MeshComponent>("Mesh");


			ImGui::EndPopup();
		}
		ImGui::PopItemWidth();

		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap;


#pragma endregion

#pragma region TransformComponent

		DrawComponent<TransformComponent>("Transform", entity, false, [](auto& component)
			{
				

				if (DrawVec3Control("Translation", component.Translation))
				{

				}

				glm::vec3 rotation = glm::degrees(component.Rotation);
				if (DrawVec3Control("Rotation", rotation))
				{
					component.Rotation = glm::radians(rotation);
				}
				if (DrawVec3Control("Scale", component.Scale, 1.0f))
				{

				}

			});


#pragma endregion

#pragma region CameraComponent

		DrawComponent<CameraComponent>("Camera Component", entity, true, [](auto& component)
			{

				auto& camera = component.Cam;


				ImGui::Checkbox("Primary", &component.Primary);

				const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
				const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];
				if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
				{

					for (int i = 0; i < 2; i++)
					{

						bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];

						if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
						{
							currentProjectionTypeString = projectionTypeStrings[i];
							camera.SetProjectionType((SceneCamera::ProjectionType)i);
						}

						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}

					}
					ImGui::EndCombo();
				}

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
				{

					float perspFovy = camera.GetPerspVerticalFOV();
					float perspNear = camera.GetPerspNearClip();
					float perspFar = camera.GetPerspFarClip();



					if (ImGui::DragFloat("Vertical FOV", &perspFovy))
					{
						camera.SetPerspVerticalFOV(perspFovy);
					}
					if (ImGui::DragFloat("Near Clip", &perspNear))
					{
						camera.SetPerspNearClip(perspNear);
					}
					if (ImGui::DragFloat("Far Clip", &perspFar))
					{
						camera.SetPerspFarClip(perspFar);
					}


				}

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
				{
					float orthoSize = camera.GetOrthoSize();
					float orthoNear = camera.GetOrthoNearClip();
					float orthoFar = camera.GetOrthoFarClip();



					if (ImGui::DragFloat("Size", &orthoSize))
					{
						camera.SetOrthoSize(orthoSize);
					}
					if (ImGui::DragFloat("Near Clip", &orthoNear))
					{
						camera.SetOrthoNearClip(orthoNear);
					}
					if (ImGui::DragFloat("Far Clip", &orthoFar))
					{
						camera.SetOrthoFarClip(orthoFar);
					}

					ImGui::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio);



				}

			});

#pragma endregion

#pragma region ScriptComponent

		DrawComponent<ScriptComponent>("Script", entity, true, [entity, scene = m_context](auto& component) mutable
			{

				bool scriptClassExists = ScriptEngine::EntityClassExists(component.ClassName);

				static char buffer[64];
				strcpy_s(buffer, sizeof(buffer), component.ClassName.c_str());

				UI::ScopedStyleColor textColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.3f, 1.0f), !scriptClassExists);


				if (ImGui::InputText("Class", buffer, sizeof(buffer)))
				{
					component.ClassName = buffer;
				
					return;

				}


				// Fields

				bool isSceneRunning = scene->IsRunning();
				// If Scene Running
				if (isSceneRunning)
				{
					Ref<ScriptInstance> scriptInstance = ScriptEngine::GetEntityScriptInstance(entity.GetUUID());
					if (scriptInstance)
					{
						const auto& fields = scriptInstance->GetScriptClass()->GetFields();

						for (const auto& [name, field] : fields)
						{

							if (field.Type == ScriptFieldType::Float)
							{

								float data = scriptInstance->GetFieldValue<float>(name);
								if (ImGui::DragFloat(name.c_str(), &data, 0.1f))
								{

									scriptInstance->SetFieldValue(name, data);

								}

							}

						}
					}


				}
				else
				{

					if (scriptClassExists)
					{
						Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(component.ClassName);
						const auto& fields = entityClass->GetFields();

						auto& entityFields = ScriptEngine::GetScriptFieldMap(entity.GetUUID());


						for (const auto& [name, field] : fields)
						{
							// Field has been set in editor
							if (entityFields.find(name) != entityFields.end())
							{
								ScriptFieldInstance& scriptField = entityFields[name];
								if (field.Type == ScriptFieldType::Float)
								{

									float data = scriptField.GetValue<float>();
									if (ImGui::DragFloat(name.c_str(), &data, 0.1f))
									{
										scriptField.SetValue<float>(data);
									}

								}
							}
							else
							{

								if (field.Type == ScriptFieldType::Float)
								{

									float data = 0.0f;
									if (ImGui::DragFloat(name.c_str(), &data, 0.1f))
									{
										ScriptFieldInstance& fieldInstance = entityFields[name];
										fieldInstance.Field = field;
										fieldInstance.SetValue<float>(data);
									}

								}
							}



						}
					}

				}

			});

#pragma endregion

#pragma region SpriteRendererComponent

			DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, true, [](auto& component)
				{
					// Color
					if (ImGui::ColorEdit4("Color", glm::value_ptr(component.Color)))
					{

					}

					// Texture

					ImGui::Button("Texture", ImVec2(100.0f, 0.0f));


					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
						{
							const wchar_t* path = (const wchar_t*)payload->Data;
							std::filesystem::path texturePath(path);
							component.Texture = Texture2D::Create(texturePath.string());


						}
						ImGui::EndDragDropTarget();
					}

					// Tiling Factor
					if (ImGui::DragFloat("Tiling Factor", &component.TilingFactor, 0.1f, 0.0f, 100.0f))
					{

						component.Texture->SetTilingFactor(component.TilingFactor);
					}
				});

#pragma endregion

#pragma region MeshComponent

			DrawComponent<MeshComponent>("Mesh", entity, true, [](auto& component)
				{
					char buffer[512];
					memset(buffer, 0, sizeof(buffer));
					strcpy_s(buffer, sizeof(buffer), component.FilePath.c_str());

					ImGui::InputText("Path", buffer, sizeof(buffer));
					component.FilePath = buffer;

					if (ImGui::Button("Load Mesh"))
					{
						std::string filePath = FileDialogs::OpenFile("");

						std::filesystem::path absPath(filePath);
						std::filesystem::path baseDir = Project::GetAssetAbsolutePath();

						std::filesystem::path relativePath = std::filesystem::relative(absPath, baseDir);



						std::string path = relativePath.string();



						strcpy_s(buffer, sizeof(buffer), path.c_str());
						component.SetModel(std::string(buffer));
					}
					ImGui::SameLine();
					if (ImGui::Button("Clear Mesh"))
					{
						component.SetModel(std::string());
					}

					ImGui::Text("Status: %s", component.HasModel() ? "Loaded" : "No model");
				});

#pragma endregion


	}

	template<typename T>
	void SceneHierarchyPanel::DisplayAddComponentEntry(const std::string& entryName) {
		if (!m_selectionContext.HasComponent<T>())
		{
			if (ImGui::MenuItem(entryName.c_str()))
			{
				m_selectionContext.AddComponent<T>();
				ImGui::CloseCurrentPopup();
			}
		}
	}

}
