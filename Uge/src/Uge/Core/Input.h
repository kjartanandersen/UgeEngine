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
	 * @brief Static interface for querying live input state.
	 * @ingroup group_core
	 *
	 * This is the *polling* half of input handling: it answers "is this key down right
	 * now?" and is meant for continuous actions such as camera movement. For discrete
	 * actions, handle the events in @ref group_events instead, which report each press
	 * exactly once and can be consumed by a layer.
	 *
	 * Key and button arguments come from KeyCodes.h and MouseButtonCodes.h.
	 *
	 * @code
	 * if (Input::IsKeyPressed(UG_KEY_W))
	 *     m_position.y += m_speed * ts;
	 * @endcode
	 *
	 * @note Implemented per platform; see `Platform/Windows/WIndowsInput.cpp`. Requires a
	 * window to exist, so do not call it before the application is constructed.
	 */
	class Input
	{

	public:
		
		/**
		 * @brief Tests whether a key is currently held down.
		 * @param keyCode A `UG_KEY_*` code from KeyCodes.h.
		 * @return `true` while the key is down.
		 */
		static bool IsKeyPressed(int keyCode);
		/**
		 * @brief Tests whether a mouse button is currently held down.
		 * @param button A `UG_MOUSE_BUTTON_*` code from MouseButtonCodes.h.
		 * @return `true` while the button is down.
		 */
		static bool IsMouseButtonPressed(int button);

		/**
		 * @brief Cursor x position.
		 * @return X coordinate in pixels, relative to the window's top-left corner.
		 */
		static float GetMouseX();
		/**
		 * @brief Cursor y position.
		 * @return Y coordinate in pixels, relative to the window's top-left corner.
		 */
		static float GetMouseY();
		/**
		 * @brief Cursor position.
		 * @return An `(x, y)` pair in pixels, relative to the window's top-left corner.
		 */
		static std::pair<float, float> GetMousePos();

	};




}


