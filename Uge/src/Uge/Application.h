#pragma once

#include "Core.h"
#include "Events/Event.h"
#include "Uge/Events/ApplicationEvent.h"

#include "Window.h"



namespace Uge
{
	class UG_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

	private:
		bool OnWindowClose(WindowCloseEvent& e);


		std::unique_ptr<Window> m_window;
		bool m_running = true;

	};

	// To be defined in a client
	Application* CreateApplication();

}