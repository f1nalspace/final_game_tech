/*
@TODO(final): Disable all libs like kernel32.lib and user32.lib, so we cacn test the win32 api runtime loading of libraries.
*/
#define FPL_IMPLEMENTATION
#define FPL_NO_WINDOW
#define FPL_NO_OPENGL
#define FPL_AUTO_NAMESPACE
#include "final_platform_layer.hpp"

static void MemoryTests() {
	uint8_t *mem8 = (uint8_t *)MemoryAllocate(sizeof(uint8_t) * 2048);
	MemoryFree(mem8);
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
	ThreadContext threads[3];
	threads[0] = ThreadCreate(TestThreadProc, (void *)1);
	threads[1] = ThreadCreate(TestThreadProc, (void *)2);
	threads[2] = ThreadCreate(TestThreadProc, (void *)3);
	ThreadWaitForMultiple(threads, FPL_ARRAYCOUNT(threads));
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