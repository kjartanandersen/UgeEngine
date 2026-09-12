/**
 * @file OpenGLBuffer.h
 * @brief OpenGL implementations of the vertex and index buffers.
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
		 * @brief Allocates a dynamic buffer with no initial contents.
		 * @param size Capacity in bytes.
		 */
		OpenGLVertexBuffer(uint32_t size);
		/**
		 * @brief Allocates a static buffer and uploads its contents.
		 * @param vertices Source vertex data.
		 * @param size Size of @p vertices in bytes.
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
	 * @brief An OpenGL element array buffer holding 32-bit indices.
	 * @ingroup group_platform
	 */
	class OpenGLIndexBuffer : public IndexBuffer
	{

	public:
		/**
		 * @brief Allocates the buffer and uploads the indices.
		 * @param indices Source index data.
		 * @param size Number of indices, **not** a byte count.
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
