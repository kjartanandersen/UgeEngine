/**
 * @file RendererSettingsPanel.h
 * @brief The renderer settings panel.
 * @ingroup group_editor
 */

#pragma once

namespace Uge
{

	/**
	 * @brief Edits Uge::PostProcess's exposure, tonemap and bloom settings. They aren't saved.
	 * @ingroup group_editor
	 */
	class RendererSettingsPanel
	{
	public:
		/** @brief Draws the panel and applies changes at once. */
		void OnImGuiRender();
	};

}
