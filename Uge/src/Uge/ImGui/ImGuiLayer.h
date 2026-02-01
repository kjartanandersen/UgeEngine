#pragma once


#include "Uge/Core/Layer.h"

#include "imgui.h"
#include <Uge/Events/MouseEvent.h>
#include <Uge/Events/KeyEvent.h>
#include <Uge/Events/ApplicationEvent.h>

namespace Uge
{

	class ImGuiLayer : public Layer
	{

	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& event) override;

		void Begin();
		void End();

		void BlockEvents(bool block) { m_blockEvents = block; };

		void SetDarkThemeColors();
	private:


	private:

		bool m_blockEvents = false;

		float m_time = 0.0f;
		ImFont* mainFont;


	};


}


