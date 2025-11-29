#ifndef COMMON_H
#define COMMON_H

#include <final_platform_layer.h>

#include "json/json.h"

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

extern json_object_element_t *JSONObjectFindElementByName(json_object_t *obj, const char *name);
extern json_value_t *JSONObjectFindValueByName(json_object_t *obj, const char *name);

fpl_inline json_object_t *JSONValueAsObject(json_value_t *value) {
	if (value == fpl_null || value->type != json_type_object || value->payload == fpl_null) {
		return fpl_null;
	}
	json_object_t *result = (json_object_t *)value->payload;
	return result;
}

fpl_inline json_array_t *JSONValueAsArray(json_value_t *value) {
	if (value == fpl_null || value->type != json_type_array || value->payload == fpl_null) {
		return fpl_null;
	}
	json_array_t *result = (json_array_t *)value->payload;
	return result;
}

fpl_inline String JSONValueAsString(json_value_t *value) {
	if (value == fpl_null || value->type != json_type_string || value->payload == fpl_null) {
		return StringEmpty;
	}
	json_string_t *str = (json_string_t *)value->payload;
	if (str->string_size == 0) {
		return StringEmpty;
	}
	String result = StringInit(str->string, str->string_size);
	return result;
}

fpl_inline String JSONElementAsString(json_object_element_t *element) {
	if (element == fpl_null || element->value == fpl_null) {
		return StringEmpty;
	}
	json_value_t *value = element->value;
	return JSONValueAsString(value);
}

fpl_inline int32_t JSONValueAsS32(json_value_t *value, const uint32_t defaultValue) {
	if (value == fpl_null || value->type != json_type_number || value->payload == fpl_null) {
		return defaultValue;
	}
	json_number_t *number = (json_number_t *)value->payload;
	if (number->number == fpl_null || number->number_size == 0) {
		return defaultValue;
	}
	int32_t result = fplStringToS32Len(number->number, number->number_size);
	return result;
}

#endif // COMMON_H
