/**
 * @file Framebuffer.h
 * @brief Off-screen render targets.
 * @ingroup group_renderer
 */

#pragma once

#include "Uge/Core/Core.h"

namespace Uge
{
	/**
	 * @brief Framebuffer attachment formats.
	 * @ingroup group_renderer
	 */
	enum class FramebufferTextureFormat
	{

		None = 0,

		// Color
		RGBA8,
		RGBA16F, ///< Half float; holds values above `1.0`, which `RGBA8` clamps.
		RED_INTEGER, ///< One integer channel, for entity IDs.

		// Depth/Stencil
		DEPTH24STENCIL8,

		// Defaults
		Depth = DEPTH24STENCIL8 ///< The default depth format.

	};

	/**
	 * @brief One framebuffer attachment.
	 * @ingroup group_renderer
	 */
	struct FramebufferTextureSpecification
	{

		/** @brief An attachment with no format. */
		FramebufferTextureSpecification() = default;
		/**
		 * @brief An attachment of a format. Implicit, so a list can hold bare formats.
		 * @param format The format.
		 */
		FramebufferTextureSpecification(FramebufferTextureFormat format)
			: TextureFormat(format) {}


		FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None; ///< The format.
		// TODO: Filtering/Wrap
	};

	/**
	 * @brief A framebuffer's attachments. Their order gives the attachment indices.
	 * @ingroup group_renderer
	 */
	struct FramebufferAttachmentSpecification
	{
		/** @brief No attachments. */
		FramebufferAttachmentSpecification() = default;
		/**
		 * @brief A list of attachments.
		 * @param attachments The attachments in index order, with at most one depth format.
		 */
		FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> attachments)
			: Attachments(attachments) {}

		std::vector<FramebufferTextureSpecification> Attachments; ///< The attachments, in index order.

	};

	/**
	 * @brief The settings a framebuffer is created from.
	 * @ingroup group_renderer
	 */
	struct FramebufferSpecification
	{
		/** @brief Size in pixels. */
		uint32_t Width, Height; ///< Size in pixels.
		FramebufferAttachmentSpecification Attachments; ///< The attachments.
		uint32_t Samples = 1; ///< MSAA sample count; `1` for none.

		bool SwapChainTarget = false; ///< Whether to draw to the window instead of a texture.


	};


	/**
	 * @brief An off-screen render target. It doesn't resize itself; call Resize().
	 * @ingroup group_renderer
	 */
	class Framebuffer
	{
	public:
		/** @brief Releases the framebuffer and its attachments. */
		virtual ~Framebuffer() {};
		//virtual FramebufferSpecification& GetSpecification() = 0;
		/**
		 * @brief The specification, with the current size.
		 * @return The specification.
		 */
		virtual const FramebufferSpecification& GetSpecification() const = 0;

		/** @brief Binds the framebuffer and sets the viewport to its size. */
		virtual void Bind() = 0;
		/** @brief Binds the window's framebuffer. */
		virtual void Unbind() = 0;

		/**
		 * @brief Recreates the attachments at a new size, losing their contents.
		 * @param width Width in pixels.
		 * @param height Height in pixels.
		 *
		 * A zero size, or one above the GPU's limit, is ignored with a warning.
		 */
		virtual void Resize(uint32_t width ,uint32_t height) = 0;
		/**
		 * @brief Reads one pixel of an integer attachment. Stalls the GPU.
		 * @param attachmentIndex The attachment.
		 * @param x Pixel x.
		 * @param y Pixel y.
		 * @return The value at the pixel.
		 */
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;

		/**
		 * @brief A colour attachment's texture.
		 * @param index The attachment.
		 * @return The OpenGL texture name.
		 */
		virtual uint32_t GetColorAttachment(uint32_t index = 0) const = 0;

		/**
		 * @brief Binds a colour attachment to a texture slot.
		 * @param index The attachment.
		 * @param slot Texture unit.
		 *
		 * Don't sample an attachment of the framebuffer being drawn to.
		 */
		virtual void BindColorAttachment(uint32_t index, uint32_t slot) const = 0;

		/**
		 * @brief Fills an integer attachment with a value.
		 * @param attachmentIndex The attachment.
		 * @param value The value.
		 */
		virtual void ClearAttachment(uint32_t attachmentIndex, int value) = 0;

		/**
		 * @brief Creates a framebuffer.
		 * @param spec The settings.
		 * @return The framebuffer.
		 */
		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);



	};


}

