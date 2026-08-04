/**
 * @file Environment.h
 * @brief The precomputed maps that make image-based lighting a few texture fetches.
 * @ingroup group_renderer
 */

#pragma once

#include <filesystem>

#include "Uge/Core/Core.h"
#include "Uge/Asset/Asset.h"

#include "Uge/Renderer/Texture.h"
#include "Uge/Renderer/TextureCube.h"

namespace Uge
{

	/**
	 * @brief An environment map and the three products derived from it.
	 * @ingroup group_renderer
	 *
	 * Lighting a surface from its whole surroundings means integrating incoming radiance over
	 * a hemisphere, which is far too expensive per fragment. Splitting that integral into
	 * parts that depend only on the environment lets all of them be computed once, at import,
	 * and reduces shading to three lookups:
	 *
	 * - #Skybox — the environment itself, drawn behind the scene so reflections have a
	 *   visible source.
	 * - #Irradiance — cosine-convolved; the diffuse half, one fetch per fragment.
	 * - #Prefiltered — convolved against the GGX lobe, one roughness per mip level.
	 * - #BrdfLut — the view-angle and roughness dependent scale and bias applied to F0.
	 *
	 * The last two are Karis' split-sum approximation, which is what makes the specular half
	 * tractable.
	 *
	 * @note Building these needs a live graphics context, so an Uge::Environment cannot be
	 * created before the renderer is up.
	 *
	 * @see Uge::EnvironmentImporter, `assets/shaders/PrefilterEnvironment.glsl`
	 */
	class Environment : public Asset
	{
	public:
		Ref<TextureCube> Skybox; ///< The environment cubemap, as projected from the source image.
		Ref<TextureCube> Irradiance; ///< Diffuse irradiance; low resolution, no mips needed.
		Ref<TextureCube> Prefiltered; ///< Specular radiance, roughness across the mip chain.
		Ref<Texture2D> BrdfLut; ///< Split-sum scale and bias, indexed by view angle and roughness.

		/**
		 * @brief Whether every map was built successfully.
		 * @return `true` if the environment is usable for rendering.
		 */
		bool IsValid() const
		{
			return Skybox && Irradiance && Prefiltered && BrdfLut;
		}

		/**
		 * @brief Mip levels in #Prefiltered, which the roughness lookup is scaled against.
		 * @return Level count, or `1` if the map is missing.
		 */
		uint32_t GetPrefilteredMipCount() const
		{
			return Prefiltered ? Prefiltered->GetMipLevelCount() : 1;
		}

		/**
		 * @brief Builds an environment from an equirectangular HDR image.
		 * @param path Filesystem path to a `.hdr` file.
		 * @return The built environment, or null if the image could not be loaded.
		 *
		 * @note Blocking, and the slowest import in the engine: the prefilter pass takes 1024
		 * importance samples per texel across every face and mip.
		 */
		static Ref<Environment> Create(const std::filesystem::path& path);

		/**
		 * @brief The asset type this class represents.
		 * @return Uge::AssetType::Environment.
		 */
		static AssetType GetStaticType() { return AssetType::Environment; }
		/**
		 * @brief This instance's asset type.
		 * @return Uge::AssetType::Environment.
		 */
		virtual AssetType GetType() const override { return GetStaticType(); }
	};

}
