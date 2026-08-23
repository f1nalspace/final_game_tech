/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Test

Description:
	This demo is used to test all the things. It is basically a unit-test.

Requirements:
	- C99 Compiler
	- Final Platform Layer

Author:
	Torsten Spaete

Changelog:
	## 2026-08-23
	- Added the process tests from process_tests.c, wired in through TestProcess()
	- Added the child mode detection in main(), so the test executable can start itself as the child program for the process tests

	## 2026-06-17
	- Adjusted tests to match feature that unlimited threads can be created

	## 2026-05-06
	- Converted from C++ to C99
	- Removed AssertEquals<T> templates in favor of type-specific functions
	- Wired in fpl_security_tests.c entry points
	- Fixed ThreadLimits Test first test fails, because the thread was finished too fast

	## 2025-03-30
	- Test available/used thread count

	## 2021-09-07
	- Added thread limit tests

	## 2019-06-17
	- Reflect api changes in FPL 0.9.4

	## 2019-05-30
	- Fixed os version was not properly printed (%d instead of %s)
	- Rearranged code a bit

	## 2018-10-22
	- Reflect api changes in FPL 0.9.3

	## 2018-09-24
	- Reflect api changes in FPL 0.9.2
	- Transition of test framework from C++ to C99

	## 2018-08-10
	- Correction for api change in fplPlatformInit

	## 2018-08-09
	- Correction for api change in fplMemoryInfo
	- Added a new more strings tests

	## 2018-06-29
	- Added condition-variable tests

	## 2018-05-15:
	- Corrected for api change in FPL v0.8.1+
	- Added semaphores sync test

	## 2018-05-10:
	- Small bugfixes

	## 2018-04-27:
	- Added wrap test for unsigned integers for AtomicExchange

	## 2018-04-23:
	- Initial creation of this description block
	- Forced Visual-Studio-Project to compile in C++ always

License:
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#ifndef FPL_IMPLEMENTATION
#	define FPL_IMPLEMENTATION
#endif

#ifndef FPL_NO_AUDIO
#	define FPL_NO_AUDIO
#endif

#ifndef FPL_NO_VIDEO
#	define FPL_NO_VIDEO
#endif

#ifndef FPL_NO_WINDOW
#	define FPL_NO_WINDOW
#endif

#ifndef FPL_IMPLEMENTATION
#	define FPL_IMPLEMENTATION
#endif

#include <final_platform_layer.h>

#ifndef FT_IMPLEMENTATION
#	define FT_IMPLEMENTATION
#endif
#include "final_test.h"

#include <math.h> // sqrt

#include "security_tests.c"
#include "process_tests.c"

static void TestColdInit(void) {
	ftMsg("Test Cold-Initialize of InitPlatform\n");
	{
		size_t errorCount = fplGetErrorCount();
		ftAssertSizeEquals(0, errorCount);
		bool inited = fplPlatformInit(fplInitFlags_None, fpl_null);
		ftAssert(inited);
		fplPlatformResultType resultType = fplGetPlatformResult();
		ftAssert(resultType == fplPlatformResultType_Success);
		const char *errorStr = fplGetLastError();
		ftAssertStringEquals("", errorStr);
		fplPlatformRelease();
	}
}

static void TestInit(void) {
	ftMsg("Test InitPlatform with All init flags\n");
	{
		fplErrorsClear();
		bool inited = fplPlatformInit(fplInitFlags_All, fpl_null);
		ftAssert(inited);
		fplPlatformResultType resultType = fplGetPlatformResult();
		ftAssert(resultType == fplPlatformResultType_Success);
		const char *errorStr = fplGetLastError();
		ftAssertStringEquals("", errorStr);
		fplPlatformRelease();
	}
	ftMsg("Test InitPlatform with None init flags\n");
	{
		fplErrorsClear();
		bool inited = fplPlatformInit(fplInitFlags_None, fpl_null);
		ftAssert(inited);
		fplPlatformResultType resultType = fplGetPlatformResult();
		ftAssert(resultType == fplPlatformResultType_Success);
		const fplSettings *settings = fplGetCurrentSettings();
		ftIsNotNull(settings);
		const char *errorStr = fplGetLastError();
		ftAssertStringEquals("", errorStr);
		fplPlatformRelease();
	}
	ftMsg("Test fplGetCurrentSettings in non-initialized state\n");
	{
		ftIsFalse(fpl__global__InitState.isInitialized);
		fplErrorsClear();
		const fplSettings *settings = fplGetCurrentSettings();
		ftIsNull(settings);
		size_t errorCount = fplGetErrorCount();
		ftAssertSizeEquals(1, errorCount);
		const char *errorStr = fplGetLastError();
		ftAssertStringNotEquals("", errorStr);
	}
}

static void TestOSInfos(void) {
	ftMsg("Get Platform Type:\n");
	{
		fplPlatformType platType = fplGetPlatformType();
		ftAssert(fplPlatformType_Unknown != platType);
		fplConsoleFormatOut("\tPlatform: %s\n", fplGetPlatformName(platType));
	}
	ftMsg("Get OS Type:\n");
	{
		fplOSVersionInfos osInfos = fplZeroInit;
		bool r = fplOSGetVersionInfos(&osInfos);
		ftIsTrue(r);
		fplConsoleFormatOut("\tName: %s\n", osInfos.osName);
		fplConsoleFormatOut("\tVersion: %s.%s.%s.%s\n",
			osInfos.osVersion.version.parts.major,
			osInfos.osVersion.version.parts.minor,
			osInfos.osVersion.version.parts.fix,
			osInfos.osVersion.version.parts.build);
		fplConsoleFormatOut("\tDistribution Name: %s\n", osInfos.distributionName);
		fplConsoleFormatOut("\tDistribution Version: %s.%s.%s.%s\n",
			osInfos.distributionVersion.version.parts.major,
			osInfos.distributionVersion.version.parts.minor,
			osInfos.distributionVersion.version.parts.fix,
			osInfos.distributionVersion.version.parts.build);
	}
	ftMsg("Get Session User name:\n");
	{
		char nameBuffer[256] = fplZeroInit;
		bool r = fplSessionGetUsername(nameBuffer, fplArrayCount(nameBuffer));
		ftIsTrue(r);
		fplConsoleFormatOut("\tCurrent Username: %s\n", nameBuffer);
	}
}

static void TestSizes(void) {
	// @NOTE(final): This may be pretty useless, because stdint.h guarantees the size
	ftAssertSizeEquals(1, sizeof(uint8_t));
	ftAssertSizeEquals(1, sizeof(int8_t));
	ftAssertSizeEquals(2, sizeof(uint16_t));
	ftAssertSizeEquals(2, sizeof(int16_t));
	ftAssertSizeEquals(4, sizeof(uint32_t));
	ftAssertSizeEquals(4, sizeof(int32_t));
	ftAssertSizeEquals(8, sizeof(uint64_t));
	ftAssertSizeEquals(8, sizeof(int64_t));
#if defined(FT_ARCH_X64)
	ftAssertSizeEquals(8, sizeof(intptr_t));
	ftAssertSizeEquals(8, sizeof(uintptr_t));
	ftAssertSizeEquals(8, sizeof(size_t));
#else
	ftAssertSizeEquals(4, sizeof(intptr_t));
	ftAssertSizeEquals(4, sizeof(uintptr_t));
	ftAssertSizeEquals(4, sizeof(size_t));
#endif
}

// File-scope structs for fplOffsetOf / fplMin / fplMax tests (C99 has no
// templated/local struct types, so we declare these globally and reuse them).
struct fplAlignAs(4) MacrosAlign4HiLo {
	uint64_t a;
	uint32_t b;
	uint16_t c;
	uint8_t d;
};
typedef struct MacrosAlign4HiLo MacrosAlign4HiLo;

struct fplAlignAs(4) MacrosAlign4LoHi {
	uint8_t a;
	uint16_t b;
	uint32_t c;
	uint64_t d;
};
typedef struct MacrosAlign4LoHi MacrosAlign4LoHi;

struct fplAlignAs(8) MacrosAlign8LoHi {
	uint8_t a;
	uint16_t b;
	uint8_t c[3];
	uint64_t d;
};
typedef struct MacrosAlign8LoHi MacrosAlign8LoHi;

struct MacrosIntPair {
	int a;
	int b;
};

struct MacrosFloatPair {
	float a;
	float b;
};

static void TestMacros(void) {
	//
	// fplArrayCount
	//
	ftMsg("[fplArrayCount] Test static char array\n");
	{
		char staticArray[137] = fplZeroInit;
		uint32_t actual = fplArrayCount(staticArray);
		ftAssertU32Equals(137, actual);
	}
	ftMsg("[fplArrayCount] Test static int array\n");
	{
		int staticArray[349] = fplZeroInit;
		uint32_t actual = fplArrayCount(staticArray);
		ftAssertU32Equals(349, actual);
	}
	ftMsg("[fplArrayCount] Test static bool array\n");
	{
		bool staticArray[961] = fplZeroInit;
		uint32_t actual = fplArrayCount(staticArray);
		ftAssertU32Equals(961, actual);
	}
	ftMsg("[fplArrayCount] Test static void pointer array\n");
	{
		void *staticArray[35] = fplZeroInit;
		uint32_t actual = fplArrayCount(staticArray);
		ftAssertU32Equals(35, actual);
	}

	// @NOTE(final): We now use _countof() or ARRAY_SIZE() so it is expected to produce a compile error when passing a raw pointer to it
#if defined(FPL__NO_ARRAYCOUNT_VALIDATION)
	ftMsg("[fplArrayCount] Test fpl_null\n");
	{
		int *emptyArray = fpl_null;
		uint32_t actual = fplArrayCount(emptyArray);
		uint32_t expected = sizeof(int *) / sizeof(int);
		ftAssertU32Equals(expected, actual);
	}
	ftMsg("[fplArrayCount] Test pointer from references static array\n");
	{
		int staticArray[3] = fplZeroInit;
		int *refArray = &staticArray[0];
		uint32_t actual = fplArrayCount(refArray);
		uint32_t expected = sizeof(int *) / sizeof(int);
		ftAssertU32Equals(expected, actual);
	}
#endif

	//
	// fplOffsetOf
	//
	ftMsg("[fplOffsetOf] Test alignment of 4 (High to low)\n");
	{
		ftAssertSizeEquals(0, fplOffsetOf(struct MacrosAlign4HiLo, a));
		ftAssertSizeEquals(8, fplOffsetOf(struct MacrosAlign4HiLo, b));
		ftAssertSizeEquals(12, fplOffsetOf(struct MacrosAlign4HiLo, c));
		ftAssertSizeEquals(14, fplOffsetOf(struct MacrosAlign4HiLo, d));
	}

	ftMsg("[fplOffsetOf] Test alignment of 4 (Low to High)\n");
	{
		ftAssertSizeEquals(0, fplOffsetOf(struct MacrosAlign4LoHi, a));
		ftAssertSizeEquals(2, fplOffsetOf(struct MacrosAlign4LoHi, b));
		ftAssertSizeEquals(4, fplOffsetOf(struct MacrosAlign4LoHi, c));
		ftAssertSizeEquals(8, fplOffsetOf(struct MacrosAlign4LoHi, d));
	}

	ftMsg("[fplOffsetOf] Test alignment of 8 (Low to High)\n");
	{
		ftAssertSizeEquals(0, fplOffsetOf(struct MacrosAlign8LoHi, a));
		ftAssertSizeEquals(2, fplOffsetOf(struct MacrosAlign8LoHi, b));
		ftAssertSizeEquals(4, fplOffsetOf(struct MacrosAlign8LoHi, c));
		ftAssertSizeEquals(8, fplOffsetOf(struct MacrosAlign8LoHi, d));
	}

	//
	// fplMin/fplMax
	//
	ftMsg("[fplMin] Test integers\n");
	{
		ftAssertS32Equals(3, fplMin(3, 7));
		ftAssertS32Equals(3, fplMin(7, 3));
		ftAssertS32Equals(-7, fplMin(-7, -3));
		ftAssertS32Equals(-7, fplMin(-3, -7));
		struct MacrosIntPair instance = { 3, 7 };
		struct MacrosIntPair *instancePtr = &instance;
		ftAssertS32Equals(3, fplMin(instancePtr->a, instancePtr->b));
	}
	ftMsg("[fplMin] Test floats\n");
	{
		ftAssertFloatEqualsDefault(3.0f, fplMin(3.0f, 7.0f));
		ftAssertFloatEqualsDefault(3.0f, fplMin(7.0f, 3.0f));
		ftAssertFloatEqualsDefault(-7.0f, fplMin(-7.0f, -3.0f));
		ftAssertFloatEqualsDefault(-7.0f, fplMin(-3.0f, -7.0f));
		struct MacrosFloatPair instance = { 3.0f, 7.0f };
		struct MacrosFloatPair *instancePtr = &instance;
		ftAssertFloatEqualsDefault(3.0f, fplMin(instancePtr->a, instancePtr->b));
	}
	ftMsg("[fplMax] Test integers\n");
	{
		ftAssertS32Equals(7, fplMax(3, 7));
		ftAssertS32Equals(7, fplMax(7, 3));
		ftAssertS32Equals(-3, fplMax(-3, -7));
		ftAssertS32Equals(-3, fplMax(-7, -3));
		struct MacrosIntPair instance = { 3, 7 };
		struct MacrosIntPair *instancePtr = &instance;
		ftAssertS32Equals(7, fplMax(instancePtr->a, instancePtr->b));
	}
	ftMsg("[fplMax] Test floats\n");
	{
		ftAssertFloatEqualsDefault(7.0f, fplMax(3.0f, 7.0f));
		ftAssertFloatEqualsDefault(7.0f, fplMax(7.0f, 3.0f));
		ftAssertFloatEqualsDefault(-3.0f, fplMax(-3.0f, -7.0f));
		ftAssertFloatEqualsDefault(-3.0f, fplMax(-7.0f, -3.0f));
		struct MacrosFloatPair instance = { 3.0f, 7.0f };
		struct MacrosFloatPair *instancePtr = &instance;
		ftAssertFloatEqualsDefault(7.0f, fplMax(instancePtr->a, instancePtr->b));
	}

	//
	// fplKiloBytes, fplMegaBytes, ...
	//
	{
		ftMsg("[FPL_KILOBYTES] Test 0 KB \n");
		ftAssertSizeEquals(0, fplKiloBytes(0));
		ftMsg("[FPL_KILOBYTES] Test 8 KB \n");
		ftAssertSizeEquals(8192, fplKiloBytes(8));
		ftMsg("[FPL_MEGABYTES] Test 0 MB \n");
		ftAssertSizeEquals(0, fplMegaBytes(0));
		ftMsg("[FPL_MEGABYTES] Test 8 MB \n");
		ftAssertSizeEquals(8388608, fplMegaBytes(8));
		ftMsg("[FPL_GIGABYTES] Test 0 GB \n");
		ftAssertSizeEquals(0, fplGigaBytes(0));
		ftMsg("[FPL_GIGABYTES] Test 1 GB \n");
		ftAssertSizeEquals(1073741824, fplGigaBytes(1));
#if defined(FT_ARCH_X64)
		ftMsg("[FPL_GIGABYTES] Test 4 GB \n");
		ftAssertSizeEquals(4294967296, fplGigaBytes(4));
		ftMsg("[FPL_TERABYTES] Test 0 TB \n");
		ftAssertSizeEquals(0, fplTeraBytes(0));
		ftMsg("[FPL_TERABYTES] Test 2 TB \n");
		ftAssertSizeEquals(2199023255552, fplTeraBytes(2));
#endif
	}
}

static void TestMemory(void) {
	ftMsg("Test normal allocation and deallocation\n");
	{
		size_t memSize = fplKiloBytes(42);
		uint8_t *mem = (uint8_t *)fplMemoryAllocate(memSize);
		for (size_t i = 0; i < memSize; ++i) {
			uint8_t value = *mem++;
			ftAssertU8Equals(0, value);
		}
		fplMemoryFree(mem);
	}
	{
		size_t memSize = fplMegaBytes(512);
		void *mem = fplMemoryAllocate(memSize);
		ftIsNotNull(mem);
		fplMemoryFree(mem);
	}

	ftMsg("Test aligned allocation and deallocation\n");
	{
		size_t memSize = fplKiloBytes(42);
		uint8_t *mem = (uint8_t *)fplMemoryAlignedAllocate(memSize, 16);
		for (size_t i = 0; i < memSize; ++i) {
			uint8_t value = *(mem + i);
			ftAssertU8Equals(0, value);
		}
		fplMemoryAlignedFree(mem);
	}
	{
		size_t memSize = fplMegaBytes(512);
		void *mem = fplMemoryAlignedAllocate(memSize, 16);
		ftIsNotNull(mem);
		fplMemoryAlignedFree(mem);
	}

	ftMsg("Test memory clear\n");
	{
		size_t memSize = 100;
		uint8_t *mem = (uint8_t *)fplMemoryAllocate(memSize);
		for (size_t i = 0; i < memSize; ++i) {
			mem[i] = (uint8_t)i; // Dont care about wrap
		}
		fplMemorySet(mem, 0, memSize);
		for (size_t i = 0; i < memSize; ++i) {
			uint8_t value = *mem++;
			ftAssertU8Equals(0, value);
		}
		fplMemoryFree(mem);
	}

	ftMsg("Test memory set\n");
	{
		size_t memSize = 100;
		uint8_t *mem = (uint8_t *)fplMemoryAllocate(memSize);
		for (size_t i = 0; i < memSize; ++i) {
			mem[i] = (uint8_t)i; // Dont care about wrap
		}
		fplMemorySet(mem, 128, memSize);
		for (size_t i = 0; i < memSize; ++i) {
			uint8_t value = *mem++;
			ftAssertU8Equals(128, value);
		}
		fplMemoryFree(mem);
	}
}

static void TestPaths(void) {
	if (fplPlatformInit(fplInitFlags_None, fpl_null)) {

		char homePathBuffer[1024] = fplZeroInit;
		fplGetHomePath(homePathBuffer, fplArrayCount(homePathBuffer));
		ftMsg("Home Path:\n%s\n", homePathBuffer);

		char exeFilePathBuffer[1024] = fplZeroInit;
		fplGetExecutableFilePath(exeFilePathBuffer, fplArrayCount(exeFilePathBuffer));
		ftMsg("Executable file Path:\n%s\n", exeFilePathBuffer);

		char extractedPathBuffer[1024] = fplZeroInit;
		fplExtractFilePath(exeFilePathBuffer, extractedPathBuffer, fplArrayCount(extractedPathBuffer));
		ftMsg("Extracted path:\n%s\n", extractedPathBuffer);

		const char *exeFileName = fplExtractFileName(exeFilePathBuffer);
		ftMsg("Extracted filename:\n%s\n", exeFileName);

		const char *exeFileExt = fplExtractFileExtension(exeFilePathBuffer);
		ftMsg("Extracted extension:\n%s\n", exeFileExt);

		char combinedPathBuffer[1024 * 10] = fplZeroInit;
		fplPathCombine(combinedPathBuffer, fplArrayCount(combinedPathBuffer), 4, "Hallo", "Welt", "der", "Programmierer");
		ftMsg("Combined path:\n%s\n", combinedPathBuffer);

		char changedFileExtBuffer[1024] = fplZeroInit;
		fplChangeFileExtension(exeFilePathBuffer, ".obj", changedFileExtBuffer, fplArrayCount(changedFileExtBuffer));
		ftMsg("Changed file ext 1:\n%s\n", changedFileExtBuffer);
		fplChangeFileExtension(exeFileName, ".obj", changedFileExtBuffer, fplArrayCount(changedFileExtBuffer));
		ftMsg("Changed file ext 2:\n%s\n", changedFileExtBuffer);
		fplChangeFileExtension(".dll", ".obj", changedFileExtBuffer, fplArrayCount(changedFileExtBuffer));
		ftMsg("Changed file ext 3:\n%s\n", changedFileExtBuffer);
		fplChangeFileExtension("", ".obj", changedFileExtBuffer, fplArrayCount(changedFileExtBuffer));
		ftMsg("Changed file ext 4:\n%s\n", changedFileExtBuffer);
		fplChangeFileExtension(".dll", "", changedFileExtBuffer, fplArrayCount(changedFileExtBuffer));
		ftMsg("Changed file ext 5:\n%s\n", changedFileExtBuffer);
		fplChangeFileExtension("", "", changedFileExtBuffer, fplArrayCount(changedFileExtBuffer));
		ftMsg("Changed file ext 5:\n%s\n", changedFileExtBuffer);

		fplPlatformRelease();
	}
}


static void TestHardware(void) {
	char cpuNameBuffer[1024] = fplZeroInit;
	fplCPUGetName(cpuNameBuffer, fplArrayCount(cpuNameBuffer));
	ftMsg("Processor name: %s\n", cpuNameBuffer);

	size_t coreCount = fplCPUGetCoreCount();
	ftAssert(coreCount > 0);
	ftMsg("Processor cores: %zu\n", coreCount);

	fplCPUCapabilities cpuCaps = fplZeroInit;
	fplCPUGetCapabilities(&cpuCaps);
	const char *cpuTypeName = fplGetCPUCapabilitiesTypeName(cpuCaps.type);
	ftMsg("Processor capabilities (%s):\n", cpuTypeName);
	if (cpuCaps.type == fplCPUCapabilitiesType_X86) {
		ftMsg("\tMMX: %s\n", (cpuCaps.x86.hasMMX ? "yes" : "no"));
		ftMsg("\tSSE: %s\n", (cpuCaps.x86.hasSSE ? "yes" : "no"));
		ftMsg("\tSSE2: %s\n", (cpuCaps.x86.hasSSE2 ? "yes" : "no"));
		ftMsg("\tSSE3: %s\n", (cpuCaps.x86.hasSSE3 ? "yes" : "no"));
		ftMsg("\tSSSE3: %s\n", (cpuCaps.x86.hasSSSE3 ? "yes" : "no"));
		ftMsg("\tSSE4.1: %s\n", (cpuCaps.x86.hasSSE4_1 ? "yes" : "no"));
		ftMsg("\tSSE4.2: %s\n", (cpuCaps.x86.hasSSE4_2 ? "yes" : "no"));
		ftMsg("\tAVX: %s\n", (cpuCaps.x86.hasAVX ? "yes" : "no"));
		ftMsg("\tAVX2: %s\n", (cpuCaps.x86.hasAVX2 ? "yes" : "no"));
		ftMsg("\tAVX512: %s\n", (cpuCaps.x86.hasAVX512 ? "yes" : "no"));
		ftMsg("\tFMA3: %s\n", (cpuCaps.x86.hasFMA3 ? "yes" : "no"));
		ftMsg("\tEM64T: %s\n", (cpuCaps.x86.hasEM64T ? "yes" : "no"));
		ftMsg("\tAES-NI: %s\n", (cpuCaps.x86.hasAES_NI ? "yes" : "no"));
		ftMsg("\tSHA: %s\n", (cpuCaps.x86.hasSHA ? "yes" : "no"));
		ftMsg("\tBMI1: %s\n", (cpuCaps.x86.hasBMI1 ? "yes" : "no"));
		ftMsg("\tBMI2: %s\n", (cpuCaps.x86.hasBMI2 ? "yes" : "no"));
		ftMsg("\tADX: %s\n", (cpuCaps.x86.hasADX ? "yes" : "no"));
		ftMsg("\tF16C: %s\n", (cpuCaps.x86.hasF16C ? "yes" : "no"));
	} else if (cpuCaps.type == fplCPUCapabilitiesType_ARM) {
		ftMsg("\tNeon: %s\n", (cpuCaps.arm.hasNEON ? "yes" : "no"));
		ftMsg("\tAES: %s\n", (cpuCaps.arm.hasAES ? "yes" : "no"));
		ftMsg("\tSHA1: %s\n", (cpuCaps.arm.hasSHA1 ? "yes" : "no"));
		ftMsg("\tSHA2: %s\n", (cpuCaps.arm.hasSHA2 ? "yes" : "no"));
		ftMsg("\tCRC32: %s\n", (cpuCaps.arm.hasCRC32 ? "yes" : "no"));
		ftMsg("\tPMULL: %s\n", (cpuCaps.arm.hasPMULL ? "yes" : "no"));
	}

	fplMemoryInfos memInfos = fplZeroInit;
	fplMemoryGetUsage(&memInfos);
	ftMsg("Installed physical memory (bytes): %llu\n", (unsigned long long)memInfos.totalPhysicalSize);
	ftMsg("Total physical memory (bytes): %llu\n", (unsigned long long)memInfos.totalPhysicalSize);
	ftMsg("Available physical memory (bytes): %llu\n", (unsigned long long)memInfos.freePhysicalSize);
	ftMsg("Total cache memory (bytes): %llu\n", (unsigned long long)memInfos.totalCacheSize);
	ftMsg("Available cache memory (bytes): %llu\n", (unsigned long long)memInfos.freeCacheSize);
	ftMsg("Page size (bytes): %llu\n", (unsigned long long)memInfos.pageSize);
	ftMsg("Total number of memory pages: %llu\n", (unsigned long long)memInfos.totalPageCount);
	ftMsg("Available number memory pages: %llu\n", (unsigned long long)memInfos.freePageCount);

	ftMsg("RDTSC:\n");
	double tmp = 1.0;
	for (int i = 0; i < 1000; ++i) {
		tmp *= 2 + 1;
		tmp += (double)i * 400;
		tmp = sqrt(tmp);
		const uint64_t cycles = fplCPURDTSC();
		ftMsg("\tRun[%d]: %f, %llu\n", i, tmp, (unsigned long long)cycles);
	}

	fplCPUArchType archType = fplCPUGetArchitecture();
	const char *archStr = fplCPUGetArchName(archType);
	ftMsg("Processor archicture: %s\n", archStr);
}

static void EmptyThreadproc(const fplThreadHandle *context, void *data) {
	(void)context;
	(void)data;
}

typedef struct ThreadData {
	fplThreadHandle *thread;
	int num;
	int sleepFor;
} ThreadData;

static void SingleThreadProc(const fplThreadHandle *context, void *data) {
	(void)context;
	ThreadData *d = (ThreadData *)data;
	ftMsg("Sleep in thread %d for %d ms\n", d->num, d->sleepFor);
	fplThreadSleep(d->sleepFor);
}

static void SimpleMultiThreadTest(const size_t threadCount) {
	ftLine();
	ThreadData threadData[FPL_MAX_THREAD_COUNT] = fplZeroInit;
	for (size_t threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
		threadData[threadIndex].num = (int)(threadIndex + 1);
		threadData[threadIndex].sleepFor = (int)(1 + threadIndex) * 500;
	}
	ftMsg("Start %zu threads\n", threadCount);
	for (size_t threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
		threadData[threadIndex].thread = fplThreadCreate(SingleThreadProc, &threadData[threadIndex]);
	}
	ftMsg("Wait all %zu threads for exit\n", threadCount);
	fplThreadWaitForAll(&threadData[0].thread, threadCount, sizeof(ThreadData), FPL_TIMEOUT_INFINITE);
	ftMsg("All %zu threads are done\n", threadCount);

	ftMsg("Terminate %zu threads\n", threadCount);
	for (size_t threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
		ftAssertS32Equals((int32_t)fplThreadState_Stopped, (int32_t)threadData[threadIndex].thread->currentState);
		fplThreadTerminate(threadData[threadIndex].thread);
	}
}

typedef struct MutableThreadData {
	fplSemaphoreHandle semaphore;
	volatile int32_t value;
} MutableThreadData;

typedef struct WriteThreadData {
	ThreadData base;
	MutableThreadData *data;
	int32_t valueToWrite;
} WriteThreadData;

typedef struct ReadThreadData {
	ThreadData base;
	MutableThreadData *data;
	int32_t expectedValue;
} ReadThreadData;

static void WriteDataThreadProc(const fplThreadHandle *context, void *data) {
	(void)context;
	WriteThreadData *d = (WriteThreadData *)data;
	ftMsg("Sleep in thread %d for %d ms\n", d->base.num, d->base.sleepFor);
	fplThreadSleep(d->base.sleepFor);
	fplAtomicStoreS32(&d->data->value, d->valueToWrite);
}

static void ReadDataThreadProc(const fplThreadHandle *context, void *data) {
	(void)context;
	ReadThreadData *d = (ReadThreadData *)data;
	ftMsg("Sleep in thread %d for %d ms\n", d->base.num, d->base.sleepFor);
	fplThreadSleep(d->base.sleepFor);
	int32_t actualValue = fplAtomicLoadS32(&d->data->value);
	ftAssertS32Equals(d->expectedValue, actualValue);
}

static void SyncThreadsTestAtomics(void) {
	ftLine();
	ftMsg("Sync test for 1 reader and 1 writer using atomics\n");
	{
		MutableThreadData mutableData = fplZeroInit;
		mutableData.value = 0;

		ReadThreadData readData = fplZeroInit;
		readData.base.num = 2;
		readData.base.sleepFor = 5000;
		readData.data = &mutableData;
		readData.expectedValue = 42;

		WriteThreadData writeData = fplZeroInit;
		writeData.base.num = 1;
		writeData.base.sleepFor = 3000;
		writeData.data = &mutableData;
		writeData.valueToWrite = 42;

		fplThreadHandle *threads[2];
		uint32_t threadCount = fplArrayCount(threads);

		ftMsg("Start %u threads\n", threadCount);
		threads[0] = fplThreadCreate(ReadDataThreadProc, &readData);
		threads[1] = fplThreadCreate(WriteDataThreadProc, &writeData);

		ftMsg("Wait for %u threads to exit\n", threadCount);
		fplThreadWaitForAll(threads, threadCount, sizeof(fplThreadHandle *), FPL_TIMEOUT_INFINITE);

		ftMsg("Release resources for %u threads\n", threadCount);
		for (uint32_t index = 0; index < threadCount; ++index) {
			ftAssertS32Equals((int32_t)fplThreadState_Stopped, (int32_t)threads[index]->currentState);
			fplThreadTerminate(threads[index]);
		}
	}
}

static void WriteDataSemaphoreThreadProc(const fplThreadHandle *context, void *data) {
	(void)context;
	WriteThreadData *d = (WriteThreadData *)data;
	ftMsg("Sleep in thread %d for %d ms\n", d->base.num, d->base.sleepFor);
	fplThreadSleep(d->base.sleepFor);
	ftMsg("Wait for semaphore in thread %d\n", d->base.num);
	fplSemaphoreWait(&d->data->semaphore, FPL_TIMEOUT_INFINITE);
	int32_t v = d->data->value;
	if (d->base.num % 2 == 0) {
		v--;
	} else {
		v++;
	}
	d->data->value = v;
	fplSemaphoreRelease(&d->data->semaphore);
}

static void SyncThreadsTestSemaphores(const size_t numWriters) {
	ftIsTrue(numWriters >= 2);

	ftLine();
	ftMsg("Sync test for %zu writers using semaphores\n", numWriters);
	{
		MutableThreadData mutableData = fplZeroInit;
		uint32_t initialValue = (uint32_t)numWriters - 1;
		ftIsTrue(fplSemaphoreInit(&mutableData.semaphore, initialValue));
		mutableData.value = 0;

		WriteThreadData writeDatas[FPL_MAX_THREAD_COUNT] = fplZeroInit;
		ftMsg("Start %zu threads\n", numWriters);
		for (uint32_t i = 0; i < numWriters; ++i) {
			writeDatas[i].base.num = i + 1;
			writeDatas[i].base.sleepFor = 3000;
			writeDatas[i].data = &mutableData;
			writeDatas[i].base.thread = fplThreadCreate(WriteDataSemaphoreThreadProc, &writeDatas[i]);
		}

		ftMsg("Wait for %zu threads to exit\n", numWriters);
		// @TODO(final): Use stride of WriteThreadData instead
		fplThreadWaitForAll(&writeDatas[0].base.thread, numWriters, sizeof(WriteThreadData), FPL_TIMEOUT_INFINITE);
		int32_t expectedValue = (numWriters % 2 == 0) ? 0 : 1;
		ftAssertS32Equals(expectedValue, mutableData.value);

		ftMsg("Release resources for %zu threads\n", numWriters);
		for (uint32_t index = 0; index < numWriters; ++index) {
			ftAssertS32Equals((int32_t)fplThreadState_Stopped, (int32_t)writeDatas[index].base.thread->currentState);
			fplThreadTerminate(writeDatas[index].base.thread);
		}
		fplSemaphoreDestroy(&mutableData.semaphore);
	}
}

typedef enum ConditionTestType {
	ConditionTestType_Signal,
	ConditionTestType_ConditionSignal
} ConditionTestType;

typedef struct SlaveThreadData {
	ThreadData base;
	fplSignalHandle signal;
	fplConditionVariable condition;
	fplMutexHandle mutex;
	ConditionTestType testType;
	bool isSuccess;
} SlaveThreadData;

typedef struct MasterThreadData {
	ThreadData base;
	SlaveThreadData *slaveThreads;
	uint32_t slaveCount;
	ConditionTestType testType;
} MasterThreadData;

static void ThreadSlaveProc(const fplThreadHandle *context, void *data) {
	(void)context;
	SlaveThreadData *d = (SlaveThreadData *)data;

	if (d->testType == ConditionTestType_Signal) {
		ftMsg("Slave-Thread %d waits for signal\n", d->base.num);
		fplSignalWaitForOne(&d->signal, FPL_TIMEOUT_INFINITE);
		d->isSuccess = true;
		ftMsg("Got signal on Slave-Thread %d\n", d->base.num);
	} else if (d->testType == ConditionTestType_ConditionSignal) {
		ftMsg("Slave-Thread %d waits on condition\n", d->base.num);
		fplConditionWait(&d->condition, &d->mutex, FPL_TIMEOUT_INFINITE);
		d->isSuccess = true;
		ftMsg("Got condition on Slave-Thread %d\n", d->base.num);
	}

	ftMsg("Slave-Thread %d is done\n", d->base.num);
}

static void ThreadMasterProc(const fplThreadHandle *context, void *data) {
	(void)context;
	MasterThreadData *d = (MasterThreadData *)data;
	ftMsg("Master-Thread %d waits for 5 seconds\n", d->base.num);
	fplThreadSleep(5000);

	for (uint32_t signalIndex = 0; signalIndex < d->slaveCount; ++signalIndex) {
		if (d->testType == ConditionTestType_Signal) {
			ftMsg("Master-Thread %d sets signal %u\n", d->base.num, signalIndex);
			fplSignalSet(&d->slaveThreads[signalIndex].signal);
		} else if (d->testType == ConditionTestType_ConditionSignal) {
			ftMsg("Master-Thread %d sends signal to condition %u\n", d->base.num, signalIndex);
			fplConditionSignal(&d->slaveThreads[signalIndex].condition);
		}
	}

	ftMsg("Master-Thread %d is done\n", d->base.num);
}

static void ConditionThreadsTest(const size_t threadCount, const ConditionTestType testType) {
	ftAssert(threadCount > 1);

	ftLine();

	if (testType == ConditionTestType_Signal) {
		ftMsg("Signals test for %zu threads\n", threadCount);
	} else if (testType == ConditionTestType_ConditionSignal) {
		ftMsg("Condition-Variable (Single) test for %zu threads\n", threadCount);
	}

	MasterThreadData masterData = fplZeroInit;
	masterData.base.num = 1;
	masterData.testType = testType;

	SlaveThreadData slaveDatas[FPL_MAX_THREAD_COUNT] = fplZeroInit;
	size_t slaveThreadCount = threadCount - 1;
	for (size_t threadIndex = 0; threadIndex < slaveThreadCount; ++threadIndex) {
		slaveDatas[threadIndex].base.num = masterData.base.num + (int)threadIndex + 1;
		slaveDatas[threadIndex].testType = testType;
		if (testType == ConditionTestType_Signal) {
			ftIsTrue(fplSignalInit(&slaveDatas[threadIndex].signal, fplSignalValue_Unset));
		} else if (testType == ConditionTestType_ConditionSignal) {
			ftIsTrue(fplMutexInit(&slaveDatas[threadIndex].mutex));
			ftIsTrue(fplConditionInit(&slaveDatas[threadIndex].condition));
		}
		masterData.slaveCount++;
	}
	masterData.slaveThreads = slaveDatas;

	ftMsg("Start %zu slave threads, 1 master thread\n", slaveThreadCount);
	fplThreadHandle *threads[FPL_MAX_THREAD_COUNT];
	for (size_t threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
		if (threadIndex == 0) {
			threads[threadIndex] = fplThreadCreate(ThreadMasterProc, &masterData);
		} else {
			threads[threadIndex] = fplThreadCreate(ThreadSlaveProc, &slaveDatas[threadIndex - 1]);
		}
	}

	ftMsg("Wait for %zu threads to exit\n", threadCount);
	fplThreadWaitForAll(threads, threadCount, sizeof(fplThreadHandle *), FPL_TIMEOUT_INFINITE);

	ftMsg("Release resources for %zu threads\n", threadCount);
	for (size_t slaveIndex = 0; slaveIndex < slaveThreadCount; ++slaveIndex) {
		ftIsTrue(slaveDatas[slaveIndex].isSuccess);
	}
	for (size_t threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
		fplThreadHandle *thread = threads[threadIndex];
		ftAssertS32Equals((int32_t)fplThreadState_Stopped, (int32_t)thread->currentState);
	}
	for (size_t slaveIndex = 0; slaveIndex < slaveThreadCount; ++slaveIndex) {
		if (testType == ConditionTestType_Signal) {
			fplSignalDestroy(&slaveDatas[slaveIndex].signal);
		} else if (testType == ConditionTestType_ConditionSignal) {
			fplConditionDestroy(&slaveDatas[slaveIndex].condition);
			fplMutexDestroy(&slaveDatas[slaveIndex].mutex);
		}
	}
}

typedef struct ThreadLimitData {
	fplThreadHandle *handle;
	fplSignalHandle signal;
} ThreadLimitData;

static void ThreadLimitThreadProc(const fplThreadHandle *context, void *opaque) {
	(void)context;
	ThreadLimitData *data = (ThreadLimitData *)opaque;
	fplSignalWaitForOne(&data->signal, FPL_TIMEOUT_INFINITE);
	fplThreadSleep(2000);
}

static void ThreadLimitThreeSecProc(const fplThreadHandle *context, void *opaque) {
	(void)context;
	(void)opaque;
	fplThreadSleep(3000);
}

// Thread storage is no longer bounded by a fixed FPL_MAX_THREAD_COUNT array, instead threads
// are stored in a growable bucket list. The 'extra' parameter creates threads beyond a single
// bucket so multiple buckets get exercised. All creations must succeed.
static void ThreadStorageTest(const size_t extra) {
	ftLine();
	ftMsg("Thread storage test with '%zu' threads beyond one bucket\n", extra);

	{
		size_t usedThreadCount = fplGetUsedThreadCount();
		ftMsg("Used threads initial %zu\n", usedThreadCount);
		ftAssertSizeEquals(0, usedThreadCount);

		fplThreadHandle *oneThread = fplThreadCreate(ThreadLimitThreeSecProc, fpl_null);
		ftIsNotNull(oneThread);
		size_t usedWithOne = fplGetUsedThreadCount();
		size_t availableWithOne = fplGetAvailableThreadCount();
		size_t totalWithOne = fplGetTotalThreadCount();
		ftMsg("Used/Available/Total threads with one active thread %zu/%zu/%zu\n", usedWithOne, availableWithOne, totalWithOne);
		ftAssertSizeEquals(1, usedWithOne);
		// At least one bucket exists now and used + available must account for every managed slot
		ftIsTrue(totalWithOne >= 1);
		ftAssertSizeEquals(totalWithOne, usedWithOne + availableWithOne);
		fplThreadWaitForOne(oneThread, 4000);

		size_t usedAfterDone = fplGetUsedThreadCount();
		ftMsg("Used threads after single thread is done %zu\n", usedAfterDone);
		ftAssertSizeEquals(0, usedAfterDone);
	}

	// Create more threads than a single bucket holds, every single one must be created (no fixed cap anymore)
	size_t threadCount = FPL_MAX_THREAD_COUNT + extra;
	ThreadLimitData *datas = (ThreadLimitData *)fplMemoryAllocate(sizeof(ThreadLimitData) * threadCount);
	for (size_t threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
		ThreadLimitData *data = &datas[threadIndex];
		bool signalInitialized = fplSignalInit(&data->signal, fplSignalValue_Unset);
		ftIsTrue(signalInitialized);
		data->handle = fplThreadCreate(ThreadLimitThreadProc, data);
		ftIsNotNull(data->handle);
	}
	size_t usedAll = fplGetUsedThreadCount();
	size_t totalAll = fplGetTotalThreadCount();
	ftMsg("Used/Total threads with %zu active threads %zu/%zu\n", threadCount, usedAll, totalAll);
	ftAssertSizeEquals(threadCount, usedAll);
	ftIsTrue(totalAll >= threadCount);

	for (size_t signalIndex = 0; signalIndex < threadCount; ++signalIndex) {
		ThreadLimitData *data = &datas[signalIndex];
		fplSignalSet(&data->signal);
	}
	// Wait per-thread, threadCount can exceed FPL_MAX_THREAD_COUNT which is the limit for a single multi-wait call.
	for (size_t threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
		ThreadLimitData *data = &datas[threadIndex];
		fplThreadWaitForOne(data->handle, FPL_TIMEOUT_INFINITE);
	}
	size_t usedAfterAll = fplGetUsedThreadCount();
	ftAssertSizeEquals(0, usedAfterAll);
	for (size_t threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
		ThreadLimitData *data = &datas[threadIndex];
		fplSignalDestroy(&data->signal);
	}
	fplMemoryFree(datas);

	// Slot reuse, all threads have stopped so creating again must reuse slots and not grow the total
	{
		size_t totalBeforeReuse = fplGetTotalThreadCount();
		fplThreadHandle *reuseThread = fplThreadCreate(EmptyThreadproc, fpl_null);
		ftIsNotNull(reuseThread);
		fplThreadWaitForOne(reuseThread, FPL_TIMEOUT_INFINITE);
		size_t totalAfterReuse = fplGetTotalThreadCount();
		ftMsg("Total threads before/after reuse %zu/%zu\n", totalBeforeReuse, totalAfterReuse);
		ftAssertSizeEquals(totalBeforeReuse, totalAfterReuse);
	}
}

// Hammers thread creation and teardown from multiple threads at once, regression for the
// find-and-reserve race where two concurrent creates could be handed the same thread slot.
typedef struct ConcurrentCreatorData {
	volatile uint32_t *createFailures;
	volatile uint32_t *workersRun;
	size_t threadsPerCreator;
} ConcurrentCreatorData;

static void ConcurrentTinyWorkerProc(const fplThreadHandle *context, void *opaque) {
	(void)context;
	ConcurrentCreatorData *data = (ConcurrentCreatorData *)opaque;
	fplAtomicFetchAndAddU32(data->workersRun, 1);
}

static void ConcurrentCreatorProc(const fplThreadHandle *context, void *opaque) {
	(void)context;
	ConcurrentCreatorData *data = (ConcurrentCreatorData *)opaque;
	for (size_t workerIndex = 0; workerIndex < data->threadsPerCreator; ++workerIndex) {
		fplThreadHandle *worker = fplThreadCreate(ConcurrentTinyWorkerProc, data);
		if (worker == fpl_null) {
			fplAtomicFetchAndAddU32(data->createFailures, 1);
			continue;
		}
		fplThreadWaitForOne(worker, FPL_TIMEOUT_INFINITE);
	}
}

static void ThreadConcurrentCreateTest(const size_t creatorCount, const size_t threadsPerCreator) {
	ftLine();
	ftMsg("Concurrent thread create/teardown test, %zu creators x %zu threads\n", creatorCount, threadsPerCreator);

	volatile uint32_t createFailures = 0;
	volatile uint32_t workersRun = 0;
	ConcurrentCreatorData sharedData = fplZeroInit;
	sharedData.createFailures = &createFailures;
	sharedData.workersRun = &workersRun;
	sharedData.threadsPerCreator = threadsPerCreator;

	fplThreadHandle **creators = (fplThreadHandle **)fplMemoryAllocate(sizeof(fplThreadHandle *) * creatorCount);
	for (size_t creatorIndex = 0; creatorIndex < creatorCount; ++creatorIndex) {
		creators[creatorIndex] = fplThreadCreate(ConcurrentCreatorProc, &sharedData);
		ftIsNotNull(creators[creatorIndex]);
	}
	fplThreadWaitForAll(&creators[0], creatorCount, sizeof(fplThreadHandle *), FPL_TIMEOUT_INFINITE);

	uint32_t totalWorkers = fplAtomicLoadU32(&workersRun);
	uint32_t totalFailures = fplAtomicLoadU32(&createFailures);
	size_t usedAtEnd = fplGetUsedThreadCount();
	ftMsg("Workers run %u, create failures %u, used at end %zu\n", totalWorkers, totalFailures, usedAtEnd);
	ftAssertU32Equals(0, totalFailures);
	ftAssertU32Equals((uint32_t)(creatorCount * threadsPerCreator), totalWorkers);
	ftAssertSizeEquals(0, usedAtEnd);

	fplMemoryFree(creators);
}

static void TestThreading(void) {
	if (fplPlatformInit(fplInitFlags_None, fpl_null)) {
		//
		// Thread storage (unbounded, bucketed) and counts
		//
		{
			ThreadStorageTest(0);
			ThreadStorageTest(1);
			ThreadStorageTest(2);
			ThreadStorageTest(4);
			ThreadStorageTest(8);
			ThreadStorageTest(16);
			ThreadStorageTest(32);
		}

		//
		// Concurrent thread create/teardown (find-and-reserve race regression)
		//
		{
			ThreadConcurrentCreateTest(4, 50);
			ThreadConcurrentCreateTest(8, 100);
		}

		//
		// Single threading test
		//
		ftLine();
		ftMsg("Test 1 empty thread\n");
		{
			fplThreadHandle *thread;
			ftMsg("Start thread\n");
			thread = fplThreadCreate(EmptyThreadproc, fpl_null);
			ftMsg("Wait thread for exit\n");
			fplThreadWaitForOne(thread, UINT32_MAX);
			ftMsg("Thread is done\n");
			ftAssertS32Equals((int32_t)fplThreadState_Stopped, (int32_t)thread->currentState);
			fplThreadTerminate(thread);
		}

		ftLine();
		ftMsg("Test 1 sleeping-thread\n");
		{
			ThreadData threadData = fplZeroInit;
			threadData.num = 1;
			threadData.sleepFor = 3000;
			ftMsg("Start thread %d\n", threadData.num);
			fplThreadHandle *thread = fplThreadCreate(SingleThreadProc, &threadData);
			ftMsg("Wait thread %d for exit\n", threadData.num);
			fplThreadWaitForOne(thread, UINT32_MAX);
			ftMsg("Thread %d is done\n", threadData.num);
			ftAssertS32Equals((int32_t)fplThreadState_Stopped, (int32_t)thread->currentState);
			fplThreadTerminate(thread);
		}

		//
		// Multi threads test
		//
		size_t coreCount = fplCPUGetCoreCount();
		size_t threadCountForCores = coreCount > 2 ? coreCount - 1 : 1;
		{
			SimpleMultiThreadTest(2);
			SimpleMultiThreadTest(3);
			SimpleMultiThreadTest(4);
			SimpleMultiThreadTest(threadCountForCores);
		}



		//
		// Sync tests
		//
		{
			SyncThreadsTestAtomics();
			SyncThreadsTestSemaphores(2);
			SyncThreadsTestSemaphores(3);
			SyncThreadsTestSemaphores(4);
			SyncThreadsTestSemaphores(threadCountForCores);
		}

		//
		// Signals tests
		//
		{
			ConditionThreadsTest(2, ConditionTestType_Signal);
			ConditionThreadsTest(3, ConditionTestType_Signal);
			ConditionThreadsTest(4, ConditionTestType_Signal);
			ConditionThreadsTest(threadCountForCores, ConditionTestType_Signal);
		}

		//
		// Condition tests
		//
		{
			ConditionThreadsTest(2, ConditionTestType_ConditionSignal);
			ConditionThreadsTest(3, ConditionTestType_ConditionSignal);
			ConditionThreadsTest(4, ConditionTestType_ConditionSignal);
			ConditionThreadsTest(threadCountForCores, ConditionTestType_ConditionSignal);
		}

		fplPlatformRelease();
	}
}

static void TestFiles(void) {
#if defined(FPL_PLATFORM_WINDOWS)
	const char *testNotExistingFile = "C:\\Windows\\i_am_not_existing.lib";
	const char *testExistingFile = "C:\\Windows\\notepad.exe";
	const char *testRootPath = "C:\\";
	const char *testRootFilter = "Program*";
#else
	const char *testNotExistingFile = "/i_am_not_existing.whatever";
	const char *testExistingFile = "/usr/sbin/nologin";
	const char *testRootPath = "/";
	const char *testRootFilter = "us*";
#endif

	ftMsg("Test File Exists\n");
	{
		bool nonExisting = fplFileExists(testNotExistingFile);
		ftIsFalse(nonExisting);
		bool existing = fplFileExists(testExistingFile);
		ftIsTrue(existing);
	}
	ftMsg("Test File Size\n");
	{
		uint32_t emptySize = fplFileGetSizeFromPath32(testNotExistingFile);
		ftAssertU32Equals(0, emptySize);
		uint32_t existingSize = fplFileGetSizeFromPath32(testExistingFile);
		ftAssert(existingSize > 0);
	}
	ftMsg("Test Directory Iterations without filter\n");
	{
		fplFileEntry fileEntry = fplZeroInit;
		for (bool r = fplDirectoryListBegin(testRootPath, "*.*", &fileEntry); r; r = fplDirectoryListNext(&fileEntry)) {
			ftMsg("%s\n", fileEntry.name);
		}
		fplDirectoryListEnd(&fileEntry);
	}
	ftMsg("Test Directory Iterations with all filter\n");
	{
		fplFileEntry fileEntry = fplZeroInit;
		for (bool r = fplDirectoryListBegin(testRootPath, "*", &fileEntry); r; r = fplDirectoryListNext(&fileEntry)) {
			ftMsg("%s\n", fileEntry.name);
		}
		fplDirectoryListEnd(&fileEntry);
	}
	ftMsg("Test Directory Iterations with root filter '%s'\n", testRootFilter);
	{
		fplFileEntry fileEntry = fplZeroInit;
		bool r = fplDirectoryListBegin(testRootPath, testRootFilter, &fileEntry);
		ftMsg("%s\n", fileEntry.name);
		ftIsTrue(r);
		fplDirectoryListEnd(&fileEntry);
	}
}

static void TestAtomics(void) {
	// @TODO(final): Add integral wrap test for all atomics

	ftMsg("Test AtomicExchangeU32 with different values\n");
	{
		const uint32_t expectedBefore = 42;
		const uint32_t expectedAfter = 1337;
		volatile uint32_t t = expectedBefore;
		uint32_t actual = fplAtomicExchangeU32(&t, expectedAfter);
		ftAssertU32Equals(expectedBefore, actual);
		ftAssertU32Equals(expectedAfter, (uint32_t)t);
	}
	ftMsg("Test AtomicExchangeU32 with negative value\n");
	{
		const uint32_t expectedBefore = 42;
		const uint32_t exchangeValue = (uint32_t)-1;
		const uint32_t expectedAfter = (uint32_t)UINT32_MAX;
		volatile uint32_t t = expectedBefore;
		uint32_t actual = fplAtomicExchangeU32(&t, exchangeValue);
		ftAssertU32Equals(expectedBefore, actual);
		ftAssertU32Equals(expectedAfter, (uint32_t)t);
	}
	ftMsg("Test AtomicExchangeU32 with same value\n");
	{
		const uint32_t expectedBefore = 1;
		const uint32_t exchangeValue = expectedBefore;
		const uint32_t expectedAfter = exchangeValue;
		volatile uint32_t t = expectedBefore;
		uint32_t actual = fplAtomicExchangeU32(&t, exchangeValue);
		ftAssertU32Equals(expectedBefore, actual);
		ftAssertU32Equals(expectedAfter, (uint32_t)t);
	}
	ftMsg("Test AtomicExchangeU32 with UINT32_MAX\n");
	{
		const uint32_t expectedBefore = 1;
		const uint32_t exchangeValue = UINT32_MAX;
		const uint32_t expectedAfter = exchangeValue;
		volatile uint32_t t = expectedBefore;
		uint32_t actual = fplAtomicExchangeU32(&t, exchangeValue);
		ftAssertU32Equals(expectedBefore, actual);
		ftAssertU32Equals(expectedAfter, (uint32_t)t);
	}
	ftMsg("Test AtomicExchangeU32 with INT32_MAX + 1\n");
	{
		const uint32_t expectedBefore = 1;
		const uint32_t exchangeValue = (uint32_t)INT32_MAX + 1;
		const uint32_t expectedAfter = exchangeValue;
		volatile uint32_t t = expectedBefore;
		uint32_t actual = fplAtomicExchangeU32(&t, exchangeValue);
		ftAssertU32Equals(expectedBefore, actual);
		ftAssertU32Equals(expectedAfter, (uint32_t)t);
	}
	ftMsg("Test AtomicExchangeS32 with different values\n");
	{
		const int32_t expectedBefore = 42;
		const int32_t exchangeValue = 1337;
		const int32_t expectedAfter = exchangeValue;
		volatile int32_t t = expectedBefore;
		int32_t actual = fplAtomicExchangeS32(&t, exchangeValue);
		ftAssertS32Equals(expectedBefore, actual);
		ftAssertS32Equals(expectedAfter, (int32_t)t);
	}
	ftMsg("Test AtomicExchangeS32 with negative value\n");
	{
		const int32_t expectedBefore = 42;
		const int32_t exchangeValue = -1;
		const int32_t expectedAfter = exchangeValue;
		volatile int32_t t = expectedBefore;
		int32_t actual = fplAtomicExchangeS32(&t, exchangeValue);
		ftAssertS32Equals(expectedBefore, actual);
		ftAssertS32Equals(expectedAfter, (int32_t)t);
	}
	ftMsg("Test AtomicExchangeS32 with same value\n");
	{
		const int32_t expectedBefore = 1;
		const int32_t exchangeValue = expectedBefore;
		const int32_t expectedAfter = exchangeValue;
		volatile int32_t t = expectedBefore;
		int32_t actual = fplAtomicExchangeS32(&t, exchangeValue);
		ftAssertS32Equals(expectedBefore, actual);
		ftAssertS32Equals(expectedAfter, (int32_t)t);
	}

	ftMsg("Test AtomicExchangeU64 with different values\n");
	{
		const uint64_t expectedBefore = 42;
		const uint64_t expectedAfter = 1337;
		volatile uint64_t t = expectedBefore;
		uint64_t actual = fplAtomicExchangeU64(&t, expectedAfter);
		ftAssertU64Equals(expectedBefore, actual);
		ftAssertU64Equals(expectedAfter, (uint64_t)t);
	}
	ftMsg("Test AtomicExchangeU64 with negative value\n");
	{
		const uint64_t expectedBefore = 42;
		const uint64_t exchangeValue = (uint64_t)-1;
		const uint64_t expectedAfter = (uint64_t)UINT64_MAX;
		volatile uint64_t t = expectedBefore;
		uint64_t actual = fplAtomicExchangeU64(&t, exchangeValue);
		ftAssertU64Equals(expectedBefore, actual);
		ftAssertU64Equals(expectedAfter, (uint64_t)t);
	}
	ftMsg("Test AtomicExchangeU64 with same value\n");
	{
		const uint64_t expectedBefore = 1;
		const uint64_t exchangeValue = expectedBefore;
		const uint64_t expectedAfter = exchangeValue;
		volatile uint64_t t = expectedBefore;
		uint64_t actual = fplAtomicExchangeU64(&t, exchangeValue);
		ftAssertU64Equals(expectedBefore, actual);
		ftAssertU64Equals(expectedAfter, (uint64_t)t);
	}
	ftMsg("Test AtomicExchangeU64 with UINT64_MAX\n");
	{
		const uint64_t expectedBefore = 1;
		const uint64_t exchangeValue = UINT64_MAX;
		const uint64_t expectedAfter = exchangeValue;
		volatile uint64_t t = expectedBefore;
		uint64_t actual = fplAtomicExchangeU64(&t, exchangeValue);
		ftAssertU64Equals(expectedBefore, actual);
		ftAssertU64Equals(expectedAfter, (uint64_t)t);
	}
	ftMsg("Test AtomicExchangeU64 with INT64_MAX + 1\n");
	{
		const uint64_t expectedBefore = 1;
		const uint64_t exchangeValue = (uint64_t)INT64_MAX + 1;
		const uint64_t expectedAfter = exchangeValue;
		volatile uint64_t t = expectedBefore;
		uint64_t actual = fplAtomicExchangeU64(&t, exchangeValue);
		ftAssertU64Equals(expectedBefore, actual);
		ftAssertU64Equals(expectedAfter, (uint64_t)t);
	}
	ftMsg("Test AtomicExchangeS64 with different values\n");
	{
		const int64_t expectedBefore = 42;
		const int64_t exchangeValue = 1337;
		const int64_t expectedAfter = exchangeValue;
		volatile int64_t t = expectedBefore;
		int64_t actual = fplAtomicExchangeS64(&t, exchangeValue);
		ftAssertS64Equals(expectedBefore, actual);
		ftAssertS64Equals(expectedAfter, (int64_t)t);
	}
	ftMsg("Test AtomicExchangeS64 with negative value\n");
	{
		const int64_t expectedBefore = 42;
		const int64_t exchangeValue = -1;
		const int64_t expectedAfter = exchangeValue;
		volatile int64_t t = expectedBefore;
		int64_t actual = fplAtomicExchangeS64(&t, exchangeValue);
		ftAssertS64Equals(expectedBefore, actual);
		ftAssertS64Equals(expectedAfter, (int64_t)t);
	}
	ftMsg("Test AtomicExchangeS64 with same value\n");
	{
		const int64_t expectedBefore = 1;
		const int64_t exchangeValue = expectedBefore;
		const int64_t expectedAfter = exchangeValue;
		volatile int64_t t = expectedBefore;
		int64_t actual = fplAtomicExchangeS64(&t, exchangeValue);
		ftAssertS64Equals(expectedBefore, actual);
		ftAssertS64Equals(expectedAfter, (int64_t)t);
	}

	//
	// AtomicCompareAndSwap
	//
	ftMsg("Test AtomicCompareAndSwapU32 with exchange\n");
	{
		const uint32_t initialValue = UINT16_MAX + 3;
		volatile uint32_t value = initialValue;
		uint32_t comparand = initialValue;
		uint32_t exchange = UINT16_MAX + 6;
		uint32_t actual = fplAtomicCompareAndSwapU32(&value, comparand, exchange);
		ftAssertU32Equals(initialValue, actual);
		ftAssertU32Equals(exchange, (uint32_t)value);
	}
	ftMsg("Test AtomicCompareAndSwapU32 no exchange\n");
	{
		const uint32_t initialValue = UINT16_MAX + 3;
		volatile uint32_t value = initialValue;
		uint32_t comparand = initialValue + 6;
		uint32_t exchange = UINT16_MAX + 6;
		uint32_t actual = fplAtomicCompareAndSwapU32(&value, comparand, exchange);
		ftAssertU32Equals(initialValue, actual);
		ftAssertU32Equals(initialValue, (uint32_t)value);
	}
	ftMsg("Test AtomicCompareAndSwapU64 with exchange\n");
	{
		const uint64_t initialValue = UINT32_MAX + 3ULL;
		volatile uint64_t value = initialValue;
		uint64_t comparand = initialValue;
		uint64_t exchange = UINT32_MAX + 6ULL;
		uint64_t actual = fplAtomicCompareAndSwapU64(&value, comparand, exchange);
		ftAssertU64Equals(initialValue, actual);
		ftAssertU64Equals(exchange, (uint64_t)value);
	}
	ftMsg("Test AtomicCompareAndSwapU64 no exchange\n");
	{
		const uint64_t initialValue = UINT32_MAX + 3ULL;
		volatile uint64_t value = initialValue;
		uint64_t comparand = initialValue + 6;
		uint64_t exchange = UINT32_MAX + 6ULL;
		uint64_t actual = fplAtomicCompareAndSwapU64(&value, comparand, exchange);
		ftAssertU64Equals(initialValue, actual);
		ftAssertU64Equals(initialValue, (uint64_t)value);
	}

	//
	// AtomicFetchAndAdd
	//
	ftMsg("Test AtomicFetchAndAddU32 with 3\n");
	{
		const uint32_t initial = UINT16_MAX + 42UL;
		volatile uint32_t value = initial;
		uint32_t addend = 3;
		uint32_t actual = fplAtomicFetchAndAddU32(&value, addend);
		ftAssertU32Equals(initial, actual);
		ftAssertU32Equals(initial + addend, (uint32_t)value);
	}
	ftMsg("Test AtomicFetchAndAddU64 with 3\n");
	{
		const uint64_t initial = UINT32_MAX + 42ULL;
		volatile uint64_t value = initial;
		uint64_t addend = 3;
		uint64_t actual = fplAtomicFetchAndAddU64(&value, addend);
		ftAssertU64Equals(initial, actual);
		ftAssertU64Equals(initial + addend, (uint64_t)value);
	}
	ftMsg("Test AtomicFetchAndAddS32 with -3\n");
	{
		const int32_t initial = INT16_MAX + 42;
		volatile int32_t value = initial;
		int32_t addend = -3;
		int32_t actual = fplAtomicFetchAndAddS32(&value, addend);
		ftAssertS32Equals(initial, actual);
		ftAssertS32Equals(initial + addend, (int32_t)value);
	}
	ftMsg("Test AtomicFetchAndAddS64 with -3\n");
	{
		const int64_t initial = INT32_MAX + 42LL;
		volatile int64_t value = initial;
		int64_t addend = -3;
		int64_t actual = fplAtomicFetchAndAddS64(&value, addend);
		ftAssertS64Equals(initial, actual);
		ftAssertS64Equals(initial + addend, (int64_t)value);
	}
	ftMsg("Test AtomicFetchAndAddSize with 13\n");
	{
		const size_t initial = 42ULL;
		volatile size_t value = initial;
		size_t addend = 1024ULL;
		size_t actual = fplAtomicFetchAndAddSize(&value, addend);
		ftAssertSizeEquals(initial, actual);
		ftAssertSizeEquals(initial + addend, (size_t)value);
	}
	ftMsg("Test AtomicFetchAndAddPtr with 16\n");
	{
		char buffer[64];
		buffer[16] = 'A';
		void *initial = (void *)buffer;
		volatile void *value = initial;
		intptr_t addend = 16;
		void *actual = fplAtomicFetchAndAddPtr(&value, addend);
		ftAssertPointerEquals(initial, actual);
		ftAssertPointerEquals((void *)((intptr_t)initial + addend), (void *)value);
		size_t offset = (uintptr_t)value - (uintptr_t)initial;
		char c = buffer[offset];
		ftAssertCharEquals('A', c);
	}
	ftMsg("Test AtomicFetchAndAddPtr with 0\n");
	{
		char buffer[64];
		buffer[0] = 'A';
		void *initial = (void *)buffer;
		volatile void *value = initial;
		intptr_t addend = 0;
		void *actual = fplAtomicFetchAndAddPtr(&value, addend);
		ftAssertPointerEquals(initial, actual);
		ftAssertPointerEquals((void *)((intptr_t)initial + addend), (void *)value);
		size_t offset = (uintptr_t)value - (uintptr_t)initial;
		char c = buffer[offset];
		ftAssertCharEquals('A', c);
	}
	ftMsg("Test AtomicFetchAndAddPtr with -16\n");
	{
		char buffer[64];
		buffer[16] = 'A';
		void *initial = (void *)((intptr_t)buffer + 16);
		volatile void *value = initial;
		intptr_t addend = -16;
		void *actual = fplAtomicFetchAndAddPtr(&value, addend);
		ftAssertPointerEquals(initial, actual);
		ftAssertPointerEquals((void *)((intptr_t)initial + addend), (void *)value);
		size_t offset = (uintptr_t)initial - (uintptr_t)value;
		char c = buffer[offset];
		ftAssertCharEquals('A', c);
	}

	//
	// AtomicAddAndFetch
	//
	ftMsg("Test AtomicAddAndFetchU32 with 3\n");
	{
		const uint32_t initial = UINT16_MAX + 42UL;
		const uint32_t addend = 3;
		const uint32_t expected = initial + addend;
		volatile uint32_t value = initial;
		uint32_t actual = fplAtomicAddAndFetchU32(&value, addend);
		ftAssertU32Equals(expected, actual);
		ftAssertU32Equals(expected, (uint32_t)value);
	}
	ftMsg("Test AtomicAddAndFetchU64 with 3\n");
	{
		const uint64_t initial = UINT32_MAX + 42ULL;
		const uint64_t addend = 3;
		const uint64_t expected = initial + addend;
		volatile uint64_t value = initial;
		uint64_t actual = fplAtomicAddAndFetchU64(&value, addend);
		ftAssertU64Equals(expected, actual);
		ftAssertU64Equals(expected, (uint64_t)value);
	}
	ftMsg("Test AtomicAddAndFetchS32 with -3\n");
	{
		const int32_t initial = INT16_MAX + 42;
		const int32_t addend = -3;
		const int32_t expected = initial + addend;
		volatile int32_t value = initial;
		int32_t actual = fplAtomicAddAndFetchS32(&value, addend);
		ftAssertS32Equals(expected, actual);
		ftAssertS32Equals(expected, (int32_t)value);
	}
	ftMsg("Test AtomicAddAndFetchS64 with -3\n");
	{
		const int64_t initial = INT32_MAX + 42LL;
		const int64_t addend = -3;
		const int64_t expected = initial + addend;
		volatile int64_t value = initial;
		int64_t actual = fplAtomicAddAndFetchS64(&value, addend);
		ftAssertS64Equals(expected, actual);
		ftAssertS64Equals(expected, (int64_t)value);
	}
	ftMsg("Test AtomicAddAndFetchSize with 13\n");
	{
		const size_t initial = 42ULL;
		const size_t addend = 1024ULL;
		const size_t expected = initial + addend;
		volatile size_t value = initial;
		size_t actual = fplAtomicAddAndFetchSize(&value, addend);
		ftAssertSizeEquals(expected, actual);
		ftAssertSizeEquals(expected, (size_t)value);
	}
	ftMsg("Test AtomicAddAndFetchPtr with 16\n");
	{
		char buffer[64];
		buffer[16] = 'A';
		void *initial = (void *)buffer;
		intptr_t addend = 16;
		const void *expected = (void *)((intptr_t)initial + addend);
		volatile void *value = initial;
		void *actual = fplAtomicAddAndFetchPtr(&value, addend);
		ftAssertPointerEquals(expected, actual);
		ftAssertPointerEquals(expected, (void *)value);
		size_t offset = (uintptr_t)value - (uintptr_t)initial;
		char c = buffer[offset];
		ftAssertCharEquals('A', c);
	}
	ftMsg("Test AtomicAddAndFetchPtr with 0\n");
	{
		char buffer[64];
		buffer[0] = 'A';
		void *initial = (void *)buffer;
		intptr_t addend = 0;
		const void *expected = (void *)((intptr_t)initial + addend);
		volatile void *value = initial;
		void *actual = fplAtomicAddAndFetchPtr(&value, addend);
		ftAssertPointerEquals(expected, actual);
		ftAssertPointerEquals(expected, (void *)value);
		size_t offset = (uintptr_t)value - (uintptr_t)initial;
		char c = buffer[offset];
		ftAssertCharEquals('A', c);
	}
	ftMsg("Test AtomicAddAndFetchPtr with -16\n");
	{
		char buffer[64];
		buffer[16] = 'A';
		void *initial = (void *)((intptr_t)buffer + 16);
		intptr_t addend = -16;
		const void *expected = (void *)((intptr_t)initial + addend);
		volatile void *value = initial;
		void *actual = fplAtomicAddAndFetchPtr(&value, addend);
		ftAssertPointerEquals(expected, actual);
		ftAssertPointerEquals(expected, (void *)value);
		size_t offset = (uintptr_t)initial - (uintptr_t)value;
		char c = buffer[offset];
		ftAssertCharEquals('A', c);
	}

	//
	// AtomicIncrement
	//
	ftMsg("Test AtomicIncrementU32 with 35\n");
	{
		const uint32_t initial = UINT16_MAX + 35UL;
		const uint32_t expected = initial + 1UL;
		volatile uint32_t value = initial;
		uint32_t actual = fplAtomicIncrementU32(&value);
		ftAssertU32Equals(expected, actual);
		ftAssertU32Equals(expected, (uint32_t)value);
	}
	ftMsg("Test AtomicIncrementU64 with 35\n");
	{
		const uint64_t initial = UINT32_MAX + 35ULL;
		const uint64_t expected = initial + 1ULL;
		volatile uint64_t value = initial;
		uint64_t actual = fplAtomicIncrementU64(&value);
		ftAssertU64Equals(expected, actual);
		ftAssertU64Equals(expected, (uint64_t)value);
	}
	ftMsg("Test AtomicIncrementS32 with 35\n");
	{
		const int32_t initial = INT16_MAX + 35L;
		const int32_t expected = initial + 1L;
		volatile int32_t value = initial;
		int32_t actual = fplAtomicIncrementS32(&value);
		ftAssertS32Equals(expected, actual);
		ftAssertS32Equals(expected, (int32_t)value);
	}
	ftMsg("Test AtomicIncrementS32 with -35\n");
	{
		const int32_t initial = INT16_MAX - 35L;
		const int32_t expected = initial + 1L;
		volatile int32_t value = initial;
		int32_t actual = fplAtomicIncrementS32(&value);
		ftAssertS32Equals(expected, actual);
		ftAssertS32Equals(expected, (int32_t)value);
	}
	ftMsg("Test AtomicIncrementS64 with 35\n");
	{
		const int64_t initial = INT32_MAX + 35LL;
		const int64_t expected = initial + 1LL;
		volatile int64_t value = initial;
		int64_t actual = fplAtomicIncrementS64(&value);
		ftAssertS64Equals(expected, actual);
		ftAssertS64Equals(expected, (int64_t)value);
	}
	ftMsg("Test AtomicIncrementSize with 35\n");
	{
		const size_t initial = 1024;
		const size_t expected = initial + 1LL;
		volatile size_t value = initial;
		size_t actual = fplAtomicIncrementSize(&value);
		ftAssertSizeEquals(expected, actual);
		ftAssertSizeEquals(expected, (size_t)value);
	}
	ftMsg("Test AtomicIncrementPtr with 16\n");
	{
		char buffer[64];
		buffer[sizeof(void *)] = 'A';
		void *initial = (void *)buffer;
		const void *expected = (const void *)((uintptr_t)initial + sizeof(void *));
		volatile void *value = initial;
		void *actual = fplAtomicIncrementPtr(&value);
		ftAssertPointerEquals(expected, actual);
		ftAssertPointerEquals(expected, (void *)value);
		size_t offset = (uintptr_t)value - (uintptr_t)initial;
		char c = buffer[offset];
		ftAssertCharEquals('A', c);
	}
}

static void TestStrings(void) {
	ftMsg("Test ansi string length\n");
	{
		size_t actual = fplGetStringLength(fpl_null);
		ftAssertSizeEquals(0, actual);
	}
	{
		size_t actual = fplGetStringLength("");
		ftAssertSizeEquals(0, actual);
	}
	{
		size_t actual = fplGetStringLength("ABC");
		ftAssertSizeEquals(3, actual);
	}
	{
		size_t actual = fplGetStringLength("ABC Hello World!");
		ftAssertSizeEquals(16, actual);
	}
	{
		char buffer[32];
		buffer[0] = 'A';
		buffer[1] = 'B';
		buffer[2] = 'C';
		buffer[3] = 0;
		size_t actual = fplGetStringLength(buffer);
		ftAssertSizeEquals(3, actual);
	}

	ftMsg("Test string equal\n");
	{
		bool res = fplIsStringEqual(fpl_null, fpl_null);
		ftAssertTrue(res);
	}
	{
		bool res = fplIsStringEqual(fpl_null, "");
		ftAssertFalse(res);
	}
	{
		bool res = fplIsStringEqual("B", "A");
		ftAssertFalse(res);
	}
	{
		bool res = fplIsStringEqual("A", "A");
		ftAssertTrue(res);
	}
	{
		bool res = fplIsStringEqual("Hello", "World");
		ftAssertFalse(res);
	}
	{
		bool res = fplIsStringEqual("World", "World");
		ftAssertTrue(res);
	}
	{
		bool res = fplIsStringEqualLen(fpl_null, 0, fpl_null, 0);
		ftAssertFalse(res);
	}
	{
		bool res = fplIsStringEqualLen("", 0, fpl_null, 0);
		ftAssertFalse(res);
	}
	{
		bool res = fplIsStringEqualLen(fpl_null, 0, "", 0);
		ftAssertFalse(res);
	}
	{
		bool res = fplIsStringEqualLen("", 0, "", 0);
		ftAssertTrue(res);
	}
	{
		bool res = fplIsStringEqualLen("B", 1, "A", 1);
		ftAssertFalse(res);
	}
	{
		bool res = fplIsStringEqualLen("A", 1, "A", 1);
		ftAssertTrue(res);
	}
	{
		bool res = fplIsStringEqualLen("A", 1, "A", 0);
		ftAssertFalse(res);
	}
	{
		bool res = fplIsStringEqualLen("A", 1, "B", 1);
		ftAssertFalse(res);
	}
	{
		bool res = fplIsStringEqualLen("Hello", 5, "World", 5);
		ftAssertFalse(res);
	}
	{
		bool res = fplIsStringEqualLen("Hello", 3, "World", 5);
		ftAssertFalse(res);
	}
	{
		bool res = fplIsStringEqualLen("World", 5, "Hello", 3);
		ftAssertFalse(res);
	}
	{
		bool res = fplIsStringEqualLen("Hello", 5, "Hello", 5);
		ftAssertTrue(res);
	}
	{
		bool res = fplIsStringEqualLen("Hello", 3, "Hello", 3);
		ftAssertTrue(res);
	}

	ftMsg("Test append string\n");
	{
		ftIsNull(fplStringAppend(fpl_null, fpl_null, 0));
	}
	{
		char buffer[64] = fplZeroInit;
		fplStringAppend(fpl_null, buffer, fplArrayCount(buffer));
		ftAssertStringEquals("", buffer);
	}
	{
		char buffer[64] = fplZeroInit;
		fplStringAppend("Hello", buffer, fplArrayCount(buffer));
		ftAssertStringEquals("Hello", buffer);
	}
	{
		char buffer[64] = fplZeroInit;
		fplCopyString("Hello", buffer, fplArrayCount(buffer));
		fplStringAppend(" World", buffer, fplArrayCount(buffer));
		ftAssertStringEquals("Hello World", buffer);
	}

	ftMsg("Test format ansi string\n");
	{
		size_t res = fplStringFormat(fpl_null, 0, fpl_null);
		ftAssertSizeEquals(0, res);
	}
	{
		char buffer[1];
		size_t res = fplStringFormat(buffer, 0, "");
		ftAssertSizeEquals(0, res);
	}
	{
		char buffer[1];
		size_t res = fplStringFormat(buffer, fplArrayCount(buffer), "A");
		ftAssertSizeEquals(0, res);
	}
	{
		char buffer[2];
		size_t res = fplStringFormat(buffer, fplArrayCount(buffer), "A");
		ftAssertSizeEquals(1, res);
		bool matches = fplIsStringEqualLen("A", 1, buffer, 1);
		ftAssertTrue(matches);
	}
	{
		char buffer[5];
		size_t res = fplStringFormat(buffer, fplArrayCount(buffer), "Hello");
		ftAssertSizeEquals(0, res);
	}
	{
		char buffer[6];
		size_t res = fplStringFormat(buffer, fplArrayCount(buffer), "Hello");
		ftAssertSizeEquals(5, res);
		bool r = fplIsStringEqualLen("Hello", 5, buffer, 5);
		ftAssertTrue(r);
	}
	{
		char buffer[6];
		size_t res = fplStringFormat(buffer, fplArrayCount(buffer), "%s", "Hello");
		ftAssertSizeEquals(5, res);
		bool r = fplIsStringEqualLen("Hello", 5, buffer, 5);
		ftAssertTrue(r);
	}
	{
		char buffer[20];
		size_t res = fplStringFormat(buffer, fplArrayCount(buffer), "%4xd-%2d-%2d %2d:%2d:%2d", 2009, 11, 17, 13, 47, 25);
		ftAssertSizeEquals(0, res);
	}
	{
		char buffer[20];
		size_t res = fplStringFormat(buffer, fplArrayCount(buffer), "%4d-%2d-%2d %2d:%2d:%2d", 2009, 11, 17, 13, 47, 25);
		ftAssertSizeEquals(19, res);
		bool r = fplIsStringEqual("2009-11-17 13:47:25", buffer);
		ftAssertTrue(r);
	}

	ftMsg("Test fplS32ToString\n");
	{
		char smallBuffer[2];
		char bigBuffer[16];

		ftAssertSizeEquals(1, fplS32ToString(0, fpl_null, 0));
		ftAssertSizeEquals(1, fplS32ToString(0, fpl_null, 4));
		ftAssertSizeEquals(0, fplS32ToString(11, smallBuffer, fplArrayCount(smallBuffer)));

		ftAssertSizeEquals(1, fplS32ToString(7, smallBuffer, fplArrayCount(smallBuffer)));
		ftAssertStringEquals("7", smallBuffer);

		ftAssertSizeEquals(3, fplS32ToString(129, bigBuffer, fplArrayCount(bigBuffer)));
		ftAssertStringEquals("129", bigBuffer);

		ftAssertSizeEquals(4, fplS32ToString(1337, bigBuffer, fplArrayCount(bigBuffer)));
		ftAssertStringEquals("1337", bigBuffer);

		ftAssertSizeEquals(8, fplS32ToString(-1234567, bigBuffer, fplArrayCount(bigBuffer)));
		ftAssertStringEquals("-1234567", bigBuffer);
	}

	ftMsg("Test fplStringToS32\n");
	{
		ftAssertS32Equals(0, fplStringToS32(fpl_null));
		ftAssertS32Equals(0, fplStringToS32(""));
		ftAssertS32Equals(0, fplStringToS32("bullshit"));
		ftAssertS32Equals(0, fplStringToS32("0x"));
		ftAssertS32Equals(0, fplStringToS32("0xFFBBCCDD"));
		ftAssertS32Equals(0, fplStringToS32("0"));
		ftAssertS32Equals(7, fplStringToS32("7"));
		ftAssertS32Equals(10, fplStringToS32("10"));
		ftAssertS32Equals(1337, fplStringToS32("1337"));
		ftAssertS32Equals(-1234567, fplStringToS32("-1234567"));
	}

	ftMsg("Test fplStringToS32Len\n");
	{
		ftAssertS32Equals(0, fplStringToS32Len(fpl_null, 0));
		ftAssertS32Equals(0, fplStringToS32Len(fpl_null, 1));
		ftAssertS32Equals(0, fplStringToS32Len("", 0));
		ftAssertS32Equals(0, fplStringToS32Len("123", 0));
		ftAssertS32Equals(0, fplStringToS32Len("bullshit", 8));
		ftAssertS32Equals(0, fplStringToS32Len("0x", 2));
		ftAssertS32Equals(0, fplStringToS32Len("0xFFBBCCDD", 10));
		ftAssertS32Equals(0, fplStringToS32Len("0", 1));
		ftAssertS32Equals(7, fplStringToS32Len("7", 1));
		ftAssertS32Equals(10, fplStringToS32Len("10", 2));
		ftAssertS32Equals(1337, fplStringToS32Len("1337", 4));
		ftAssertS32Equals(-1234567, fplStringToS32Len("-1234567", 8));
	}
}

// Converts utf8 -> wide -> utf8 and verifies the result matches the original bytes.
// This is platform independent even for astral code points, since the intermediate wide form
// (UTF-32 on POSIX, UTF-16 on Windows) never needs to be inspected directly.
static void CheckUtf8RoundTrip(const char *utf8, const size_t utf8Len) {
	wchar_t wide[64];
	size_t wideCount = fplUTF8StringToWideString(utf8, utf8Len, wide, fplArrayCount(wide));
	ftAssertTrue(wideCount > 0);
	char back[256];
	size_t backLen = fplWideStringToUTF8String(wide, wideCount, back, fplArrayCount(back));
	ftAssertSizeEquals(utf8Len, backLen);
	ftAssertStringEquals(utf8, back);
}

static void TestUnicodeConversion(void) {
	ftMsg("Test UTF-8 <-> WideString conversion (wchar_t is %zu bytes)\n", sizeof(wchar_t));

	// Explicit UTF-8 byte sequences so the tests never depend on the source-file encoding.
	// 'H'/'A'=1 byte, 'ö'=U+00F6 (2 bytes), '€'=U+20AC (3 bytes), '😀'=U+1F600 (4 bytes).
	static const char asciiUtf8[] = { 'H', 'e', 'l', 'l', 'o', 0 };
	static const char umlautUtf8[] = { (char)0xC3, (char)0xB6, 0 };
	static const char euroUtf8[] = { (char)0xE2, (char)0x82, (char)0xAC, 0 };
	static const char emojiUtf8[] = { (char)0xF0, (char)0x9F, (char)0x98, (char)0x80, 0 };
	static const char mixedUtf8[] = { 'A', (char)0xC3, (char)0xB6, (char)0xE2, (char)0x82, (char)0xAC, (char)0xF0, (char)0x9F, (char)0x98, (char)0x80, 0 };

	const uint32_t codePointA = 0x0041;
	const uint32_t codePointUmlaut = 0x00F6;
	const uint32_t codePointEuro = 0x20AC;

	ftMsg("Test invalid arguments\n");
	{
		ftAssertSizeEquals(0, fplUTF8StringToWideString(fpl_null, 1, fpl_null, 0));
		ftAssertSizeEquals(0, fplUTF8StringToWideString(asciiUtf8, 0, fpl_null, 0));
		ftAssertSizeEquals(0, fplWideStringToUTF8String(fpl_null, 1, fpl_null, 0));
		ftAssertSizeEquals(0, fplWideStringToUTF8String(L"A", 0, fpl_null, 0));
	}

	ftMsg("Test UTF-8 to WideString size query (dest == null)\n");
	{
		// A single BMP code point is exactly one wide unit on every platform.
		ftAssertSizeEquals(5, fplUTF8StringToWideString(asciiUtf8, 5, fpl_null, 0));
		ftAssertSizeEquals(1, fplUTF8StringToWideString(umlautUtf8, 2, fpl_null, 0));
		ftAssertSizeEquals(1, fplUTF8StringToWideString(euroUtf8, 3, fpl_null, 0));
	}

	ftMsg("Test UTF-8 to WideString decoding (BMP code points)\n");
	{
		wchar_t wide[16];
		size_t n = fplUTF8StringToWideString(asciiUtf8, 5, wide, fplArrayCount(wide));
		ftAssertSizeEquals(5, n);
		ftAssertU32Equals((uint32_t)'H', (uint32_t)wide[0]);
		ftAssertU32Equals((uint32_t)'o', (uint32_t)wide[4]);
		ftAssertU32Equals(0, (uint32_t)wide[5]);
	}
	{
		wchar_t wide[16];
		size_t n = fplUTF8StringToWideString(umlautUtf8, 2, wide, fplArrayCount(wide));
		ftAssertSizeEquals(1, n);
		ftAssertU32Equals(codePointUmlaut, (uint32_t)wide[0]);
		ftAssertU32Equals(0, (uint32_t)wide[1]);
	}
	{
		wchar_t wide[16];
		size_t n = fplUTF8StringToWideString(euroUtf8, 3, wide, fplArrayCount(wide));
		ftAssertSizeEquals(1, n);
		ftAssertU32Equals(codePointEuro, (uint32_t)wide[0]);
	}

	ftMsg("Test WideString to UTF-8 encoding (BMP code points)\n");
	{
		wchar_t wide[] = { (wchar_t)codePointA, 0 };
		char utf8[16];
		size_t n = fplWideStringToUTF8String(wide, 1, utf8, fplArrayCount(utf8));
		ftAssertSizeEquals(1, n);
		ftAssertStringEquals("A", utf8);
	}
	{
		wchar_t wide[] = { (wchar_t)codePointUmlaut, 0 };
		char utf8[16];
		size_t n = fplWideStringToUTF8String(wide, 1, utf8, fplArrayCount(utf8));
		ftAssertSizeEquals(2, n);
		ftAssertStringEquals(umlautUtf8, utf8);
	}
	{
		wchar_t wide[] = { (wchar_t)codePointEuro, 0 };
		char utf8[16];
		size_t n = fplWideStringToUTF8String(wide, 1, utf8, fplArrayCount(utf8));
		ftAssertSizeEquals(3, n);
		ftAssertStringEquals(euroUtf8, utf8);
	}

	ftMsg("Test UTF-8 -> WideString -> UTF-8 round-trip (1 to 4 byte sequences)\n");
	{
		CheckUtf8RoundTrip(asciiUtf8, 5);
		CheckUtf8RoundTrip(umlautUtf8, 2);
		CheckUtf8RoundTrip(euroUtf8, 3);
		CheckUtf8RoundTrip(emojiUtf8, 4);
		CheckUtf8RoundTrip(mixedUtf8, 10);
	}

	ftMsg("Test insufficient destination buffer returns zero\n");
	{
		// 5 code points need room for 5 + NUL.
		wchar_t tooSmall[1];
		ftAssertSizeEquals(0, fplUTF8StringToWideString(asciiUtf8, 5, tooSmall, fplArrayCount(tooSmall)));
		wchar_t exactNoNul[5];
		ftAssertSizeEquals(0, fplUTF8StringToWideString(asciiUtf8, 5, exactNoNul, fplArrayCount(exactNoNul)));
		wchar_t justEnough[6];
		ftAssertSizeEquals(5, fplUTF8StringToWideString(asciiUtf8, 5, justEnough, fplArrayCount(justEnough)));
	}
	{
		// '€' encodes to 3 bytes and still needs room for the NUL.
		wchar_t wide[] = { (wchar_t)codePointEuro, 0 };
		char tooSmall[3];
		ftAssertSizeEquals(0, fplWideStringToUTF8String(wide, 1, tooSmall, fplArrayCount(tooSmall)));
		char justEnough[4];
		ftAssertSizeEquals(3, fplWideStringToUTF8String(wide, 1, justEnough, fplArrayCount(justEnough)));
	}

#if defined(FPL_SUBPLATFORM_STD_STRINGS)
	// The POSIX implementation rejects malformed UTF-8; the Win32 codepage path substitutes U+FFFD instead.
	ftMsg("Test malformed UTF-8 returns zero (POSIX)\n");
	{
		static const char loneContinuation[] = { (char)0x80, 0 };
		ftAssertSizeEquals(0, fplUTF8StringToWideString(loneContinuation, 1, fpl_null, 0));
		static const char truncatedLead[] = { (char)0xC3, 0 };
		ftAssertSizeEquals(0, fplUTF8StringToWideString(truncatedLead, 1, fpl_null, 0));
	}
#endif
}

static void TestLocalization(void) {
	fplPlatformInit(fplInitFlags_None, fpl_null);
	char buffer[16];
	ftAssert(fplGetSystemLocale(fplLocaleFormat_ISO639, buffer, fplArrayCount(buffer)) > 0);
	fplConsoleFormatOut("System Locale (ISO-639): %s\n", buffer);
	ftAssert(fplGetUserLocale(fplLocaleFormat_ISO639, buffer, fplArrayCount(buffer)) > 0);
	fplConsoleFormatOut("User Locale (ISO-639): %s\n", buffer);
	ftAssert(fplGetInputLocale(fplLocaleFormat_ISO639, buffer, fplArrayCount(buffer)) > 0);
	fplConsoleFormatOut("Input Locale (ISO-639): %s\n", buffer);
	fplPlatformRelease();
}

static inline void DefaultInlineTest(void) {
	fplConsoleFormatOut("This should be inlined");
}
fpl_force_inline void ForceInlineTest(void) {
	fplConsoleFormatOut("This should be always inlined");
}
fpl_no_inline void NoInlineTest(void) {
	fplConsoleFormatOut("This should not be inlined");
}

static void TestInlining(void) {
	DefaultInlineTest();
	ForceInlineTest();
	NoInlineTest();
}

static void TestTimes(void) {
	ftMsg("Test fplTimestampQuery and fplTimestampElapsed\n");
	// 0.5 secs
	{
		fplTimestamp start = fplTimestampQuery();
		fplThreadSleep(500);
		fplTimestamp ende = fplTimestampQuery();
		double delta = fplTimestampElapsed(start, ende);
		ftAssert(delta >= 0.5 && delta < 0.75);
	}

	// 1.0 secs
	{
		fplTimestamp start = fplTimestampQuery();
		fplThreadSleep(750);
		fplTimestamp ende = fplTimestampQuery();
		double delta = fplTimestampElapsed(start, ende);
		ftAssert(delta >= 0.75 && delta < 1.0);
	}

	// 1.5 secs
	{
		fplTimestamp start = fplTimestampQuery();
		fplThreadSleep(1500);
		fplTimestamp ende = fplTimestampQuery();
		double delta = fplTimestampElapsed(start, ende);
		ftAssert(delta >= 1.5 && delta < 1.75);
	}
}

// Smoke test for the multi-backend gamepad merge in fpl__InputSystem_PollGamepad. With no real controller plugged in, every disconnected slot must come back zeroed. Verifies the system layer clears outStates once and that backends do not leave stale data.
static void TestGamepadPollMerge() {
	ftMsg("Test fplPollGamepadStates merge contract\n");
	if (!fplPlatformInit(fplInitFlags_Gamepad, fpl_null)) {
		ftMsg("  skipped: gamepad init failed\n");
		return;
	}
	fplGamepadStates states;
	for (size_t i = 0; i < fplArrayCount(states.deviceStates); ++i) {
		states.deviceStates[i].isConnected = true;
		states.deviceStates[i].leftStickX = 0.5f;
	}
	fplPollGamepadStates(&states);
	for (size_t i = 0; i < fplArrayCount(states.deviceStates); ++i) {
		const fplGamepadState *s = &states.deviceStates[i];
		if (s->isConnected) {
			continue;
		}
		ftAssert(s->leftStickX == 0.0f);
		ftAssert(s->leftStickY == 0.0f);
		ftAssert(s->rightStickX == 0.0f);
		ftAssert(s->rightStickY == 0.0f);
		ftAssert(s->leftTrigger == 0.0f);
		ftAssert(s->rightTrigger == 0.0f);
	}
	fplPlatformRelease();
}

static void TestProcess(void) {
	ftMsg("Process tests\n");
	if (!fplPlatformInit(fplInitFlags_None, fpl_null)) {
		ftFail("Failed to initialize platform");
		return;
	}
	FPLProcessTests_All();
	fplPlatformRelease();
}

static void TestSecurity(void) {
	ftMsg("Security & stability tests\n");
	if (!fplPlatformInit(fplInitFlags_None, fpl_null)) {
		ftFail("Failed to initialize platform");
		return;
	}
	FPLSecurityTests_Strings();
	FPLSecurityTests_Paths();
	FPLSecurityTests_Files();
	FPLSecurityTests_Conversions();
	FPLSecurityTests_Types();
	FPLSecurityTests_Misc();
	fplPlatformRelease();
}

int main(int argc, char *args[]) {
	// The process tests start this executable again in one of the child modes, that mode replaces the whole test run
	ProcessTestChildMode childMode = ProcessTestsGetChildMode(argc, args);
	if (childMode != ProcessTestChildMode_None) {
		int childExitCode = ProcessTestsRunAsChild(childMode, argc, args);
		return childExitCode;
	}
	TestColdInit();
	TestInit();
	TestSizes();
	TestMacros();
	TestInlining();
	TestSecurity();
	TestStrings();
	TestUnicodeConversion();
	TestLocalization();
	TestMemory();
	TestOSInfos();
	TestHardware();
	TestTimes();
	TestPaths();
	TestFiles();
	TestAtomics();
	TestThreading();
	TestProcess();
	TestGamepadPollMerge();
	return 0;
}
