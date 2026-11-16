/**
 * @file Bloom.h
 * @brief Bloom: light bleeding from bright pixels.
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
	 * @brief Spreads light from pixels brighter than white into their surroundings.
	 * @ingroup group_renderer
	 *
	 * The bright parts of the scene are halved repeatedly, then added back up level by
	 * level, each one wider and softer. The scene must be in a floating-point target;
	 * in `RGBA8`, nothing passes the threshold.
	 */
	class Bloom
	{
	public:
		/**
		 * @brief Resizes the levels for a scene size. Does nothing if the size is unchanged.
		 * @param width Scene width in pixels.
		 * @param height Scene height in pixels.
		 */
		static void Resize(uint32_t width, uint32_t height);

		/** @brief Releases the shaders, uniform buffer and framebuffers. */
		static void Shutdown();

		/**
		 * @brief Runs the bloom passes.
		 * @param scene Framebuffer whose colour attachment 0 holds the linear HDR scene.
		 * @param settings Whether bloom is on, its threshold and knee.
		 *
		 * Leaves no framebuffer bound, and turns alpha blending and depth testing back on.
		 */
		static void Render(const Ref<Framebuffer>& scene, const RenderSettings& settings);

		/**
		 * @brief Binds the bloom result to a texture slot.
		 * @param slot Texture unit.
		 * @pre HasResult() is true.
		 */
		static void BindResult(uint32_t slot);

		/**
		 * @brief Whether Render() produced a result this frame.
		 * @return `true` if it did.
		 */
		static bool HasResult();

	private:
		static void EnsureResources();

	private:
		/** @brief The shaders, uniform buffer and levels. */
		struct BloomData
		{
			Ref<Shader> DownsampleShader; ///< Threshold and halve.
			Ref<Shader> UpsampleShader; ///< Tent filter, added up.
			Ref<UniformBuffer> SettingsUniformBuffer; ///< Texel size and threshold, per pass.

			std::vector<Ref<Framebuffer>> Levels; ///< Each half the size of the last.
			uint32_t Width = 0; ///< Scene width the levels were built for.
			uint32_t Height = 0; ///< Scene height the levels were built for.

			bool Initialized = false; ///< Whether the shaders and buffer exist.
			bool HasResult = false; ///< Whether the last Render() produced a result.
		};

		static BloomData s_bloomData;
	};

}
