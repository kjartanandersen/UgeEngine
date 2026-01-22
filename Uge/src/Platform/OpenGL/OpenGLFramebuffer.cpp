#include <ugpch.h>
#include "OpenGLFramebuffer.h"

#include <glad/glad.h>

namespace Uge
{

	static const uint32_t m_maxFramebufferSize = 8192;


	OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec)
		: m_specification(spec)
	{

		Invalidate();

	}

	OpenGLFramebuffer::~OpenGLFramebuffer()
	{
		glDeleteFramebuffers(1, &m_rendererID);
		glDeleteTextures(1, &m_colorAttachment);
		glDeleteTextures(1, &m_depthAttachment);


	}

	void OpenGLFramebuffer::Invalidate()
	{

		if (m_rendererID)
		{

			glDeleteFramebuffers(1, &m_rendererID);
			glDeleteTextures(1, &m_colorAttachment);
			glDeleteTextures(1, &m_depthAttachment);

		}

		glCreateFramebuffers(1, &m_rendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_rendererID);
		

		glCreateTextures(GL_TEXTURE_2D, 1, &m_colorAttachment);
		glBindTexture(GL_TEXTURE_2D, m_colorAttachment);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_specification.Width, m_specification.Height, 0, 
			GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorAttachment, 0);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_depthAttachment);
		glBindTexture(GL_TEXTURE_2D, m_depthAttachment);

		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, m_specification.Width, m_specification.Height);
		// glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_specification.Width, m_specification.Height, 0,
		//  GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depthAttachment, 0);

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			UG_CORE_ASSERT(false, "Framebuffer is incomplete!");

		}


		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}

	void OpenGLFramebuffer::Bind()
	{

		glBindFramebuffer(GL_FRAMEBUFFER, m_rendererID);
		glViewport(0, 0, m_specification.Width, m_specification.Height);
	}

	void OpenGLFramebuffer::Unbind()
	{

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}

	void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height)
	{

		if (width == 0 || height == 0 || width > m_maxFramebufferSize || height > m_maxFramebufferSize)
		{
			UG_CORE_WARN("Attempt to resize framebuffer to: Width: {0}, Height: {1}", width, height);
			return;
		}

		m_specification.Width = width;
		m_specification.Height = height;

		Invalidate();


	}

}