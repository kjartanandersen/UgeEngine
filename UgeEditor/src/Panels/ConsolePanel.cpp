#include <ugpch.h>
#include "ConsolePanel.h"

#include "Uge/UI/UI.h"

namespace Uge
{

	namespace
	{
		/** @brief Short label shown on the level filter buttons. */
		const char* LevelToLabel(spdlog::level::level_enum level)
		{
			switch (level)
			{
				case spdlog::level::trace:    return "Trace";
				case spdlog::level::debug:    return "Debug";
				case spdlog::level::info:     return "Info";
				case spdlog::level::warn:     return "Warn";
				case spdlog::level::err:      return "Error";
				case spdlog::level::critical: return "Fatal";
				default:                      return "Off";
			}
		}

		/** @brief Text colour used for a record of the given severity. */
		ImVec4 LevelToColor(spdlog::level::level_enum level)
		{
			switch (level)
			{
				case spdlog::level::trace:    return ImVec4(0.55f, 0.55f, 0.58f, 1.0f);
				case spdlog::level::debug:    return ImVec4(0.45f, 0.75f, 0.85f, 1.0f);
				case spdlog::level::info:     return ImVec4(0.85f, 0.86f, 0.88f, 1.0f);
				case spdlog::level::warn:     return ImVec4(0.95f, 0.77f, 0.25f, 1.0f);
				case spdlog::level::err:      return ImVec4(0.92f, 0.35f, 0.32f, 1.0f);
				case spdlog::level::critical: return ImVec4(1.00f, 0.35f, 0.75f, 1.0f);
				default:                      return ImVec4(0.85f, 0.86f, 0.88f, 1.0f);
			}
		}

		// Levels offered as filter buttons; `off` is never used as a record severity.
		constexpr spdlog::level::level_enum s_filterLevels[] = {
			spdlog::level::trace,
			spdlog::level::debug,
			spdlog::level::info,
			spdlog::level::warn,
			spdlog::level::err,
			spdlog::level::critical
		};
	}

	void ConsolePanel::OnImGuiRender()
	{
		UG_PROFILE_FUNCTION();

		ImGui::Begin("Console");

		DrawToolbar();
		ImGui::Separator();

		RefreshCache();

		if (ImGui::BeginChild("##consoleScroll", ImVec2(0.0f, 0.0f), false,
			ImGuiWindowFlags_HorizontalScrollbar))
		{
			// The buffer holds thousands of records, so only the visible rows are built.
			ImGuiListClipper clipper;
			clipper.Begin((int)m_visible.size());

			while (clipper.Step())
			{
				for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
				{
					const LogEntry& entry = m_entries[m_visible[row]];

					UI::ScopedStyleColor color(ImGuiCol_Text, LevelToColor(entry.Level));
					ImGui::TextUnformatted(
						("[" + entry.Time + "] " + entry.Logger + ": " + entry.Message).c_str());
				}
			}

			// Only follow the tail while the view is already pinned to the bottom, so
			// scrolling back to read something does not fight the incoming records.
			if (m_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
			{
				ImGui::SetScrollHereY(1.0f);
			}
		}
		ImGui::EndChild();

		ImGui::End();
	}

	void ConsolePanel::DrawToolbar()
	{
		if (ImGui::Button("Clear"))
		{
			LogBuffer::Clear();
		}

		ImGui::SameLine();
		ImGui::Checkbox("Auto-scroll", &m_autoScroll);

		ImGui::SameLine();
		ImGui::TextUnformatted("|");

		for (spdlog::level::level_enum level : s_filterLevels)
		{
			ImGui::SameLine();

			const bool enabled = m_levelEnabled[level];
			const ImVec4 color = LevelToColor(level);

			// Enabled filters are tinted with their level colour; disabled ones stay grey.
			UI::ScopedStyleColor text(ImGuiCol_Text, enabled ? color : ImVec4(0.45f, 0.45f, 0.48f, 1.0f));

			char label[64];
			std::snprintf(label, sizeof(label), "%s (%u)", LevelToLabel(level), LogBuffer::GetCount(level));

			if (ImGui::SmallButton(label))
			{
				m_levelEnabled[level] = !enabled;
				m_filtersDirty = true;
			}
		}

		ImGui::SameLine();
		ImGui::SetNextItemWidth(220.0f);
		if (m_textFilter.Draw("##consoleFilter"))
		{
			m_filtersDirty = true;
		}

		ImGui::SameLine();
		ImGui::TextDisabled("(search)");
	}

	void ConsolePanel::RefreshCache()
	{
		const uint64_t version = LogBuffer::GetVersion();
		const bool bufferChanged = version != m_cachedVersion;

		if (!bufferChanged && !m_filtersDirty)
		{
			return;
		}

		if (bufferChanged)
		{
			m_entries = LogBuffer::Snapshot();
			m_cachedVersion = version;
		}

		m_visible.clear();
		m_visible.reserve(m_entries.size());

		for (size_t i = 0; i < m_entries.size(); i++)
		{
			const LogEntry& entry = m_entries[i];

			if (!m_levelEnabled[entry.Level])
			{
				continue;
			}
			if (!m_textFilter.PassFilter(entry.Message.c_str()))
			{
				continue;
			}

			m_visible.push_back(i);
		}

		m_filtersDirty = false;
	}

}
