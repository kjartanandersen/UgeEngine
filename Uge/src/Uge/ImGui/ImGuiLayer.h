#pragma once


#include "Uge/Layer.h"

#include <Uge/Events/MouseEvent.h>
#include <Uge/Events/KeyEvent.h>
#include <Uge/Events/ApplicationEvent.h>

namespace Uge
{

	class UG_API ImGuiLayer : public Layer
	{

	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		void Begin();
		void End();


	private:

		float m_time = 0.0f;
		


	};


}


