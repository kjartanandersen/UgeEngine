/**
 * @file OpenGLTexture.h
 * @brief OpenGL implementation of Uge::Texture2D.
 * @ingroup group_platform
 */

#pragma once

#include "Uge/Renderer/Texture.h"

#include <glad/glad.h>

namespace Uge
{


	/**
	 * @brief An immutable-storage OpenGL 2D texture.
	 * @ingroup group_platform
	 *
	 * Uses direct state access, so no binding is needed to upload or configure the texture.
	 */
	class OpenGLTexture2D : public Texture2D
	{

	public:
		/**
		 * @brief Allocates the texture and optionally uploads its pixels.
		 * @param specification Size, format and mip settings.
		 * @param data Initial pixel data; an empty buffer leaves the texture uninitialized.
		 */
		OpenGLTexture2D(const TextureSpecification& specification, Buffer data = Buffer());

		/** @brief Deletes the GL texture. */
		virtual ~OpenGLTexture2D();

		virtual const TextureSpecification& GetSpecification() const override { return m_specification; }

		virtual uint32_t GetWidth() const override { return m_width; };
		virtual uint32_t GetHeight() const override { return m_height; };
		virtual uint32_t GetRendererID() const override { return m_rendererID; }

		virtual void SetData(Buffer data) override;

		virtual void Bind(uint32_t slot = 0) const override;
		virtual void UnBind(uint32_t slot = 0) const override;

		virtual bool IsLoaded() const override { return m_isLoaded; }

		virtual void SetTilingFactor(float tilingFactor) override { m_tilingFactor = tilingFactor; }


		virtual bool operator==(const Texture& other) const override 
		{ 
			return m_rendererID == ((OpenGLTexture2D&)other).m_rendererID;
		}

	private:

		TextureSpecification m_specification;

		bool m_isLoaded = false;
		uint32_t m_width, m_height;

		uint32_t m_rendererID;
		GLenum m_internalFormat, m_dataFormat;

		/** @brief Component type of the pixel data SetData() uploads. */
		GLenum m_pixelType;
		/** @brief Levels allocated by `glTextureStorage2D`; `1` when mips are disabled. */
		uint32_t m_mipLevels = 1;



	};

}




