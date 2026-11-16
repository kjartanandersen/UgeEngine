/**
 * @file Buffer.h
 * @brief GPU vertex and index buffers, and vertex layouts.
 * @ingroup group_renderer
 */

#pragma once


namespace Uge
{

	/**
	 * @brief Vertex attribute types.
	 * @ingroup group_renderer
	 */
	enum class ShaderDataType
	{

		None = 0,
		Float,
		Float2,
		Float3,
		Float4,
		Mat3x3,
		Mat4x4,
		Int, ///< 32-bit signed.
		Int2,
		Int3,
		Int4,
		Bool ///< One byte.


	};

	/**
	 * @brief The size of a type.
	 * @param type The type.
	 * @return Size in bytes; asserts and returns `0` for an unknown type.
	 * @ingroup group_renderer
	 */
	static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{

		switch (type)
		{
			case ShaderDataType::Float:		return 4 * 1;
			case ShaderDataType::Float2:	return 4 * 2;
			case ShaderDataType::Float3:	return 4 * 3;
			case ShaderDataType::Float4:	return 4 * 4;
			case ShaderDataType::Mat3x3:	return 4 * 3 * 3;
			case ShaderDataType::Mat4x4:	return 4 * 4 * 4;
			case ShaderDataType::Int:		return 4 * 1;
			case ShaderDataType::Int2:		return 4 * 2;
			case ShaderDataType::Int3:		return 4 * 3;
			case ShaderDataType::Int4:		return 4 * 4;
			case ShaderDataType::Bool:		return 1;
		}

		UG_CORE_ASSERT(false, "Unknown Shader Data Type!");
		return 0;


	}

	/**
	 * @brief One attribute of a vertex layout.
	 * @ingroup group_renderer
	 */
	struct BufferElement
	{

		std::string name; ///< Attribute name.
		ShaderDataType type; ///< Type.
		uint32_t offset; ///< Byte offset in the vertex, set by Uge::BufferLayout.
		uint32_t size; ///< Size in bytes.
		bool normalized; ///< Whether integer data is mapped to `[0, 1]`.

		/** @brief An empty element. */
		BufferElement() = default;

		/**
		 * @brief Describes an attribute.
		 * @param type Type.
		 * @param name Attribute name.
		 * @param normalized `true` to map integer data to `[0, 1]`.
		 */
		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
			: name(name), type(type), size(ShaderDataTypeSize(type)),
			offset(0), normalized(normalized) { }

		/**
		 * @brief The number of components, such as `3` for `Float3`.
		 * @return The count; asserts and returns `0` for an unknown type.
		 */
		uint32_t GetComponentCount() const
		{

			switch (type)
			{
				case ShaderDataType::Float:		return 1;
				case ShaderDataType::Float2:	return 2;
				case ShaderDataType::Float3:	return 3;
				case ShaderDataType::Float4:	return 4;
				case ShaderDataType::Mat3x3:	return 3 * 3;
				case ShaderDataType::Mat4x4:	return 4 * 4;
				case ShaderDataType::Int:		return 1;
				case ShaderDataType::Int2:		return 2;
				case ShaderDataType::Int3:		return 3;
				case ShaderDataType::Int4:		return 4;
				case ShaderDataType::Bool:		return 1;

			}

			UG_CORE_ASSERT(false, "Unknown Shader Data Type!");
			return 0;

		}


	};


	/**
	 * @brief A vertex's attributes, in order.
	 * @ingroup group_renderer
	 *
	 * The order must match the vertex struct; nothing checks it.
	 */
	class BufferLayout
	{
	public:

		/** @brief An empty layout. */
		BufferLayout() {}

		/**
		 * @brief Builds a layout, computing the offsets and the stride.
		 * @param elements The attributes, in vertex order.
		 */
		BufferLayout(const std::initializer_list<BufferElement>& elements)
			: m_elements(elements)
		{

			CalculateOffsetsAndStride();

		}

		/**
		 * @brief The size of one vertex.
		 * @return Stride in bytes.
		 */
		inline uint32_t GetStride() const { return m_stride; }
		/**
		 * @brief The attributes.
		 * @return The attributes.
		 */
		inline const std::vector<BufferElement>& GetElements() const { return m_elements; }


		/** @brief The first attribute. @return Begin iterator. */
		std::vector<BufferElement>::iterator begin() { return m_elements.begin(); }
		/** @brief Past the last attribute. @return End iterator. */
		std::vector<BufferElement>::iterator end() { return m_elements.end(); }

		/** @brief The first attribute. @return Begin iterator. */
		std::vector<BufferElement>::const_iterator begin() const { return m_elements.begin(); }
		/** @brief Past the last attribute. @return End iterator. */
		std::vector<BufferElement>::const_iterator end() const { return m_elements.end(); }

	private:
		void CalculateOffsetsAndStride()
		{

			uint32_t offset = 0;
			m_stride = 0;
			
			for (auto& element : m_elements)
			{

				element.offset = offset;
				offset += element.size;
				m_stride += element.size;


			}

		}

	private:

		std::vector<BufferElement> m_elements;
		uint32_t m_stride = 0;



	};

	/**
	 * @brief A GPU buffer of vertices, with their layout.
	 * @ingroup group_renderer
	 *
	 * Set the layout before adding the buffer to a Uge::VertexArray.
	 */
	class VertexBuffer
	{

	public:
		/** @brief Releases the buffer. */
		virtual ~VertexBuffer() {}

		/** @brief Binds the buffer. */
		virtual void Bind() const = 0;
		/** @brief Unbinds the buffer. */
		virtual void Unbind() const = 0;

		/**
		 * @brief The vertex layout.
		 * @return The layout.
		 */
		virtual const BufferLayout& GetLayout() const = 0;
		/**
		 * @brief Sets the vertex layout.
		 * @param layout The layout.
		 */
		virtual void SetLayout(const BufferLayout& layout) = 0;

		/**
		 * @brief Replaces the contents.
		 * @param data Bytes to copy.
		 * @param size Number of bytes; at most the buffer's size.
		 */
		virtual void SetData(void* data, uint32_t size) = 0;

		/**
		 * @brief Creates an empty buffer, to fill with SetData().
		 * @param size Size in bytes.
		 * @return The buffer.
		 */
		static Ref<VertexBuffer> Create(uint32_t size);
		/**
		 * @brief Creates a buffer holding vertex data.
		 * @param vertices The vertex data.
		 * @param size Its size in bytes.
		 * @return The buffer.
		 */
		static Ref<VertexBuffer> Create(float* vertices, uint32_t size);

	};

	/**
	 * @brief A GPU buffer of 32-bit indices.
	 * @ingroup group_renderer
	 */
	class IndexBuffer
	{
	public:
		/** @brief Releases the buffer. */
		virtual ~IndexBuffer() {}

		/** @brief Binds the buffer. */
		virtual void Bind() const = 0;
		/** @brief Unbinds the buffer. */
		virtual void Unbind() const = 0;

		/**
		 * @brief The number of indices.
		 * @return The count.
		 */
		virtual uint32_t GetCount() const = 0;

		/**
		 * @brief Creates a buffer holding indices.
		 * @param indices The indices.
		 * @param count Number of indices, not bytes.
		 * @return The buffer.
		 */
		static Ref<IndexBuffer> Create(uint32_t* indices, uint32_t count);


	};


}


