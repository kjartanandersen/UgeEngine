/**
 * @file PostProcess.h
 * @brief Tonemapping of the HDR scene into a displayable image.
 * @ingroup group_renderer
 */

#pragma once

#include "Uge/Core/Core.h"

#include "Uge/Renderer/Framebuffer.h"
#include "Uge/Renderer/Shader.h"
#include "Uge/Renderer/UniformBuffer.h"
#include "Uge/Renderer/VertexArray.h"

namespace Uge
{

	/**
	 * @brief Tonemap curves.
	 * @ingroup group_renderer
	 */
	enum class TonemapMode : int32_t
	{
		None = 0, ///< No curve; values above `1.0` clip.
		Reinhard = 1, ///< `c / (c + 1)`.
		ACES = 2 ///< Filmic curve.
	};

	/**
	 * @brief Exposure, tonemap and bloom settings.
	 * @ingroup group_renderer
	 */
	struct RenderSettings
	{
		float Exposure = 1.0f; ///< Multiplier applied before the curve.

		/** @brief The tonemap curve. */
		TonemapMode Tonemap = TonemapMode::ACES;

		bool BloomEnabled = true; ///< Whether bloom runs.

		/** @brief Linear brightness above which a pixel blooms; `1.0` is white. */
		float BloomThreshold = 1.0f;

		/** @brief Width of the soft ramp around #BloomThreshold. */
		float BloomKnee = 0.5f;

		float BloomIntensity = 0.5f; ///< How much bloom is added to the scene.
	};

	/**
	 * @brief Applies exposure, bloom, the tonemap curve and sRGB encoding to the linear
	 * HDR scene.
	 * @ingroup group_renderer
	 *
	 * Everything drawn into the scene must be linear, or it is encoded twice.
	 */
	class PostProcess
	{
	public:
		/**
		 * @brief Draws @p source into @p target. Leaves @p target bound.
		 * @param source Framebuffer whose colour attachment 0 holds the linear HDR scene.
		 * @param target Framebuffer to draw into; must not be @p source.
		 */
		static void Resolve(const Ref<Framebuffer>& source, const Ref<Framebuffer>& target);

		/**
		 * @brief Releases the shader and uniform buffer. The next Resolve() recreates them.
		 */
		static void Shutdown();

		/**
		 * @brief The settings.
		 * @return The settings.
		 */
		static const RenderSettings& GetSettings() { return s_settings; }
		/**
		 * @brief Replaces the settings.
		 * @param settings The settings.
		 */
		static void SetSettings(const RenderSettings& settings) { s_settings = settings; }

	private:
		static void EnsureResources();

	private:
		/** @brief The shader and uniform buffer. */
		struct ResolveData
		{
			Ref<Shader> TonemapShader; ///< The tonemap shader.
			Ref<UniformBuffer> SettingsUniformBuffer; ///< Exposure, curve and bloom intensity.
			bool Initialized = false; ///< Whether the resources exist.
		};

		static ResolveData s_resolveData;
		static RenderSettings s_settings;
	};

}
