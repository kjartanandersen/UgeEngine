#include <Uge.h>

#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>

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
		   -0.5f, -0.5f, 0.0f,		0.7f, 0.2f, 0.1f, 1.0f,
			0.5f, -0.5f, 0.0f,		0.1f, 0.5f, 0.4f, 1.0f,
			0.0f,  0.5f, 0.0f,      0.2f, 0.2f, 0.8f, 1.0f
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
		   -0.5f, -0.5f, 0.0f,
			0.5f, -0.5f, 0.0f,
			0.5f,  0.5f, 0.0f,
		   -0.5f,  0.5f, 0.0f
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


		// Triangle Shader

		std::string vertexSrc = R"(
			#version 330 core
				
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_ModelMatrix;

			out vec3 v_Position;
			out vec4 v_Color;
			
			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * u_ModelMatrix * vec4(a_Position, 1.0);

				
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

		// Flat Color Shader

		std::string flatColorVertexSrc = R"(
			#version 330 core
				
			layout(location = 0) in vec3 a_Position;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_ModelMatrix;

			out vec3 v_Position;
			out vec4 v_Color;
			
			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * u_ModelMatrix * vec4(a_Position, 1.0);

				
			}


		)";


		std::string flatColorFragmentSrc = R"(
			#version 330 core
				
			layout(location = 0) out vec4 o_color;

			in vec3 v_Position;

			uniform vec4 u_Color;
			
			void main()
			{
				o_color = u_Color;
			}


		)";



		m_flatColorShader.reset(new Uge::Shader(flatColorVertexSrc, flatColorFragmentSrc));

	}

	void OnUpdate(Uge::Timestep timestep) override
	{
		 
		//UG_TRACE("Delta Time: {0}s ( {1}ms )", timestep.GetSeconds(), timestep.GetMilliseconds());

		// Camera Position
		if (Uge::Input::IsKeyPressed(UG_KEY_A))
			m_cameraPosition.x -= m_cameraMovementSpeed * timestep;

		else if (Uge::Input::IsKeyPressed(UG_KEY_D))
			m_cameraPosition.x += m_cameraMovementSpeed * timestep;

		if (Uge::Input::IsKeyPressed(UG_KEY_W))
			m_cameraPosition.y += m_cameraMovementSpeed * timestep;

		else if (Uge::Input::IsKeyPressed(UG_KEY_S))
			m_cameraPosition.y -= m_cameraMovementSpeed * timestep;


		// Camera Rotation
		if (Uge::Input::IsKeyPressed(UG_KEY_Q))
			m_cameraRotation += m_cameraRotationSpeed *   timestep;

		if (Uge::Input::IsKeyPressed(UG_KEY_E))
			m_cameraRotation -= m_cameraRotationSpeed *   timestep;

		Uge::RenderCommand::SetClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1));
		Uge::RenderCommand::Clear();

		//m_camera.SetPosition(m_cameraPosition);
		//m_camera.SetRotation(m_cameraRotation);
		m_camera.SetPositionAndRotation(m_cameraPosition, m_cameraRotation);

		Uge::Renderer::BeginScene(m_camera);
		{
			

			glm::mat4 otherTransform = glm::translate(glm::mat4(1.0f), 
				glm::vec3(1.0f, 1.0f, 0.0f));

			
			// multicolor triangle
			Uge::Renderer::Submit(m_shader, m_vertexArray);
			
			// White squares
			static glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

			glm::vec4 redColor( 0.8f, 0.2f, 0.3f, 1.0f);
			glm::vec4 blueColor(0.2f, 0.3f, 0.8f, 1.0f);

			for (int y = 0; y < 20; y++)
			{
				for (int x = 0; x < 20; x++)
				{
					glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
					glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
					if (x % 2 == 0)
						m_flatColorShader->UploadUniformFloat4("u_Color", redColor);
					else
						m_flatColorShader->UploadUniformFloat4("u_Color", blueColor);
					Uge::Renderer::Submit(m_flatColorShader, m_squareVA, transform);

				}

			}

			

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

	std::shared_ptr<Uge::Shader> m_flatColorShader;
	std::shared_ptr<Uge::VertexArray> m_squareVA;

	Uge::OrthographicCamera m_camera;
	glm::vec3 m_cameraPosition;
	float m_cameraRotation = 0.0f;
	float m_cameraMovementSpeed = 1.0f;
	float m_cameraRotationSpeed = 50.0f;


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