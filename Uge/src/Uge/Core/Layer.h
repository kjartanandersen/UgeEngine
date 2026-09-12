/**
 * @file Layer.h
 * @brief Base class for the units of behaviour held by the layer stack.
 * @ingroup group_core
 */

#pragma once

#include "Uge/Core/Core.h"
#include "Uge/Events/Event.h"

#include "Uge/Core/Timestep.h"

namespace Uge
{


	/**
	 * @brief A named slice of application behaviour that receives updates and events.
	 * @ingroup group_core
	 *
	 * Layers are the engine's primary extension point. A client subclasses Layer, overrides
	 * the callbacks it needs, and hands an instance to Application::PushLayer or
	 * Application::PushOverlay, which takes ownership.
	 *
	 * Every callback has an empty default, so a subclass only overrides what it uses.
	 *
	 * @note Updates run bottom-to-top while events propagate top-to-bottom, so the
	 * most recently pushed layer sees input first.
	 *
	 * @see Uge::EditorLayer for a full example.
	 */
	class Layer
	{

	public:
		/**
		 * @brief Constructs a layer.
		 * @param debugName Name used in logs and profiling output.
		 */
		Layer(const std::string& debugName = "Layer");
		/** @brief Virtual destructor; the layer stack deletes layers through this pointer. */
		virtual ~Layer();

		/**
		 * @brief Called once when the layer is pushed onto the stack.
		 *
		 * The window and renderer are already initialized here, so this is the right place to
		 * create framebuffers, load assets and set up state.
		 */
		virtual void OnAttach() {}
		/** @brief Called once when the layer is popped, before it is deleted. */
		virtual void OnDetach() {}
		/**
		 * @brief Called once per frame to advance and draw the layer.
		 * @param timestep Seconds elapsed since the previous frame.
		 * @note Not called while the window is minimized.
		 */
		virtual void OnUpdate(Timestep timestep) {}
		/**
		 * @brief Called once per frame inside the ImGui frame to emit UI.
		 *
		 * ImGui begin/end are handled by the ImGui layer, so this only issues widget calls.
		 */
		virtual void OnImGuiRender() {}
		/**
		 * @brief Called when an event reaches this layer.
		 * @param event The event; set Event::m_handled to stop it reaching lower layers.
		 */
		virtual void OnEvent(Event& event) {}

		/**
		 * @brief Returns the debug name given at construction.
		 * @return Const reference to the layer's name.
		 */
		inline const std::string& GetName() const { return m_debugName; }

	protected:
		std::string m_debugName; ///< Name used in logs and profiling output.
	};


}

