/**
 * @file VertexArray.h
 * @brief Binds vertex buffers and an index buffer into one drawable unit.
 * @ingroup group_renderer
 */

#pragma once

#include <memory>
#include "Uge/Renderer/Buffer.h"

namespace Uge
{


	/**
	 * @brief Groups vertex buffers and an index buffer into a single bindable object.
	 * @ingroup group_renderer
	 *
	 * This is what gets handed to Uge::RenderCommand::DrawIndexed. Binding one restores
	 * every attribute pointer configured when its buffers were added, so per-draw setup
	 * reduces to a single bind.
	 *
	 * @code
	 * Ref<VertexArray> va = VertexArray::Create();
	 * vb->SetLayout({ { ShaderDataType::Float3, "a_Position" } });
	 * va->AddVertexBuffer(vb);
	 * va->SetIndexBuffer(ib);
	 * RenderCommand::DrawIndexed(va);
	 * @endcode
	 */
	class VertexArray
	{

	public:
		/** @brief Releases the vertex array object. */
		virtual ~VertexArray() {}

		/** @brief Binds this vertex array, restoring its attribute configuration. */
		virtual void Bind() const = 0;
		/** @brief Unbinds the vertex array. */
		virtual void Unbind() const = 0;

		/**
		 * @brief Adds a vertex buffer and configures its attributes from its layout.
		 * @param vertexBuffer Buffer to add; a layout must already be set on it.
		 *
		 * Several buffers may be added, which is how interleaved and separate attribute
		 * streams are both supported. Attribute indices continue from the previous buffer.
		 */
		virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)  = 0;
		/**
		 * @brief Sets the index buffer used for indexed draws.
		 * @param indexBuffer Buffer to associate; replaces any previous one.
		 */
		virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)  = 0;

		/**
		 * @brief The vertex buffers added so far.
		 * @return Const reference to the buffer list, in insertion order.
		 */
		virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const = 0;
		/**
		 * @brief The associated index buffer.
		 * @return Const reference to the index buffer; null if none was set.
		 */
		virtual const Ref<IndexBuffer>& GetIndexBuffers() const = 0;
		

		/**
		 * @brief Creates a vertex array for the active graphics API.
		 * @return The backend's vertex array implementation.
		 */
		static Ref<VertexArray> Create();

	};



}

