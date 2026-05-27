#pragma once

#include <Uge/Core/UUID.h>


namespace Uge
{
	using AssetHandle = UUID;

	enum class AssetType
	{
		None = 0,
		Scene,
		Texture2D
	};

	class Asset
	{


	public:

		AssetHandle m_handle;

	public:
		virtual AssetType GetType() const = 0;

	protected:



	};

}