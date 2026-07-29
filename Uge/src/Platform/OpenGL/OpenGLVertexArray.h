/**
 * @file OpenGLVertexArray.h
 * @brief OpenGL implementation of Uge::VertexArray.
 * @ingroup group_platform
 */

#pragma once

#include "Uge/Renderer/VertexArray.h"

namespace Uge
{

	/**
	 * @brief An OpenGL vertex array object.
	 * @ingroup group_platform
	 *
	 * Translates each Uge::BufferElement into a `glVertexAttribPointer` call when a vertex
	 * buffer is added, so the attribute setup is recorded once and restored by binding.
	 */
	class OpenGLVertexArray : public VertexArray
	{

	public:
		/** @brief Creates the vertex array object. */
		OpenGLVertexArray();
		/** @brief Deletes the vertex array object. */
		virtual ~OpenGLVertexArray();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) override;
		virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) override;

		virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const override
		{ 
			return m_vertexBuffers; 
		};
		virtual const Ref<IndexBuffer>& GetIndexBuffers() const override
		{
			return m_indexBuffer;
		};


	private:
		uint32_t m_rendererID;
		uint32_t m_VertexBufferIndex = 0;
		std::vector<Ref<VertexBuffer>> m_vertexBuffers;
		Ref<IndexBuffer> m_indexBuffer;

	};



}

