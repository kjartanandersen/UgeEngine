#pragma once

#include <Uge/Core/UUID.h>

#include <string_view>


namespace Uge
{
	using AssetHandle = UUID;

	enum class AssetType : uint16_t
	{
		None = 0,
		Scene,
		Texture2D,
		Mesh,
		Material
	};

	std::string_view AssetTypeToString(AssetType type);
	AssetType AssetTypeFromString(std::string_view type);

	class Asset
	{


	public:

		AssetHandle m_handle;

	public:
		virtual AssetType GetType() const = 0;

	protected:



	};

}