/**
 * @file OpenGLTextureCube.h
 * @brief OpenGL implementation of Uge::TextureCube.
 * @ingroup group_platform
 */

#pragma once

#include "Uge/Renderer/TextureCube.h"

#include <glad/glad.h>

namespace Uge
{

	/**
	 * @brief An immutable-storage OpenGL cubemap.
	 * @ingroup group_platform
	 *
	 * Uses direct state access, so no binding is needed to configure the texture.
	 * Faces are filled by rendering into them; see Uge::OpenGLEnvironmentBuilder.
	 */
	class OpenGLTextureCube : public TextureCube
	{
	public:
		/**
		 * @brief Allocates the cubemap's storage.
		 * @param specification Face size, format and mip settings.
		 */
		explicit OpenGLTextureCube(const TextureCubeSpecification& specification);

		/** @brief Deletes the GL texture. */
		virtual ~OpenGLTextureCube();

		virtual const TextureCubeSpecification& GetSpecification() const override { return m_specification; }

		virtual uint32_t GetSize() const override { return m_specification.Size; }
		virtual uint32_t GetMipLevelCount() const override { return m_mipLevels; }
		virtual uint32_t GetRendererID() const override { return m_rendererID; }

		virtual void Bind(uint32_t slot = 0) const override;
		virtual void GenerateMips() override;

	private:
		TextureCubeSpecification m_specification;

		uint32_t m_rendererID = 0;
		uint32_t m_mipLevels = 1;
	};

}
