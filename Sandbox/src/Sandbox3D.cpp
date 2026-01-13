#include "Sandbox3D.h"

#include "imgui.h"



#include <glm/gtc/type_ptr.hpp>

Sandbox3D::Sandbox3D()
	: Layer("Sandbox2D"), m_cameraController(75.0f,1280.0f / 720.0f, true)
{
}

void Sandbox3D::OnAttach()
{
	UG_PROFILE_FUNCTION();

	ImGuiIO& io = ImGui::GetIO();
	m_texture = Uge::Texture2D::Create("assets/textures/Checkerboard.png");
	m_mainFont = io.Fonts->AddFontFromFileTTF("C:\\Programming\\c++\\GameEngines\\Uge\\Uge\\assets\\fonts\\PlayfairDisplayBold-nRv8g.ttf", 32.5f);
	IM_ASSERT(m_mainFont != NULL);

}

void Sandbox3D::OnDetach()
{
	UG_PROFILE_FUNCTION();
}

void Sandbox3D::OnUpdate(Uge::Timestep ts)
{

	UG_PROFILE_FUNCTION();

	// Update	
	m_cameraController.OnUpdate(ts);

	


	// Render
	{
		UG_PROFILE_SCOPE("Render Prep");
		Uge::RenderCommand::SetClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1));
		Uge::RenderCommand::Clear();

	}

	Uge::Renderer3D::BeginScene(m_cameraController.GetCamera());
	{
		UG_PROFILE_SCOPE("Render Draw");
		// Flat color
		Uge::Renderer3D::DrawCube({ 1.0f, 0.0f, 0.0f }, 45.0f, { 1.0f, 1.0f, 1.0f },
			m_texture, 1.0f, {1.0f, 0.9f, 0.9f, 1.0f});

	




	}
	Uge::Renderer3D::EndScene();


}

void Sandbox3D::OnEvent(Uge::Event& e)
{
	m_cameraController.OnEvent(e);
}

void Sandbox3D::OnImGuiRender()
{
	UG_PROFILE_FUNCTION();

	ImGui::PushFont(m_mainFont);
	ImGui::Begin("Settings");
	{


		ImGui::PushID(0);
		ImGui::ColorEdit4("Square Color", glm::value_ptr(m_square1Color));
		ImGui::PopID();

		ImGui::PushID(1);
		ImGui::ColorEdit4("Square Color", glm::value_ptr(m_square2Color));
		ImGui::PopID();



		ImGui::Text("Sandbox2D");
	}
	ImGui::PopFont();
	ImGui::End();
}
