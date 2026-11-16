/**
 * @file ImGuiLayer.h
 * @brief The Dear ImGui overlay.
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
	 * @brief Owns the Dear ImGui context and starts and ends its frames.
	 * @ingroup group_core
	 *
	 * Docking and viewports are on.
	 */
	class ImGuiLayer : public Layer
	{

	public:
		/** @brief Creates the layer. The ImGui context is created in OnAttach(). */
		ImGuiLayer();
		/** @brief Destroys the layer. */
		~ImGuiLayer();

		/** @brief Creates the ImGui context, sets the style and starts the GLFW and OpenGL backends. */
		virtual void OnAttach() override;
		/** @brief Shuts down the ImGui backends and destroys the context. */
		virtual void OnDetach() override;
		/** @brief Draws nothing. */
		virtual void OnImGuiRender() override;
		/**
		 * @brief With blocking on, marks mouse and keyboard events handled while ImGui uses them.
		 * @param event The event.
		 */
		virtual void OnEvent(Event& event) override;

		/** @brief Starts an ImGui frame. */
		void Begin();
		/** @brief Ends the ImGui frame and renders it, including any extra platform windows. */
		void End();

		/**
		 * @brief Sets whether ImGui keeps the input it uses from lower layers.
		 * @param block `true` to keep it.
		 */
		void BlockEvents(bool block) { m_blockEvents = block; };

		/** @brief Applies the dark colour scheme. */
		void SetDarkThemeColors();

		/**
		 * @brief The ImGui ID of the widget in use.
		 * @return The ID, or `0` if none.
		 */
		uint32_t GetActiveWidgetID() const;
	private:


	private:

		bool m_blockEvents = false;

		float m_time = 0.0f;

	};


}


