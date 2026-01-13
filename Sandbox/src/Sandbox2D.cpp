#include "Sandbox2D.h"

#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>




Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_cameraController(1280.0f / 720.0f, true)
{
}

void Sandbox2D::OnUpdate(Uge::Timestep ts)
{

	UG_PROFILE_FUNCTION();

	// Update	
	m_cameraController.OnUpdate(ts);
	
	
	



	// Render
	{
		UG_PROFILE_SCOPE("Renderer Prep");
		Uge::RenderCommand::SetClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1));
		Uge::RenderCommand::Clear();

	
	}

	{

		Uge::Renderer2D::BeginScene(m_cameraController.GetCamera());
		{
			UG_PROFILE_SCOPE("Renderer Draw");

			// Flat color
			Uge::Renderer2D::DrawQuad({ 1.0f, 0.0f, 0.0f }, { 0.3f, 0.3f }, m_square1Color);

			Uge::Renderer2D::DrawQuad({ -1.0f, 0.0f, 0.0f }, { 0.3f, 0.3f }, m_square2Color);
			// Texture
			//Uge::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, m_square2Color);




		}
		Uge::Renderer2D::EndScene();
	
	
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

	ImGuiIO& io = ImGui::GetIO();
	m_texture = Uge::Texture2D::Create("assets/textures/Checkerboard.png");
	m_mainFont = io.Fonts->AddFontFromFileTTF("C:\\Programming\\c++\\GameEngines\\Uge\\Uge\\assets\\fonts\\PlayfairDisplayBold-nRv8g.ttf", 32.5f);
	IM_ASSERT(m_mainFont != NULL);




}

void Sandbox2D::OnDetach()
{


	UG_PROFILE_FUNCTION();

}

void Sandbox2D::OnImGuiRender()
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
