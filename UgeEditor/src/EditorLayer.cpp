#include "EditorLayer.h"

#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>


namespace Uge
{


	EditorLayer::EditorLayer()
		: Layer("Sandbox2D"), m_cameraController(1280.0f / 720.0f, true)
	{
	}

	void EditorLayer::OnUpdate(Timestep ts)
	{

		UG_PROFILE_FUNCTION();

		if (m_shouldResize)
		{
			m_frameBuffer->Resize((uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y);

			m_cameraController.OnResize(m_viewportSize.x, m_viewportSize.y);

			m_activeScene->OnViewportResize((uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y);
			m_shouldResize = false;
		}

		if (m_viewportFocused)
		{
			// Update	
			m_cameraController.OnUpdate(ts);

		}

		

		// Render
		Renderer2D::ResetStats();
		{
			UG_PROFILE_SCOPE("Renderer Prep")
			m_frameBuffer->Bind();
			RenderCommand::SetClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1));
			RenderCommand::Clear();
		}

		m_activeScene->OnUpdate(ts);
		m_frameBuffer->Unbind();

	}



	void EditorLayer::OnEvent(Event& e)
	{

		m_cameraController.OnEvent(e);


	}



	void EditorLayer::OnAttach()
	{
		UG_PROFILE_FUNCTION();

		FramebufferSpecification fbSpec{ 1280, 720 };
		m_frameBuffer = Framebuffer::Create(fbSpec);


		m_activeScene = CreateRef<Scene>();
		
		// Entity
		m_square1Ent = m_activeScene->CreateEntity("Red Square Entity");
		m_square1Ent.AddComponent<SpriteRendererComponent>(glm::vec4{1.0f, 0.0f, 0.0f, 1.0f});

		m_square2Ent = m_activeScene->CreateEntity("Green Square Entity");
		m_square2Ent.AddComponent<SpriteRendererComponent>(glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f });
		m_square2Ent.GetComponent<TransformComponent>().Translation = glm::vec3{ 1.0f, 0.0f, 0.0f };


		// Load ImGui Font
		ImGuiIO& io = ImGui::GetIO();
		m_mainFontBold = io.Fonts->AddFontFromFileTTF("C:\\Programming\\c++\\GameEngines\\Uge\\UgeEditor\\assets\\fonts\\Roboto-Regular\\static\\Roboto-Bold.ttf", 24.5f);
		m_mainFont = io.Fonts->AddFontFromFileTTF("C:\\Programming\\c++\\GameEngines\\Uge\\UgeEditor\\assets\\fonts\\Roboto-Regular\\static\\Roboto-Regular.ttf", 24.5f);
		
		IM_ASSERT(m_mainFont != NULL);
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		m_texture = Texture2D::Create("assets/textures/Checkerboard.png");

		// Load sprite sheet
		//m_spriteSheet = Texture2D::Create("assets/game/textures/RPGpack_sheet_2X.png");

		//m_textureStairs = SubTexture2D::CreateFromCoords(m_spriteSheet, { 7, 6 }, { 128, 128 }, { 1, 1 });
		//m_textureBarrel = SubTexture2D::CreateFromCoords(m_spriteSheet, { 2, 1 }, { 128, 128 }, { 1, 2 });

		m_cameraController.SetZoomLevel(5.0f);

		// Camera Entities
		m_cameraEnt = m_activeScene->CreateEntity("Camera A Entity");
		m_cameraEnt.AddComponent<CameraComponent>();

		m_secondCameraEnt = m_activeScene->CreateEntity("Camera B Entity");
		auto& cc = m_secondCameraEnt.AddComponent<CameraComponent>();
		cc.Primary = false;

		class CameraController : public ScriptableEntity
		{

		public:
			void OnCreate()
			{
				
				
			}

			void OnDestroy()
			{

			}

			void OnUpdate(Timestep ts)
			{
				
				auto& translation = GetComponent<TransformComponent>().Translation;
				float speed = 5.0f;

				if (Input::IsKeyPressed(UG_KEY_A))
					translation.x -= speed * ts;
				if (Input::IsKeyPressed(UG_KEY_D))
					translation.x += speed * ts;
				if (Input::IsKeyPressed(UG_KEY_W))
					translation.y += speed * ts;
				if (Input::IsKeyPressed(UG_KEY_S))	
					translation.y -= speed * ts;

			}


		};

		m_cameraEnt.AddComponent<NativeScriptComponent>().Bind<CameraController>();
		m_secondCameraEnt.AddComponent<NativeScriptComponent>().Bind<CameraController>();


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
					// Disabling fullscreen would allow the window to be moved to the front of other windows,
					// which we can't undo at the moment without finer window depth/z control.
					ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
					ImGui::MenuItem("Padding", NULL, &opt_padding);
					ImGui::Separator();

					if (ImGui::MenuItem("Exit", "")) Application::Get().CloseProgram();
					ImGui::Separator();

					ImGui::EndMenu();
				}

				ImGui::EndMenuBar();
			}

			m_sceneHierarchyPanel.OnImGuiRender();


			ImGui::Begin("Stats");
			{

				auto stats = Renderer2D::GetStats();
				ImGui::Text("Renderer2D Stats:");
				ImGui::Text("Draw Calls: %d", stats.DrawCalls);
				ImGui::Text("Quad Count: %d", stats.QuadCount);
				ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
				ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

				//ImGui::Dummy({ 0.0f, 100.0f });
				ImGui::Separator();
				ImGui::Text("Viewport Panel Size");
				ImGui::Text("X: %f", m_viewportSize.x);
				ImGui::Text("Y: %f", m_viewportSize.y);
			}
			ImGui::End();

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});

			ImGui::Begin("Scene Viewport");
			{
				m_viewportFocused = ImGui::IsWindowFocused();
				m_viewportHovered = ImGui::IsWindowHovered();
				
				Application::Get().GetImGuiLayer()->BlockEvents(!(m_viewportHovered && m_viewportFocused));
				

				ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

					

				if (m_viewportSize != *((glm::vec2*)&viewportPanelSize))
				{

					m_viewportSize = {viewportPanelSize.x, viewportPanelSize.y};
					
					m_shouldResize = true;
				}


				uint32_t textureID = m_frameBuffer->GetColorAttachment();
				ImGui::Image((void*)textureID, ImVec2{ m_viewportSize.x, m_viewportSize.y }, { 0, 1 }, { 1, 0 });


			}
			ImGui::End();
			ImGui::PopStyleVar();

		}
		ImGui::End();

		
		ImGui::PopFont();

	}




	


}


