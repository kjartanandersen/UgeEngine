/**
 * @file PostProcess.h
 * @brief The pass that turns the scene's HDR render target into a displayable image.
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
	 * @brief Curve used to map unbounded linear radiance into displayable range.
	 * @ingroup group_renderer
	 */
	enum class TonemapMode : int32_t
	{
		None = 0, ///< No curve; values above `1.0` are clipped by the output target.
		Reinhard = 1, ///< `c / (c + 1)`. Never clips, but desaturates and flattens contrast.
		ACES = 2 ///< Filmic curve; rolls highlights towards white rather than clipping them.
	};

	/**
	 * @brief Tunable parameters of the resolve pass.
	 * @ingroup group_renderer
	 *
	 * Uge::TonemapMode::None reproduces the pre-HDR pipeline exactly, which is what makes it
	 * useful for checking a change has not altered the image; it is not a sensible way to
	 * view an HDR scene.
	 */
	struct RenderSettings
	{
		float Exposure = 1.0f; ///< Linear multiplier applied before the curve.

		/**
		 * @brief Curve applied to the exposed colour.
		 *
		 * ACES by default. Without a curve every value from `1.0` upwards clips to the same
		 * pure white, so a specular highlight has no falloff between its core and its edge
		 * and reads as a flat disc rather than a highlight.
		 */
		TonemapMode Tonemap = TonemapMode::ACES;

		bool BloomEnabled = true; ///< Whether to run the bloom chain at all.

		/**
		 * @brief Brightness above which a pixel starts contributing to bloom.
		 *
		 * In linear units, so `1.0` means "brighter than white". Below that, ordinary lit
		 * surfaces start to glow and the image turns hazy.
		 */
		float BloomThreshold = 1.0f;

		/**
		 * @brief Width of the soft ramp either side of #BloomThreshold.
		 *
		 * A hard cutoff makes bloom snap on and off as a surface crosses the threshold —
		 * very visible on a moving specular highlight. This ramps it in instead.
		 */
		float BloomKnee = 0.5f;

		float BloomIntensity = 0.5f; ///< How much of the bloom result is added back.
	};

	/**
	 * @brief Resolves the HDR scene target into a displayable low-dynamic-range image.
	 * @ingroup group_renderer
	 *
	 * The scene is rendered into an `RGBA16F` target holding **linear** radiance, which is
	 * unbounded above — a bright emissive surface lands well past `1.0`. This pass applies
	 * exposure, a tonemap curve and the sRGB encode, writing into an `RGBA8` target the
	 * editor can display.
	 *
	 * @code
	 * sceneFramebuffer->Unbind();
	 * PostProcess::Resolve(sceneFramebuffer, displayFramebuffer);
	 * @endcode
	 *
	 * @note Resources are created on first use rather than at startup, because the graphics
	 * context does not exist when static initializers run. @see Uge::Model
	 *
	 * @warning Because the scene target is linear, everything drawn into it must be linear
	 * too. The 2D shaders decode their sRGB textures and vertex colours on the way in for
	 * exactly this reason; a pass that writes sRGB values straight into the target is
	 * encoded a second time here and comes out washed out.
	 */
	class PostProcess
	{
	public:
		/**
		 * @brief Resolves @p source into @p target.
		 * @param source Framebuffer whose colour attachment 0 holds linear HDR radiance.
		 * @param target Framebuffer to write the displayable image into; bound by this call.
		 *
		 * Draws a fullscreen triangle with depth testing off. Leaves @p target bound and
		 * depth testing restored.
		 *
		 * @warning @p source and @p target must be different framebuffers. Sampling a
		 * texture that is also the current render target is undefined.
		 */
		static void Resolve(const Ref<Framebuffer>& source, const Ref<Framebuffer>& target);

		/**
		 * @brief The current resolve parameters.
		 * @return Const reference to the settings.
		 */
		static const RenderSettings& GetSettings() { return s_settings; }
		/**
		 * @brief Replaces the resolve parameters.
		 * @param settings New exposure and tonemap curve; applied from the next Resolve().
		 */
		static void SetSettings(const RenderSettings& settings) { s_settings = settings; }

	private:
		static void EnsureResources();

	private:
		/** @brief Shader, fullscreen geometry and uniform buffer shared by every resolve. */
		struct ResolveData
		{
			Ref<Shader> TonemapShader; ///< Shader applying exposure, curve and sRGB encode.
			Ref<UniformBuffer> SettingsUniformBuffer; ///< Per-resolve exposure and curve.
			bool Initialized = false; ///< Whether the shared resources have been created.
		};

		static ResolveData s_resolveData;
		static RenderSettings s_settings;
	};

}
