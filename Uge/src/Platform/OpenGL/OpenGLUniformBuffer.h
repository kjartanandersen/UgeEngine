/**
 * @file OpenGLUniformBuffer.h
 * @brief OpenGL implementation of Uge::UniformBuffer.
 * @ingroup group_platform
 */

#pragma once

#include "Uge/Renderer/UniformBuffer.h"

namespace Uge
{

	/**
	 * @brief An OpenGL uniform buffer object bound to an indexed binding point.
	 * @ingroup group_platform
	 */
	class OpenGLUniformBuffer : public UniformBuffer
	{
	public:
		/**
		 * @brief Creates the buffer and binds it to a binding point.
		 * @param size Buffer size in bytes.
		 * @param binding Binding index matching the shader's `layout(binding = N)`.
		 */
		OpenGLUniformBuffer(uint32_t size, uint32_t binding);
		/** @brief Deletes the GL buffer. */
		virtual ~OpenGLUniformBuffer();

		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;
	private:
		uint32_t m_RendererID = 0;
	};


}