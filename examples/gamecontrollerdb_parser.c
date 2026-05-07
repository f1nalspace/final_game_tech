/*
-------------------------------------------------------------------------------
Name:
	GameControllerDB Parser

Description:
	Parses a single line of an SDL gamecontrollerdb.txt file into a
	fplGamepadMapping structure that maps SDL inputs (buttons, axes, hats)
	onto FPL gamepad button/axis types.

	Line format (SDL 2.0.16):
		<guid>,<device-name>,<key>:<value>,...,platform:<name>,

	Value formats:
		bN        SDL button index N
		aN        SDL axis index N
		+aN / -aN positive/negative half-axis
		aN~       inverted axis
		hN.M      hat N, mask M (1=up, 2=right, 4=down, 8=left)

	Device name and the runtime flags (isConnected, isActive) are ignored.

Requirements:
	- C99 Compiler
	- Final Platform Layer
-------------------------------------------------------------------------------
*/

#include <final_platform_layer.h>

// SDL input source kind for a single mapping entry.
typedef enum fplGamepadInputType {
	fplGamepadInputType_None = 0,
	fplGamepadInputType_Button,
	fplGamepadInputType_Axis,
	fplGamepadInputType_Hat,
} fplGamepadInputType;

// Half-axis selector for axis bindings.
typedef enum fplGamepadAxisSign {
	fplGamepadAxisSign_Full = 0,
	fplGamepadAxisSign_Positive,
	fplGamepadAxisSign_Negative,
} fplGamepadAxisSign;

// Single SDL input source bound to one FPL button or axis slot.
typedef struct fplGamepadInputBinding {
	fplGamepadInputType type;
	uint32_t index;
	uint32_t hatMask;
	fplGamepadAxisSign axisSign;
	bool axisInverted;
} fplGamepadInputBinding;

// FPL analog axis slots filled from a mapping line.
typedef enum fplGamepadAxisType {
	fplGamepadAxisType_LeftX = 0,
	fplGamepadAxisType_LeftY,
	fplGamepadAxisType_RightX,
	fplGamepadAxisType_RightY,
	fplGamepadAxisType_LeftTrigger,
	fplGamepadAxisType_RightTrigger,
	fplGamepadAxisType_Count,
} fplGamepadAxisType;

// Target platform of a mapping entry (from "platform:" field).
typedef enum fplGamepadPlatform {
	fplGamepadPlatform_Unknown = 0,
	fplGamepadPlatform_Windows,
	fplGamepadPlatform_Linux,
	fplGamepadPlatform_MacOS,
	fplGamepadPlatform_Android,
	fplGamepadPlatform_iOS,
} fplGamepadPlatform;

// Full mapping parsed from one gamecontrollerdb line.
typedef struct fplGamepadMapping {
	char guid[33];
	fplGamepadInputBinding buttons[14];
	fplGamepadInputBinding axes[fplGamepadAxisType_Count];
	fplGamepadPlatform platform;
} fplGamepadMapping;

// Returns offset of first occurrence of c in [str, str+len), or len if not found.
static size_t fpl__FindChar(const char *str, size_t len, char c) {
	for (size_t i = 0; i < len; ++i) {
		if (str[i] == c) return i;
	}
	return len;
}

// Returns the length of a leading run of decimal digits.
static size_t fpl__SpanDigits(const char *str, size_t len) {
	size_t i = 0;
	while (i < len && str[i] >= '0' && str[i] <= '9') ++i;
	return i;
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
	fplGamepadInputBinding b = fplZeroInit;
	if (valLen == 0) return false;

	size_t pos = 0;
	if (val[pos] == '+') { b.axisSign = fplGamepadAxisSign_Positive; ++pos; }
	else if (val[pos] == '-') { b.axisSign = fplGamepadAxisSign_Negative; ++pos; }

	if (pos >= valLen) return false;
	char typeChar = val[pos++];

	if (typeChar == 'b') {
		b.type = fplGamepadInputType_Button;
		size_t digits = fpl__SpanDigits(val + pos, valLen - pos);
		if (digits == 0) return false;
		if (!fplTryStringToS32Len(val + pos, digits, &b.index)) return false;
	} else if (typeChar == 'a') {
		b.type = fplGamepadInputType_Axis;
		size_t digits = fpl__SpanDigits(val + pos, valLen - pos);
		if (digits == 0) return false;
		if (!fplTryStringToS32Len(val + pos, digits, &b.index)) return false;
		if (pos + digits < valLen && val[pos + digits] == '~') b.axisInverted = true;
	} else if (typeChar == 'h') {
		b.type = fplGamepadInputType_Hat;
		size_t rem = valLen - pos;
		size_t dot = fpl__FindChar(val + pos, rem, '.');
		if (dot == 0 || dot >= rem) return false;
		if (!fplTryStringToS32Len(val + pos, dot, &b.index)) return false;
		size_t maskStart = pos + dot + 1;
		if (maskStart >= valLen) return false;
		if (!fplTryStringToS32Len(val + maskStart, valLen - maskStart, &b.hatMask)) return false;
	} else {
		return false;
	}

	*outBinding = b;
	return true;
}

// Raw SDL-style input snapshot used to drive fplApplyGamepadMapping.
// Caller is responsible for filling these from whatever joystick API they use.
typedef struct fplGamepadRawInput {
	// Each axis is expected in SDL convention: -1.0 .. +1.0 (triggers idle at -1.0, fully pressed at +1.0).
	float axes[8];
	// Buttons masks: false=not pressed, true=pressed
	bool buttons[32];
	// Hat masks: bit 1=up, 2=right, 4=down, 8=left (SDL_HAT_*).
	uint8_t hats[8];
	// Number of buttons
	uint32_t buttonCount;
	// Number of axes
	uint32_t axisCount;
	// Number of hats
	uint32_t hatCount;
} fplGamepadRawInput;

// Threshold above which an analog input is treated as a digital press.
#define FPL_GAMEPAD_DIGITAL_THRESHOLD 0.5f

static bool fpl__EvalBindingDigital(const fplGamepadInputBinding *b, const fplGamepadRawInput *in) {
	switch (b->type) {
		case fplGamepadInputType_Button:
			if (b->index < 0 || b->index >= in->buttonCount) return false;
			return in->buttons[b->index];
		case fplGamepadInputType_Hat:
			if (b->index < 0 || b->index >= in->hatCount) return false;
			return (in->hats[b->index] & (uint8_t)b->hatMask) != 0;
		case fplGamepadInputType_Axis: {
			if (b->index < 0 || b->index >= in->axisCount) return false;
			float v = in->axes[b->index];
			if (b->axisInverted) v = -v;
			if (b->axisSign == fplGamepadAxisSign_Negative) v = -v;
			return v > FPL_GAMEPAD_DIGITAL_THRESHOLD;
		}
		default:
			return false;
	}
}

static float fpl__EvalBindingAnalog(const fplGamepadInputBinding *b, const fplGamepadRawInput *in) {
	switch (b->type) {
		case fplGamepadInputType_Button:
			if (b->index < 0 || b->index >= in->buttonCount) return 0.0f;
			return in->buttons[b->index] ? 1.0f : 0.0f;
		case fplGamepadInputType_Hat:
			if (b->index < 0 || b->index >= in->hatCount) return 0.0f;
			return (in->hats[b->index] & (uint8_t)b->hatMask) ? 1.0f : 0.0f;
		case fplGamepadInputType_Axis: {
			if (b->index < 0 || b->index >= in->axisCount) return 0.0f;
			float v = in->axes[b->index];
			if (b->axisInverted) v = -v;
			// Half-axis: remap [-1..+1] to [0..1] using only the requested half (used by triggers).
			if (b->axisSign == fplGamepadAxisSign_Positive) return (v + 1.0f) * 0.5f;
			if (b->axisSign == fplGamepadAxisSign_Negative) return (-v + 1.0f) * 0.5f;
			return v;
		}
		default:
			return 0.0f;
	}
}

// Applies mapping + raw input onto outState. Buttons and analog axes are written;
// deviceName, isConnected and isActive are left untouched per project policy.
bool fplApplyGamepadMapping(const fplGamepadMapping *mapping, const fplGamepadRawInput *input, fplGamepadState *outState) {
	if (mapping == fpl_null || input == fpl_null || outState == fpl_null) return false;

	for (size_t i = 0; i < fplArrayCount(mapping->buttons); ++i) {
		const fplGamepadInputBinding *b = &mapping->buttons[i];
		bool down = (b->type != fplGamepadInputType_None) && fpl__EvalBindingDigital(b, input);
		outState->buttons[i].isDown = down ? 1 : 0;
	}

	outState->leftStickX    = fpl__EvalBindingAnalog(&mapping->axes[fplGamepadAxisType_LeftX],        input);
	outState->leftStickY    = fpl__EvalBindingAnalog(&mapping->axes[fplGamepadAxisType_LeftY],        input);
	outState->rightStickX   = fpl__EvalBindingAnalog(&mapping->axes[fplGamepadAxisType_RightX],       input);
	outState->rightStickY   = fpl__EvalBindingAnalog(&mapping->axes[fplGamepadAxisType_RightY],       input);
	outState->leftTrigger   = fpl__EvalBindingAnalog(&mapping->axes[fplGamepadAxisType_LeftTrigger],  input);
	outState->rightTrigger  = fpl__EvalBindingAnalog(&mapping->axes[fplGamepadAxisType_RightTrigger], input);

	return true;
}

// Parses one line of a gamecontrollerdb.txt file into outMapping.
// Returns true if the line yielded a usable mapping (has GUID and at least a name field).
// Comment lines, empty lines, and malformed lines return false.
bool fplParseGameControllerMappingLine(const char *line, fplGamepadMapping *outMapping) {
	if (line == fpl_null || outMapping == fpl_null) return false;

	fplGamepadMapping zero = fplZeroInit;
	*outMapping = zero;

	size_t lineLen = fplGetStringLength(line);
	while (lineLen > 0) {
		char c = line[lineLen - 1];
		if (c == '\n' || c == '\r' || c == ' ' || c == '\t') --lineLen;
		else break;
	}
	if (lineLen == 0) return false;
	if (line[0] == '#') return false;

	// GUID
	size_t guidEnd = fpl__FindChar(line, lineLen, ',');
	if (guidEnd == 0 || guidEnd >= lineLen) return false;
	fplCopyStringLen(line, guidEnd, outMapping->guid, fplArrayCount(outMapping->guid));
	size_t pos = guidEnd + 1;

	// Skip device name
	size_t remaining = lineLen - pos;
	size_t nameEnd = fpl__FindChar(line + pos, remaining, ',');
	if (nameEnd >= remaining) return false;
	pos += nameEnd + 1;

	// Key:value pairs
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

		if (pairEnd >= remaining) break;
		pos += pairEnd + 1;
	}

	return true;
}
