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
#include <assert.h>

#define ftAssert(exp) assert(exp)

#define ftExpects(expectedValue, actualValue) ftAssert((expectedValue) == (actualValue))
#define ftNotExpects(notExpectedValue, actualValue) ftAssert((notExpectedValue) != (actualValue))

#define ftIsTrue(value) ftExpects(true, value)
#define ftIsFalse(value) ftExpects(false, value)

#define ftIsNull(value) ftExpects(nullptr, value)
#define ftIsNotNull(value) ftNotExpects(nullptr, value)

ft_api void ftLine();
ft_api void ftMsg(const char *format, ...);
ft_api void ftAssertStringEquals(const char *expectedStr, const char *actualStr);
ft_api void ftAssertU8Equals(const uint8_t expectedValue, const uint8_t actualValue);
ft_api void ftAssertU16Equals(const uint16_t expectedValue, const uint16_t actualValue);
ft_api void ftAssertU32Equals(const uint32_t expectedValue, const uint32_t actualValue);
ft_api void ftAssertU64Equals(const uint64_t expectedValue, const uint64_t actualValue);
ft_api void ftAssertS8Equals(const int8_t expectedValue, const int8_t actualValue);
ft_api void ftAssertS16Equals(const int16_t expectedValue, const int16_t actualValue);
ft_api void ftAssertS32Equals(const int32_t expectedValue, const int32_t actualValue);
ft_api void ftAssertS64Equals(const int64_t expectedValue, const int64_t actualValue);
ft_api void ftAssertSizeEquals(const size_t expectedValue, const size_t actualValue);
ft_api void ftAssertFloatEquals(const float expectedValue, const float actualValue, const float tolerance = 0.0001f);
ft_api void ftAssertDoubleEquals(const double expectedValue, const double actualValue, const double tolerance = 0.0001);
ft_api void ftAssertPointerEquals(const void *expectedValue, const void *actualValue);

#endif // FT_HEADER

#if defined(FT_IMPLEMENTATION) && !defined(FT_IMPLEMENTED)
#	define FT_IMPLEMENTED

#include <string.h> // strstr
#include <stdarg.h> // va_start, va_end, va_list, va_arg
#include <stdio.h> // vsnprintf
#include <math.h> // fabs

ft_api void ftLine() {
	fprintf(stdout, "*******************************************************************************\n");
}

ft_api void ftMsg(const char *format, ...) {
	va_list argList;
	va_start(argList, format);
	vfprintf(stdout, format, argList);
	va_end(argList);
}

ft_api void ftAssertStringEquals(const char *expectedStr, const char *actualStr) {
	bool match = false;
	ftNotExpects(nullptr, expectedStr);
	ftNotExpects(nullptr, actualStr);
	const char *r = strstr(expectedStr, actualStr);
	ftNotExpects(nullptr, r);
}
ft_api void ftAssertStringNotEquals(const char *expectedStr, const char *actualStr) {
	bool match = false;
	ftNotExpects(nullptr, expectedStr);
	ftNotExpects(nullptr, actualStr);
	const char *r = strstr(expectedStr, actualStr);
	ftExpects(nullptr, r);
}

ft_api void ftAssertU8Equals(const uint8_t expectedValue, const uint8_t actualValue) {
	bool passed = (expectedValue == actualValue);
	ftIsTrue(passed);
}
ft_api void ftAssertS8Equals(const int8_t expectedValue, const int8_t actualValue) {
	bool passed = (expectedValue == actualValue);
	ftIsTrue(passed);
}

ft_api void ftAssertU16Equals(const uint16_t expectedValue, const uint16_t actualValue) {
	bool passed = (expectedValue == actualValue);
	ftIsTrue(passed);
}
ft_api void ftAssertS16Equals(const int16_t expectedValue, const int16_t actualValue) {
	bool passed = (expectedValue == actualValue);
	ftIsTrue(passed);
}

ft_api void ftAssertU32Equals(const uint32_t expectedValue, const uint32_t actualValue) {
	bool passed = (expectedValue == actualValue);
	ftIsTrue(passed);
}
ft_api void ftAssertS32Equals(const int32_t expectedValue, const int32_t actualValue) {
	bool passed = (expectedValue == actualValue);
	ftIsTrue(passed);
}

ft_api void ftAssertU64Equals(const uint64_t expectedValue, const uint64_t actualValue) {
	bool passed = (expectedValue == actualValue);
	ftIsTrue(passed);
}
ft_api void ftAssertS64Equals(const int64_t expectedValue, const int64_t actualValue) {
	bool passed = (expectedValue == actualValue);
	ftIsTrue(passed);
}

ft_api void ftAssertSizeEquals(const size_t expectedValue, const size_t actualValue) {
	bool passed = (expectedValue == actualValue);
	ftIsTrue(passed);
}

ft_api void ftAssertDoubleEquals(const double expectedValue, const double actualValue, const double tolerance) {
	double diff = fabs(expectedValue - actualValue);
	bool passed = diff <= tolerance;
	ftIsTrue(passed);
}

ft_api void ftAssertFloatEquals(const float expectedValue, const float actualValue, const float tolerance) {
	float diff = (float)fabs(expectedValue - actualValue);
	bool passed = diff <= tolerance;
	ftIsTrue(passed);
}

ft_api void ftAssertPointerEquals(const void *expectedValue, const void *actualValue) {
	bool passed = (expectedValue == actualValue);
	ftIsTrue(passed);
}

#endif // FT_IMPLEMENTATION
