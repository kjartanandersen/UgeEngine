/**
 * @file EnvironmentImporter.h
 * @brief Imports `.hdr` images as environments.
 * @ingroup group_asset
 */

#pragma once

#include "Asset.h"
#include "AssetMetadata.h"

#include <Uge/Renderer/Environment.h>

namespace Uge
{

	/**
	 * @brief Imports equirectangular `.hdr` images as environments.
	 * @ingroup group_asset
	 *
	 * The import renders the lighting maps on the GPU, so it needs a GL context.
	 */
	class EnvironmentImporter
	{
	public:
		/**
		 * @brief Imports an environment.
		 * @param handle Handle to give the environment.
		 * @param metadata Its registry entry.
		 * @return The environment, or null on failure.
		 */
		static Ref<Environment> ImportEnvironment(AssetHandle handle, const AssetMetadata& metadata);
	};

}
