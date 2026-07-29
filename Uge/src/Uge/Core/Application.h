/**
 * @file Application.h
 * @brief The engine's root object: owns the window, the layer stack and the frame loop.
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
	 * @brief The `argc`/`argv` pair forwarded from `main`.
	 * @ingroup group_core
	 *
	 * Passed to Uge::CreateApplication so a client can react to command-line arguments,
	 * for example to open a specific project on startup.
	 */
	struct ApplicationCommandLineArgs
	{
		int Count = 0; ///< Number of entries in #Args.
		char** Args = nullptr; ///< Argument vector, owned by the C runtime.

		/**
		 * @brief Returns the argument at @p index.
		 * @param index Zero-based argument index; must be less than #Count.
		 * @return The argument string, owned by the C runtime.
		 * @note Asserts in Debug when @p index is out of range.
		 */
		const char* operator[](int index) const
		{
			UG_CORE_ASSERT(index < Count);
			return Args[index];
		}
	};

	/**
	 * @brief Startup configuration supplied by the client application.
	 * @ingroup group_core
	 */
	struct ApplicationSpecification
	{
		std::string Name = "Uge Application"; ///< Window title and application name.
		std::string WorkingDirectory; ///< Directory to switch to on startup; empty leaves it unchanged.
		ApplicationCommandLineArgs CommandLineArgs; ///< Arguments forwarded from `main`.
	};


	/**
	 * @brief Owns the window, the ImGui overlay and the layer stack, and runs the frame loop.
	 * @ingroup group_core
	 *
	 * Exactly one instance may exist; the constructor asserts otherwise and installs it as
	 * the singleton returned by Get(). Clients derive from this class and return an
	 * instance from Uge::CreateApplication.
	 *
	 * Construction switches to ApplicationSpecification::WorkingDirectory when one is set,
	 * creates the window, initializes the renderer, and pushes an Uge::ImGuiLayer overlay.
	 *
	 * @par Frame loop
	 * Each iteration of Run() computes a Uge::Timestep from the elapsed wall time, drains
	 * the main-thread queue, calls Layer::OnUpdate on every layer bottom-to-top (skipped
	 * while minimized), then brackets Layer::OnImGuiRender with the ImGui layer's
	 * begin/end, and finally swaps buffers via Window::OnUpdate.
	 *
	 * @see Layer, LayerStack, Window
	 */
	class Application
	{
	public:
		/**
		 * @brief Creates the window and renderer and installs the singleton instance.
		 * @param spec Startup configuration: name, working directory and command-line arguments.
		 * @note Asserts if an Application already exists.
		 */
		Application(const ApplicationSpecification& spec);
		
		
		/** @brief Shuts down the script engine and the renderer. */
		virtual ~Application();

		/**
		 * @brief Runs the frame loop until CloseProgram() is called or the window is closed.
		 *
		 * Blocks for the lifetime of the program; called by the `main` in EntryPoint.h.
		 */
		void Run();

		/**
		 * @brief Routes an event to the application and then through the layer stack.
		 * @param e The event, mutated as it is handled.
		 *
		 * Window close and resize are handled first, then the event is offered to layers from
		 * top to bottom, stopping as soon as one sets Event::m_handled.
		 */
		void OnEvent(Event& e);

		/**
		 * @brief Adds a normal layer and calls its Layer::OnAttach.
		 * @param layer Layer to insert; the stack takes ownership and deletes it at shutdown.
		 */
		void PushLayer(Layer* layer);
		/**
		 * @brief Adds an overlay above every normal layer and calls its Layer::OnAttach.
		 * @param overlay Layer to insert; the stack takes ownership and deletes it at shutdown.
		 *
		 * Overlays update last but receive events first, which is why the ImGui layer is one.
		 */
		void PushOverlay(Layer* overlay);

		/**
		 * @brief Returns the singleton application instance.
		 * @return Reference to the running application.
		 */
		inline static Application& Get() { return *s_instance; }
		/**
		 * @brief Returns the platform window.
		 * @return Reference to the window owned by this application.
		 */
		inline Window& GetWindow() { return *m_window; }
		/** @brief Requests shutdown; Run() returns after the current frame completes. */
		inline void CloseProgram() { m_running = false; }

		/**
		 * @brief Returns the ImGui overlay pushed during construction.
		 * @return Non-owning pointer to the ImGui layer.
		 */
		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }


		/**
		 * @brief Returns the specification this application was constructed with.
		 * @return Const reference to the stored specification.
		 */
		const ApplicationSpecification& GetSpecifications() const { return m_specification; }

		/**
		 * @brief Queues a callable to run on the main thread at the start of the next frame.
		 * @param func Work to execute; invoked once and then discarded.
		 *
		 * Thread-safe. Use it to marshal work off worker threads onto the thread that owns the
		 * OpenGL context, since renderer calls are only valid there.
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

	// To be defined in a client
	/**
	 * @brief Creates the client application; **implemented by the client, not the engine**.
	 * @param args Command-line arguments forwarded from `main`.
	 * @return A heap-allocated application; `main` deletes it after Run() returns.
	 * @ingroup group_core
	 *
	 * @see UgeEditor/src/UgeEditorApp.cpp for the editor's implementation.
	 */
	Application* CreateApplication(ApplicationCommandLineArgs args);

}