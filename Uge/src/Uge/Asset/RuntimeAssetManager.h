/**
 * @file RuntimeAssetManager.h
 * @brief Asset manager for shipped builds, serving pre-packed assets.
 * @ingroup group_asset
 */

#pragma once

#include "AssetManagerBase.h"

namespace Uge
{

	/**
	 * @brief Asset manager for a packaged game: no importing, no registry file.
	 * @ingroup group_asset
	 *
	 * The counterpart to Uge::EditorAssetManager. Where the editor tracks loose files and
	 * imports them on demand, this one is intended to read assets already converted into
	 * an engine-ready pack, so shipped builds do not carry the importers.
	 *
	 * @note Currently a placeholder — the interface is declared but not yet implemented.
	 * The editor manager is what the engine uses today.
	 */
	class RuntimeAssetManager : public AssetManagerBase
	{

	public:


	};

}