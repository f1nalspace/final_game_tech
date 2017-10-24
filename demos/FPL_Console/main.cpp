#define FPL_IMPLEMENTATION
#define FPL_ENABLE_WINDOW 0
#define FPL_ENABLE_OPENGL 0
#include "final_platform_layer.hpp"

static void MemoryTests() {
	using namespace fpl;
	using namespace fpl::memory;

	uint8_t *mem8 = (uint8_t *)AllocateMemory(sizeof(uint8_t) * 2048);
}

static void PathTests() {
	using namespace fpl;
	using namespace fpl::console;
	using namespace fpl::paths;
	using namespace fpl::hardware;

	char homePathBuffer[1024] = {};
	GetHomePath(homePathBuffer, FPL_ARRAYCOUNT(homePathBuffer));
	ConsoleFormatOut("Home Path:\n%s\n", homePathBuffer);

	char exeFilePathBuffer[1024] = {};
	GetExecutableFilePath(exeFilePathBuffer, FPL_ARRAYCOUNT(exeFilePathBuffer));
	ConsoleFormatOut("Executable file Path:\n%s\n", exeFilePathBuffer);

	char extractedPathBuffer[1024] = {};
	ExtractFilePath(extractedPathBuffer, FPL_ARRAYCOUNT(extractedPathBuffer), exeFilePathBuffer);
	ConsoleFormatOut("Extracted path:\n%s\n", extractedPathBuffer);

	char *exeFileName = ExtractFileName(exeFilePathBuffer);
	ConsoleFormatOut("Extracted filename:\n%s\n", exeFileName);

	char *exeFileExt = ExtractFileExtension(exeFilePathBuffer);
	ConsoleFormatOut("Extracted extension:\n%s\n", exeFileExt);

	char combinedPathBuffer[1024 * 10] = {};
	CombinePath(combinedPathBuffer, FPL_ARRAYCOUNT(combinedPathBuffer), 4, "Hallo", "Welt", "der", "Programmierer");
	ConsoleFormatOut("Combined path:\n%s\n", combinedPathBuffer);

	char changedFileExtBuffer[1024] = {};
	ChangeFileExtension(changedFileExtBuffer, FPL_ARRAYCOUNT(changedFileExtBuffer), exeFilePathBuffer, ".obj");
	ConsoleFormatOut("Changed file ext 1:\n%s\n", changedFileExtBuffer);
	ChangeFileExtension(changedFileExtBuffer, FPL_ARRAYCOUNT(changedFileExtBuffer), exeFileName, ".obj");
	ConsoleFormatOut("Changed file ext 2:\n%s\n", changedFileExtBuffer);
	ChangeFileExtension(changedFileExtBuffer, FPL_ARRAYCOUNT(changedFileExtBuffer), ".dll", ".obj");
	ConsoleFormatOut("Changed file ext 3:\n%s\n", changedFileExtBuffer);
	ChangeFileExtension(changedFileExtBuffer, FPL_ARRAYCOUNT(changedFileExtBuffer), "", ".obj");
	ConsoleFormatOut("Changed file ext 4:\n%s\n", changedFileExtBuffer);
	ChangeFileExtension(changedFileExtBuffer, FPL_ARRAYCOUNT(changedFileExtBuffer), ".dll", "");
	ConsoleFormatOut("Changed file ext 5:\n%s\n", changedFileExtBuffer);
	ChangeFileExtension(changedFileExtBuffer, FPL_ARRAYCOUNT(changedFileExtBuffer), "", "");
	ConsoleFormatOut("Changed file ext 5:\n%s\n", changedFileExtBuffer);

	char cpuNameBuffer[1024] = {};
	GetProcessorName(cpuNameBuffer, FPL_ARRAYCOUNT(cpuNameBuffer));
	ConsoleFormatOut("Processor name:\n%s\n", cpuNameBuffer);
}

static void FilesTest() {
	using namespace fpl;
	using namespace fpl::console;
	using namespace fpl::files;

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

int main(int argc, char **args) {
	using namespace fpl;

	InitPlatform(InitFlags::None);
	PathTests();
	FilesTest();
	ReleasePlatform();
}
