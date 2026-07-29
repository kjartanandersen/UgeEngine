/**
 * @file UI.h
 * @brief Reusable ImGui helpers shared by the editor panels.
 * @ingroup group_ui
 */

#pragma once

#include "imgui.h"

namespace Uge::UI
{

	/**
	 * @brief RAII guard that pushes an ImGui style colour and pops it on scope exit.
	 * @ingroup group_ui
	 *
	 * Removes the need to pair every `ImGui::PushStyleColor` with a matching pop on every
	 * exit path, including early returns.
	 *
	 * @code
	 * {
	 *     UI::ScopedStyleColor button(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
	 *     ImGui::Button("Delete");
	 * }   // colour popped here
	 * @endcode
	 *
	 * The `predicate` parameter makes conditional styling straightforward: when it is
	 * `false` nothing is pushed and nothing is popped.
	 */
	struct ScopedStyleColor
	{
		/** @brief Constructs an inactive guard that pushes nothing. */
		ScopedStyleColor() = default;

		/**
		 * @brief Pushes a style colour given as a float RGBA vector.
		 * @param index Which ImGui colour slot to override.
		 * @param color Replacement colour, components in `[0, 1]`.
		 * @param predicate When `false`, the guard does nothing at all.
		 */
		ScopedStyleColor(ImGuiCol index, ImVec4 color, bool predicate = true)
			: m_set(predicate)
		{
			if (predicate)
			{
				ImGui::PushStyleColor(index, color);

			}



		}
		/**
		 * @brief Pushes a style colour given as a packed 32-bit value.
		 * @param index Which ImGui colour slot to override.
		 * @param color Replacement colour, packed as `IM_COL32`.
		 * @param predicate When `false`, the guard does nothing at all.
		 */
		ScopedStyleColor(ImGuiCol index, ImU32 color, bool predicate = true)
			: m_set(predicate)

		{
			if (predicate)
			{
				ImGui::PushStyleColor(index, color);

			}

		}

		/** @brief Pops the style colour if one was pushed. */
		~ScopedStyleColor()
		{
			if (m_set)
			{
				ImGui::PopStyleColor();

			}

		}


	private:
		bool m_set = false;

	};

}