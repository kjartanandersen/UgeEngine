/**
 * @file BufferLayoutTests.cpp
 * @brief Tests for the vertex layout description in Uge/Renderer/Buffer.h.
 */

#include <ugpch.h>

#include "Uge/Renderer/Buffer.h"

#include <gtest/gtest.h>

using Uge::BufferElement;
using Uge::BufferLayout;
using Uge::ShaderDataType;

TEST(BufferLayoutTest, EmptyLayoutHasNoStride)
{
	BufferLayout layout;

	EXPECT_EQ(layout.GetStride(), 0u);
	EXPECT_TRUE(layout.GetElements().empty());
}

TEST(BufferLayoutTest, ComputesOffsetsAndStrideInDeclarationOrder)
{
	// The layout Renderer2D uses, minus the parts that vary: if these offsets drift, the
	// attribute pointers no longer match the vertex struct and geometry comes out
	// scrambled rather than failing loudly.
	BufferLayout layout = {
		{ ShaderDataType::Float3, "a_Position" },
		{ ShaderDataType::Float4, "a_Color" },
		{ ShaderDataType::Float2, "a_TexCoord" },
		{ ShaderDataType::Int, "a_EntityID" }
	};

	const std::vector<BufferElement>& elements = layout.GetElements();
	ASSERT_EQ(elements.size(), 4u);

	EXPECT_EQ(elements[0].offset, 0u);
	EXPECT_EQ(elements[1].offset, 12u);
	EXPECT_EQ(elements[2].offset, 28u);
	EXPECT_EQ(elements[3].offset, 36u);

	EXPECT_EQ(layout.GetStride(), 40u);
}

TEST(BufferLayoutTest, StrideIsTheSumOfTheElementSizes)
{
	BufferLayout layout = {
		{ ShaderDataType::Mat4x4, "a_Transform" },
		{ ShaderDataType::Bool, "a_Flag" }
	};

	EXPECT_EQ(layout.GetStride(), 64u + 1u);
}

TEST(BufferLayoutTest, SingleElementLayoutStartsAtZero)
{
	BufferLayout layout = { { ShaderDataType::Float3, "a_Position" } };

	ASSERT_EQ(layout.GetElements().size(), 1u);
	EXPECT_EQ(layout.GetElements()[0].offset, 0u);
	EXPECT_EQ(layout.GetStride(), 12u);
}

TEST(BufferLayoutTest, IsIterable)
{
	BufferLayout layout = {
		{ ShaderDataType::Float3, "a_Position" },
		{ ShaderDataType::Float2, "a_TexCoord" }
	};

	uint32_t total = 0;
	for (const BufferElement& element : layout)
	{
		total += element.size;
	}

	EXPECT_EQ(total, layout.GetStride());
}

TEST(BufferElementTest, RecordsNameTypeAndNormalization)
{
	BufferElement element(ShaderDataType::Float4, "a_Color", true);

	EXPECT_EQ(element.name, "a_Color");
	EXPECT_EQ(element.type, ShaderDataType::Float4);
	EXPECT_EQ(element.size, 16u);
	EXPECT_TRUE(element.normalized);
}

TEST(BufferElementTest, DefaultsToNotNormalized)
{
	BufferElement element(ShaderDataType::Float3, "a_Position");

	EXPECT_FALSE(element.normalized);
}

TEST(BufferElementTest, ComponentCountsMatchTheTypes)
{
	// The OpenGL vertex array passes this straight to glVertexAttribPointer.
	EXPECT_EQ(BufferElement(ShaderDataType::Float, "f").GetComponentCount(), 1u);
	EXPECT_EQ(BufferElement(ShaderDataType::Float2, "f").GetComponentCount(), 2u);
	EXPECT_EQ(BufferElement(ShaderDataType::Float3, "f").GetComponentCount(), 3u);
	EXPECT_EQ(BufferElement(ShaderDataType::Float4, "f").GetComponentCount(), 4u);
	EXPECT_EQ(BufferElement(ShaderDataType::Mat3x3, "f").GetComponentCount(), 9u);
	EXPECT_EQ(BufferElement(ShaderDataType::Mat4x4, "f").GetComponentCount(), 16u);
	EXPECT_EQ(BufferElement(ShaderDataType::Int, "i").GetComponentCount(), 1u);
	EXPECT_EQ(BufferElement(ShaderDataType::Int2, "i").GetComponentCount(), 2u);
	EXPECT_EQ(BufferElement(ShaderDataType::Int3, "i").GetComponentCount(), 3u);
	EXPECT_EQ(BufferElement(ShaderDataType::Int4, "i").GetComponentCount(), 4u);
	EXPECT_EQ(BufferElement(ShaderDataType::Bool, "b").GetComponentCount(), 1u);
}

TEST(BufferElementTest, SizesMatchTheTypes)
{
	EXPECT_EQ(BufferElement(ShaderDataType::Float, "f").size, 4u);
	EXPECT_EQ(BufferElement(ShaderDataType::Float2, "f").size, 8u);
	EXPECT_EQ(BufferElement(ShaderDataType::Float3, "f").size, 12u);
	EXPECT_EQ(BufferElement(ShaderDataType::Float4, "f").size, 16u);
	EXPECT_EQ(BufferElement(ShaderDataType::Mat3x3, "f").size, 36u);
	EXPECT_EQ(BufferElement(ShaderDataType::Mat4x4, "f").size, 64u);
	EXPECT_EQ(BufferElement(ShaderDataType::Int4, "i").size, 16u);

	// The one type whose size is not a multiple of four, and the reason a layout ending
	// in a Bool has an odd stride.
	EXPECT_EQ(BufferElement(ShaderDataType::Bool, "b").size, 1u);
}
