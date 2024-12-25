// TEMP(final): Remove define impl define
#ifndef FPL_NET_IMPLEMENTATION
#define FPL_NET_IMPLEMENTATION
#endif

#ifndef FPL_NETWORK_API
#define FPL_NETWORK_API

#include <stdbool.h>
#include <stdint.h>

// API
#if defined(FPL_NETWORK_PRIVATE)
#	define fplNetwork_api static
#else
#	define fplNetwork_api extern
#endif

// 32-bit or 64-bit
#if defined(_WIN32)
#	if defined(_WIN64)
#		define FPL_NETWORK_IS_64BIT
#	else
#		define FPL_NETWORK_IS_32BIT
#	endif
#elif defined(__GNUC__)
#	if defined(__LP64__)
#		define FPL_NETWORK_IS_64BIT
#	else
#		define FPL_NETWORK_IS_32BIT
#	endif
#else
#	if (defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 8) || (sizeof(void *) == 8)
#		define FPL_NETWORK_IS_64BIT
#	else
#		define FPL_NETWORK_IS_32BIT
#	endif
#endif

// Platform
#if defined(_WIN32) || defined(_WIN64)
#	define FPL_NETWORK_WIN32
#elif defined(__ANDROID__)
#	define FPL_NETWORK_LINUX
#	define FPL_NETWORK_POSIX
#elif defined(__linux__) || defined(__gnu_linux__)
#	define FPL_NETWORK_LINUX
#	define FPL_NETWORK_POSIX
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__) || defined(__bsdi__)
#	define FPL_NETWORK_BSD
#	define FPL_NETWORK_POSIX
#elif defined(unix) || defined(__unix) || defined(__unix__)
#	define FPL_NETWORK_UNIX
#	define FPL_NETWORK_POSIX
#else
#	error "Unsupported platform"
#endif

// Platform includes
#if defined(FPL_NETWORK_WIN32)
#	include <winsock2.h>
#	include <ws2tcpip.h>
#endif

// C vs C++
#if defined(__cplusplus)
#define FPL_NETWORK_CPP
#else
#define FPL_NETWORK_C
#endif

#define FPL_NETWORK_ADDRESS4_STRING_MIN_LENGTH 7
#define FPL_NETWORK_ADDRESS4_STRING_MAX_LENGTH 16

// xxx.xxx.xxx.xxx
typedef union fplNetworkAddress4 {
	struct {
		uint8_t a;
		uint8_t b;
		uint8_t c;
		uint8_t d;
	};
	uint8_t u8[4];
} fplNetworkAddress4;

#define FPL_NETWORK_ADDRESS6_STRING_MIN_LENGTH 40
#define FPL_NETWORK_ADDRESS6_STRING_MAX_LENGTH 40

// xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx
typedef union fplNetworkAddress6 {
	struct {
		uint16_t a;
		uint16_t b;
		uint16_t c;
		uint16_t d;
		uint16_t e;
		uint16_t f;
		uint16_t g;
		uint16_t h;
	};
	uint16_t u16[8];
} fplNetworkAddress6;

typedef enum fplNetworkIpAddressType {
	fplNetworkIpAddressType_None = 0,
	fplNetworkIpAddressType_V4,
	fplNetworkIpAddressType_V6,
} fplNetworkIpAddressType;

typedef struct fplNetworkIpAddress {
	fplNetworkIpAddressType type;
	union {
		fplNetworkAddress4 v4;
		fplNetworkAddress6 v6;
	};
} fplNetworkIpAddress;

// Public API
fplNetwork_api bool fplNetworkIsBigEndian();

fplNetwork_api uint16_t fplNetworkHostToNetU16(const uint16_t host16);
fplNetwork_api uint16_t fplNetworkNetToHostU16(const uint16_t net16);

fplNetwork_api uint32_t fplNetworkHostToNetU32(const uint32_t host32);
fplNetwork_api uint32_t fplNetworkNetToHostU32(const uint32_t net32);

fplNetwork_api uint64_t fplNetworkHostToNetU64(const uint64_t host64);
fplNetwork_api uint64_t fplNetworkNetToHostU64(const uint64_t net64);

fplNetwork_api bool fplNetworkStringToAddress4(const char *buffer, const size_t maxNameLen, fplNetworkAddress4 *outAddress);
fplNetwork_api size_t fplNetworkAddressToString4(const fplNetworkAddress4 *address, const size_t maxBufferLen, char *outBuffer);

fplNetwork_api bool fplNetworkStringToAddress6(const char *buffer, const size_t maxNameLen, fplNetworkAddress6 *outAddress);
fplNetwork_api size_t fplNetworkAddressToString6(const fplNetworkAddress6 *address, const size_t maxBufferLen, char *outBuffer);

fplNetwork_api bool fplNetworkStringToAddressAuto(const char *buffer, const size_t maxNameLen, fplNetworkIpAddress *outAddress);
fplNetwork_api size_t fplNetworkAddressToStringAuto(const fplNetworkIpAddress *address, const size_t maxBufferLen, char *outBuffer);

fplNetwork_api void fplNetworkTest();

#endif // FPL_NETWORK_API

#if !defined(FPL__NET_IMPL) && defined(FPL_NET_IMPLEMENTATION)
#define FPL__NET_IMPL

#include <string.h>

#define FPL_NETWORK__ARRAYCOUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

#define FPL_NETWORK__CRASH() { *(int *)0 = 0xBAD; }

#define FPL_NETWORK__ASSERT(exp) if (!(exp)) {FPL_NETWORK__CRASH()}

#if defined(FPL_NETWORK_CPP)
#define FPL_NETWORK__ZERO {}
#else
#define FPL_NETWORK__ZERO {0}
#endif

static inline bool fpl__NetIsBE() {
	uint32_t x = 1;
	return (*(uint8_t *)&x) == 0; // Check if the first byte is 0
}

fplNetwork_api bool fplNetworkIsBigEndian() {
	return fpl__NetIsBE() > 0;
}

// Swap routines by:
// https://stackoverflow.com/a/2637138

static inline uint16_t fplNetwork__SwapU16(const uint16_t value16) {
	uint16_t result = (value16 << 8) | (value16 >> 8);
	return result;
}

static inline uint32_t fplNetwork__SwapU32(const uint32_t value32) {
	uint32_t result = ((value32 << 8) & 0xFF00FF00U) | ((value32 >> 8) & 0xFF00FFU);
	return (result << 16) | (result >> 16);
}

static inline uint64_t fplNetwork__SwapU64(const uint64_t value64) {
	uint64_t result = ((value64 << 8) & 0xFF00FF00FF00FF00U) | ((value64 >> 8) & 0x00FF00FF00FF00FFU);
	result = ((result << 16) & 0xFFFF0000FFFF0000U) | ((result >> 16) & 0x0000FFFF0000FFFFU);
	result = (result << 32) | (result >> 32);
	return result;
}

fplNetwork_api uint16_t fplNetworkHostToNetU16(const uint16_t host16) {
	return fpl__NetIsBE() ? host16 : fplNetwork__SwapU16(host16);
}
fplNetwork_api uint16_t fplNetworkNetToHostU16(const uint16_t net16) {
	return fpl__NetIsBE() ? net16 : fplNetwork__SwapU16(net16);
}

fplNetwork_api uint32_t fplNetworkHostToNetU32(const uint32_t host32) {
	return fpl__NetIsBE() ? host32 : fplNetwork__SwapU32(host32);
}
fplNetwork_api uint32_t fplNetworkNetToHostU32(const uint32_t net32) {
	return fpl__NetIsBE() ? net32 : fplNetwork__SwapU32(net32);
}

fplNetwork_api uint64_t fplNetworkHostToNetU64(const uint64_t host64) {
	return fpl__NetIsBE() ? host64 : fplNetwork__SwapU64(host64);
}
fplNetwork_api uint64_t fplNetworkNetToHostU64(const uint64_t net64) {
	return fpl__NetIsBE() ? net64 : fplNetwork__SwapU64(net64);
}

static inline bool fpl__NetworkIsNumeric(const char ch) {
	return ch >= '0' && ch <= '9';
}

static inline bool fpl__NetworkIsHexLow(const char ch) {
	return ch >= 'a' && ch <= 'f';
}

static inline bool fpl__NetworkIsHexHigh(const char ch) {
	return ch >= 'A' && ch <= 'F';
}

static bool fpl__NetworkParseInt(const char *buffer, const size_t length, const bool zeroTerminated, uint32_t *outValue, size_t *outLen) {
	if (buffer == NULL || length == 0 || outValue == NULL || outLen == NULL) {
		return false;
	}

	uint32_t tmp = 0;

	size_t numberLen = 0;

	const char *p = buffer;
	size_t offset = 0;
	while (offset < length) {
		char c = *p;
		if (c != 0) {
			if (fpl__NetworkIsNumeric(c)) {
				int num = (int)c - '0';
				tmp = (tmp * 10) + num;
				++numberLen;
			} else {
				break;
			}
		} else if (zeroTerminated) {
			break;
		}
		++p;
		++offset;
	}

	if (numberLen > 0) {
		*outValue = tmp;
		*outLen = numberLen;
		return true;
	}

	return false;
}

static bool fpl__NetworkParseHex(const char *buffer, const size_t length, const bool zeroTerminated, uint32_t *outValue, size_t *outLen) {
	if (buffer == NULL || length == 0 || outValue == NULL || outLen == NULL) {
		return false;
	}

	uint32_t tmp = 0;

	size_t numberLen = 0;

	const char *p = buffer;
	size_t offset = 0;
	while (offset < length) {
		char c = *p;
		if (c != 0) {
			if (fpl__NetworkIsNumeric(c)) {
				int num = (int)c - '0';
				tmp = (tmp * 16) + num;
				++numberLen;
			} else if (fpl__NetworkIsHexLow(c)) {
				int num = (int)c - 'a';
				tmp = (tmp * 16) + num;
				++numberLen;
			} else if (fpl__NetworkIsHexHigh(c)) {
				int num = (int)c - 'A';
				tmp = (tmp * 16) + num;
				++numberLen;
			} else {
				break;
			}
		} else if (zeroTerminated) {
			break;
		}
		++p;
		++offset;
	}

	if (numberLen > 0) {
		*outValue = tmp;
		*outLen = numberLen;
		return true;
	}

	return false;
}

fplNetwork_api bool fplNetworkStringToAddress4(const char *buffer, const size_t maxBufferLen, fplNetworkAddress4 *outAddress) {
	if (buffer == NULL || buffer == 0 || maxBufferLen < FPL_NETWORK_ADDRESS4_STRING_MIN_LENGTH || outAddress == NULL) {
		return false;
	}

	// Format: AAA.BBB.CCC.DDD
	// Example: 192.168.1.42

	uint32_t values[4] = FPL_NETWORK__ZERO;
	bool separatorCheck[4] = { true, true, true, false };

	size_t tmpCharLen = 0;

	size_t namePos = 0;

	for (uint8_t groupIndex = 0; groupIndex < 4; ++groupIndex) {
		if (namePos >= maxBufferLen) {
			return false;
		}
		tmpCharLen = 0;
		if (!fpl__NetworkParseInt(&buffer[namePos], 3, false, &values[groupIndex], &tmpCharLen))
			return false;
		FPL_NETWORK__ASSERT(tmpCharLen <= 3);
		namePos += tmpCharLen;
		if (separatorCheck[groupIndex] && (namePos >= maxBufferLen || buffer[namePos] != '.')) {
			return false;
		}
		++namePos;
	}

	memset(outAddress, 0, sizeof(*outAddress));
	for (uint8_t groupIndex = 0; groupIndex < 4; ++groupIndex) {
		outAddress->u8[groupIndex] = values[groupIndex] & 0xFF;
	}

	return true;
}

fplNetwork_api size_t fplNetworkAddressToString4(const fplNetworkAddress4 *address, const size_t maxBufferLen, char *outBuffer) {
	return 0;
}

fplNetwork_api bool fplNetworkStringToAddress6(const char *buffer, const size_t maxBufferLen, fplNetworkAddress6 *outAddress) {
	return false;
}

fplNetwork_api size_t fplNetworkAddressToString6(const fplNetworkAddress6 *address, const size_t maxBufferLen, char *outBuffer) {
	return 0;
}

fplNetwork_api bool fplNetworkStringToAddressAuto(const char *buffer, const size_t maxBufferLen, fplNetworkIpAddress *outAddress) {
	return false;
}

fplNetwork_api size_t fplNetworkAddressToStringAuto(const fplNetworkIpAddress *address, const size_t maxBufferLen, char *outBuffer) {
	return 0;
}

//
// Test Suite
//

static void fplNetwork__TestSwapU16() {
	uint16_t testValues[] = {
		0x0001,
		0x1234,
		0xFFFF,
		0xABCD
	};
	uint16_t expectedValues[] = {
		0x0100,
		0x3412,
		0xFFFF,
		0xCDAB
	};
	for (size_t i = 0; i < FPL_NETWORK__ARRAYCOUNT(testValues); i++) {
		uint16_t testValue = testValues[i];
		uint16_t expectedValue = expectedValues[i];
		uint16_t swapValue = fplNetwork__SwapU16(testValue);
		FPL_NETWORK__ASSERT(expectedValue == swapValue);
	}
}

static void fplNetwork__TestSwapU32() {
	uint32_t testValues[] = {
		0x00000001,
		0x12345678,
		0xFFFFFFFF,
		0xAABBCCDD
	};
	uint32_t expectedValues[] = {
		0x01000000,
		0x78563412,
		0xFFFFFFFF,
		0xDDCCBBAA
	};
	for (size_t i = 0; i < FPL_NETWORK__ARRAYCOUNT(testValues); i++) {
		uint32_t testValue = testValues[i];
		uint32_t expectedValue = expectedValues[i];
		uint32_t swapValue = fplNetwork__SwapU32(testValue);
		FPL_NETWORK__ASSERT(expectedValue == swapValue);
	}
}

static void fplNetwork__TestSwapU64() {
	uint64_t testValues[] = {
		0x0000000000000001,
		0x0123456789ABCDEF,
		0xFFFFFFFFFFFFFFFF,
		0xAABBCCDDEEFF0011
	};
	uint64_t expectedValues[] = {
		0x0100000000000000,
		0xEFCDAB8967452301,
		0xFFFFFFFFFFFFFFFF,
		0x1100FFEEDDCCBBAA
	};
	for (size_t i = 0; i < FPL_NETWORK__ARRAYCOUNT(testValues); i++) {
		uint64_t testValue = testValues[i];
		uint64_t expectedValue = expectedValues[i];
		uint64_t swapValue = fplNetwork__SwapU64(testValue);
		FPL_NETWORK__ASSERT(expectedValue == swapValue);
	}
}

typedef struct {
	bool expectedSuccess;
	fplNetworkAddress4 expectedAddress;
	const char *str;
} fpl__NetworkAddressStringToAddressTestData4;

static void fpl__NetworkAddressStringToAddressTest4() {
	fpl__NetworkAddressStringToAddressTestData4 values[] = {
		{false,FPL_NETWORK__ZERO,""},							// Empty
		{false,FPL_NETWORK__ZERO," "},							// Too short
		{false,FPL_NETWORK__ZERO,"               "},			// No data, no separators
		{false,FPL_NETWORK__ZERO,"..."},						// No data, ony separators
		{false,FPL_NETWORK__ZERO,"   .   .   .   "},			// No data, ony separators
		{false,FPL_NETWORK__ZERO,"000111222333"},				// Only data without separators
		{false,FPL_NETWORK__ZERO,"000..111..222..333"},			// Too many separators
		{false,FPL_NETWORK__ZERO,"0000.11111.222222.333333"},	// Individual groups are wrong

		{false,FPL_NETWORK__ZERO,"0  .   .   .   "},			// Individual groups are whitespace
		{false,FPL_NETWORK__ZERO,"   .0  .   .   "},			// Individual groups are whitespace
		{false,FPL_NETWORK__ZERO,"   .   .0  .   "},			// Individual groups are whitespace
		{false,FPL_NETWORK__ZERO,"   .   .   .0  "},			// Individual groups are whitespace
		{false,FPL_NETWORK__ZERO,"  0.   .   .   "},			// Individual groups are whitespace
		{false,FPL_NETWORK__ZERO,"   .  0.   .   "},			// Individual groups are whitespace
		{false,FPL_NETWORK__ZERO,"   .   .  0.   "},			// Individual groups are whitespace
		{false,FPL_NETWORK__ZERO,"   .   .   .  0"},			// Individual groups are whitespace

		{false,FPL_NETWORK__ZERO,"42 .   .   .   "},			// Individual groups are whitespace
		{false,FPL_NETWORK__ZERO,"   .42 .   .   "},			// Individual groups are whitespace
		{false,FPL_NETWORK__ZERO,"   .   .42 .   "},			// Individual groups are whitespace
		{false,FPL_NETWORK__ZERO,"   .   .   .42 "},			// Individual groups are whitespace
		{false,FPL_NETWORK__ZERO," 42.   .   .   "},			// Individual groups are whitespace
		{false,FPL_NETWORK__ZERO,"   . 42.   .   "},			// Individual groups are whitespace
		{false,FPL_NETWORK__ZERO,"   .   . 42.   "},			// Individual groups are whitespace
		{false,FPL_NETWORK__ZERO,"   .   .   . 42"},			// Individual groups are whitespace

		{false,FPL_NETWORK__ZERO,"255.   .   .   "},			// Individual groups are whitespace
		{false,FPL_NETWORK__ZERO,"   .255.   .   "},			// Individual groups are whitespace
		{false,FPL_NETWORK__ZERO,"   .   .255.   "},			// Individual groups are whitespace
		{false,FPL_NETWORK__ZERO,"   .   .   .255"},			// Individual groups are whitespace
		{false,FPL_NETWORK__ZERO,"999.   .   .   "},			// Individual groups are whitespace
		{false,FPL_NETWORK__ZERO,"   .999.   .   "},			// Individual groups are whitespace
		{false,FPL_NETWORK__ZERO,"   .   .999.   "},			// Individual groups are whitespace
		{false,FPL_NETWORK__ZERO,"   .   .   .999"},			// Individual groups are whitespace

		{false,FPL_NETWORK__ZERO,"  1.  2.  3.  4"},			// Wrong whitespaces
		{false,FPL_NETWORK__ZERO,"1  .2  .3  .4  "},			// Wrong whitespaces

		{true,{100,100,100,100},"100.100.100.100"},
		{true,{255,255,255,255},"255.255.255.255"},
		{true,{104,157,208,239},"104.157.208.239"},
		{true,{255,0,0,0},"255.0.0.0"},
		{true,{0,0,0,255},"0.0.0.255"},
		{true,{1,10,100,200},"1.10.100.200"},
		{true,{1,10,100,53},"001.010.100.053"},
	};

	for (size_t i = 0; i < FPL_NETWORK__ARRAYCOUNT(values); i++) {
		const char *buffer = values[i].str;
		size_t bufferLen = strlen(buffer);
		bool expectedSuccess = values[i].expectedSuccess;
		fplNetworkAddress4 expectedAddress = values[i].expectedAddress;
		fplNetworkAddress4 actualAddress = FPL_NETWORK__ZERO;
		bool res = fplNetworkStringToAddress4(buffer, bufferLen, &actualAddress);
		FPL_NETWORK__ASSERT(expectedSuccess == res);
		FPL_NETWORK__ASSERT(expectedAddress.a == actualAddress.a);
		FPL_NETWORK__ASSERT(expectedAddress.b == actualAddress.b);
		FPL_NETWORK__ASSERT(expectedAddress.c == actualAddress.c);
		FPL_NETWORK__ASSERT(expectedAddress.d == actualAddress.d);
	}
}

typedef struct {
	bool expectedSuccess;
	fplNetworkAddress6 expectedAddress;
	const char *str;
} fpl__NetworkAddressStringToAddressTestData6;

// Test function for IPv6 address parsing
static void fpl__NetworkAddressStringToAddressTest6() {
	fpl__NetworkAddressStringToAddressTestData6 values[] = {
		// Invalid cases:
		{false, FPL_NETWORK__ZERO, ""},                          // Empty string
		{false, FPL_NETWORK__ZERO, " "},                          // Whitespace only
		{false, FPL_NETWORK__ZERO, "notanipv6address"},           // Invalid characters
		{false, FPL_NETWORK__ZERO, "2001::abcd::1234"},           // Too many "::" shorthand
		{false, FPL_NETWORK__ZERO, "2001:db8::1234:5678:90ab:"},  // Trailing colon
		{false, FPL_NETWORK__ZERO, "2001:db8:abcd:1234:5678:90ab::1234"}, // Too many segments with "::"
		{false, FPL_NETWORK__ZERO, "2001:db8:abcd::1234::5678"},  // Invalid shorthand placement
		{false, FPL_NETWORK__ZERO, "2001:db8:abcd:::1234:5678"},  // Invalid shorthand placement

		// Valid cases:
		{true, {0x2001, 0x0db8, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}, "2001:db8::"}, // Valid compressed
		{true, {0x2001, 0x0db8, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001}, "2001:db8::1"}, // Valid with a single segment
		{true, {0x2001, 0x0db8, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001}, "2001:db8:0:0:0:0:0:1"}, // Fully expanded
		{true, {0x2001, 0x0db8, 0x00f0, 0x0123, 0x4567, 0x89ab, 0xcdef, 0x0123}, "2001:db8:f0:1234:5678:9abc:def0:1234"}, // Full expanded with hexadecimal numbers
		{true, {0x2001, 0x0db8, 0x00f0, 0x0123, 0x4567, 0x89ab, 0xcdef, 0x0123}, "2001:db8:F0:1234:5678:9ABC:DEf0:1234"}, // Full expanded with mixed case
		{true, {0x2001, 0x0db8, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006}, "2001:db8:1:2:3:4:5:6"}, // Valid mixed sections with "::"
		{true, {0x2001, 0x0db8, 0x1020, 0x3040, 0x5060, 0x7080, 0x90a0, 0xb0c0}, "2001:db8:1020:3040:5060:7080:90a0:b0c0"} // Valid with all segments fully expanded
	};

	for (size_t i = 0; i < FPL_NETWORK__ARRAYCOUNT(values); i++) {
		const char *buffer = values[i].str;
		size_t bufferLen = strlen(buffer);
		bool expectedSuccess = values[i].expectedSuccess;
		fplNetworkAddress6 expectedAddress = values[i].expectedAddress;
		fplNetworkAddress6 actualAddress = FPL_NETWORK__ZERO;
		bool res = fplNetworkStringToAddress6(buffer, bufferLen, &actualAddress);
		FPL_NETWORK__ASSERT(expectedSuccess == res);
		if (expectedSuccess) {
			FPL_NETWORK__ASSERT(memcmp(&expectedAddress, &actualAddress, sizeof(fplNetworkAddress6)) == 0);
		}
	}
}

fplNetwork_api void fplNetworkTest() {
	fplNetwork__TestSwapU16();
	fplNetwork__TestSwapU32();
	fplNetwork__TestSwapU64();
	fpl__NetworkAddressStringToAddressTest4();
	fpl__NetworkAddressStringToAddressTest6();
}

#endif // FINAL_SOCKET_IMPL