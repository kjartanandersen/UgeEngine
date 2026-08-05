#include <ugpch.h>
#include "DebugPanel.h"

#include "Uge/Asset/EditorAssetManager.h"
#include "Uge/Debug/CrashHandler.h"
#include "Uge/Debug/FrameProfiler.h"
#include "Uge/Project/Project.h"
#include "Uge/Renderer/GraphicsContext.h"
#include "Uge/Renderer/RenderStats.h"
#include "Uge/Scene/Components.h"
#include "Uge/Scene/Scene.h"

#include <imgui.h>

namespace Uge
{

	namespace
	{
		constexpr const char* s_traceFilePath = "UgeProfile-Capture.json";

		/** @brief Draws a `label: value` row using the table currently being built. */
		void StatRow(const char* label, const std::string& value)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextUnformatted(label);
			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted(value.c_str());
		}

		/** @brief Draws a `label: value` row for an unsigned counter. */
		void StatRow(const char* label, uint32_t value)
		{
			StatRow(label, std::to_string(value));
		}
	}

	void DebugPanel::SetContext(const Ref<Scene>& context)
	{
		m_context = context;
		m_hoveredEntity = {};
	}

	void DebugPanel::SetHoveredEntity(Entity entity)
	{
		m_hoveredEntity = entity;
	}

	void DebugPanel::OnUpdate()
	{
		if (m_captureFramesRemaining <= 0)
		{
			return;
		}

		if (--m_captureFramesRemaining == 0)
		{
			UG_PROFILE_END_SESSION();
			UG_INFO("Profiler trace written to '{0}'", s_traceFilePath);
		}
	}

	void DebugPanel::OnImGuiRender()
	{
		UG_PROFILE_FUNCTION();

		// Not "Debug": that is the title of ImGui's own internal fallback window, and two
		// tabs with the same label are indistinguishable once docked.
		ImGui::Begin("Diagnostics");

		if (ImGui::CollapsingHeader("Frame", ImGuiTreeNodeFlags_DefaultOpen))
		{
			DrawFrameSection();
		}

		if (ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen))
		{
			DrawRendererSection();
		}

		if (ImGui::CollapsingHeader("Scene"))
		{
			DrawSceneSection();
		}

		if (ImGui::CollapsingHeader("Profiler"))
		{
			DrawProfilerSection();
		}

		if (ImGui::CollapsingHeader("Tools"))
		{
			DrawToolsSection();
		}

		ImGui::End();

		if (m_showImGuiDemo)
		{
			ImGui::ShowDemoWindow(&m_showImGuiDemo);
		}
		if (m_showImGuiMetrics)
		{
			ImGui::ShowMetricsWindow(&m_showImGuiMetrics);
		}
		if (m_showImGuiStyleEditor)
		{
			ImGui::Begin("Style Editor", &m_showImGuiStyleEditor);
			ImGui::ShowStyleEditor();
			ImGui::End();
		}
	}

	void DebugPanel::DrawFrameSection()
	{
		const float frameMs = FrameProfiler::GetLastFrameMs();
		const float averageMs = FrameProfiler::GetAverageFrameMs();

		ImGui::Text("Frame time: %.3f ms (%.1f FPS)", frameMs, frameMs > 0.0f ? 1000.0f / frameMs : 0.0f);
		ImGui::Text("Average:    %.3f ms (%.1f FPS)", averageMs, averageMs > 0.0f ? 1000.0f / averageMs : 0.0f);

		const std::vector<float> history = FrameProfiler::GetFrameTimeHistory();
		if (!history.empty())
		{
			const float peak = *std::max_element(history.begin(), history.end());

			// Scaled to the peak so a spike is visible rather than clipped, but never
			// below a 33 ms ceiling or an idle scene's noise fills the plot.
			ImGui::PlotLines("##frametimes", history.data(), (int)history.size(), 0,
				nullptr, 0.0f, std::max(peak * 1.1f, 33.0f), ImVec2(0.0f, 60.0f));
		}
	}

	void DebugPanel::DrawRendererSection()
	{
		const RenderStats& stats = RenderStats::Get();

		if (ImGui::BeginTable("##renderStats", 2, ImGuiTableFlags_SizingStretchProp))
		{
			StatRow("Draw calls", stats.DrawCalls);
			StatRow("Indices", stats.IndexCount);
			StatRow("Triangles", stats.TriangleCount);
			StatRow("2D quads", stats.Quad2DCount);
			StatRow("Text quads", stats.Text2DQuadCount);
			StatRow("Mesh draws", stats.MeshDrawCount);
			ImGui::EndTable();
		}

		ImGui::Separator();

		const GraphicsDeviceInfo& device = GraphicsContext::GetDeviceInfo();
		if (ImGui::BeginTable("##deviceInfo", 2, ImGuiTableFlags_SizingStretchProp))
		{
			StatRow("Vendor", device.Vendor);
			StatRow("Device", device.Renderer);
			StatRow("API", device.Version);
			StatRow("GLSL", device.ShadingLanguage);
			StatRow("Debug output", device.DebugOutputEnabled ? "enabled" : "disabled");
			ImGui::EndTable();
		}
	}

	void DebugPanel::DrawSceneSection()
	{
		if (!m_context)
		{
			ImGui::TextDisabled("No scene.");
			return;
		}

		std::string hovered = "None";
		if (m_hoveredEntity)
		{
			hovered = m_hoveredEntity.GetComponent<TagComponent>().Tag;
		}

		if (ImGui::BeginTable("##sceneStats", 2, ImGuiTableFlags_SizingStretchProp))
		{
			// Every entity carries an IDComponent, so this view is the whole scene.
			StatRow("Entities", (uint32_t)m_context->GetAllEntitiesWith<IDComponent>().size());
			StatRow("Hovered entity", hovered);

			if (Project::GetActive())
			{
				const auto loaded = Project::GetActive()->GetEditorAssetManager()->GetLoadedAssetsNames();
				StatRow("Loaded assets", (uint32_t)loaded.size());
			}

			ImGui::EndTable();
		}
	}

	void DebugPanel::DrawProfilerSection()
	{
		if (m_captureFramesRemaining > 0)
		{
			ImGui::Text("Capturing... %d frames left", m_captureFramesRemaining);
		}
		else
		{
			ImGui::SetNextItemWidth(120.0f);
			ImGui::DragInt("##captureFrames", &m_captureFrameCount, 1.0f, 1, 6000, "%d frames");
			ImGui::SameLine();

			if (ImGui::Button("Capture trace"))
			{
				UG_PROFILE_BEGIN_SESSION("Capture", s_traceFilePath);
				m_captureFramesRemaining = m_captureFrameCount;
			}

			ImGui::SameLine();
			ImGui::TextDisabled("(?)");
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Writes '%s' next to the editor.\n"
					"Open it in chrome://tracing or ui.perfetto.dev.", s_traceFilePath);
			}
		}

		ImGui::Separator();

		const std::vector<ProfileEntry> entries = FrameProfiler::GetLastFrame();
		if (entries.empty())
		{
			ImGui::TextDisabled("No timings. Profiling is compiled out in Dist builds.");
			return;
		}

		const float frameMs = FrameProfiler::GetLastFrameMs();

		constexpr ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg
			| ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable;

		if (ImGui::BeginTable("##profilerScopes", 4, flags, ImVec2(0.0f, 260.0f)))
		{
			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableSetupColumn("Scope", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("ms", ImGuiTableColumnFlags_WidthFixed, 70.0f);
			ImGui::TableSetupColumn("Calls", ImGuiTableColumnFlags_WidthFixed, 55.0f);
			ImGui::TableSetupColumn("% frame", ImGuiTableColumnFlags_WidthFixed, 70.0f);
			ImGui::TableHeadersRow();

			// Already sorted by descending cost when the frame was closed.
			for (const ProfileEntry& entry : entries)
			{
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				ImGui::TextUnformatted(entry.Name.c_str());

				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%.3f", entry.TotalMs);

				ImGui::TableSetColumnIndex(2);
				ImGui::Text("%u", entry.Calls);

				ImGui::TableSetColumnIndex(3);
				ImGui::Text("%.1f", frameMs > 0.0f ? (entry.TotalMs / frameMs) * 100.0f : 0.0f);
			}

			ImGui::EndTable();
		}
	}

	void DebugPanel::DrawToolsSection()
	{
		ImGui::Checkbox("ImGui demo window", &m_showImGuiDemo);
		ImGui::Checkbox("ImGui metrics window", &m_showImGuiMetrics);
		ImGui::Checkbox("ImGui style editor", &m_showImGuiStyleEditor);

		ImGui::Separator();
		ImGui::TextDisabled("Log file: %s", Log::GetLogFilePath());
		ImGui::TextDisabled("Crash dumps: %s", CrashHandler::GetDumpDirectory());
	}

}
