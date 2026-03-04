#pragma once

#include "Uge/Renderer/Texture.h"

#include <glad/glad.h>

namespace Uge
{


	class OpenGLTexture2D : public Texture2D
	{

	public:
		OpenGLTexture2D(uint32_t width, uint32_t height, const std::string& name);
		OpenGLTexture2D(const std::string& path, const std::string& name = "");
		OpenGLTexture2D(const unsigned char* encodedData, uint32_t size, const std::string& name = "");

		virtual ~OpenGLTexture2D();

		virtual uint32_t GetWidth() const override { return m_width; };
		virtual uint32_t GetHeight() const override { return m_height; };
		virtual uint32_t GetRendererID() const override { return m_rendererID; }

		virtual void SetData(void* data, uint32_t size) override;

		virtual void Bind(uint32_t slot = 0) const override;
		virtual void UnBind(uint32_t slot = 0) const override;

		virtual bool operator==(const Texture& other) const override 
		{ 
			return m_rendererID == ((OpenGLTexture2D&)other).m_rendererID;
		}

	private:
		uint32_t m_rendererID;
		GLenum m_internalFormat, m_dataFormat;



	};

}




