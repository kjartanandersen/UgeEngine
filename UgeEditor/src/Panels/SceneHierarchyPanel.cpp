#include "SceneHierarchyPanel.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <glm/gtc/type_ptr.hpp>


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


		ImGui::SameLine();
		ImGui::PushItemWidth(-1);
		if (ImGui::Button("Add Component"))
		{
			ImGui::OpenPopup("AddComponent");
		}
		if (ImGui::BeginPopup("AddComponent"))
		{

			if (ImGui::MenuItem("Camera"))
			{
				auto& camComp = m_selectionContext.AddComponent<CameraComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Sprite Renderer"))
			{
				m_selectionContext.AddComponent<SpriteRendererComponent>();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
		ImGui::PopItemWidth();

		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap;

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

		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, true, [](auto& component) 
		{
			if (ImGui::ColorEdit4("Color", glm::value_ptr(component.Color)))
			{

			}

		});

		

	}

}