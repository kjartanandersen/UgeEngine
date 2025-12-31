#pragma once

#include "Core.h"

#include "Window.h"
#include "Uge/LayerStack.h"

#include "Uge/Events/Event.h"
#include "Uge/Events/ApplicationEvent.h"

#include "Uge/Core/Timestep.h"

#include "Uge/ImGui/ImGuiLayer.h"


namespace Uge
{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		inline static Application& Get() { return *s_instance; }
		inline Window& GetWindow() { return *m_window; }
		inline void CloseProgram() { m_running = false; }

	private:
		bool OnWindowClose(WindowCloseEvent& e);

	private:
		std::unique_ptr<Window> m_window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_running = true;
		LayerStack m_layerStack;

		float m_lastFrameTime = 0.0f;

	private:
		static Application* s_instance;

	};

	// To be defined in a client
	Application* CreateApplication();

}