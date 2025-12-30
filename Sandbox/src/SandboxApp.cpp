#include <Uge.h>

#include "imgui/imgui.h"

class ExampleLayer : public Uge::Layer
{

public:
	ExampleLayer()
		: Layer("Example"), m_camera(-1.6f, 1.6f, -0.9f, 0.9f), m_cameraPosition({0.0f, 0.0f, 0.0f})
	{
		m_vertexArray.reset(Uge::VertexArray::Create());

		// Vertex Array
		// Vertex Buffer
		// Index Buffer


		float vertices[3 * 7] =
		{
			/* Vertices */				/* Color */
		   -0.5f, -0.5f, 0.0f,		0.7f, 0.2f, 0.8f, 1.0f,
			0.5f, -0.5f, 0.0f,		0.2f, 0.3f, 0.8f, 1.0f,
			0.0f,  0.5f, 0.0f,      0.3f, 0.8f, 0.3f, 1.0f
		};

		std::shared_ptr<Uge::VertexBuffer> m_vertexBuffer;
		m_vertexBuffer.reset(Uge::VertexBuffer::Create(vertices, sizeof(vertices)));


		Uge::BufferLayout layout =
		{

			{ Uge::ShaderDataType::Float3, "a_Position"},
			{ Uge::ShaderDataType::Float4, "a_Color"}
		};

		m_vertexBuffer->SetLayout(layout);


		m_vertexArray->AddVertexBuffer(m_vertexBuffer);

		uint32_t indices[3] = { 0, 1, 2 };

		std::shared_ptr<Uge::IndexBuffer> m_indexBuffer;
		m_indexBuffer.reset(Uge::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_vertexArray->SetIndexBuffer(m_indexBuffer);


		m_squareVA.reset(Uge::VertexArray::Create());

		float squareVertices[3 * 4] =
		{
			/* Vertices */
		   -0.75f, -0.75f, 0.0f,
			0.75f, -0.75f, 0.0f,
			0.75f,  0.75f, 0.0f,
		   -0.75f,  0.75f, 0.0f
		};


		std::shared_ptr<Uge::VertexBuffer> squareVB;
		squareVB.reset(Uge::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));

		Uge::BufferLayout squareVBlayout =
		{
			{ Uge::ShaderDataType::Float3, "a_Position"}
		};

		squareVB->SetLayout(squareVBlayout);
		m_squareVA->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };

		std::shared_ptr<Uge::IndexBuffer> squareIB;

		squareIB.reset(Uge::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));

		m_squareVA->SetIndexBuffer(squareIB);



		std::string vertexSrc = R"(
			#version 330 core
				
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			uniform mat4 u_ViewProjection;

			out vec3 v_Position;
			out vec4 v_Color;
			
			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * vec4(a_Position, 1.0);

				
			}


		)";


		std::string fragmentSrc = R"(
			#version 330 core
				
			layout(location = 0) out vec4 o_color;

			in vec3 v_Position;
			in vec4 v_Color;
			
			void main()
			{
				o_color = v_Color;
			}


		)";



		m_shader.reset(new Uge::Shader(vertexSrc, fragmentSrc));


		std::string whiteVertexSrc = R"(
			#version 330 core
				
			layout(location = 0) in vec3 a_Position;

			uniform mat4 u_ViewProjection;

			out vec3 v_Position;
			out vec4 v_Color;
			
			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * vec4(a_Position, 1.0);

				
			}


		)";


		std::string whiteFragmentSrc = R"(
			#version 330 core
				
			layout(location = 0) out vec4 o_color;

			in vec3 v_Position;
			
			void main()
			{
				o_color = vec4(0.3f, 0.8f, 0.3f, 1.0f);
			}


		)";



		m_whiteShader.reset(new Uge::Shader(whiteVertexSrc, whiteFragmentSrc));

	}

	void OnUpdate() override
	{


		if (Uge::Input::IsKeyPressed(UG_KEY_A))
			m_cameraPosition.x -= m_cameraMovementSpeed;

		else if (Uge::Input::IsKeyPressed(UG_KEY_D))
			m_cameraPosition.x += m_cameraMovementSpeed;

		if (Uge::Input::IsKeyPressed(UG_KEY_W))
			m_cameraPosition.y += m_cameraMovementSpeed;

		else if (Uge::Input::IsKeyPressed(UG_KEY_S))
			m_cameraPosition.y -= m_cameraMovementSpeed;

		if (Uge::Input::IsKeyPressed(UG_KEY_Q))
			m_cameraRotation += m_cameraRotationSpeed;

		if (Uge::Input::IsKeyPressed(UG_KEY_E))
			m_cameraRotation -= m_cameraRotationSpeed;

		Uge::RenderCommand::SetClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1));
		Uge::RenderCommand::Clear();

		m_camera.SetPosition(m_cameraPosition);
		m_camera.SetRotation(m_cameraRotation);

		Uge::Renderer::BeginScene(m_camera);
		{

			// White square
			Uge::Renderer::Submit(m_whiteShader, m_squareVA);

			// multicolor triangle
			Uge::Renderer::Submit(m_shader, m_vertexArray);

		}

		Uge::Renderer::EndScene();

		


	}

	virtual void OnImGuiRender() override
	{

		//ImGui::Begin("Test");
		//ImGui::Text("Hello World");
		//ImGui::End();
		


	}

	

	void OnEvent(Uge::Event& event) override
	{

	}

private:
	std::shared_ptr<Uge::Shader> m_shader;
	std::shared_ptr<Uge::VertexArray> m_vertexArray;

	std::shared_ptr<Uge::Shader> m_whiteShader;
	std::shared_ptr<Uge::VertexArray> m_squareVA;

	Uge::OrthographicCamera m_camera;
	glm::vec3 m_cameraPosition;
	float m_cameraRotation = 0.0f;
	float m_cameraMovementSpeed = 0.005f;
	float m_cameraRotationSpeed = 0.5f;

};


class Sandbox : public Uge::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}
	~Sandbox()
	{

	}
};

Uge::Application* Uge::CreateApplication()
{
	return new Sandbox();
}