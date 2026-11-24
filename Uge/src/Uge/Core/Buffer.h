/**
 * @file Buffer.h
 * @brief Raw byte buffers.
 * @ingroup group_core
 */

#pragma once

#include <stdint.h>
#include <cstring>

namespace Uge
{
	/**
	 * @brief A pointer and size over raw bytes.
	 * @ingroup group_core
	 *
	 * Copies share the bytes, and nothing is freed automatically: memory from Allocate()
	 * or Copy() needs a Release().
	 */
	struct Buffer
	{

		uint8_t* Data = nullptr; ///< First byte, or `nullptr` when empty.
		uint64_t Size = 0; ///< Number of bytes.

		/** @brief An empty buffer. */
		Buffer() = default;
		/**
		 * @brief Allocates @p size uninitialized bytes.
		 * @param size Bytes to allocate.
		 */
		Buffer(uint64_t size) 
		{
			Allocate(size);

		}

		/**
		 * @brief A view of memory owned elsewhere.
		 * @param data First byte; not copied or freed.
		 * @param size Number of bytes.
		 */
		Buffer(const void* data, uint64_t size)
			: Data((uint8_t*)data), Size(size)
		{
		}

		/** @brief Copies the pointer and size, not the bytes. */
		Buffer(const Buffer&) = default;

		/**
		 * @brief Copies the bytes into new memory.
		 * @param other Buffer to copy.
		 * @return The copy; the caller must Release() it.
		 */
		static Buffer Copy(Buffer other)
		{
			Buffer result(other.Size);
			memcpy(result.Data, other.Data, other.Size);
			return result;
		}
	
		/**
		 * @brief Releases the current memory and allocates @p size uninitialized bytes.
		 * @param size Bytes to allocate.
		 */
		void Allocate(uint64_t size)
		{
			Release();

			Data = (uint8_t*)malloc(size);
			Size = size;

		}

		/** @brief Frees the memory and empties the buffer. Safe to call twice. */
		void Release()
		{
			free(Data);
			Data = nullptr;
			Size = 0;
		}

		/**
		 * @brief The bytes as a `T*`.
		 * @tparam T Type to view them as.
		 * @return The pointer; null if the buffer is empty.
		 */
		template <typename T>
		T* As()
		{
			return (T*)Data;
		}

		/**
		 * @brief Whether the buffer holds data.
		 * @return `true` if #Data is set.
		 */
		operator bool() const
		{

			return (bool)Data;
		}
	};

	/**
	 * @brief Owns a Buffer and releases it on destruction.
	 * @ingroup group_core
	 *
	 * Don't copy one: both copies would free the memory.
	 */
	struct ScopedBuffer
	{

		/**
		 * @brief Takes ownership of a buffer.
		 * @param buffer Buffer to release on destruction.
		 */
		ScopedBuffer(Buffer buffer)
			: m_buffer(buffer)
		{
		}

		/**
		 * @brief Allocates @p size uninitialized bytes.
		 * @param size Bytes to allocate.
		 */
		ScopedBuffer(uint64_t size)
			: m_buffer(size)
		{
		}

		/** @brief Releases the buffer. */
		~ScopedBuffer()
		{
			m_buffer.Release();
		}

		/**
		 * @brief The first byte.
		 * @return The data pointer.
		 */
		uint8_t*  Data() { return m_buffer.Data; }
		/**
		 * @brief The size.
		 * @return Size in bytes.
		 */
		uint64_t  Size() { return m_buffer.Size; }

		/**
		 * @brief The bytes as a `T*`.
		 * @tparam T Type to view them as.
		 * @return The pointer.
		 */
		template<typename T>
		T* As()
		{
			return m_buffer.As<T>();
		}

		/**
		 * @brief Whether the buffer holds data.
		 * @return `true` if it does.
		 */
		operator bool() const
		{
			return m_buffer;
		}

	private:
		Buffer m_buffer;
	};

}