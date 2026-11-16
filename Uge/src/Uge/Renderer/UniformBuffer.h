/**
 * @file UniformBuffer.h
 * @brief Uniform buffers.
 * @ingroup group_renderer
 */

#pragma once

#include "Uge/Core/Core.h"

namespace Uge
{

	/**
	 * @brief A uniform buffer at a binding point, seen by every shader that declares it.
	 * @ingroup group_renderer
	 *
	 * The C++ struct must match the block's `std140` layout, which pads a `vec3` to 16 bytes.
	 */
	class UniformBuffer
	{
	public:
		/** @brief Releases the buffer. */
		virtual ~UniformBuffer() {}
		/**
		 * @brief Writes into the buffer.
		 * @param data Bytes to copy.
		 * @param size Number of bytes.
		 * @param offset Byte offset to write at.
		 */
		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;

		/**
		 * @brief Creates a uniform buffer and binds it.
		 * @param size Size in bytes.
		 * @param binding Binding point, as in the shader's `layout(binding = N)`.
		 * @return The buffer.
		 */
		static Ref<UniformBuffer> Create(uint32_t size, uint32_t binding);
	};


}