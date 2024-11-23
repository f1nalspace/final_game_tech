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

#define FPL_NETWORK_ADDRESS_V4_NAME_LEN 16

// xxx.xxx.xxx.xxx
typedef union fplNetworkAddressV4 {
	struct {
		uint8_t a;
		uint8_t b;
		uint8_t c;
		uint8_t d;
	};
	uint8_t u8[4];
} fplNetworkAddressV4;

typedef char fplNetworkAddressV4Name[FPL_NETWORK_ADDRESS_V4_NAME_LEN];

#define FPL_NETWORK_ADDRESS_V6_NAME_LEN 40

// xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx
typedef union fplNetworkAddressV6 {
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
	uint8_t u16[8];
} fplNetworkAddressV6;

typedef char fplNetworkAddressV6Name[FPL_NETWORK_ADDRESS_V6_NAME_LEN];

typedef enum fplNetworkIpAddressType {
	fplNetworkIpAddressType_None = 0,
	fplNetworkIpAddressType_V4,
	fplNetworkIpAddressType_V6,
} fplNetworkIpAddressType;

typedef struct fplNetworkIpAddressName {
	fplNetworkIpAddressType type;
	union {
		fplNetworkAddressV4Name v4;
		fplNetworkAddressV6Name v6;
	};
} fplNetworkIpAddressName;

typedef struct fplNetworkIpAddress {
	fplNetworkIpAddressType type;
	union {
		fplNetworkAddressV4 v4;
		fplNetworkAddressV6 v6;
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

fplNetwork_api bool fplNetworkIPV4NameToAddress(const fplNetworkAddressV4Name *inName4, fplNetworkAddressV4 *outAddress4);
fplNetwork_api bool fplNetworkIPV4AddressToName(const fplNetworkAddressV4 *inAddress4, fplNetworkAddressV4Name *outName4);

fplNetwork_api bool fplNetworkIPV6NameToAddress(const fplNetworkAddressV6Name *inName6, fplNetworkAddressV6 *outAddress6);
fplNetwork_api bool fplNetworkIPV6AddressToName(const fplNetworkAddressV6 *inAddress6, fplNetworkAddressV6Name *outName6);

fplNetwork_api bool fplNetworkIPNameToAddress(const fplNetworkIpAddressName *inName, fplNetworkIpAddress *outAddress);
fplNetwork_api bool fplNetworkIPAddressToName(const fplNetworkIpAddress *inAddress, fplNetworkIpAddressName *outName);

fplNetwork_api void fplNetworkTest();

#endif // FPL_NETWORK_API

#if !defined(FPL__NET_IMPL) && defined(FPL_NET_IMPLEMENTATION)
#define FPL__NET_IMPL

#define FPL_NETWORK__ARRAYCOUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

#define FPL_NETWORK__CRASH() { *(int *)0 = 0xBAD; }

#define FPL_NETWORK__ASSERT(exp) if (!(exp)) {FPL_NETWORK__CRASH()}

static bool fplNetwork__IsBE() {
	uint32_t x = 1;
	return (*(uint8_t *)&x) == 0; // Check if the first byte is 0
}

fplNetwork_api bool fplNetworkIsBigEndian() {
	return fplNetwork__IsBE() > 0;
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
	return fplNetwork__IsBE() ? host16 : fplNetwork__SwapU16(host16);
}
fplNetwork_api uint16_t fplNetworkNetToHostU16(const uint16_t net16) {
	return fplNetwork__IsBE() ? net16 : fplNetwork__SwapU16(net16);
}

fplNetwork_api uint32_t fplNetworkHostToNetU32(const uint32_t host32) {
	return fplNetwork__IsBE() ? host32 : fplNetwork__SwapU32(host32);
}
fplNetwork_api uint32_t fplNetworkNetToHostU32(const uint32_t net32) {
	return fplNetwork__IsBE() ? net32 : fplNetwork__SwapU32(net32);
}

fplNetwork_api uint64_t fplNetworkHostToNetU64(const uint64_t host64) {
	return fplNetwork__IsBE() ? host64: fplNetwork__SwapU64(host64);
}
fplNetwork_api uint64_t fplNetworkNetToHostU64(const uint64_t net64) {
	return fplNetwork__IsBE() ? net64 : fplNetwork__SwapU64(net64);
}

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

fplNetwork_api void fplNetworkTest() {
	fplNetwork__TestSwapU16();
	fplNetwork__TestSwapU32();
	fplNetwork__TestSwapU64();
}

#endif // FINAL_SOCKET_IMPL