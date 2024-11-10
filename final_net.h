// TEMP(final): Remove define impl define
#ifndef FINAL_SOCKET_IMPLEMENTATION
#define FINAL_SOCKET_IMPLEMENTATION
#endif

#ifndef FINAL_NET_API
#define FINAL_NET_API

#include <stdbool.h>
#include <stdint.h>

// API
#if defined(FNET_PRIVATE)
#define fnet_api static
#else
#define fnet_api extern
#endif

// 32-bit or 64-bit
#if defined(_WIN32)
#	if defined(_WIN64)
#		define FNET_IS_64BIT
#	else
#		define FNET_IS_32BIT
#	endif
#elif defined(__GNUC__)
#	if defined(__LP64__)
#		define FNET_IS_64BIT
#	else
#		define FNET_IS_32BIT
#	endif
#else
#	if (defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 8) || (sizeof(void *) == 8)
#		define FNET_IS_64BIT
#	else
#		define FNET_IS_32BIT
#	endif
#endif

// Public API
fnet_api bool fnetIsBigEndian();

fnet_api uint16_t fnetHostToNetU16(const uint16_t host16);
fnet_api uint16_t fnetNetToHostU16(const uint16_t net16);

fnet_api uint32_t fnetHostToNetU32(const uint32_t host32);
fnet_api uint32_t fnetNetToHostU32(const uint32_t net32);

fnet_api uint64_t fnetHostToNetU64(const uint64_t host64);
fnet_api uint64_t fnetNetToHostU64(const uint64_t net64);

fnet_api void fnetTest();

#endif // FINAL_NET_API

#if !defined(FINAL_SOCKET_IMPL) && defined(FINAL_SOCKET_IMPLEMENTATION)
#define FINAL_SOCKET_IMPL

#define FNET_ARRAYCOUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

#define FNET_CRASH() { *(int *)0 = 0xBAD; }

#define FNET_ASSERT(exp) if (!(exp)) {FNET_CRASH()}

static bool fnet__IsBE() {
	uint32_t x = 1;
	return (*(uint8_t *)&x) == 0; // Check if the first byte is 0
}

typedef struct {
	union {
		uint8_t lowLE;
		uint8_t highLE;
	};
	union {
		uint8_t highBE;
		uint8_t lowBE;
	};
	uint16_t u16;
} fnet__U16;

fnet_api bool fnetIsBigEndian() {
	return fnet__IsBE() > 0;
}

static inline uint16_t fnet__SwapU16(const uint16_t value16) {
	uint16_t result = (value16 << 8) | (value16 >> 8);
	return result;
}

static inline uint32_t fnet__SwapU32(const uint32_t value32) {
	uint32_t result = ((value32 << 8) & 0xFF00FF00U) | ((value32 >> 8) & 0xFF00FFU);
	return (result << 16) | (result >> 16);
}

static inline uint64_t fnet__SwapU64(const uint64_t value64) {
	uint64_t result = ((value64 << 8) & 0xFF00FF00FF00FF00U) | ((value64 >> 8) & 0x00FF00FF00FF00FFU);
	result = ((result << 16) & 0xFFFF0000FFFF0000U) | ((result >> 16) & 0x0000FFFF0000FFFFU);
	result = (result << 32) | (result >> 32);
	return result;
}

fnet_api uint16_t fnetHostToNetU16(const uint16_t host16) {
	return fnet__IsBE() ? host16 : fnet__SwapU16(host16);
}
fnet_api uint16_t fnetNetToHostU16(const uint16_t net16) {
	return fnet__IsBE() ? net16 : fnet__SwapU16(net16);
}

fnet_api uint32_t fnetHostToNetU32(const uint32_t host32) {
	return fnet__IsBE() ? host32 : fnet__SwapU32(host32);
}
fnet_api uint32_t fnetNetToHostU32(const uint32_t net32) {
	return fnet__IsBE() ? net32 : fnet__SwapU32(net32);
}

fnet_api uint64_t fnetHostToNetU64(const uint64_t host64) {
	return fnet__IsBE() ? host64: fnet__SwapU64(host64);
}
fnet_api uint64_t fnetNetToHostU64(const uint64_t net64) {
	return fnet__IsBE() ? net64 : fnet__SwapU64(net64);
}

static void fnet__TestSwapU16() {
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
	for (size_t i = 0; i < FNET_ARRAYCOUNT(testValues); i++) {
		uint16_t testValue = testValues[i];
		uint16_t expectedValue = expectedValues[i];
		uint16_t swapValue = fnet__SwapU16(testValue);
		FNET_ASSERT(expectedValue == swapValue);
	}
}

static void fnet__TestSwapU32() {
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
	for (size_t i = 0; i < FNET_ARRAYCOUNT(testValues); i++) {
		uint32_t testValue = testValues[i];
		uint32_t expectedValue = expectedValues[i];
		uint32_t swapValue = fnet__SwapU32(testValue);
		FNET_ASSERT(expectedValue == swapValue);
	}
}

static void fnet__TestSwapU64() {
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
	for (size_t i = 0; i < FNET_ARRAYCOUNT(testValues); i++) {
		uint64_t testValue = testValues[i];
		uint64_t expectedValue = expectedValues[i];
		uint64_t swapValue = fnet__SwapU64(testValue);
		FNET_ASSERT(expectedValue == swapValue);
	}
}

fnet_api void fnetTest() {
	fnet__TestSwapU16();
	fnet__TestSwapU32();
	fnet__TestSwapU64();
}

#endif // FINAL_SOCKET_IMPL