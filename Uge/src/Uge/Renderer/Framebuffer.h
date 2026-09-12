/**
 * @file Framebuffer.h
 * @brief Off-screen render targets and their attachment description.
 * @ingroup group_renderer
 */

#pragma once

#include "Uge/Core/Core.h"

namespace Uge
{
	/**
	 * @brief Formats available for a framebuffer attachment.
	 * @ingroup group_renderer
	 *
	 * `RED_INTEGER` is what makes mouse picking work: the scene is drawn a second time with
	 * each fragment writing its entity ID, and Uge::Framebuffer::ReadPixel reads it back.
	 *
	 * `RGBA16F` is what makes the scene target able to hold light: an `RGBA8` attachment
	 * clamps at `1.0` on write, so a bright emissive surface is already clipped by the time
	 * a tonemap or bloom pass could do anything with it.
	 */
	enum class FramebufferTextureFormat
	{

		None = 0, ///< Unset / invalid.

		// Color
		RGBA8, ///< Standard 8-bit-per-channel colour attachment.
		RGBA16F, ///< Half-float colour attachment, for linear values above `1.0`.
		RED_INTEGER, ///< Single integer channel; used for per-pixel entity IDs.

		// Depth/Stencil
		DEPTH24STENCIL8, ///< Combined 24-bit depth and 8-bit stencil.

		// Defaults
		Depth = DEPTH24STENCIL8 ///< Alias for the default depth format.

	};

	/**
	 * @brief Describes a single framebuffer attachment.
	 * @ingroup group_renderer
	 * @todo Filtering and wrap modes are not configurable yet.
	 */
	struct FramebufferTextureSpecification
	{

		/** @brief Constructs an unset attachment specification. */
		FramebufferTextureSpecification() = default;
		/**
		 * @brief Constructs an attachment specification.
		 * @param format Attachment format. Implicit, so a format can be written directly in an
		 *        attachment list.
		 */
		FramebufferTextureSpecification(FramebufferTextureFormat format)
			: TextureFormat(format) {}


		FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None; ///< Format of this attachment.
		// TODO: Filtering/Wrap
	};

	/**
	 * @brief The ordered list of attachments a framebuffer should have.
	 * @ingroup group_renderer
	 *
	 * The order matters: it determines the attachment index passed to
	 * Uge::Framebuffer::GetColorAttachment and Uge::Framebuffer::ReadPixel.
	 */
	struct FramebufferAttachmentSpecification
	{
		/** @brief Constructs an empty attachment list. */
		FramebufferAttachmentSpecification() = default;
		/**
		 * @brief Constructs an attachment list.
		 * @param attachments Attachments in index order; a depth format may appear once.
		 */
		FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> attachments)
			: Attachments(attachments) {}

		std::vector<FramebufferTextureSpecification> Attachments; ///< Attachments, in index order.

	};

	/**
	 * @brief Full description of a framebuffer.
	 * @ingroup group_renderer
	 *
	 * @code
	 * FramebufferSpecification spec;
	 * spec.Attachments = { FramebufferTextureFormat::RGBA8,         // colour
	 *                      FramebufferTextureFormat::RED_INTEGER,  // entity ID
	 *                      FramebufferTextureFormat::Depth };
	 * spec.Width  = 1280;
	 * spec.Height = 720;
	 * m_framebuffer = Framebuffer::Create(spec);
	 * @endcode
	 */
	struct FramebufferSpecification
	{
		/** @brief Render target dimensions in pixels. */
		uint32_t Width, Height; ///< Render target dimensions in pixels.
		FramebufferAttachmentSpecification Attachments; ///< Attachments to create.
		uint32_t Samples = 1; ///< MSAA sample count; `1` disables multisampling.

		bool SwapChainTarget = false; ///< `true` to target the window's backbuffer instead of a texture.


	};


	/**
	 * @brief An off-screen render target with one or more attachments.
	 * @ingroup group_renderer
	 *
	 * The editor renders the scene into one of these and displays its colour attachment as
	 * an ImGui image, which is what makes the viewport a dockable panel rather than the
	 * whole window.
	 *
	 * @warning Resizing is not automatic. When the viewport panel changes size, call
	 * Resize() — rendering to a framebuffer whose size no longer matches the viewport
	 * produces a stretched or clipped image.
	 */
	class Framebuffer
	{
	public:
		/** @brief Releases the framebuffer and its attachments. */
		virtual ~Framebuffer() {};
		//virtual FramebufferSpecification& GetSpecification() = 0;
		/**
		 * @brief The specification this framebuffer was created from.
		 * @return Const reference to the specification, with the current size.
		 */
		virtual const FramebufferSpecification& GetSpecification() const = 0;

		/** @brief Binds this framebuffer as the render target and sets the viewport to its size. */
		virtual void Bind() = 0;
		/** @brief Restores the window's default framebuffer as the render target. */
		virtual void Unbind() = 0;

		/**
		 * @brief Recreates the attachments at a new size.
		 * @param width New width in pixels.
		 * @param height New height in pixels.
		 * @note Destroys and recreates the GPU textures, discarding their contents.
		 */
		virtual void Resize(uint32_t width ,uint32_t height) = 0;
		/**
		 * @brief Reads back one pixel from an integer attachment.
		 * @param attachmentIndex Index of the attachment to read.
		 * @param x Pixel x coordinate.
		 * @param y Pixel y coordinate.
		 * @return The integer stored at that pixel; `-1` where nothing was drawn.
		 *
		 * How the editor resolves a click to an entity. Reading back stalls the pipeline, so
		 * do it once per click, not per frame.
		 */
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;

		/**
		 * @brief The native handle of a colour attachment.
		 * @param index Attachment index, in the order declared in the specification.
		 * @return The OpenGL texture name, suitable for `ImGui::Image`.
		 */
		virtual uint32_t GetColorAttachment(uint32_t index = 0) const = 0;

		/**
		 * @brief Binds a colour attachment to a sampler slot so a later pass can read it.
		 * @param index Attachment index, in the order declared in the specification.
		 * @param slot Texture unit to bind to.
		 *
		 * What lets a post-process pass sample the scene it is resolving. Exists so callers
		 * do not have to take GetColorAttachment()'s native handle and bind it themselves,
		 * which would put backend calls in platform-agnostic code.
		 *
		 * @warning The attachment must not also be the current render target. Reading and
		 * writing the same texture in one draw is undefined; resolve into a second
		 * framebuffer.
		 */
		virtual void BindColorAttachment(uint32_t index, uint32_t slot) const = 0;

		/**
		 * @brief Clears an integer attachment to a constant.
		 * @param attachmentIndex Index of the attachment to clear.
		 * @param value Value to write to every pixel.
		 *
		 * The entity-ID attachment is cleared to `-1` each frame so empty space reads back as
		 * "no entity".
		 */
		virtual void ClearAttachment(uint32_t attachmentIndex, int value) = 0;

		/**
		 * @brief Creates a framebuffer for the active graphics API.
		 * @param spec Size, attachments and sample count.
		 * @return The backend's framebuffer implementation.
		 */
		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);



	};


}

