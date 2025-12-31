#pragma once

#include "Uge/Core.h"
#include "Uge/Events/Event.h"

#include "Uge/Core/Timestep.h"

namespace Uge
{


	class Layer
	{

	public:
		Layer(const std::string& debugName = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(Timestep timestep) {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& event) {}

		inline const std::string& GetName() const { return m_debugName; }

	protected:
		std::string m_debugName;
	};


}

