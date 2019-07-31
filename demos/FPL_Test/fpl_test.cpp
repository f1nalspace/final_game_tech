/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Test

Description:
	This demo is used to test all the things. It is basically a unit-test.

Requirements:
	- C++ Compiler
	- Final Platform Layer

Author:
	Torsten Spaete

Changelog:
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
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#define FPL_NO_AUDIO
#define FPL_NO_VIDEO
#define FPL_NO_WINDOW
#define FPL_LOGGING
#include <final_platform_layer.h>

#define FT_IMPLEMENTATION
#include "final_test.h"

// C++ typeid
#include <typeinfo>

template<typename T>
inline void AssertEquals(const T expected, const T actual) {}

template<>
inline void AssertEquals(const uint32_t expected, const uint32_t actual) {
	ftAssertU32Equals(expected, actual);
}
template<>
inline void AssertEquals(const uint64_t expected, const uint64_t actual) {
	ftAssertU64Equals(expected, actual);
}
template<>
inline void AssertEquals(const int32_t expected, const int32_t actual) {
	ftAssertS32Equals(expected, actual);
}
template<>
inline void AssertEquals(const int64_t expected, const int64_t actual) {
	ftAssertS64Equals(expected, actual);
}
template<>
inline void AssertEquals(const double expected, const double actual) {
	ftAssertDoubleEquals(expected, actual);
}
template<>
inline void AssertEquals(const float expected, const float actual) {
	ftAssertFloatEquals(expected, actual);
}

static void TestColdInit() {
	ftMsg("Test Cold-Initialize of InitPlatform\n");
	{
		size_t errorCount = fplGetErrorCount();
		ftAssertSizeEquals(0, errorCount);
		bool inited = fplPlatformInit(fplInitFlags_None, nullptr);
		ftAssert(inited);
		fplPlatformResultType resultType = fplGetPlatformResult();
		ftAssert(resultType == fplPlatformResultType_Success);
		const char* errorStr = fplGetLastError();
		ftAssertStringEquals("", errorStr);
		fplPlatformRelease();
	}
}

static void TestInit() {
	ftMsg("Test InitPlatform with All init flags\n");
	{
		fplClearErrors();
		bool inited = fplPlatformInit(fplInitFlags_All, nullptr);
		ftAssert(inited);
		fplPlatformResultType resultType = fplGetPlatformResult();
		ftAssert(resultType == fplPlatformResultType_Success);
		const char* errorStr = fplGetLastError();
		ftAssertStringEquals("", errorStr);
		fplPlatformRelease();
	}
	ftMsg("Test InitPlatform with None init flags\n");
	{
		fplClearErrors();
		bool inited = fplPlatformInit(fplInitFlags_None, fpl_null);
		ftAssert(inited);
		fplPlatformResultType resultType = fplGetPlatformResult();
		ftAssert(resultType == fplPlatformResultType_Success);
		const fplSettings* settings = fplGetCurrentSettings();
		ftIsNotNull(settings);
		const char* errorStr = fplGetLastError();
		ftAssertStringEquals("", errorStr);
		fplPlatformRelease();
	}
	ftMsg("Test fplGetCurrentSettings in non-initialized state\n");
	{
		ftIsFalse(fpl__global__InitState.isInitialized);
		fplClearErrors();
		const fplSettings* settings = fplGetCurrentSettings();
		ftIsNull(settings);
		size_t errorCount = fplGetErrorCount();
		ftAssertSizeEquals(1, errorCount);
		const char* errorStr = fplGetLastError();
		ftAssertStringNotEquals("", errorStr);
	}
}

static void TestOSInfos() {
	ftMsg("Get Platform Type:\n");
	{
		fplPlatformType platType = fplGetPlatformType();
		ftAssert(fplPlatformType_Unknown != platType);
		fplConsoleFormatOut("\tPlatform: %s\n", fplGetPlatformName(platType));
	}
	ftMsg("Get OS Type:\n");
	{
		fplOSInfos osInfos = {};
		bool r = fplGetOperatingSystemInfos(&osInfos);
		ftIsTrue(r);
		fplConsoleFormatOut("\tName: %s\n", osInfos.osName);
		fplConsoleFormatOut("\tVersion: %s.%s.%s.%s\n", osInfos.osVersion.major, osInfos.osVersion.minor, osInfos.osVersion.fix, osInfos.osVersion.build);
		fplConsoleFormatOut("\tDistribution Name: %s\n", osInfos.distributionName);
		fplConsoleFormatOut("\tDistribution Version: %s.%s.%s.%s\n", osInfos.distributionVersion.major, osInfos.distributionVersion.minor, osInfos.distributionVersion.fix, osInfos.distributionVersion.build);
	}
	ftMsg("Get User Infos:\n");
	{
		char nameBuffer[256] = {};
		bool r = fplGetCurrentUsername(nameBuffer, fplArrayCount(nameBuffer));
		ftIsTrue(r);
		fplConsoleFormatOut("\tCurrent Username: %s\n", nameBuffer);
	}
}

static void TestSizes() {
	// @NOTE(final): This may be pretty useless, because stdint.h guarantees the size
	ftExpects(1, sizeof(uint8_t));
	ftExpects(1, sizeof(int8_t));
	ftExpects(2, sizeof(uint16_t));
	ftExpects(2, sizeof(int16_t));
	ftExpects(4, sizeof(uint32_t));
	ftExpects(4, sizeof(int32_t));
	ftExpects(8, sizeof(uint64_t));
	ftExpects(8, sizeof(int64_t));
#if defined(FT_ARCH_X64)
	ftExpects(8, sizeof(intptr_t));
	ftExpects(8, sizeof(uintptr_t));
	ftExpects(8, sizeof(size_t));
#else
	ftExpects(4, sizeof(intptr_t));
	ftExpects(4, sizeof(uintptr_t));
	ftExpects(4, sizeof(size_t));
#endif
}

static void TestMacros() {
	//
	// fplArrayCount
	//
	ftMsg("[fplArrayCount] Test static char array\n");
	{
		char staticArray[137] = {};
		uint32_t actual = fplArrayCount(staticArray);
		ftExpects(137, actual);
	}
	ftMsg("[fplArrayCount] Test static int array\n");
	{
		int staticArray[349] = {};
		uint32_t actual = fplArrayCount(staticArray);
		ftExpects(349, actual);
	}
	ftMsg("[fplArrayCount] Test static bool array\n");
	{
		bool staticArray[961] = {};
		uint32_t actual = fplArrayCount(staticArray);
		ftExpects(961, actual);
	}
	ftMsg("[fplArrayCount] Test static void pointer array\n");
	{
		void* staticArray[35] = {};
		uint32_t actual = fplArrayCount(staticArray);
		ftExpects(35, actual);
	}

	// @NOTE(final): This is a simple/stupid macro, so when you pass a pointer, you basically get 2 always
	ftMsg("[fplArrayCount] Test nullptr\n");
	{
		int* emptyArray = nullptr;
		uint32_t actual = fplArrayCount(emptyArray);
		uint32_t expected = sizeof(int*) / sizeof(int);
		ftExpects(expected, actual);
	}
	ftMsg("[fplArrayCount] Test pointer from references static array\n");
	{
		int staticArray[3] = {};
		int* refArray = &staticArray[0];
		uint32_t actual = fplArrayCount(refArray);
		uint32_t expected = sizeof(int*) / sizeof(int);
		ftExpects(expected, actual);
	}

	//
	// fplOffsetOf
	//
	ftMsg("[fplOffsetOf] Test alignment of 4 (High to low)\n");
	{
#	pragma pack(push, 4)
		struct TestStruct {
			uint64_t a;
			uint32_t b;
			uint16_t c;
			uint8_t d;
		};
#	pragma pack(pop)
		ftExpects(0, fplOffsetOf(TestStruct, a));
		ftExpects(8, fplOffsetOf(TestStruct, b));
		ftExpects(12, fplOffsetOf(TestStruct, c));
		ftExpects(14, fplOffsetOf(TestStruct, d));
	}

	ftMsg("[fplOffsetOf] Test alignment of 4 (Low to High)\n");
	{
#	pragma pack(push, 4)
		struct TestStruct {
			uint8_t a;
			uint16_t b;
			uint32_t c;
			uint64_t d;
		};
#	pragma pack(pop)
		ftExpects(0, fplOffsetOf(TestStruct, a));
		ftExpects(2, fplOffsetOf(TestStruct, b));
		ftExpects(4, fplOffsetOf(TestStruct, c));
		ftExpects(8, fplOffsetOf(TestStruct, d));
	}

	ftMsg("[fplOffsetOf] Test alignment of 8 (Low to High)\n");
	{
#	pragma pack(push, 8)
		struct TestStruct {
			uint8_t a;
			uint16_t b;
			uint8_t c[3];
			uint64_t d;
		};
#	pragma pack(pop)
		ftExpects(0, fplOffsetOf(TestStruct, a));
		ftExpects(2, fplOffsetOf(TestStruct, b));
		ftExpects(4, fplOffsetOf(TestStruct, c));
		ftExpects(8, fplOffsetOf(TestStruct, d));
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
		struct TestStruct {
			int a;
			int b;
		};
		TestStruct instance = { 3, 7 };
		TestStruct* instancePtr = &instance;
		ftAssertS32Equals(3, fplMin(instancePtr->a, instancePtr->b));
	}
	ftMsg("[fplMin] Test floats\n");
	{
		ftAssertFloatEquals(3.0f, fplMin(3.0f, 7.0f));
		ftAssertFloatEquals(3.0f, fplMin(7.0f, 3.0f));
		ftAssertFloatEquals(-7.0f, fplMin(-7.0f, -3.0f));
		ftAssertFloatEquals(-7.0f, fplMin(-3.0f, -7.0f));
		struct TestStruct {
			float a;
			float b;
		};
		TestStruct instance = { 3.0f, 7.0f };
		TestStruct* instancePtr = &instance;
		ftAssertFloatEquals(3.0f, fplMin(instancePtr->a, instancePtr->b));
	}
	ftMsg("[fplMax] Test integers\n");
	{
		ftAssertS32Equals(7, fplMax(3, 7));
		ftAssertS32Equals(7, fplMax(7, 3));
		ftAssertS32Equals(-3, fplMax(-3, -7));
		ftAssertS32Equals(-3, fplMax(-7, -3));
		struct TestStruct {
			int a;
			int b;
		};
		TestStruct instance = { 3, 7 };
		TestStruct* instancePtr = &instance;
		ftAssertS32Equals(7, fplMax(instancePtr->a, instancePtr->b));
	}
	ftMsg("[fplMax] Test floats\n");
	{
		ftAssertFloatEquals(7.0f, fplMax(3.0f, 7.0f));
		ftAssertFloatEquals(7.0f, fplMax(7.0f, 3.0f));
		ftAssertFloatEquals(-3.0f, fplMax(-3.0f, -7.0f));
		ftAssertFloatEquals(-3.0f, fplMax(-7.0f, -3.0f));
		struct TestStruct {
			float a;
			float b;
		};
		TestStruct instance = { 3.0f, 7.0f };
		TestStruct* instancePtr = &instance;
		ftAssertFloatEquals(7.0f, fplMax(instancePtr->a, instancePtr->b));
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

static void TestMemory() {
	ftMsg("Test normal allocation and deallocation\n");
	{
		size_t memSize = fplKiloBytes(42);
		uint8_t* mem = (uint8_t*)fplMemoryAllocate(memSize);
		for (size_t i = 0; i < memSize; ++i) {
			uint8_t value = *mem++;
			ftAssertU8Equals(0, value);
		}
		fplMemoryFree(mem);
	}
	{
		size_t memSize = fplMegaBytes(512);
		void* mem = fplMemoryAllocate(memSize);
		ftIsNotNull(mem);
		fplMemoryFree(mem);
	}

	ftMsg("Test aligned allocation and deallocation\n");
	{
		size_t memSize = fplKiloBytes(42);
		uint8_t* mem = (uint8_t*)fplMemoryAlignedAllocate(memSize, 16);
		for (size_t i = 0; i < memSize; ++i) {
			uint8_t value = *(mem + i);
			ftAssertU8Equals(0, value);
		}
		fplMemoryAlignedFree(mem);
	}
	{
		size_t memSize = fplMegaBytes(512);
		void* mem = fplMemoryAlignedAllocate(memSize, 16);
		ftIsNotNull(mem);
		fplMemoryAlignedFree(mem);
	}

	ftMsg("Test memory clear\n");
	{
		size_t memSize = 100;
		uint8_t* mem = (uint8_t*)fplMemoryAllocate(memSize);
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
		uint8_t* mem = (uint8_t*)fplMemoryAllocate(memSize);
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

static void TestPaths() {
	if (fplPlatformInit(fplInitFlags_None, fpl_null)) {

		char homePathBuffer[1024] = {};
		fplGetHomePath(homePathBuffer, fplArrayCount(homePathBuffer));
		ftMsg("Home Path:\n%s\n", homePathBuffer);

		char exeFilePathBuffer[1024] = {};
		fplGetExecutableFilePath(exeFilePathBuffer, fplArrayCount(exeFilePathBuffer));
		ftMsg("Executable file Path:\n%s\n", exeFilePathBuffer);

		char extractedPathBuffer[1024] = {};
		fplExtractFilePath(exeFilePathBuffer, extractedPathBuffer, fplArrayCount(extractedPathBuffer));
		ftMsg("Extracted path:\n%s\n", extractedPathBuffer);

		const char* exeFileName = fplExtractFileName(exeFilePathBuffer);
		ftMsg("Extracted filename:\n%s\n", exeFileName);

		const char* exeFileExt = fplExtractFileExtension(exeFilePathBuffer);
		ftMsg("Extracted extension:\n%s\n", exeFileExt);

		char combinedPathBuffer[1024 * 10] = {};
		fplPathCombine(combinedPathBuffer, fplArrayCount(combinedPathBuffer), 4, "Hallo", "Welt", "der", "Programmierer");
		ftMsg("Combined path:\n%s\n", combinedPathBuffer);

		char changedFileExtBuffer[1024] = {};
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


static void TestHardware() {
	char cpuNameBuffer[1024] = {};
	fplGetProcessorName(cpuNameBuffer, fplArrayCount(cpuNameBuffer));
	ftMsg("Processor name: %s\n", cpuNameBuffer);

	size_t coreCount = fplGetProcessorCoreCount();
	ftAssert(coreCount > 0);
	ftMsg("Processor cores: %zu\n", coreCount);

	fplMemoryInfos memInfos = {};
	fplGetRunningMemoryInfos(&memInfos);
	ftMsg("Installed physical memory (bytes): %llu\n", memInfos.totalPhysicalSize);
	ftMsg("Total physical memory (bytes): %llu\n", memInfos.totalPhysicalSize);
	ftMsg("Available physical memory (bytes): %llu\n", memInfos.freePhysicalSize);
	ftMsg("Total cache memory (bytes): %llu\n", memInfos.totalCacheSize);
	ftMsg("Available cache memory (bytes): %llu\n", memInfos.freeCacheSize);
	ftMsg("Page size (bytes): %llu\n", memInfos.pageSize);
	ftMsg("Total number of memory pages: %llu\n", memInfos.totalPageCount);
	ftMsg("Available number memory pages: %llu\n", memInfos.freePageCount);

	fplArchType archType = fplGetProcessorArchitecture();
	const char* archStr = fplGetArchTypeString(archType);
	ftMsg("Processor archicture: %s\n", archStr);
}

static void EmptyThreadproc(const fplThreadHandle* context, void* data) {
}

struct ThreadData {
	fplThreadHandle* thread;
	int num;
	int sleepFor;
};

static void SingleThreadProc(const fplThreadHandle* context, void* data) {
	ThreadData* d = (ThreadData*)data;
	ftMsg("Sleep in thread %d for %d ms\n", d->num, d->sleepFor);
	fplThreadSleep(d->sleepFor);
}

static void SimpleMultiThreadTest(const size_t threadCount) {
	ftLine();
	ThreadData threadData[FPL__MAX_THREAD_COUNT] = {};
	for (size_t threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
		threadData[threadIndex].num = (int)(threadIndex + 1);
		threadData[threadIndex].sleepFor = (int)(1 + threadIndex) * 500;
	}
	ftMsg("Start %d threads\n", threadCount);
	for (size_t threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
		threadData[threadIndex].thread = fplThreadCreate(SingleThreadProc, &threadData[threadIndex]);
	}
	ftMsg("Wait all %d threads for exit\n", threadCount);
	fplThreadWaitForAll(&threadData[0].thread, threadCount, sizeof(ThreadData), FPL_TIMEOUT_INFINITE);
	ftMsg("All %d threads are done\n", threadCount);

	ftMsg("Terminate %d threads\n", threadCount);
	for (size_t threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
		ftExpects(fplThreadState_Stopped, threadData[threadIndex].thread->currentState);
		fplThreadTerminate(threadData[threadIndex].thread);
	}
}

struct MutableThreadData {
	fplSemaphoreHandle semaphore;
	volatile int32_t value;
};

struct WriteThreadData {
	ThreadData base;
	MutableThreadData* data;
	int32_t valueToWrite;
};

struct ReadThreadData {
	ThreadData base;
	MutableThreadData* data;
	int32_t expectedValue;
};

static void WriteDataThreadProc(const fplThreadHandle* context, void* data) {
	WriteThreadData* d = (WriteThreadData*)data;
	ftMsg("Sleep in thread %d for %d ms\n", d->base.num, d->base.sleepFor);
	fplThreadSleep(d->base.sleepFor);
	fplAtomicStoreS32(&d->data->value, d->valueToWrite);
}

static void ReadDataThreadProc(const fplThreadHandle* context, void* data) {
	ReadThreadData* d = (ReadThreadData*)data;
	ftMsg("Sleep in thread %d for %d ms\n", d->base.num, d->base.sleepFor);
	fplThreadSleep(d->base.sleepFor);
	int32_t actualValue = fplAtomicLoadS32(&d->data->value);
	ftExpects(d->expectedValue, actualValue);
}

static void SyncThreadsTestAtomics() {
	ftLine();
	ftMsg("Sync test for 1 reader and 1 writer using atomics\n");
	{
		MutableThreadData mutableData = {};
		mutableData.value = 0;

		ReadThreadData readData = {};
		readData.base.num = 2;
		readData.base.sleepFor = 5000;
		readData.data = &mutableData;
		readData.expectedValue = 42;

		WriteThreadData writeData = {};
		writeData.base.num = 1;
		writeData.base.sleepFor = 3000;
		writeData.data = &mutableData;
		writeData.valueToWrite = 42;

		fplThreadHandle* threads[2];
		uint32_t threadCount = fplArrayCount(threads);

		ftMsg("Start %zu threads\n", threadCount);
		threads[0] = fplThreadCreate(ReadDataThreadProc, &readData);
		threads[1] = fplThreadCreate(WriteDataThreadProc, &writeData);

		ftMsg("Wait for %zu threads to exit\n", threadCount);
		fplThreadWaitForAll(threads, threadCount, sizeof(fplThreadHandle*), FPL_TIMEOUT_INFINITE);

		ftMsg("Release resources for %zu threads\n", threadCount);
		for (uint32_t index = 0; index < threadCount; ++index) {
			ftExpects(fplThreadState_Stopped, threads[index]->currentState);
			fplThreadTerminate(threads[index]);
		}
	}
}

static void WriteDataSemaphoreThreadProc(const fplThreadHandle* context, void* data) {
	WriteThreadData* d = (WriteThreadData*)data;
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
		MutableThreadData mutableData = {};
		uint32_t initialValue = (uint32_t)numWriters - 1;
		ftIsTrue(fplSemaphoreInit(&mutableData.semaphore, initialValue));
		mutableData.value = 0;

		WriteThreadData writeDatas[FPL__MAX_THREAD_COUNT] = {};
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
			ftExpects(fplThreadState_Stopped, writeDatas[index].base.thread->currentState);
			fplThreadTerminate(writeDatas[index].base.thread);
		}
		fplSemaphoreDestroy(&mutableData.semaphore);
	}
}

enum class ConditionTestType {
	Signal,
	ConditionSignal
};

struct SlaveThreadData {
	ThreadData base;
	fplSignalHandle signal;
	fplConditionVariable condition;
	fplMutexHandle mutex;
	ConditionTestType testType;
	bool isSuccess;
};

struct MasterThreadData {
	ThreadData base;
	SlaveThreadData* slaveThreads;
	uint32_t slaveCount;
	ConditionTestType testType;
};

static void ThreadSlaveProc(const fplThreadHandle* context, void* data) {
	SlaveThreadData* d = (SlaveThreadData*)data;

	if (d->testType == ConditionTestType::Signal) {
		ftMsg("Slave-Thread %d waits for signal\n", d->base.num);
		fplSignalWaitForOne(&d->signal, FPL_TIMEOUT_INFINITE);
		d->isSuccess = true;
		ftMsg("Got signal on Slave-Thread %d\n", d->base.num);
	} else if (d->testType == ConditionTestType::ConditionSignal) {
		ftMsg("Slave-Thread %d waits on condition\n", d->base.num);
		fplConditionWait(&d->condition, &d->mutex, FPL_TIMEOUT_INFINITE);
		d->isSuccess = true;
		ftMsg("Got condition on Slave-Thread %d\n", d->base.num);
	}

	ftMsg("Slave-Thread %d is done\n", d->base.num);
}

static void ThreadMasterProc(const fplThreadHandle* context, void* data) {
	MasterThreadData* d = (MasterThreadData*)data;
	ftMsg("Master-Thread %d waits for 5 seconds\n", d->base.num);
	fplThreadSleep(5000);

	for (uint32_t signalIndex = 0; signalIndex < d->slaveCount; ++signalIndex) {
		if (d->testType == ConditionTestType::Signal) {
			ftMsg("Master-Thread %d sets signal %d\n", d->base.num, signalIndex);
			fplSignalSet(&d->slaveThreads[signalIndex].signal);
		} else if (d->testType == ConditionTestType::ConditionSignal) {
			ftMsg("Master-Thread %d sends signal to condition %d\n", d->base.num, signalIndex);
			fplConditionSignal(&d->slaveThreads[signalIndex].condition);
		}
	}

	ftMsg("Master-Thread %d is done\n", d->base.num);
}

static void ConditionThreadsTest(const size_t threadCount, const ConditionTestType testType) {
	ftAssert(threadCount > 1);

	ftLine();

	if (testType == ConditionTestType::Signal) {
		ftMsg("Signals test for %zu threads\n", threadCount);
	} else if (testType == ConditionTestType::ConditionSignal) {
		ftMsg("Condition-Variable (Single) test for %zu threads\n", threadCount);
	}

	MasterThreadData masterData = {};
	masterData.base.num = 1;
	masterData.testType = testType;

	SlaveThreadData slaveDatas[FPL__MAX_THREAD_COUNT] = {};
	size_t slaveThreadCount = threadCount - 1;
	for (size_t threadIndex = 0; threadIndex < slaveThreadCount; ++threadIndex) {
		slaveDatas[threadIndex].base.num = masterData.base.num + (int)threadIndex + 1;
		slaveDatas[threadIndex].testType = testType;
		if (testType == ConditionTestType::Signal) {
			ftIsTrue(fplSignalInit(&slaveDatas[threadIndex].signal, fplSignalValue_Unset));
		} else if (testType == ConditionTestType::ConditionSignal) {
			ftIsTrue(fplMutexInit(&slaveDatas[threadIndex].mutex));
			ftIsTrue(fplConditionInit(&slaveDatas[threadIndex].condition));
		}
		size_t i = masterData.slaveCount++;
	}
	masterData.slaveThreads = slaveDatas;

	ftMsg("Start %zu slave threads, 1 master thread\n", slaveThreadCount);
	fplThreadHandle* threads[FPL__MAX_THREAD_COUNT];
	for (size_t threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
		if (threadIndex == 0) {
			threads[threadIndex] = fplThreadCreate(ThreadMasterProc, &masterData);
		} else {
			threads[threadIndex] = fplThreadCreate(ThreadSlaveProc, &slaveDatas[threadIndex - 1]);
		}
	}

	ftMsg("Wait for %zu threads to exit\n", threadCount);
	fplThreadWaitForAll(threads, threadCount, sizeof(fplThreadHandle*), FPL_TIMEOUT_INFINITE);

	ftMsg("Release resources for %zu threads\n", threadCount);
	for (size_t slaveIndex = 0; slaveIndex < slaveThreadCount; ++slaveIndex) {
		ftIsTrue(slaveDatas[slaveIndex].isSuccess);
	}
	for (size_t threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
		fplThreadHandle* thread = threads[threadIndex];
		ftExpects(fplThreadState_Stopped, thread->currentState);
	}
	for (size_t slaveIndex = 0; slaveIndex < slaveThreadCount; ++slaveIndex) {
		if (testType == ConditionTestType::Signal) {
			fplSignalDestroy(&slaveDatas[slaveIndex].signal);
		} else if (testType == ConditionTestType::ConditionSignal) {
			fplConditionDestroy(&slaveDatas[slaveIndex].condition);
			fplMutexDestroy(&slaveDatas[slaveIndex].mutex);
		}
	}
}

static void TestThreading() {
	if (fplPlatformInit(fplInitFlags_None, fpl_null)) {
		//
		// Single threading test
		//
		ftLine();
		ftMsg("Test 1 empty thread\n");
		{
			fplThreadHandle* thread;
			ftMsg("Start thread\n");
			thread = fplThreadCreate(EmptyThreadproc, nullptr);
			ftMsg("Wait thread for exit\n");
			fplThreadWaitForOne(thread, UINT32_MAX);
			ftMsg("Thread is done\n");
			ftExpects(fplThreadState_Stopped, thread->currentState);
			fplThreadTerminate(thread);
		}

		ftLine();
		ftMsg("Test 1 sleeping-thread\n");
		{
			ThreadData threadData = {};
			threadData.num = 1;
			threadData.sleepFor = 3000;
			ftMsg("Start thread %d\n", threadData.num);
			fplThreadHandle* thread = fplThreadCreate(SingleThreadProc, &threadData);
			ftMsg("Wait thread %d for exit\n", threadData.num);
			fplThreadWaitForOne(thread, UINT32_MAX);
			ftMsg("Thread %d is done\n", threadData.num);
			ftExpects(fplThreadState_Stopped, thread->currentState);
			fplThreadTerminate(thread);
		}

		//
		// Multi threads test
		//
		size_t coreCount = fplGetProcessorCoreCount();
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
			ConditionThreadsTest(2, ConditionTestType::Signal);
			ConditionThreadsTest(3, ConditionTestType::Signal);
			ConditionThreadsTest(4, ConditionTestType::Signal);
			ConditionThreadsTest(threadCountForCores, ConditionTestType::Signal);
		}

		//
		// Condition tests
		//
		{
			ConditionThreadsTest(2, ConditionTestType::ConditionSignal);
			ConditionThreadsTest(3, ConditionTestType::ConditionSignal);
			ConditionThreadsTest(4, ConditionTestType::ConditionSignal);
			ConditionThreadsTest(threadCountForCores, ConditionTestType::ConditionSignal);
		}

		fplPlatformRelease();
	}
}

static void TestFiles() {
#if defined(FPL_PLATFORM_WINDOWS)
	const char* testNotExistingFile = "C:\\Windows\\i_am_not_existing.lib";
	const char* testExistingFile = "C:\\Windows\\notepad.exe";
	const char* testRootPath = "C:\\";
	const char* testRootFilter = "Program*";
#else
	const char* testNotExistingFile = "/i_am_not_existing.whatever";
	const char* testExistingFile = "/usr/sbin/nologin";
	const char* testRootPath = "/";
	const char* testRootFilter = "us*";
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
		uint32_t emptySize = fplGetFileSizeFromPath32(testNotExistingFile);
		ftAssertU32Equals(0, emptySize);
		uint32_t existingSize = fplGetFileSizeFromPath32(testExistingFile);
		ftAssert(existingSize > 0);
	}
	ftMsg("Test Directory Iterations without filter\n");
	{
		fplFileEntry fileEntry = {};
		for (bool r = fplListDirBegin(testRootPath, "*.*", &fileEntry); r; r = fplListDirNext(&fileEntry)) {
			ftMsg("%s\n", fileEntry.name);
		}
		fplListDirEnd(&fileEntry);
	}
	ftMsg("Test Directory Iterations with all filter\n");
	{
		fplFileEntry fileEntry = {};
		for (bool r = fplListDirBegin(testRootPath, "*", &fileEntry); r; r = fplListDirNext(&fileEntry)) {
			ftMsg("%s\n", fileEntry.name);
		}
		fplListDirEnd(&fileEntry);
	}
	ftMsg("Test Directory Iterations with root filter '%s'\n", testRootFilter);
	{
		fplFileEntry fileEntry = {};
		bool r = fplListDirBegin(testRootPath, testRootFilter, &fileEntry);
		ftMsg("%s\n", fileEntry.name);
		ftIsTrue(r);
		fplListDirEnd(&fileEntry);
	}
}

static void TestAtomics() {
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
		const uint32_t exchangeValue = -1;
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
		const uint64_t exchangeValue = -1;
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
		void* initial = (void*)buffer;
		volatile void* value = initial;
		intptr_t addend = 16;
		void* actual = fplAtomicFetchAndAddPtr(&value, addend);
		ftAssertPointerEquals(initial, actual);
		ftAssertPointerEquals((void*)((intptr_t)initial + addend), (void*)value);
		size_t offset = (uintptr_t)value - (uintptr_t)initial;
		char c = buffer[offset];
		ftAssertCharEquals('A', c);
	}
	ftMsg("Test AtomicFetchAndAddPtr with 0\n");
	{
		char buffer[64];
		buffer[0] = 'A';
		void* initial = (void*)buffer;
		volatile void* value = initial;
		intptr_t addend = 0;
		void* actual = fplAtomicFetchAndAddPtr(&value, addend);
		ftAssertPointerEquals(initial, actual);
		ftAssertPointerEquals((void*)((intptr_t)initial + addend), (void*)value);
		size_t offset = (uintptr_t)value - (uintptr_t)initial;
		char c = buffer[offset];
		ftAssertCharEquals('A', c);
	}
	ftMsg("Test AtomicFetchAndAddPtr with -16\n");
	{
		char buffer[64];
		buffer[16] = 'A';
		void* initial = (void*)((intptr_t)buffer + 16);
		volatile void* value = initial;
		intptr_t addend = -16;
		void* actual = fplAtomicFetchAndAddPtr(&value, addend);
		ftAssertPointerEquals(initial, actual);
		ftAssertPointerEquals((void*)((intptr_t)initial + addend), (void*)value);
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
		const int64_t expected = initial + addend;
		volatile size_t value = initial;
		size_t actual = fplAtomicAddAndFetchSize(&value, addend);
		ftAssertSizeEquals(expected, actual);
		ftAssertSizeEquals(expected, (size_t)value);
	}
	ftMsg("Test AtomicAddAndFetchPtr with 16\n");
	{
		char buffer[64];
		buffer[16] = 'A';
		void* initial = (void*)buffer;
		intptr_t addend = 16;
		const void* expected = (void*)((intptr_t)initial + addend);
		volatile void* value = initial;
		void* actual = fplAtomicAddAndFetchPtr(&value, addend);
		ftAssertPointerEquals(expected, actual);
		ftAssertPointerEquals(expected, (void*)value);
		size_t offset = (uintptr_t)value - (uintptr_t)initial;
		char c = buffer[offset];
		ftAssertCharEquals('A', c);
	}
	ftMsg("Test AtomicAddAndFetchPtr with 0\n");
	{
		char buffer[64];
		buffer[0] = 'A';
		void* initial = (void*)buffer;
		intptr_t addend = 0;
		const void* expected = (void*)((intptr_t)initial + addend);
		volatile void* value = initial;
		void* actual = fplAtomicAddAndFetchPtr(&value, addend);
		ftAssertPointerEquals(expected, actual);
		ftAssertPointerEquals(expected, (void*)value);
		size_t offset = (uintptr_t)value - (uintptr_t)initial;
		char c = buffer[offset];
		ftAssertCharEquals('A', c);
	}
	ftMsg("Test AtomicAddAndFetchPtr with -16\n");
	{
		char buffer[64];
		buffer[16] = 'A';
		void* initial = (void*)((intptr_t)buffer + 16);
		intptr_t addend = -16;
		const void* expected = (void*)((intptr_t)initial + addend);
		volatile void* value = initial;
		void* actual = fplAtomicAddAndFetchPtr(&value, addend);
		ftAssertPointerEquals(expected, actual);
		ftAssertPointerEquals(expected, (void*)value);
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
		ftAssertS32Equals(expected, (uint32_t)value);
	}
	ftMsg("Test AtomicIncrementS32 with -35\n");
	{
		const int32_t initial = INT16_MAX - 35L;
		const int32_t expected = initial + 1L;
		volatile int32_t value = initial;
		int32_t actual = fplAtomicIncrementS32(&value);
		ftAssertS32Equals(expected, actual);
		ftAssertS32Equals(expected, (uint32_t)value);
	}
	ftMsg("Test AtomicIncrementS64 with 35\n");
	{
		const int64_t initial = INT32_MAX + 35LL;
		const int64_t expected = initial + 1LL;
		volatile int64_t value = initial;
		int64_t actual = fplAtomicIncrementS64(&value);
		ftAssertS64Equals(expected, actual);
		ftAssertS64Equals(expected, (uint64_t)value);
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
}

static void TestStrings() {
	ftMsg("Test ansi string length\n");
	{
		size_t actual = fplGetStringLength(nullptr);
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
		bool res = fplIsStringEqual(nullptr, nullptr);
		ftExpects(true, res);
	}
	{
		bool res = fplIsStringEqual(nullptr, "");
		ftExpects(false, res);
	}
	{
		bool res = fplIsStringEqual("B", "A");
		ftExpects(false, res);
	}
	{
		bool res = fplIsStringEqual("A", "A");
		ftExpects(true, res);
	}
	{
		bool res = fplIsStringEqual("Hello", "World");
		ftExpects(false, res);
	}
	{
		bool res = fplIsStringEqual("World", "World");
		ftExpects(true, res);
	}
	{
		bool res = fplIsStringEqualLen(nullptr, 0, nullptr, 0);
		ftExpects(false, res);
	}
	{
		bool res = fplIsStringEqualLen("", 0, nullptr, 0);
		ftExpects(false, res);
	}
	{
		bool res = fplIsStringEqualLen(nullptr, 0, "", 0);
		ftExpects(false, res);
	}
	{
		bool res = fplIsStringEqualLen("", 0, "", 0);
		ftExpects(true, res);
	}
	{
		bool res = fplIsStringEqualLen("B", 1, "A", 1);
		ftExpects(false, res);
	}
	{
		bool res = fplIsStringEqualLen("A", 1, "A", 1);
		ftExpects(true, res);
	}
	{
		bool res = fplIsStringEqualLen("A", 1, "A", 0);
		ftExpects(false, res);
	}
	{
		bool res = fplIsStringEqualLen("A", 1, "B", 1);
		ftExpects(false, res);
	}
	{
		bool res = fplIsStringEqualLen("Hello", 5, "World", 5);
		ftExpects(false, res);
	}
	{
		bool res = fplIsStringEqualLen("Hello", 3, "World", 5);
		ftExpects(false, res);
	}
	{
		bool res = fplIsStringEqualLen("World", 5, "Hello", 3);
		ftExpects(false, res);
	}
	{
		bool res = fplIsStringEqualLen("Hello", 5, "Hello", 5);
		ftExpects(true, res);
	}
	{
		bool res = fplIsStringEqualLen("Hello", 3, "Hello", 3);
		ftExpects(true, res);
	}

	ftMsg("Test append string\n");
	{
		ftIsNull(fplStringAppend(fpl_null, fpl_null, 0));
	}
	{
		char buffer[64] = {};
		fplStringAppend(fpl_null, buffer, fplArrayCount(buffer));
		ftAssertStringEquals("", buffer);
	}
	{
		char buffer[64] = {};
		fplStringAppend("Hello", buffer, fplArrayCount(buffer));
		ftAssertStringEquals("Hello", buffer);
	}
	{
		char buffer[64] = {};
		fplCopyString("Hello", buffer, fplArrayCount(buffer));
		fplStringAppend(" World", buffer, fplArrayCount(buffer));
		ftAssertStringEquals("Hello World", buffer);
	}

	ftMsg("Test format ansi string\n");
	{
		char* res = fplFormatString(nullptr, 0, nullptr);
		ftExpects(nullptr, res);
	}
	{
		char buffer[1];
		char* res = fplFormatString(buffer, 0, "");
		ftExpects(nullptr, res);
	}
	{
		char buffer[1];
		char* res = fplFormatString(buffer, fplArrayCount(buffer), "A");
		ftExpects(nullptr, res);
	}
	{
		char buffer[2];
		char* res = fplFormatString(buffer, fplArrayCount(buffer), "A");
		ftIsNotNull(res);
		bool matches = fplIsStringEqualLen("A", 1, buffer, 1);
		ftExpects(true, matches);
	}
	{
		char buffer[5];
		char* res = fplFormatString(buffer, fplArrayCount(buffer), "Hello");
		ftExpects(nullptr, res);
	}
	{
		char buffer[6];
		char* res = fplFormatString(buffer, fplArrayCount(buffer), "Hello");
		ftIsNotNull(res);
		bool r = fplIsStringEqualLen("Hello", 5, buffer, 5);
		ftExpects(true, r);
	}
	{
		char buffer[6];
		char* res = fplFormatString(buffer, fplArrayCount(buffer), "%s", "Hello");
		ftIsNotNull(res);
		bool r = fplIsStringEqualLen("Hello", 5, buffer, 5);
		ftExpects(true, r);
	}
	{
		char buffer[20];
		char* res = fplFormatString(buffer, fplArrayCount(buffer), "%4xd-%2d-%2d %2d:%2d:%2d", 2009, 11, 17, 13, 47, 25);
		ftExpects(nullptr, res);
	}
	{
		char buffer[20];
		char* res = fplFormatString(buffer, fplArrayCount(buffer), "%4d-%2d-%2d %2d:%2d:%2d", 2009, 11, 17, 13, 47, 25);
		ftIsNotNull(res);
		bool r = fplIsStringEqual("2009-11-17 13:47:25", buffer);
		ftExpects(true, r);
	}

	ftMsg("Test fplS32ToString\n");
	{
		char smallBuffer[2];
		char bigBuffer[16];
		ftIsNull(fplS32ToString(0, nullptr, 0));
		ftIsNull(fplS32ToString(0, nullptr, 4));
		ftIsNull(fplS32ToString(11, smallBuffer, fplArrayCount(smallBuffer)));
		ftIsNotNull(fplS32ToString(7, smallBuffer, fplArrayCount(smallBuffer)));
		ftAssertStringEquals("7", smallBuffer);
		ftIsNotNull(fplS32ToString(129, bigBuffer, fplArrayCount(bigBuffer)));
		ftAssertStringEquals("129", bigBuffer);
		ftIsNotNull(fplS32ToString(1337, bigBuffer, fplArrayCount(bigBuffer)));
		ftAssertStringEquals("1337", bigBuffer);
		ftIsNotNull(fplS32ToString(-1234567, bigBuffer, fplArrayCount(bigBuffer)));
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

static void TestLocalization() {
	fplPlatformInit(fplInitFlags_None, fpl_null);
	char buffer[16];
	ftIsTrue(fplGetSystemLocale(fplLocaleFormat_ISO639, buffer, fplArrayCount(buffer)));
	fplConsoleFormatOut("System Locale (ISO-639): %s\n", buffer);
	ftIsTrue(fplGetUserLocale(fplLocaleFormat_ISO639, buffer, fplArrayCount(buffer)));
	fplConsoleFormatOut("User Locale (ISO-639): %s\n", buffer);
	ftIsTrue(fplGetInputLocale(fplLocaleFormat_ISO639, buffer, fplArrayCount(buffer)));
	fplConsoleFormatOut("Input Locale (ISO-639): %s\n", buffer);
	fplPlatformRelease();
}

int main(int argc, char* args[]) {
	TestColdInit();
	TestInit();
	TestLocalization();
	TestMemory();
	TestOSInfos();
	TestHardware();
	TestSizes();
	TestMacros();
	TestAtomics();
	TestPaths();
	TestFiles();
	TestStrings();
	TestThreading();
	return 0;
}
