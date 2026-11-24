/**
 * @file VertexArray.h
 * @brief Vertex arrays.
 * @ingroup group_renderer
 */

#pragma once

#include <memory>
#include "Uge/Renderer/Buffer.h"

namespace Uge
{


	/**
	 * @brief Vertex buffers, their attribute layout and an index buffer, bound together.
	 * @ingroup group_renderer
	 */
	class VertexArray
	{

	public:
		/** @brief Releases the vertex array. */
		virtual ~VertexArray() {}

		/** @brief Binds the vertex array. */
		virtual void Bind() const = 0;
		/** @brief Unbinds the vertex array. */
		virtual void Unbind() const = 0;

		/**
		 * @brief Adds a vertex buffer and sets up its attributes from its layout.
		 *
		 * Attribute indices continue from the previous buffer's.
		 *
		 * @param vertexBuffer The buffer, with its layout set.
		 */
		virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)  = 0;
		/**
		 * @brief Sets the index buffer.
		 * @param indexBuffer The buffer.
		 */
		virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)  = 0;

		/**
		 * @brief The vertex buffers, in the order added.
		 * @return The buffers.
		 */
		virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const = 0;
		/**
		 * @brief The index buffer.
		 * @return The buffer, or null.
		 */
		virtual const Ref<IndexBuffer>& GetIndexBuffers() const = 0;
		

		/**
		 * @brief Creates a vertex array.
		 * @return The vertex array.
		 */
		static Ref<VertexArray> Create();

	};



}

