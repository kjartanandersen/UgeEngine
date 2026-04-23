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

	struct ApplicationSpecification
	{
		std::string Name = "Uge Application";
		std::string WorkingDirectory;
		ApplicationCommandLineArgs CommandLineArgs;
	};


	class Application
	{
	public:
		Application(const ApplicationSpecification& spec);
		
		
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		inline static Application& Get() { return *s_instance; }
		inline Window& GetWindow() { return *m_window; }
		inline void CloseProgram() { m_running = false; }

		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }


		const ApplicationSpecification& GetSpecifications() const { return m_specification; }

		void SubmitToMainThreadQueue(const std::function<void()> func);

	protected:

	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		void ExecuteMainThreadQueue();

	private:
		ApplicationSpecification m_specification;

		Scope<Window> m_window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_running = true;
		bool m_minimized = false;
		LayerStack m_layerStack;

		float m_lastFrameTime = 0.0f;

		std::mutex m_mainThreadQueueMutex;
		std::vector<std::function<void()>> m_mainThreadQueue;

	private:
		static Application* s_instance;

	};

	// To be defined in a client
	Application* CreateApplication(ApplicationCommandLineArgs args);

}