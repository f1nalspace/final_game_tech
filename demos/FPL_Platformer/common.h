#ifndef COMMON_H
#define COMMON_H

#include <final_platform_layer.h>

#define constant static const

#define IsBitMaskSet(value, mask) (((value) & (mask)) == (mask))

typedef struct String {
	const char *str;
	size_t len;
} String;

fpl_inline String StringInit(const char *str, const size_t len) {
	String result = fplStructInit(String, str, len);
	return result;
}

#define StringEmpty fplStructInit(String, 0)

#define IID16_STRING_LENGTH 36

typedef struct IID16 {
	uint32_t a;
	uint16_t b;
	uint16_t c;
	uint8_t d[8];
} IID16;

#define IID16Empty fplStructInit(IID16, 0)

fpl_inline IID16 IID16Init(const uint32_t a, const uint16_t b, const uint16_t c, const uint8_t d[8]) {
	IID16 result = fplZeroInit;
	result.a = a;
	result.b = b;
	result.c = c;
	for (uint8_t i = 0; i < 8; ++i)
		result.d[i] = d[i];
	return result;
}

// Tries to parse a IID/GUID string such as "5bb13300-ac70-11f0-842c-7f4fc27d982e" into a IID16 structure
extern bool TryIID16ParseString(const char *str, const size_t len, IID16 *outValue);

// Parse a IID/GUID string such as "5bb13300-ac70-11f0-842c-7f4fc27d982e" into a IID16 structure
extern IID16 IID16ParseString(const char *str, const size_t len);
extern IID16 IID16ParseStringType(const String str);

#endif // COMMON_H
