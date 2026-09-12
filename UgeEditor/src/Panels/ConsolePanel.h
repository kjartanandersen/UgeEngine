/**
 * @file ConsolePanel.h
 * @brief Editor panel showing engine and client log output.
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
	 * @brief In-editor log console, backed by Uge::LogBuffer.
	 * @ingroup group_editor
	 *
	 * The engine's `UG_CORE_*` and `UG_*` macros otherwise only reach the terminal, which
	 * is not visible when the editor is run from Explorer or a debugger. This panel shows
	 * the same records, colours them by severity, and lets them be filtered by level and
	 * by text.
	 *
	 * The buffer is only re-copied when Uge::LogBuffer::GetVersion changes, so a quiet
	 * frame costs nothing but the draw.
	 */
	class ConsolePanel
	{
	public:
		/** @brief Draws the panel. */
		void OnImGuiRender();

	private:
		/** @brief Re-copies and re-filters the cached records if the buffer has changed. */
		void RefreshCache();

		/** @brief Draws the toolbar row: clear, auto-scroll, level toggles and search. */
		void DrawToolbar();

		std::vector<LogEntry> m_entries;   ///< Cached copy of the buffer.
		std::vector<size_t> m_visible;     ///< Indices into m_entries passing the filters.
		uint64_t m_cachedVersion = 0;      ///< Buffer version m_entries was copied at.
		bool m_filtersDirty = true;        ///< Set when a filter changes, forcing a re-filter.

		/** @brief Per-level visibility, indexed by `spdlog::level::level_enum`. */
		bool m_levelEnabled[spdlog::level::n_levels] = { true, true, true, true, true, true, true };

		ImGuiTextFilter m_textFilter;      ///< Search box state.
		bool m_autoScroll = true;          ///< Whether to follow the tail of the log.
	};

}
