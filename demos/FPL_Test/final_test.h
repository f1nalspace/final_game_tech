#ifndef FT_HEADER
#	define FT_HEADER

//
// Architecture detection (x64, x86)
//
#if defined(_M_X64) || defined(__x86_64__) || defined(__amd64__)
#	define FT_ARCH_X64
#elif defined(_M_IX86) || defined(__i386__) || defined(__X86__) || defined(_X86_)
#	define FT_ARCH_X86
#elif defined(__arm__) || defined(_M_ARM)
#	if defined(__aarch64__)
#		define FT_ARCH_ARM64
#	else
#		define FT_ARCH_ARM32
#	endif
#else
#	error "This architecture is not supported!"
#endif // FT_ARCH

#if !defined(FT_PRIVATE)
#	define ft_api extern
#else
#	define ft_api static
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

#define ftAssert(exp) assert(exp)

ft_api void ftLine(void);
ft_api void ftMsg(const char *format, ...);
ft_api void ftFail(const char *format, ...);

ft_api void ftAssertTrue(const bool value);
ft_api void ftAssertFalse(const bool value);
ft_api void ftAssertNull(const void *value);
ft_api void ftAssertNotNull(const void *value);

ft_api void ftAssertStringEquals(const char *expectedStr, const char *actualStr);
ft_api void ftAssertStringNotEquals(const char *expectedStr, const char *actualStr);

ft_api void ftAssertU8Equals(const uint8_t expectedValue, const uint8_t actualValue);
ft_api void ftAssertU16Equals(const uint16_t expectedValue, const uint16_t actualValue);
ft_api void ftAssertU32Equals(const uint32_t expectedValue, const uint32_t actualValue);
ft_api void ftAssertU64Equals(const uint64_t expectedValue, const uint64_t actualValue);
ft_api void ftAssertS8Equals(const int8_t expectedValue, const int8_t actualValue);
ft_api void ftAssertS16Equals(const int16_t expectedValue, const int16_t actualValue);
ft_api void ftAssertS32Equals(const int32_t expectedValue, const int32_t actualValue);
ft_api void ftAssertS64Equals(const int64_t expectedValue, const int64_t actualValue);
ft_api void ftAssertSizeEquals(const size_t expectedValue, const size_t actualValue);
ft_api void ftAssertFloatEquals(const float expectedValue, const float actualValue, const float tolerance);
ft_api void ftAssertDoubleEquals(const double expectedValue, const double actualValue, const double tolerance);
ft_api void ftAssertPointerEquals(const void *expectedValue, const void *actualValue);
ft_api void ftAssertCharEquals(const char expectedValue, const char actualValue);

// Convenience wrappers with default tolerance
#define ftAssertFloatEqualsDefault(expected, actual) ftAssertFloatEquals((expected), (actual), 0.0001f)
#define ftAssertDoubleEqualsDefault(expected, actual) ftAssertDoubleEquals((expected), (actual), 0.0001)

// Generic helpers (kept for legacy call-sites)
#define ftExpects(expectedValue, actualValue) ftAssert((expectedValue) == (actualValue))
#define ftNotExpects(notExpectedValue, actualValue) ftAssert((notExpectedValue) != (actualValue))

#define ftIsTrue(value) ftAssertTrue((bool)(value))
#define ftIsFalse(value) ftAssertFalse((bool)(value))
#define ftIsNull(value) ftAssertNull((const void *)(value))
#define ftIsNotNull(value) ftAssertNotNull((const void *)(value))

#endif // FT_HEADER

#if defined(FT_IMPLEMENTATION) && !defined(FT_IMPLEMENTED)
#	define FT_IMPLEMENTED

#include <string.h> // strstr
#include <stdarg.h> // va_start, va_end, va_list, va_arg
#include <stdio.h>  // vsnprintf, fprintf
#include <math.h>   // fabs

ft_api void ftLine(void) {
	fprintf(stdout, "*******************************************************************************\n");
	fflush(stdout);
}

ft_api void ftMsg(const char *format, ...) {
	va_list argList;
	va_start(argList, format);
	vfprintf(stdout, format, argList);
	va_end(argList);
	fflush(stdout);
}

ft_api void ftFail(const char *format, ...) {
	va_list argList;
	fprintf(stderr, "ASSERTION FAILED: ");
	va_start(argList, format);
	vfprintf(stderr, format, argList);
	va_end(argList);
	fprintf(stderr, "\n");
	fflush(stderr);
}

ft_api void ftAssertTrue(const bool value) {
	if (!value) {
		ftFail("expected 'true', got 'false'");
	}
	ftAssert(value);
}
ft_api void ftAssertFalse(const bool value) {
	if (value) {
		ftFail("expected 'false', got 'true'");
	}
	ftAssert(!value);
}

ft_api void ftAssertNull(const void *value) {
	if (value != NULL) {
		ftFail("expected NULL pointer, got %p", value);
	}
	ftAssert(value == NULL);
}
ft_api void ftAssertNotNull(const void *value) {
	if (value == NULL) {
		ftFail("expected non-NULL pointer, got NULL");
	}
	ftAssert(value != NULL);
}

ft_api void ftAssertStringEquals(const char *expectedStr, const char *actualStr) {
	if (expectedStr == NULL) {
		ftFail("ftAssertStringEquals: expectedStr is NULL");
	}
	ftAssert(expectedStr != NULL);
	if (actualStr == NULL) {
		ftFail("ftAssertStringEquals: actualStr is NULL (expected '%s')", expectedStr);
	}
	ftAssert(actualStr != NULL);
	const char *r = strstr(expectedStr, actualStr);
	if (r == NULL) {
		ftFail("expected string contains '%s', actual '%s'", actualStr, expectedStr);
	}
	ftAssert(r != NULL);
}

ft_api void ftAssertStringNotEquals(const char *expectedStr, const char *actualStr) {
	if (expectedStr == NULL) {
		ftFail("ftAssertStringNotEquals: expectedStr is NULL");
	}
	ftAssert(expectedStr != NULL);
	if (actualStr == NULL) {
		ftFail("ftAssertStringNotEquals: actualStr is NULL");
	}
	ftAssert(actualStr != NULL);
	const char *r = strstr(expectedStr, actualStr);
	if (r != NULL) {
		ftFail("expected string '%s' to NOT contain '%s'", expectedStr, actualStr);
	}
	ftAssert(r == NULL);
}

ft_api void ftAssertU8Equals(const uint8_t expectedValue, const uint8_t actualValue) {
	if (expectedValue != actualValue) {
		ftFail("expected uint8 %u, got %u", (unsigned)expectedValue, (unsigned)actualValue);
	}
	ftAssert(expectedValue == actualValue);
}
ft_api void ftAssertS8Equals(const int8_t expectedValue, const int8_t actualValue) {
	if (expectedValue != actualValue) {
		ftFail("expected int8 %d, got %d", (int)expectedValue, (int)actualValue);
	}
	ftAssert(expectedValue == actualValue);
}

ft_api void ftAssertU16Equals(const uint16_t expectedValue, const uint16_t actualValue) {
	if (expectedValue != actualValue) {
		ftFail("expected uint16 %u, got %u", (unsigned)expectedValue, (unsigned)actualValue);
	}
	ftAssert(expectedValue == actualValue);
}
ft_api void ftAssertS16Equals(const int16_t expectedValue, const int16_t actualValue) {
	if (expectedValue != actualValue) {
		ftFail("expected int16 %d, got %d", (int)expectedValue, (int)actualValue);
	}
	ftAssert(expectedValue == actualValue);
}

ft_api void ftAssertU32Equals(const uint32_t expectedValue, const uint32_t actualValue) {
	if (expectedValue != actualValue) {
		ftFail("expected uint32 %lu, got %lu", (unsigned long)expectedValue, (unsigned long)actualValue);
	}
	ftAssert(expectedValue == actualValue);
}
ft_api void ftAssertS32Equals(const int32_t expectedValue, const int32_t actualValue) {
	if (expectedValue != actualValue) {
		ftFail("expected int32 %ld, got %ld", (long)expectedValue, (long)actualValue);
	}
	ftAssert(expectedValue == actualValue);
}

ft_api void ftAssertU64Equals(const uint64_t expectedValue, const uint64_t actualValue) {
	if (expectedValue != actualValue) {
		ftFail("expected uint64 %llu, got %llu", (unsigned long long)expectedValue, (unsigned long long)actualValue);
	}
	ftAssert(expectedValue == actualValue);
}
ft_api void ftAssertS64Equals(const int64_t expectedValue, const int64_t actualValue) {
	if (expectedValue != actualValue) {
		ftFail("expected int64 %lld, got %lld", (long long)expectedValue, (long long)actualValue);
	}
	ftAssert(expectedValue == actualValue);
}

ft_api void ftAssertSizeEquals(const size_t expectedValue, const size_t actualValue) {
	if (expectedValue != actualValue) {
		ftFail("expected size %llu, got %llu", (unsigned long long)expectedValue, (unsigned long long)actualValue);
	}
	ftAssert(expectedValue == actualValue);
}

ft_api void ftAssertDoubleEquals(const double expectedValue, const double actualValue, const double tolerance) {
	double diff = fabs(expectedValue - actualValue);
	if (diff > tolerance) {
		ftFail("expected double %.9f (+/- %.9f), got %.9f (diff %.9f)", expectedValue, tolerance, actualValue, diff);
	}
	ftAssert(diff <= tolerance);
}

ft_api void ftAssertFloatEquals(const float expectedValue, const float actualValue, const float tolerance) {
	float diff = (float)fabs(expectedValue - actualValue);
	if (diff > tolerance) {
		ftFail("expected float %.7f (+/- %.7f), got %.7f (diff %.7f)", (double)expectedValue, (double)tolerance, (double)actualValue, (double)diff);
	}
	ftAssert(diff <= tolerance);
}

ft_api void ftAssertPointerEquals(const void *expectedValue, const void *actualValue) {
	if (expectedValue != actualValue) {
		ftFail("expected pointer %p, got %p", expectedValue, actualValue);
	}
	ftAssert(expectedValue == actualValue);
}

ft_api void ftAssertCharEquals(const char expectedValue, const char actualValue) {
	if (expectedValue != actualValue) {
		ftFail("expected char '%c' (0x%02x), got '%c' (0x%02x)",
			(expectedValue >= 0x20 && (unsigned char)expectedValue < 0x7F) ? expectedValue : '?',
			(unsigned)(unsigned char)expectedValue,
			(actualValue >= 0x20 && (unsigned char)actualValue < 0x7F) ? actualValue : '?',
			(unsigned)(unsigned char)actualValue);
	}
	ftAssert(expectedValue == actualValue);
}

#endif // FT_IMPLEMENTATION
