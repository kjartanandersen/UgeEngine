/**
 * @file Utf8Tests.cpp
 * @brief Tests for Uge::Utf8::DecodeNext.
 */

#include <ugpch.h>

#include "Uge/Core/Utf8.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace
{
	// Decodes the whole text, one call per code point.
	std::vector<uint32_t> DecodeAll(const std::string& text)
	{
		std::vector<uint32_t> codepoints;
		size_t offset = 0;
		while (offset < text.size())
		{
			const size_t before = offset;
			codepoints.push_back(Uge::Utf8::DecodeNext(text, offset));
			EXPECT_GT(offset, before);
			EXPECT_LE(offset, text.size());
		}
		return codepoints;
	}

	constexpr uint32_t k_replacement = Uge::Utf8::ReplacementCharacter;
}

TEST(Utf8Test, DecodesAsciiOneBytePerCodePoint)
{
	EXPECT_EQ(DecodeAll("Az ?"), (std::vector<uint32_t>{ 'A', 'z', ' ', '?' }));
	EXPECT_EQ(DecodeAll(std::string("\0A", 2)), (std::vector<uint32_t>{ 0, 'A' }));
}

TEST(Utf8Test, DecodesMultiByteSequences)
{
	// U+00E9, U+20AC, U+1F600 and the largest code point, U+10FFFF.
	EXPECT_EQ(DecodeAll("a\xC3\xA9" "b"), (std::vector<uint32_t>{ 'a', 0xE9, 'b' }));
	EXPECT_EQ(DecodeAll("\xE2\x82\xAC"), (std::vector<uint32_t>{ 0x20AC }));
	EXPECT_EQ(DecodeAll("\xF0\x9F\x98\x80"), (std::vector<uint32_t>{ 0x1F600 }));
	EXPECT_EQ(DecodeAll("\xF4\x8F\xBF\xBF"), (std::vector<uint32_t>{ 0x10FFFF }));
}

TEST(Utf8Test, MovesTheOffsetPastEachSequence)
{
	const std::string text = "\xC3\xA9\xE2\x82\xAC" "A";
	size_t offset = 0;

	EXPECT_EQ(Uge::Utf8::DecodeNext(text, offset), 0xE9u);
	EXPECT_EQ(offset, 2u);
	EXPECT_EQ(Uge::Utf8::DecodeNext(text, offset), 0x20ACu);
	EXPECT_EQ(offset, 5u);
	EXPECT_EQ(Uge::Utf8::DecodeNext(text, offset), (uint32_t)'A');
	EXPECT_EQ(offset, 6u);
}

TEST(Utf8Test, SkipsABytePerReplacementWhenItCannotStartASequence)
{
	// A continuation byte on its own, and 0xFF, which UTF-8 never uses.
	EXPECT_EQ(DecodeAll("\x80" "A"), (std::vector<uint32_t>{ k_replacement, 'A' }));
	EXPECT_EQ(DecodeAll("\xFF\xFE"), (std::vector<uint32_t>{ k_replacement, k_replacement }));
}

TEST(Utf8Test, ReplacesACutShortSequenceOnceAndKeepsTheByteThatEndedIt)
{
	EXPECT_EQ(DecodeAll("\xE2\x82"), (std::vector<uint32_t>{ k_replacement }));
	EXPECT_EQ(DecodeAll("\xE2\x82" "A"), (std::vector<uint32_t>{ k_replacement, 'A' }));
	EXPECT_EQ(DecodeAll("\xF0\x9F" "\xC3\xA9"), (std::vector<uint32_t>{ k_replacement, 0xE9 }));
}

TEST(Utf8Test, ReplacesOverlongEncodingsSurrogatesAndValuesAboveTheLast)
{
	// '/' in two and three bytes.
	EXPECT_EQ(DecodeAll("\xC0\xAF"), (std::vector<uint32_t>{ k_replacement }));
	EXPECT_EQ(DecodeAll("\xE0\x80\xAF"), (std::vector<uint32_t>{ k_replacement }));

	// U+D800 and U+DFFF, the ends of the surrogate range.
	EXPECT_EQ(DecodeAll("\xED\xA0\x80"), (std::vector<uint32_t>{ k_replacement }));
	EXPECT_EQ(DecodeAll("\xED\xBF\xBF"), (std::vector<uint32_t>{ k_replacement }));

	// 0x110000, one past U+10FFFF.
	EXPECT_EQ(DecodeAll("\xF4\x90\x80\x80"), (std::vector<uint32_t>{ k_replacement }));
}
