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
	 * @brief A texture that needs no GL context, for registering texture assets in tests.
	 */
	class MockTexture2D : public Uge::Texture2D
	{
	public:
		/**
		 * @brief A mock of a given size.
		 * @param width Width.
		 * @param height Height.
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

		/** @brief The specification. @return The specification. */
		const Uge::TextureSpecification& GetSpecification() const override { return m_specification; }

		/** @brief The width. @return Width in pixels. */
		uint32_t GetWidth() const override { return m_specification.Width; }
		/** @brief The height. @return Height in pixels. */
		uint32_t GetHeight() const override { return m_specification.Height; }
		/** @brief A fixed renderer ID. @return `1`. */
		uint32_t GetRendererID() const override { return 1; }

		/** @brief Always loaded. @return `true`. */
		bool IsLoaded() const override { return true; }

		/** @brief Stores the tiling factor. @param tilingFactor Repeat count. */
		void SetTilingFactor(float tilingFactor) override { m_tilingFactor = tilingFactor; }

		/**
		 * @brief Whether @p other is this object.
		 * @param other The other texture.
		 * @return `true` if it is.
		 */
		bool operator==(const Uge::Texture& other) const override { return this == &other; }

	private:
		Uge::TextureSpecification m_specification;
	};

}
