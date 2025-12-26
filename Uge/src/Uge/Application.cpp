#include "ugpch.h"
#include "Application.h"


#include "Uge/Log.h"
#include "Platform/OpenGL/OpenGLBuffer.h"


#include <glad/glad.h>

#include "Uge/Input.h"
#include "Uge/KeyCodes.h"

#include <GLFW/glfw3.h>

#include "glm/glm.hpp"

namespace Uge
{
	
	
	Application* Application::s_instance = nullptr;

	


	Application::Application()
	{
		UG_CORE_ASSERT(!s_instance, "Application Already Exists!");
		s_instance = this;

		m_window = std::unique_ptr<Window>(Window::Create());
		m_window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

		// Vertex Array
		// Vertex Buffer
		// Index Buffer

		m_vertexArray.reset(VertexArray::Create());
		


		float vertices[3 * 7] =
		{
			/* Vertices */				/* Color */
		   -0.5f, -0.5f, 0.0f,		0.7f, 0.2f, 0.8f, 1.0f,
			0.5f, -0.5f, 0.0f,		0.2f, 0.3f, 0.8f, 1.0f,
			0.0f,  0.5f, 0.0f,      0.3f, 0.8f, 0.3f, 1.0f
		};

		std::shared_ptr<VertexBuffer> m_vertexBuffer;
		m_vertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));

	
		BufferLayout layout =
		{

			{ ShaderDataType::Float3, "a_Position"},
			{ ShaderDataType::Float4, "a_Color"}
		};

		m_vertexBuffer->SetLayout(layout);
		m_vertexArray->AddVertexBuffer(m_vertexBuffer);



		uint32_t indices[3] = { 0, 1, 2 };

		std::shared_ptr<IndexBuffer> m_indexBuffer;
		m_indexBuffer.reset(IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_vertexArray->SetIndexBuffer(m_indexBuffer);


		m_squareVA.reset(VertexArray::Create());
		
		float squareVertices[3 * 4] =
		{
			/* Vertices */	
		   -0.75f, -0.75f, 0.0f,
			0.75f, -0.75f, 0.0f,
		    0.75f,  0.75f, 0.0f,
		   -0.75f,  0.75f, 0.0f
		};
		
		
		std::shared_ptr<VertexBuffer> squareVB;
		squareVB.reset(VertexBuffer::Create(squareVertices, sizeof(squareVertices)));

		BufferLayout squareVBlayout =
		{
			{ ShaderDataType::Float3, "a_Position"}
		};

		squareVB->SetLayout(squareVBlayout);
		m_squareVA->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };

		std::shared_ptr<IndexBuffer> squareIB;
			
		squareIB.reset(IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		
		m_squareVA->SetIndexBuffer(squareIB);



		std::string vertexSrc = R"(
			#version 330 core
				
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			out vec3 v_Position;
			out vec4 v_Color;
			
			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = vec4(a_Position, 1.0);

				
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



		m_shader.reset(new Shader(vertexSrc, fragmentSrc));


		std::string whiteVertexSrc = R"(
			#version 330 core
				
			layout(location = 0) in vec3 a_Position;

			out vec3 v_Position;
			out vec4 v_Color;
			
			void main()
			{
				v_Position = a_Position;
				gl_Position = vec4(a_Position, 1.0);

				
			}


		)";


		std::string whiteFragmentSrc = R"(
			#version 330 core
				
			layout(location = 0) out vec4 o_color;

			in vec3 v_Position;
			
			void main()
			{
				o_color = vec4(1.0, 1.0, 1.0, 1.0);
			}


		)";



		m_whiteShader.reset(new Shader(whiteVertexSrc, whiteFragmentSrc));


	}

	Application::~Application()
	{

	}

	void Application::Run()
	{

		while (m_running)
		{
			if (Uge::Input::IsKeyPressed(UG_KEY_ESCAPE))
				CloseProgram();

			

			glClearColor(0.1f, 0.1f, 0.1f, 1);
			glClear(GL_COLOR_BUFFER_BIT);


			m_whiteShader->Bind();
			m_squareVA->Bind();
			glDrawElements(GL_TRIANGLES, m_squareVA->GetIndexBuffers()->GetCount(), GL_UNSIGNED_INT, nullptr);

			m_shader->Bind();
			m_vertexArray->Bind();
			glDrawElements(GL_TRIANGLES, m_vertexArray->GetIndexBuffers()->GetCount(), GL_UNSIGNED_INT, nullptr);



			for (auto layer : m_layerStack)
				layer->OnUpdate();

			m_ImGuiLayer->Begin();

			for (auto layer : m_layerStack)
				layer->OnImGuiRender();

			m_ImGuiLayer->End();

			m_window->OnUpdate();

		}
	}

	void Application::OnEvent(Event& e)
	{

		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));

		for ( auto it = m_layerStack.end(); it != m_layerStack.begin(); )
		{

			(*--it)->OnEvent(e);
			if (e.m_handled)
				break;


		}


	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		
		m_running = false;



		return true;

	}

	void Application::PushLayer(Layer* layer)
	{

		m_layerStack.PushLayer(layer);
		layer->OnAttach();


	}

	void Application::PushOverlay(Layer* overlay)
	{

		m_layerStack.PushOverlay(overlay);
		overlay->OnAttach();


	}

}

