#include "ugpch.h"
#include "OpenGLTexture.h"

#include <stb_image.h>



namespace Uge
{

	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height, const std::string& name)
	{
		m_width = width;
		m_height = height;
		m_name = name;

		UG_PROFILE_FUNCTION();

		m_internalFormat = GL_RGBA8;
		m_dataFormat = GL_RGBA;



		glCreateTextures(GL_TEXTURE_2D, 1, &m_rendererID);
		glTextureStorage2D(m_rendererID, 1, m_internalFormat, m_width, m_height);

		glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);



	}


	OpenGLTexture2D::OpenGLTexture2D(const std::string& path, const std::string& name)
	{
		m_path = path;
		m_name = name;
		UG_PROFILE_FUNCTION();

		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		stbi_uc* data = nullptr;

		{
			UG_PROFILE_SCOPE("stbi_load - OpenGLTexture2D::OpenGLTexture2D(const std::string& path) stbi_load");

			data = stbi_load(path.c_str(), &width, 
				&height, &channels, STBI_rgb_alpha);

		}


		UG_CORE_ASSERT(data, "Failed to load image!");
		(void)channels;

		m_width = width;
		m_height = height;

		m_internalFormat = GL_RGBA8;
		m_dataFormat = GL_RGBA;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_rendererID);
		glTextureStorage2D(m_rendererID, 1, m_internalFormat, m_width, m_height);

		glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTextureSubImage2D(m_rendererID, 0, 0, 0, m_width, m_height, m_dataFormat, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);

	}

	OpenGLTexture2D::OpenGLTexture2D(const unsigned char* encodedData, uint32_t size, const std::string& name)
	{
		m_name = name;
		UG_PROFILE_FUNCTION();

		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		stbi_uc* data = stbi_load_from_memory(encodedData, (int)size, &width, &height, &channels, STBI_rgb_alpha);

		UG_CORE_ASSERT(data, "Failed to load image from memory!");
		(void)channels;

		m_width = width;
		m_height = height;

		m_internalFormat = GL_RGBA8;
		m_dataFormat = GL_RGBA;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_rendererID);
		glTextureStorage2D(m_rendererID, 1, m_internalFormat, m_width, m_height);

		glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTextureSubImage2D(m_rendererID, 0, 0, 0, m_width, m_height, m_dataFormat, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);
	}

	

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		UG_PROFILE_FUNCTION();

		glDeleteTextures(1, &m_rendererID);


	}

	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
#ifdef UG_DEBUG
		uint32_t bpp = m_dataFormat == GL_RGBA ? 4 : 3;
		UG_CORE_ASSERT(size == m_width * m_height * bpp, "Data must be the entire texture!");

#endif // UG_DEBUG

		glTextureSubImage2D(m_rendererID, 0, 0, 0, m_width, m_height, m_dataFormat, GL_UNSIGNED_BYTE, data);


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
