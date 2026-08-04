/**
 * @file RendererSettingsPanel.h
 * @brief Editor panel exposing the resolve pass's exposure, tonemap and bloom controls.
 * @ingroup group_editor
 */

#pragma once

namespace Uge
{

	/**
	 * @brief Panel for tuning how the HDR scene is turned into a displayable image.
	 * @ingroup group_editor
	 *
	 * These are look controls, not scene data: they belong to the renderer rather than to any
	 * entity, so they are deliberately not serialized with the scene. @see Uge::RenderSettings
	 */
	class RendererSettingsPanel
	{
	public:
		/** @brief Draws the panel and applies any change immediately. */
		void OnImGuiRender();
	};

}
