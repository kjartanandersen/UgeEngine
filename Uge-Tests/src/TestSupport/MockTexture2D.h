/**
 * @file MockTexture2D.h
 * @brief A Uge::Texture2D with no GPU behind it.
 */

#pragma once

#include "Uge/Renderer/Texture.h"

#include <gmock/gmock.h>

namespace UgeTests
{

	/**
	 * @brief A texture asset that never touches OpenGL.
	 *
	 * Uge::Texture2D::Create returns the OpenGL implementation, which needs a live context,
	 * so a test cannot make a real texture. Everything above the backend only asks a texture
	 * for its size, its renderer ID and its asset type, all of which a mock can answer —
	 * which is enough to register one as an asset and drive the code paths that branch on
	 * "is there a valid texture here".
	 */
	class MockTexture2D : public Uge::Texture2D
	{
	public:
		/**
		 * @brief Constructs a mock reporting the given dimensions.
		 * @param width Width to report.
		 * @param height Height to report.
		 */
		MockTexture2D(uint32_t width = 4, uint32_t height = 4)
		{
			m_specification.Width = width;
			m_specification.Height = height;

			m_width = width;
			m_height = height;
		}

		MOCK_METHOD(void, SetData, (Uge::Buffer), (override));
		MOCK_METHOD(void, Bind, (uint32_t), (const, override));
		MOCK_METHOD(void, UnBind, (uint32_t), (const, override));

		/** @brief The specification this mock reports. @return Const reference to it. */
		const Uge::TextureSpecification& GetSpecification() const override { return m_specification; }

		/** @brief Reported width. @return Width in pixels. */
		uint32_t GetWidth() const override { return m_specification.Width; }
		/** @brief Reported height. @return Height in pixels. */
		uint32_t GetHeight() const override { return m_specification.Height; }
		/** @brief A stand-in native handle. @return A fixed non-zero id. */
		uint32_t GetRendererID() const override { return 1; }

		/** @brief Always loaded. @return `true`. */
		bool IsLoaded() const override { return true; }

		/** @brief Records the tiling factor. @param tilingFactor Repeat count. */
		void SetTilingFactor(float tilingFactor) override { m_tilingFactor = tilingFactor; }

		/**
		 * @brief Compares by identity.
		 * @param other Texture to compare against.
		 * @return `true` if it is this same object.
		 */
		bool operator==(const Uge::Texture& other) const override { return this == &other; }

	private:
		Uge::TextureSpecification m_specification;
	};

}
