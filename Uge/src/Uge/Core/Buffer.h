/**
 * @file Buffer.h
 * @brief Raw byte-buffer types used for file and asset data.
 * @ingroup group_core
 */

#pragma once

#include <stdint.h>
#include <cstring>

namespace Uge
{
	// Non-owning raw buffer struct
	/**
	 * @brief A non-owning pointer/size pair over raw bytes.
	 * @ingroup group_core
	 *
	 * Copying a Buffer copies the pointer, not the bytes, and the destructor does nothing.
	 * Whoever calls Allocate() (or Copy()) is responsible for a matching Release().
	 *
	 * @warning Because ownership is manual, prefer ScopedBuffer wherever the lifetime is
	 * tied to a scope. Use Buffer for views and for data owned elsewhere.
	 *
	 * @code
	 * Buffer data = FileSystem::ReadFileBinary(path);
	 * auto* words = data.As<uint32_t>();
	 * data.Release();
	 * @endcode
	 */
	struct Buffer
	{

		uint8_t* Data = nullptr; ///< First byte, or `nullptr` when empty.
		uint64_t Size = 0; ///< Number of bytes addressed by #Data.

		/** @brief Constructs an empty buffer that owns nothing. */
		Buffer() = default;
		/**
		 * @brief Allocates an uninitialized buffer.
		 * @param size Number of bytes to allocate.
		 */
		Buffer(uint64_t size) 
		{
			Allocate(size);

		}

		/**
		 * @brief Creates a view over memory owned by someone else.
		 * @param data Pointer to the first byte; not copied and not freed by this buffer.
		 * @param size Number of bytes visible through the view.
		 */
		Buffer(const void* data, uint64_t size)
			: Data((uint8_t*)data), Size(size)
		{
		}

		/** @brief Copies the pointer and size; does **not** copy the bytes. @see Copy */
		Buffer(const Buffer&) = default;

		/**
		 * @brief Deep-copies a buffer into freshly allocated memory.
		 * @param other Source buffer to duplicate.
		 * @return A new buffer owning its own copy of the bytes; the caller must Release() it.
		 */
		static Buffer Copy(Buffer other)
		{
			Buffer result(other.Size);
			memcpy(result.Data, other.Data, other.Size);
			return result;
		}
	
		/**
		 * @brief Releases any current allocation and allocates @p size uninitialized bytes.
		 * @param size Number of bytes to allocate.
		 */
		void Allocate(uint64_t size)
		{
			Release();

			Data = (uint8_t*)malloc(size);
			Size = size;

		}

		/** @brief Frees the memory and resets the buffer to empty. Safe to call twice. */
		void Release()
		{
			free(Data);
			Data = nullptr;
			Size = 0;
		}

		/**
		 * @brief Reinterprets the bytes as a pointer to `T`.
		 * @tparam T Type to view the data as.
		 * @return The data pointer cast to `T*`; `nullptr` if the buffer is empty.
		 */
		template <typename T>
		T* As()
		{
			return (T*)Data;
		}

		/**
		 * @brief Tests whether the buffer holds data.
		 * @return `true` if the data pointer is non-null.
		 */
		operator bool() const
		{

			return (bool)Data;
		}
	};

	/**
	 * @brief RAII wrapper that releases its Buffer when it goes out of scope.
	 * @ingroup group_core
	 *
	 * @code
	 * ScopedBuffer data = FileSystem::ReadFileBinary(path);
	 * if (data)
	 *     Process(data.Data(), data.Size());   // freed automatically
	 * @endcode
	 *
	 * @warning Non-copyable in intent: copying one would free the same memory twice.
	 */
	struct ScopedBuffer
	{

		/**
		 * @brief Takes ownership of an existing buffer.
		 * @param buffer Buffer whose memory this object will release.
		 */
		ScopedBuffer(Buffer buffer)
			: m_buffer(buffer)
		{
		}

		/**
		 * @brief Allocates an uninitialized buffer of @p size bytes.
		 * @param size Number of bytes to allocate.
		 */
		ScopedBuffer(uint64_t size)
			: m_buffer(size)
		{
		}

		/** @brief Releases the owned buffer. */
		~ScopedBuffer()
		{
			m_buffer.Release();
		}

		/**
		 * @brief Pointer to the first byte.
		 * @return The owned buffer's data pointer.
		 */
		uint8_t*  Data() { return m_buffer.Data; }
		/**
		 * @brief Size of the owned buffer.
		 * @return Size in bytes.
		 */
		uint64_t  Size() { return m_buffer.Size; }

		/**
		 * @brief Reinterprets the bytes as a pointer to `T`.
		 * @tparam T Type to view the data as.
		 * @return The data pointer cast to `T*`.
		 */
		template<typename T>
		T* As()
		{
			return m_buffer.As<T>();
		}

		/**
		 * @brief Tests whether the owned buffer holds data.
		 * @return `true` if the buffer is non-empty.
		 */
		operator bool() const
		{
			return m_buffer;
		}

	private:
		Buffer m_buffer;
	};

}