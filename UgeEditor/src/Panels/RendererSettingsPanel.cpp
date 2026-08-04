#include <ugpch.h>
#include "RendererSettingsPanel.h"

#include "Uge/Renderer/PostProcess.h"

#include <imgui.h>

namespace Uge
{

	void RendererSettingsPanel::OnImGuiRender()
	{
		ImGui::Begin("Renderer Settings");

		RenderSettings settings = PostProcess::GetSettings();

		ImGui::SeparatorText("Tonemapping");

		ImGui::DragFloat("Exposure", &settings.Exposure, 0.01f, 0.0f, 20.0f);

		const char* tonemapNames[] = { "None", "Reinhard", "ACES" };
		int tonemapIndex = static_cast<int>(settings.Tonemap);
		if (ImGui::Combo("Curve", &tonemapIndex, tonemapNames, IM_ARRAYSIZE(tonemapNames)))
		{
			settings.Tonemap = static_cast<TonemapMode>(tonemapIndex);
		}

		if (settings.Tonemap == TonemapMode::None)
		{
			ImGui::TextWrapped(
				"None still applies the sRGB encode - it only skips the curve, so anything "
				"brighter than white is clipped rather than rolled off.");
		}

		ImGui::SeparatorText("Bloom");

		ImGui::Checkbox("Enabled", &settings.BloomEnabled);

		// Disabling the widgets rather than hiding them keeps the panel from jumping around
		// as bloom is toggled.
		ImGui::BeginDisabled(!settings.BloomEnabled);
		{
			ImGui::DragFloat("Intensity", &settings.BloomIntensity, 0.01f, 0.0f, 5.0f);

			// Above 1.0 by default: 1.0 is white, so a lower threshold makes ordinary lit
			// surfaces glow and the whole image turns hazy.
			ImGui::DragFloat("Threshold", &settings.BloomThreshold, 0.01f, 0.0f, 10.0f);
			ImGui::DragFloat("Knee", &settings.BloomKnee, 0.01f, 0.0f, 2.0f);

			ImGui::TextWrapped(
				"Threshold is in linear units, where 1.0 is white. Knee softens the cutoff so "
				"bloom fades in rather than snapping on.");
		}
		ImGui::EndDisabled();

		PostProcess::SetSettings(settings);

		ImGui::End();
	}

}
