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

namespace ft {
	ft_api void Line();
	ft_api void Msg(const char *format, ...);
	ft_api void AssertStringEquals(const char *expectedStr, const char *actualStr);
	ft_api void AssertU8Equals(const uint8_t expectedValue, const uint8_t actualValue);
	ft_api void AssertU16Equals(const uint16_t expectedValue, const uint16_t actualValue);
	ft_api void AssertU32Equals(const uint32_t expectedValue, const uint32_t actualValue);
	ft_api void AssertU64Equals(const uint64_t expectedValue, const uint64_t actualValue);
	ft_api void AssertS8Equals(const int8_t expectedValue, const int8_t actualValue);
	ft_api void AssertS16Equals(const int16_t expectedValue, const int16_t actualValue);
	ft_api void AssertS32Equals(const int32_t expectedValue, const int32_t actualValue);
	ft_api void AssertS64Equals(const int64_t expectedValue, const int64_t actualValue);
	ft_api void AssertSizeEquals(const size_t expectedValue, const size_t actualValue);
	ft_api void AssertFloatEquals(const float expectedValue, const float actualValue, const float tolerance = 0.0001f);
	ft_api void AssertDoubleEquals(const double expectedValue, const double actualValue, const double tolerance = 0.0001);
	ft_api void AssertPointerEquals(const void *expectedValue, const void *actualValue);
};

#define FT_ASSERT(exp) if(!(exp)) {*(int *)0 = 0;}

#define FT_EXPECTS(expectedValue, actualValue) FT_ASSERT((expectedValue) == (actualValue))
#define FT_NOT_EXPECTS(notExpectedValue, actualValue) FT_ASSERT((notExpectedValue) != (actualValue))

#define FT_IS_TRUE(value) FT_EXPECTS(true, value)
#define FT_IS_FALSE(value) FT_EXPECTS(false, value)

#define FT_IS_NULL(value) FT_EXPECTS(nullptr, value)
#define FT_IS_NOT_NULL(value) FT_NOT_EXPECTS(nullptr, value)

#endif // FT_HEADER


#if defined(FT_IMPLEMENTATION) && !defined(FT_IMPLEMENTED)
#	define FT_IMPLEMENTED

#include <string.h> // strstr
#include <stdarg.h> // va_start, va_end, va_list, va_arg
#include <stdio.h> // vsnprintf
#include <math.h> // fabs

namespace ft {
	ft_api void Line() {
		fprintf(stdout, "*******************************************************************************\n");
	}

	ft_api void Msg(const char *format, ...) {
		va_list argList;
		va_start(argList, format);
		vfprintf(stdout, format, argList);
		va_end(argList);
	}

	ft_api void AssertStringEquals(const char *expectedStr, const char *actualStr) {
		bool match = false;
		FT_NOT_EXPECTS(nullptr, expectedStr);
		FT_NOT_EXPECTS(nullptr, actualStr);
		const char *r = strstr(expectedStr, actualStr);
		FT_NOT_EXPECTS(nullptr, r);
	}

	ft_api void AssertU8Equals(const uint8_t expectedValue, const uint8_t actualValue) {
		bool passed = (expectedValue == actualValue);
		FT_IS_TRUE(passed);
	}
	ft_api void AssertS8Equals(const int8_t expectedValue, const int8_t actualValue) {
		bool passed = (expectedValue == actualValue);
		FT_IS_TRUE(passed);
	}

	ft_api void AssertU16Equals(const uint16_t expectedValue, const uint16_t actualValue) {
		bool passed = (expectedValue == actualValue);
		FT_IS_TRUE(passed);
	}
	ft_api void AssertS16Equals(const int16_t expectedValue, const int16_t actualValue) {
		bool passed = (expectedValue == actualValue);
		FT_IS_TRUE(passed);
	}

	ft_api void AssertU32Equals(const uint32_t expectedValue, const uint32_t actualValue) {
		bool passed = (expectedValue == actualValue);
		FT_IS_TRUE(passed);
	}
	ft_api void AssertS32Equals(const int32_t expectedValue, const int32_t actualValue) {
		bool passed = (expectedValue == actualValue);
		FT_IS_TRUE(passed);
	}

	ft_api void AssertU64Equals(const uint64_t expectedValue, const uint64_t actualValue) {
		bool passed = (expectedValue == actualValue);
		FT_IS_TRUE(passed);
	}
	ft_api void AssertS64Equals(const int64_t expectedValue, const int64_t actualValue) {
		bool passed = (expectedValue == actualValue);
		FT_IS_TRUE(passed);
	}

	ft_api void AssertSizeEquals(const size_t expectedValue, const size_t actualValue) {
		bool passed = (expectedValue == actualValue);
		FT_IS_TRUE(passed);
	}

	ft_api void AssertDoubleEquals(const double expectedValue, const double actualValue, const double tolerance) {
		double diff = fabs(expectedValue - actualValue);
		bool passed = diff <= tolerance;
		FT_IS_TRUE(passed);
	}

	ft_api void AssertFloatEquals(const float expectedValue, const float actualValue, const float tolerance) {
		float diff = (float)fabs(expectedValue - actualValue);
		bool passed = diff <= tolerance;
		FT_IS_TRUE(passed);
	}

	ft_api void AssertPointerEquals(const void *expectedValue, const void *actualValue) {
		bool passed = (expectedValue == actualValue);
		FT_IS_TRUE(passed);
	}
};

#endif // FT_IMPLEMENTATION
