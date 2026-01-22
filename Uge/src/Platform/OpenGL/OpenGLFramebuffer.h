#pragma once

#include "Uge/Renderer/Framebuffer.h"

namespace Uge
{
	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(const FramebufferSpecification& spec);
		virtual ~OpenGLFramebuffer();

		virtual const FramebufferSpecification& GetSpecification() const override { return m_specification; };

		void Invalidate();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void Resize(uint32_t width, uint32_t height) override;

		virtual uint32_t GetColorAttachment() const override { return m_colorAttachment; };

	private:
		uint32_t m_rendererID = 0;
		uint32_t m_colorAttachment = 0, m_depthAttachment = 0;
		FramebufferSpecification m_specification;



	};




}