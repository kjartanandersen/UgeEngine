#include "SceneHierarchyPanel.h"

#include <imgui.h>

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

}