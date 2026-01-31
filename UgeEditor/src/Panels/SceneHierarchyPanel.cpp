#include "SceneHierarchyPanel.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <glm/gtc/type_ptr.hpp>


namespace Uge
{

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
	{
		SetContext(context);


	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_context = context;


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

		bool openedBase = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());

		if (ImGui::IsItemClicked())
		{

			m_selectionContext = entity;

		}

		if (openedBase)
		{
			ImGuiTreeNodeFlags flags = ((m_selectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;

			bool opened = ImGui::TreeNodeEx((void*)9817239, flags, tag.c_str());

			if (opened)
			{
				
				ImGui::TreePop();
			}

			ImGui::TreePop();
		}



	}

	static bool DrawVec3Control(const std::string& label, glm::vec3& values, 
		float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		bool hasChanged = false;
		ImGui::PushID(label.c_str());
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();


		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		
		float lineHeight = GImGui->Font->LegacySize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight +3.0f, lineHeight };

		// X
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		if (ImGui::Button("X", buttonSize))
		{
			values.x = resetValue;
		}

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
		if (ImGui::Button("Y", buttonSize))
		{
			values.y = resetValue;
		}

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
		if (ImGui::Button("Z", buttonSize))
		{
			values.z = resetValue;
		}

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


	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		if (entity.HasComponent<TagComponent>())
		{

			auto& tag = entity.GetComponent<TagComponent>().Tag;



			char buffer[256];
			memset(buffer ,0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), tag.c_str());

			if (ImGui::InputText("Tag", buffer, sizeof(buffer))) 
			{
				
				tag = std::string(buffer);

			}

		}

		if (entity.HasComponent<TransformComponent>())
		{
			if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Transform"))
			{

				auto& tc = entity.GetComponent<TransformComponent>();

				if (DrawVec3Control("Translation", tc.Translation))
				{

				}

				glm::vec3 rotation = glm::degrees(tc.Rotation);
				if (DrawVec3Control("Rotation", rotation))
				{
					tc.Rotation = glm::radians(rotation);
				}
				if (DrawVec3Control("Scale", tc.Scale, 1.0f))
				{

				}
				ImGui::TreePop();

			}
		}

		if (entity.HasComponent<CameraComponent>())
		{
			if (ImGui::TreeNodeEx((void*)typeid(CameraComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Camera"))
			{
				auto& camComponent = entity.GetComponent<CameraComponent>();
				auto& camera = camComponent.Cam;


				ImGui::Checkbox("Primary", &camComponent.Primary);
				
				const char* projectionTypeStrings[] = {"Perspective", "Orthographic"};
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

					ImGui::Checkbox("Fixed Aspect Ratio", &camComponent.FixedAspectRatio);



				}


				ImGui::TreePop();
			}
		}

		if (entity.HasComponent<SpriteRendererComponent>())
		{
			if (ImGui::TreeNodeEx((void*)typeid(SpriteRendererComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Sprite Renderer"))
			{

				auto& src = entity.GetComponent<SpriteRendererComponent>();

				if (ImGui::ColorEdit4("Color", glm::value_ptr(src.Color)))
				{



				}
				ImGui::TreePop();

			}
		}

	}

}