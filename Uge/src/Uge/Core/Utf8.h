/**
 * @file Utf8.h
 * @brief Decoding UTF-8 text into code points.
 * @ingroup group_core
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

/**
 * @brief Decodes UTF-8 one code point at a time.
 * @ingroup group_core
 */
namespace Uge::Utf8
{

	/**
	 * @brief U+FFFD, which DecodeNext() returns for bytes that aren't valid UTF-8.
	 * @ingroup group_core
	 */
	constexpr uint32_t ReplacementCharacter = 0xFFFD;

	/**
	 * @brief Decodes the code point that starts at `offset`, and moves `offset` past it.
	 * @param text UTF-8 text.
	 * @param offset Byte index into `text`; less than its size. Moved forward by at least one.
	 * @return The code point, or Uge::Utf8::ReplacementCharacter for an invalid sequence.
	 * @ingroup group_core
	 *
	 * A byte that can't start a sequence is skipped on its own. A sequence cut short by the
	 * end of the text or by a byte that isn't a continuation byte is skipped up to that
	 * byte. An overlong encoding, a surrogate (U+D800 to U+DFFF) and a value above U+10FFFF
	 * are skipped whole. Each gives one replacement character.
	 */
	uint32_t DecodeNext(std::string_view text, size_t& offset);

}
