/**
 * @file Application.h
 * @brief The application: window, layer stack and frame loop.
 * @ingroup group_core
 */

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

	/**
	 * @brief The `argc` and `argv` from `main`.
	 * @ingroup group_core
	 */
	struct ApplicationCommandLineArgs
	{
		int Count = 0; ///< Number of arguments.
		char** Args = nullptr; ///< The arguments, owned by the C runtime.

		/**
		 * @brief The argument at @p index.
		 * @param index Must be less than #Count; asserts otherwise.
		 * @return The argument.
		 */
		const char* operator[](int index) const
		{
			UG_CORE_ASSERT(index < Count);
			return Args[index];
		}
	};

	/**
	 * @brief Startup settings, supplied by the client.
	 * @ingroup group_core
	 */
	struct ApplicationSpecification
	{
		std::string Name = "Uge Application"; ///< Window title and application name.
		std::string WorkingDirectory; ///< Directory to switch to on startup; empty leaves it unchanged.
		ApplicationCommandLineArgs CommandLineArgs; ///< Arguments from `main`.
	};


	/**
	 * @brief Owns the window, the ImGui overlay and the layer stack, and runs the frame loop.
	 * @ingroup group_core
	 *
	 * A client derives from it and returns an instance from Uge::CreateApplication.
	 */
	class Application
	{
	public:
		/**
		 * @brief Creates the window and starts the renderer and physics.
		 * @param spec Startup settings.
		 *
		 * Asserts if an Application already exists.
		 */
		Application(const ApplicationSpecification& spec);


		/** @brief Destroys the layers, then shuts the subsystems down. */
		virtual ~Application();

		/**
		 * @brief Runs the frame loop until CloseProgram() is called or the window closes.
		 *
		 * Each frame runs the queued main-thread work, updates the layers (not while
		 * minimized), draws ImGui and swaps buffers.
		 */
		void Run();

		/**
		 * @brief Handles window close and resize, then passes the event to the layers.
		 * @param e The event.
		 *
		 * Layers get it from top to bottom, until one marks it handled.
		 */
		void OnEvent(Event& e);

		/**
		 * @brief Adds a layer and attaches it.
		 * @param layer Layer to add; the stack owns it.
		 */
		void PushLayer(Layer* layer);
		/**
		 * @brief Adds an overlay above every layer and attaches it.
		 * @param overlay Layer to add; the stack owns it.
		 *
		 * Overlays update last and get events first.
		 */
		void PushOverlay(Layer* overlay);

		/**
		 * @brief The application.
		 * @return The running instance.
		 */
		inline static Application& Get() { return *s_instance; }
		/**
		 * @brief The window.
		 * @return The window.
		 */
		inline Window& GetWindow() { return *m_window; }
		/** @brief Ends Run() after the current frame. */
		inline void CloseProgram() { m_running = false; }

		/**
		 * @brief The ImGui overlay.
		 * @return The layer, owned by the layer stack.
		 */
		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }


		/**
		 * @brief The startup settings.
		 * @return The settings.
		 */
		const ApplicationSpecification& GetSpecifications() const { return m_specification; }

		/**
		 * @brief Queues work to run on the main thread at the start of the next frame.
		 * @param func Work to run once.
		 *
		 * Thread-safe.
		 */
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

	/**
	 * @brief Creates the application. Implemented by the client.
	 * @param args Command-line arguments.
	 * @return The application; `main` deletes it after Run() returns.
	 * @ingroup group_core
	 */
	Application* CreateApplication(ApplicationCommandLineArgs args);

}