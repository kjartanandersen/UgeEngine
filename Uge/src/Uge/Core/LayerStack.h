/**
 * @file LayerStack.h
 * @brief The application's layers and overlays.
 * @ingroup group_core
 */

#pragma once

#include "Uge/Core/Core.h"
#include "Layer.h"

#include <vector>

namespace Uge

{

	/**
	 * @brief Owns the layers, with every overlay above every normal layer.
	 * @ingroup group_core
	 *
	 * Iteration runs bottom to top. The stack deletes its layers, so push only
	 * heap-allocated ones and don't delete a pushed layer.
	 */
	class LayerStack
	{
	public:

		/** @brief An empty stack. */
		LayerStack();
		/** @brief Detaches and deletes the remaining layers. */
		~LayerStack();

		/**
		 * @brief Adds a layer above the other layers and below the overlays.
		 * @param layer Layer to add; the stack owns it.
		 *
		 * Doesn't call Layer::OnAttach.
		 */
		void PushLayer(Layer* layer);
		/**
		 * @brief Adds an overlay on top.
		 * @param overlay Layer to add; the stack owns it.
		 *
		 * Doesn't call Layer::OnAttach.
		 */
		void PushOverlay(Layer* overlay);
		/**
		 * @brief Removes a layer and detaches it.
		 * @param layer Layer to remove; the caller owns it again. Ignored unless it is a
		 * normal layer in the stack.
		 */
		void PopLayer(Layer* layer);
		/**
		 * @brief Removes an overlay, without detaching it.
		 * @param overlay Overlay to remove; the caller owns it again. Ignored unless it is an
		 * overlay in the stack.
		 */
		void PopOverlay(Layer* overlay);

		/** @brief Detaches and deletes every layer, then every overlay. */
		void Clear();

		/**
		 * @brief The bottom layer.
		 * @return Begin iterator.
		 */
		std::vector<Layer*>::iterator begin() { return m_layers.begin(); }
		/**
		 * @brief Past the top overlay.
		 * @return End iterator.
		 */
		std::vector<Layer*>::iterator end() { return m_layers.end(); }


	private:
		std::vector<Layer*> m_layers;
		unsigned int m_layerInsertIndex = 0;


	};


}

