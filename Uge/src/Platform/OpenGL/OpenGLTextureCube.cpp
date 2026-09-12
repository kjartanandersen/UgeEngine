#include <ugpch.h>
#include "OpenGLTextureCube.h"

namespace Uge
{

	namespace Utils
	{
		static GLenum UgeImageFormatToGLInternalFormatCube(ImageFormat format)
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

		static uint32_t CalculateCubeMipLevels(uint32_t size)
		{
			uint32_t levels = 1;
			while (size > 1)
			{
				size >>= 1;
				++levels;
			}
			return levels;
		}
	}

	OpenGLTextureCube::OpenGLTextureCube(const TextureCubeSpecification& specification)
		: m_specification(specification)
	{
		if (!m_specification.GenerateMips)
		{
			m_mipLevels = 1;
		}
		else if (m_specification.MipLevels > 0)
		{
			m_mipLevels = std::min(m_specification.MipLevels,
				Utils::CalculateCubeMipLevels(m_specification.Size));
		}
		else
		{
			m_mipLevels = Utils::CalculateCubeMipLevels(m_specification.Size);
		}

		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_rendererID);
		glTextureStorage2D(m_rendererID, m_mipLevels,
			Utils::UgeImageFormatToGLInternalFormatCube(m_specification.Format),
			m_specification.Size, m_specification.Size);

		// Trilinear when there is a chain, because the prefiltered map interpolates between
		// levels to reach a roughness that falls between two of them.
		glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER,
			m_mipLevels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Clamping every axis, plus seamless filtering enabled once in OpenGLRendererAPI::Init,
		// is what stops a visible crease along the cube's edges where a filter kernel would
		// otherwise wrap around to the opposite side of the face.
		glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}

	OpenGLTextureCube::~OpenGLTextureCube()
	{
		glDeleteTextures(1, &m_rendererID);
	}

	void OpenGLTextureCube::Bind(uint32_t slot) const
	{
		UG_PROFILE_FUNCTION();

		glBindTextureUnit(slot, m_rendererID);
	}

	void OpenGLTextureCube::GenerateMips()
	{
		if (m_mipLevels > 1)
		{
			glGenerateTextureMipmap(m_rendererID);
		}
	}

}
