/**
 * @file Texture.h
 * @brief Textures.
 * @ingroup group_renderer
 */

#pragma once

#include <string>

#include "Uge/Core/Core.h"
#include "Uge/Asset/Asset.h"
#include "Uge/Core/Buffer.h"


namespace Uge
{

	/**
	 * @brief Texture pixel formats.
	 * @ingroup group_renderer
	 */
	enum class ImageFormat
	{
		None = 0,
		R8,
		RGB8,
		RGBA8,
		RG16F,
		RGBA16F,
		RGBA32F
	};

	/**
	 * @brief The size of one pixel.
	 * @param format The format.
	 * @return Size in bytes; asserts and returns `0` for an unknown format.
	 * @ingroup group_renderer
	 */
	uint32_t ImageFormatBytesPerPixel(ImageFormat format);

	/**
	 * @brief The settings a texture is created from. The defaults are 1x1 RGBA8.
	 * @ingroup group_renderer
	 */
	struct TextureSpecification
	{

		uint32_t Width = 1; ///< Width in pixels.
		uint32_t Height = 1; ///< Height in pixels.
		ImageFormat Format = ImageFormat::RGBA8; ///< Pixel format.
		bool GenerateMips = true; ///< Whether to generate mipmaps.
	};


	/**
	 * @brief A GPU texture, and an asset.
	 * @ingroup group_renderer
	 */
	class Texture: public Asset
	{

	public:
		/** @brief Releases the texture. */
		virtual ~Texture() = default;

		/**
		 * @brief The specification.
		 * @return The specification.
		 */
		virtual const TextureSpecification& GetSpecification() const = 0;

		/** @brief The width. @return Width in pixels. */
		virtual uint32_t GetWidth() const = 0;
		/** @brief The height. @return Height in pixels. */
		virtual uint32_t GetHeight() const = 0;
		/**
		 * @brief The OpenGL texture name.
		 * @return The name.
		 */
		virtual uint32_t GetRendererID() const = 0;

		/**
		 * @brief Uploads pixels.
		 * @param data Width x height x bytes-per-pixel bytes.
		 */
		virtual void SetData(Buffer data) = 0;

		/**
		 * @brief Binds the texture to a slot.
		 * @param slot Texture unit.
		 */
		virtual void Bind(uint32_t slot = 0) const = 0;
		/**
		 * @brief Unbinds the texture from a slot.
		 * @param slot Texture unit.
		 */
		virtual void UnBind(uint32_t slot = 0) const = 0;

		/**
		 * @brief Whether the image loaded.
		 * @return `false` if it failed to load.
		 */
		virtual bool IsLoaded() const = 0;

		/**
		 * @brief Sets how many times the texture repeats.
		 * @param tilingFactor Repeat count.
		 */
		virtual void SetTilingFactor(float tilingFactor) = 0;

		/**
		 * @brief Whether two textures are the same GPU texture.
		 * @param other The other texture.
		 * @return `true` if they are.
		 */
		virtual bool operator==(const Texture& other) const = 0;

	};


	/**
	 * @brief A 2D texture.
	 * @ingroup group_renderer
	 */
	class Texture2D : public Texture
	{
		
	public:

		/**
		 * @brief Creates a texture.
		 * @param specification Size, format and mipmaps.
		 * @param data Pixels, or an empty buffer to leave the texture uninitialized.
		 * @return The texture.
		 */
		static Ref<Texture2D> Create(const TextureSpecification& specification, Buffer data = Buffer());

		/**
		 * @brief The asset type.
		 * @return Uge::AssetType::Texture2D.
		 */
		static AssetType GetStaticType() { return AssetType::Texture2D; }
		/**
		 * @brief The asset type.
		 * @return Uge::AssetType::Texture2D.
		 */
		virtual AssetType GetType() const override { return GetStaticType(); }


	public:
		std::string m_path; ///< The source file, if any.
		uint32_t m_width; ///< Width in pixels.
		uint32_t m_height; ///< Height in pixels.

		float m_tilingFactor = 1.0f; ///< Repeat count.



	protected:


	};

}


