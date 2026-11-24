/**
 * @file Layer.h
 * @brief Base class of the application's layers.
 * @ingroup group_core
 */

#pragma once

#include "Uge/Core/Core.h"
#include "Uge/Events/Event.h"

#include "Uge/Core/Timestep.h"

namespace Uge
{


	/**
	 * @brief A part of the application that receives updates and events.
	 * @ingroup group_core
	 *
	 * Every callback does nothing by default.
	 */
	class Layer
	{

	public:
		/**
		 * @brief Creates a layer.
		 * @param debugName Name for logs and profiling.
		 */
		Layer(const std::string& debugName = "Layer");
		/** @brief Virtual, because the layer stack deletes layers through a base pointer. */
		virtual ~Layer();

		/** @brief Called once when the layer is added. */
		virtual void OnAttach() {}
		/** @brief Called once when the layer is removed, before it is deleted. */
		virtual void OnDetach() {}
		/**
		 * @brief Called every frame, except while the window is minimized.
		 * @param timestep Seconds since the previous frame.
		 */
		virtual void OnUpdate(Timestep timestep) {}
		/** @brief Called every frame, inside the ImGui frame, to draw UI. */
		virtual void OnImGuiRender() {}
		/**
		 * @brief Called when an event reaches the layer.
		 * @param event The event; set Event::m_handled to keep it from lower layers.
		 */
		virtual void OnEvent(Event& event) {}

		/**
		 * @brief The debug name.
		 * @return The name.
		 */
		inline const std::string& GetName() const { return m_debugName; }

	protected:
		std::string m_debugName; ///< Name for logs and profiling.
	};


}

