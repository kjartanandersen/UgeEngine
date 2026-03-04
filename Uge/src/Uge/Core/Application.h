#pragma once

#include "Core.h"

#include "Window.h"
#include "Uge/Core/LayerStack.h"

#include "Uge/Events/Event.h"
#include "Uge/Events/ApplicationEvent.h"

#include "Uge/Core/Timestep.h"

#include "Uge/ImGui/ImGuiLayer.h"


namespace Uge
{

	struct ApplicationCommandLineArgs
	{
		int Count = 0;
		char** Args = nullptr;

		const char* operator[](int index) const
		{
			UG_CORE_ASSERT(index < Count);
			return Args[index];
		}
	};


	class Application
	{
	public:
		Application(bool is3D, const std::string& name = "Uge App");
		Application(bool is3D, const std::string& name = "Uge App", 
			ApplicationCommandLineArgs args = ApplicationCommandLineArgs());
		
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		inline static Application& Get() { return *s_instance; }
		inline Window& GetWindow() { return *m_window; }
		inline void CloseProgram() { m_running = false; }

		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }


		ApplicationCommandLineArgs GetCommandLineArgs() const { return m_CommandLineArgs; }

	protected:
		bool m_is3D;

	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

	private:
		ApplicationCommandLineArgs m_CommandLineArgs;

		Scope<Window> m_window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_running = true;
		bool m_minimized = false;
		LayerStack m_layerStack;

		float m_lastFrameTime = 0.0f;

	private:
		static Application* s_instance;

	};

	// To be defined in a client
	Application* CreateApplication();
	Application* CreateApplication(ApplicationCommandLineArgs args);

}