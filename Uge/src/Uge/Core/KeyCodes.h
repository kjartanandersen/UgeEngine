/**
 * @file KeyCodes.h
 * @brief Keyboard key identifiers, mirroring the GLFW key codes.
 * @ingroup group_core
 */

#pragma once

/* Printable keys */
// From glfw3.h

namespace Uge
{
	/**
	 * @brief Platform-independent keyboard key identifiers.
	 * @ingroup group_core
	 *
	 * Values match the `GLFW_KEY_*` constants so the Windows backend can pass them
	 * through unchanged. Use these with Uge::Input::IsKeyPressed and when inspecting
	 * Uge::KeyEvent::GetKeyCode.
	 *
	 * Printable keys use their ASCII value, so `UG_KEY_A` is `65`. Function and modifier
	 * keys start at 256.
	 */
	enum KeyCode
	{
		UG_KEY_SPACE = 32,
		UG_KEY_APOSTROPHE = 39,  /* ' */
		UG_KEY_COMMA = 44,  /* , */
		UG_KEY_MINUS = 45,  /* - */
		UG_KEY_PERIOD = 46,  /* . */
		UG_KEY_SLASH = 47,  /* / */
		UG_KEY_0 = 48,
		UG_KEY_1 = 49,
		UG_KEY_2 = 50,
		UG_KEY_3 = 51,
		UG_KEY_4 = 52,
		UG_KEY_5 = 53,
		UG_KEY_6 = 54,
		UG_KEY_7 = 55,
		UG_KEY_8 = 56,
		UG_KEY_9 = 57,
		UG_KEY_SEMICOLON = 59,  /* ; */
		UG_KEY_EQUAL = 61,  /* = */
		UG_KEY_A = 65,
		UG_KEY_B = 66,
		UG_KEY_C = 67,
		UG_KEY_D = 68,
		UG_KEY_E = 69,
		UG_KEY_F = 70,
		UG_KEY_G = 71,
		UG_KEY_H = 72,
		UG_KEY_I = 73,
		UG_KEY_J = 74,
		UG_KEY_K = 75,
		UG_KEY_L = 76,
		UG_KEY_M = 77,
		UG_KEY_N = 78,
		UG_KEY_O = 79,
		UG_KEY_P = 80,
		UG_KEY_Q = 81,
		UG_KEY_R = 82,
		UG_KEY_S = 83,
		UG_KEY_T = 84,
		UG_KEY_U = 85,
		UG_KEY_V = 86,
		UG_KEY_W = 87,
		UG_KEY_X = 88,
		UG_KEY_Y = 89,
		UG_KEY_Z = 90,
		UG_KEY_LEFT_BRACKET = 91,  /* [ */
		UG_KEY_BACKSLASH = 92,  /* \ */
		UG_KEY_RIGHT_BRACKET = 93,  /* ] */
		UG_KEY_GRAVE_ACCENT = 96,  /* ` */
		UG_KEY_WORLD_1 = 161, /* non-US #1 */
		UG_KEY_WORLD_2 = 162, /* non-US #2 */

		/* Function keys */
		UG_KEY_ESCAPE = 256,
		UG_KEY_ENTER = 257,
		UG_KEY_TAB = 258,
		UG_KEY_BACKSPACE = 259,
		UG_KEY_INSERT = 260,
		UG_KEY_DELETE = 261,
		UG_KEY_RIGHT = 262,
		UG_KEY_LEFT = 263,
		UG_KEY_DOWN = 264,
		UG_KEY_UP = 265,
		UG_KEY_PAGE_UP = 266,
		UG_KEY_PAGE_DOWN = 267,
		UG_KEY_HOME = 268,
		UG_KEY_END = 269,
		UG_KEY_CAPS_LOCK = 280,
		UG_KEY_SCROLL_LOCK = 281,
		UG_KEY_NUM_LOCK = 282,
		UG_KEY_PRINT_SCREEN = 283,
		UG_KEY_PAUSE = 284,
		UG_KEY_F1 = 290,
		UG_KEY_F2 = 291,
		UG_KEY_F3 = 292,
		UG_KEY_F4 = 293,
		UG_KEY_F5 = 294,
		UG_KEY_F6 = 295,
		UG_KEY_F7 = 296,
		UG_KEY_F8 = 297,
		UG_KEY_F9 = 298,
		UG_KEY_F10 = 299,
		UG_KEY_F11 = 300,
		UG_KEY_F12 = 301,
		UG_KEY_F13 = 302,
		UG_KEY_F14 = 303,
		UG_KEY_F15 = 304,
		UG_KEY_F16 = 305,
		UG_KEY_F17 = 306,
		UG_KEY_F18 = 307,
		UG_KEY_F19 = 308,
		UG_KEY_F20 = 309,
		UG_KEY_F21 = 310,
		UG_KEY_F22 = 311,
		UG_KEY_F23 = 312,
		UG_KEY_F24 = 313,
		UG_KEY_F25 = 314,
		UG_KEY_KP_0 = 320,
		UG_KEY_KP_1 = 321,
		UG_KEY_KP_2 = 322,
		UG_KEY_KP_3 = 323,
		UG_KEY_KP_4 = 324,
		UG_KEY_KP_5 = 325,
		UG_KEY_KP_6 = 326,
		UG_KEY_KP_7 = 327,
		UG_KEY_KP_8 = 328,
		UG_KEY_KP_9 = 329,
		UG_KEY_KP_DECIMAL = 330,
		UG_KEY_KP_DIVIDE = 331,
		UG_KEY_KP_MULTIPLY = 332,
		UG_KEY_KP_SUBTRACT = 333,
		UG_KEY_KP_ADD = 334,
		UG_KEY_KP_ENTER = 335,
		UG_KEY_KP_EQUAL = 336,
		UG_KEY_LEFT_SHIFT = 340,
		UG_KEY_LEFT_CONTROL = 341,
		UG_KEY_LEFT_ALT = 342,
		UG_KEY_LEFT_SUPER = 343,
		UG_KEY_RIGHT_SHIFT = 344,
		UG_KEY_RIGHT_CONTROL = 345,
		UG_KEY_RIGHT_ALT = 346,
		UG_KEY_RIGHT_SUPER = 347,
		UG_KEY_MENU = 348
	};

}


