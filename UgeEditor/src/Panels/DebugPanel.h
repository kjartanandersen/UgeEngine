/**
 * @file DebugPanel.h
 * @brief The diagnostics panel.
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
	 * @brief Shows frame times, Uge::RenderStats, Uge::FrameProfiler timings and scene
	 * counts, and captures Chrome traces.
	 * @ingroup group_editor
	 */
	class DebugPanel
	{
	public:
		/** @brief Draws the panel, and the ImGui tool windows that are turned on. */
		void OnImGuiRender();

		/**
		 * @brief Sets the scene to report on.
		 * @param context The scene, or null.
		 */
		void SetContext(const Ref<Scene>& context);

		/**
		 * @brief Sets the entity under the cursor.
		 * @param entity The entity, or a null entity.
		 */
		void SetHoveredEntity(Entity entity);

		/**
		 * @brief Counts down a trace capture, which lasts a number of frames. Call once per
		 * frame.
		 */
		void OnUpdate();

	private:
		/** @brief Frame time, FPS and the frame-time graph. */
		void DrawFrameSection();
		/** @brief Render counters and the GPU's description. */
		void DrawRendererSection();
		/** @brief Entity and asset counts, and the hovered entity. */
		void DrawSceneSection();
		/** @brief The timing table and the trace capture controls. */
		void DrawProfilerSection();
		/** @brief Toggles for ImGui's demo, metrics and style windows. */
		void DrawToolsSection();

		Ref<Scene> m_context;
		Entity m_hoveredEntity;

		int m_captureFramesRemaining = 0; ///< Frames left in the capture.
		int m_captureFrameCount = 120;    ///< Frames the next capture lasts.

		bool m_showImGuiDemo = false;
		bool m_showImGuiMetrics = false;
		bool m_showImGuiStyleEditor = false;
	};

}
