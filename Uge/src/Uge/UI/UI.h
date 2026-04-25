#pragma once

#include "imgui.h"

namespace Uge::UI
{

	struct ScopedStyleColor
	{
		ScopedStyleColor() = default;

		ScopedStyleColor(ImGuiCol index, ImVec4 color, bool predicate = true)
			: m_set(predicate)
		{
			if (predicate)
			{
				ImGui::PushStyleColor(index, color);

			}



		}
		ScopedStyleColor(ImGuiCol index, ImU32 color, bool predicate = true)
			: m_set(predicate)

		{
			if (predicate)
			{
				ImGui::PushStyleColor(index, color);

			}

		}

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