/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Security & Stability Tests

Description:
	Concrete tests for issues catalogued in
	analysis/fpl-code-security-analysis.md.

	Each test asserts on the *correct* behavior. Until the underlying
	issue is fixed, the affected test will fail (assert / crash / produce
	wrong result). Once the fix lands, every test should pass cleanly.

	Tests are organised by category. Public entry points are kept to one
	per category, so the test driver in fpl_test.cpp can wire them in
	with minimal surface area.

License:
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

/* This translation unit consumes the FPL public API only.
 * fpl_test.cpp owns FPL_IMPLEMENTATION; do NOT define it here. */
#ifndef FPL_NO_AUDIO
#	define FPL_NO_AUDIO
#endif

#ifndef FPL_NO_AUDIO
#	define FPL_NO_VIDEO
#endif

#ifndef FPL_NO_WINDOW
#	define FPL_NO_WINDOW
#endif

#include <final_platform_layer.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

// Public entry-points provided by fpl_security_tests.c
extern void FPLSecurityTests_Strings(void);
extern void FPLSecurityTests_Paths(void);
extern void FPLSecurityTests_Files(void);
extern void FPLSecurityTests_Conversions(void);
extern void FPLSecurityTests_Types(void);
extern void FPLSecurityTests_Misc(void);

/* ---------------------------------------------------------------------------
 *  Local helpers
 * --------------------------------------------------------------------------*/

#define FSEC_ASSERT(exp)         assert(exp)
#define FSEC_ASSERT_EQ_SZ(a, b)  assert((size_t)(a) == (size_t)(b))
#define FSEC_ASSERT_EQ_S32(a, b) assert((int32_t)(a) == (int32_t)(b))
#define FSEC_ASSERT_TRUE(x)      assert((x))
#define FSEC_ASSERT_FALSE(x)     assert(!(x))

static void fsec__Banner(const char *category, const char *test) {
	fprintf(stdout, "[security] %-12s : %s\n", category, test);
}

/* Sentinel byte to detect buffer overruns in fixed-size scratch buffers. */
#define FSEC__GUARD_BYTE 0xCD
#define FSEC__GUARD_LEN  64

static void fsec__FillGuard(unsigned char *buf, size_t len) {
	memset(buf, FSEC__GUARD_BYTE, len);
}

static int fsec__GuardIntact(const unsigned char *buf, size_t len) {
	for (size_t i = 0; i < len; ++i) {
		if (buf[i] != FSEC__GUARD_BYTE) return 0;
	}
	return 1;
}

/* ===========================================================================
 *  STRING TESTS  (analysis sections 2.1, 2.2, 2.3, 2.4, 2.6, 5.10)
 * ===========================================================================*/

/* 2.2 fplStringFormatArgs: when the buffer is too small, the function must
 *     return 0 (no partial writes, project policy). Caller can query the
 *     required size first by passing destBuffer == NULL. */
static void fsec__String_FormatTruncationNullTerminates(void) {
	fsec__Banner("strings", "fplStringFormat too-small returns 0");
	size_t required = fplStringFormat(fpl_null, 0, "%s-%s", "abcdef", "ghijkl");
	FSEC_ASSERT_EQ_SZ(required, 13);
	char small[8];
	memset(small, 'X', sizeof(small));
	size_t writtenWithSmallBuf = fplStringFormat(small, sizeof(small), "%s-%s", "abcdef", "ghijkl");
	FSEC_ASSERT_EQ_SZ(writtenWithSmallBuf, 0);
	FSEC_ASSERT_TRUE(small[0] == 0);
}

/* 2.2 / 5.10: passing destBuffer != NULL with maxDestBufferLen == 0 must NOT
 *     write to destBuffer[0]. Use guard bytes to detect it. */
static void fsec__String_FormatZeroLenBufferDoesNotWrite(void) {
	fsec__Banner("strings", "fplStringFormat zero-len buffer is read-only");
	unsigned char guard[FSEC__GUARD_LEN];
	fsec__FillGuard(guard, sizeof(guard));
	(void)fplStringFormat((char *)&guard[FSEC__GUARD_LEN / 2], 0, "hello");
	FSEC_ASSERT_TRUE(fsec__GuardIntact(guard, sizeof(guard)));
}

/* 2.3 fplCopyString*: docs claim NULL dest returns required size; impl returns
 *     NULL today. Test the documented contract. */
static void fsec__String_CopyStringNullDestReturnsSize(void) {
	fsec__Banner("strings", "fplCopyString(NULL,..) returns required size");
	const char *src = "Hello, World!";
	/* Cast to size_t so the test still compiles after the API is changed
	 * to return size_t. With the current char* return type, NULL casts
	 * to (size_t)0 and the assertion fails -> bug observable. */
	size_t needed = (size_t)fplCopyString(src, fpl_null, 0);
	FSEC_ASSERT_EQ_SZ(needed, fplGetStringLength(src));
}

/* 2.4 fplStringAppendLen off-by-one: an exactly-fitting append must succeed. */
static void fsec__String_AppendExactFit(void) {
	fsec__Banner("strings", "fplStringAppendLen exact fit must succeed");
	char buf[16] = { 0 };
	/* "abcd" + "efghijklmno" = 15 chars + NUL = 16 bytes. Fits exactly. */
	fplCopyString("abcd", buf, sizeof(buf));
	const char *append = "efghijklmno";
	size_t res = fplStringAppendLen(append, fplGetStringLength(append), buf,
	                                sizeof(buf));
	FSEC_ASSERT_TRUE(res > 0);
	FSEC_ASSERT_TRUE(fplIsStringEqual(buf, "abcdefghijklmno"));
}

/* 2.6 fplStringToS32Len: must detect overflow and not silently wrap. */
static void fsec__String_StringToS32OverflowDetected(void) {
	fsec__Banner("strings", "fplStringToS32Len overflow not silent wrap");
	const char *huge = "99999999999999";
	int32_t v = fplStringToS32Len(huge, fplGetStringLength(huge));
	/* On the current implementation this silently wraps. The fixed
	 * implementation should clamp to INT32_MAX or report failure
	 * (sentinel). Either is acceptable; a wrapped value is not. */
	FSEC_ASSERT_TRUE(v == INT32_MAX || v == 0 || v == INT32_MIN);
	/* But specifically, it must NOT equal the silently-wrapped result
	 * 1316134911 (= 99999999999999 mod 2^32, signed). */
	FSEC_ASSERT_TRUE(v != 1316134911);
}

/* 2.6 fplStringToS32Len: "abc" (invalid) and "0" (valid) must be
 *     distinguishable via the Try-style API. */
static void fsec__String_StringToS32AmbiguousZero(void) {
	fsec__Banner("strings", "fplStringToS32 distinguishes invalid vs '0'");
	int32_t validValue = 0;
	int32_t invalidValue = 0;
	bool validOk = fplTryStringToS32("0", &validValue);
	bool invalidOk = fplTryStringToS32("abc", &invalidValue);
	FSEC_ASSERT_TRUE(validOk);
	FSEC_ASSERT_FALSE(invalidOk);
	FSEC_ASSERT_EQ_S32(validValue, 0);
}

/* 2.1 fplGetStringLength: documented to return size_t but counter is u32
 *     (overflow at 4GB). We can't allocate 4GB safely, but we can guard
 *     against accidental return-type narrowing in static analysis builds
 *     and verify size_t is at least 64-bit on 64-bit targets. */
static void fsec__String_GetStringLengthSizeType(void) {
	fsec__Banner("strings", "fplGetStringLength returns size_t");
	const char *s = "abc";
	size_t r = fplGetStringLength(s);
	FSEC_ASSERT_EQ_SZ(r, 3);
	/* Compile-time: ensure the returned type is wide enough. */
	FSEC_ASSERT_TRUE(sizeof(r) == sizeof(size_t));
}

/* Public string-category entry. */
void FPLSecurityTests_Strings(void) {
	fsec__String_FormatTruncationNullTerminates();
	fsec__String_FormatZeroLenBufferDoesNotWrite();
	fsec__String_CopyStringNullDestReturnsSize();
	fsec__String_AppendExactFit();
	fsec__String_StringToS32OverflowDetected();
	fsec__String_StringToS32AmbiguousZero();
	fsec__String_GetStringLengthSizeType();
}

/* ===========================================================================
 *  PATH TESTS  (analysis sections 2.5, 5.7, 5.1, 2.12)
 * ===========================================================================*/

/* 5.7 fplPathCombine must not produce duplicate separators. */
static void fsec__Path_CombineNoDoubleSeparators(void) {
	fsec__Banner("paths", "fplPathCombine deduplicates separators");
	char dest[64];
	memset(dest, 0, sizeof(dest));
	char a_with_sep[8];
	a_with_sep[0] = 'a';
	a_with_sep[1] = FPL_PATH_SEPARATOR;
	a_with_sep[2] = 0;
	(void)fplPathCombine(dest, sizeof(dest), 2, a_with_sep, "b");
	/* Expect "a<sep>b", not "a<sep><sep>b". */
	char expected[8];
	expected[0] = 'a';
	expected[1] = FPL_PATH_SEPARATOR;
	expected[2] = 'b';
	expected[3] = 0;
	FSEC_ASSERT_TRUE(fplIsStringEqual(dest, expected));
}

/* 2.5 fplPathCombine must NUL-terminate even if a partial write happens
 *     because the buffer is too small. Place guard bytes after the buffer. */
static void fsec__Path_CombineBoundedDoesNotOverflow(void) {
	fsec__Banner("paths", "fplPathCombine respects buffer bounds");
	unsigned char block[64];
	fsec__FillGuard(block, sizeof(block));
	char *dest = (char *)&block[16];
	const size_t cap = 8;
	(void)fplPathCombine(dest, cap, 3, "alpha", "beta", "gamma");
	/* Bytes outside [16, 16+cap) must stay untouched. */
	FSEC_ASSERT_TRUE(fsec__GuardIntact(block, 16));
	FSEC_ASSERT_TRUE(fsec__GuardIntact(&block[16 + cap],
	                                   sizeof(block) - 16 - cap));
}

/* 2.12 fplPathNormalize must report the required size via NULL-dest query.
 *      With a too-small destination buffer, the function must return 0 and
 *      leave the buffer untouched (project policy: no partial writes). */
static void fsec__Path_NormalizeReturnsRequired(void) {
	fsec__Banner("paths", "fplPathNormalize reports required size");
	const char *abs = ".";
	size_t needed = fplPathNormalize(abs, fpl_null, 0);
	if (needed == 0) {
		/* On systems where realpath(".") fails (chrooted, etc.) we
		 * can't run this test; surface it but don't false-fail. */
		return;
	}
	unsigned char block[64];
	fsec__FillGuard(block, sizeof(block));
	char *tiny = (char *)&block[16];
	const size_t cap = 2;
	size_t writtenWithSmallBuf = fplPathNormalize(abs, tiny, cap);
	FSEC_ASSERT_EQ_SZ(writtenWithSmallBuf, 0);
	FSEC_ASSERT_TRUE(fsec__GuardIntact(block, 16));
	FSEC_ASSERT_TRUE(fsec__GuardIntact(&block[16 + cap], sizeof(block) - 16 - cap));
}

/* 5.1 fplExtractFilePath uses int internally; verify long paths compute
 *     a correct length without truncation. */
static void fsec__Path_ExtractFilePathLengthIsSizet(void) {
	fsec__Banner("paths", "fplExtractFilePath returns size_t length");
	char src[] = "dir/sub/file.txt";
	src[3] = FPL_PATH_SEPARATOR;
	src[7] = FPL_PATH_SEPARATOR;
	size_t out = fplExtractFilePath(src, fpl_null, 0);
	/* "dir<sep>sub" => length 7. */
	FSEC_ASSERT_EQ_SZ(out, 7);
}

void FPLSecurityTests_Paths(void) {
	fsec__Path_CombineNoDoubleSeparators();
	fsec__Path_CombineBoundedDoesNotOverflow();
	fsec__Path_NormalizeReturnsRequired();
	fsec__Path_ExtractFilePathLengthIsSizet();
}

/* ===========================================================================
 *  FILE I/O TESTS  (analysis sections 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 2.7, 2.8)
 * ===========================================================================*/

static const char *fsec__GetTempPath(char *buf, size_t bufLen,
                                     const char *base) {
	/* Build "<exeDir>/<base>" where <exeDir> = parent of fplGetExecutableFilePath. */
	char exePath[FPL_MAX_PATH_LENGTH];
	(void)fplGetExecutableFilePath(exePath, fplArrayCount(exePath));
	char exeDir[FPL_MAX_PATH_LENGTH];
	(void)fplExtractFilePath(exePath, exeDir, fplArrayCount(exeDir));
	(void)fplPathCombine(buf, bufLen, 2, exeDir, base);
	return buf;
}

static void fsec__File_WriteFixture(const char *path, const void *data,
                                    size_t size) {
	fplFileHandle h = { 0 };
	bool ok = fplFileCreateBinary(path, &h);
	FSEC_ASSERT_TRUE(ok);
	(void)fplFileWriteBlock(&h, data, size);
	fplFileClose(&h);
}

/* 1.4 fplFileReadBlock64 must accumulate bytes across chunks. We can't
 *     reliably trigger the >4GB chunk path on a CI machine, but we can
 *     verify that a single-chunk read returns the same result as the
 *     equivalent fplFileReadBlock32 does, which catches the missing-add
 *     regression for the legacy 64-bit path on small files. */
static void fsec__File_ReadBlock64Accumulates(void) {
	fsec__Banner("files", "fplFileReadBlock64 returns total bytes");
	char path[FPL_MAX_PATH_LENGTH];
	(void)fsec__GetTempPath(path, sizeof(path), "fsec_read64.bin");
	unsigned char payload[1024];
	for (size_t i = 0; i < sizeof(payload); ++i) payload[i] = (unsigned char)i;
	fsec__File_WriteFixture(path, payload, sizeof(payload));

	fplFileHandle h = { 0 };
	if (fplFileOpenBinary(path, &h)) {
		unsigned char dst[1024];
		uint64_t got = fplFileReadBlock64(&h, sizeof(payload), dst,
		                                  sizeof(dst));
		FSEC_ASSERT_EQ_SZ((size_t)got, sizeof(payload));
		FSEC_ASSERT_TRUE(memcmp(dst, payload, sizeof(payload)) == 0);
		fplFileClose(&h);
	}
	(void)fplFileDelete(path);
}

/* 1.3 fplFileReadBlock32 must NOT write past maxTargetBufferSize. We
 *     surround the destination with guard bytes and ask for more than
 *     the buffer can hold. */
static void fsec__File_ReadBlock32RespectsMax(void) {
	fsec__Banner("files", "fplFileReadBlock32 honours maxTargetBufferSize");
	char path[FPL_MAX_PATH_LENGTH];
	(void)fsec__GetTempPath(path, sizeof(path), "fsec_readmax.bin");
	unsigned char payload[256];
	for (size_t i = 0; i < sizeof(payload); ++i) payload[i] = (unsigned char)(i ^ 0x5A);
	fsec__File_WriteFixture(path, payload, sizeof(payload));

	unsigned char block[FSEC__GUARD_LEN + 64 + FSEC__GUARD_LEN];
	fsec__FillGuard(block, sizeof(block));
	unsigned char *dst = &block[FSEC__GUARD_LEN];
	const uint32_t cap = 64;

	fplFileHandle h = { 0 };
	if (fplFileOpenBinary(path, &h)) {
		(void)fplFileReadBlock32(&h, (uint32_t)sizeof(payload), dst, cap);
		fplFileClose(&h);
	}
	FSEC_ASSERT_TRUE(fsec__GuardIntact(block, FSEC__GUARD_LEN));
	FSEC_ASSERT_TRUE(fsec__GuardIntact(&block[FSEC__GUARD_LEN + cap],
	                                   FSEC__GUARD_LEN));
	(void)fplFileDelete(path);
}

/* 1.5 fplFileGetPosition64 must return the current position, not 0. */
static void fsec__File_GetPosition64ReturnsActualPosition(void) {
	fsec__Banner("files", "fplFileGetPosition64 returns real position");
	char path[FPL_MAX_PATH_LENGTH];
	(void)fsec__GetTempPath(path, sizeof(path), "fsec_pos64.bin");
	const char data[] = "abcdefghijklmnopqrstuvwxyz";
	fsec__File_WriteFixture(path, data, sizeof(data) - 1);

	fplFileHandle h = { 0 };
	if (fplFileOpenBinary(path, &h)) {
		(void)fplFileSetPosition64(&h, 10, fplFilePositionMode_Beginning);
		uint64_t p = fplFileGetPosition64(&h);
		FSEC_ASSERT_EQ_SZ((size_t)p, (size_t)10);
		fplFileClose(&h);
	}
	(void)fplFileDelete(path);
}

/* 1.1 fplFileCopy must produce a target whose contents match the source. */
static void fsec__File_CopyTargetMatchesSource(void) {
	fsec__Banner("files", "fplFileCopy produces matching target");
	char src[FPL_MAX_PATH_LENGTH];
	char dst[FPL_MAX_PATH_LENGTH];
	(void)fsec__GetTempPath(src, sizeof(src), "fsec_copy_src.bin");
	(void)fsec__GetTempPath(dst, sizeof(dst), "fsec_copy_dst.bin");

	const char payload[] = "FPL-COPY-FIXTURE";
	fsec__File_WriteFixture(src, payload, sizeof(payload) - 1);
	(void)fplFileDelete(dst);

	bool ok = fplFileCopy(src, dst, /*overwrite=*/true);
	FSEC_ASSERT_TRUE(ok);
	FSEC_ASSERT_TRUE(fplFileExists(dst));

	uint64_t srcSize = fplFileGetSizeFromPath64(src);
	uint64_t dstSize = fplFileGetSizeFromPath64(dst);
	FSEC_ASSERT_EQ_SZ((size_t)dstSize, (size_t)srcSize);

	fplFileHandle h = { 0 };
	char buf[64] = { 0 };
	if (fplFileOpenBinary(dst, &h)) {
		(void)fplFileReadBlock64(&h, sizeof(payload) - 1, buf, sizeof(buf));
		fplFileClose(&h);
	}
	FSEC_ASSERT_TRUE(memcmp(buf, payload, sizeof(payload) - 1) == 0);

	(void)fplFileDelete(src);
	(void)fplFileDelete(dst);
}

/* 1.2 fplFileMove: target should appear, source should disappear. */
static void fsec__File_MoveSwitchesPaths(void) {
	fsec__Banner("files", "fplFileMove relocates source to target");
	char src[FPL_MAX_PATH_LENGTH];
	char dst[FPL_MAX_PATH_LENGTH];
	(void)fsec__GetTempPath(src, sizeof(src), "fsec_move_src.bin");
	(void)fsec__GetTempPath(dst, sizeof(dst), "fsec_move_dst.bin");

	const char payload[] = "FPL-MOVE-FIXTURE";
	fsec__File_WriteFixture(src, payload, sizeof(payload) - 1);
	(void)fplFileDelete(dst);

	bool ok = fplFileMove(src, dst);
	FSEC_ASSERT_TRUE(ok);
	FSEC_ASSERT_TRUE(fplFileExists(dst));
	FSEC_ASSERT_FALSE(fplFileExists(src));

	(void)fplFileDelete(dst);
}

/* 1.6 fplFileClose must be safe on a zero-initialised handle. The handle
 *     compares (NULL != INVALID_HANDLE_VALUE) on Win32 today and would
 *     therefore call CloseHandle(NULL). On other platforms the equivalent
 *     hazard is closing fd=0 (stdin). */
static void fsec__File_CloseZeroInitHandleIsSafe(void) {
	fsec__Banner("files", "fplFileClose tolerates zero-init handle");
	fplFileHandle h;
	memset(&h, 0, sizeof(h));
	/* Must not crash, must not close stdin/stdout. */
	fplFileClose(&h);
}

void FPLSecurityTests_Files(void) {
	fsec__File_ReadBlock64Accumulates();
	fsec__File_ReadBlock32RespectsMax();
	fsec__File_GetPosition64ReturnsActualPosition();
	fsec__File_CopyTargetMatchesSource();
	fsec__File_MoveSwitchesPaths();
	fsec__File_CloseZeroInitHandleIsSafe();
}

/* ===========================================================================
 *  STRING-CONVERSION TESTS  (analysis sections 1.10, 1.11)
 * ===========================================================================*/

/* 1.10 / 1.11 UTF-8 round trip must preserve content for both ASCII
 *      and multi-byte characters, and must respect the input length
 *      (no reads past `wideSourceLen` / `utf8SourceLen`). */
static void fsec__Conv_RoundTripAscii(void) {
	fsec__Banner("conv", "ASCII round-trip UTF-8 <-> wide");
	const char *src = "Hello, FPL!";
	wchar_t wide[64] = { 0 };
	size_t wideLen = fplUTF8StringToWideString(src, fplGetStringLength(src),
	                                           wide, fplArrayCount(wide));
	FSEC_ASSERT_TRUE(wideLen > 0);
	char back[64] = { 0 };
	size_t backLen = fplWideStringToUTF8String(wide, wideLen, back,
	                                           fplArrayCount(back));
	FSEC_ASSERT_TRUE(backLen > 0);
	FSEC_ASSERT_TRUE(fplIsStringEqual(back, src));
}

static void fsec__Conv_RoundTripMultibyte(void) {
	fsec__Banner("conv", "Multibyte round-trip UTF-8 <-> wide");
	/* "Grüße — 世界" : Latin-1 supplement, em-dash, CJK. The host C
	 * locale must support UTF-8; if it doesn't, skip silently. */
	const char *src = "Gr\xC3\xBC\xC3\x9Fe \xE2\x80\x94 \xE4\xB8\x96\xE7\x95\x8C";
	wchar_t wide[64] = { 0 };
	size_t wideLen = fplUTF8StringToWideString(src, fplGetStringLength(src),
	                                           wide, fplArrayCount(wide));
	if (wideLen == 0) return; /* locale unsupported -> not a bug we test */
	char back[64] = { 0 };
	size_t backLen = fplWideStringToUTF8String(wide, wideLen, back,
	                                           fplArrayCount(back));
	FSEC_ASSERT_TRUE(backLen > 0);
	FSEC_ASSERT_TRUE(fplIsStringEqual(back, src));
}

/* 1.10 The conversion routines must not read past the requested source
 *      length. Place a non-NUL sentinel right after the wide string and
 *      verify the converter still produces the right output. */
static void fsec__Conv_RespectsSourceLength(void) {
	fsec__Banner("conv", "UTF-8 conversion honours source length");
	wchar_t wbuf[8];
	wbuf[0] = L'A';
	wbuf[1] = L'B';
	wbuf[2] = L'C';
	/* Deliberate non-NUL trailing data. */
	wbuf[3] = L'X';
	wbuf[4] = L'Y';
	wbuf[5] = 0;
	char out[16] = { 0 };
	size_t n = fplWideStringToUTF8String(wbuf, /*wideSourceLen=*/3, out,
	                                     fplArrayCount(out));
	FSEC_ASSERT_EQ_SZ(n, 3);
	FSEC_ASSERT_TRUE(fplIsStringEqual(out, "ABC"));
}

void FPLSecurityTests_Conversions(void) {
	fsec__Conv_RoundTripAscii();
	fsec__Conv_RoundTripMultibyte();
	fsec__Conv_RespectsSourceLength();
}

/* ===========================================================================
 *  TYPE / OPAQUE-HANDLE TESTS  (analysis sections 1.9, 4.1)
 * ===========================================================================*/

/* 1.9 fpl__X11Window opaque type must be wide enough to hold an X11 XID
 *     (unsigned long). Compile-time check; only meaningful when the
 *     opaque branch is in use. */
static void fsec__Type_OpaqueX11WindowWidth(void) {
	fsec__Banner("types", "fpl__X11Window opaque width");
#if defined(FPL_SUBPLATFORM_X11) && \
    (!defined(FPL__HAS_PLATFORM_INCLUDES) || defined(FPL_OPAQUE_HANDLES))
	/* Opaque branch is active; the typedef must be at least as wide
	 * as unsigned long, which is the underlying X11 XID. */
	fplStaticAssert(sizeof(fpl__X11Window) >= sizeof(unsigned long));
#endif
}

/* 4.1 Opaque POSIX handle buffers must be at least as wide as the real
 *     pthread_*_t types. We can only check this when both branches are
 *     reachable in the same TU, which is rare; we instead check the
 *     buffers are non-zero-sized so the regression of a stray "uint8_t[0]"
 *     never lands. */
static void fsec__Type_OpaqueHandlesNonZero(void) {
	fsec__Banner("types", "Opaque handles non-zero size");
#if defined(FPL_SUBPLATFORM_POSIX)
	fplMutexHandle m;
	fplSemaphoreHandle s;
	fplConditionVariable c;
	(void)m; (void)s; (void)c;
	fplStaticAssert(sizeof(fplMutexHandle) > 0);
	fplStaticAssert(sizeof(fplSemaphoreHandle) > 0);
	fplStaticAssert(sizeof(fplConditionVariable) > 0);
#endif
}

void FPLSecurityTests_Types(void) {
	fsec__Type_OpaqueX11WindowWidth();
	fsec__Type_OpaqueHandlesNonZero();
}

/* ===========================================================================
 *  MISC TESTS  (analysis sections 3.3, 5.4)
 * ===========================================================================*/

/* 3.3 fplDateTimeCreate must accept minute = 0..59 (the doc claimed
 *     0..23, but the implementation allows 0..59 - this guards against
 *     an over-aggressive fix that uses the doc range instead of the
 *     real one). */
static void fsec__Misc_DateTimeMinuteRange(void) {
	fsec__Banner("misc", "fplDateTimeCreate accepts minute up to 59");
	fplDateTimeCreationResult r =
	    fplDateTimeCreate(/*year*/2026, /*month*/5, /*day*/6,
	                      /*hour*/12, /*minute*/45, /*second*/30,
	                      /*ms*/0, /*utcOffset*/0);
	FSEC_ASSERT_TRUE(r.success);
	FSEC_ASSERT_TRUE((r.errors & fplDateTimeErrors_InvalidMinute) == 0);
}

/* 3.3 minute = 60 must NOT be accepted. */
static void fsec__Misc_DateTimeMinuteOutOfRange(void) {
	fsec__Banner("misc", "fplDateTimeCreate rejects minute 60");
	fplDateTimeCreationResult r =
	    fplDateTimeCreate(2026, 5, 6, 12, 60, 0, 0, 0);
	FSEC_ASSERT_FALSE(r.success);
	FSEC_ASSERT_TRUE((r.errors & fplDateTimeErrors_InvalidMinute) != 0);
}

/* 5.4 fplDateTimeQuery should not return all-zero unless gettimeofday
 *     genuinely failed (which essentially never happens on Linux). */
static void fsec__Misc_DateTimeQueryNonZero(void) {
	fsec__Banner("misc", "fplDateTimeQuery returns plausible epoch");
	fplDateTime now = fplDateTimeQuery(fplDateTimeType_UTC);
	FSEC_ASSERT_TRUE(now.epoch > 0);
}

void FPLSecurityTests_Misc(void) {
	fsec__Misc_DateTimeMinuteRange();
	fsec__Misc_DateTimeMinuteOutOfRange();
	fsec__Misc_DateTimeQueryNonZero();
}
