/**
 * @file ConsolePanel.h
 * @brief The log console panel.
 * @ingroup group_editor
 */

#pragma once

#include <imgui.h>

#include <cstdint>
#include <vector>

#include "Uge/Debug/LogBuffer.h"

namespace Uge
{

	/**
	 * @brief Shows the Uge::LogBuffer records, coloured by level, filtered by level and text.
	 * @ingroup group_editor
	 */
	class ConsolePanel
	{
	public:
		/** @brief Draws the panel. */
		void OnImGuiRender();

	private:
		/** @brief Copies the buffer if its version changed, and filters again if needed. */
		void RefreshCache();

		/** @brief Draws the clear, auto-scroll, level and search controls. */
		void DrawToolbar();

		std::vector<LogEntry> m_entries;   ///< A copy of the buffer.
		std::vector<size_t> m_visible;     ///< Indices of the entries that pass the filters.
		uint64_t m_cachedVersion = 0;      ///< The buffer version m_entries was copied at.
		bool m_filtersDirty = true;        ///< Whether a filter changed since the last filtering.

		/** @brief Which levels are shown, by `spdlog::level::level_enum`. */
		bool m_levelEnabled[spdlog::level::n_levels] = { true, true, true, true, true, true, true };

		ImGuiTextFilter m_textFilter;      ///< The search box.
		bool m_autoScroll = true;          ///< Whether to follow new records.
	};

}
