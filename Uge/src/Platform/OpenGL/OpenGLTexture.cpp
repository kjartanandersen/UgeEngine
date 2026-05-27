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
			case ImageFormat::RGB8:  return GL_RGB;
			case ImageFormat::RGBA8: return GL_RGBA;
			}

			UG_CORE_ASSERT(false);
			return 0;
		}

		static GLenum UgeImageFormatToGLInternalFormat(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::RGB8:  return GL_RGB8;
			case ImageFormat::RGBA8: return GL_RGBA8;
			}

			UG_CORE_ASSERT(false);
			return 0;
		}
	}

	OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification& specification, Buffer data)
		: m_specification(specification), m_width(m_specification.Width), m_height(m_specification.Height)
	{

		m_internalFormat = Utils::UgeImageFormatToGLInternalFormat(m_specification.Format);
		m_dataFormat = Utils::UgeImageFormatToGLDataFormat(m_specification.Format);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_rendererID);
		glTextureStorage2D(m_rendererID, 1, m_internalFormat, m_width, m_height);

		glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		if (data)
		{
			// glTextureSubImage2D(m_rendererID, 0, 0, 0, m_width, m_height, m_dataFormat, GL_UNSIGNED_BYTE, data.Data);

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
		uint32_t bpp = m_dataFormat == GL_RGBA ? 4 : 3;
		UG_CORE_ASSERT(data.Size == m_width * m_height * bpp, "Data must be the entire texture!");

#endif // UG_DEBUG

		glTextureSubImage2D(m_rendererID, 0, 0, 0, m_width, m_height, m_dataFormat, GL_UNSIGNED_BYTE, data.Data);


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
