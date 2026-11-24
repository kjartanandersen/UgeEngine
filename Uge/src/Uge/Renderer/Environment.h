/**
 * @file Environment.h
 * @brief The maps used for image-based lighting.
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
	 * @brief An environment cubemap and the lighting maps built from it.
	 * @ingroup group_renderer
	 *
	 * #Prefiltered and #BrdfLut are the two halves of the split-sum approximation for
	 * specular light. Building them needs a GL context.
	 */
	class Environment : public Asset
	{
	public:
		Ref<TextureCube> Skybox; ///< The environment.
		Ref<TextureCube> Irradiance; ///< Diffuse light.
		Ref<TextureCube> Prefiltered; ///< Specular light, one roughness per mip level.
		Ref<Texture2D> BrdfLut; ///< Scale and bias for F0, by view angle and roughness.

		/**
		 * @brief Whether every map exists.
		 * @return `true` if they all do.
		 */
		bool IsValid() const
		{
			return Skybox && Irradiance && Prefiltered && BrdfLut;
		}

		/**
		 * @brief The number of mip levels in #Prefiltered.
		 * @return The count, or `1` if there is no map.
		 */
		uint32_t GetPrefilteredMipCount() const
		{
			return Prefiltered ? Prefiltered->GetMipLevelCount() : 1;
		}

		/**
		 * @brief Builds an environment from an equirectangular HDR image. Slow.
		 * @param path Path to a `.hdr` file.
		 * @return The environment, or null if the image doesn't load.
		 */
		static Ref<Environment> Create(const std::filesystem::path& path);

		/**
		 * @brief The asset type.
		 * @return Uge::AssetType::Environment.
		 */
		static AssetType GetStaticType() { return AssetType::Environment; }
		/**
		 * @brief The asset type.
		 * @return Uge::AssetType::Environment.
		 */
		virtual AssetType GetType() const override { return GetStaticType(); }
	};

}
