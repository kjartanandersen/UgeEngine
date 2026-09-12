/**
 * @file DebugPanel.h
 * @brief Editor panel showing frame timings, renderer counters and scene diagnostics.
 * @ingroup group_editor
 */

#pragma once

#include <cstdint>

#include "Uge/Core/Core.h"
#include "Uge/Scene/Entity.h"

namespace Uge
{

	class Scene;

	/**
	 * @brief Live diagnostics for the running editor.
	 * @ingroup group_editor
	 *
	 * Brings together what the engine already measures but never showed: the frame time
	 * that Uge::Application computes each frame, the Uge::RenderStats counters every draw
	 * path feeds, and the per-scope timings Uge::FrameProfiler aggregates from the
	 * `UG_PROFILE_*` macros. It also drives on-demand Chrome-trace capture, which is
	 * otherwise not reachable at runtime.
	 */
	class DebugPanel
	{
	public:
		/** @brief Draws the panel, and any ImGui tool windows it has enabled. */
		void OnImGuiRender();

		/**
		 * @brief Points the panel at the scene whose statistics it should report.
		 * @param context Scene to inspect; may be null.
		 */
		void SetContext(const Ref<Scene>& context);

		/**
		 * @brief Supplies the entity under the cursor, recomputed by the editor each frame.
		 * @param entity The hovered entity, or a null entity when the cursor is over nothing.
		 */
		void SetHoveredEntity(Entity entity);

		/**
		 * @brief Advances an in-progress trace capture; call once per frame.
		 *
		 * Capture is measured in frames rather than seconds so that a trace covers a known
		 * number of frames regardless of how slow they are.
		 */
		void OnUpdate();

	private:
		/** @brief Frame time, FPS and the frame-time graph. */
		void DrawFrameSection();
		/** @brief Draw calls, triangles and per-path counters, plus the GPU description. */
		void DrawRendererSection();
		/** @brief Entity and asset counts, and the hovered entity. */
		void DrawSceneSection();
		/** @brief Sortable per-scope timing table and the trace capture control. */
		void DrawProfilerSection();
		/** @brief Toggles for ImGui's own demo, metrics and style windows. */
		void DrawToolsSection();

		Ref<Scene> m_context;
		Entity m_hoveredEntity;

		int m_captureFramesRemaining = 0; ///< Frames left in the active trace capture.
		int m_captureFrameCount = 120;    ///< Frames the next capture should cover.

		bool m_showImGuiDemo = false;
		bool m_showImGuiMetrics = false;
		bool m_showImGuiStyleEditor = false;
	};

}
