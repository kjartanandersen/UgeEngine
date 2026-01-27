#include "Sandbox2D.h"

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


Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_cameraController(1280.0f / 720.0f, true), m_particleSystem(100000)
{
}

void Sandbox2D::OnUpdate(Uge::Timestep ts)
{

	UG_PROFILE_FUNCTION();

	// Update	
	m_cameraController.OnUpdate(ts);

	// Render
	Uge::Renderer2D::ResetStats();
	{
		UG_PROFILE_SCOPE("Renderer Prep");

		

		Uge::RenderCommand::SetClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1));
		Uge::RenderCommand::Clear();

	
	}

	{
		static float rotation = 0.0f;
		rotation += ts * 50.0f;

#if 0
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



		Uge::Renderer2D::BeginScene(m_cameraController.GetCamera());
		{
			UG_PROFILE_SCOPE("Renderer Draw");

			for (float y=-5.0f; y<5.0f; y+=0.5f)
			{
				for (float x=-5.0f; x<5.0f; x+=0.5f)
				{
					glm::vec4 color = { (x + 5) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.5f };
					Uge::Renderer2D::DrawQuad({ x, y }, { 0.45f, 0.45f }, color);

				}




			}



		}
		Uge::Renderer2D::EndScene();
#endif




		if (Uge::Input::IsMouseButtonPressed(UG_MOUSE_BUTTON_LEFT))
		{
			auto [x, y] = Uge::Input::GetMousePos();
			auto width = Uge::Application::Get().GetWindow().GetWidth();
			auto height = Uge::Application::Get().GetWindow().GetHeight();

			auto bounds = m_cameraController.GetBounds();
			//auto pos = m_cameraController.GetCamera().GetPosition();
			x = (x / width) * bounds.GetWidth() - bounds.GetWidth() * 0.5f;
			y = bounds.GetHeight() * 0.5f - (y / height) * bounds.GetHeight();
			//m_particle.Position = { x + pos.x, y + pos.y };
			for (int i = 0; i < 50; i++)
				m_particleSystem.Emit(m_particle);
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


		m_particleSystem.OnUpdate(ts);
		m_particleSystem.OnRender(m_cameraController.GetCamera());
	
	
	}

	

	//std::dynamic_pointer_cast<Uge::OpenGLShader>(m_flatColorShader)->Bind();
	//std::dynamic_pointer_cast<Uge::OpenGLShader>(m_flatColorShader)->UploadUniformFloat4("u_Color", m_squareColor);
}

void Sandbox2D::OnEvent(Uge::Event& e)
{

	m_cameraController.OnEvent(e);


}



void Sandbox2D::OnAttach()
{
	UG_PROFILE_FUNCTION();

	Uge::FramebufferSpecification fbSpec{1280, 720};
	
	


	ImGuiIO& io = ImGui::GetIO();
	m_texture = Uge::Texture2D::Create("assets/textures/Checkerboard.png");
	m_mainFont = io.Fonts->AddFontFromFileTTF("C:\\Programming\\c++\\GameEngines\\Uge\\Uge\\assets\\fonts\\PlayfairDisplayBold-nRv8g.ttf", 32.5f);
	IM_ASSERT(m_mainFont != NULL);

	

	// Load sprite sheet
	m_spriteSheet = Uge::Texture2D::Create("assets/game/textures/RPGpack_sheet_2X.png");

	m_textureStairs = Uge::SubTexture2D::CreateFromCoords(m_spriteSheet, { 7, 6 }, { 128, 128 }, { 1, 1 });
	m_textureBarrel = Uge::SubTexture2D::CreateFromCoords(m_spriteSheet, { 2, 1 }, { 128, 128  }, {1, 2});
	
	m_mapWidth = s_mapWidth;
	m_mapHeight = strlen(s_mapTiles) / m_mapWidth;
	
	m_textureMap['D'] = Uge::SubTexture2D::CreateFromCoords(m_spriteSheet, { 6, 11 }, { 128, 128  }, {1, 2});
	m_textureMap['W'] = Uge::SubTexture2D::CreateFromCoords(m_spriteSheet, { 11, 11 }, { 128, 128  }, {1, 2});
	

	// Init here
	m_particle.ColorBegin = { 254 / 255.0f, 212 / 255.0f, 123 / 255.0f, 1.0f };
	m_particle.ColorEnd = { 254 / 255.0f, 109 / 255.0f, 41 / 255.0f, 1.0f };
	m_particle.SizeBegin = 0.5f, m_particle.SizeVariation = 0.3f, m_particle.SizeEnd = 0.0f;
	m_particle.LifeTime = 5.0f;
	m_particle.Velocity = { 0.0f, 0.0f };
	m_particle.VelocityVariation = { 3.0f, 1.0f };
	m_particle.Position = { 0.0f, 0.0f };


	m_cameraController.SetZoomLevel(5.0f);



}

void Sandbox2D::OnDetach()
{


	UG_PROFILE_FUNCTION();

}

void Sandbox2D::OnImGuiRender()
{
	UG_PROFILE_FUNCTION();


	
	/*****************************
	*
	* Dockspace
	*
	*****************************/


	ImGui::PushFont(m_mainFont);

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
	}
	ImGui::End();


	ImGui::PopFont();

	

	


}
