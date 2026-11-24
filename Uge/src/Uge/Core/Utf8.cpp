#include <ugpch.h>

#include "Utf8.h"

namespace Uge::Utf8
{

	uint32_t DecodeNext(std::string_view text, size_t& offset)
	{
		const uint8_t lead = static_cast<uint8_t>(text[offset]);
		if (lead < 0x80)
		{
			offset++;
			return lead;
		}

		// The lead byte gives the sequence length and the code point's first bits. Each
		// length has a smallest value, and anything below it is an overlong encoding.
		size_t length;
		uint32_t codepoint;
		uint32_t smallest;
		if ((lead & 0xE0) == 0xC0)
		{
			length = 2;
			codepoint = lead & 0x1F;
			smallest = 0x80;
		}
		else if ((lead & 0xF0) == 0xE0)
		{
			length = 3;
			codepoint = lead & 0x0F;
			smallest = 0x800;
		}
		else if ((lead & 0xF8) == 0xF0)
		{
			length = 4;
			codepoint = lead & 0x07;
			smallest = 0x10000;
		}
		else
		{
			// A continuation byte, or 0xF8 to 0xFF, which UTF-8 never uses.
			offset++;
			return ReplacementCharacter;
		}

		for (size_t i = 1; i < length; i++)
		{
			if (offset + i >= text.size())
			{
				offset = text.size();
				return ReplacementCharacter;
			}

			const uint8_t continuation = static_cast<uint8_t>(text[offset + i]);
			if ((continuation & 0xC0) != 0x80)
			{
				// The byte that ended the sequence is decoded on the next call.
				offset += i;
				return ReplacementCharacter;
			}

			codepoint = (codepoint << 6) | (continuation & 0x3F);
		}

		offset += length;

		if (codepoint < smallest || codepoint > 0x10FFFF || (codepoint >= 0xD800 && codepoint <= 0xDFFF))
		{
			return ReplacementCharacter;
		}

		return codepoint;
	}

}
