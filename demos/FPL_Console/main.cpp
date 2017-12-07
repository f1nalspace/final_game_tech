/**
 * This contains all the "unit"-tests for making sure that everything works.
 */

#define FPL_IMPLEMENTATION
#define FPL_NO_WINDOW
#define FPL_AUTO_NAMESPACE

// @NOTE(final): Any assert should fire immediatly, regardless of the configuration
#define FPL_FORCE_ASSERTIONS
#include "final_platform_layer.hpp"

// @NOTE(final): C++ Standard Library (We dont want to use fpl here, because we want to test it from independent systems)
#include <iostream> // cout
#include <string> // string
#include <stdarg.h> // va_list, va_start, va_end
#include <typeinfo> // typeid

#define ASSERTION_CRASH() {*(int *)0 = 0xBAD;}

#if defined(FPL_COMPILER_GCC)
#	define __FUNCTION__ ""
#endif

struct TestLineAssertionInfo {
	char *filename;
	char *functionName;
	int line;
};

template <typename T>
static void TestAssert(const T &expected, const T &actual, const TestLineAssertionInfo &lineInfo, const std::string &message = "") {
	bool success = (expected == actual);
	if (!success) {
		std::cerr << "Failed assertion in file '" << lineInfo.filename << "', function '" << lineInfo.functionName << "', line " << lineInfo.line;
		if (message.size() > 0) {
			std::cerr << " -> " << message;
		}
		std::cerr << std::endl;
		std::cerr << "Expected type '" << typeid(T).name() << "' of '" << expected << "' but got '" << actual << "'!" << std::endl;
		ASSERTION_CRASH();
	}
}

template <typename T>
static void TestNotAssert(const T &notExpected, const T &actual, const TestLineAssertionInfo &lineInfo, const std::string &message = "") {
	bool success = (notExpected != actual);
	if (!success) {
		std::cerr << "Failed assertion in file '" << lineInfo.filename << "', function '" << lineInfo.functionName << "', line " << lineInfo.line;
		if (message.size() > 0) {
			std::cerr << " -> " << message;
		}
		std::cerr << std::endl;
		std::cerr << "Expected type '" << typeid(T).name() << "' of not '" << notExpected << "' but got '" << actual << "'!" << std::endl;
		ASSERTION_CRASH();
	}
}

static void TestLog(const char *section, const char *format, ...) {
	char buffer[2048];
	va_list argList;
	va_start(argList, format);
	#if defined(__STDC_WANT_SECURE_LIB__) && __STDC_WANT_SECURE_LIB__
	vsprintf_s(buffer, FPL_ARRAYCOUNT(buffer), format, argList);
	#else
	vsprintf(buffer, format, argList);
	#endif
	va_end(argList);
	std::cout << "[" << section << "] " << buffer << std::endl;
}

#define LAI {__FILE__, __FUNCTION__, __LINE__}
#define FN __FUNCTION__

static void MemoryTests() {
	TestLog(FN, "Test size macros");
	{
		TestAssert<size_t>(0ull, FPL_KILOBYTES(0), LAI, "0 KB");
		TestAssert<size_t>(0ull, FPL_MEGABYTES(0), LAI, "0 MB");
		TestAssert<size_t>(0ull, FPL_GIGABYTES(0), LAI, "0 GB");
		TestAssert<size_t>(0ull, FPL_TERABYTES(0), LAI, "0 TB");
		TestAssert<size_t>(13ull * 1024ull, FPL_KILOBYTES(13), LAI, "13 KB");
		TestAssert<size_t>(137ull * 1024ull * 1024ull, FPL_MEGABYTES(137), LAI, "137 MB");
		TestAssert<size_t>(3ull * 1024ull * 1024ull * 1024ull, FPL_GIGABYTES(3), LAI, "3 GB");
#if defined(FPL_ARCH_X64)
		TestAssert<size_t>(813ull * 1024ull * 1024ull * 1024ull, FPL_GIGABYTES(813), LAI, "813 GB");
		TestAssert<size_t>(2ull * 1024ull * 1024ull * 1024ull * 1024ull, FPL_TERABYTES(2), LAI, "2 TB");
#endif
	}

	TestLog(FN, "Test normal allocation and deallocation");
	{
		size_t memSize = FPL_KILOBYTES(42);
		uint8_t *mem = (uint8_t *)MemoryAllocate(memSize);
		for (size_t i = 0; i < memSize; ++i) {
			uint8_t value = *mem++;
			TestAssert<uint8_t>(0, value, LAI, "42 KB must be zero");
		}
		MemoryFree(mem);
	}
	{
		size_t memSize = FPL_MEGABYTES(512);
		void *mem = MemoryAllocate(memSize);
		TestNotAssert<void *>(mem, nullptr, LAI, "512 MB of memory must be allocatd");
		MemoryFree(mem);
	}

	TestLog(FN, "Test aligned allocation and deallocation");
	{
		size_t memSize = FPL_KILOBYTES(42);
		uint8_t *mem = (uint8_t *)MemoryAlignedAllocate(memSize, 16);
		for (size_t i = 0; i < memSize; ++i) {
			uint8_t value = *(mem + i);
			TestAssert<uint8_t>(0, value, LAI, "42 KB must be zero");
		}
		MemoryAlignedFree(mem);
	}
	{
		size_t memSize = FPL_MEGABYTES(512);
		void *mem = MemoryAlignedAllocate(memSize, 16);
		TestNotAssert<void *>(mem, nullptr, LAI, "512 MB of memory must be allocatd");
		MemoryAlignedFree(mem);
	}
}

static void PathTests() {
	char homePathBuffer[1024] = {};
	GetHomePath(homePathBuffer, FPL_ARRAYCOUNT(homePathBuffer));
	ConsoleFormatOut("Home Path:\n%s\n", homePathBuffer);

	char exeFilePathBuffer[1024] = {};
	GetExecutableFilePath(exeFilePathBuffer, FPL_ARRAYCOUNT(exeFilePathBuffer));
	ConsoleFormatOut("Executable file Path:\n%s\n", exeFilePathBuffer);

	char extractedPathBuffer[1024] = {};
	ExtractFilePath(exeFilePathBuffer, extractedPathBuffer, FPL_ARRAYCOUNT(extractedPathBuffer));
	ConsoleFormatOut("Extracted path:\n%s\n", extractedPathBuffer);

	char *exeFileName = ExtractFileName(exeFilePathBuffer);
	ConsoleFormatOut("Extracted filename:\n%s\n", exeFileName);

	char *exeFileExt = ExtractFileExtension(exeFilePathBuffer);
	ConsoleFormatOut("Extracted extension:\n%s\n", exeFileExt);

	char combinedPathBuffer[1024 * 10] = {};
	CombinePath(combinedPathBuffer, FPL_ARRAYCOUNT(combinedPathBuffer), 4, "Hallo", "Welt", "der", "Programmierer");
	ConsoleFormatOut("Combined path:\n%s\n", combinedPathBuffer);

	char changedFileExtBuffer[1024] = {};
	ChangeFileExtension(exeFilePathBuffer, ".obj", changedFileExtBuffer, FPL_ARRAYCOUNT(changedFileExtBuffer));
	ConsoleFormatOut("Changed file ext 1:\n%s\n", changedFileExtBuffer);
	ChangeFileExtension(exeFileName, ".obj", changedFileExtBuffer, FPL_ARRAYCOUNT(changedFileExtBuffer));
	ConsoleFormatOut("Changed file ext 2:\n%s\n", changedFileExtBuffer);
	ChangeFileExtension(".dll", ".obj", changedFileExtBuffer, FPL_ARRAYCOUNT(changedFileExtBuffer));
	ConsoleFormatOut("Changed file ext 3:\n%s\n", changedFileExtBuffer);
	ChangeFileExtension("", ".obj", changedFileExtBuffer, FPL_ARRAYCOUNT(changedFileExtBuffer));
	ConsoleFormatOut("Changed file ext 4:\n%s\n", changedFileExtBuffer);
	ChangeFileExtension(".dll", "", changedFileExtBuffer, FPL_ARRAYCOUNT(changedFileExtBuffer));
	ConsoleFormatOut("Changed file ext 5:\n%s\n", changedFileExtBuffer);
	ChangeFileExtension("", "", changedFileExtBuffer, FPL_ARRAYCOUNT(changedFileExtBuffer));
	ConsoleFormatOut("Changed file ext 5:\n%s\n", changedFileExtBuffer);
}

static void HardwareTest() {
	char cpuNameBuffer[1024] = {};
	GetProcessorName(cpuNameBuffer, FPL_ARRAYCOUNT(cpuNameBuffer));
	ConsoleFormatOut("Processor name:\n%s\n", cpuNameBuffer);

	uint32_t coreCount = GetProcessorCoreCount();
	ConsoleFormatOut("Processor cores:%d\n", coreCount);
}

static void FilesTest() {
	bool nonExisting = FileExists("C:\\Windows\\i_am_not_existing.lib");
	FPL_ASSERT(!nonExisting);
	bool notepadExists = FileExists("C:\\Windows\\notepad.exe");
	FPL_ASSERT(notepadExists);
	uint32_t emptySize = GetFileSize32("C:\\Windows\\i_am_not_existing.lib");
	FPL_ASSERT(emptySize == 0);
	uint32_t notepadSize = GetFileSize32("C:\\Windows\\notepad.exe");
	FPL_ASSERT(notepadSize > 0);
	FileEntry fileEntry;
	if (ListFilesBegin("C:\\*", &fileEntry)) {
		ConsoleFormatOut("%s\n", fileEntry.path);
		while (ListFilesNext(&fileEntry)) {
			ConsoleFormatOut("%s\n", fileEntry.path);
		}
		ListFilesEnd(&fileEntry);
	}
}

static void TestThreadProc(const ThreadContext &context, void *data) {
	uint32_t ms = (uint32_t)(intptr_t)(data) * 1000;
	ConsoleFormatOut("Thread '%llu' started\n", context.id);
	ThreadSleep(ms);
	ConsoleFormatOut("Thread '%llu' finished\n", context.id);
}

static void ThreadingTest() {
	ThreadContext *threads[3];
	threads[0] = ThreadCreate(TestThreadProc, (void *)1);
	threads[1] = ThreadCreate(TestThreadProc, (void *)2);
	threads[2] = ThreadCreate(TestThreadProc, (void *)3);
	ThreadWaitForAll(threads, FPL_ARRAYCOUNT(threads));
}


int main(int argc, char **args) {
	InitPlatform(InitFlags::None);
	MemoryTests();
	ThreadingTest();
	HardwareTest();
	PathTests();
	FilesTest();
	ReleasePlatform();
}
