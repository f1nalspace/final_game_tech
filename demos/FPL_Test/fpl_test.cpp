#define FPL_IMPLEMENTATION
#define FPL_NO_AUDIO
#define FPL_NO_VIDEO
#define FPL_NO_WINDOW
#include <final_platform_layer.h>

#define FT_IMPLEMENTATION
#include "final_test.h"

static void TestInit() {
    ft::Line();
	ft::Msg("Test InitPlatform with All init flags\n");
	{
		fplClearPlatformErrors();
		fplInitResultType result = fplPlatformInit(fplInitFlags_All, nullptr);
		FT_ASSERT(result == fplInitResultType_Success);
		const char *errorStr = fplGetPlatformError();
		ft::AssertStringEquals("", errorStr);
		fplPlatformRelease();
	}
	ft::Msg("Test InitPlatform with None init flags\n");
	{
		fplClearPlatformErrors();
		fplInitResultType result = fplPlatformInit(fplInitFlags_None, fpl_null);
		FT_ASSERT(result == fplInitResultType_Success);
		const char *errorStr = fplGetPlatformError();
		ft::AssertStringEquals("", errorStr);
		fplPlatformRelease();
	}
}

static void TestOSInfos() {
    ft::Line();
	ft::Msg("Get Platform Type\n");
	{
		fplPlatformType platType = fplGetPlatformType();
		FT_ASSERT(fplPlatformType_Unknown != platType);
	}
	ft::Msg("Get OS Type\n");
	{
		fplOSInfos osInfos = {};
		bool r = fplGetOperatingSystemInfos(&osInfos);
		//FT_IS_TRUE(r);
		fplConsoleFormatOut("System Name: %s\n", osInfos.systemName);
		fplConsoleFormatOut("System Version: %d.%d.%d.%d\n", osInfos.systemVersion.major, osInfos.systemVersion.minor, osInfos.systemVersion.fix, osInfos.systemVersion.build);
		fplConsoleFormatOut("Kernel Name: %s\n", osInfos.kernelName);
		fplConsoleFormatOut("Kernel Version: %d.%d.%d.%d\n", osInfos.kernelVersion.major, osInfos.kernelVersion.minor, osInfos.kernelVersion.fix, osInfos.kernelVersion.build);
	}
	ft::Msg("Get User Infos\n");
	{
		char nameBuffer[256] = {};
		bool r = fplGetCurrentUsername(nameBuffer, FPL_ARRAYCOUNT(nameBuffer));
		//FT_IS_TRUE(r);
		fplConsoleFormatOut("Current Username: %s\n", nameBuffer);
	}
}

static void TestSizes() {
    ft::Line();
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
    ft::Line();
	//
	// FPL_ARRAYCOUNT
	//
	ft::Msg("[FPL_ARRAYCOUNT] Test static char array\n");
	{
		char staticArray[137] = {};
		uint32_t actual = FPL_ARRAYCOUNT(staticArray);
		FT_EXPECTS(137, actual);
	}
	ft::Msg("[FPL_ARRAYCOUNT] Test static int array\n");
	{
		int staticArray[349] = {};
		uint32_t actual = FPL_ARRAYCOUNT(staticArray);
		FT_EXPECTS(349, actual);
	}
	ft::Msg("[FPL_ARRAYCOUNT] Test static bool array\n");
	{
		bool staticArray[961] = {};
		uint32_t actual = FPL_ARRAYCOUNT(staticArray);
		FT_EXPECTS(961, actual);
	}
	ft::Msg("[FPL_ARRAYCOUNT] Test static void pointer array\n");
	{
		void *staticArray[35] = {};
		uint32_t actual = FPL_ARRAYCOUNT(staticArray);
		FT_EXPECTS(35, actual);
	}

	// @NOTE(final): This is a simple/stupid macro, so when you pass a pointer, you basically get 2 always
	ft::Msg("[FPL_ARRAYCOUNT] Test nullptr\n");
	{
		int *emptyArray = nullptr;
		uint32_t actual = FPL_ARRAYCOUNT(emptyArray);
		uint32_t expected = sizeof(int *) / sizeof(int);
		FT_EXPECTS(expected, actual);
	}
	ft::Msg("[FPL_ARRAYCOUNT] Test pointer from references static array\n");
	{
		int staticArray[3] = {};
		int *refArray = &staticArray[0];
		uint32_t actual = FPL_ARRAYCOUNT(refArray);
		uint32_t expected = sizeof(int *) / sizeof(int);
		FT_EXPECTS(expected, actual);
	}

	//
	// FPL_OFFSETOF
	//
	ft::Msg("[FPL_OFFSETOF] Test alignment of 4 (High to low)\n");
	{
#	pragma pack(push, 4)
		struct TestStruct {
			uint64_t a;
			uint32_t b;
			uint16_t c;
			uint8_t d;
		};
#	pragma pack(pop)
		FT_EXPECTS(0, FPL_OFFSETOF(TestStruct, a));
		FT_EXPECTS(8, FPL_OFFSETOF(TestStruct, b));
		FT_EXPECTS(12, FPL_OFFSETOF(TestStruct, c));
		FT_EXPECTS(14, FPL_OFFSETOF(TestStruct, d));
	}

	ft::Msg("[FPL_OFFSETOF] Test alignment of 4 (Low to High)\n");
	{
#	pragma pack(push, 4)
		struct TestStruct {
			uint8_t a;
			uint16_t b;
			uint32_t c;
			uint64_t d;
		};
#	pragma pack(pop)
		FT_EXPECTS(0, FPL_OFFSETOF(TestStruct, a));
		FT_EXPECTS(2, FPL_OFFSETOF(TestStruct, b));
		FT_EXPECTS(4, FPL_OFFSETOF(TestStruct, c));
		FT_EXPECTS(8, FPL_OFFSETOF(TestStruct, d));
	}

	ft::Msg("[FPL_OFFSETOF] Test alignment of 8 (Low to High)\n");
	{
#	pragma pack(push, 8)
		struct TestStruct {
			uint8_t a;
			uint16_t b;
			uint8_t c[3];
			uint64_t d;
		};
#	pragma pack(pop)
		FT_EXPECTS(0, FPL_OFFSETOF(TestStruct, a));
		FT_EXPECTS(2, FPL_OFFSETOF(TestStruct, b));
		FT_EXPECTS(4, FPL_OFFSETOF(TestStruct, c));
		FT_EXPECTS(8, FPL_OFFSETOF(TestStruct, d));
	}

	//
	// FPL_MIN/FPL_MAX
	//
	ft::Msg("[FPL_MIN] Test integers\n");
	{
		ft::AssertS32Equals(3, FPL_MIN(3, 7));
		ft::AssertS32Equals(3, FPL_MIN(7, 3));
		ft::AssertS32Equals(-7, FPL_MIN(-7, -3));
		ft::AssertS32Equals(-7, FPL_MIN(-3, -7));
		struct TestStruct {
			int a;
			int b;
		};
		TestStruct instance = { 3, 7 };
		TestStruct *instancePtr = &instance;
		ft::AssertS32Equals(3, FPL_MIN(instancePtr->a, instancePtr->b));
	}
	ft::Msg("[FPL_MIN] Test floats\n");
	{
		ft::AssertFloatEquals(3.0f, FPL_MIN(3.0f, 7.0f));
		ft::AssertFloatEquals(3.0f, FPL_MIN(7.0f, 3.0f));
		ft::AssertFloatEquals(-7.0f, FPL_MIN(-7.0f, -3.0f));
		ft::AssertFloatEquals(-7.0f, FPL_MIN(-3.0f, -7.0f));
		struct TestStruct {
			float a;
			float b;
		};
		TestStruct instance = { 3.0f, 7.0f };
		TestStruct *instancePtr = &instance;
		ft::AssertFloatEquals(3.0f, FPL_MIN(instancePtr->a, instancePtr->b));
	}
	ft::Msg("[FPL_MAX] Test integers\n");
	{
		ft::AssertS32Equals(7, FPL_MAX(3, 7));
		ft::AssertS32Equals(7, FPL_MAX(7, 3));
		ft::AssertS32Equals(-3, FPL_MAX(-3, -7));
		ft::AssertS32Equals(-3, FPL_MAX(-7, -3));
		struct TestStruct {
			int a;
			int b;
		};
		TestStruct instance = { 3, 7 };
		TestStruct *instancePtr = &instance;
		ft::AssertS32Equals(7, FPL_MAX(instancePtr->a, instancePtr->b));
	}
	ft::Msg("[FPL_MAX] Test floats\n");
	{
		ft::AssertFloatEquals(7.0f, FPL_MAX(3.0f, 7.0f));
		ft::AssertFloatEquals(7.0f, FPL_MAX(7.0f, 3.0f));
		ft::AssertFloatEquals(-3.0f, FPL_MAX(-3.0f, -7.0f));
		ft::AssertFloatEquals(-3.0f, FPL_MAX(-7.0f, -3.0f));
		struct TestStruct {
			float a;
			float b;
		};
		TestStruct instance = { 3.0f, 7.0f };
		TestStruct *instancePtr = &instance;
		ft::AssertFloatEquals(7.0f, FPL_MAX(instancePtr->a, instancePtr->b));
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
    ft::Line();
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
}

static void TestPaths() {
    ft::Line();
	if(fplPlatformInit(fplInitFlags_None, fpl_null)) {

		char homePathBuffer[1024] = {};
		fplGetHomePath(homePathBuffer, FPL_ARRAYCOUNT(homePathBuffer));
		ft::Msg("Home Path:\n%s\n", homePathBuffer);

		char exeFilePathBuffer[1024] = {};
		fplGetExecutableFilePath(exeFilePathBuffer, FPL_ARRAYCOUNT(exeFilePathBuffer));
		ft::Msg("Executable file Path:\n%s\n", exeFilePathBuffer);

		char extractedPathBuffer[1024] = {};
		fplExtractFilePath(exeFilePathBuffer, extractedPathBuffer, FPL_ARRAYCOUNT(extractedPathBuffer));
		ft::Msg("Extracted path:\n%s\n", extractedPathBuffer);

		const char *exeFileName = fplExtractFileName(exeFilePathBuffer);
		ft::Msg("Extracted filename:\n%s\n", exeFileName);

		const char *exeFileExt = fplExtractFileExtension(exeFilePathBuffer);
		ft::Msg("Extracted extension:\n%s\n", exeFileExt);

		char combinedPathBuffer[1024 * 10] = {};
		fplPathCombine(combinedPathBuffer, FPL_ARRAYCOUNT(combinedPathBuffer), 4, "Hallo", "Welt", "der", "Programmierer");
		ft::Msg("Combined path:\n%s\n", combinedPathBuffer);

		char changedFileExtBuffer[1024] = {};
		fplChangeFileExtension(exeFilePathBuffer, ".obj", changedFileExtBuffer, FPL_ARRAYCOUNT(changedFileExtBuffer));
		ft::Msg("Changed file ext 1:\n%s\n", changedFileExtBuffer);
		fplChangeFileExtension(exeFileName, ".obj", changedFileExtBuffer, FPL_ARRAYCOUNT(changedFileExtBuffer));
		ft::Msg("Changed file ext 2:\n%s\n", changedFileExtBuffer);
		fplChangeFileExtension(".dll", ".obj", changedFileExtBuffer, FPL_ARRAYCOUNT(changedFileExtBuffer));
		ft::Msg("Changed file ext 3:\n%s\n", changedFileExtBuffer);
		fplChangeFileExtension("", ".obj", changedFileExtBuffer, FPL_ARRAYCOUNT(changedFileExtBuffer));
		ft::Msg("Changed file ext 4:\n%s\n", changedFileExtBuffer);
		fplChangeFileExtension(".dll", "", changedFileExtBuffer, FPL_ARRAYCOUNT(changedFileExtBuffer));
		ft::Msg("Changed file ext 5:\n%s\n", changedFileExtBuffer);
		fplChangeFileExtension("", "", changedFileExtBuffer, FPL_ARRAYCOUNT(changedFileExtBuffer));
		ft::Msg("Changed file ext 5:\n%s\n", changedFileExtBuffer);

		fplPlatformRelease();
	}
}

static void TestHardware() {
    ft::Line();

	char cpuNameBuffer[1024] = {};
	fplGetProcessorName(cpuNameBuffer, FPL_ARRAYCOUNT(cpuNameBuffer));
	ft::Msg("Processor name: %s\n", cpuNameBuffer);

	size_t coreCount = fplGetProcessorCoreCount();
	ft::Msg("Processor cores: %z\n", coreCount);

	fplMemoryInfos memInfos;
	if(fplGetRunningMemoryInfos(&memInfos)) {
		ft::Msg("Physical total memory (bytes): %z\n", memInfos.totalPhysicalSize);
		ft::Msg("Physical available memory (bytes): %z\n", memInfos.availablePhysicalSize);
		ft::Msg("Physical used memory (bytes): %z\n", memInfos.usedPhysicalSize);
		ft::Msg("Virtual total memory (bytes): %z\n", memInfos.totalVirtualSize);
		ft::Msg("Virtual used memory (bytes): %z\n", memInfos.usedVirtualSize);
		ft::Msg("Page total memory (bytes): %z\n", memInfos.totalPageSize);
		ft::Msg("Page used memory (bytes): %z\n", memInfos.usedPageSize);
	}

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
	fplMutexHandle lock;
	volatile int32_t value;
	bool useLock;
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
	fplThreadSleep(d->base.sleepFor);
	fplAtomicStoreS32(&d->data->value, d->valueToWrite);
}

static void ReadDataThreadProc(const fplThreadHandle *context, void *data) {
	ReadThreadData *d = (ReadThreadData *)data;
	fplThreadSleep(d->base.sleepFor);
	int32_t actualValue = fplAtomicLoadS32(&d->data->value);
	FT_EXPECTS(d->expectedValue, actualValue);
}

static void SyncThreadsTest() {
	ft::Line();
	ft::Msg("Sync test for 1 reader and 1 writer\n");
	{
		MutableThreadData mutableData = {};
		FT_IS_TRUE(fplMutexInit(&mutableData.lock));
		mutableData.useLock = false;
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
		uint32_t threadCount = FPL_ARRAYCOUNT(threads);

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
		fplMutexDestroy(&mutableData.lock);
	}
}

struct SlaveThreadData {
	ThreadData base;
	fplSignalHandle signal;
	bool isSignaled;
};

struct MasterThreadData {
	ThreadData base;
	fplSignalHandle *signals[FPL__MAX_SIGNAL_COUNT];
	uint32_t signalCount;
};

static void ThreadSlaveProc(const fplThreadHandle *context, void *data) {
	SlaveThreadData *d = (SlaveThreadData *)data;

	ft::Msg("Slave-Thread %d waits for signal\n", d->base.num);
	fplSignalWaitForOne(&d->signal, UINT32_MAX);
	d->isSignaled = true;
	ft::Msg("Got signal on Slave-Thread %d\n", d->base.num);

	ft::Msg("Slave-Thread %d is done\n", d->base.num);
}

static void ThreadMasterProc(const fplThreadHandle *context, void *data) {
	MasterThreadData *d = (MasterThreadData *)data;
	ft::Msg("Master-Thread %d waits for 5 seconds\n", d->base.num);
	fplThreadSleep(5000);

	for(uint32_t signalIndex = 0; signalIndex < d->signalCount; ++signalIndex) {
		ft::Msg("Master-Thread %d sets signal %d\n", d->base.num, signalIndex);
		fplSignalSet(d->signals[signalIndex]);
	}

	ft::Msg("Master-Thread %d is done\n", d->base.num);
}

static void ThreadSignalsTest(const size_t slaveCount) {
	FT_ASSERT(slaveCount > 0);

	size_t threadCount = slaveCount + 1;

	ft::Line();
	ft::Msg("Signals test for %zu threads\n", threadCount);

	MasterThreadData masterData = {};
	masterData.base.num = 1;

	SlaveThreadData slaveDatas[FPL__MAX_THREAD_COUNT] = {};
	for(size_t threadIndex = 0; threadIndex < slaveCount; ++threadIndex) {
		slaveDatas[threadIndex].base.num = masterData.base.num + (int)threadIndex + 1;
		FT_IS_TRUE(fplSignalInit(&slaveDatas[threadIndex].signal, fplSignalValue_Unset));
		size_t i = masterData.signalCount++;
		masterData.signals[i] = &slaveDatas[threadIndex].signal;
	}

	ft::Msg("Start %zu slave threads, 1 master thread\n", slaveCount);
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
	for(size_t slaveIndex = 0; slaveIndex < slaveCount; ++slaveIndex) {
		FT_IS_TRUE(slaveDatas[slaveIndex].isSignaled);
		fplSignalDestroy(&slaveDatas[slaveIndex].signal);
	}
	for(size_t threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
		fplThreadHandle *thread = threads[threadIndex];
		FT_EXPECTS(fplThreadState_Stopped, thread->currentState);
		fplThreadTerminate(thread);
	}
}

struct ConditionThreadSharedData {
	fplMutexHandle mutex;
	fplConditionVariable cond;
};

struct ConditionMasterThreadData {
	ThreadData base;
	ConditionThreadSharedData *shared;
	size_t slaveCount;
};

struct ConditionSlaveThreadData {
	ThreadData base;
	ConditionThreadSharedData *shared;
};

static void ConditionThreadSlaveProc(const fplThreadHandle *context, void *data) {
	ConditionSlaveThreadData *slaveData = (ConditionSlaveThreadData *)data;
	int id = slaveData->base.num;
	ft::Msg("Started Slave-Thread %d\n", id);
	if (fplMutexLock(&slaveData->shared->mutex)) {
		ft::Msg("Wait for Condition on Slave-Thread %d\n", id);
		fplConditionWait(&slaveData->shared->cond, &slaveData->shared->mutex, UINT32_MAX);
		fplMutexUnlock(&slaveData->shared->mutex);
	}
	ft::Msg("Finished Slave-Thread %d\n", id);
}

static void ConditionThreadMasterProc(const fplThreadHandle *context, void *data) {
	ConditionMasterThreadData *masterData = (ConditionMasterThreadData *)data;
	int id = masterData->base.num;
	ft::Msg("Started Master-Thread %d\n", id);
	ft::Msg("Sleep 5 secs in Master-Thread %d\n", id);
	fplThreadSleep(5000);
	if (masterData->slaveCount == 1) {
		ft::Msg("Signal Condition in Master-Thread %d\n", id);
		fplConditionSignal(&masterData->shared->cond);
	} else {
		ft::Msg("Broadcast Condition in Master-Thread %d\n", id);
		fplConditionBroadcast(&masterData->shared->cond);
	}
	ft::Msg("Finished Master-Thread %d\n", id);
}

static void ThreadConditionsTest(size_t slaveCount) {
	ft::Line();
	ft::Msg("Thread Condition Test for %d Slave-Threads\n", slaveCount);

	size_t threadCount = slaveCount + 1;

	ConditionThreadSharedData shared = {};
	FT_ASSERT(fplMutexInit(&shared.mutex));
	FT_ASSERT(fplConditionInit(&shared.cond));

	fplThreadHandle *threads[FPL__MAX_THREAD_COUNT];

	fplThreadHandle *masterThread;
	ConditionMasterThreadData masterData = {};
	masterData.base.num = 1;
	masterData.shared = &shared;
	masterData.slaveCount = slaveCount;

	ConditionSlaveThreadData slaveDatas[FPL__MAX_THREAD_COUNT] = {};

	ft::Msg("Start %zu slave threads, 1 master thread\n", slaveCount);
	for (int threadIndex = 0; threadIndex < slaveCount; ++threadIndex) {
		slaveDatas[threadIndex].base.num = masterData.base.num + threadIndex + 1;
		slaveDatas[threadIndex].shared = &shared;
		threads[threadIndex] = fplThreadCreate(ConditionThreadSlaveProc, &slaveDatas[threadIndex]);
	}
	threads[slaveCount] = fplThreadCreate(ConditionThreadMasterProc, &masterData);
	ft::Msg("Wait for %d Threads to complete\n", threadCount);
	fplThreadWaitForAll(threads, threadCount, UINT32_MAX);

	ft::Msg("Release resources for %zu threads\n", threadCount);
	for(size_t threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
		fplThreadHandle *thread = threads[threadIndex];
		FT_EXPECTS(fplThreadState_Stopped, thread->currentState);
		fplThreadTerminate(thread);
	}
	fplConditionDestroy(&shared.cond);
	fplMutexDestroy(&shared.mutex);
}

static void TestThreading() {
	if(fplPlatformInit(fplInitFlags_None, fpl_null)) {
		size_t coreCount = fplGetProcessorCoreCount();
		size_t threadCountForCores = coreCount > 2 ? coreCount - 1 : 1;

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
			SyncThreadsTest();
		}

		//
		// Condition tests
		//
		{
			ThreadConditionsTest(1);
			ThreadConditionsTest(2);
			ThreadConditionsTest(3);
			ThreadConditionsTest(4);
			ThreadConditionsTest(threadCountForCores);
		}

		//
		// Signal tests
		//
		{
			ThreadSignalsTest(1);
            ThreadSignalsTest(2);
            ThreadSignalsTest(3);
            ThreadSignalsTest(4);
            ThreadSignalsTest(threadCountForCores);
		}

		fplPlatformRelease();
	}
}

static void TestFiles() {
    ft::Line();
	ft::Msg("Test File Exists\n");
	{
		bool nonExisting = fplFileExists("C:\\Windows\\i_am_not_existing.lib");
		FPL_ASSERT(!nonExisting);
		bool notepadExists = fplFileExists("C:\\Windows\\notepad.exe");
		FPL_ASSERT(notepadExists);
	}
	ft::Msg("Test File Size\n");
	{
		uint32_t emptySize = fplGetFileSizeFromPath32("C:\\Windows\\i_am_not_existing.lib");
		FPL_ASSERT(emptySize == 0);
		uint32_t notepadSize = fplGetFileSizeFromPath32("C:\\Windows\\notepad.exe");
		FPL_ASSERT(notepadSize > 0);
	}
	ft::Msg("Test Directory Iterations\n");
	{
		fplFileEntry fileEntry;
		if(fplListFilesBegin("C:\\*", &fileEntry)) {
			ft::Msg("%s\n", fileEntry.path);
			while(fplListFilesNext(&fileEntry)) {
				ft::Msg("%s\n", fileEntry.path);
			}
			fplListFilesEnd(&fileEntry);
		}
	}
}

static void TestAtomics() {
    ft::Line();
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
    ft::Line();
    ft::Msg("Test ansi string length\n");
	{
		size_t actual = fplGetAnsiStringLength(nullptr);
		ft::AssertSizeEquals(0, actual);
	}
	{
		size_t actual = fplGetAnsiStringLength("");
		ft::AssertSizeEquals(0, actual);
	}
	{
		size_t actual = fplGetAnsiStringLength("ABC");
		ft::AssertSizeEquals(3, actual);
	}
	{
		size_t actual = fplGetAnsiStringLength("ABC Hello World!");
		ft::AssertSizeEquals(16, actual);
	}
	{
		char buffer[32];
		buffer[0] = 'A';
		buffer[1] = 'B';
		buffer[2] = 'C';
		buffer[3] = 0;
		size_t actual = fplGetAnsiStringLength(buffer);
		ft::AssertSizeEquals(3, actual);
	}

	ft::Msg("Test wide string length\n");
	{
		size_t actual = fplGetWideStringLength(nullptr);
		ft::AssertSizeEquals(0, actual);
	}
	{
		size_t actual = fplGetWideStringLength(L"");
		ft::AssertSizeEquals(0, actual);
	}
	{
		size_t actual = fplGetWideStringLength(L"ABC");
		ft::AssertSizeEquals(3, actual);
	}
	{
		size_t actual = fplGetWideStringLength(L"ABC Hello World!");
		ft::AssertSizeEquals(16, actual);
	}
	{
		wchar_t buffer[32];
		buffer[0] = 'A';
		buffer[1] = 'B';
		buffer[2] = 'C';
		buffer[3] = 0;
		size_t actual = fplGetWideStringLength(buffer);
		ft::AssertSizeEquals(3, actual);
	}

	ft::Msg("Test string equal\n");
	{
		bool res = fplIsStringEqual(nullptr, nullptr);
		FT_EXPECTS(true, res);
	}
	{
		bool res = fplIsStringEqualLen(nullptr, 0, nullptr, 0);
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
		bool res = fplIsStringEqualLen("B", 1, "A", 1);
		FT_EXPECTS(false, res);
	}
	{
		bool res = fplIsStringEqualLen("A", 1, "A", 1);
		FT_EXPECTS(true, res);
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

	ft::Msg("Test format ansi string\n");
	{
		char *res = fplFormatAnsiString(nullptr, 0, nullptr);
		FT_EXPECTS(nullptr, res);
	}
	{
		char buffer[1];
		char *res = fplFormatAnsiString(buffer, 0, "");
		FT_EXPECTS(nullptr, res);
	}
	{
		char buffer[1];
		char *res = fplFormatAnsiString(buffer, FPL_ARRAYCOUNT(buffer), "A");
		FT_EXPECTS(nullptr, res);
	}
	{
		char buffer[2];
		char *res = fplFormatAnsiString(buffer, FPL_ARRAYCOUNT(buffer), "A");
		bool matches = fplIsStringEqualLen("A", 1, res, 1);
		FT_EXPECTS(true, matches);
	}
	{
		char buffer[5];
		char *res = fplFormatAnsiString(buffer, FPL_ARRAYCOUNT(buffer), "Hello");
		FT_EXPECTS(nullptr, res);
	}
	{
		char buffer[6];
		char *res = fplFormatAnsiString(buffer, FPL_ARRAYCOUNT(buffer), "Hello");
		bool r = fplIsStringEqualLen("Hello", 5, res, 5);
		FT_EXPECTS(true, r);
	}
	{
		char buffer[6];
		char *res = fplFormatAnsiString(buffer, FPL_ARRAYCOUNT(buffer), "%s", "Hello");
		bool r = fplIsStringEqualLen("Hello", 5, res, 5);
		FT_EXPECTS(true, r);
	}
	{
		char buffer[20];
		char *res = fplFormatAnsiString(buffer, FPL_ARRAYCOUNT(buffer), "%4xd-%2d-%2d %2d:%2d:%2d", 2009, 11, 17, 13, 47, 25);
		FT_EXPECTS(nullptr, res);
	}
	{
		char buffer[20];
		char *res = fplFormatAnsiString(buffer, FPL_ARRAYCOUNT(buffer), "%4d-%2d-%2d %2d:%2d:%2d", 2009, 11, 17, 13, 47, 25);
		bool r = fplIsStringEqual("2009-11-17 13:47:25", res);
		FT_EXPECTS(true, r);
	}
}


int main(int argc, char *args[]) {
	TestOSInfos();
	TestHardware();
	TestSizes();
	TestMacros();
	TestAtomics();
	TestMemory();
	TestPaths();
	TestFiles();
	TestStrings();
	TestThreading();
	TestInit();
	return 0;
}
