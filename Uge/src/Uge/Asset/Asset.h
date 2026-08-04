/**
 * @file Asset.h
 * @brief The asset base class, asset handles and the asset type enumeration.
 * @ingroup group_asset
 */

#pragma once

#include <Uge/Core/UUID.h>

#include <string_view>


namespace Uge
{
	/**
	 * @brief Identifies an asset. An alias for Uge::UUID.
	 * @ingroup group_asset
	 *
	 * Components store handles rather than pointers, so an asset can be reloaded, moved on
	 * disk or not yet loaded without invalidating anything referring to it. A handle of
	 * `0` conventionally means "none".
	 *
	 * Resolve one at the point of use:
	 * @code
	 * if (auto texture = AssetManager::GetAsset<Texture2D>(component.Texture))
	 *     texture->Bind();
	 * @endcode
	 */
	using AssetHandle = UUID;

	/**
	 * @brief The kinds of asset the engine can load.
	 * @ingroup group_asset
	 *
	 * Uge::AssetImporter::ImportAsset dispatches on this to pick an importer, so a new
	 * asset type needs an entry here and a corresponding importer.
	 */
	enum class AssetType : uint16_t
	{
		None = 0, ///< Unset / invalid.
		Scene, ///< A Uge::Scene, stored as a `.uge` YAML file.
		Texture2D, ///< A Uge::Texture2D loaded from an image file.
		Mesh, ///< A Uge::Model imported through assimp.
		Material, ///< A Uge::Material; usually memory-only, created during mesh import.
		Environment ///< A Uge::Environment built from an equirectangular `.hdr` image.
	};

	/**
	 * @brief Converts an asset type to its serialized name.
	 * @param type Type to convert.
	 * @return A static string such as `"Texture2D"`.
	 * @ingroup group_asset
	 */
	std::string_view AssetTypeToString(AssetType type);
	/**
	 * @brief Parses an asset type from its serialized name.
	 * @param type String produced by Uge::AssetTypeToString.
	 * @return The matching type, or Uge::AssetType::None if unrecognized.
	 * @ingroup group_asset
	 */
	AssetType AssetTypeFromString(std::string_view type);

	/**
	 * @brief Polymorphic base for everything the asset system manages.
	 * @ingroup group_asset
	 *
	 * Subclassed by Uge::Scene, Uge::Texture2D, Uge::Model and Uge::Material. Each
	 * subclass provides a static `GetStaticType()` alongside the virtual GetType(), which
	 * is what lets Uge::AssetManager::GetAsset check the type at compile time.
	 */
	class Asset
	{


	public:


		AssetHandle m_handle; ///< This asset's identifier, assigned when it is registered.

	public:
		/**
		 * @brief This instance's runtime asset type.
		 * @return The asset type; matches the subclass's `GetStaticType()`.
		 */
		virtual AssetType GetType() const = 0;

		/**
		 * @brief The asset's display name.
		 * @return Name as shown in the editor, prefixed with the asset type.
		 */
		std::string GetName() const { return m_name; }
		/**
		 * @brief Sets the display name.
		 * @param name Base name; stored prefixed with the asset type, as `"Texture2D - foo"`.
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