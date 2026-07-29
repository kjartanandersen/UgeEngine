/**
 * @file MouseButtonCodes.h
 * @brief Mouse button identifiers, mirroring the GLFW button codes.
 * @ingroup group_core
 */

#pragma once

namespace Uge
{
	// From glfw3.h
	/**
	 * @brief Platform-independent mouse button identifiers.
	 * @ingroup group_core
	 *
	 * Values match the `GLFW_MOUSE_BUTTON_*` constants. The numbered aliases and the named
	 * ones overlap: `UG_MOUSE_BUTTON_LEFT` is `UG_MOUSE_BUTTON_1`, and so on. Prefer the
	 * named forms.
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
		UG_MOUSE_BUTTON_LAST = UG_MOUSE_BUTTON_8, ///< Highest valid button index.
		UG_MOUSE_BUTTON_LEFT = UG_MOUSE_BUTTON_1, ///< Primary (left) button.
		UG_MOUSE_BUTTON_RIGHT = UG_MOUSE_BUTTON_2, ///< Secondary (right) button.
		UG_MOUSE_BUTTON_MIDDLE = UG_MOUSE_BUTTON_3 ///< Middle button / wheel click.
	};

}



