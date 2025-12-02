#include "common.h"

extern uint32_t ParseHexU32(const char *str, const size_t len) {
	if (str == fpl_null || len == 0) {
		return 0; // Invalid arguments
	}
	const char *p = str;
	uint32_t value = 0;
	while (*p && ((size_t)(p - str) < len)) {
		char c = *p;
		int v;
		if (c >= '0' && c <= '9') {
			v = (int)(*p - '0');
		} else if (c >= 'a' && c <= 'f') {
			v = (int)(*p - 'a') + 10;
		} else if (c >= 'A' && c <= 'F') {
			v = (int)(*p - 'A') + 10;
		} else {
			return 0;
		}
		value *= 16;
		value += (uint32_t)v;
		++p;
	}
	uint32_t result = (uint32_t)value;
	return(result);
}

extern bool TryIID16ParseString(const char *str, const size_t len, IID16 *outValue) {
	if (str == fpl_null || len == 0 || outValue == fpl_null) {
		return false; // Invalid arguments
	}
	if (len != IID16_STRING_LENGTH) {
		return false; // String too short or too long
	}
	fplClearStruct(outValue);
	int pos = 0;
	outValue->a = ParseHexU32(str + pos, 8);
	pos += (8 + 1);
	outValue->b = (uint16_t)ParseHexU32(str + pos, 4);
	pos += (4 + 1);
	outValue->c = (uint16_t)ParseHexU32(str + pos, 4);
	pos += (4 + 1);

	int minusoffset = 0;
	for (int x = 0; x < 16; x += 2) {
		if (x == 4) {
			minusoffset = 1; // We don't want the "minus" to be included in the the byte
		}
		int byteindex = x / 2;
		const char *part = str + (pos + x + minusoffset);
		outValue->d[byteindex] = (uint8_t)ParseHexU32(part, 2);
	}

	return true;
}

extern IID16 IID16ParseString(const char *str, const size_t len) {
	IID16 result;
	if (TryIID16ParseString(str, len, &result)) {
		return result;
	}
	return IID16Empty;
}

extern IID16 IID16ParseStringType(const String str) {
	return IID16ParseString(str.str, str.len);
}