/**
 * @file OpenGLFramebuffer.h
 * @brief OpenGL implementation of Uge::Framebuffer.
 * @ingroup group_platform
 */

#pragma once

#include "Uge/Renderer/Framebuffer.h"

namespace Uge
{
	/**
	 * @brief An OpenGL framebuffer with colour and depth attachments.
	 * @ingroup group_platform
	 */
	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		/**
		 * @brief Creates the framebuffer and its attachments.
		 * @param spec The settings.
		 */
		OpenGLFramebuffer(const FramebufferSpecification& spec);
		/** @brief Deletes the framebuffer and its attachments. */
		virtual ~OpenGLFramebuffer();

		virtual const FramebufferSpecification& GetSpecification() const override { return m_specification; };

		/**
		 * @brief Recreates the framebuffer and its attachments, losing their contents.
		 */
		void Invalidate();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) override;

		virtual void ClearAttachment(uint32_t attachmentIndex, int value) override;


		virtual uint32_t GetColorAttachment(uint32_t index = 0) const override
		{
			UG_CORE_ASSERT(index < m_colorAttachments.size());
			return m_colorAttachments[index];
		};

		virtual void BindColorAttachment(uint32_t index, uint32_t slot) const override;

	private:
		uint32_t m_rendererID = 0;
		FramebufferSpecification m_specification;

		std::vector<FramebufferTextureSpecification> m_colorAttachmentSpecifications;
		FramebufferTextureSpecification m_depthAttachmentSpecification = FramebufferTextureFormat::None;

		std::vector<uint32_t> m_colorAttachments;
		uint32_t m_depthAttachment = 0;


	};




}