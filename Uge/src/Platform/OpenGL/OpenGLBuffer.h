/**
 * @file OpenGLBuffer.h
 * @brief OpenGL vertex and index buffers.
 * @ingroup group_platform
 */

#pragma once

#include "Uge/Renderer/Buffer.h"

namespace Uge
{


	/**
	 * @brief An OpenGL vertex buffer object.
	 * @ingroup group_platform
	 */
	class OpenGLVertexBuffer : public VertexBuffer
	{

	public:
		/**
		 * @brief An empty dynamic buffer.
		 * @param size Size in bytes.
		 */
		OpenGLVertexBuffer(uint32_t size);
		/**
		 * @brief A static buffer holding vertex data.
		 * @param vertices The vertex data.
		 * @param size Its size in bytes.
		 */
		OpenGLVertexBuffer(float* vertices, uint32_t size);
		/** @brief Deletes the GL buffer. */
		virtual ~OpenGLVertexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual const BufferLayout& GetLayout() const override { return m_layout; };
		virtual void SetLayout(const BufferLayout& layout) override { m_layout = layout; }

		virtual void SetData(void* data, uint32_t size) override;

	private:
		uint32_t m_rendererID;
		BufferLayout m_layout;


	};

	/**
	 * @brief An OpenGL buffer of 32-bit indices.
	 * @ingroup group_platform
	 */
	class OpenGLIndexBuffer : public IndexBuffer
	{

	public:
		/**
		 * @brief A buffer holding indices.
		 * @param indices The indices.
		 * @param size Number of indices, not bytes.
		 */
		OpenGLIndexBuffer(uint32_t* indices, uint32_t size);
		/** @brief Deletes the GL buffer. */
		virtual ~OpenGLIndexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;


		virtual uint32_t GetCount() const override { return m_count; }

	private:
		uint32_t m_rendererID;
		uint32_t m_count;


	};


}
