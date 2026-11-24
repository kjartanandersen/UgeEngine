/**
 * @file MouseButtonCodes.h
 * @brief Mouse button codes.
 * @ingroup group_core
 */

#pragma once

namespace Uge
{
	// From glfw3.h
	/**
	 * @brief Mouse buttons, with the same values as GLFW's `GLFW_MOUSE_BUTTON_*` codes.
	 * @ingroup group_core
	 */
	enum MouseButton
	{
		UG_MOUSE_BUTTON_1 = 0,
		UG_MOUSE_BUTTON_2 = 1,
		UG_MOUSE_BUTTON_3 = 2,
		UG_MOUSE_BUTTON_4 = 3,
		UG_MOUSE_BUTTON_5 = 4,
		UG_MOUSE_BUTTON_6 = 5,
		UG_MOUSE_BUTTON_7 = 6,
		UG_MOUSE_BUTTON_8 = 7,
		UG_MOUSE_BUTTON_LAST = UG_MOUSE_BUTTON_8, ///< Highest button.
		UG_MOUSE_BUTTON_LEFT = UG_MOUSE_BUTTON_1,
		UG_MOUSE_BUTTON_RIGHT = UG_MOUSE_BUTTON_2,
		UG_MOUSE_BUTTON_MIDDLE = UG_MOUSE_BUTTON_3 ///< Wheel click.
	};

}



