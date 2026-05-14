#include "EditorLayer.h"

#include "Uge/Scripting/ScriptEngine.h"
#include "Uge/Renderer/Font.h"

#include "imgui.h"
#include <cstdint>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

#include <ImGuizmo.h>


namespace Uge
{



	static Ref<Font> s_font;

	EditorLayer::EditorLayer()
		: Layer("Sandbox2D")
	{
		s_font = Font::GetDefault();
	}

	void EditorLayer::OnUpdate(Timestep ts)
	{

		UG_PROFILE_FUNCTION();

		m_activeScene->OnViewportResize((uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y);
		if (m_shouldResize)
		{
			m_frameBuffer->Resize((uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y);


			m_editorCamera.SetViewportSize(m_viewportSize.x, m_viewportSize.y);

			m_shouldResize = false;
		}

		if (m_viewportFocused)
		{
			// Update	

		}

		// Render
		m_frameBuffer->Bind();
		{
			{
				UG_PROFILE_SCOPE("Renderer Prep")
				RenderCommand::SetClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1));
				RenderCommand::Clear();
			}
			m_frameBuffer->ClearAttachment(1, -1);

			switch (m_sceneState)
			{
			case Uge::EditorLayer::SceneState::Edit:
				m_editorCamera.OnUpdate(ts);
				m_activeScene->OnUpdateEditor(ts, m_editorCamera);
				
				break;
			case Uge::EditorLayer::SceneState::Play:
				m_activeScene->OnUpdateRuntime(ts);

				break;
			default:
				
				break;
			}

		
			auto [mx, my] = ImGui::GetMousePos();
			mx -= m_viewportBounds[0].x;
			my -= m_viewportBounds[0].y;
			glm::vec2 viewportSize = m_viewportBounds[1] - m_viewportBounds[0];
			my = viewportSize.y - my;

			int mouseX = (int)mx;
			int mouseY = (int)my;

			const auto& fbSpec = m_frameBuffer->GetSpecification();
			if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)fbSpec.Width && mouseY < (int)fbSpec.Height)
			{
				int pixelData = m_frameBuffer->ReadPixel(1, mouseX, mouseY);
				Entity hoveredEntity =
					pixelData == -1
					? Entity()
					: Entity((entt::entity)pixelData, m_activeScene.get());

				m_hoveredEntity = hoveredEntity ? hoveredEntity : Entity();
			}
			else
			{
				m_hoveredEntity = Entity();
			}

		}
		m_frameBuffer->Unbind();

	}

	void EditorLayer::OnAttach()
	{
		UG_PROFILE_FUNCTION();
		

		m_iconPlay = Texture2D::Create("Resources/Icons/PlayButton.png");
		m_iconStop = Texture2D::Create("Resources/Icons/StopButton.png");
		m_iconPause = Texture2D::Create("Resources/Icons/PauseButton.png");
		m_iconStep = Texture2D::Create("Resources/Icons/StepButton.png");
		

		m_activeScene = CreateRef<Scene>();
		
		// Load ImGui Font
		ImGuiIO& io = ImGui::GetIO();
		m_mainFontBold = io.Fonts->AddFontFromFileTTF("C:\\Programming\\c++\\GameEngines\\Uge\\UgeEditor\\assets\\fonts\\Roboto-Regular\\static\\Roboto-Bold.ttf", 24.5f);
		m_mainFont = io.Fonts->AddFontFromFileTTF("C:\\Programming\\c++\\GameEngines\\Uge\\UgeEditor\\assets\\fonts\\Roboto-Regular\\static\\Roboto-Regular.ttf", 24.5f);

		IM_ASSERT(m_mainFont != NULL);
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		
		FramebufferSpecification fbSpec{ 1280, 720 };
		fbSpec.Attachments = { FramebufferTextureFormat::RGBA8,FramebufferTextureFormat::RED_INTEGER,  FramebufferTextureFormat::Depth };
		m_frameBuffer = Framebuffer::Create(fbSpec);

		m_editorScene = CreateRef<Scene>();
		m_activeScene = m_editorScene;



		auto appSpec = Application::Get().GetSpecifications();


		if (appSpec.CommandLineArgs.Count > 1)
		{
			auto projectFilePath = appSpec.CommandLineArgs[1];
			OpenProject(projectFilePath);
		}
		else
		{

			// TODO: prompt the user to select a directory

			if (!OpenProject())
			{
				Application::Get().CloseProgram();
			}
			// NewProject();

		}

		// std::filesystem::path checkPath = Project::GetAssetFileSystemPath("Textures/Checkerboard.png");
		// m_texture = Texture2D::Create(checkPath.string());


		m_editorCamera = EditorCamera(60.0f, 16.0f/9.0f, 0.01f, 10000.0f);



		m_sceneHierarchyPanel.SetContext(m_activeScene);

		

	}

	void EditorLayer::OnDetach()
	{


		UG_PROFILE_FUNCTION();

	}

	void EditorLayer::OnImGuiRender()
	{
		UG_PROFILE_FUNCTION();
		
#pragma region DockspacePrep


		/*****************************
		*
		* Dockspace
		*
		*****************************/

		ImGui::PushFont(m_mainFont);

		// TL;DR; this demo is more complicated than what most users you would normally use.
	// If we remove all options we are showcasing, this demo would become a simple call to ImGui::DockSpaceOverViewport() !!
	// In this specific demo, we are not using DockSpaceOverViewport() because:

		static bool isOpen = true;
		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		if (!opt_padding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
#pragma endregion

		ImGui::Begin("DockSpace Demo", &isOpen, window_flags);
		{

			if (!opt_padding)
				ImGui::PopStyleVar();

			if (opt_fullscreen)
				ImGui::PopStyleVar(2);

			// Submit the DockSpace
			// REMINDER: THIS IS A DEMO FOR ADVANCED USAGE OF DockSpace()!
			// MOST REGULAR APPLICATIONS WILL SIMPLY WANT TO CALL DockSpaceOverViewport(). READ COMMENTS ABOVE.
			ImGuiIO& io = ImGui::GetIO();
			ImGuiStyle& style = ImGui::GetStyle();
			float minWidthSize = style.WindowMinSize.x;
			style.WindowMinSize.x = 370.0f;
			if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
			{
				ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
				ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
			}

			style.WindowMinSize.x = minWidthSize;

			// Show demo options and help
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{

					if (ImGui::MenuItem("Open Project", "Ctrl+P"))
					{
						OpenProject();

					}
					ImGui::Separator();
					if (ImGui::MenuItem("New Scene", "Ctrl+N"))
					{
						NewScene();

					}

					if (ImGui::MenuItem("Open Scene...", "Ctrl+O"))
					{
						OpenScene();

					}

					if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
					{
						SaveScene();

					}

					if (ImGui::MenuItem("Save Scene As", "Ctrl+Shift+S"))
					{
						SaveSceneAs();

					}

					if (ImGui::MenuItem("Exit"))
					{
						Application::Get().CloseProgram();

					}
					ImGui::Separator();

					ImGui::EndMenu();
				}
				//ImGui::EndMenuBar();

				if (ImGui::BeginMenu("Script"))
				{
					if (ImGui::MenuItem("Reload assembly", "Ctrl+R"))
					{
						ScriptEngine::ReloadAssembly();

					}

					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}

			m_sceneHierarchyPanel.OnImGuiRender();
			m_contentBrowserPanel->OnImGuiRender();


			ImGui::Begin("Stats");
			{
				std::string name = "None";
				if (m_hoveredEntity)
				{
					name = m_hoveredEntity.GetComponent<TagComponent>().Tag;
				}
				ImGui::Text("Hovered Entity: %s", name.c_str());

				//ImGui::Dummy({ 0.0f, 100.0f });
				ImGui::Separator();
				ImGui::Text("Viewport Panel Size");
				ImGui::Text("X: %f", m_viewportSize.x);
				ImGui::Text("Y: %f", m_viewportSize.y);

				ImGui::Image((ImTextureID)Font::GetDefault()->GetAtlasTexture()->GetRendererID(), {512, 512}, {0, 1}, {1, 0});
			}
			ImGui::End();

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});

			ImGui::Begin("Scene Viewport");
			{
				//ImVec2 viewportOffset = ImGui::GetCursorPos();			// Includes Tab bar

				auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
				auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
				auto viewportOffset = ImGui::GetWindowPos();
				m_viewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
				m_viewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

				m_viewportFocused = ImGui::IsWindowFocused();
				m_viewportHovered = ImGui::IsWindowHovered();

				
				Application::Get().GetImGuiLayer()->BlockEvents(!m_viewportHovered);
				

				ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

					

				if (m_viewportSize != *((glm::vec2*)&viewportPanelSize))
				{

					m_viewportSize = {viewportPanelSize.x, viewportPanelSize.y};
					
					m_shouldResize = true;
				}


				uint32_t textureID = m_frameBuffer->GetColorAttachment();
				ImGui::Image((void*)(uintptr_t)textureID, ImVec2{ m_viewportSize.x, m_viewportSize.y }, { 0, 1 }, { 1, 0 });

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						const wchar_t* path = (const wchar_t*)payload->Data;
						OpenScene(path);

					}
					


					ImGui::EndDragDropTarget();
				}

				// Gizmos
				Entity selectedEntity = m_sceneHierarchyPanel.GetSelectedEntity();
				if (selectedEntity && m_gizmoType != -1 && m_sceneState == SceneState::Edit)
				{

					ImGuizmo::SetOrthographic(false);
					ImGuizmo::SetDrawlist();
					// ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, 
					// 	ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

					ImGuizmo::SetRect(m_viewportBounds[0].x, m_viewportBounds[0].y, 
						m_viewportBounds[1].x - m_viewportBounds[0].x, 
						m_viewportBounds[1].y - m_viewportBounds[0].y);

					// Camera
					
					// Runtime camera from entity
					//auto cameraEntity = m_activeScene->GetPrimaryCameraEntity();
					//const auto& camera = cameraEntity.GetComponent<CameraComponent>().Cam;
					//const glm::mat4& camProj = camera.GetProjection();
					//glm::mat4 cameraView = glm::inverse(
					//	cameraEntity.GetComponent<TransformComponent>().GetTransform());

					// Editor Camera

					const glm::mat4& camProj = m_editorCamera.GetProjection();
					glm::mat4 cameraView = m_editorCamera.GetViewMatrix();



					// Entity
					auto& tc = selectedEntity.GetComponent<TransformComponent>();
					glm::mat4 transform = tc.GetTransform();

					// Snapping
					bool snap = Input::IsKeyPressed(KeyCode::UG_KEY_LEFT_CONTROL);
					float snapVal = 0.5f;		// Snap to 0.5 meters for translation and scale

					if (m_gizmoType == ImGuizmo::OPERATION::ROTATE)
					{
						
						snapVal = 45.0f;		// Snap to 45 degrees for rotation
					}

					float snapValues[3] = { snapVal, snapVal, snapVal };




					ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(camProj),
						(ImGuizmo::OPERATION)m_gizmoType, ImGuizmo::MODE::LOCAL, glm::value_ptr(transform)
					, nullptr, snap ? snapValues : nullptr);

					if (ImGuizmo::IsUsing())
					{
						glm::vec3 transl, rot, scale;
						Math::DecomposeTransform(transform, transl, rot, scale);
						glm::vec3 deltaRot = rot - tc.Rotation;

						tc.Translation = transl;
						tc.Rotation += deltaRot;
						tc.Scale = scale;

					}


				}
			}
			ImGui::End();
			ImGui::PopStyleVar();

		}
		UI_Toolbar();
		ImGui::End();

		
		ImGui::PopFont();

	}

	void EditorLayer::UI_Toolbar()
	{

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		auto& colors = ImGui::GetStyle().Colors;
		const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
		const auto& buttonActive = colors[ImGuiCol_ButtonActive];
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));


		ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		{

			float size = ImGui::GetWindowHeight() - 16.0f;

			{
				Ref<Texture2D> icon = m_sceneState == SceneState::Play ? m_iconStop : m_iconPlay;
				ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
				if (ImGui::ImageButton("SceneState", (ImTextureID)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1)))
				{
					if (m_sceneState == SceneState::Edit)
					{
						OnScenePlay();
					}
					else if (m_sceneState == SceneState::Play)
					{
						OnSceneStop();
					}
				}

			}
			if (m_sceneState != SceneState::Edit)
			{
				bool isPaused = m_activeScene->IsPaused();
				ImGui::SameLine();
				{
					Ref<Texture2D> icon = m_iconPause;
					if (ImGui::ImageButton("PauseBtn", (ImTextureID)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1)))
					{
						m_activeScene->SetPaused(!isPaused);
					}

				}

				if (isPaused)
				{

					ImGui::SameLine();
					{
						Ref<Texture2D> icon = m_iconStep;
						if (ImGui::ImageButton("StepBtn", (ImTextureID)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1)))
						{
							m_activeScene->Step(1);
						}

					}

				}

			}

			
		}
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
		ImGui::End();

	}

	

	void EditorLayer::OnEvent(Event& e)
	{

		m_editorCamera.OnEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(UG_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(UG_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));


	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		// Shortcuts
		if (e.GetRepeatCount() > 0)
		{
			return false;
		}

		bool ctrlPressed = (Input::IsKeyPressed(KeyCode::UG_KEY_LEFT_CONTROL) || Input::IsKeyPressed(KeyCode::UG_KEY_RIGHT_CONTROL));
		bool shift = (Input::IsKeyPressed(KeyCode::UG_KEY_LEFT_SHIFT) || Input::IsKeyPressed(KeyCode::UG_KEY_RIGHT_SHIFT));

		if (!m_editorCamera.IsBeingRotated())
		{
			switch (e.GetKeyCode())
			{
				case KeyCode::UG_KEY_S:
				{
			

					if (ctrlPressed && shift)
					{
						SaveSceneAs();
					}
					else if (ctrlPressed)
					{
						SaveScene();
					}

			
					break;
				}
				case KeyCode::UG_KEY_N:
				{


					if (ctrlPressed)
					{
						NewScene();
					}


					break;
				}
				case KeyCode::UG_KEY_O:
				{


					if (ctrlPressed)
					{
						OpenScene();
					}


					break;
				}
				case KeyCode::UG_KEY_P:
				{


					if (ctrlPressed)
					{
						OpenProject();
					}


					break;
				}

				case KeyCode::UG_KEY_Q:
				{
					m_gizmoType = -1;
					break;
				}

				case KeyCode::UG_KEY_W:
				{
					m_gizmoType = ImGuizmo::OPERATION::TRANSLATE;
					break;
				}

				case KeyCode::UG_KEY_E:
				{
					m_gizmoType = ImGuizmo::OPERATION::ROTATE;
					break;
				}

				case KeyCode::UG_KEY_R:
				{
					if (ctrlPressed)
					{
						ScriptEngine::ReloadAssembly();
					}
					else
					{
						m_gizmoType = ImGuizmo::OPERATION::SCALE;

					}
					break;

				}

				case KeyCode::UG_KEY_T:
				{
					m_gizmoType = ImGuizmo::OPERATION::UNIVERSAL;
					break;
				}

				case KeyCode::UG_KEY_DELETE:
				{
					if (Application::Get().GetImGuiLayer()->GetActiveWidgetID() == 0)
					{
						Entity selectedEnt = m_sceneHierarchyPanel.GetSelectedEntity();
						if (selectedEnt)
						{
							m_sceneHierarchyPanel.SetSelectedEntity({});
							m_activeScene->DestroyEntity(selectedEnt);
						}


					}
					break;
				}

				case KeyCode::UG_KEY_D:
				{
					if (ctrlPressed)
					{
						OnDuplicateEntry();
					}

					break;
				}

				default:
					break;
			}

		}


		return false;
	}

	bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{

		if (e.GetMouseButton() == MouseButton::UG_MOUSE_BUTTON_LEFT)
		{
			if (m_viewportHovered && !ImGuizmo::IsOver() && !Input::IsKeyPressed(KeyCode::UG_KEY_LEFT_ALT))
			{
				m_sceneHierarchyPanel.SetSelectedEntity(m_hoveredEntity);

			}

		}

		return false;
	}

	void EditorLayer::SaveProject()
	{

		// Project::SaveActive();
	}

	void EditorLayer::NewProject()
	{
		Project::New();

	}

	bool EditorLayer::OpenProject()
	{

		std::string filepath = FileDialogs::OpenFile("Uge Project (*.ugproj)\0*.ugproj\0");


		if (filepath.empty())
		{
			return false;
		}
		else
		{
			OpenProject(filepath);
			return true;
		}

	}

	void EditorLayer::OpenProject(const std::filesystem::path& path)
	{
		if (Project::Load(path))
		{

			ScriptEngine::Init();

			auto startScenePath = Project::GetAssetFileSystemPath(Project::GetActive()->GetConfig().StartScene);
			OpenScene(startScenePath);
			m_contentBrowserPanel = CreateScope<ContentBrowserPanel>();



		}

	}

	void EditorLayer::SaveScene()
	{

		if (!m_editorScenePath.empty())
		{
			SerializeScene(m_activeScene, m_editorScenePath);
		}
		else
		{
			SaveSceneAs();
		}

	}

	void EditorLayer::SaveSceneAs()
	{
		std::string filepath = FileDialogs::SaveFile("Uge Scene (*.uge)\0*.uge\0\0");

		std::filesystem::path absPath(filepath);
		std::filesystem::path baseDir = Project::GetAssetAbsolutePath();

		std::filesystem::path relativePath = std::filesystem::relative(absPath, baseDir);



		std::filesystem::path path = Project::GetAssetFileSystemPath(relativePath).string();

		if (!path.empty())
		{
			SerializeScene(m_activeScene, path);
			m_editorScenePath = path;

		}

	}

	void EditorLayer::NewScene()
	{
		m_activeScene = CreateRef<Scene>();
		// m_activeScene->OnViewportResize((uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y);
		m_sceneHierarchyPanel.SetContext(m_activeScene);

		m_editorScenePath = std::filesystem::path();

	}

	void EditorLayer::OpenScene()
	{
		std::string filepath = FileDialogs::OpenFile("Uge Scene (*.uge)\0*.uge\0");
		
		std::filesystem::path absPath(filepath);
		std::filesystem::path baseDir = Project::GetAssetAbsolutePath();

		std::filesystem::path relativePath = std::filesystem::relative(absPath, baseDir);



		std::filesystem::path path = Project::GetAssetFileSystemPath(relativePath).string();
		
		
		if (!path.empty())
		{
			
			OpenScene(path);

		}

	}

	void EditorLayer::OpenScene(const std::filesystem::path& path)
	{

		if (m_sceneState != SceneState::Edit)
		{
			OnSceneStop();
		}

		if (path.extension().string() != ".uge")
		{
			UG_WARN("Could not load {0}: Not a scene file!", path.filename().string());
			return;
		}

		Ref<Scene> newScene = CreateRef<Scene>();

		m_activeScene = CreateRef<Scene>();
		SceneSerializer serializer(newScene);
		
		if (serializer.DeSerialize(path.string()))
		{
			m_editorScene = newScene;

			m_editorScene->OnViewportResize((uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y);
			m_sceneHierarchyPanel.SetContext(m_editorScene);

			m_activeScene = m_editorScene;
			m_editorScenePath = path;


		}


	}

	void EditorLayer::OnScenePlay()
	{

		m_sceneState = SceneState::Play;

		m_activeScene = Scene::Copy(m_editorScene);
		m_activeScene->OnRuntimeStart();

		m_sceneHierarchyPanel.SetContext(m_activeScene);

	}

	void EditorLayer::OnSceneStop()
	{

		UG_CORE_ASSERT(m_sceneState == SceneState::Play);

		if (m_sceneState == SceneState::Play)
		{
			m_activeScene->OnRuntimeStop();

		}
		
		m_sceneState = SceneState::Edit;

		m_activeScene = m_editorScene;

		m_sceneHierarchyPanel.SetContext(m_activeScene);
	
	}

	void EditorLayer::OnScenePause()
	{
		if (m_sceneState == SceneState::Edit)
		{
			return;
		}

		m_activeScene->SetPaused(true);
	}

	void EditorLayer::SerializeScene(Ref<Scene> scene, const std::filesystem::path& path)
	{

		SceneSerializer serializer(scene);
		serializer.Serialize(path.string());


	}

	void EditorLayer::OnDuplicateEntry()
	{

		if (m_sceneState != SceneState::Edit)
		{
			return;
		}

		Entity selectedEnt = m_sceneHierarchyPanel.GetSelectedEntity();
		if (selectedEnt)
		{
			Entity newEnt = m_editorScene->DuplicateEntity(selectedEnt);
			m_sceneHierarchyPanel.SetSelectedEntity(newEnt);
		}


	}

	

}


