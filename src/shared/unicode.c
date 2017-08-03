#include "unicode.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct utf8_decode
{
	int            bytes_needed;
	int            bytes_seen;
	uint32_t       codepoint;
	uint32_t       lead;
	bool           strict;
	uint8_t        utf8_high;
	uint8_t        utf8_low;
};

size_t
cesu8_emit(uint32_t codepoint, uint8_t* *p_ptr)
{
	// CESU-8: code points above U+FFFF are encoded as a surrogate pair, like UTF-16.
	// may emit up to 6 bytes.  surrogate code points themselves and code points beyond
	// U+10FFFF are not legal and will not be encoded.

	uint32_t utf16_hi;
	uint32_t utf16_lo;

	if (codepoint <= 0xffff)
		return utf8_emit(codepoint, p_ptr);
	else if (codepoint <= 0x10ffff) {
		utf16_hi = 0xd800 + ((codepoint - 0x010000) >> 10);
		utf16_lo = 0xdc00 + ((codepoint - 0x010000) & 0x3ff);
		*(*p_ptr)++ = (utf16_hi >> 12) + 0xe0;
		*(*p_ptr)++ = 0x80 + (utf16_hi >> 6 & 0x3f);
		*(*p_ptr)++ = 0x80 + (utf16_hi & 0x3f);
		*(*p_ptr)++ = (utf16_lo >> 12) + 0xe0;
		*(*p_ptr)++ = 0x80 + (utf16_lo >> 6 & 0x3f);
		*(*p_ptr)++ = 0x80 + (utf16_lo & 0x3f);
		return 6;
	}
	else {
		// not a legal Unicode code point
		return 0;
	}
}

utf8_decode_t*
utf8_decode_start(bool strict)
{
	utf8_decode_t* cx;

	cx = calloc(1, sizeof(utf8_decode_t));
	cx->strict = strict;
	cx->utf8_low = 0x80;
	cx->utf8_high = 0xbf;
	return cx;
}

utf8_ret_t
utf8_decode_end(utf8_decode_t* cx)
{
	bool is_ok;

	is_ok = cx->bytes_needed == 0;
	free(cx);
	return is_ok
		? UTF8_OK
		: UTF8_ERROR;
}

utf8_ret_t
utf8_decode_next(utf8_decode_t* cx, uint8_t byte, uint32_t *out_codepoint)
{
	// UTF-8 streaming decoder based on WHATWG Encoding specification:
	// https://encoding.spec.whatwg.org/#utf-8-decoder

	// note: this decoder supports both strict UTF-8 as well as CESU-8.  CESU-8 is enabled
	//       by passing false for the `strict` argument of utf8_decode_start().

	if (cx->bytes_needed == 0) {
		if (byte <= 0x7f) {
			if (out_codepoint)
				*out_codepoint = byte;
			return UTF8_CODEPOINT;
		}
		else if (byte >= 0xc2 && byte <= 0xdf) {
			cx->bytes_needed = 1;
			cx->codepoint = byte & 0x1f;
			return UTF8_CONTINUE;
		}
		else if (byte >= 0xe0 && byte <= 0xef) {
			if (byte == 0xe0 && cx->strict)
				cx->utf8_low = 0xa0;
			if (byte == 0xed && cx->strict)
				cx->utf8_high = 0x9f;
			cx->bytes_needed = 2;
			cx->codepoint = byte & 0xf;
			return UTF8_CONTINUE;
		}
		else if (byte >= 0xf0 && byte <= 0xf4) {
			if (byte == 0xf0 && cx->strict)
				cx->utf8_low = 0x90;
			if (byte == 0xf4 && cx->strict)
				cx->utf8_high = 0x8f;
			cx->bytes_needed = 3;
			cx->codepoint = byte & 0x7;
			return UTF8_CONTINUE;
		}
		else {
			return UTF8_ERROR;
		}
	}
	else {
		if (byte >= cx->utf8_low && byte <= cx->utf8_high) {
			cx->utf8_low = 0x80;
			cx->utf8_high = 0xbf;
			cx->codepoint = (cx->codepoint << 6) | (byte & 0x3f);
			if (++cx->bytes_seen < cx->bytes_needed)
				return UTF8_CONTINUE;
			else if (cx->codepoint < 0xd800 || cx->codepoint > 0xdfff) {
				if (out_codepoint)
					*out_codepoint = cx->codepoint;
				cx->codepoint = 0x0000;
				cx->bytes_needed = 0;
				cx->bytes_seen = 0;
				return UTF8_CODEPOINT;
			}
			else if (cx->codepoint >= 0xd800 && cx->codepoint <= 0xdbff && cx->lead == 0x0000) {
				cx->lead = cx->codepoint;
				cx->codepoint = 0x0000;
				cx->bytes_needed = 0;
				cx->bytes_seen = 0;
				return UTF8_CONTINUE;
			}
			else if (cx->codepoint >= 0xdc00 && cx->codepoint <= 0xdfff && cx->lead != 0x0000) {
				if (out_codepoint)
					*out_codepoint = 0x010000 | ((cx->lead - 0xd800) << 10) | (cx->codepoint - 0xdc00);
				cx->lead = 0x0000;
				cx->codepoint = 0x0000;
				cx->bytes_needed = 0;
				cx->bytes_seen = 0;
				return UTF8_CODEPOINT;
			}
			else {
				cx->codepoint = 0x0000;
				cx->bytes_needed = 0;
				cx->bytes_seen = 0;
				return UTF8_ERROR;
			}
		}
		else {
			// this is a dangerous situation.  we've just encountered an invalid continuation
			// byte which may in fact be the initial byte of another character; if we signal
			// an encoding error here, that character would be masked.  to try to avoid that, we signal
			// "retry" so the caller knows to re-send the last byte.
			cx->codepoint = 0x0000;
			cx->bytes_needed = 0;
			cx->bytes_seen = 0;
			cx->utf8_low = 0x80;
			cx->utf8_high = 0xbf;
			return UTF8_RETRY;
		}
	}
}

size_t
utf8_emit(uint32_t codepoint, uint8_t* *p_ptr)
{
	// UTF-8: code points above U+FFFF are encoded directly (i.e. not as surrogate
	// pairs).  may emit up to 4 bytes.  surrogate code points and code points beyond
	// U+10FFFF will not be encoded.

	if (codepoint <= 0x007f) {
		*(*p_ptr)++ = (uint8_t)codepoint;
		return 1;
	}
	else if (codepoint <= 0x07ff) {
		*(*p_ptr)++ = (codepoint >> 6) + 0xc0;
		*(*p_ptr)++ = 0x80 + (codepoint & 0x3f);
		return 2;
	}
	else if (codepoint <= 0xffff) {
		if (codepoint >= 0xd800 && codepoint <= 0xdfff)
			return 0;  // don't encode surrogates
		*(*p_ptr)++ = (codepoint >> 12) + 0xe0;
		*(*p_ptr)++ = 0x80 + (codepoint >> 6 & 0x3f);
		*(*p_ptr)++ = 0x80 + (codepoint & 0x3f);
		return 3;
	}
	else if (codepoint <= 0x10ffff) {
		*(*p_ptr)++ = (codepoint >> 18) + 0xf0;
		*(*p_ptr)++ = 0x80 + (codepoint >> 12 & 0x3f);
		*(*p_ptr)++ = 0x80 + (codepoint >> 6 & 0x3f);
		*(*p_ptr)++ = 0x80 + (codepoint & 0x3f);
		return 4;
	}
	else {
		// not a legal Unicode code point
		return 0;
	}
}
