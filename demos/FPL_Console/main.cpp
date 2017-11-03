#define FPL_IMPLEMENTATION
#define FPL_ENABLE_WINDOW 0
#define FPL_ENABLE_OPENGL 0
#define FPL_AUTO_NAMESPACE 1
#include "final_platform_layer.hpp"

static void MemoryTests() {
	uint8_t *mem8 = (uint8_t *)AllocateMemory(sizeof(uint8_t) * 2048);
}

static void PathTests() {
	char homePathBuffer[1024] = {};
	GetHomePath(homePathBuffer, FPL_ARRAYCOUNT(homePathBuffer));
	ConsoleFormatOut("Home Path:\n%s\n", homePathBuffer);
	ConsoleFormatOut("Home Path (Direct):\n%s\n", GetHomePath());

	char exeFilePathBuffer[1024] = {};
	GetExecutableFilePath(exeFilePathBuffer, FPL_ARRAYCOUNT(exeFilePathBuffer));
	ConsoleFormatOut("Executable file Path:\n%s\n", exeFilePathBuffer);
	ConsoleFormatOut("Executable file Path (Direct):\n%s\n", GetExecutableFilePath());

	char extractedPathBuffer[1024] = {};
	ExtractFilePath(exeFilePathBuffer, extractedPathBuffer, FPL_ARRAYCOUNT(extractedPathBuffer));
	ConsoleFormatOut("Extracted path:\n%s\n", extractedPathBuffer);
	ConsoleFormatOut("Extracted path (Direct):\n%s\n", ExtractFilePath(exeFilePathBuffer));

	char *exeFileName = ExtractFileName(exeFilePathBuffer);
	ConsoleFormatOut("Extracted filename:\n%s\n", exeFileName);

	char *exeFileExt = ExtractFileExtension(exeFilePathBuffer);
	ConsoleFormatOut("Extracted extension:\n%s\n", exeFileExt);

	char combinedPathBuffer[1024 * 10] = {};
	CombinePath(combinedPathBuffer, FPL_ARRAYCOUNT(combinedPathBuffer), 4, "Hallo", "Welt", "der", "Programmierer");
	ConsoleFormatOut("Combined path:\n%s\n", combinedPathBuffer);
	ConsoleFormatOut("Combined path (Direct):\n%s\n", CombinePath(4, "Hallo", "Welt", "der", "Programmierer"));

	char changedFileExtBuffer[1024] = {};
	ChangeFileExtension(exeFilePathBuffer, ".obj", changedFileExtBuffer, FPL_ARRAYCOUNT(changedFileExtBuffer));
	ConsoleFormatOut("Changed file ext 1:\n%s\n", changedFileExtBuffer);
	ConsoleFormatOut("Changed file ext 1 (Direct):\n%s\n", ChangeFileExtension(exeFilePathBuffer, ".obj"));
	ChangeFileExtension(exeFileName, ".obj", changedFileExtBuffer, FPL_ARRAYCOUNT(changedFileExtBuffer));
	ConsoleFormatOut("Changed file ext 2:\n%s\n", changedFileExtBuffer);
	ConsoleFormatOut("Changed file ext 2 (Direct):\n%s\n", ChangeFileExtension(exeFileName, ".obj"));
	ChangeFileExtension(".dll", ".obj", changedFileExtBuffer, FPL_ARRAYCOUNT(changedFileExtBuffer));
	ConsoleFormatOut("Changed file ext 3:\n%s\n", changedFileExtBuffer);
	ConsoleFormatOut("Changed file ext 3 (Direct):\n%s\n", ChangeFileExtension(".dll", ".obj"));
	ChangeFileExtension("", ".obj", changedFileExtBuffer, FPL_ARRAYCOUNT(changedFileExtBuffer));
	ConsoleFormatOut("Changed file ext 4:\n%s\n", changedFileExtBuffer);
	ConsoleFormatOut("Changed file ext 4 (Direct):\n%s\n", ChangeFileExtension("", ".obj"));
	ChangeFileExtension(".dll", "", changedFileExtBuffer, FPL_ARRAYCOUNT(changedFileExtBuffer));
	ConsoleFormatOut("Changed file ext 5:\n%s\n", changedFileExtBuffer);
	ConsoleFormatOut("Changed file ext 5 (Direct):\n%s\n", ChangeFileExtension(".dll", ""));
	ChangeFileExtension("", "", changedFileExtBuffer, FPL_ARRAYCOUNT(changedFileExtBuffer));
	ConsoleFormatOut("Changed file ext 5:\n%s\n", changedFileExtBuffer);
	ConsoleFormatOut("Changed file ext 5 (Direct):\n%s\n", ChangeFileExtension("", ""));

	char cpuNameBuffer[1024] = {};
	GetProcessorName(cpuNameBuffer, FPL_ARRAYCOUNT(cpuNameBuffer));
	ConsoleFormatOut("Processor name:\n%s\n", cpuNameBuffer);
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

int main(int argc, char **args) {
	InitPlatform(InitFlags::None);
	PathTests();
	FilesTest();
	ReleasePlatform();
}
