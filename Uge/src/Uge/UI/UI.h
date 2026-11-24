/**
 * @file UI.h
 * @brief ImGui helpers.
 * @ingroup group_ui
 */

#pragma once

#include "imgui.h"

namespace Uge::UI
{

	/**
	 * @brief Pushes an ImGui style colour and pops it at the end of the scope.
	 * @ingroup group_ui
	 */
	struct ScopedStyleColor
	{
		/** @brief A guard that pushes nothing. */
		ScopedStyleColor() = default;

		/**
		 * @brief Pushes a style colour.
		 * @param index ImGui colour slot.
		 * @param color RGBA colour.
		 * @param predicate When `false`, nothing is pushed or popped.
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
		 * @brief Pushes a style colour.
		 * @param index ImGui colour slot.
		 * @param color Colour packed with `IM_COL32`.
		 * @param predicate When `false`, nothing is pushed or popped.
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