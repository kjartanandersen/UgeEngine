#include "Sandbox2D.h"

#include "imgui.h"

#include <glm/gtc/matrix_transform.hpp>

#include "Platform/OpenGL/OpenGLShader.h"

#include <glm/gtc/type_ptr.hpp>


Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_cameraController(1280.0f / 720.0f, true)
{
}

void Sandbox2D::OnUpdate(Uge::Timestep ts)
{

	// Update
	m_cameraController.OnUpdate(ts);


	// Render
	Uge::RenderCommand::SetClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1));
	Uge::RenderCommand::Clear();

	Uge::Renderer::BeginScene(m_cameraController.GetCamera());
	{



		std::dynamic_pointer_cast<Uge::OpenGLShader>(m_flatColorShader)->Bind();
		std::dynamic_pointer_cast<Uge::OpenGLShader>(m_flatColorShader)->UploadUniformFloat4("u_Color", m_squareColor);



		Uge::Renderer::Submit(m_flatColorShader, m_squareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.0f)));


	}
	Uge::Renderer::EndScene();

}

void Sandbox2D::OnEvent(Uge::Event& e)
{

	m_cameraController.OnEvent(e);


}



void Sandbox2D::OnAttach()
{

	


	m_squareVA = Uge::VertexArray::Create();

	float squareVertices[3 * 4] =
	{
		/* Vertices */			
	   -0.5f, -0.5f, 0.0f,		
		0.5f, -0.5f, 0.0f,		
		0.5f,  0.5f, 0.0f,		
	   -0.5f,  0.5f, 0.0f		
	};


	Uge::Ref<Uge::VertexBuffer> squareVB;
	squareVB = Uge::VertexBuffer::Create(squareVertices, sizeof(squareVertices));

	Uge::BufferLayout squareVBlayout =
	{
		{ Uge::ShaderDataType::Float3, "a_Position"}
	};

	squareVB->SetLayout(squareVBlayout);
	m_squareVA->AddVertexBuffer(squareVB);

	uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };

	Uge::Ref<Uge::IndexBuffer> squareIB;

	squareIB = Uge::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));

	m_squareVA->SetIndexBuffer(squareIB);


	m_flatColorShader = Uge::Shader::Create("assets/shaders/FlatColorShader.glsl");





}

void Sandbox2D::OnDetach()
{




}

void Sandbox2D::OnImGuiRender()
{

	ImGui::Begin("Settings");
	{

		ImGui::ColorEdit4("Square Color", glm::value_ptr(m_squareColor));
		ImGui::Text("Sandbox2D");
	}
	ImGui::End();



}
