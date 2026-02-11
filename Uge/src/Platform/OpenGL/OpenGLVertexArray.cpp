#include <ugpch.h>
#include "OpenGLVertexArray.h"

#include <glad/glad.h>

namespace Uge
{

	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{

		switch (type)
		{
		case Uge::ShaderDataType::Float:	return GL_FLOAT;
		case Uge::ShaderDataType::Float2:	return GL_FLOAT;
		case Uge::ShaderDataType::Float3:	return GL_FLOAT;
		case Uge::ShaderDataType::Float4:	return GL_FLOAT;
		case Uge::ShaderDataType::Mat3x3:	return GL_FLOAT;
		case Uge::ShaderDataType::Mat4x4:	return GL_FLOAT;
		case Uge::ShaderDataType::Int:		return GL_INT;
		case Uge::ShaderDataType::Int2:		return GL_INT;
		case Uge::ShaderDataType::Int3:		return GL_INT;
		case Uge::ShaderDataType::Int4:		return GL_INT;
		case Uge::ShaderDataType::Bool:		return GL_BOOL;

		}


	}


	OpenGLVertexArray::OpenGLVertexArray()
	{
		UG_PROFILE_FUNCTION();

		glCreateVertexArrays(1, &m_rendererID);


	}
	OpenGLVertexArray::~OpenGLVertexArray()
	{
		UG_PROFILE_FUNCTION();

		glDeleteVertexArrays(1, &m_rendererID);



	}
	void OpenGLVertexArray::Bind() const
	{
		UG_PROFILE_FUNCTION();

		glBindVertexArray(m_rendererID);

	}

	void OpenGLVertexArray::Unbind() const
	{
		UG_PROFILE_FUNCTION();

		glBindVertexArray(0);


	}

	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{

		UG_PROFILE_FUNCTION();

		UG_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

		glBindVertexArray(m_rendererID);

		vertexBuffer->Bind();


		uint32_t index = 0;

		const auto& layout = vertexBuffer->GetLayout();

		for (const auto& element : layout)
		{

			switch (element.type)
			{
				case Uge::ShaderDataType::None:
				case Uge::ShaderDataType::Float:
				case Uge::ShaderDataType::Float2:
				case Uge::ShaderDataType::Float3:
				case Uge::ShaderDataType::Float4:
				{
					glEnableVertexAttribArray(m_VertexBufferIndex);
					glVertexAttribPointer(m_VertexBufferIndex,
						element.GetComponentCount(),
						ShaderDataTypeToOpenGLBaseType(element.type),
						element.normalized ? GL_TRUE : GL_FALSE,
						layout.GetStride(),
						(const void*)element.offset);
					m_VertexBufferIndex++;
					break;
				}
				case Uge::ShaderDataType::Int:
				case Uge::ShaderDataType::Int2:
				case Uge::ShaderDataType::Int3:
				case Uge::ShaderDataType::Int4:
				case Uge::ShaderDataType::Bool:
				{
					glEnableVertexAttribArray(m_VertexBufferIndex);
					glVertexAttribIPointer(m_VertexBufferIndex,
						element.GetComponentCount(),
						ShaderDataTypeToOpenGLBaseType(element.type),
						layout.GetStride(),
						(const void*)element.offset);
					m_VertexBufferIndex++;
					break;

				}

				case Uge::ShaderDataType::Mat3x3:
				case Uge::ShaderDataType::Mat4x4:
				{
					uint8_t count = element.GetComponentCount();
					for (uint8_t i = 0; i < count; i++)
					{
						glEnableVertexAttribArray(m_VertexBufferIndex);
						glVertexAttribPointer(m_VertexBufferIndex,
							count,
							ShaderDataTypeToOpenGLBaseType(element.type),
							element.normalized ? GL_TRUE : GL_FALSE,
							layout.GetStride(),
							(const void*)(element.offset + sizeof(float) * count * i));
						glVertexAttribDivisor(m_VertexBufferIndex, 1);
						m_VertexBufferIndex++;
					}
					break;
				}
				
				
				default:
				{
					UG_CORE_ASSERT(false, "Unknown ShaderDataType!");

				}
			
			}
		}

		m_vertexBuffers.push_back(vertexBuffer);

	}

	void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{

		UG_PROFILE_FUNCTION();

		glBindVertexArray(m_rendererID);

		indexBuffer->Bind();

		m_indexBuffer = indexBuffer;

	}

}

