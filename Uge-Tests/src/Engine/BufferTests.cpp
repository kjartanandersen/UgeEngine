/**
 * @file BufferTests.cpp
 * @brief Tests for Uge::Buffer and Uge::ScopedBuffer, the raw byte buffers.
 */

#include <ugpch.h>

#include "Uge/Core/Buffer.h"

#include <gtest/gtest.h>

using Uge::Buffer;
using Uge::ScopedBuffer;

TEST(BufferTest, DefaultConstructedBufferIsEmpty)
{
	Buffer buffer;

	EXPECT_EQ(buffer.Data, nullptr);
	EXPECT_EQ(buffer.Size, 0u);
	EXPECT_FALSE(buffer);
}

TEST(BufferTest, AllocateGivesTheRequestedSize)
{
	Buffer buffer(64);

	ASSERT_NE(buffer.Data, nullptr);
	EXPECT_EQ(buffer.Size, 64u);
	EXPECT_TRUE(buffer);

	buffer.Release();
}

TEST(BufferTest, ReleaseResetsTheBufferAndIsSafeToRepeat)
{
	Buffer buffer(32);
	buffer.Release();

	EXPECT_EQ(buffer.Data, nullptr);
	EXPECT_EQ(buffer.Size, 0u);

	// Release() is the manual half of a manually managed buffer, so calling it twice —
	// which happens whenever an error path and a success path both clean up — must not
	// double-free.
	buffer.Release();
	EXPECT_EQ(buffer.Data, nullptr);
}

TEST(BufferTest, AllocateReleasesAnyPreviousAllocation)
{
	Buffer buffer(16);
	buffer.Allocate(128);

	ASSERT_NE(buffer.Data, nullptr);
	EXPECT_EQ(buffer.Size, 128u);

	buffer.Release();
}

TEST(BufferTest, ViewConstructorDoesNotCopyTheBytes)
{
	uint8_t bytes[] = { 1, 2, 3, 4 };
	Buffer view(bytes, sizeof(bytes));

	EXPECT_EQ(view.Data, bytes);
	EXPECT_EQ(view.Size, sizeof(bytes));

	// A view aliases its source, so writing through one is visible in the other.
	view.Data[0] = 42;
	EXPECT_EQ(bytes[0], 42);
}

TEST(BufferTest, CopyDuplicatesTheBytesIntoNewMemory)
{
	uint8_t bytes[] = { 10, 20, 30, 40 };
	Buffer source(bytes, sizeof(bytes));

	Buffer copy = Buffer::Copy(source);

	ASSERT_NE(copy.Data, nullptr);
	ASSERT_NE(copy.Data, source.Data);
	EXPECT_EQ(copy.Size, source.Size);
	EXPECT_EQ(std::memcmp(copy.Data, bytes, sizeof(bytes)), 0);

	// The whole point of Copy over the copy constructor: the two no longer alias.
	bytes[0] = 99;
	EXPECT_EQ(copy.Data[0], 10);

	copy.Release();
}

TEST(BufferTest, CopyConstructorSharesTheSameMemory)
{
	Buffer owner(8);
	owner.Data[0] = 7;

	Buffer alias = owner;

	EXPECT_EQ(alias.Data, owner.Data);
	EXPECT_EQ(alias.Data[0], 7);

	// Only one of them may be released; that asymmetry is why ScopedBuffer exists.
	owner.Release();
}

TEST(BufferTest, AsReinterpretsTheBytes)
{
	Buffer buffer(4 * sizeof(uint32_t));

	uint32_t* words = buffer.As<uint32_t>();
	ASSERT_NE(words, nullptr);

	words[0] = 0xDEADBEEF;
	words[3] = 0x12345678;

	EXPECT_EQ(buffer.As<uint32_t>()[0], 0xDEADBEEFu);
	EXPECT_EQ(buffer.As<uint32_t>()[3], 0x12345678u);

	buffer.Release();
}

TEST(BufferTest, ScopedBufferExposesTheOwnedAllocation)
{
	ScopedBuffer buffer(48);

	ASSERT_NE(buffer.Data(), nullptr);
	EXPECT_EQ(buffer.Size(), 48u);
	EXPECT_TRUE(buffer);
}

TEST(BufferTest, ScopedBufferAdoptsAnExistingBuffer)
{
	Buffer raw(16);
	raw.As<uint8_t>()[0] = 5;

	ScopedBuffer owner(raw);

	EXPECT_EQ(owner.Data(), raw.Data);
	EXPECT_EQ(owner.Size(), 16u);
	EXPECT_EQ(owner.As<uint8_t>()[0], 5);

	// owner releases the allocation on the way out; raw must not be released as well.
}

TEST(BufferTest, ScopedBufferOverAnEmptyBufferIsFalse)
{
	ScopedBuffer empty{ Buffer() };

	EXPECT_FALSE(empty);
	EXPECT_EQ(empty.Data(), nullptr);
	EXPECT_EQ(empty.Size(), 0u);
}
