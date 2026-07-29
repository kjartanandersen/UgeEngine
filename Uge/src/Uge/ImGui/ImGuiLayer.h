/**
 * @file ImGuiLayer.h
 * @brief The overlay that hosts Dear ImGui and brackets every frame's UI.
 * @ingroup group_core
 */

#pragma once


#include "Uge/Core/Layer.h"

#include "imgui.h"
#include <Uge/Events/MouseEvent.h>
#include <Uge/Events/KeyEvent.h>
#include <Uge/Events/ApplicationEvent.h>

namespace Uge
{

	/**
	 * @brief Owns the Dear ImGui context and drives its per-frame begin/end.
	 * @ingroup group_core
	 *
	 * Uge::Application creates one during construction and pushes it as an overlay, so it
	 * sits above every other layer and sees input first. Each frame the application calls
	 * Begin(), lets every layer emit widgets from Uge::Layer::OnImGuiRender, then calls
	 * End() to render the draw data.
	 *
	 * Docking and viewports are enabled, which is what allows the editor's panels to be
	 * rearranged and torn off.
	 *
	 * @note Clients do not create this layer themselves; reach it through
	 * Uge::Application::GetImGuiLayer.
	 */
	class ImGuiLayer : public Layer
	{

	public:
		/** @brief Constructs the layer. The ImGui context is created later, in OnAttach(). */
		ImGuiLayer();
		/** @brief Destroys the layer. */
		~ImGuiLayer();

		/**
		 * @brief Creates the ImGui context, loads fonts and initializes the GLFW/OpenGL backends.
		 */
		virtual void OnAttach() override;
		/** @brief Shuts down the ImGui backends and destroys the context. */
		virtual void OnDetach() override;
		/** @brief Emits this layer's own UI; unused by default. */
		virtual void OnImGuiRender() override;
		/**
		 * @brief Optionally consumes events that ImGui wants.
		 * @param event Event being propagated.
		 *
		 * When event blocking is on, mouse and keyboard events are marked handled whenever
		 * ImGui has captured the corresponding device, which stops them reaching the scene
		 * below. @see BlockEvents
		 */
		virtual void OnEvent(Event& event) override;

		/**
		 * @brief Starts a new ImGui frame.
		 *
		 * Called by Uge::Application::Run before any layer's Uge::Layer::OnImGuiRender.
		 */
		void Begin();
		/**
		 * @brief Ends the ImGui frame and renders its draw data.
		 *
		 * Also updates and renders the additional platform windows when viewports are on.
		 */
		void End();

		/**
		 * @brief Controls whether events ImGui captures are hidden from lower layers.
		 * @param block `true` to let ImGui swallow input it is using.
		 *
		 * The editor turns blocking off while the viewport is focused, so camera controls keep
		 * working over the scene image, and back on elsewhere so typing in a field does not
		 * also trigger scene shortcuts.
		 */
		void BlockEvents(bool block) { m_blockEvents = block; };

		/** @brief Applies the engine's dark colour scheme to the current ImGui style. */
		void SetDarkThemeColors();

		/**
		 * @brief Returns the ImGui ID of the widget currently being interacted with.
		 * @return The active widget's ID, or `0` when none is active.
		 *
		 * Used to suppress global keyboard shortcuts while a text field has focus.
		 */
		uint32_t GetActiveWidgetID() const;
	private:


	private:

		bool m_blockEvents = false;

		float m_time = 0.0f;
		ImFont* mainFont;


	};


}


