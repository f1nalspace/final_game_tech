#define FPL_IMPLEMENTATION
#define FPL_ENABLE_WINDOW 0
#define FPL_ENABLE_OPENGL 0
#define FPL_ENABLE_C_RUNTIME_LIBRARY 1
#include "final_platform_layer.h"

static void PathTests() {
	char homePathBuffer[1024];
	fpl_GetHomePath(homePathBuffer, FPL_ARRAYCOUNT(homePathBuffer));
	fpl_ConsoleFormatOut("Home Path:\n%s\n", homePathBuffer);

	char exeFilePathBuffer[1024];
	fpl_GetExecutableFilePath(exeFilePathBuffer, FPL_ARRAYCOUNT(exeFilePathBuffer));
	fpl_ConsoleFormatOut("Executable file Path:\n%s\n", exeFilePathBuffer);

	char extractedPathBuffer[1024];
	fpl_ExtractFilePath(extractedPathBuffer, FPL_ARRAYCOUNT(extractedPathBuffer), exeFilePathBuffer);
	fpl_ConsoleFormatOut("Extracted path:\n%s\n", extractedPathBuffer);

	char *exeFileName = fpl_ExtractFileName(exeFilePathBuffer);
	fpl_ConsoleFormatOut("Extracted filename:\n%s\n", exeFileName);

	char *exeFileExt = fpl_ExtractFileExtension(exeFilePathBuffer);
	fpl_ConsoleFormatOut("Extracted extension:\n%s\n", exeFileExt);

	char combinedPathBuffer[1024 * 10] = { 0 };
	fpl_CombinePath(combinedPathBuffer, FPL_ARRAYCOUNT(combinedPathBuffer), 4, "Hallo", "Welt", "der", "Programmierer");
	fpl_ConsoleFormatOut("Combined path:\n%s\n", combinedPathBuffer);

	char changedFileExtBuffer[1024] = { 0 };
	fpl_ChangeFileExtension(changedFileExtBuffer, FPL_ARRAYCOUNT(changedFileExtBuffer), exeFilePathBuffer, ".obj");
	fpl_ConsoleFormatOut("Changed file ext 1:\n%s\n", changedFileExtBuffer);
	fpl_ChangeFileExtension(changedFileExtBuffer, FPL_ARRAYCOUNT(changedFileExtBuffer), exeFileName, ".obj");
	fpl_ConsoleFormatOut("Changed file ext 2:\n%s\n", changedFileExtBuffer);
	fpl_ChangeFileExtension(changedFileExtBuffer, FPL_ARRAYCOUNT(changedFileExtBuffer), ".dll", ".obj");
	fpl_ConsoleFormatOut("Changed file ext 3:\n%s\n", changedFileExtBuffer);
	fpl_ChangeFileExtension(changedFileExtBuffer, FPL_ARRAYCOUNT(changedFileExtBuffer), "", ".obj");
	fpl_ConsoleFormatOut("Changed file ext 4:\n%s\n", changedFileExtBuffer);
	fpl_ChangeFileExtension(changedFileExtBuffer, FPL_ARRAYCOUNT(changedFileExtBuffer), ".dll", "");
	fpl_ConsoleFormatOut("Changed file ext 5:\n%s\n", changedFileExtBuffer);
	fpl_ChangeFileExtension(changedFileExtBuffer, FPL_ARRAYCOUNT(changedFileExtBuffer), "", "");
	fpl_ConsoleFormatOut("Changed file ext 5:\n%s\n", changedFileExtBuffer);
}

static void FilesTest() {
	fpl_bool32 nonExisting = fpl_FileExists("C:\\Windows\\i_am_not_existing.lib");
	FPL_ASSERT(!nonExisting);
	fpl_bool32 notepadExists = fpl_FileExists("C:\\Windows\\notepad.exe");
	FPL_ASSERT(notepadExists);
	uint32_t emptySize = fpl_GetFileSize32("C:\\Windows\\i_am_not_existing.lib");
	FPL_ASSERT(emptySize == 0);
	uint32_t notepadSize = fpl_GetFileSize32("C:\\Windows\\notepad.exe");
	FPL_ASSERT(notepadSize > 0);
	fpl_FileEntry fileEntry;
	if (fpl_ListFilesBegin("C:\\*", &fileEntry)) {
		fpl_ConsoleFormatOut("%s\n", fileEntry.path);
		while (fpl_ListFilesNext(&fileEntry)) {
			fpl_ConsoleFormatOut("%s\n", fileEntry.path);
		}
		fpl_ListFilesEnd(&fileEntry);
	}
}

int main(int argc, char **args) {
	fpl_Init(fpl_InitFlags_None);
	PathTests();
	FilesTest();
	fpl_Release();
}
