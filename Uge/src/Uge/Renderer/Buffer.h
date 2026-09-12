/**
 * @file Buffer.h
 * @brief Vertex and index buffers, and the vertex layout description.
 * @ingroup group_renderer
 *
 * @note Distinct from Uge/Core/Buffer.h, which is a raw CPU byte buffer. This header
 * describes GPU buffers.
 */

#pragma once


namespace Uge
{

	/**
	 * @brief Element types a vertex attribute can have.
	 * @ingroup group_renderer
	 *
	 * Backend-independent; Uge::OpenGLVertexArray maps these to the matching GL types when
	 * it configures the attribute pointers.
	 */
	enum class ShaderDataType
	{

		None = 0, ///< Unset / invalid.
		Float, ///< One 32-bit float.
		Float2, ///< Two 32-bit floats.
		Float3, ///< Three 32-bit floats.
		Float4, ///< Four 32-bit floats.
		Mat3x3, ///< 3x3 float matrix.
		Mat4x4, ///< 4x4 float matrix.
		Int, ///< One 32-bit signed integer.
		Int2, ///< Two 32-bit signed integers.
		Int3, ///< Three 32-bit signed integers.
		Int4, ///< Four 32-bit signed integers.
		Bool ///< One byte-sized boolean.


	};

	/**
	 * @brief Size in bytes of one element of @p type.
	 * @param type Shader data type to measure.
	 * @return Size in bytes; `0` for an unrecognized type, which also asserts in Debug.
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
	 * @brief One attribute within a vertex layout.
	 * @ingroup group_renderer
	 *
	 * The #offset is not supplied by the caller: Uge::BufferLayout computes it, along with
	 * the stride, from the order the elements were declared in.
	 */
	struct BufferElement
	{

		std::string name; ///< Attribute name, matching the shader input.
		ShaderDataType type; ///< Element type.
		uint32_t offset; ///< Byte offset within the vertex; filled in by Uge::BufferLayout.
		uint32_t size; ///< Size of this attribute in bytes.
		bool normalized; ///< Whether integer data is normalized into `[0, 1]`.

		/** @brief Constructs an empty element. */
		BufferElement() = default;

		/**
		 * @brief Describes one vertex attribute.
		 * @param type Element type.
		 * @param name Attribute name, matching the shader input; used for debugging.
		 * @param normalized `true` to map integer data into `[0, 1]` when read by the shader.
		 */
		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
			: name(name), type(type), size(ShaderDataTypeSize(type)), 
			offset(0), normalized(normalized) { }

		/**
		 * @brief Number of scalar components in this attribute.
		 * @return For example `3` for `Float3` and `16` for `Mat4x4`; `0` if the type is
		 *         unrecognized, which also asserts in Debug.
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
	 * @brief An ordered set of Uge::BufferElement describing one vertex's memory layout.
	 * @ingroup group_renderer
	 *
	 * Construct from an initializer list; offsets and the stride are computed
	 * automatically, in declaration order.
	 *
	 * @code
	 * vertexBuffer->SetLayout({
	 *     { ShaderDataType::Float3, "a_Position" },
	 *     { ShaderDataType::Float4, "a_Color"    },
	 *     { ShaderDataType::Float2, "a_TexCoord" }
	 * });
	 * @endcode
	 *
	 * @warning The element order must match the vertex struct on the CPU side. Nothing
	 * verifies this, and a mismatch shows up as scrambled geometry rather than an error.
	 */
	class BufferLayout
	{
	public:

		/** @brief Constructs an empty layout. */
		BufferLayout() {}

		/**
		 * @brief Builds a layout and computes each element's offset and the total stride.
		 * @param elements Attributes in the order they appear in the vertex.
		 */
		BufferLayout(const std::initializer_list<BufferElement>& elements) 
			: m_elements(elements)
		{
			
			CalculateOffsetsAndStride();
			
		}

		/**
		 * @brief Distance between consecutive vertices.
		 * @return Stride in bytes.
		 */
		inline uint32_t GetStride() const { return m_stride; }
		/**
		 * @brief The attributes making up this layout.
		 * @return Const reference to the element list.
		 */
		inline const std::vector<BufferElement>& GetElements() const { return m_elements; }


		/** @brief Iterator to the first attribute. @return Begin iterator. */
		std::vector<BufferElement>::iterator begin() { return m_elements.begin(); }
		/** @brief Iterator past the last attribute. @return End iterator. */
		std::vector<BufferElement>::iterator end() { return m_elements.end(); }

		/** @brief Const iterator to the first attribute. @return Begin iterator. */
		std::vector<BufferElement>::const_iterator begin() const { return m_elements.begin(); }
		/** @brief Const iterator past the last attribute. @return End iterator. */
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
	 * @brief A GPU buffer of vertex data, together with its layout.
	 * @ingroup group_renderer
	 *
	 * Create one of two ways: with initial data, for static geometry, or with a size only,
	 * for a dynamic buffer refilled each frame through SetData(). The 2D batcher uses the
	 * second form.
	 *
	 * @warning A layout must be set before the buffer is added to a Uge::VertexArray.
	 */
	class VertexBuffer
	{

	public:
		/** @brief Releases the GPU buffer. */
		virtual ~VertexBuffer() {}

		/** @brief Binds this buffer as the active vertex buffer. */
		virtual void Bind() const = 0;
		/** @brief Unbinds the vertex buffer. */
		virtual void Unbind() const = 0;

		/**
		 * @brief The layout describing this buffer's vertices.
		 * @return Const reference to the layout.
		 */
		virtual const BufferLayout& GetLayout() const = 0;
		/**
		 * @brief Assigns the vertex layout.
		 * @param layout Attribute description matching the vertex struct.
		 */
		virtual void SetLayout(const BufferLayout& layout) = 0;
		
		/**
		 * @brief Uploads new contents to a dynamic buffer.
		 * @param data Source bytes to copy.
		 * @param size Number of bytes; must not exceed the buffer's allocated size.
		 */
		virtual void SetData(void* data, uint32_t size) = 0;

		/**
		 * @brief Creates an empty, dynamically updatable vertex buffer.
		 * @param size Capacity in bytes.
		 * @return The backend's vertex buffer implementation.
		 */
		static Ref<VertexBuffer> Create(uint32_t size);
		/**
		 * @brief Creates a vertex buffer initialized with static data.
		 * @param vertices Source vertex data to upload.
		 * @param size Size of @p vertices in bytes.
		 * @return The backend's vertex buffer implementation.
		 */
		static Ref<VertexBuffer> Create(float* vertices, uint32_t size);

	};

	/**
	 * @brief A GPU buffer of 32-bit vertex indices.
	 * @ingroup group_renderer
	 */
	class IndexBuffer
	{
	public:
		/** @brief Releases the GPU buffer. */
		virtual ~IndexBuffer() {}

		/** @brief Binds this buffer as the active index buffer. */
		virtual void Bind() const = 0;
		/** @brief Unbinds the index buffer. */
		virtual void Unbind() const = 0;

		/**
		 * @brief Number of indices held.
		 * @return Index count, which is the element count for a full draw.
		 */
		virtual uint32_t GetCount() const = 0;

		/**
		 * @brief Creates an index buffer initialized with data.
		 * @param indices Source indices to upload.
		 * @param count Number of indices, **not** a byte count.
		 * @return The backend's index buffer implementation.
		 */
		static Ref<IndexBuffer> Create(uint32_t* indices, uint32_t count);


	};


}


