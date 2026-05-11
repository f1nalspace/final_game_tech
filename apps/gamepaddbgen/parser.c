/*
-------------------------------------------------------------------------------
Name:
	GameControllerDB Parser

Description:
	Parses one line of an SDL gamecontrollerdb.txt file into a fplGamepadMapping
	(see final_platform_layer.h for the public type) and provides the encoder /
	decoder used by the offline blob generator.

	Line format (SDL 2.0.16): <guid>,<device-name>,<key>:<value>,...,platform:<name>,
	Value forms: bN, aN, +aN, -aN, aN~, hN.M (mask 1=up,2=right,4=down,8=left).
	Device name and the runtime flags isConnected/isActive are ignored.

	A separate uint16_t-packed form (fplGamepadInputBindingEncoded) is used only
	for the on-disk blob, with explicit shift/mask helpers (no C bitfields, so it
	stays valid in both C99 and C++).

	Encoded binding bit layout (fplGamepadInputBindingEncoded, 16 bits):
		bits  0..1  type          (fplGamepadInputType, 4 values)
		bits  2..7  index         (0..63)
		bits  8..11 hatMask       (0..15, only 1/2/4/8 used in practice)
		bits 12..13 axisSign      (fplGamepadAxisSign, 3 values)
		bit  14     axisInverted  (0/1)
		bit  15     reserved      (must be 0)

	Blob layout (no embedded header — entry count is supplied separately as a
	#define from the generator):
		per entry (57 bytes), entries sorted ascending by raw GUID bytes:
			[16 B]            GUID raw (hex-decoded big-endian)
			[ 1 B]            platform (fplGamepadPlatform)
			[20 × u16 LE]     encoded bindings (buttons[14] then axes[6])

Requirements:
	- C99 Compiler (also valid C++)
	- Final Platform Layer
-------------------------------------------------------------------------------
*/

#include <final_platform_layer.h>

// Packed 16-bit binding used only for on-disk storage.
typedef uint16_t fplGamepadInputBindingEncoded;

#define FPL_BINDING_TYPE_BITS         2
#define FPL_BINDING_INDEX_BITS        6
#define FPL_BINDING_HATMASK_BITS      4
#define FPL_BINDING_AXISSIGN_BITS     2
#define FPL_BINDING_AXISINVERTED_BITS 1

#define FPL_BINDING_TYPE_SHIFT         0
#define FPL_BINDING_INDEX_SHIFT        (FPL_BINDING_TYPE_SHIFT + FPL_BINDING_TYPE_BITS)
#define FPL_BINDING_HATMASK_SHIFT      (FPL_BINDING_INDEX_SHIFT + FPL_BINDING_INDEX_BITS)
#define FPL_BINDING_AXISSIGN_SHIFT     (FPL_BINDING_HATMASK_SHIFT + FPL_BINDING_HATMASK_BITS)
#define FPL_BINDING_AXISINVERTED_SHIFT (FPL_BINDING_AXISSIGN_SHIFT + FPL_BINDING_AXISSIGN_BITS)

#define FPL_BINDING_TYPE_MAX           ((1u << FPL_BINDING_TYPE_BITS) - 1u)
#define FPL_BINDING_INDEX_MAX          ((1u << FPL_BINDING_INDEX_BITS) - 1u)
#define FPL_BINDING_HATMASK_MAX        ((1u << FPL_BINDING_HATMASK_BITS) - 1u)
#define FPL_BINDING_AXISSIGN_MAX       ((1u << FPL_BINDING_AXISSIGN_BITS) - 1u)

// Number of analog axis slots stored in a fplGamepadMapping.
#define FPL_GAMEPAD_AXIS_COUNT     ((int)fplGamepadAxisType_Count)
// Total binding slots in a mapping (buttons + axes).
#define FPL_GAMEPAD_BINDING_COUNT  (FPL_GAMEPAD_BUTTON_COUNT + FPL_GAMEPAD_AXIS_COUNT)
// On-disk per-entry size: 16 + 1 + 20*2 = 57 bytes.
#define FPL_GAMEPAD_BLOB_ENTRY_SIZE (FPL_GAMEPAD_GUID_SIZE + 1 + FPL_GAMEPAD_BINDING_COUNT * 2)

// Returns true if every field of b fits inside its packed bit width (lossless encode possible).
static fpl_force_inline bool fplBindingFitsEncoded(const fplGamepadInputBinding *b) {
	return
		((uint32_t)b->type     <= FPL_BINDING_TYPE_MAX) &&
		(b->index              <= FPL_BINDING_INDEX_MAX) &&
		(b->hatMask            <= FPL_BINDING_HATMASK_MAX) &&
		((uint32_t)b->axisSign <= FPL_BINDING_AXISSIGN_MAX);
}

// Packs a binding into its 16-bit encoded form (caller must verify it fits).
static fpl_force_inline fplGamepadInputBindingEncoded fplEncodeBinding(const fplGamepadInputBinding *b) {
	uint32_t v =
		(((uint32_t)b->type     & FPL_BINDING_TYPE_MAX)     << FPL_BINDING_TYPE_SHIFT) |
		((b->index              & FPL_BINDING_INDEX_MAX)    << FPL_BINDING_INDEX_SHIFT) |
		((b->hatMask            & FPL_BINDING_HATMASK_MAX)  << FPL_BINDING_HATMASK_SHIFT) |
		(((uint32_t)b->axisSign & FPL_BINDING_AXISSIGN_MAX) << FPL_BINDING_AXISSIGN_SHIFT) |
		((b->axisInverted ? 1u : 0u) << FPL_BINDING_AXISINVERTED_SHIFT);
	return (fplGamepadInputBindingEncoded)v;
}

// Unpacks a 16-bit encoded binding back into the in-memory struct.
static fpl_force_inline void fplDecodeBinding(fplGamepadInputBindingEncoded enc, fplGamepadInputBinding *out) {
	out->type         = (fplGamepadInputType)((enc >> FPL_BINDING_TYPE_SHIFT) & FPL_BINDING_TYPE_MAX);
	out->index        = (uint32_t)((enc >> FPL_BINDING_INDEX_SHIFT) & FPL_BINDING_INDEX_MAX);
	out->hatMask      = (uint32_t)((enc >> FPL_BINDING_HATMASK_SHIFT) & FPL_BINDING_HATMASK_MAX);
	out->axisSign     = (fplGamepadAxisSign)((enc >> FPL_BINDING_AXISSIGN_SHIFT) & FPL_BINDING_AXISSIGN_MAX);
	out->axisInverted = ((enc >> FPL_BINDING_AXISINVERTED_SHIFT) & 1u) != 0u;
}

// Returns offset of first occurrence of c in [str, str+len), or len if not found.
static size_t fpl__FindChar(const char *str, size_t len, char c) {
	for (size_t i = 0; i < len; ++i) {
		if (str[i] == c) {
			return i;
		}
	}
	return len;
}

// Returns the length of a leading run of decimal digits.
static size_t fpl__SpanDigits(const char *str, size_t len) {
	size_t i = 0;
	while (i < len && str[i] >= '0' && str[i] <= '9') {
		++i;
	}
	return i;
}

// Decodes one hex character; returns -1 if invalid.
static int fpl__HexDigit(char c) {
	if (c >= '0' && c <= '9') {
		return c - '0';
	}
	if (c >= 'a' && c <= 'f') {
		return c - 'a' + 10;
	}
	if (c >= 'A' && c <= 'F') {
		return c - 'A' + 10;
	}
	return -1;
}

// Decodes 32 hex chars at hex into 16 raw bytes. Returns false on malformed input.
static bool fpl__DecodeGuid(const char *hex, size_t hexLen, uint8_t outBytes[FPL_GAMEPAD_GUID_SIZE]) {
	if (hexLen != FPL_GAMEPAD_GUID_SIZE * 2) {
		return false;
	}
	for (size_t i = 0; i < FPL_GAMEPAD_GUID_SIZE; ++i) {
		int hi = fpl__HexDigit(hex[i * 2]);
		int lo = fpl__HexDigit(hex[i * 2 + 1]);
		if (hi < 0 || lo < 0) {
			return false;
		}
		outBytes[i] = (uint8_t)((hi << 4) | lo);
	}
	return true;
}

static bool fpl__TryMapButtonKey(const char *key, size_t keyLen, fplGamepadButtonType *outType) {
	struct { const char *name; fplGamepadButtonType type; } table[] = {
		{ "a",             fplGamepadButtonType_ActionA },
		{ "b",             fplGamepadButtonType_ActionB },
		{ "x",             fplGamepadButtonType_ActionX },
		{ "y",             fplGamepadButtonType_ActionY },
		{ "start",         fplGamepadButtonType_Start },
		{ "back",          fplGamepadButtonType_Back },
		{ "leftstick",     fplGamepadButtonType_LeftThumb },
		{ "rightstick",    fplGamepadButtonType_RightThumb },
		{ "leftshoulder",  fplGamepadButtonType_LeftShoulder },
		{ "rightshoulder", fplGamepadButtonType_RightShoulder },
		{ "dpup",          fplGamepadButtonType_DPadUp },
		{ "dpdown",        fplGamepadButtonType_DPadDown },
		{ "dpleft",        fplGamepadButtonType_DPadLeft },
		{ "dpright",       fplGamepadButtonType_DPadRight },
	};
	for (size_t i = 0; i < fplArrayCount(table); ++i) {
		if (fplIsStringEqualLen(key, keyLen, table[i].name, fplGetStringLength(table[i].name))) {
			*outType = table[i].type;
			return true;
		}
	}
	return false;
}

static bool fpl__TryMapAxisKey(const char *key, size_t keyLen, fplGamepadAxisType *outType) {
	struct { const char *name; fplGamepadAxisType type; } table[] = {
		{ "leftx",        fplGamepadAxisType_LeftX },
		{ "lefty",        fplGamepadAxisType_LeftY },
		{ "rightx",       fplGamepadAxisType_RightX },
		{ "righty",       fplGamepadAxisType_RightY },
		{ "lefttrigger",  fplGamepadAxisType_LeftTrigger },
		{ "righttrigger", fplGamepadAxisType_RightTrigger },
	};
	for (size_t i = 0; i < fplArrayCount(table); ++i) {
		if (fplIsStringEqualLen(key, keyLen, table[i].name, fplGetStringLength(table[i].name))) {
			*outType = table[i].type;
			return true;
		}
	}
	return false;
}

static fplGamepadPlatform fpl__ParsePlatform(const char *val, size_t valLen) {
	struct { const char *name; fplGamepadPlatform platform; } table[] = {
		{ "Windows",  fplGamepadPlatform_Windows },
		{ "Linux",    fplGamepadPlatform_Linux },
		{ "Mac OS X", fplGamepadPlatform_MacOS },
		{ "Android",  fplGamepadPlatform_Android },
		{ "iOS",      fplGamepadPlatform_iOS },
	};
	for (size_t i = 0; i < fplArrayCount(table); ++i) {
		if (fplIsStringEqualLen(val, valLen, table[i].name, fplGetStringLength(table[i].name))) {
			return table[i].platform;
		}
	}
	return fplGamepadPlatform_Unknown;
}

// Parses a single SDL value descriptor like "b3", "+a1", "-a4", "a2~", "h0.4".
static bool fpl__ParseBinding(const char *val, size_t valLen, fplGamepadInputBinding *outBinding) {
	if (valLen == 0) {
		return false;
	}

	fplGamepadInputBinding b = fplZeroInit;

	size_t pos = 0;
	if (val[pos] == '+') {
		b.axisSign = fplGamepadAxisSign_Positive;
		++pos;
	} else if (val[pos] == '-') {
		b.axisSign = fplGamepadAxisSign_Negative;
		++pos;
	}

	if (pos >= valLen) {
		return false;
	}
	char typeChar = val[pos++];

	if (typeChar == 'b') {
		b.type = fplGamepadInputType_Button;
		size_t digits = fpl__SpanDigits(val + pos, valLen - pos);
		if (digits == 0) {
			return false;
		}
		int32_t parsed;
		if (!fplTryStringToS32Len(val + pos, digits, &parsed) || parsed < 0) {
			return false;
		}
		b.index = (uint32_t)parsed;
	} else if (typeChar == 'a') {
		b.type = fplGamepadInputType_Axis;
		size_t digits = fpl__SpanDigits(val + pos, valLen - pos);
		if (digits == 0) {
			return false;
		}
		int32_t parsed;
		if (!fplTryStringToS32Len(val + pos, digits, &parsed) || parsed < 0) {
			return false;
		}
		b.index = (uint32_t)parsed;
		if (pos + digits < valLen && val[pos + digits] == '~') {
			b.axisInverted = true;
		}
	} else if (typeChar == 'h') {
		b.type = fplGamepadInputType_Hat;
		size_t rem = valLen - pos;
		size_t dot = fpl__FindChar(val + pos, rem, '.');
		if (dot == 0 || dot >= rem) {
			return false;
		}
		int32_t parsedIndex;
		if (!fplTryStringToS32Len(val + pos, dot, &parsedIndex) || parsedIndex < 0) {
			return false;
		}
		b.index = (uint32_t)parsedIndex;
		size_t maskStart = pos + dot + 1;
		if (maskStart >= valLen) {
			return false;
		}
		int32_t parsedMask;
		if (!fplTryStringToS32Len(val + maskStart, valLen - maskStart, &parsedMask) || parsedMask < 0) {
			return false;
		}
		b.hatMask = (uint32_t)parsedMask;
	} else {
		return false;
	}

	*outBinding = b;
	return true;
}

// Parses one line of a gamecontrollerdb.txt file into outMapping. Returns true if the line yielded a usable mapping. Comment, empty, and malformed lines return false.
bool fplParseGameControllerMappingLine(const char *line, fplGamepadMapping *outMapping) {
	if (line == fpl_null || outMapping == fpl_null) {
		return false;
	}

	fplGamepadMapping zero = fplZeroInit;
	*outMapping = zero;

	size_t lineLen = fplGetStringLength(line);
	while (lineLen > 0) {
		char c = line[lineLen - 1];
		if (c == '\n' || c == '\r' || c == ' ' || c == '\t') {
			--lineLen;
		} else {
			break;
		}
	}
	if (lineLen == 0) {
		return false;
	}
	if (line[0] == '#') {
		return false;
	}

	// GUID (32 hex chars decoded into 16 raw bytes).
	size_t guidEnd = fpl__FindChar(line, lineLen, ',');
	if (guidEnd == 0 || guidEnd >= lineLen) {
		return false;
	}
	if (!fpl__DecodeGuid(line, guidEnd, outMapping->guid)) {
		return false;
	}
	size_t pos = guidEnd + 1;

	// Skip device name.
	size_t remaining = lineLen - pos;
	size_t nameEnd = fpl__FindChar(line + pos, remaining, ',');
	if (nameEnd >= remaining) {
		return false;
	}
	pos += nameEnd + 1;

	// Key:value pairs.
	while (pos < lineLen) {
		remaining = lineLen - pos;
		size_t pairEnd = fpl__FindChar(line + pos, remaining, ',');
		size_t pairLen = pairEnd;

		if (pairLen > 0) {
			const char *pair = line + pos;
			size_t colon = fpl__FindChar(pair, pairLen, ':');
			if (colon > 0 && colon < pairLen) {
				const char *key = pair;
				size_t keyLen = colon;
				const char *value = pair + colon + 1;
				size_t valueLen = pairLen - colon - 1;

				if (fplIsStringEqualLen(key, keyLen, "platform", 8)) {
					outMapping->platform = fpl__ParsePlatform(value, valueLen);
				} else {
					fplGamepadButtonType btnType;
					fplGamepadAxisType axType;
					fplGamepadInputBinding binding;
					if (fpl__TryMapButtonKey(key, keyLen, &btnType)) {
						if (fpl__ParseBinding(value, valueLen, &binding)) {
							outMapping->buttons[btnType] = binding;
						}
					} else if (fpl__TryMapAxisKey(key, keyLen, &axType)) {
						if (fpl__ParseBinding(value, valueLen, &binding)) {
							outMapping->axes[axType] = binding;
						}
					}
					// Unknown keys (e.g. "guide") are silently ignored.
				}
			}
		}

		if (pairEnd >= remaining) {
			break;
		}
		pos += pairEnd + 1;
	}

	return true;
}

// Little-endian helpers (kept explicit to stay portable across host endianness).
static fpl_force_inline uint16_t fpl__ReadU16LE(const uint8_t *p) {
	return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}
static fpl_force_inline uint32_t fpl__ReadU32LE(const uint8_t *p) {
	return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
static fpl_force_inline void fpl__WriteU16LE(uint8_t *p, uint16_t v) {
	p[0] = (uint8_t)(v & 0xFF);
	p[1] = (uint8_t)((v >> 8) & 0xFF);
}
static fpl_force_inline void fpl__WriteU32LE(uint8_t *p, uint32_t v) {
	p[0] = (uint8_t)(v & 0xFF);
	p[1] = (uint8_t)((v >> 8) & 0xFF);
	p[2] = (uint8_t)((v >> 16) & 0xFF);
	p[3] = (uint8_t)((v >> 24) & 0xFF);
}

// Serializes one mapping into the 57-byte fixed-size entry slot. Returns false if any binding exceeds the packed bit ranges (entry not written).
bool fplEncodeGamepadMappingEntry(const fplGamepadMapping *mapping, uint8_t out[FPL_GAMEPAD_BLOB_ENTRY_SIZE]) {
	for (size_t i = 0; i < FPL_GAMEPAD_BUTTON_COUNT; ++i) {
		if (!fplBindingFitsEncoded(&mapping->buttons[i])) {
			return false;
		}
	}
	for (size_t i = 0; i < FPL_GAMEPAD_AXIS_COUNT; ++i) {
		if (!fplBindingFitsEncoded(&mapping->axes[i])) {
			return false;
		}
	}
	for (size_t i = 0; i < FPL_GAMEPAD_GUID_SIZE; ++i) {
		out[i] = mapping->guid[i];
	}
	out[FPL_GAMEPAD_GUID_SIZE] = (uint8_t)mapping->platform;
	uint8_t *bp = out + FPL_GAMEPAD_GUID_SIZE + 1;
	for (size_t i = 0; i < FPL_GAMEPAD_BUTTON_COUNT; ++i) {
		fpl__WriteU16LE(bp, fplEncodeBinding(&mapping->buttons[i]));
		bp += 2;
	}
	for (size_t i = 0; i < FPL_GAMEPAD_AXIS_COUNT; ++i) {
		fpl__WriteU16LE(bp, fplEncodeBinding(&mapping->axes[i]));
		bp += 2;
	}
	return true;
}

// Deserializes one fixed-size entry into a mapping struct.
void fplDecodeGamepadMappingEntry(const uint8_t in[FPL_GAMEPAD_BLOB_ENTRY_SIZE], fplGamepadMapping *outMapping) {
	for (size_t i = 0; i < FPL_GAMEPAD_GUID_SIZE; ++i) {
		outMapping->guid[i] = in[i];
	}
	outMapping->platform = (fplGamepadPlatform)in[FPL_GAMEPAD_GUID_SIZE];
	const uint8_t *bp = in + FPL_GAMEPAD_GUID_SIZE + 1;
	for (size_t i = 0; i < FPL_GAMEPAD_BUTTON_COUNT; ++i) {
		fplDecodeBinding(fpl__ReadU16LE(bp), &outMapping->buttons[i]);
		bp += 2;
	}
	for (size_t i = 0; i < FPL_GAMEPAD_AXIS_COUNT; ++i) {
		fplDecodeBinding(fpl__ReadU16LE(bp), &outMapping->axes[i]);
		bp += 2;
	}
}

// Compares two raw GUIDs lexicographically. <0 if a<b, 0 equal, >0 if a>b.
static int fpl__CompareGuid(const uint8_t a[FPL_GAMEPAD_GUID_SIZE], const uint8_t b[FPL_GAMEPAD_GUID_SIZE]) {
	for (size_t i = 0; i < FPL_GAMEPAD_GUID_SIZE; ++i) {
		if (a[i] != b[i]) {
			return (int)a[i] - (int)b[i];
		}
	}
	return 0;
}

// Decompresses the whole blob into outMappings. Caller passes the entry count from FPL_GAMEPAD_MAPPING_TABLE_ENTRY_COUNT (no header bytes in the blob).
uint32_t fplDecompressGamepadMappingTable(const uint8_t *blob, uint32_t entryCount, fplGamepadMapping *outMappings, uint32_t maxMappings) {
	if (blob == fpl_null || outMappings == fpl_null || entryCount == 0) {
		return 0;
	}
	if (entryCount > maxMappings) {
		entryCount = maxMappings;
	}
	for (uint32_t i = 0; i < entryCount; ++i) {
		fplDecodeGamepadMappingEntry(blob + (size_t)i * FPL_GAMEPAD_BLOB_ENTRY_SIZE, &outMappings[i]);
	}
	return entryCount;
}

// Looks up a mapping in an already-decompressed, GUID-sorted table. Prefers an exact platform match, falls back to the first GUID match.
bool fplFindGamepadMapping(const fplGamepadMapping *table, uint32_t tableCount, const uint8_t guid[FPL_GAMEPAD_GUID_SIZE], fplGamepadPlatform platform, fplGamepadMapping *outMapping) {
	if (table == fpl_null || outMapping == fpl_null || tableCount == 0) {
		return false;
	}

	int32_t lo = 0;
	int32_t hi = (int32_t)tableCount - 1;
	int32_t found = -1;
	while (lo <= hi) {
		int32_t mid = lo + (hi - lo) / 2;
		int cmp = fpl__CompareGuid(table[mid].guid, guid);
		if (cmp < 0) {
			lo = mid + 1;
		} else if (cmp > 0) {
			hi = mid - 1;
		} else {
			found = mid;
			break;
		}
	}
	if (found < 0) {
		return false;
	}

	int32_t start = found;
	while (start > 0 && fpl__CompareGuid(table[start - 1].guid, guid) == 0) {
		--start;
	}
	int32_t fallback = -1;
	for (int32_t i = start; i < (int32_t)tableCount; ++i) {
		if (fpl__CompareGuid(table[i].guid, guid) != 0) {
			break;
		}
		if (table[i].platform == platform) {
			*outMapping = table[i];
			return true;
		}
		if (fallback < 0) {
			fallback = i;
		}
	}
	if (fallback >= 0) {
		*outMapping = table[fallback];
		return true;
	}
	return false;
}
