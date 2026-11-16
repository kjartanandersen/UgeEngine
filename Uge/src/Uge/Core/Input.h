/**
 * @file Input.h
 * @brief Polled keyboard and mouse state.
 * @ingroup group_core
 */

#pragma once

#include "Uge/Core/Core.h"

namespace Uge
{

	/**
	 * @brief Polls the keyboard and mouse.
	 * @ingroup group_core
	 *
	 * Needs the window, so it can't be used before the application exists.
	 */
	class Input
	{

	public:

		/**
		 * @brief Whether a key is down.
		 * @param keyCode A `UG_KEY_*` code.
		 * @return `true` while it is down.
		 */
		static bool IsKeyPressed(int keyCode);
		/**
		 * @brief Whether a mouse button is down.
		 * @param button A `UG_MOUSE_BUTTON_*` code.
		 * @return `true` while it is down.
		 */
		static bool IsMouseButtonPressed(int button);

		/**
		 * @brief The cursor's x position.
		 * @return Pixels from the window's left edge.
		 */
		static float GetMouseX();
		/**
		 * @brief The cursor's y position.
		 * @return Pixels from the window's top edge.
		 */
		static float GetMouseY();
		/**
		 * @brief The cursor position.
		 * @return `(x, y)` in pixels from the window's top-left corner.
		 */
		static std::pair<float, float> GetMousePos();

	};




}


