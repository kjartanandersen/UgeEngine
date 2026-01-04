#include <Uge.h>

#include "imgui.h"


#include "Platform/OpenGL/OpenGLShader.h"
#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtc/type_ptr.hpp>

class ExampleLayer : public Uge::Layer
{

public:
	ExampleLayer()
		: Layer("Example"), m_camera(-1.6f, 1.6f, -0.9f, 0.9f), m_cameraPosition({0.0f, 0.0f, 0.0f})
	{
		m_vertexArray = Uge::VertexArray::Create();

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

		Uge::Ref<Uge::VertexBuffer> m_vertexBuffer;
		m_vertexBuffer = Uge::VertexBuffer::Create(vertices, sizeof(vertices));


		Uge::BufferLayout layout =
		{

			{ Uge::ShaderDataType::Float3, "a_Position"},
			{ Uge::ShaderDataType::Float4, "a_Color"}
		};

		m_vertexBuffer->SetLayout(layout);


		m_vertexArray->AddVertexBuffer(m_vertexBuffer);

		uint32_t indices[3] = { 0, 1, 2 };

		Uge::Ref<Uge::IndexBuffer> m_indexBuffer;
		m_indexBuffer = Uge::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
		m_vertexArray->SetIndexBuffer(m_indexBuffer);


		m_squareVA = Uge::VertexArray::Create();

		float squareVertices[5 * 4] =
		{
			/* Vertices */			/* Texture Coordinates */
		   -0.5f, -0.5f, 0.0f,		0.0f, 0.0f,
			0.5f, -0.5f, 0.0f,		1.0f, 0.0f,
			0.5f,  0.5f, 0.0f,		1.0f, 1.0f,
		   -0.5f,  0.5f, 0.0f,		0.0f, 1.0f
		};


		Uge::Ref<Uge::VertexBuffer> squareVB;
		squareVB = Uge::VertexBuffer::Create(squareVertices, sizeof(squareVertices));

		Uge::BufferLayout squareVBlayout =
		{
			{ Uge::ShaderDataType::Float3, "a_Position"},
			{ Uge::ShaderDataType::Float2, "a_TextCoord"}
		};

		squareVB->SetLayout(squareVBlayout);
		m_squareVA->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };

		Uge::Ref<Uge::IndexBuffer> squareIB;

		squareIB = Uge::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));

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



		m_shader.reset(Uge::Shader::Create(vertexSrc, fragmentSrc));

		// Flat Color Shader

		std::string flatColorVertexSrc = R"(
			#version 330 core
				
			layout(location = 0) in vec3 a_Position;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_ModelMatrix;

			out vec3 v_Position;
			
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

		m_flatColorShader.reset(Uge::Shader::Create(flatColorVertexSrc, flatColorFragmentSrc));



		m_textureShader.reset(Uge::Shader::Create("assets/shaders/Texture.glsl"));

		m_texture   = Uge::Texture2D::Create("assets/textures/Checkerboard.png");
		m_chTexture = Uge::Texture2D::Create("assets/textures/ChernoLogo.png");

		std::dynamic_pointer_cast<Uge::OpenGLShader>(m_textureShader)->Bind();
		std::dynamic_pointer_cast<Uge::OpenGLShader>(m_textureShader)->UploadUniformInt("u_Texture", 0);

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
			//Uge::Renderer::Submit(m_shader, m_vertexArray);
			
			// White squares
			static glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

			std::dynamic_pointer_cast<Uge::OpenGLShader>(m_flatColorShader)->Bind();
			std::dynamic_pointer_cast<Uge::OpenGLShader>(m_flatColorShader)->UploadUniformFloat4("u_Color", m_squareColor);

			for (int y = 0; y < 20; y++)
			{
				for (int x = 0; x < 20; x++)
				{
					glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
					glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
					Uge::Renderer::Submit(m_flatColorShader, m_squareVA, transform);

				}

			}

			m_texture->Bind();
			
			Uge::Renderer::Submit(m_textureShader, m_squareVA, 
				glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

			m_chTexture->Bind();

			Uge::Renderer::Submit(m_textureShader, m_squareVA,
				glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

		}
		Uge::Renderer::EndScene();

		


	}

	

	void OnEvent(Uge::Event& event) override
	{

	}

	/******************* ImGui Methods ************/

	void OnAttach() override
	{
		ImGuiIO& io = ImGui::GetIO();

		io.Fonts->AddFontDefault();
		mainFont = io.Fonts->AddFontFromFileTTF("C:\\Programming\\c++\\GameEngines\\Uge\\Uge\\assets\\fonts\\PlayfairDisplayBold-nRv8g.ttf", 32.5f);
		IM_ASSERT(mainFont != NULL);

	}

	virtual void OnImGuiRender() override
	{

		ImGui::PushFont(mainFont);
		ImGui::Begin("Settings");
		{

			ImGui::ColorEdit4("Square Color", glm::value_ptr(m_squareColor));
		}
		ImGui::End();
		ImGui::PopFont();



	}

private:
	Uge::Ref<Uge::Shader> m_shader;
	Uge::Ref<Uge::VertexArray> m_vertexArray;

	Uge::Ref<Uge::Shader> m_flatColorShader, m_textureShader;
	Uge::Ref<Uge::VertexArray> m_squareVA;

	Uge::Ref<Uge::Texture2D> m_texture, m_chTexture;

	ImFont* mainFont;

	Uge::OrthographicCamera m_camera;
	glm::vec3 m_cameraPosition;
	float m_cameraRotation = 0.0f;
	float m_cameraMovementSpeed = 1.0f;
	float m_cameraRotationSpeed = 50.0f;

	glm::vec4 m_squareColor = {0.2f, 0.3f, 0.8f, 1.0f};


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