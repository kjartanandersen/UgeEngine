#include <ugpch.h>
#include "OpenGLBuffer.h"

#include <glad/glad.h>

namespace Uge
{

	/************************************************************
	*					Vertex Buffers
	************************************************************/


	OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, uint32_t size)
	{
		UG_PROFILE_FUNCTION();

		//glCreateBuffers(1, &m_rendererID);

		glCreateBuffers(1, &m_rendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);

		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		UG_PROFILE_FUNCTION();


		glDeleteBuffers(1, &m_rendererID);


	}

	void OpenGLVertexBuffer::Bind() const
	{

		UG_PROFILE_FUNCTION();

		glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);


	}

	void OpenGLVertexBuffer::Unbind() const
	{
		UG_PROFILE_FUNCTION();

		glBindBuffer(GL_ARRAY_BUFFER, 0);


	}


	/************************************************************
	*					Index Buffers
	************************************************************/


	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, uint32_t count)
		: m_count(count)
	{
		UG_PROFILE_FUNCTION();

		glCreateBuffers(1, &m_rendererID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);

	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		UG_PROFILE_FUNCTION();

		glDeleteBuffers(1, &m_rendererID);


	}

	void OpenGLIndexBuffer::Bind() const
	{
		UG_PROFILE_FUNCTION();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID);

	}

	void OpenGLIndexBuffer::Unbind() const
	{
		UG_PROFILE_FUNCTION();


		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	}

}