#include <ugpch.h>
#include "SceneHierarchyPanel.h"

#include "Uge/Scripting/ScriptEngine.h"
#include "Uge/Utils/PlatformUtils.h"
#include "Uge/UI/UI.h"
#include "Uge/Asset/AssetManager.h"
#include "Uge/Project/Project.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>


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
		ImGui::Text("%s", label.c_str());
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
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, "%s", name.c_str());
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
					if constexpr (std::is_same_v<T, MeshComponent>)
					{
						// Copy the handle out first: `component` dangles once the
						// component is gone.
						AssetHandle mesh = component.Mesh;
						entity.RemoveComponent<T>();
						entity.GetScene()->ReleaseMeshIfUnused(mesh);
					}
					else
					{
						entity.RemoveComponent<T>();
					}
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

		// "%s", not the tag directly: the tag is user text, and TreeNodeEx treats its last
		// argument as a format string — an entity renamed to something containing a "%s"
		// otherwise reads a vararg that was never passed.
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, "%s", tag.c_str());

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
			DisplayAddComponentEntry<TextComponent>("Text Component");
			DisplayAddComponentEntry<SkyLightComponent>("Sky Light");
			DisplayAddComponentEntry<DirectionalLightComponent>("Directional Light");
			DisplayAddComponentEntry<RigidbodyComponent>("Rigidbody");
			DisplayAddComponentEntry<BoxColliderComponent>("Box Collider");
			DisplayAddComponentEntry<SphereColliderComponent>("Sphere Collider");
			DisplayAddComponentEntry<CapsuleColliderComponent>("Capsule Collider");
			DisplayAddComponentEntry<MeshColliderComponent>("Mesh Collider");


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
					
					std::string label = "None";
					bool isTextureValid = false;
					if (component.Texture != 0)
					{
						if (AssetManager::IsAssetHandleValid(component.Texture) && AssetManager::GetAssetType(component.Texture) == AssetType::Texture2D)
						{
							const auto& metadata = Project::GetActive()->GetEditorAssetManager()->GetMetadata(component.Texture);
							label = metadata.FilePath.filename().string();
							isTextureValid = true;
						}
						else
						{
							label = "Invalid";
						}
					}

					ImVec2 BtnLabelSize = ImGui::CalcTextSize(label.c_str());
					BtnLabelSize.x += 20.0f;
					float btnLabelWidth = glm::max<float>(100.0f, BtnLabelSize.x);


					ImGui::Button(label.c_str(), BtnLabelSize);
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
						{

							AssetHandle handle = *(AssetHandle*)payload->Data;

							// TODO: validate
							if (AssetManager::GetAssetType(handle) == AssetType::Texture2D)
							{
								component.Texture = handle;

							}
							else
							{
								UG_CORE_WARN("Wrong Asset Type!");
							}

							
						}
						ImGui::EndDragDropTarget();

					}
					if (isTextureValid)
					{

						ImGui::SameLine();

						ImVec2 xLabelSize = ImGui::CalcTextSize("X");
						float buttonSize = xLabelSize.y + ImGui::GetStyle().FramePadding.y * 2.0f;
						if (ImGui::Button("X", ImVec2(buttonSize, buttonSize)))
						{
							component.Texture = 0;
						}
					}


					ImGui::SameLine();
					ImGui::Text("Texture");

					// Tiling Factor
					if (ImGui::DragFloat("Tiling Factor", &component.TilingFactor, 0.1f, 0.0f, 100.0f))
					{

						// component.Texture->SetTilingFactor(component.TilingFactor);
					}
				});

#pragma endregion

#pragma region MeshComponent

			DrawComponent<MeshComponent>("Mesh", entity, true, [scene = m_context](auto& component)
				{
					std::string label = "None";
					bool isMeshValid = false;
					if (component.Mesh != 0)
					{
						if (AssetManager::IsAssetHandleValid(component.Mesh) && AssetManager::GetAssetType(component.Mesh) == AssetType::Mesh)
						{
							const auto& metadata = Project::GetActive()->GetEditorAssetManager()->GetMetadata(component.Mesh);
							label = metadata.FilePath.filename().string();
							isMeshValid = true;
						}
						else
						{
							label = "Invalid";
						}
					}

					ImVec2 BtnLabelSize = ImGui::CalcTextSize(label.c_str());
					BtnLabelSize.x += 20.0f;

					ImGui::Button(label.c_str(), BtnLabelSize);
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
						{
							AssetHandle handle = *(AssetHandle*)payload->Data;

							if (AssetManager::GetAssetType(handle) == AssetType::Mesh)
							{
								// Dropping onto a slot that already held a model drops
								// that model's last reference.
								AssetHandle previous = component.Mesh;
								component.Mesh = handle;
								if (previous != handle)
								{
									scene->ReleaseMeshIfUnused(previous);
								}
							}
							else
							{
								UG_CORE_WARN("Wrong Asset Type!");
							}
						}
						ImGui::EndDragDropTarget();
					}

					if (isMeshValid)
					{
						ImGui::SameLine();

						ImVec2 xLabelSize = ImGui::CalcTextSize("X");
						float buttonSize = xLabelSize.y + ImGui::GetStyle().FramePadding.y * 2.0f;
						if (ImGui::Button("X", ImVec2(buttonSize, buttonSize)))
						{
							AssetHandle mesh = component.Mesh;
							component.Mesh = 0;
							scene->ReleaseMeshIfUnused(mesh);
						}
					}

					ImGui::SameLine();
					ImGui::Text("Mesh");

					ImGui::Text("Status: %s", isMeshValid ? "Loaded" : "No model");
				});

#pragma endregion

#pragma region SkyLightComponent

			DrawComponent<SkyLightComponent>("Sky Light", entity, true, [](auto& component)
				{
					std::string label = "None";
					bool isEnvironmentValid = false;
					if (component.Environment != 0)
					{
						if (AssetManager::IsAssetHandleValid(component.Environment)
							&& AssetManager::GetAssetType(component.Environment) == AssetType::Environment)
						{
							const auto& metadata =
								Project::GetActive()->GetEditorAssetManager()->GetMetadata(component.Environment);
							label = metadata.FilePath.filename().string();
							isEnvironmentValid = true;
						}
						else
						{
							label = "Invalid";
						}
					}

					ImVec2 btnLabelSize = ImGui::CalcTextSize(label.c_str());
					btnLabelSize.x += 20.0f;

					ImGui::Button(label.c_str(), btnLabelSize);
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
						{
							AssetHandle handle = *(AssetHandle*)payload->Data;

							if (AssetManager::GetAssetType(handle) == AssetType::Environment)
							{
								component.Environment = handle;
							}
							else
							{
								UG_CORE_WARN("Wrong Asset Type!");
							}
						}
						ImGui::EndDragDropTarget();
					}

					if (isEnvironmentValid)
					{
						ImGui::SameLine();

						ImVec2 xLabelSize = ImGui::CalcTextSize("X");
						float buttonSize = xLabelSize.y + ImGui::GetStyle().FramePadding.y * 2.0f;
						if (ImGui::Button("X", ImVec2(buttonSize, buttonSize)))
						{
							component.Environment = 0;
						}
					}

					ImGui::SameLine();
					ImGui::Text("Environment");

					ImGui::DragFloat("Intensity", &component.Intensity, 0.05f, 0.0f, 20.0f);

					ImGui::Text("Status: %s", isEnvironmentValid ? "Loaded" : "No environment");
				});

#pragma endregion

#pragma region DirectionalLightComponent

			DrawComponent<DirectionalLightComponent>("Directional Light", entity, true, [](auto& component)
				{
					ImGui::ColorEdit3("Color", glm::value_ptr(component.Color));

					// Uncapped at the top: this is radiance, and the diffuse term divides albedo
					// by pi, so plausible sunlight sits well above 1.
					ImGui::DragFloat("Intensity", &component.Intensity, 0.05f, 0.0f, 100.0f);

					ImGui::TextWrapped(
						"Shines along the entity's local -Z; rotate the entity to aim it. "
						"Position is ignored.");
				});

#pragma endregion

#pragma region TextComponent

			DrawComponent<TextComponent>("Text Renderer", entity, true, [](auto& component)
				{
					ImGui::InputTextMultiline("Text String", &component.TextString);
					ImGui::DragFloat("Kerning", &component.Kerning, 0.025f, 0.0f, 100.0f);
					ImGui::DragFloat("Line Spacing", &component.LineSpacing, 0.025f, 0.0f, 100.0f);
					ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));



				});

#pragma endregion

#pragma region RigidBodyComponent

			DrawComponent<RigidbodyComponent>("Rigidbody", entity, true, [](auto& component)
			{
				const char* bodyTypeStrings[] = { "Static", "Kinematic", "Dynamic" };
				const char* current = bodyTypeStrings[(int)component.Type];

				if (ImGui::BeginCombo("Body Type", current))
				{
					for (int i = 0; i < 3; i++)
					{
						bool selected = current == bodyTypeStrings[i];
						if (ImGui::Selectable(bodyTypeStrings[i], selected))
							component.Type = (BodyType)i;
						if (selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				ImGui::DragFloat("Mass", &component.Mass, 0.1f, 0.0f, 10000.0f);
				ImGui::DragFloat("Linear Damping", &component.LinearDamping, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Angular Damping", &component.AngularDamping, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Gravity Factor", &component.GravityFactor, 0.05f, -2.0f, 2.0f);
				ImGui::Checkbox("Fixed Rotation", &component.FixedRotation);
			});
#pragma endregion

#pragma region BoxColliderComponent

			DrawComponent<BoxColliderComponent>("Box Collider", entity, true, [](auto& component)
			{

				if (DrawVec3Control("Offset", component.Offset))
				{

				}

				if (DrawVec3Control("HalfExtents", component.HalfExtents))
				{

				}

				if (ImGui::CollapsingHeader("Physics Material"))
				{
					ImGui::DragFloat("Friction", &component.Material.Friction);
					ImGui::DragFloat("Restitution", &component.Material.Restitution);
					ImGui::DragFloat("Density", &component.Material.Density);
				}

				ImGui::Checkbox("Is Trigger", &component.IsTrigger);


			});
#pragma endregion

#pragma region SphereColliderComponent

			DrawComponent<SphereColliderComponent>("Sphere Collider", entity, true, [](auto& component)
			{

				if (DrawVec3Control("Offset", component.Offset))
				{

				}

				ImGui::DragFloat("Radius", &component.Radius);

				if (ImGui::CollapsingHeader("Physics Material"))
				{
					ImGui::DragFloat("Friction", &component.Material.Friction);
					ImGui::DragFloat("Restitution", &component.Material.Restitution);
					ImGui::DragFloat("Density", &component.Material.Density);
				}

				ImGui::Checkbox("Is Trigger", &component.IsTrigger);

			});

#pragma endregion

#pragma region SphereColliderComponent

			DrawComponent<CapsuleColliderComponent>("Capsule Collider", entity, true, [](auto& component)
			{

				if (DrawVec3Control("Offset", component.Offset))
				{

				}

				ImGui::DragFloat("Radius", &component.Radius);
				ImGui::DragFloat("Half Height", &component.HalfHeight);

				if (ImGui::CollapsingHeader("Physics Material"))
				{
					ImGui::DragFloat("Friction", &component.Material.Friction);
					ImGui::DragFloat("Restitution", &component.Material.Restitution);
					ImGui::DragFloat("Density", &component.Material.Density);
				}

				ImGui::Checkbox("Is Trigger", &component.IsTrigger);

			});

#pragma endregion

#pragma region MeshColliderComponent

			DrawComponent<MeshColliderComponent>("Mesh Collider", entity, true, [](auto& component)
			{

				std::string label = "None";
				bool isMeshValid = false;
				if (component.Mesh != 0)
				{
					if (AssetManager::IsAssetHandleValid(component.Mesh)
						&& AssetManager::GetAssetType(component.Mesh) == AssetType::Mesh)
					{
						const auto& metadata =
							Project::GetActive()->GetEditorAssetManager()->GetMetadata(component.Mesh);
						label = metadata.FilePath.filename().string();
						isMeshValid = true;
					}
					else
					{
						label = "Invalid";
					}
				}

				ImVec2 btnLabelSize = ImGui::CalcTextSize(label.c_str());
				btnLabelSize.x += 20.0f;

				ImGui::Button(label.c_str(), btnLabelSize);
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						AssetHandle handle = *(AssetHandle*)payload->Data;

						if (AssetManager::GetAssetType(handle) == AssetType::Mesh)
						{
							component.Mesh = handle;
						}
						else
						{
							UG_CORE_WARN("Wrong Asset Type!");
						}
					}
					ImGui::EndDragDropTarget();
				}

				if (isMeshValid)
				{
					ImGui::SameLine();

					ImVec2 xLabelSize = ImGui::CalcTextSize("X");
					float buttonSize = xLabelSize.y + ImGui::GetStyle().FramePadding.y * 2.0f;
					if (ImGui::Button("X", ImVec2(buttonSize, buttonSize)))
					{
						component.Mesh = 0;
					}
				}

				ImGui::SameLine();
				ImGui::Text("Mesh Collider");

				ImGui::Checkbox("Is Convex", &component.Convex);


				if (ImGui::CollapsingHeader("Physics Material"))
				{
					ImGui::DragFloat("Friction", &component.Material.Friction);
					ImGui::DragFloat("Restitution", &component.Material.Restitution);
					ImGui::DragFloat("Density", &component.Material.Density);
				}

				ImGui::Checkbox("Is Trigger", &component.IsTrigger);

				

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
