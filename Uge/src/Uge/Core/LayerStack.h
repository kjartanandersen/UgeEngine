/**
 * @file LayerStack.h
 * @brief Ordered, owning container of the application's layers and overlays.
 * @ingroup group_core
 */

#pragma once

#include "Uge/Core/Core.h"
#include "Layer.h"

#include <vector>

namespace Uge

{

	/**
	 * @brief Owns the application's layers, keeping normal layers below all overlays.
	 * @ingroup group_core
	 *
	 * The stack is a single vector partitioned by an insert index: normal layers occupy
	 * `[0, m_layerInsertIndex)` and overlays follow. PushLayer inserts at the partition
	 * point and advances it; PushOverlay appends, so overlays always stay on top.
	 *
	 * Iterating front-to-back gives update order (bottom layer first). The application
	 * iterates in reverse for events, so the topmost layer gets first refusal.
	 *
	 * @warning The stack owns its layers and deletes them in the destructor. Never push a
	 * stack-allocated layer, and never delete a pushed layer yourself.
	 */
	class LayerStack
	{
	public:

		/** @brief Constructs an empty stack. */
		LayerStack();
		/** @brief Calls Layer::OnDetach on every remaining layer and deletes it. */
		~LayerStack();

		/**
		 * @brief Inserts a normal layer above the existing normal layers but below all overlays.
		 * @param layer Layer to insert; ownership transfers to the stack.
		 * @note Does not call Layer::OnAttach; Application::PushLayer does that.
		 */
		void PushLayer(Layer* layer);
		/**
		 * @brief Appends an overlay above every other layer.
		 * @param overlay Layer to insert; ownership transfers to the stack.
		 * @note Does not call Layer::OnAttach; Application::PushOverlay does that.
		 */
		void PushOverlay(Layer* overlay);
		/**
		 * @brief Removes a normal layer, calling its Layer::OnDetach.
		 * @param layer Layer to remove; ownership returns to the caller. Ignored if absent.
		 * @note Only searches the normal-layer partition; overlays are unaffected.
		 */
		void PopLayer(Layer* layer);
		/**
		 * @brief Removes an overlay.
		 * @param overlay Overlay to remove; ownership returns to the caller. Ignored if absent.
		 * @note Only searches the overlay partition.
		 */
		void PopOverlay(Layer* overlay);

		/**
		 * @brief Iterator to the bottom-most layer.
		 * @return Begin iterator, in update order.
		 */
		std::vector<Layer*>::iterator begin() { return m_layers.begin(); }
		/**
		 * @brief Iterator past the top-most overlay.
		 * @return End iterator.
		 */
		std::vector<Layer*>::iterator end() { return m_layers.end(); }


	private:
		std::vector<Layer*> m_layers;
		unsigned int m_layerInsertIndex = 0;


	};


}

