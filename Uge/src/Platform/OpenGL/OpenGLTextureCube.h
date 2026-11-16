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
	 * @brief An OpenGL cubemap with immutable storage.
	 * @ingroup group_platform
	 */
	class OpenGLTextureCube : public TextureCube
	{
	public:
		/**
		 * @brief Allocates the cubemap.
		 * @param specification Size, format and mipmaps.
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
