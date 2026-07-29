/**
 * @file UniformBuffer.h
 * @brief GPU uniform buffer bound to a shader binding point.
 * @ingroup group_renderer
 */

#pragma once

#include "Uge/Core/Core.h"

namespace Uge
{

	/**
	 * @brief A uniform buffer object shared by every shader that declares its binding.
	 * @ingroup group_renderer
	 *
	 * The efficient way to supply per-frame data such as the camera matrices: upload once
	 * per frame and every shader using the binding point sees it, instead of setting the
	 * same uniform on each shader by name.
	 *
	 * @code
	 * // matches: layout(std140, binding = 0) uniform Camera { mat4 u_ViewProjection; };
	 * s_data.CameraUniformBuffer = UniformBuffer::Create(sizeof(CameraData), 0);
	 * s_data.CameraUniformBuffer->SetData(&cameraData, sizeof(CameraData));
	 * @endcode
	 *
	 * @warning The CPU-side struct must match the shader block's `std140` layout, which
	 * pads `vec3` to 16 bytes. A mismatch silently produces wrong values.
	 */
	class UniformBuffer
	{
	public:
		/** @brief Releases the GPU buffer. */
		virtual ~UniformBuffer() {}
		/**
		 * @brief Uploads data into the buffer.
		 * @param data Source bytes to copy.
		 * @param size Number of bytes to write.
		 * @param offset Byte offset within the buffer to write at.
		 */
		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;

		/**
		 * @brief Creates a uniform buffer for the active graphics API.
		 * @param size Buffer size in bytes.
		 * @param binding Binding point index, matching the shader's `layout(binding = N)`.
		 * @return The backend's uniform buffer implementation.
		 */
		static Ref<UniformBuffer> Create(uint32_t size, uint32_t binding);
	};


}