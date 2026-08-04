#include "ugpch.h"
#include "OpenGLTexture.h"


namespace Uge
{

	namespace Utils
	{
		static GLenum UgeImageFormatToGLDataFormat(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::R8:      return GL_RED;
			case ImageFormat::RGB8:    return GL_RGB;
			case ImageFormat::RGBA8:   return GL_RGBA;
			case ImageFormat::RG16F:   return GL_RG;
			case ImageFormat::RGBA16F: return GL_RGBA;
			case ImageFormat::RGBA32F: return GL_RGBA;
			}

			UG_CORE_ASSERT(false);
			return 0;
		}

		static GLenum UgeImageFormatToGLInternalFormat(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::R8:      return GL_R8;
			case ImageFormat::RGB8:    return GL_RGB8;
			case ImageFormat::RGBA8:   return GL_RGBA8;
			case ImageFormat::RG16F:   return GL_RG16F;
			case ImageFormat::RGBA16F: return GL_RGBA16F;
			case ImageFormat::RGBA32F: return GL_RGBA32F;
			}

			UG_CORE_ASSERT(false);
			return 0;
		}

		// The type of one component in the pixel data handed to SetData, which is not implied
		// by the internal format: a GL_RGBA16F texture is uploaded from GL_FLOAT data, since
		// stb_image's HDR loader produces 32-bit floats and the driver converts on upload.
		static GLenum UgeImageFormatToGLPixelType(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::R8:
			case ImageFormat::RGB8:
			case ImageFormat::RGBA8:   return GL_UNSIGNED_BYTE;
			case ImageFormat::RG16F:
			case ImageFormat::RGBA16F:
			case ImageFormat::RGBA32F: return GL_FLOAT;
			}

			UG_CORE_ASSERT(false);
			return 0;
		}

		// A full chain down to 1x1. Prefiltered environment maps address roughness through the
		// mip level, so the chain has to exist before anything can be rendered into it.
		static uint32_t CalculateMipLevels(uint32_t width, uint32_t height)
		{
			uint32_t levels = 1;
			uint32_t size = std::max(width, height);
			while (size > 1)
			{
				size >>= 1;
				++levels;
			}
			return levels;
		}
	}

	OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification& specification, Buffer data)
		: m_specification(specification), m_width(m_specification.Width), m_height(m_specification.Height)
	{

		m_internalFormat = Utils::UgeImageFormatToGLInternalFormat(m_specification.Format);
		m_dataFormat = Utils::UgeImageFormatToGLDataFormat(m_specification.Format);
		m_pixelType = Utils::UgeImageFormatToGLPixelType(m_specification.Format);

		m_mipLevels = m_specification.GenerateMips
			? Utils::CalculateMipLevels(m_width, m_height)
			: 1;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_rendererID);
		glTextureStorage2D(m_rendererID, m_mipLevels, m_internalFormat, m_width, m_height);

		// Sampling a mipped texture with GL_LINEAR would read level 0 forever and the chain
		// below would be dead weight.
		glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER,
			m_mipLevels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		if (data)
		{
			SetData(data);

		}

	}

	

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		UG_PROFILE_FUNCTION();

		glDeleteTextures(1, &m_rendererID);


	}

	void OpenGLTexture2D::SetData(Buffer data)
	{
#ifdef UG_DEBUG
		// Derived from the format rather than guessed from m_dataFormat, which cannot tell an
		// 8-bit RGBA from a 16-bit float one - both report GL_RGBA.
		const uint64_t bpp = ImageFormatBytesPerPixel(m_specification.Format);
		UG_CORE_ASSERT(data.Size == (uint64_t)m_width * m_height * bpp, "Data must be the entire texture!");

#endif // UG_DEBUG

		glTextureSubImage2D(m_rendererID, 0, 0, 0, m_width, m_height, m_dataFormat, m_pixelType, data.Data);

		// Only level 0 was uploaded; without this the rest of the chain stays undefined and a
		// minified sample returns whatever the driver left in the allocation.
		if (m_mipLevels > 1)
		{
			glGenerateTextureMipmap(m_rendererID);
		}

		m_isLoaded = true;

	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		UG_PROFILE_FUNCTION();

		glBindTextureUnit(slot, m_rendererID);

	}

	void OpenGLTexture2D::UnBind(uint32_t slot) const
	{
		UG_PROFILE_FUNCTION();

		glBindTextureUnit(slot, 0);
	}

}
