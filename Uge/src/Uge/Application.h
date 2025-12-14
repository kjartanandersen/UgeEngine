#pragma once

#include "Core.h"

#include "Window.h"
#include "Uge/LayerStack.h"
#include "Uge/Events/Event.h"
#include "Uge/Events/ApplicationEvent.h"




namespace Uge
{
	class UG_API Application
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

	private:
		bool OnWindowClose(WindowCloseEvent& e);


		std::unique_ptr<Window> m_window;
		bool m_running = true;
		LayerStack m_layerStack;

	private:
		static Application* s_instance;

	};

	// To be defined in a client
	Application* CreateApplication();

}