/**
 * @file Bloom.h
 * @brief The light-bleed pass that makes bright surfaces read as emitting light.
 * @ingroup group_renderer
 */

#pragma once

#include <vector>

#include "Uge/Core/Core.h"
#include "Uge/Renderer/Framebuffer.h"
#include "Uge/Renderer/Shader.h"
#include "Uge/Renderer/UniformBuffer.h"

namespace Uge
{

	struct RenderSettings;

	/**
	 * @brief Spreads light from bright pixels into their surroundings.
	 * @ingroup group_renderer
	 *
	 * A surface brighter than white cannot be shown as brighter than white — the display has
	 * no headroom left. Bloom conveys that extra intensity by bleeding it outwards instead,
	 * which is why an emissive headlight only reads as *emitting* once this pass runs rather
	 * than merely as a white shape.
	 *
	 * The chain thresholds the scene, halves it repeatedly, then accumulates back up, each
	 * level adding a wider and softer contribution:
	 *
	 * @code
	 * scene --threshold+downsample--> [0] --> [1] --> ... --> [n-1]
	 *                                  ^-------^-------^--------'   (additive upsample)
	 * @endcode
	 *
	 * @note Each level is its own Uge::Framebuffer rather than a mip of one texture. Mip
	 * levels would be marginally cheaper, but rendering into individual levels sits outside
	 * what Uge::Framebuffer expresses, and allocating a different number of levels than are
	 * written is a mistake that produces plausible-looking output rather than an error.
	 *
	 * @warning Requires the scene to have been rendered into a floating-point target. Against
	 * an `RGBA8` target nothing exceeds `1.0`, so no pixel ever passes the threshold.
	 */
	class Bloom
	{
	public:
		/**
		 * @brief Resizes the chain to match a new scene resolution.
		 * @param width Scene width in pixels.
		 * @param height Scene height in pixels.
		 *
		 * Rebuilds the levels only when the size actually changes, so it is safe to call
		 * every frame.
		 */
		static void Resize(uint32_t width, uint32_t height);

		/**
		 * @brief Runs the threshold, downsample and upsample chain.
		 * @param scene Framebuffer whose colour attachment 0 holds the linear HDR scene.
		 * @param settings Threshold, knee and whether bloom is enabled at all.
		 *
		 * Does nothing when bloom is disabled or the chain has no levels. Leaves no
		 * framebuffer bound and restores alpha blending and depth testing.
		 */
		static void Render(const Ref<Framebuffer>& scene, const RenderSettings& settings);

		/**
		 * @brief Binds the accumulated bloom to a sampler slot.
		 * @param slot Texture unit to bind to.
		 * @pre HasResult() must be true.
		 */
		static void BindResult(uint32_t slot);

		/**
		 * @brief Whether a bloom result is available to sample.
		 * @return `true` if Render() has produced a usable texture this frame.
		 */
		static bool HasResult();

	private:
		static void EnsureResources();

	private:
		/** @brief Shaders, geometry and the level chain shared by every bloom pass. */
		struct BloomData
		{
			Ref<Shader> DownsampleShader; ///< Threshold and halve.
			Ref<Shader> UpsampleShader; ///< Tent filter, accumulated additively.
			Ref<UniformBuffer> SettingsUniformBuffer; ///< Per-pass texel size and threshold.

			std::vector<Ref<Framebuffer>> Levels; ///< Progressively halved targets.
			uint32_t Width = 0; ///< Scene width the chain was built for.
			uint32_t Height = 0; ///< Scene height the chain was built for.

			bool Initialized = false; ///< Whether the shared resources have been created.
			bool HasResult = false; ///< Whether the last Render() produced output.
		};

		static BloomData s_bloomData;
	};

}
