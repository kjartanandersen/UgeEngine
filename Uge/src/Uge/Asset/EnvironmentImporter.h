/**
 * @file EnvironmentImporter.h
 * @brief Loads HDR images into Uge::Environment assets.
 * @ingroup group_asset
 */

#pragma once

#include "Asset.h"
#include "AssetMetadata.h"

#include <Uge/Renderer/Environment.h>

namespace Uge
{

	/**
	 * @brief Imports equirectangular `.hdr` images as environment maps.
	 * @ingroup group_asset
	 *
	 * The import is not just a file read: it runs four GPU passes to derive the irradiance,
	 * prefiltered and BRDF maps image-based lighting needs. @see Uge::Environment
	 */
	class EnvironmentImporter
	{
	public:
		/**
		 * @brief Imports an environment described by registry metadata.
		 * @param handle Handle to assign to the imported environment.
		 * @param metadata Metadata whose file path is resolved against the asset directory.
		 * @return The built environment, or null on failure.
		 */
		static Ref<Environment> ImportEnvironment(AssetHandle handle, const AssetMetadata& metadata);
	};

}
