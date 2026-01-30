#include "SceneHierarchyPanel.h"

#include <imgui.h>

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

				auto& transform = entity.GetComponent<TransformComponent>().Transform;

				if (ImGui::DragFloat3("Position", glm::value_ptr(transform[3]), 0.1f))
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



	}

}