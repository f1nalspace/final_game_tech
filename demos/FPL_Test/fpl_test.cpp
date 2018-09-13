/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Test

Description:
	This demo is used to test all the things. It is basically a unit-test.

Requirements:
	- C++
	- Final Platform Layer

Author:
	Torsten Spaete

Changelog:
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
#include <final_platform_layer.h>

#define FT_IMPLEMENTATION
#include "final_test.h"

static void TestColdInit() {
	ft::Msg("Test Cold-Initialize of InitPlatform\n");
	{
		size_t errorCount = fplGetErrorCount();
		ft::AssertSizeEquals(0, errorCount);
		bool inited = fplPlatformInit(fplInitFlags_None, nullptr);
		FT_ASSERT(inited && (fplGetPlatformResult() == fplPlatformResultType_Success));
		const char *errorStr = fplGetLastError();
		ft::AssertStringEquals("", errorStr);
		fplPlatformRelease();
	}
}

static void TestInit() {
	ft::Msg("Test InitPlatform with All init flags\n");
	{
		fplClearErrors();
		bool inited = fplPlatformInit(fplInitFlags_All, nullptr);
		FT_ASSERT(inited && (fplGetPlatformResult() == fplPlatformResultType_Success));
		const char *errorStr = fplGetLastError();
		ft::AssertStringEquals("", errorStr);
		fplPlatformRelease();
	}
	ft::Msg("Test InitPlatform with None init flags\n");
	{
		fplClearErrors();
		bool inited = fplPlatformInit(fplInitFlags_None, fpl_null);
		FT_ASSERT(inited && (fplGetPlatformResult() == fplPlatformResultType_Success));
		const fplSettings *settings = fplGetCurrentSettings();
		FT_IS_NOT_NULL(settings);
		const char *errorStr = fplGetLastError();
		ft::AssertStringEquals("", errorStr);
		fplPlatformRelease();
	}
	ft::Msg("Test fplGetCurrentSettings in non-initialized state\n");
	{
		FT_IS_FALSE(fpl__global__InitState.isInitialized);
		fplClearErrors();
		const fplSettings *settings = fplGetCurrentSettings();
		FT_IS_NULL(settings);
		size_t errorCount = fplGetErrorCount();
		ft::AssertSizeEquals(1, errorCount);
		const char *errorStr = fplGetLastError();
		ft::AssertStringNotEquals("", errorStr);
	}
}

static void TestOSInfos() {
	ft::Msg("Get Platform Type\n");
	{
		fplPlatformType platType = fplGetPlatformType();
		FT_ASSERT(fplPlatformType_Unknown != platType);
	}
	ft::Msg("Get OS Type\n");
	{
		fplOSInfos osInfos = {};
		bool r = fplGetOperatingSystemInfos(&osInfos);
		FT_IS_TRUE(r);
		fplConsoleFormatOut("OS Name: %s\n", osInfos.osName);
		fplConsoleFormatOut("OS Version: %d.%d.%d.%d\n", osInfos.osVersion.major, osInfos.osVersion.minor, osInfos.osVersion.fix, osInfos.osVersion.build);
		fplConsoleFormatOut("Distribution Name: %s\n", osInfos.distributionName);
		fplConsoleFormatOut("Distribution Version: %d.%d.%d.%d\n", osInfos.distributionVersion.major, osInfos.distributionVersion.minor, osInfos.distributionVersion.fix, osInfos.distributionVersion.build);
	}
	ft::Msg("Get User Infos\n");
	{
		char nameBuffer[256] = {};
		bool r = fplGetCurrentUsername(nameBuffer, fplArrayCount(nameBuffer));
		FT_IS_TRUE(r);
		fplConsoleFormatOut("Current Username: %s\n", nameBuffer);
	}
}

static void TestSizes() {
	// @NOTE(final): This may be pretty useless, because stdint.h guarantees the size
	FT_EXPECTS(1, sizeof(uint8_t));
	FT_EXPECTS(1, sizeof(int8_t));
	FT_EXPECTS(2, sizeof(uint16_t));
	FT_EXPECTS(2, sizeof(int16_t));
	FT_EXPECTS(4, sizeof(uint32_t));
	FT_EXPECTS(4, sizeof(int32_t));
	FT_EXPECTS(8, sizeof(uint64_t));
	FT_EXPECTS(8, sizeof(int64_t));
#if defined(FT_ARCH_X64)
	FT_EXPECTS(8, sizeof(intptr_t));
	FT_EXPECTS(8, sizeof(uintptr_t));
	FT_EXPECTS(8, sizeof(size_t));
#else
	FT_EXPECTS(4, sizeof(intptr_t));
	FT_EXPECTS(4, sizeof(uintptr_t));
	FT_EXPECTS(4, sizeof(size_t));
#endif
}

static void TestMacros() {
	//
	// fplArrayCount
	//
	ft::Msg("[fplArrayCount] Test static char array\n");
	{
		char staticArray[137] = {};
		uint32_t actual = fplArrayCount(staticArray);
		FT_EXPECTS(137, actual);
	}
	ft::Msg("[fplArrayCount] Test static int array\n");
	{
		int staticArray[349] = {};
		uint32_t actual = fplArrayCount(staticArray);
		FT_EXPECTS(349, actual);
	}
	ft::Msg("[fplArrayCount] Test static bool array\n");
	{
		bool staticArray[961] = {};
		uint32_t actual = fplArrayCount(staticArray);
		FT_EXPECTS(961, actual);
	}
	ft::Msg("[fplArrayCount] Test static void pointer array\n");
	{
		void *staticArray[35] = {};
		uint32_t actual = fplArrayCount(staticArray);
		FT_EXPECTS(35, actual);
	}

	// @NOTE(final): This is a simple/stupid macro, so when you pass a pointer, you basically get 2 always
	ft::Msg("[fplArrayCount] Test nullptr\n");
	{
		int *emptyArray = nullptr;
		uint32_t actual = fplArrayCount(emptyArray);
		uint32_t expected = sizeof(int *) / sizeof(int);
		FT_EXPECTS(expected, actual);
	}
	ft::Msg("[fplArrayCount] Test pointer from references static array\n");
	{
		int staticArray[3] = {};
		int *refArray = &staticArray[0];
		uint32_t actual = fplArrayCount(refArray);
		uint32_t expected = sizeof(int *) / sizeof(int);
		FT_EXPECTS(expected, actual);
	}

	//
	// fplOffsetOf
	//
	ft::Msg("[fplOffsetOf] Test alignment of 4 (High to low)\n");
	{
#	pragma pack(push, 4)
		struct TestStruct {
			uint64_t a;
			uint32_t b;
			uint16_t c;
			uint8_t d;
		};
#	pragma pack(pop)
		FT_EXPECTS(0, fplOffsetOf(TestStruct, a));
		FT_EXPECTS(8, fplOffsetOf(TestStruct, b));
		FT_EXPECTS(12, fplOffsetOf(TestStruct, c));
		FT_EXPECTS(14, fplOffsetOf(TestStruct, d));
	}

	ft::Msg("[fplOffsetOf] Test alignment of 4 (Low to High)\n");
	{
#	pragma pack(push, 4)
		struct TestStruct {
			uint8_t a;
			uint16_t b;
			uint32_t c;
			uint64_t d;
		};
#	pragma pack(pop)
		FT_EXPECTS(0, fplOffsetOf(TestStruct, a));
		FT_EXPECTS(2, fplOffsetOf(TestStruct, b));
		FT_EXPECTS(4, fplOffsetOf(TestStruct, c));
		FT_EXPECTS(8, fplOffsetOf(TestStruct, d));
	}

	ft::Msg("[fplOffsetOf] Test alignment of 8 (Low to High)\n");
	{
#	pragma pack(push, 8)
		struct TestStruct {
			uint8_t a;
			uint16_t b;
			uint8_t c[3];
			uint64_t d;
		};
#	pragma pack(pop)
		FT_EXPECTS(0, fplOffsetOf(TestStruct, a));
		FT_EXPECTS(2, fplOffsetOf(TestStruct, b));
		FT_EXPECTS(4, fplOffsetOf(TestStruct, c));
		FT_EXPECTS(8, fplOffsetOf(TestStruct, d));
	}

	//
	// fplMin/fplMax
	//
	ft::Msg("[fplMin] Test integers\n");
	{
		ft::AssertS32Equals(3, fplMin(3, 7));
		ft::AssertS32Equals(3, fplMin(7, 3));
		ft::AssertS32Equals(-7, fplMin(-7, -3));
		ft::AssertS32Equals(-7, fplMin(-3, -7));
		struct TestStruct {
			int a;
			int b;
		};
		TestStruct instance = { 3, 7 };
		TestStruct *instancePtr = &instance;
		ft::AssertS32Equals(3, fplMin(instancePtr->a, instancePtr->b));
	}
	ft::Msg("[fplMin] Test floats\n");
	{
		ft::AssertFloatEquals(3.0f, fplMin(3.0f, 7.0f));
		ft::AssertFloatEquals(3.0f, fplMin(7.0f, 3.0f));
		ft::AssertFloatEquals(-7.0f, fplMin(-7.0f, -3.0f));
		ft::AssertFloatEquals(-7.0f, fplMin(-3.0f, -7.0f));
		struct TestStruct {
			float a;
			float b;
		};
		TestStruct instance = { 3.0f, 7.0f };
		TestStruct *instancePtr = &instance;
		ft::AssertFloatEquals(3.0f, fplMin(instancePtr->a, instancePtr->b));
	}
	ft::Msg("[fplMax] Test integers\n");
	{
		ft::AssertS32Equals(7, fplMax(3, 7));
		ft::AssertS32Equals(7, fplMax(7, 3));
		ft::AssertS32Equals(-3, fplMax(-3, -7));
		ft::AssertS32Equals(-3, fplMax(-7, -3));
		struct TestStruct {
			int a;
			int b;
		};
		TestStruct instance = { 3, 7 };
		TestStruct *instancePtr = &instance;
		ft::AssertS32Equals(7, fplMax(instancePtr->a, instancePtr->b));
	}
	ft::Msg("[fplMax] Test floats\n");
	{
		ft::AssertFloatEquals(7.0f, fplMax(3.0f, 7.0f));
		ft::AssertFloatEquals(7.0f, fplMax(7.0f, 3.0f));
		ft::AssertFloatEquals(-3.0f, fplMax(-3.0f, -7.0f));
		ft::AssertFloatEquals(-3.0f, fplMax(-7.0f, -3.0f));
		struct TestStruct {
			float a;
			float b;
		};
		TestStruct instance = { 3.0f, 7.0f };
		TestStruct *instancePtr = &instance;
		ft::AssertFloatEquals(7.0f, fplMax(instancePtr->a, instancePtr->b));
	}

	//
	// FPL_KILOBYTES, FPL_MEGABYTES, ...
	//
	{
		ft::Msg("[FPL_KILOBYTES] Test 0 KB \n");
		ft::AssertSizeEquals(0, FPL_KILOBYTES(0));
		ft::Msg("[FPL_KILOBYTES] Test 8 KB \n");
		ft::AssertSizeEquals(8192, FPL_KILOBYTES(8));
		ft::Msg("[FPL_MEGABYTES] Test 0 MB \n");
		ft::AssertSizeEquals(0, FPL_MEGABYTES(0));
		ft::Msg("[FPL_MEGABYTES] Test 8 MB \n");
		ft::AssertSizeEquals(8388608, FPL_MEGABYTES(8));
		ft::Msg("[FPL_GIGABYTES] Test 0 GB \n");
		ft::AssertSizeEquals(0, FPL_GIGABYTES(0));
		ft::Msg("[FPL_GIGABYTES] Test 1 GB \n");
		ft::AssertSizeEquals(1073741824, FPL_GIGABYTES(1));
#if defined(FT_ARCH_X64)
		ft::Msg("[FPL_GIGABYTES] Test 4 GB \n");
		ft::AssertSizeEquals(4294967296, FPL_GIGABYTES(4));
		ft::Msg("[FPL_TERABYTES] Test 0 TB \n");
		ft::AssertSizeEquals(0, FPL_TERABYTES(0));
		ft::Msg("[FPL_TERABYTES] Test 2 TB \n");
		ft::AssertSizeEquals(2199023255552, FPL_TERABYTES(2));
#endif
	}
}

static void TestMemory() {
	ft::Msg("Test normal allocation and deallocation\n");
	{
		size_t memSize = FPL_KILOBYTES(42);
		uint8_t *mem = (uint8_t *)fplMemoryAllocate(memSize);
		for(size_t i = 0; i < memSize; ++i) {
			uint8_t value = *mem++;
			ft::AssertU8Equals(0, value);
		}
		fplMemoryFree(mem);
	}
	{
		size_t memSize = FPL_MEGABYTES(512);
		void *mem = fplMemoryAllocate(memSize);
		FT_IS_NOT_NULL(mem);
		fplMemoryFree(mem);
	}

	ft::Msg("Test aligned allocation and deallocation\n");
	{
		size_t memSize = FPL_KILOBYTES(42);
		uint8_t *mem = (uint8_t *)fplMemoryAlignedAllocate(memSize, 16);
		for(size_t i = 0; i < memSize; ++i) {
			uint8_t value = *(mem + i);
			ft::AssertU8Equals(0, value);
		}
		fplMemoryAlignedFree(mem);
	}
	{
		size_t memSize = FPL_MEGABYTES(512);
		void *mem = fplMemoryAlignedAllocate(memSize, 16);
		FT_IS_NOT_NULL(mem);
		fplMemoryAlignedFree(mem);
	}

	ft::Msg("Test memory clear\n");
	{
		size_t memSize = 100;
		uint8_t *mem = (uint8_t *)fplMemoryAllocate(memSize);
		for(size_t i = 0; i < memSize; ++i) {
			mem[i] = (uint8_t)i; // Dont care about wrap
		}
		fplMemorySet(mem, 0, memSize);
		for(size_t i = 0; i < memSize; ++i) {
			uint8_t value = *mem++;
			ft::AssertU8Equals(0, value);
		}
		fplMemoryFree(mem);
	}

	ft::Msg("Test memory set\n");
	{
		size_t memSize = 100;
		uint8_t *mem = (uint8_t *)fplMemoryAllocate(memSize);
		for(size_t i = 0; i < memSize; ++i) {
			mem[i] = (uint8_t)i; // Dont care about wrap
		}
		fplMemorySet(mem, 128, memSize);
		for(size_t i = 0; i < memSize; ++i) {
			uint8_t value = *mem++;
			ft::AssertU8Equals(128, value);
		}
		fplMemoryFree(mem);
	}
}

static void TestPaths() {
	if(fplPlatformInit(fplInitFlags_None, fpl_null)) {

		char homePathBuffer[1024] = {};
		fplGetHomePath(homePathBuffer, fplArrayCount(homePathBuffer));
		ft::Msg("Home Path:\n%s\n", homePathBuffer);

		char exeFilePathBuffer[1024] = {};
		fplGetExecutableFilePath(exeFilePathBuffer, fplArrayCount(exeFilePathBuffer));
		ft::Msg("Executable file Path:\n%s\n", exeFilePathBuffer);

		char extractedPathBuffer[1024] = {};
		fplExtractFilePath(exeFilePathBuffer, extractedPathBuffer, fplArrayCount(extractedPathBuffer));
		ft::Msg("Extracted path:\n%s\n", extractedPathBuffer);

		const char *exeFileName = fplExtractFileName(exeFilePathBuffer);
		ft::Msg("Extracted filename:\n%s\n", exeFileName);

		const char *exeFileExt = fplExtractFileExtension(exeFilePathBuffer);
		ft::Msg("Extracted extension:\n%s\n", exeFileExt);

		char combinedPathBuffer[1024 * 10] = {};
		fplPathCombine(combinedPathBuffer, fplArrayCount(combinedPathBuffer), 4, "Hallo", "Welt", "der", "Programmierer");
		ft::Msg("Combined path:\n%s\n", combinedPathBuffer);

		char changedFileExtBuffer[1024] = {};
		fplChangeFileExtension(exeFilePathBuffer, ".obj", changedFileExtBuffer, fplArrayCount(changedFileExtBuffer));
		ft::Msg("Changed file ext 1:\n%s\n", changedFileExtBuffer);
		fplChangeFileExtension(exeFileName, ".obj", changedFileExtBuffer, fplArrayCount(changedFileExtBuffer));
		ft::Msg("Changed file ext 2:\n%s\n", changedFileExtBuffer);
		fplChangeFileExtension(".dll", ".obj", changedFileExtBuffer, fplArrayCount(changedFileExtBuffer));
		ft::Msg("Changed file ext 3:\n%s\n", changedFileExtBuffer);
		fplChangeFileExtension("", ".obj", changedFileExtBuffer, fplArrayCount(changedFileExtBuffer));
		ft::Msg("Changed file ext 4:\n%s\n", changedFileExtBuffer);
		fplChangeFileExtension(".dll", "", changedFileExtBuffer, fplArrayCount(changedFileExtBuffer));
		ft::Msg("Changed file ext 5:\n%s\n", changedFileExtBuffer);
		fplChangeFileExtension("", "", changedFileExtBuffer, fplArrayCount(changedFileExtBuffer));
		ft::Msg("Changed file ext 5:\n%s\n", changedFileExtBuffer);

		fplPlatformRelease();
	}
}


static void TestHardware() {
	char cpuNameBuffer[1024] = {};
	fplGetProcessorName(cpuNameBuffer, fplArrayCount(cpuNameBuffer));
	ft::Msg("Processor name: %s\n", cpuNameBuffer);

	size_t coreCount = fplGetProcessorCoreCount();
	FT_ASSERT(coreCount > 0);
	ft::Msg("Processor cores: %zu\n", coreCount);

	fplMemoryInfos memInfos = {};
	fplGetRunningMemoryInfos(&memInfos);
	ft::Msg("Installed physical memory (bytes): %z\n", memInfos.totalPhysicalSize);
	ft::Msg("Total physical memory (bytes): %z\n", memInfos.totalPhysicalSize);
	ft::Msg("Available physical memory (bytes): %z\n", memInfos.freePhysicalSize);
	ft::Msg("Total cache memory (bytes): %z\n", memInfos.totalCacheSize);
	ft::Msg("Available cache memory (bytes): %z\n", memInfos.freeCacheSize);
	ft::Msg("Page size (bytes): %z\n", memInfos.pageSize);
	ft::Msg("Total number of memory pages: %z\n", memInfos.totalPageCount);
	ft::Msg("Available number memory pages: %z\n", memInfos.freePageCount);

	fplArchType archType = fplGetRunningArchitecture();
	const char *archStr = fplGetArchTypeString(archType);
	ft::Msg("System archicture: %s\n", archStr);
}

static void EmptyThreadproc(const fplThreadHandle *context, void *data) {
}

struct ThreadData {
	int num;
	int sleepFor;
};

static void SingleThreadProc(const fplThreadHandle *context, void *data) {
	ThreadData *d = (ThreadData *)data;
	ft::Msg("Sleep in thread %d for %d ms\n", d->num, d->sleepFor);
	fplThreadSleep(d->sleepFor);
}

static void SimpleMultiThreadTest(const size_t threadCount) {
	ft::Line();
	ThreadData threadData[FPL__MAX_THREAD_COUNT] = {};
	for(size_t threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
		threadData[threadIndex].num = (int)(threadIndex + 1);
		threadData[threadIndex].sleepFor = (int)(1 + threadIndex) * 500;
	}
	fplThreadHandle *threads[FPL__MAX_THREAD_COUNT];
	ft::Msg("Start %d threads\n", threadCount);
	for(size_t threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
		threads[threadIndex] = fplThreadCreate(SingleThreadProc, &threadData[threadIndex]);
	}
	ft::Msg("Wait all %d threads for exit\n", threadCount);
	fplThreadWaitForAll(threads, threadCount, UINT32_MAX);
	ft::Msg("All %d threads are done\n", threadCount);

	ft::Msg("Terminate %d threads\n", threadCount);
	for(size_t threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
		FT_EXPECTS(fplThreadState_Stopped, threads[threadIndex]->currentState);
		fplThreadTerminate(threads[threadIndex]);
	}
}

struct MutableThreadData {
	fplSemaphoreHandle semaphore;
	volatile int32_t value;
};

struct WriteThreadData {
	ThreadData base;
	MutableThreadData *data;
	int32_t valueToWrite;
};

struct ReadThreadData {
	ThreadData base;
	MutableThreadData *data;
	int32_t expectedValue;
};

static void WriteDataThreadProc(const fplThreadHandle *context, void *data) {
	WriteThreadData *d = (WriteThreadData *)data;
	ft::Msg("Sleep in thread %d for %d ms\n", d->base.num, d->base.sleepFor);
	fplThreadSleep(d->base.sleepFor);
	fplAtomicStoreS32(&d->data->value, d->valueToWrite);
}

static void ReadDataThreadProc(const fplThreadHandle *context, void *data) {
	ReadThreadData *d = (ReadThreadData *)data;
	ft::Msg("Sleep in thread %d for %d ms\n", d->base.num, d->base.sleepFor);
	fplThreadSleep(d->base.sleepFor);
	int32_t actualValue = fplAtomicLoadS32(&d->data->value);
	FT_EXPECTS(d->expectedValue, actualValue);
}

static void SyncThreadsTestAtomics() {
	ft::Line();
	ft::Msg("Sync test for 1 reader and 1 writer using atomics\n");
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

		fplThreadHandle *threads[2];
		uint32_t threadCount = fplArrayCount(threads);

		ft::Msg("Start %zu threads\n", threadCount);
		threads[0] = fplThreadCreate(ReadDataThreadProc, &readData);
		threads[1] = fplThreadCreate(WriteDataThreadProc, &writeData);

		ft::Msg("Wait for %zu threads to exit\n", threadCount);
		fplThreadWaitForAll(threads, threadCount, UINT32_MAX);

		ft::Msg("Release resources for %zu threads\n", threadCount);
		for(uint32_t index = 0; index < threadCount; ++index) {
			FT_EXPECTS(fplThreadState_Stopped, threads[index]->currentState);
			fplThreadTerminate(threads[index]);
		}
	}
}

static void WriteDataSemaphoreThreadProc(const fplThreadHandle *context, void *data) {
	WriteThreadData *d = (WriteThreadData *)data;
	ft::Msg("Sleep in thread %d for %d ms\n", d->base.num, d->base.sleepFor);
	fplThreadSleep(d->base.sleepFor);
	ft::Msg("Wait for semaphore in thread %d\n", d->base.num);
	fplSemaphoreWait(&d->data->semaphore, FPL_TIMEOUT_INFINITE);
	int32_t v = d->data->value;
	if(d->base.num % 2 == 0) {
		v--;
	} else {
		v++;
	}
	d->data->value = v;
	fplSemaphoreRelease(&d->data->semaphore);
}

static void SyncThreadsTestSemaphores(const size_t numWriters) {
	FT_IS_TRUE(numWriters >= 2);

	ft::Line();
	ft::Msg("Sync test for %zu writers using semaphores\n", numWriters);
	{
		MutableThreadData mutableData = {};
		uint32_t initialValue = (uint32_t)numWriters - 1;
		FT_IS_TRUE(fplSemaphoreInit(&mutableData.semaphore, initialValue));
		mutableData.value = 0;

		WriteThreadData writeDatas[FPL__MAX_THREAD_COUNT] = {};
		fplThreadHandle *threads[FPL__MAX_THREAD_COUNT] = {};
		ft::Msg("Start %zu threads\n", numWriters);
		for(uint32_t i = 0; i < numWriters; ++i) {
			writeDatas[i].base.num = i + 1;
			writeDatas[i].base.sleepFor = 3000;
			writeDatas[i].data = &mutableData;
			threads[i] = fplThreadCreate(WriteDataSemaphoreThreadProc, &writeDatas[i]);
		}

		ft::Msg("Wait for %zu threads to exit\n", numWriters);
		fplThreadWaitForAll(threads, numWriters, UINT32_MAX);
		int32_t expectedValue = (numWriters % 2 == 0) ? 0 : 1;
		ft::AssertS32Equals(expectedValue, mutableData.value);

		ft::Msg("Release resources for %zu threads\n", numWriters);
		for(uint32_t index = 0; index < numWriters; ++index) {
			FT_EXPECTS(fplThreadState_Stopped, threads[index]->currentState);
			fplThreadTerminate(threads[index]);
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
	SlaveThreadData *slaveThreads;
	uint32_t slaveCount;
	ConditionTestType testType;
};

static void ThreadSlaveProc(const fplThreadHandle *context, void *data) {
	SlaveThreadData *d = (SlaveThreadData *)data;

	if(d->testType == ConditionTestType::Signal) {
		ft::Msg("Slave-Thread %d waits for signal\n", d->base.num);
		fplSignalWaitForOne(&d->signal, FPL_TIMEOUT_INFINITE);
		d->isSuccess = true;
		ft::Msg("Got signal on Slave-Thread %d\n", d->base.num);
	} else if(d->testType == ConditionTestType::ConditionSignal) {
		ft::Msg("Slave-Thread %d waits on condition\n", d->base.num);
		fplConditionWait(&d->condition, &d->mutex, FPL_TIMEOUT_INFINITE);
		d->isSuccess = true;
		ft::Msg("Got condition on Slave-Thread %d\n", d->base.num);
	}

	ft::Msg("Slave-Thread %d is done\n", d->base.num);
}

static void ThreadMasterProc(const fplThreadHandle *context, void *data) {
	MasterThreadData *d = (MasterThreadData *)data;
	ft::Msg("Master-Thread %d waits for 5 seconds\n", d->base.num);
	fplThreadSleep(5000);

	for(uint32_t signalIndex = 0; signalIndex < d->slaveCount; ++signalIndex) {
		if(d->testType == ConditionTestType::Signal) {
			ft::Msg("Master-Thread %d sets signal %d\n", d->base.num, signalIndex);
			fplSignalSet(&d->slaveThreads[signalIndex].signal);
		} else if(d->testType == ConditionTestType::ConditionSignal) {
			ft::Msg("Master-Thread %d sends signal to condition %d\n", d->base.num, signalIndex);
			fplConditionSignal(&d->slaveThreads[signalIndex].condition);
		}
	}

	ft::Msg("Master-Thread %d is done\n", d->base.num);
}

static void ConditionThreadsTest(const size_t threadCount, const ConditionTestType testType) {
	FT_ASSERT(threadCount > 1);

	ft::Line();

	if(testType == ConditionTestType::Signal) {
		ft::Msg("Signals test for %zu threads\n", threadCount);
	} else if(testType == ConditionTestType::ConditionSignal) {
		ft::Msg("Condition-Variable (Single) test for %zu threads\n", threadCount);
	}

	MasterThreadData masterData = {};
	masterData.base.num = 1;
	masterData.testType = testType;

	SlaveThreadData slaveDatas[FPL__MAX_THREAD_COUNT] = {};
	size_t slaveThreadCount = threadCount - 1;
	for(size_t threadIndex = 0; threadIndex < slaveThreadCount; ++threadIndex) {
		slaveDatas[threadIndex].base.num = masterData.base.num + (int)threadIndex + 1;
		slaveDatas[threadIndex].testType = testType;
		if(testType == ConditionTestType::Signal) {
			FT_IS_TRUE(fplSignalInit(&slaveDatas[threadIndex].signal, fplSignalValue_Unset));
		} else if(testType == ConditionTestType::ConditionSignal) {
			FT_IS_TRUE(fplMutexInit(&slaveDatas[threadIndex].mutex));
			FT_IS_TRUE(fplConditionInit(&slaveDatas[threadIndex].condition));
		}
		size_t i = masterData.slaveCount++;
	}
	masterData.slaveThreads = slaveDatas;

	ft::Msg("Start %zu slave threads, 1 master thread\n", slaveThreadCount);
	fplThreadHandle *threads[FPL__MAX_THREAD_COUNT];
	for(size_t threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
		if(threadIndex == 0) {
			threads[threadIndex] = fplThreadCreate(ThreadMasterProc, &masterData);
		} else {
			threads[threadIndex] = fplThreadCreate(ThreadSlaveProc, &slaveDatas[threadIndex - 1]);
		}
	}

	ft::Msg("Wait for %zu threads to exit\n", threadCount);
	fplThreadWaitForAll(threads, threadCount, UINT32_MAX);

	ft::Msg("Release resources for %zu threads\n", threadCount);
	for(size_t slaveIndex = 0; slaveIndex < slaveThreadCount; ++slaveIndex) {
		FT_IS_TRUE(slaveDatas[slaveIndex].isSuccess);
	}
	for(size_t threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
		fplThreadHandle *thread = threads[threadIndex];
		FT_EXPECTS(fplThreadState_Stopped, thread->currentState);
	}
	for(size_t slaveIndex = 0; slaveIndex < slaveThreadCount; ++slaveIndex) {
		if(testType == ConditionTestType::Signal) {
			fplSignalDestroy(&slaveDatas[slaveIndex].signal);
		} else if(testType == ConditionTestType::ConditionSignal) {
			fplConditionDestroy(&slaveDatas[slaveIndex].condition);
			fplMutexDestroy(&slaveDatas[slaveIndex].mutex);
		}
	}
}

static void TestThreading() {
	if(fplPlatformInit(fplInitFlags_None, fpl_null)) {
		//
		// Single threading test
		//
		ft::Line();
		ft::Msg("Test 1 empty thread\n");
		{
			fplThreadHandle *thread;
			ft::Msg("Start thread\n");
			thread = fplThreadCreate(EmptyThreadproc, nullptr);
			ft::Msg("Wait thread for exit\n");
			fplThreadWaitForOne(thread, UINT32_MAX);
			ft::Msg("Thread is done\n");
			FT_EXPECTS(fplThreadState_Stopped, thread->currentState);
			fplThreadTerminate(thread);
		}

		ft::Line();
		ft::Msg("Test 1 sleeping-thread\n");
		{
			ThreadData threadData = {};
			threadData.num = 1;
			threadData.sleepFor = 3000;
			ft::Msg("Start thread %d\n", threadData.num);
			fplThreadHandle *thread = fplThreadCreate(SingleThreadProc, &threadData);
			ft::Msg("Wait thread %d for exit\n", threadData.num);
			fplThreadWaitForOne(thread, UINT32_MAX);
			ft::Msg("Thread %d is done\n", threadData.num);
			FT_EXPECTS(fplThreadState_Stopped, thread->currentState);
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
#if defined(FPL_PLATFORM_WIN32)
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

	ft::Msg("Test File Exists\n");
	{
		bool nonExisting = fplFileExists(testNotExistingFile);
		FT_IS_FALSE(nonExisting);
		bool existing = fplFileExists(testExistingFile);
		FT_IS_TRUE(existing);
	}
	ft::Msg("Test File Size\n");
	{
		uint32_t emptySize = fplGetFileSizeFromPath32(testNotExistingFile);
		ft::AssertU32Equals(0, emptySize);
		uint32_t existingSize = fplGetFileSizeFromPath32(testExistingFile);
		FT_ASSERT(existingSize > 0);
	}
	ft::Msg("Test Directory Iterations without filter\n");
	{
		fplFileEntry fileEntry = {};
		for(bool r = fplListDirBegin(testRootPath, "*.*", &fileEntry); r; r = fplListDirNext(&fileEntry)) {
			ft::Msg("%s\n", fileEntry.fullPath);
		}
		fplListDirEnd(&fileEntry);
	}
	ft::Msg("Test Directory Iterations with all filter\n");
	{
		fplFileEntry fileEntry = {};
		for(bool r = fplListDirBegin(testRootPath, "*", &fileEntry); r; r = fplListDirNext(&fileEntry)) {
			ft::Msg("%s\n", fileEntry.fullPath);
		}
		fplListDirEnd(&fileEntry);
	}
	ft::Msg("Test Directory Iterations with root filter '%s'\n", testRootFilter);
	{
		fplFileEntry fileEntry = {};
		bool r = fplListDirBegin(testRootPath, testRootFilter, &fileEntry);
		ft::Msg("%s\n", fileEntry.fullPath);
		FT_IS_TRUE(r);
		fplListDirEnd(&fileEntry);
	}
}

static void TestAtomics() {
	ft::Msg("Test AtomicExchangeU32 with different values\n");
	{
		const uint32_t expectedBefore = 42;
		const uint32_t expectedAfter = 1337;
		volatile uint32_t t = expectedBefore;
		uint32_t r = fplAtomicExchangeU32(&t, expectedAfter);
		ft::AssertU32Equals(expectedBefore, r);
		ft::AssertU32Equals(expectedAfter, (uint32_t)t);
	}
	ft::Msg("Test AtomicExchangeU32 with negative value\n");
	{
		const uint32_t expectedBefore = 42;
		const uint32_t exchangeValue = -1;
		const uint32_t expectedAfter = (uint32_t)UINT32_MAX;
		volatile uint32_t t = expectedBefore;
		uint32_t r = fplAtomicExchangeU32(&t, exchangeValue);
		ft::AssertU32Equals(expectedBefore, r);
		ft::AssertU32Equals(expectedAfter, (uint32_t)t);
	}
	ft::Msg("Test AtomicExchangeU32 with same value\n");
	{
		const uint32_t expectedBefore = 1;
		const uint32_t exchangeValue = expectedBefore;
		const uint32_t expectedAfter = exchangeValue;
		volatile uint32_t t = expectedBefore;
		uint32_t r = fplAtomicExchangeU32(&t, exchangeValue);
		ft::AssertU32Equals(expectedBefore, r);
		ft::AssertU32Equals(expectedAfter, (uint32_t)t);
	}
	ft::Msg("Test AtomicExchangeU32 with UINT32_MAX\n");
	{
		const uint32_t expectedBefore = 1;
		const uint32_t exchangeValue = UINT32_MAX;
		const uint32_t expectedAfter = exchangeValue;
		volatile uint32_t t = expectedBefore;
		uint32_t r = fplAtomicExchangeU32(&t, exchangeValue);
		ft::AssertU32Equals(expectedBefore, r);
		ft::AssertU32Equals(expectedAfter, (uint32_t)t);
	}
	ft::Msg("Test AtomicExchangeU32 with INT32_MAX + 1\n");
	{
		const uint32_t expectedBefore = 1;
		const uint32_t exchangeValue = (uint32_t)INT32_MAX + 1;
		const uint32_t expectedAfter = exchangeValue;
		volatile uint32_t t = expectedBefore;
		uint32_t r = fplAtomicExchangeU32(&t, exchangeValue);
		ft::AssertU32Equals(expectedBefore, r);
		ft::AssertU32Equals(expectedAfter, (uint32_t)t);
	}
	ft::Msg("Test AtomicExchangeS32 with different values\n");
	{
		const int32_t expectedBefore = 42;
		const int32_t exchangeValue = 1337;
		const int32_t expectedAfter = exchangeValue;
		volatile int32_t t = expectedBefore;
		int32_t r = fplAtomicExchangeS32(&t, exchangeValue);
		ft::AssertS32Equals(expectedBefore, r);
		ft::AssertS32Equals(expectedAfter, (int32_t)t);
	}
	ft::Msg("Test AtomicExchangeS32 with negative value\n");
	{
		const int32_t expectedBefore = 42;
		const int32_t exchangeValue = -1;
		const int32_t expectedAfter = exchangeValue;
		volatile int32_t t = expectedBefore;
		int32_t r = fplAtomicExchangeS32(&t, exchangeValue);
		ft::AssertS32Equals(expectedBefore, r);
		ft::AssertS32Equals(expectedAfter, (int32_t)t);
	}
	ft::Msg("Test AtomicExchangeS32 with same value\n");
	{
		const int32_t expectedBefore = 1;
		const int32_t exchangeValue = expectedBefore;
		const int32_t expectedAfter = exchangeValue;
		volatile int32_t t = expectedBefore;
		int32_t r = fplAtomicExchangeS32(&t, exchangeValue);
		ft::AssertS32Equals(expectedBefore, r);
		ft::AssertS32Equals(expectedAfter, (int32_t)t);
	}

	ft::Msg("Test AtomicExchangeU64 with different values\n");
	{
		const uint64_t expectedBefore = 42;
		const uint64_t expectedAfter = 1337;
		volatile uint64_t t = expectedBefore;
		uint64_t r = fplAtomicExchangeU64(&t, expectedAfter);
		ft::AssertU64Equals(expectedBefore, r);
		ft::AssertU64Equals(expectedAfter, (uint64_t)t);
	}
	ft::Msg("Test AtomicExchangeU64 with negative value\n");
	{
		const uint64_t expectedBefore = 42;
		const uint64_t exchangeValue = -1;
		const uint64_t expectedAfter = (uint64_t)UINT64_MAX;
		volatile uint64_t t = expectedBefore;
		uint64_t r = fplAtomicExchangeU64(&t, exchangeValue);
		ft::AssertU64Equals(expectedBefore, r);
		ft::AssertU64Equals(expectedAfter, (uint64_t)t);
	}
	ft::Msg("Test AtomicExchangeU64 with same value\n");
	{
		const uint64_t expectedBefore = 1;
		const uint64_t exchangeValue = expectedBefore;
		const uint64_t expectedAfter = exchangeValue;
		volatile uint64_t t = expectedBefore;
		uint64_t r = fplAtomicExchangeU64(&t, exchangeValue);
		ft::AssertU64Equals(expectedBefore, r);
		ft::AssertU64Equals(expectedAfter, (uint64_t)t);
	}
	ft::Msg("Test AtomicExchangeU64 with UINT64_MAX\n");
	{
		const uint64_t expectedBefore = 1;
		const uint64_t exchangeValue = UINT64_MAX;
		const uint64_t expectedAfter = exchangeValue;
		volatile uint64_t t = expectedBefore;
		uint64_t r = fplAtomicExchangeU64(&t, exchangeValue);
		ft::AssertU64Equals(expectedBefore, r);
		ft::AssertU64Equals(expectedAfter, (uint64_t)t);
	}
	ft::Msg("Test AtomicExchangeU64 with INT64_MAX + 1\n");
	{
		const uint64_t expectedBefore = 1;
		const uint64_t exchangeValue = (uint64_t)INT64_MAX + 1;
		const uint64_t expectedAfter = exchangeValue;
		volatile uint64_t t = expectedBefore;
		uint64_t r = fplAtomicExchangeU64(&t, exchangeValue);
		ft::AssertU64Equals(expectedBefore, r);
		ft::AssertU64Equals(expectedAfter, (uint64_t)t);
	}
	ft::Msg("Test AtomicExchangeS64 with different values\n");
	{
		const int64_t expectedBefore = 42;
		const int64_t exchangeValue = 1337;
		const int64_t expectedAfter = exchangeValue;
		volatile int64_t t = expectedBefore;
		int64_t r = fplAtomicExchangeS64(&t, exchangeValue);
		ft::AssertS64Equals(expectedBefore, r);
		ft::AssertS64Equals(expectedAfter, (int64_t)t);
	}
	ft::Msg("Test AtomicExchangeS64 with negative value\n");
	{
		const int64_t expectedBefore = 42;
		const int64_t exchangeValue = -1;
		const int64_t expectedAfter = exchangeValue;
		volatile int64_t t = expectedBefore;
		int64_t r = fplAtomicExchangeS64(&t, exchangeValue);
		ft::AssertS64Equals(expectedBefore, r);
		ft::AssertS64Equals(expectedAfter, (int64_t)t);
	}
	ft::Msg("Test AtomicExchangeS64 with same value\n");
	{
		const int64_t expectedBefore = 1;
		const int64_t exchangeValue = expectedBefore;
		const int64_t expectedAfter = exchangeValue;
		volatile int64_t t = expectedBefore;
		int64_t r = fplAtomicExchangeS64(&t, exchangeValue);
		ft::AssertS64Equals(expectedBefore, r);
		ft::AssertS64Equals(expectedAfter, (int64_t)t);
	}

	ft::Msg("Test AtomicCompareAndExchangeU32 with exchange\n");
	{
		volatile uint32_t value = 3;
		uint32_t comparand = 3;
		uint32_t exchange = 11;
		uint32_t after = fplAtomicCompareAndExchangeU32(&value, comparand, exchange);
		ft::AssertU32Equals((uint32_t)11, (uint32_t)value);
		ft::AssertU32Equals((uint32_t)3, after);
	}
	ft::Msg("Test AtomicCompareAndExchangeU32 no exchange\n");
	{
		volatile uint32_t value = 5;
		uint32_t comparand = 3;
		uint32_t exchange = 11;
		uint32_t after = fplAtomicCompareAndExchangeU32(&value, comparand, exchange);
		ft::AssertU32Equals((uint32_t)5, (uint32_t)value);
		ft::AssertU32Equals((uint32_t)5, after);
	}
	ft::Msg("Test AtomicCompareAndExchangeS32 with exchange\n");
	{
		volatile int32_t value = -3;
		int32_t comparand = -3;
		int32_t exchange = 11;
		int32_t after = fplAtomicCompareAndExchangeS32(&value, comparand, exchange);
		ft::AssertS32Equals((int32_t)11, (int32_t)value);
		ft::AssertS32Equals((int32_t)-3, after);
	}
	ft::Msg("Test AtomicCompareAndExchangeS32 no exchange\n");
	{
		volatile int32_t value = -5;
		int32_t comparand = -3;
		int32_t exchange = 11;
		int32_t after = fplAtomicCompareAndExchangeS32(&value, comparand, exchange);
		ft::AssertS32Equals((int32_t)-5, (int32_t)value);
		ft::AssertS32Equals((int32_t)-5, after);
	}
	ft::Msg("Test AtomicCompareAndExchangeU64 with exchange\n");
	{
		volatile uint64_t value = 3;
		uint64_t comparand = 3;
		uint64_t exchange = 11;
		uint64_t after = fplAtomicCompareAndExchangeU64(&value, comparand, exchange);
		ft::AssertU64Equals((uint64_t)11, (uint64_t)value);
		ft::AssertU64Equals((uint64_t)3, after);
	}
	ft::Msg("Test AtomicCompareAndExchangeU64 no exchange\n");
	{
		volatile uint64_t value = 5;
		uint64_t comparand = 3;
		uint64_t exchange = 11;
		uint64_t after = fplAtomicCompareAndExchangeU64(&value, comparand, exchange);
		ft::AssertU64Equals((uint64_t)5, (uint64_t)value);
		ft::AssertU64Equals((uint64_t)5, after);
	}
	ft::Msg("Test AtomicCompareAndExchangeS64 with exchange\n");
	{
		volatile int64_t value = -3;
		int64_t comparand = -3;
		int64_t exchange = 11;
		int64_t after = fplAtomicCompareAndExchangeS64(&value, comparand, exchange);
		ft::AssertS64Equals((int64_t)11, (int64_t)value);
		ft::AssertS64Equals((int64_t)-3, after);
	}
	ft::Msg("Test AtomicCompareAndExchangeS64 no exchange\n");
	{
		volatile int64_t value = -5;
		int64_t comparand = -3;
		int64_t exchange = 11;
		int64_t after = fplAtomicCompareAndExchangeS64(&value, comparand, exchange);
		ft::AssertS64Equals((int64_t)-5, (int64_t)value);
		ft::AssertS64Equals((int64_t)-5, after);
	}
}

static void TestStrings() {
	ft::Msg("Test ansi string length\n");
	{
		size_t actual = fplGetStringLength(nullptr);
		ft::AssertSizeEquals(0, actual);
	}
	{
		size_t actual = fplGetStringLength("");
		ft::AssertSizeEquals(0, actual);
	}
	{
		size_t actual = fplGetStringLength("ABC");
		ft::AssertSizeEquals(3, actual);
	}
	{
		size_t actual = fplGetStringLength("ABC Hello World!");
		ft::AssertSizeEquals(16, actual);
	}
	{
		char buffer[32];
		buffer[0] = 'A';
		buffer[1] = 'B';
		buffer[2] = 'C';
		buffer[3] = 0;
		size_t actual = fplGetStringLength(buffer);
		ft::AssertSizeEquals(3, actual);
	}

	ft::Msg("Test wide string length\n");
	{
		size_t actual = fplGetStringLengthWide(nullptr);
		ft::AssertSizeEquals(0, actual);
	}
	{
		size_t actual = fplGetStringLengthWide(L"");
		ft::AssertSizeEquals(0, actual);
	}
	{
		size_t actual = fplGetStringLengthWide(L"ABC");
		ft::AssertSizeEquals(3, actual);
	}
	{
		size_t actual = fplGetStringLengthWide(L"ABC Hello World!");
		ft::AssertSizeEquals(16, actual);
	}
	{
		wchar_t buffer[32];
		buffer[0] = 'A';
		buffer[1] = 'B';
		buffer[2] = 'C';
		buffer[3] = 0;
		size_t actual = fplGetStringLengthWide(buffer);
		ft::AssertSizeEquals(3, actual);
	}

	ft::Msg("Test string equal\n");
	{
		bool res = fplIsStringEqual(nullptr, nullptr);
		FT_EXPECTS(true, res);
	}
	{
		bool res = fplIsStringEqual(nullptr, "");
		FT_EXPECTS(false, res);
	}
	{
		bool res = fplIsStringEqual("B", "A");
		FT_EXPECTS(false, res);
	}
	{
		bool res = fplIsStringEqual("A", "A");
		FT_EXPECTS(true, res);
	}
	{
		bool res = fplIsStringEqual("Hello", "World");
		FT_EXPECTS(false, res);
	}
	{
		bool res = fplIsStringEqual("World", "World");
		FT_EXPECTS(true, res);
	}
	{
		bool res = fplIsStringEqualLen(nullptr, 0, nullptr, 0);
		FT_EXPECTS(false, res);
	}
	{
		bool res = fplIsStringEqualLen("", 0, nullptr, 0);
		FT_EXPECTS(false, res);
	}
	{
		bool res = fplIsStringEqualLen(nullptr, 0, "", 0);
		FT_EXPECTS(false, res);
	}
	{
		bool res = fplIsStringEqualLen("", 0, "", 0);
		FT_EXPECTS(true, res);
	}
	{
		bool res = fplIsStringEqualLen("B", 1, "A", 1);
		FT_EXPECTS(false, res);
	}
	{
		bool res = fplIsStringEqualLen("A", 1, "A", 1);
		FT_EXPECTS(true, res);
	}
	{
		bool res = fplIsStringEqualLen("A", 1, "A", 0);
		FT_EXPECTS(false, res);
	}
	{
		bool res = fplIsStringEqualLen("A", 1, "B", 1);
		FT_EXPECTS(false, res);
	}
	{
		bool res = fplIsStringEqualLen("Hello", 5, "World", 5);
		FT_EXPECTS(false, res);
	}
	{
		bool res = fplIsStringEqualLen("Hello", 3, "World", 5);
		FT_EXPECTS(false, res);
	}
	{
		bool res = fplIsStringEqualLen("World", 5, "Hello", 3);
		FT_EXPECTS(false, res);
	}
	{
		bool res = fplIsStringEqualLen("Hello", 5, "Hello", 5);
		FT_EXPECTS(true, res);
	}
	{
		bool res = fplIsStringEqualLen("Hello", 3, "Hello", 3);
		FT_EXPECTS(true, res);
	}

	ft::Msg("Test append string\n");
	{
		FT_IS_NULL(fplStringAppend(fpl_null, fpl_null, 0));
	}
	{
		char buffer[64] = {};
		fplStringAppend(fpl_null, buffer, fplArrayCount(buffer));
		ft::AssertStringEquals("", buffer);
	}
	{
		char buffer[64] = {};
		fplStringAppend("Hello", buffer, fplArrayCount(buffer));
		ft::AssertStringEquals("Hello", buffer);
	}
	{
		char buffer[64] = {};
		fplCopyString("Hello", buffer, fplArrayCount(buffer));
		fplStringAppend(" World", buffer, fplArrayCount(buffer));
		ft::AssertStringEquals("Hello World", buffer);
	}

	ft::Msg("Test format ansi string\n");
	{
		char *res = fplFormatString(nullptr, 0, nullptr);
		FT_EXPECTS(nullptr, res);
	}
	{
		char buffer[1];
		char *res = fplFormatString(buffer, 0, "");
		FT_EXPECTS(nullptr, res);
	}
	{
		char buffer[1];
		char *res = fplFormatString(buffer, fplArrayCount(buffer), "A");
		FT_EXPECTS(nullptr, res);
	}
	{
		char buffer[2];
		char *res = fplFormatString(buffer, fplArrayCount(buffer), "A");
		FT_IS_NOT_NULL(res);
		bool matches = fplIsStringEqualLen("A", 1, buffer, 1);
		FT_EXPECTS(true, matches);
	}
	{
		char buffer[5];
		char *res = fplFormatString(buffer, fplArrayCount(buffer), "Hello");
		FT_EXPECTS(nullptr, res);
	}
	{
		char buffer[6];
		char *res = fplFormatString(buffer, fplArrayCount(buffer), "Hello");
		FT_IS_NOT_NULL(res);
		bool r = fplIsStringEqualLen("Hello", 5, buffer, 5);
		FT_EXPECTS(true, r);
	}
	{
		char buffer[6];
		char *res = fplFormatString(buffer, fplArrayCount(buffer), "%s", "Hello");
		FT_IS_NOT_NULL(res);
		bool r = fplIsStringEqualLen("Hello", 5, buffer, 5);
		FT_EXPECTS(true, r);
	}
	{
		char buffer[20];
		char *res = fplFormatString(buffer, fplArrayCount(buffer), "%4xd-%2d-%2d %2d:%2d:%2d", 2009, 11, 17, 13, 47, 25);
		FT_EXPECTS(nullptr, res);
	}
	{
		char buffer[20];
		char *res = fplFormatString(buffer, fplArrayCount(buffer), "%4d-%2d-%2d %2d:%2d:%2d", 2009, 11, 17, 13, 47, 25);
		FT_IS_NOT_NULL(res);
		bool r = fplIsStringEqual("2009-11-17 13:47:25", buffer);
		FT_EXPECTS(true, r);
	}

	ft::Msg("Test fplS32ToString\n");
	{
		char smallBuffer[2];
		char bigBuffer[16];
		FT_IS_NULL(fplS32ToString(0, 0, nullptr));
		FT_IS_NULL(fplS32ToString(0, 4, nullptr));
		FT_IS_NULL(fplS32ToString(11, fplArrayCount(smallBuffer), smallBuffer));
		FT_IS_NOT_NULL(fplS32ToString(7, fplArrayCount(smallBuffer), smallBuffer));
		ft::AssertStringEquals("7", smallBuffer);
		FT_IS_NOT_NULL(fplS32ToString(129, fplArrayCount(bigBuffer), bigBuffer));
		ft::AssertStringEquals("129", bigBuffer);
		FT_IS_NOT_NULL(fplS32ToString(1337, fplArrayCount(bigBuffer), bigBuffer));
		ft::AssertStringEquals("1337", bigBuffer);
		FT_IS_NOT_NULL(fplS32ToString(-1234567, fplArrayCount(bigBuffer), bigBuffer));
		ft::AssertStringEquals("-1234567", bigBuffer);
	}

	ft::Msg("Test fplStringToS32\n");
	{
		ft::AssertS32Equals(0, fplStringToS32(fpl_null));
		ft::AssertS32Equals(0, fplStringToS32(""));
		ft::AssertS32Equals(0, fplStringToS32("bullshit"));
		ft::AssertS32Equals(0, fplStringToS32("0x"));
		ft::AssertS32Equals(0, fplStringToS32("0xFFBBCCDD"));
		ft::AssertS32Equals(0, fplStringToS32("0"));
		ft::AssertS32Equals(7, fplStringToS32("7"));
		ft::AssertS32Equals(10, fplStringToS32("10"));
		ft::AssertS32Equals(1337, fplStringToS32("1337"));
		ft::AssertS32Equals(-1234567, fplStringToS32("-1234567"));
	}

	ft::Msg("Test fplStringToS32Len\n");
	{
		ft::AssertS32Equals(0, fplStringToS32Len(fpl_null, 0));
		ft::AssertS32Equals(0, fplStringToS32Len(fpl_null, 1));
		ft::AssertS32Equals(0, fplStringToS32Len("", 0));
		ft::AssertS32Equals(0, fplStringToS32Len("123", 0));
		ft::AssertS32Equals(0, fplStringToS32Len("bullshit", 8));
		ft::AssertS32Equals(0, fplStringToS32Len("0x", 2));
		ft::AssertS32Equals(0, fplStringToS32Len("0xFFBBCCDD", 10));
		ft::AssertS32Equals(0, fplStringToS32Len("0", 1));
		ft::AssertS32Equals(7, fplStringToS32Len("7", 1));
		ft::AssertS32Equals(10, fplStringToS32Len("10", 2));
		ft::AssertS32Equals(1337, fplStringToS32Len("1337", 4));
		ft::AssertS32Equals(-1234567, fplStringToS32Len("-1234567", 8));
	}
}

static void TestLocalization() {
	fplPlatformInit(fplInitFlags_None, fpl_null);
	char buffer[16];
	FT_IS_TRUE(fplGetSystemLocale(fplLocaleFormat_ISO639, buffer, fplArrayCount(buffer)));
	fplConsoleFormatOut("System Locale (ISO-639): %s\n", buffer);
	FT_IS_TRUE(fplGetUserLocale(fplLocaleFormat_ISO639, buffer, fplArrayCount(buffer)));
	fplConsoleFormatOut("User Locale (ISO-639): %s\n", buffer);
	FT_IS_TRUE(fplGetInputLocale(fplLocaleFormat_ISO639, buffer, fplArrayCount(buffer)));
	fplConsoleFormatOut("Input Locale (ISO-639): %s\n", buffer);
	fplPlatformRelease();
}

int main(int argc, char *args[]) {
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
