#include "ugpch.h"
#include "Application.h"


#include "Uge/Log.h"

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


		glGenVertexArrays(1, &m_vertexArray);
		glBindVertexArray(m_vertexArray);

		glGenBuffers(1, &m_vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);


		float vertices[3 * 3] =
		{
			-0.5f, -0.5f, 0.5f,
			 0.5f, -0.5f, 0.0f,
			 0.0f,  0.5f, 0.5f
		};

		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

		glGenBuffers(1, &m_indexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);

		unsigned int indices[3] = { 0, 1, 2 };

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


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

			glBindVertexArray(m_vertexArray);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);



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

