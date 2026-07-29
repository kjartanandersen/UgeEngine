/**
 * @file Texture.h
 * @brief GPU textures, which are also assets.
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
	 * @brief Pixel formats a texture can be created with.
	 * @ingroup group_renderer
	 */
	enum class ImageFormat
	{
		None = 0, ///< Unset / invalid.
		R8, ///< Single 8-bit channel; used for font atlases and masks.
		RGB8, ///< Three 8-bit channels, no alpha.
		RGBA8, ///< Four 8-bit channels; the usual colour format.
		RGBA32F ///< Four 32-bit float channels, for HDR data.
	};

	/**
	 * @brief Describes the texture to create.
	 * @ingroup group_renderer
	 *
	 * The defaults describe a 1x1 RGBA8 texture, which is what the renderer uses for its
	 * white fallback.
	 */
	struct TextureSpecification
	{

		uint32_t Width = 1; ///< Width in pixels.
		uint32_t Height = 1; ///< Height in pixels.
		ImageFormat Format = ImageFormat::RGBA8; ///< Pixel format.
		bool GenerateMips = true; ///< Whether to generate a mip chain.
	};


	/**
	 * @brief Abstract GPU texture; an Uge::Asset, so it is referenced by handle.
	 * @ingroup group_renderer
	 *
	 * Components store an Uge::AssetHandle rather than a pointer, and resolve it through
	 * Uge::AssetManager::GetAsset at draw time. @see group_asset
	 */
	class Texture: public Asset
	{

	public:
		/** @brief Releases the GPU texture. */
		virtual ~Texture() = default;

		/**
		 * @brief The specification this texture was created from.
		 * @return Const reference to the specification.
		 */
		virtual const TextureSpecification& GetSpecification() const = 0;

		/** @brief Texture width. @return Width in pixels. */
		virtual uint32_t GetWidth() const = 0;
		/** @brief Texture height. @return Height in pixels. */
		virtual uint32_t GetHeight() const = 0;
		/**
		 * @brief The backend's native handle.
		 * @return The OpenGL texture name; used to display the texture in ImGui.
		 */
		virtual uint32_t GetRendererID() const = 0;

		/**
		 * @brief Uploads pixel data.
		 * @param data Source pixels; size must match width x height x bytes-per-pixel.
		 */
		virtual void SetData(Buffer data) = 0;

		/**
		 * @brief Binds the texture to a sampler slot.
		 * @param slot Texture unit to bind to.
		 */
		virtual void Bind(uint32_t slot = 0) const = 0;
		/**
		 * @brief Unbinds the texture from a sampler slot.
		 * @param slot Texture unit to clear.
		 */
		virtual void UnBind(uint32_t slot = 0) const = 0;

		/**
		 * @brief Whether the texture holds valid image data.
		 * @return `false` if the source image failed to load.
		 */
		virtual bool IsLoaded() const = 0;

		/**
		 * @brief Sets how many times the texture repeats across the surface.
		 * @param tilingFactor Repeat count; `1.0f` maps the image exactly once.
		 */
		virtual void SetTilingFactor(float tilingFactor) = 0;

		/**
		 * @brief Compares two textures by their native handle.
		 * @param other Texture to compare against.
		 * @return `true` if both refer to the same GPU texture.
		 *
		 * The 2D batcher uses this to reuse an already-bound texture slot.
		 */
		virtual bool operator==(const Texture& other) const = 0;

	};


	/**
	 * @brief A two-dimensional texture: the concrete texture type used throughout.
	 * @ingroup group_renderer
	 *
	 * Created either from a Uge::TextureSpecification directly, or by
	 * Uge::TextureImporter when the asset system loads an image from disk.
	 */
	class Texture2D : public Texture
	{
		
	public:

		/**
		 * @brief Creates a texture for the active graphics API.
		 * @param specification Size, format and mip settings.
		 * @param data Optional initial pixels; an empty buffer leaves the texture uninitialized.
		 * @return The backend's texture implementation.
		 */
		static Ref<Texture2D> Create(const TextureSpecification& specification, Buffer data = Buffer());

		/**
		 * @brief The asset type this class represents.
		 * @return Uge::AssetType::Texture2D.
		 */
		static AssetType GetStaticType() { return AssetType::Texture2D; }
		/**
		 * @brief This instance's asset type.
		 * @return Uge::AssetType::Texture2D.
		 */
		virtual AssetType GetType() const override { return GetStaticType(); }


	public:
		std::string m_path; ///< Source file this texture was loaded from, if any.
		uint32_t m_width; ///< Width in pixels.
		uint32_t m_height; ///< Height in pixels.

		float m_tilingFactor = 1.0f; ///< Texture repeat count; `1.0f` maps the image once.



	protected:


	};

}


