/**
 * @file Asset.h
 * @brief Asset base class, handles and types.
 * @ingroup group_asset
 */

#pragma once

#include <Uge/Core/UUID.h>

#include <string_view>


namespace Uge
{
	/**
	 * @brief Identifies an asset. `0` means none.
	 * @ingroup group_asset
	 */
	using AssetHandle = UUID;

	/**
	 * @brief Kinds of asset.
	 * @ingroup group_asset
	 */
	enum class AssetType : uint16_t
	{
		None = 0, ///< No type.
		Scene, ///< A Uge::Scene in a `.uge` file.
		Texture2D, ///< A Uge::Texture2D loaded from an image.
		Mesh, ///< A Uge::Model imported through assimp.
		Material, ///< A Uge::Material, usually created in memory by mesh import.
		Environment ///< A Uge::Environment built from an equirectangular `.hdr` image.
	};

	/**
	 * @brief The serialized name of an asset type.
	 * @param type Type to convert.
	 * @return A name such as `"AssetType::Texture2D"`.
	 * @ingroup group_asset
	 */
	std::string_view AssetTypeToString(AssetType type);
	/**
	 * @brief Parses a name written by Uge::AssetTypeToString.
	 * @param type Name to parse.
	 * @return The type, or Uge::AssetType::None for an unknown name.
	 * @ingroup group_asset
	 */
	AssetType AssetTypeFromString(std::string_view type);

	/**
	 * @brief Base class of every asset.
	 * @ingroup group_asset
	 *
	 * A subclass must also define a static `GetStaticType()` that returns the same value
	 * as GetType().
	 */
	class Asset
	{


	public:


		AssetHandle m_handle; ///< Set when the asset is registered.

	public:
		/**
		 * @brief The asset's type.
		 * @return The type.
		 */
		virtual AssetType GetType() const = 0;

		/**
		 * @brief The display name.
		 * @return The name, prefixed with the asset type.
		 */
		std::string GetName() const { return m_name; }
		/**
		 * @brief Sets the display name.
		 * @param name Name, stored as `"<type> - <name>"`.
		 */
		void SetName(const std::string& name) 
		{ 
			std::string_view sv = AssetTypeToString(GetType());
			std::string at{sv};
			m_name = at + " - " + name;
		}

	private:

		std::string m_name;



	};

}