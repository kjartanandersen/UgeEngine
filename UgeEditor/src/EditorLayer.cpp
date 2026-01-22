#include "EditorLayer.h"

#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>


static const uint32_t s_mapWidth = 24;
static const char* s_mapTiles =
"WWWWWWWWWWWWWWWWWWWWWWWW"
"WWWWWWWWWWWWWWWWWWWWWWWW"
"WWWWWWWWDDDDDDWWWWWWWWWW"
"WWWWWWWDDDDDDDDDDWWWWWWW"
"WWWWWWDDDDDDDDDDDDWWWWWW"
"WWWWWDDDDWWDDDDDDDDDDDWW"
"WWWWDDDDDWWDDDDDDDDDDDWW"
"WWWDDDDDDDDDDDDDDDDDDDWW"
"WWWWWDDDDDDDDDDDDDDDDDWW"
"WWWWWDDDDDDDDDDDDDDDDDWW"
"WWWWWWDDDDDDDDDDDDDDDDWW"
"WWWWWWWWWWWWWWWWWWWWWWWW"
"WWWWWWWWWWWWWWWWWWWWWWWW"
"WWWWWWWWWWWWWWWWWWWWWWWW";


namespace Uge
{


	EditorLayer::EditorLayer()
		: Layer("Sandbox2D"), m_cameraController(1280.0f / 720.0f, true)
	{
	}

	void EditorLayer::OnUpdate(Uge::Timestep ts)
	{

		UG_PROFILE_FUNCTION();

		if (m_shouldResize)
		{
			m_frameBuffer->Resize((uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y);

			m_cameraController.OnResize(m_viewportSize.x, m_viewportSize.y);
			m_shouldResize = false;
		}

		if (m_viewportFocused)
		{
			// Update	
			m_cameraController.OnUpdate(ts);

		}

		// Render
		Uge::Renderer2D::ResetStats();
		{
			UG_PROFILE_SCOPE("Renderer Prep");

			
			m_frameBuffer->Bind();

			Uge::RenderCommand::SetClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1));
			Uge::RenderCommand::Clear();


		}

		{
			static float rotation = 0.0f;
			rotation += ts * 50.0f;

#if 1
			Uge::Renderer2D::BeginScene(m_cameraController.GetCamera());
			{
				UG_PROFILE_SCOPE("Renderer Draw");

				// Flat color
				Uge::Renderer2D::DrawRotatedQuad({ 1.0f, 0.0f, 0.0f }, glm::radians(45.0f), { 0.3f, 0.3f }, m_square1Color);

				Uge::Renderer2D::DrawQuad({ -1.0f, 0.0f, 0.0f }, { 0.3f, 0.3f }, m_square2Color);
				Uge::Renderer2D::DrawQuad({ 0.0f, 0.5f, 0.0f }, { 0.3f, 0.3f }, m_square1Color);
				Uge::Renderer2D::DrawRotatedQuad({ 0.0f, 1.0f, 0.0f }, glm::radians(rotation), { 0.3f, 0.3f }, m_square2Color);
				// Texture
				Uge::Renderer2D::DrawRotatedQuad({ 0.0f, 0.0f, -0.1f }, glm::radians(0.0f), { 10.0f, 10.0f }, m_texture, 10.0f);




			}
			Uge::Renderer2D::EndScene();


#if 0
			Uge::Renderer2D::BeginScene(m_cameraController.GetCamera());
			{
				UG_PROFILE_SCOPE("Renderer Draw");

				for (float y = -5.0f; y < 5.0f; y += 0.5f)
				{
					for (float x = -5.0f; x < 5.0f; x += 0.5f)
					{
						glm::vec4 color = { (x + 5) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.5f };
						Uge::Renderer2D::DrawQuad({ x, y }, { 0.45f, 0.45f }, color);

					}




				}



			}
			Uge::Renderer2D::EndScene();
#endif
			

#else
			if (Uge::Input::IsMouseButtonPressed(UG_MOUSE_BUTTON_LEFT))
			{
				auto [x, y] = Uge::Input::GetMousePos();
				auto width = Uge::Application::Get().GetWindow().GetWidth();
				auto height = Uge::Application::Get().GetWindow().GetHeight();

				auto bounds = m_cameraController.GetBounds();
				auto pos = m_cameraController.GetCamera().GetPosition();
				x = (x / width) * bounds.GetWidth() - bounds.GetWidth() * 0.5f;
				y = bounds.GetHeight() * 0.5f - (y / height) * bounds.GetHeight();

			}

			Uge::Renderer2D::BeginScene(m_cameraController.GetCamera());
			{
				UG_PROFILE_SCOPE("Renderer Draw");


				//Uge::Renderer2D::DrawQuad({ 0.0f, 0.0f, 0.5f }, { 1.0f, 1.0f }, m_textureStairs);
				//Uge::Renderer2D::DrawQuad({ 1.0f, 1.0f, 0.5f }, { 1.0f, 2.0f }, m_textureBarrel);


				for (uint32_t y = 0; y < m_mapHeight; y++)
				{
					for (uint32_t x = 0; x < m_mapWidth; x++)
					{
						char tileType = s_mapTiles[x + y * m_mapWidth];
						Uge::Ref<Uge::SubTexture2D> texture;
						if (m_textureMap.find(tileType) != m_textureMap.end())
							texture = m_textureMap[tileType];
						else
							texture = m_textureBarrel;

						Uge::Renderer2D::DrawQuad({ x - (m_mapWidth / 2.0f),m_mapHeight - y - (m_mapHeight / 2.0f), 0.5f }, { 1.0f, 1.0f }, texture);


					}
				}


			}
			Uge::Renderer2D::EndScene();


#endif

			m_frameBuffer->Unbind();

		}



		//std::dynamic_pointer_cast<Uge::OpenGLShader>(m_flatColorShader)->Bind();
		//std::dynamic_pointer_cast<Uge::OpenGLShader>(m_flatColorShader)->UploadUniformFloat4("u_Color", m_squareColor);
	}

	void EditorLayer::OnEvent(Uge::Event& e)
	{

		m_cameraController.OnEvent(e);


	}



	void EditorLayer::OnAttach()
	{
		UG_PROFILE_FUNCTION();

		Uge::FramebufferSpecification fbSpec{ 1280, 720 };

		m_frameBuffer = Uge::Framebuffer::Create(fbSpec);


		ImGuiIO& io = ImGui::GetIO();
		m_texture = Uge::Texture2D::Create("assets/textures/Checkerboard.png");
		m_mainFont = io.Fonts->AddFontFromFileTTF("C:\\Programming\\c++\\GameEngines\\Uge\\Uge\\assets\\fonts\\PlayfairDisplayBold-nRv8g.ttf", 32.5f);
		IM_ASSERT(m_mainFont != NULL);
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;


		// Load sprite sheet
		m_spriteSheet = Uge::Texture2D::Create("assets/game/textures/RPGpack_sheet_2X.png");

		m_textureStairs = Uge::SubTexture2D::CreateFromCoords(m_spriteSheet, { 7, 6 }, { 128, 128 }, { 1, 1 });
		m_textureBarrel = Uge::SubTexture2D::CreateFromCoords(m_spriteSheet, { 2, 1 }, { 128, 128 }, { 1, 2 });

		m_mapWidth = s_mapWidth;
		m_mapHeight = strlen(s_mapTiles) / m_mapWidth;

		m_textureMap['D'] = Uge::SubTexture2D::CreateFromCoords(m_spriteSheet, { 6, 11 }, { 128, 128 }, { 1, 2 });
		m_textureMap['W'] = Uge::SubTexture2D::CreateFromCoords(m_spriteSheet, { 11, 11 }, { 128, 128 }, { 1, 2 });



		m_cameraController.SetZoomLevel(5.0f);



	}

	void EditorLayer::OnDetach()
	{


		UG_PROFILE_FUNCTION();

	}

	void EditorLayer::OnImGuiRender()
	{
		UG_PROFILE_FUNCTION();
		
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
			if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
			{
				ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
				ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
			}


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

					if (ImGui::MenuItem("Exit", "")) Uge::Application::Get().CloseProgram();
					ImGui::Separator();

					ImGui::EndMenu();
				}

				ImGui::EndMenuBar();
			}



			ImGui::Begin("Settings");
			{

				auto stats = Uge::Renderer2D::GetStats();
				ImGui::Text("Renderer2D Stats:");
				ImGui::Text("Draw Calls: %d", stats.DrawCalls);
				ImGui::Text("Quad Count: %d", stats.QuadCount);
				ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
				ImGui::Text("Indices: %d", stats.GetTotalIndexCount());


				ImGui::Text("");
				ImGui::Text("Squre Color Pickers:");
				ImGui::PushID(0);
				ImGui::ColorEdit4("Square Color", glm::value_ptr(m_square1Color));
				ImGui::PopID();

				ImGui::PushID(1);
				ImGui::ColorEdit4("Square Color", glm::value_ptr(m_square2Color));
				ImGui::PopID();

				ImGui::Text("Sandbox2D");
				ImGui::Text("Viewport Panel Size");
				ImGui::Text("X: %f", m_viewportSize.x);
				ImGui::Text("Y: %f", m_viewportSize.y);

				ImGui::InputText("Text", "", 0);
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

		}
		ImGui::End();

		ImGui::PopStyleVar();
		ImGui::PopFont();

	}




	


}


