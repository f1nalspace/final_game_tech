/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Process Tests

Description:
	Tests for the fplProcess* API, including the failure paths.

	The tests need a program to start. Instead of depending on any system tool,
	the test executable starts itself with one of the "--child-" arguments and
	uses the exit code and the captured text of that child as the answer. This
	behaves identically on every platform and needs no external files.

	Only the shell tests write a file, because a script is the reason the shell
	mode exists. That script is written next to the test executable and removed
	afterwards.

License:
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

/* This translation unit consumes the FPL public API only.
 * fpl_test.c owns FPL_IMPLEMENTATION; do NOT define it here. */
#include <final_platform_layer.h>

//
// Child modes, started by the tests through the test executable itself
//
#define FPL_TEST_CHILD_ARGUMENT_ECHO "--child-echo"
#define FPL_TEST_CHILD_ARGUMENT_ERROR "--child-error"
#define FPL_TEST_CHILD_ARGUMENT_EXIT "--child-exit"
#define FPL_TEST_CHILD_ARGUMENT_SLEEP "--child-sleep"
#define FPL_TEST_CHILD_ARGUMENT_CHECK_ARGS "--child-checkargs"
#define FPL_TEST_CHILD_ARGUMENT_CHECK_FILE "--child-checkfile"
#define FPL_TEST_CHILD_ARGUMENT_BOTH "--child-both"
#define FPL_TEST_CHILD_ARGUMENT_SPAM "--child-spam"
#define FPL_TEST_CHILD_ARGUMENT_LINES "--child-lines"
#define FPL_TEST_CHILD_ARGUMENT_CAT "--child-cat"

// Exit code a child reports when everything was as expected
static const int32_t processTestChildSuccessExitCode = 0;
// Exit code a child reports when the expectation was not met
static const int32_t processTestChildFailureExitCode = 1;
// Exit code the exit-code tests ask the child for, a value no runtime uses on its own
static const int32_t processTestCustomExitCode = 42;
// Argument line for a child that does nothing but exit successfully. Every test that does not care about
// the text of the child uses it, because a child without a redirected stream writes into the test console.
static const char *processTestSilentChildArgumentLine = FPL_TEST_CHILD_ARGUMENT_EXIT " 0";
// Number of milliseconds a long running child sleeps, long enough to poll and stop it
static const int32_t processTestLongSleepInMilliseconds = 30000;
// Number of milliseconds we wait for a child that is expected to keep running
static const fplTimeoutValue processTestShortTimeout = 150;
// Number of start/wait/close cycles used to detect handle or process table leaks
static const int processTestCycleCount = 25;
// Name of the marker file used to prove that the work directory was applied
static const char *processTestMarkerFileName = "fpl_test_workdir_marker.tmp";
// Text the echo child writes to the standard-output
static const char *processTestEchoText = "captured-output-line";
// Text the error child writes to the standard-error
static const char *processTestErrorText = "captured-error-line";
// Number of characters written in one go by the spam child
static const size_t processTestSpamChunkSize = 1024;
// Number of characters the spam child writes in total, deliberately not a multiple of the chunk size
static const int32_t processTestSpamTotalSize = 1000000;
// Number of characters the capture is limited to in the truncation test
static const size_t processTestCaptureLimit = 4096;
// Text every line of the line child starts with, the line number is appended to it
static const char *processTestLineTextPrefix = "line-";
// Text the line child writes as its last line, deliberately without any line ending
static const char *processTestLineTailText = "tail";
// Number of complete lines the line child writes before the incomplete one
static const int32_t processTestLineCount = 5;
// Text written into the standard-input of the cat child
static const char *processTestInputText = "first input line\nsecond input line\n";
// Number of characters the cat child reads in one go
#define PROCESS_TEST_CAT_BUFFER_SIZE 4096
// Number of characters written into the standard-input in the big input test, far more than one pipe buffer
static const int32_t processTestBigInputSize = 262144;
// Number of chunks the input callback provides before it ends the standard-input
static const int32_t processTestInputChunkCount = 4;
// Name of the script file the shell tests write next to the test executable
#if defined(FPL_PLATFORM_WINDOWS)
static const char *processTestScriptFileName = "fpl_test_shell_script.bat";
#else
static const char *processTestScriptFileName = "fpl_test_shell_script.sh";
#endif
// Text the script writes to the standard-output
static const char *processTestScriptText = "text-from-the-script";
// Exit code the script ends with, so the exit code of a script can be checked as well
static const int32_t processTestScriptExitCode = 7;
// Number of milliseconds we wait for a child that was asked to stop gracefully
static const fplTimeoutValue processTestStopTimeout = 5000;

// Arguments passed to the child, so it can verify that they arrived unchanged
static const char *processTestArgumentValues[] = {
	"plain",
	"with spaces",
	"with \"quotes\"",
};

//
// Child side
//

typedef enum ProcessTestChildMode {
	//! This is a normal test run and not a child invocation.
	ProcessTestChildMode_None = 0,
	//! Write the text to the standard-output.
	ProcessTestChildMode_Echo,
	//! Write the text to the standard-error.
	ProcessTestChildMode_Error,
	//! Exit with the requested exit code.
	ProcessTestChildMode_Exit,
	//! Sleep for the requested number of milliseconds.
	ProcessTestChildMode_Sleep,
	//! Compare the passed arguments against the expected ones.
	ProcessTestChildMode_CheckArgs,
	//! Check whether the relative file exists in the current work directory.
	ProcessTestChildMode_CheckFile,
	//! Write one line to the standard-output and one line to the standard-error.
	ProcessTestChildMode_Both,
	//! Write the requested number of characters to the standard-output.
	ProcessTestChildMode_Spam,
	//! Write complete lines and one incomplete line to the standard-output.
	ProcessTestChildMode_Lines,
	//! Read the standard-input to its end and write it back to the standard-output.
	ProcessTestChildMode_Cat,
} ProcessTestChildMode;

// Detects a child invocation without initializing anything, so a normal test run is left untouched
static ProcessTestChildMode ProcessTestsGetChildMode(const int argc, char *args[]) {
	if (argc < 2) {
		return(ProcessTestChildMode_None);
	}
	const char *modeArgument = args[1];
	if (fplIsStringEqual(modeArgument, FPL_TEST_CHILD_ARGUMENT_ECHO)) {
		return(ProcessTestChildMode_Echo);
	}
	if (fplIsStringEqual(modeArgument, FPL_TEST_CHILD_ARGUMENT_ERROR)) {
		return(ProcessTestChildMode_Error);
	}
	if (fplIsStringEqual(modeArgument, FPL_TEST_CHILD_ARGUMENT_EXIT)) {
		return(ProcessTestChildMode_Exit);
	}
	if (fplIsStringEqual(modeArgument, FPL_TEST_CHILD_ARGUMENT_SLEEP)) {
		return(ProcessTestChildMode_Sleep);
	}
	if (fplIsStringEqual(modeArgument, FPL_TEST_CHILD_ARGUMENT_CHECK_ARGS)) {
		return(ProcessTestChildMode_CheckArgs);
	}
	if (fplIsStringEqual(modeArgument, FPL_TEST_CHILD_ARGUMENT_CHECK_FILE)) {
		return(ProcessTestChildMode_CheckFile);
	}
	if (fplIsStringEqual(modeArgument, FPL_TEST_CHILD_ARGUMENT_BOTH)) {
		return(ProcessTestChildMode_Both);
	}
	if (fplIsStringEqual(modeArgument, FPL_TEST_CHILD_ARGUMENT_SPAM)) {
		return(ProcessTestChildMode_Spam);
	}
	if (fplIsStringEqual(modeArgument, FPL_TEST_CHILD_ARGUMENT_LINES)) {
		return(ProcessTestChildMode_Lines);
	}
	if (fplIsStringEqual(modeArgument, FPL_TEST_CHILD_ARGUMENT_CAT)) {
		return(ProcessTestChildMode_Cat);
	}
	return(ProcessTestChildMode_None);
}

static int ProcessTestsRunCheckArgs(const int argc, char *args[], const int firstArgumentIndex) {
	if (argc <= firstArgumentIndex) {
		return(processTestChildFailureExitCode);
	}
	int32_t expectedCount = fplStringToS32(args[firstArgumentIndex]);
	if ((expectedCount < 0) || ((size_t)expectedCount > fplArrayCount(processTestArgumentValues))) {
		return(processTestChildFailureExitCode);
	}
	int firstValueIndex = firstArgumentIndex + 1;
	if (argc != (firstValueIndex + expectedCount)) {
		return(processTestChildFailureExitCode);
	}
	for (int32_t valueIndex = 0; valueIndex < expectedCount; ++valueIndex) {
		const char *expectedValue = processTestArgumentValues[valueIndex];
		const char *actualValue = args[firstValueIndex + valueIndex];
		if (!fplIsStringEqual(expectedValue, actualValue)) {
			return(processTestChildFailureExitCode);
		}
	}
	return(processTestChildSuccessExitCode);
}

// Writes the requested number of characters to the standard-output, so the parent can prove
// that a lot more than one pipe buffer can be captured without deadlocking
static void ProcessTestsWriteSpam(const int32_t totalSize) {
	char chunk[1025];
	for (size_t charIndex = 0; charIndex < processTestSpamChunkSize; ++charIndex) {
		chunk[charIndex] = 'x';
	}
	chunk[processTestSpamChunkSize] = 0;
	int32_t remainingSize = totalSize;
	while (remainingSize >= (int32_t)processTestSpamChunkSize) {
		fplConsoleOut(chunk);
		remainingSize -= (int32_t)processTestSpamChunkSize;
	}
	if (remainingSize > 0) {
		chunk[remainingSize] = 0;
		fplConsoleOut(chunk);
	}
}

// Writes complete lines with a "\r\n" ending and one last line without any ending, so the parent can prove
// that the line splitting normalizes the ending and still delivers the incomplete rest at the end
static void ProcessTestsWriteLines(const int32_t lineCount) {
	// The first line is empty and ends with a bare "\n", so nothing is ever appended before it is delivered
	fplConsoleOut("\n");
	char lineText[64];
	for (int32_t lineIndex = 0; lineIndex < lineCount; ++lineIndex) {
		fplStringFormat(lineText, fplArrayCount(lineText), "%s%d\r\n", processTestLineTextPrefix, lineIndex);
		fplConsoleOut(lineText);
	}
	fplConsoleOut(processTestLineTailText);
}

// Reads the standard-input to its end and writes everything back to the standard-output, so the parent
// can verify exactly what arrived. The test input never contains a carriage return, so the text mode
// translation of the Windows runtime cannot change anything.
static void ProcessTestsEchoStandardInput(void) {
	char buffer[PROCESS_TEST_CAT_BUFFER_SIZE];
	for (;;) {
		size_t readBytes = fread(buffer, 1, fplArrayCount(buffer) - 1, stdin);
		if (readBytes == 0) {
			break;
		}
		buffer[readBytes] = 0;
		fplConsoleOut(buffer);
	}
}

// Runs the requested child behavior and returns the exit code the parent will see
static int ProcessTestsRunAsChild(const ProcessTestChildMode mode, const int argc, char *args[]) {
#if defined(FPL_LOGGING)
	// The child must stay silent. Its own log output would go to the very same standard-output
	// the parent captures and would show up as unexpected text there.
	fplLogSettings silentLogSettings = fplZeroInit;
	silentLogSettings.isInitialized = true;
	silentLogSettings.maxLevel = fplLogLevel_Critical;
	for (size_t writerIndex = 0; writerIndex < fplArrayCount(silentLogSettings.writers); ++writerIndex) {
		silentLogSettings.writers[writerIndex].flags = fplLogWriterFlags_None;
	}
	fplSetLogSettings(&silentLogSettings);
#endif
	if (!fplPlatformInit(fplInitFlags_None, fpl_null)) {
		return(processTestChildFailureExitCode);
	}
	const char *firstValue = (argc > 2) ? args[2] : fpl_null;
	int result = processTestChildFailureExitCode;
	switch (mode) {
		case ProcessTestChildMode_Echo:
		{
			if (firstValue != fpl_null) {
				fplConsoleOut(firstValue);
			}
			fplConsoleOut("\n");
			result = processTestChildSuccessExitCode;
		} break;

		case ProcessTestChildMode_Error:
		{
			if (firstValue != fpl_null) {
				fplConsoleError(firstValue);
			}
			fplConsoleError("\n");
			result = processTestChildSuccessExitCode;
		} break;

		case ProcessTestChildMode_Exit:
		{
			if (firstValue != fpl_null) {
				int32_t requestedExitCode = fplStringToS32(firstValue);
				result = (int)requestedExitCode;
			}
		} break;

		case ProcessTestChildMode_Sleep:
		{
			if (firstValue != fpl_null) {
				int32_t sleepTimeInMilliseconds = fplStringToS32(firstValue);
				fplThreadSleep((uint32_t)sleepTimeInMilliseconds);
				result = processTestChildSuccessExitCode;
			}
		} break;

		case ProcessTestChildMode_CheckArgs:
		{
			result = ProcessTestsRunCheckArgs(argc, args, 2);
		} break;

		case ProcessTestChildMode_CheckFile:
		{
			if (firstValue != fpl_null) {
				// The path is relative on purpose, so it only resolves when the work directory was applied
				bool fileExists = fplFileExists(firstValue);
				result = fileExists ? processTestChildSuccessExitCode : processTestChildFailureExitCode;
			}
		} break;

		case ProcessTestChildMode_Both:
		{
			fplConsoleOut(processTestEchoText);
			fplConsoleOut("\n");
			fplConsoleError(processTestErrorText);
			fplConsoleError("\n");
			result = processTestChildSuccessExitCode;
		} break;

		case ProcessTestChildMode_Spam:
		{
			if (firstValue != fpl_null) {
				int32_t totalSize = fplStringToS32(firstValue);
				ProcessTestsWriteSpam(totalSize);
				result = processTestChildSuccessExitCode;
			}
		} break;

		case ProcessTestChildMode_Lines:
		{
			if (firstValue != fpl_null) {
				int32_t lineCount = fplStringToS32(firstValue);
				ProcessTestsWriteLines(lineCount);
				result = processTestChildSuccessExitCode;
			}
		} break;

		case ProcessTestChildMode_Cat:
		{
			ProcessTestsEchoStandardInput();
			result = processTestChildSuccessExitCode;
		} break;

		default:
			break;
	}
	fplPlatformRelease();
	return(result);
}

//
// Test side
//

typedef struct ProcessTestPaths {
	//! Full path of the test executable, used as the program for every child.
	char executableFilePath[FPL_MAX_PATH_LENGTH];
	//! Directory of the test executable, used as the work directory that contains the marker file.
	char executableDirectoryPath[FPL_MAX_PATH_LENGTH];
	//! Full path of the marker file inside the executable directory.
	char markerFilePath[FPL_MAX_PATH_LENGTH];
	//! A directory that never contains the marker file.
	const char *foreignDirectoryPath;
} ProcessTestPaths;

static bool ProcessTestsQueryPaths(ProcessTestPaths *outPaths) {
	fplClearStruct(outPaths);
	size_t executablePathLen = fplGetExecutableFilePath(outPaths->executableFilePath, fplArrayCount(outPaths->executableFilePath));
	if (executablePathLen == 0) {
		return(false);
	}
	size_t directoryPathLen = fplExtractFilePath(outPaths->executableFilePath, outPaths->executableDirectoryPath, fplArrayCount(outPaths->executableDirectoryPath));
	if (directoryPathLen == 0) {
		return(false);
	}
	size_t markerPathLen = fplPathCombine(outPaths->markerFilePath, fplArrayCount(outPaths->markerFilePath), 2, outPaths->executableDirectoryPath, processTestMarkerFileName);
	if (markerPathLen == 0) {
		return(false);
	}
#if defined(FPL_PLATFORM_WINDOWS)
	outPaths->foreignDirectoryPath = "C:\\";
#else
	outPaths->foreignDirectoryPath = "/";
#endif
	return(true);
}

static bool ProcessTestsCreateMarkerFile(const char *markerFilePath) {
	fplFileHandle markerFile = fplZeroInit;
	if (!fplFileCreateBinary(markerFilePath, &markerFile)) {
		return(false);
	}
	const char markerContent[] = "fpl";
	fplFileWriteBlock32(&markerFile, markerContent, (uint32_t)sizeof(markerContent));
	fplFileClose(&markerFile);
	return(true);
}

// Starts the test executable in one of the child modes and waits for it to be finished
static bool ProcessTestsRunChild(const ProcessTestPaths *paths, const char *argumentLine, const char **arguments, const size_t argumentCount, const char *workDir, const fplProcessFlags flags, fplProcessResult *outResult) {
	fplProcessContext context = fplZeroInit;
	context.name = paths->executableFilePath;
	context.argumentLine = argumentLine;
	context.arguments = arguments;
	context.argumentCount = argumentCount;
	context.workDir = workDir;
	context.flags = flags | fplProcessFlags_AutoWait;
	fplProcessHandle handle = fplZeroInit;
	bool started = fplProcessStart(&context, &handle, outResult);
	fplProcessClose(&handle);
	return(started);
}

static void ProcessTestsInvalidArguments(const ProcessTestPaths *paths) {
	ftMsg("Test Process invalid arguments (the logged errors are expected)\n");
	{
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;

		// A missing context, handle or program name must be rejected without touching anything
		fplProcessContext validContext = fplZeroInit;
		validContext.name = paths->executableFilePath;
		ftIsFalse(fplProcessStart(fpl_null, &handle, &result));
		ftIsFalse(fplProcessStart(&validContext, fpl_null, &result));

		fplProcessContext contextWithoutName = fplZeroInit;
		ftIsFalse(fplProcessStart(&contextWithoutName, &handle, &result));
		ftIsFalse(handle.isValid);

		// Every other entry point must survive a null handle
		ftIsFalse(fplProcessUpdate(fpl_null));
		ftIsFalse(fplProcessWait(fpl_null, FPL_TIMEOUT_INFINITE, &result));
		ftIsFalse(fplProcessIsRunning(fpl_null));
		int32_t exitCode = 0;
		ftIsFalse(fplProcessTryGetExitCode(fpl_null, &exitCode));
		ftIsFalse(fplProcessRequestStop(fpl_null));
		ftIsFalse(fplProcessStop(fpl_null));
		fplProcessClose(fpl_null);
		fplProcessFreeResult(fpl_null);

		// An invalid handle must be rejected as well
		fplProcessHandle invalidHandle = fplZeroInit;
		ftIsFalse(fplProcessWait(&invalidHandle, FPL_TIMEOUT_INFINITE, &result));
		ftIsFalse(fplProcessIsRunning(&invalidHandle));
		ftIsFalse(fplProcessTryGetExitCode(&invalidHandle, &exitCode));
		ftIsFalse(fplProcessStop(&invalidHandle));

		// A missing exit code target must be rejected
		fplProcessResult startResult = fplZeroInit;
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.argumentLine = FPL_TEST_CHILD_ARGUMENT_SLEEP " 1";
		context.flags = fplProcessFlags_AutoWait;
		fplProcessHandle startedHandle = fplZeroInit;
		if (fplProcessStart(&context, &startedHandle, &startResult)) {
			ftIsFalse(fplProcessTryGetExitCode(&startedHandle, fpl_null));
		}
		fplProcessClose(&startedHandle);
	}
}

static void ProcessTestsStartFailures(void) {
	ftMsg("Test Process start failures (the logged errors are expected)\n");
	{
		// A program that does not exist must be reported as not found
		fplProcessContext context = fplZeroInit;
		context.name = "fpl_test_this_program_does_not_exist_xyz";
		context.flags = fplProcessFlags_AutoWait;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		bool started = fplProcessStart(&context, &handle, &result);
		ftIsFalse(started);
		ftIsFalse(handle.isValid);
		ftIsFalse(result.hasExited);
		ftAssert(result.type == fplProcessResultType_NotFound);
		ftAssert(result.nativeErrorCode != 0);
		fplProcessFreeResult(&result);
		fplProcessClose(&handle);
	}
	{
		// A file that exists but cannot be executed must not be reported as not found
#if defined(FPL_PLATFORM_WINDOWS)
		const char *notExecutablePath = "C:\\Windows";
#else
		const char *notExecutablePath = "/etc/hostname";
#endif
		fplProcessContext context = fplZeroInit;
		context.name = notExecutablePath;
		context.flags = fplProcessFlags_AutoWait;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		bool started = fplProcessStart(&context, &handle, &result);
		ftIsFalse(started);
		ftAssert((result.type == fplProcessResultType_AccessDenied) || (result.type == fplProcessResultType_FailedToStart));
		fplProcessFreeResult(&result);
		fplProcessClose(&handle);
	}
}

static void ProcessTestsWorkDirFailure(const ProcessTestPaths *paths) {
	ftMsg("Test Process work directory failure (the logged errors are expected)\n");
	{
		// A work directory that does not exist must fail the start, even though the program itself is fine
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.argumentLine = FPL_TEST_CHILD_ARGUMENT_ECHO " never";
		context.workDir = "fpl_test_this_directory_does_not_exist_xyz";
		context.flags = fplProcessFlags_AutoWait;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		bool started = fplProcessStart(&context, &handle, &result);
		ftIsFalse(started);
		ftIsFalse(handle.isValid);
		ftAssert(result.type != fplProcessResultType_Success);
		ftAssert(result.nativeErrorCode != 0);
		fplProcessFreeResult(&result);
		fplProcessClose(&handle);
	}
}

static void ProcessTestsExitCodes(const ProcessTestPaths *paths) {
	ftMsg("Test Process exit codes\n");
	{
		// A child that finishes normally reports success with a zero exit code
		fplProcessResult result = fplZeroInit;
		bool started = ProcessTestsRunChild(paths, processTestSilentChildArgumentLine, fpl_null, 0, fpl_null, fplProcessFlags_None, &result);
		ftIsTrue(started);
		ftIsTrue(result.hasExited);
		ftAssert(result.type == fplProcessResultType_Success);
		ftAssertS32Equals(0, result.exitCode);
		fplProcessFreeResult(&result);
	}
	{
		// A non-zero exit code is a normal result and not an error
		char argumentLine[FPL_MAX_BUFFER_LENGTH];
		fplStringFormat(argumentLine, fplArrayCount(argumentLine), "%s %d", FPL_TEST_CHILD_ARGUMENT_EXIT, processTestCustomExitCode);
		fplProcessResult result = fplZeroInit;
		bool started = ProcessTestsRunChild(paths, argumentLine, fpl_null, 0, fpl_null, fplProcessFlags_None, &result);
		ftIsTrue(started);
		ftAssert(result.type == fplProcessResultType_Success);
		ftAssertS32Equals(processTestCustomExitCode, result.exitCode);
		fplProcessFreeResult(&result);
	}
	{
		// Only when the caller asks for it, a non-zero exit code becomes a failure
		char argumentLine[FPL_MAX_BUFFER_LENGTH];
		fplStringFormat(argumentLine, fplArrayCount(argumentLine), "%s %d", FPL_TEST_CHILD_ARGUMENT_EXIT, processTestCustomExitCode);
		fplProcessResult result = fplZeroInit;
		bool started = ProcessTestsRunChild(paths, argumentLine, fpl_null, 0, fpl_null, fplProcessFlags_TreatNonZeroExitAsError, &result);
		ftIsTrue(started);
		ftAssert(result.type == fplProcessResultType_FailedWithExitCode);
		ftAssertS32Equals(processTestCustomExitCode, result.exitCode);
		fplProcessFreeResult(&result);
	}
}

static void ProcessTestsArguments(const ProcessTestPaths *paths) {
	ftMsg("Test Process arguments\n");
	{
		// The argument array must arrive unchanged, even with spaces and quotes
		const char *arguments[] = {
			FPL_TEST_CHILD_ARGUMENT_CHECK_ARGS,
			"3",
			processTestArgumentValues[0],
			processTestArgumentValues[1],
			processTestArgumentValues[2],
		};
		fplProcessResult result = fplZeroInit;
		bool started = ProcessTestsRunChild(paths, fpl_null, arguments, fplArrayCount(arguments), fpl_null, fplProcessFlags_None, &result);
		ftIsTrue(started);
		ftAssertS32Equals(processTestChildSuccessExitCode, result.exitCode);
		fplProcessFreeResult(&result);
	}
	{
		// A quoted argument line must be split into the very same arguments
		char argumentLine[FPL_MAX_BUFFER_LENGTH];
		fplStringFormat(argumentLine, fplArrayCount(argumentLine), "%s 2 %s \"%s\"", FPL_TEST_CHILD_ARGUMENT_CHECK_ARGS, processTestArgumentValues[0], processTestArgumentValues[1]);
		fplProcessResult result = fplZeroInit;
		bool started = ProcessTestsRunChild(paths, argumentLine, fpl_null, 0, fpl_null, fplProcessFlags_None, &result);
		ftIsTrue(started);
		ftAssertS32Equals(processTestChildSuccessExitCode, result.exitCode);
		fplProcessFreeResult(&result);
	}
	{
		// A wrong argument count must be detected by the child, so the check itself is proven to work
		const char *arguments[] = {
			FPL_TEST_CHILD_ARGUMENT_CHECK_ARGS,
			"3",
			processTestArgumentValues[0],
		};
		fplProcessResult result = fplZeroInit;
		bool started = ProcessTestsRunChild(paths, fpl_null, arguments, fplArrayCount(arguments), fpl_null, fplProcessFlags_None, &result);
		ftIsTrue(started);
		ftAssertS32Equals(processTestChildFailureExitCode, result.exitCode);
		fplProcessFreeResult(&result);
	}
}

static void ProcessTestsWorkDir(const ProcessTestPaths *paths) {
	ftMsg("Test Process work directory\n");
	if (!ProcessTestsCreateMarkerFile(paths->markerFilePath)) {
		ftFail("Failed creating the marker file '%s'", paths->markerFilePath);
		return;
	}
	char argumentLine[FPL_MAX_BUFFER_LENGTH];
	fplStringFormat(argumentLine, fplArrayCount(argumentLine), "%s %s", FPL_TEST_CHILD_ARGUMENT_CHECK_FILE, processTestMarkerFileName);
	{
		// The child resolves the relative marker path, so it only finds it when the work directory was applied
		fplProcessResult result = fplZeroInit;
		bool started = ProcessTestsRunChild(paths, argumentLine, fpl_null, 0, paths->executableDirectoryPath, fplProcessFlags_None, &result);
		ftIsTrue(started);
		ftAssertS32Equals(processTestChildSuccessExitCode, result.exitCode);
		fplProcessFreeResult(&result);
	}
	{
		// The same child must not find the marker in a directory that does not contain it
		fplProcessResult result = fplZeroInit;
		bool started = ProcessTestsRunChild(paths, argumentLine, fpl_null, 0, paths->foreignDirectoryPath, fplProcessFlags_None, &result);
		ftIsTrue(started);
		ftAssertS32Equals(processTestChildFailureExitCode, result.exitCode);
		fplProcessFreeResult(&result);
	}
	fplFileDelete(paths->markerFilePath);
}

static void ProcessTestsAsyncAndStop(const ProcessTestPaths *paths) {
	ftMsg("Test Process async start, polling and stopping\n");
	char sleepArgumentLine[FPL_MAX_BUFFER_LENGTH];
	fplStringFormat(sleepArgumentLine, fplArrayCount(sleepArgumentLine), "%s %d", FPL_TEST_CHILD_ARGUMENT_SLEEP, processTestLongSleepInMilliseconds);
	{
		// A running child must be reported as running and must not deliver an exit code yet
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.argumentLine = sleepArgumentLine;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult startResult = fplZeroInit;
		bool started = fplProcessStart(&context, &handle, &startResult);
		ftIsTrue(started);
		ftIsTrue(handle.isValid);
		ftAssert(handle.id != 0);
		ftIsTrue(fplProcessIsRunning(&handle));
		int32_t exitCode = 0;
		ftIsFalse(fplProcessTryGetExitCode(&handle, &exitCode));

		// Waiting with a timeout must give up instead of blocking forever
		fplProcessResult timeoutResult = fplZeroInit;
		bool exitedInTime = fplProcessWait(&handle, processTestShortTimeout, &timeoutResult);
		ftIsFalse(exitedInTime);
		ftAssert(timeoutResult.type == fplProcessResultType_Timeout);
		ftIsFalse(timeoutResult.hasExited);
		fplProcessFreeResult(&timeoutResult);

		// Stopping must end the child and the state must be reported afterwards
		ftIsTrue(fplProcessStop(&handle));
		fplProcessResult stopResult = fplZeroInit;
		ftIsTrue(fplProcessWait(&handle, FPL_TIMEOUT_INFINITE, &stopResult));
		ftIsTrue(stopResult.hasExited);
		ftIsFalse(fplProcessIsRunning(&handle));
		ftIsTrue(fplProcessTryGetExitCode(&handle, &exitCode));
#if !defined(FPL_PLATFORM_WINDOWS)
		ftAssert(stopResult.type == fplProcessResultType_Terminated);
		ftAssertS32Equals(SIGKILL, stopResult.terminationSignal);
#endif
		// Stopping an already exited process must be rejected
		ftIsFalse(fplProcessStop(&handle));
		fplProcessFreeResult(&stopResult);
		fplProcessClose(&handle);
	}
#if !defined(FPL_PLATFORM_WINDOWS)
	{
		// A graceful stop must end the child through SIGTERM
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.argumentLine = sleepArgumentLine;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult startResult = fplZeroInit;
		ftIsTrue(fplProcessStart(&context, &handle, &startResult));
		ftIsTrue(fplProcessRequestStop(&handle));
		fplProcessResult stopResult = fplZeroInit;
		ftIsTrue(fplProcessWait(&handle, FPL_TIMEOUT_INFINITE, &stopResult));
		ftAssert(stopResult.type == fplProcessResultType_Terminated);
		ftAssertS32Equals(SIGTERM, stopResult.terminationSignal);
		fplProcessFreeResult(&stopResult);
		fplProcessClose(&handle);
	}
#endif
}

static void ProcessTestsHandleLifetime(const ProcessTestPaths *paths) {
	ftMsg("Test Process handle and result lifetime\n");
	{
		// Closing and freeing twice must be harmless
		fplProcessResult result = fplZeroInit;
		bool started = ProcessTestsRunChild(paths, processTestSilentChildArgumentLine, fpl_null, 0, fpl_null, fplProcessFlags_None, &result);
		ftIsTrue(started);
		fplProcessFreeResult(&result);
		fplProcessFreeResult(&result);
		ftIsNull(result.output.text);
		ftIsNull(result.error.text);

		fplProcessHandle handle = fplZeroInit;
		fplProcessClose(&handle);
		fplProcessClose(&handle);
		ftIsFalse(handle.isValid);
	}
	{
		// Many cycles must not leak handles or leave processes behind
		bool allCyclesSucceeded = true;
		for (int cycleIndex = 0; cycleIndex < processTestCycleCount; ++cycleIndex) {
			fplProcessResult result = fplZeroInit;
			bool started = ProcessTestsRunChild(paths, processTestSilentChildArgumentLine, fpl_null, 0, fpl_null, fplProcessFlags_None, &result);
			if (!started || (result.exitCode != processTestChildSuccessExitCode)) {
				allCyclesSucceeded = false;
			}
			fplProcessFreeResult(&result);
		}
		ftIsTrue(allCyclesSucceeded);
	}
	{
		// Closing a still running process must not stop it, but it must not leave a zombie behind either
		char sleepArgumentLine[FPL_MAX_BUFFER_LENGTH];
		fplStringFormat(sleepArgumentLine, fplArrayCount(sleepArgumentLine), "%s %d", FPL_TEST_CHILD_ARGUMENT_SLEEP, (int)processTestShortTimeout);
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.argumentLine = sleepArgumentLine;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult startResult = fplZeroInit;
		ftIsTrue(fplProcessStart(&context, &handle, &startResult));
		fplProcessClose(&handle);
		ftIsFalse(handle.isValid);
		ftIsFalse(fplProcessIsRunning(&handle));
	}
}

static void ProcessTestsCurrentProcess(void) {
	ftMsg("Test Process current id\n");
	{
		uint64_t currentProcessId = fplProcessGetCurrentId();
		ftAssert(currentProcessId != 0);
		uint64_t secondQueryProcessId = fplProcessGetCurrentId();
		ftAssertU64Equals(currentProcessId, secondQueryProcessId);
	}
}

static void ProcessTestsCapture(const ProcessTestPaths *paths) {
	ftMsg("Test Process capture\n");
	char argumentLine[FPL_MAX_BUFFER_LENGTH];
	{
		// The standard-output must arrive in the output buffer, null-terminated and with the exact length
		fplStringFormat(argumentLine, fplArrayCount(argumentLine), "%s %s", FPL_TEST_CHILD_ARGUMENT_ECHO, processTestEchoText);
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.argumentLine = argumentLine;
		context.captureFlags = fplProcessCaptureFlags_CaptureOutput;
		context.flags = fplProcessFlags_AutoWait;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		ftIsTrue(fplProcessStart(&context, &handle, &result));
		ftIsNotNull(result.output.text);
		ftIsNull(result.error.text);
		ftIsFalse(result.isTruncated);
		size_t expectedLen = fplGetStringLength(processTestEchoText) + 1;
		ftAssertSizeEquals(expectedLen, result.output.len);
		ftAssertCharEquals(0, result.output.text[result.output.len]);
		fplProcessFreeResult(&result);
		ftIsNull(result.output.text);
		fplProcessClose(&handle);
	}
	{
		// Capturing only the error stream must leave the output buffer empty
		fplStringFormat(argumentLine, fplArrayCount(argumentLine), "%s %s", FPL_TEST_CHILD_ARGUMENT_ERROR, processTestErrorText);
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.argumentLine = argumentLine;
		context.captureFlags = fplProcessCaptureFlags_CaptureError;
		context.flags = fplProcessFlags_AutoWait;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		ftIsTrue(fplProcessStart(&context, &handle, &result));
		ftIsNotNull(result.error.text);
		ftIsNull(result.output.text);
		size_t expectedLen = fplGetStringLength(processTestErrorText) + 1;
		ftAssertSizeEquals(expectedLen, result.error.len);
		fplProcessFreeResult(&result);
		fplProcessClose(&handle);
	}
	{
		// A merged capture puts both streams into the output buffer, the same way "2>&1" does
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.argumentLine = FPL_TEST_CHILD_ARGUMENT_BOTH;
		context.captureFlags = fplProcessCaptureFlags_CaptureBoth;
		context.flags = fplProcessFlags_AutoWait;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		ftIsTrue(fplProcessStart(&context, &handle, &result));
		ftIsNotNull(result.output.text);
		ftIsNull(result.error.text);
		size_t expectedLen = fplGetStringLength(processTestEchoText) + fplGetStringLength(processTestErrorText) + 2;
		ftAssertSizeEquals(expectedLen, result.output.len);
		fplProcessFreeResult(&result);
		fplProcessClose(&handle);
	}
	{
		// This is the deadlock regression test: the child writes far more than one pipe buffer holds.
		// An implementation that waits first and reads afterwards hangs here forever.
		fplStringFormat(argumentLine, fplArrayCount(argumentLine), "%s %d", FPL_TEST_CHILD_ARGUMENT_SPAM, processTestSpamTotalSize);
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.argumentLine = argumentLine;
		context.captureFlags = fplProcessCaptureFlags_CaptureBoth;
		context.flags = fplProcessFlags_AutoWait;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		ftIsTrue(fplProcessStart(&context, &handle, &result));
		ftIsTrue(result.hasExited);
		ftIsFalse(result.isTruncated);
		ftAssertSizeEquals((size_t)processTestSpamTotalSize, result.output.len);
		ftAssertS32Equals(processTestChildSuccessExitCode, result.exitCode);
		fplProcessFreeResult(&result);
		fplProcessClose(&handle);
	}
	{
		// The capture limit must cut the text off, but the child still has to run to its end
		fplStringFormat(argumentLine, fplArrayCount(argumentLine), "%s %d", FPL_TEST_CHILD_ARGUMENT_SPAM, processTestSpamTotalSize);
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.argumentLine = argumentLine;
		context.captureFlags = fplProcessCaptureFlags_CaptureBoth;
		context.maxCaptureSize = processTestCaptureLimit;
		context.flags = fplProcessFlags_AutoWait;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		ftIsTrue(fplProcessStart(&context, &handle, &result));
		ftIsTrue(result.isTruncated);
		ftIsTrue(result.hasExited);
		ftAssertSizeEquals(processTestCaptureLimit, result.output.len);
		ftAssertS32Equals(processTestChildSuccessExitCode, result.exitCode);
		fplProcessFreeResult(&result);
		fplProcessClose(&handle);
	}
	{
		// Pumping by hand must collect the very same text as waiting does
		fplStringFormat(argumentLine, fplArrayCount(argumentLine), "%s %d", FPL_TEST_CHILD_ARGUMENT_SPAM, processTestSpamTotalSize);
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.argumentLine = argumentLine;
		context.captureFlags = fplProcessCaptureFlags_CaptureOutput;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult startResult = fplZeroInit;
		ftIsTrue(fplProcessStart(&context, &handle, &startResult));
		while (fplProcessUpdate(&handle)) {
		}
		fplProcessResult waitResult = fplZeroInit;
		ftIsTrue(fplProcessWait(&handle, FPL_TIMEOUT_INFINITE, &waitResult));
		ftAssertSizeEquals((size_t)processTestSpamTotalSize, waitResult.output.len);
		fplProcessFreeResult(&waitResult);
		fplProcessClose(&handle);
	}
	{
		// Closing without ever asking for a result must not leak the captured buffer
		fplStringFormat(argumentLine, fplArrayCount(argumentLine), "%s %s", FPL_TEST_CHILD_ARGUMENT_ECHO, processTestEchoText);
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.argumentLine = argumentLine;
		context.captureFlags = fplProcessCaptureFlags_CaptureOutput;
		context.flags = fplProcessFlags_AutoWait;
		fplProcessHandle handle = fplZeroInit;
		ftIsTrue(fplProcessStart(&context, &handle, fpl_null));
		fplProcessClose(&handle);
	}
}

// Number of characters the collector keeps per stream
#define PROCESS_TEST_MAX_COLLECTED_TEXT 8192
// Number of callback calls the collector remembers
#define PROCESS_TEST_MAX_COLLECTED_CALLS 64
// Number of characters the collector keeps per callback call
#define PROCESS_TEST_MAX_COLLECTED_CALL_LENGTH 128

// Collects everything the output callback delivers, so the tests can check the text, the stream and the call boundaries
typedef struct ProcessTestOutputCollector {
	char outputText[PROCESS_TEST_MAX_COLLECTED_TEXT];
	char errorText[PROCESS_TEST_MAX_COLLECTED_TEXT];
	char callTexts[PROCESS_TEST_MAX_COLLECTED_CALLS][PROCESS_TEST_MAX_COLLECTED_CALL_LENGTH];
	size_t callTextLengths[PROCESS_TEST_MAX_COLLECTED_CALLS];
	fplProcessStreamType callStreams[PROCESS_TEST_MAX_COLLECTED_CALLS];
	size_t outputLen;
	size_t errorLen;
	size_t callCount;
	uint64_t processId;
	bool hasBrokenNullTerminator;
	bool hasMissingProcess;
} ProcessTestOutputCollector;

static void ProcessTestsAppendCollectedText(char *targetText, size_t *targetLen, const size_t maxTargetLen, const char *text, const size_t textLen) {
	size_t freeLen = maxTargetLen - *targetLen - 1;
	size_t appendLen = (textLen < freeLen) ? textLen : freeLen;
	if (appendLen == 0) {
		return;
	}
	char *appendTarget = targetText + *targetLen;
	fplMemoryCopy(text, appendLen, appendTarget);
	*targetLen += appendLen;
	targetText[*targetLen] = 0;
}

static FPL_FUNC_PROCESS_OUTPUT(ProcessTestsCollectOutput) {
	ProcessTestOutputCollector *collector = (ProcessTestOutputCollector *)userData;
	if (collector == fpl_null) {
		return;
	}
	if (process == fpl_null) {
		collector->hasMissingProcess = true;
	} else {
		collector->processId = process->id;
	}
	// The text is documented to be a usable string, so the null-terminator has to be there
	if (text[textLen] != 0) {
		collector->hasBrokenNullTerminator = true;
	}
	if (collector->callCount < fplArrayCount(collector->callTexts)) {
		size_t callIndex = collector->callCount;
		char *callText = collector->callTexts[callIndex];
		// A raw chunk is far longer than one slot, so only its beginning is kept. The full length is
		// remembered separately, and every assert on the text itself uses short lines anyway.
		size_t maxCallTextLen = fplArrayCount(collector->callTexts[callIndex]) - 1;
		size_t keptTextLen = (textLen < maxCallTextLen) ? textLen : maxCallTextLen;
		if (keptTextLen > 0) {
			fplMemoryCopy(text, keptTextLen, callText);
		}
		callText[keptTextLen] = 0;
		collector->callTextLengths[callIndex] = textLen;
		collector->callStreams[callIndex] = stream;
	}
	++collector->callCount;
	if (stream == fplProcessStreamType_Error) {
		ProcessTestsAppendCollectedText(collector->errorText, &collector->errorLen, fplArrayCount(collector->errorText), text, textLen);
	} else {
		ProcessTestsAppendCollectedText(collector->outputText, &collector->outputLen, fplArrayCount(collector->outputText), text, textLen);
	}
}

static void ProcessTestsCaptureSeparate(const ProcessTestPaths *paths) {
	ftMsg("Test Process separate capture\n");
	{
		// Without the merge flag both streams get their own pipe and their own buffer
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.argumentLine = FPL_TEST_CHILD_ARGUMENT_BOTH;
		context.captureFlags = fplProcessCaptureFlags_CaptureSeparate;
		context.flags = fplProcessFlags_AutoWait;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		ftIsTrue(fplProcessStart(&context, &handle, &result));
		ftIsNotNull(result.output.text);
		ftIsNotNull(result.error.text);
		size_t expectedOutputLen = fplGetStringLength(processTestEchoText) + 1;
		size_t expectedErrorLen = fplGetStringLength(processTestErrorText) + 1;
		ftAssertSizeEquals(expectedOutputLen, result.output.len);
		ftAssertSizeEquals(expectedErrorLen, result.error.len);
		ftAssertCharEquals(0, result.output.text[result.output.len]);
		ftAssertCharEquals(0, result.error.text[result.error.len]);
		fplProcessFreeResult(&result);
		fplProcessClose(&handle);
	}
	{
		// A separate capture must survive far more than one pipe buffer as well
		char argumentLine[FPL_MAX_BUFFER_LENGTH];
		fplStringFormat(argumentLine, fplArrayCount(argumentLine), "%s %d", FPL_TEST_CHILD_ARGUMENT_SPAM, processTestSpamTotalSize);
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.argumentLine = argumentLine;
		context.captureFlags = fplProcessCaptureFlags_CaptureSeparate;
		context.flags = fplProcessFlags_AutoWait;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		ftIsTrue(fplProcessStart(&context, &handle, &result));
		ftIsTrue(result.hasExited);
		ftAssertSizeEquals((size_t)processTestSpamTotalSize, result.output.len);
		ftAssertSizeEquals(0, result.error.len);
		fplProcessFreeResult(&result);
		fplProcessClose(&handle);
	}
}

static void ProcessTestsCallbacks(const ProcessTestPaths *paths) {
	ftMsg("Test Process output callback\n");
	char argumentLine[FPL_MAX_BUFFER_LENGTH];
	{
		// A pure redirect fills the callback and leaves the result buffers empty
		fplStringFormat(argumentLine, fplArrayCount(argumentLine), "%s %s", FPL_TEST_CHILD_ARGUMENT_ECHO, processTestEchoText);
		ProcessTestOutputCollector collector = fplZeroInit;
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.argumentLine = argumentLine;
		context.captureFlags = fplProcessCaptureFlags_RedirectOutput;
		context.outputCallback = ProcessTestsCollectOutput;
		context.userData = &collector;
		context.flags = fplProcessFlags_AutoWait;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		ftIsTrue(fplProcessStart(&context, &handle, &result));
		ftIsNull(result.output.text);
		ftIsNull(result.error.text);
		ftIsFalse(collector.hasMissingProcess);
		ftIsFalse(collector.hasBrokenNullTerminator);
		ftIsTrue(collector.callCount > 0);
		ftAssert(collector.processId == handle.id);
		char expectedText[FPL_MAX_BUFFER_LENGTH];
		fplStringFormat(expectedText, fplArrayCount(expectedText), "%s\n", processTestEchoText);
		ftAssertStringEquals(expectedText, collector.outputText);
		ftAssertSizeEquals(0, collector.errorLen);
		fplProcessFreeResult(&result);
		fplProcessClose(&handle);
	}
	{
		// Buffer and callback at the same time must both get the very same text
		fplStringFormat(argumentLine, fplArrayCount(argumentLine), "%s %s", FPL_TEST_CHILD_ARGUMENT_ECHO, processTestEchoText);
		ProcessTestOutputCollector collector = fplZeroInit;
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.argumentLine = argumentLine;
		context.captureFlags = fplProcessCaptureFlags_Output | fplProcessCaptureFlags_ToBuffer | fplProcessCaptureFlags_ToCallback;
		context.outputCallback = ProcessTestsCollectOutput;
		context.userData = &collector;
		context.flags = fplProcessFlags_AutoWait;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		ftIsTrue(fplProcessStart(&context, &handle, &result));
		ftIsNotNull(result.output.text);
		ftAssertSizeEquals(result.output.len, collector.outputLen);
		ftAssertStringEquals(result.output.text, collector.outputText);
		fplProcessFreeResult(&result);
		fplProcessClose(&handle);
	}
	{
		// Redirecting both streams without merging them must report the right stream for every call
		ProcessTestOutputCollector collector = fplZeroInit;
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.argumentLine = FPL_TEST_CHILD_ARGUMENT_BOTH;
		context.captureFlags = fplProcessCaptureFlags_Both | fplProcessCaptureFlags_ToCallback;
		context.outputCallback = ProcessTestsCollectOutput;
		context.userData = &collector;
		context.flags = fplProcessFlags_AutoWait;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		ftIsTrue(fplProcessStart(&context, &handle, &result));
		char expectedOutputText[FPL_MAX_BUFFER_LENGTH];
		char expectedErrorText[FPL_MAX_BUFFER_LENGTH];
		fplStringFormat(expectedOutputText, fplArrayCount(expectedOutputText), "%s\n", processTestEchoText);
		fplStringFormat(expectedErrorText, fplArrayCount(expectedErrorText), "%s\n", processTestErrorText);
		ftAssertStringEquals(expectedOutputText, collector.outputText);
		ftAssertStringEquals(expectedErrorText, collector.errorText);
		fplProcessFreeResult(&result);
		fplProcessClose(&handle);
	}
	{
		// The line buffer delivers one call per line, without the line ending and with the incomplete rest at the end
		fplStringFormat(argumentLine, fplArrayCount(argumentLine), "%s %d", FPL_TEST_CHILD_ARGUMENT_LINES, processTestLineCount);
		ProcessTestOutputCollector collector = fplZeroInit;
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.argumentLine = argumentLine;
		context.captureFlags = fplProcessCaptureFlags_RedirectOutput;
		context.outputCallback = ProcessTestsCollectOutput;
		context.userData = &collector;
		context.flags = fplProcessFlags_AutoWait | fplProcessFlags_LineBuffered;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		ftIsTrue(fplProcessStart(&context, &handle, &result));
		ftIsFalse(collector.hasBrokenNullTerminator);
		// One call for the empty first line, one per complete line and one for the incomplete rest
		size_t expectedCallCount = (size_t)processTestLineCount + 2;
		ftAssertSizeEquals(expectedCallCount, collector.callCount);
		ftAssertStringEquals("", collector.callTexts[0]);
		ftAssertSizeEquals(0, collector.callTextLengths[0]);
		char expectedLineText[64];
		for (int32_t lineIndex = 0; lineIndex < processTestLineCount; ++lineIndex) {
			size_t callIndex = (size_t)lineIndex + 1;
			fplStringFormat(expectedLineText, fplArrayCount(expectedLineText), "%s%d", processTestLineTextPrefix, lineIndex);
			ftAssertStringEquals(expectedLineText, collector.callTexts[callIndex]);
			size_t expectedLineLen = fplGetStringLength(expectedLineText);
			ftAssertSizeEquals(expectedLineLen, collector.callTextLengths[callIndex]);
			ftAssert(collector.callStreams[callIndex] == fplProcessStreamType_Output);
		}
		ftAssertStringEquals(processTestLineTailText, collector.callTexts[processTestLineCount + 1]);
		fplProcessFreeResult(&result);
		fplProcessClose(&handle);
	}
	{
		// Without the line flag the callback gets the raw chunks, so the line endings stay untouched
		fplStringFormat(argumentLine, fplArrayCount(argumentLine), "%s %d", FPL_TEST_CHILD_ARGUMENT_LINES, processTestLineCount);
		ProcessTestOutputCollector collector = fplZeroInit;
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.argumentLine = argumentLine;
		context.captureFlags = fplProcessCaptureFlags_RedirectOutput;
		context.outputCallback = ProcessTestsCollectOutput;
		context.userData = &collector;
		context.flags = fplProcessFlags_AutoWait;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		ftIsTrue(fplProcessStart(&context, &handle, &result));
		char expectedText[FPL_MAX_BUFFER_LENGTH];
		size_t expectedTextLen = 0;
		ProcessTestsAppendCollectedText(expectedText, &expectedTextLen, fplArrayCount(expectedText), "\n", 1);
		for (int32_t lineIndex = 0; lineIndex < processTestLineCount; ++lineIndex) {
			char lineText[64];
			fplStringFormat(lineText, fplArrayCount(lineText), "%s%d\r\n", processTestLineTextPrefix, lineIndex);
			size_t lineTextLen = fplGetStringLength(lineText);
			ProcessTestsAppendCollectedText(expectedText, &expectedTextLen, fplArrayCount(expectedText), lineText, lineTextLen);
		}
		size_t tailTextLen = fplGetStringLength(processTestLineTailText);
		ProcessTestsAppendCollectedText(expectedText, &expectedTextLen, fplArrayCount(expectedText), processTestLineTailText, tailTextLen);
		ftAssertStringEquals(expectedText, collector.outputText);
		fplProcessFreeResult(&result);
		fplProcessClose(&handle);
	}
	{
		// A spam child through the callback proves that the redirect does not deadlock either
		fplStringFormat(argumentLine, fplArrayCount(argumentLine), "%s %d", FPL_TEST_CHILD_ARGUMENT_SPAM, processTestSpamTotalSize);
		ProcessTestOutputCollector collector = fplZeroInit;
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.argumentLine = argumentLine;
		context.captureFlags = fplProcessCaptureFlags_RedirectOutput;
		context.outputCallback = ProcessTestsCollectOutput;
		context.userData = &collector;
		context.flags = fplProcessFlags_AutoWait;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		ftIsTrue(fplProcessStart(&context, &handle, &result));
		ftIsTrue(result.hasExited);
		ftIsTrue(collector.callCount > 1);
		ftAssertS32Equals(processTestChildSuccessExitCode, result.exitCode);
		fplProcessFreeResult(&result);
		fplProcessClose(&handle);
	}
}

// Provides a fixed number of chunks and then ends the standard-input by returning zero
typedef struct ProcessTestInputProvider {
	char chunkText[64];
	int32_t remainingChunkCount;
} ProcessTestInputProvider;

static FPL_FUNC_PROCESS_INPUT(ProcessTestsProvideInput) {
	(void)process;
	ProcessTestInputProvider *provider = (ProcessTestInputProvider *)userData;
	if ((provider == fpl_null) || (provider->remainingChunkCount <= 0)) {
		return(0);
	}
	int32_t chunkIndex = processTestInputChunkCount - provider->remainingChunkCount;
	--provider->remainingChunkCount;
	size_t chunkLen = fplStringFormat(provider->chunkText, fplArrayCount(provider->chunkText), "chunk-%d\n", chunkIndex);
	if (chunkLen > maxTargetBufferLen) {
		chunkLen = maxTargetBufferLen;
	}
	fplMemoryCopy(provider->chunkText, chunkLen, targetBuffer);
	return(chunkLen);
}

// Starts the cat child and returns what it wrote back, so every input mode can be checked the same way
static bool ProcessTestsRunCatChild(const ProcessTestPaths *paths, const fplProcessInputMode inputMode, const char *inputText, const size_t inputTextLen, fpl_process_input_callback *inputCallback, void *userData, fplProcessResult *outResult) {
	fplProcessContext context = fplZeroInit;
	context.name = paths->executableFilePath;
	context.argumentLine = FPL_TEST_CHILD_ARGUMENT_CAT;
	context.captureFlags = fplProcessCaptureFlags_CaptureOutput;
	context.inputMode = inputMode;
	context.inputText = inputText;
	context.inputTextLen = inputTextLen;
	context.inputCallback = inputCallback;
	context.userData = userData;
	context.flags = fplProcessFlags_AutoWait;
	fplProcessHandle handle = fplZeroInit;
	bool started = fplProcessStart(&context, &handle, outResult);
	fplProcessClose(&handle);
	return(started);
}

static void ProcessTestsInput(const ProcessTestPaths *paths) {
	ftMsg("Test Process standard-input\n");
	{
		// The text mode writes the whole text and closes the standard-input, so the child sees an end-of-file
		fplProcessResult result = fplZeroInit;
		ftIsTrue(ProcessTestsRunCatChild(paths, fplProcessInputMode_Text, processTestInputText, 0, fpl_null, fpl_null, &result));
		ftIsTrue(result.hasExited);
		ftAssertS32Equals(processTestChildSuccessExitCode, result.exitCode);
		ftAssertStringEquals(processTestInputText, result.output.text);
		fplProcessFreeResult(&result);
	}
	{
		// An explicit length must be used instead of the string length, so only a part arrives
		size_t partialLen = fplGetStringLength("first input line\n");
		fplProcessResult result = fplZeroInit;
		ftIsTrue(ProcessTestsRunCatChild(paths, fplProcessInputMode_Text, processTestInputText, partialLen, fpl_null, fpl_null, &result));
		ftAssertSizeEquals(partialLen, result.output.len);
		fplProcessFreeResult(&result);
	}
	{
		// This is the input deadlock regression test: far more than one pipe buffer goes in and comes back out.
		// An implementation that writes the input first and reads the output afterwards hangs here forever.
		size_t bigInputLen = (size_t)processTestBigInputSize;
		char *bigInputText = (char *)fplMemoryAllocate(bigInputLen + 1);
		ftIsNotNull(bigInputText);
		for (size_t charIndex = 0; charIndex < bigInputLen; ++charIndex) {
			bigInputText[charIndex] = 'a' + (char)(charIndex % 26);
		}
		bigInputText[bigInputLen] = 0;
		fplProcessResult result = fplZeroInit;
		ftIsTrue(ProcessTestsRunCatChild(paths, fplProcessInputMode_Text, bigInputText, bigInputLen, fpl_null, fpl_null, &result));
		ftIsTrue(result.hasExited);
		ftAssertSizeEquals(bigInputLen, result.output.len);
		ftAssertS32Equals(processTestChildSuccessExitCode, result.exitCode);
		fplProcessFreeResult(&result);
		fplMemoryFree(bigInputText);
	}
	{
		// The callback is pulled until it returns zero, which is what closes the standard-input
		ProcessTestInputProvider provider = fplZeroInit;
		provider.remainingChunkCount = processTestInputChunkCount;
		fplProcessResult result = fplZeroInit;
		ftIsTrue(ProcessTestsRunCatChild(paths, fplProcessInputMode_Callback, fpl_null, 0, ProcessTestsProvideInput, &provider, &result));
		ftAssertS32Equals(0, provider.remainingChunkCount);
		char expectedText[FPL_MAX_BUFFER_LENGTH];
		size_t expectedTextLen = 0;
		for (int32_t chunkIndex = 0; chunkIndex < processTestInputChunkCount; ++chunkIndex) {
			char chunkText[64];
			size_t chunkTextLen = fplStringFormat(chunkText, fplArrayCount(chunkText), "chunk-%d\n", chunkIndex);
			ProcessTestsAppendCollectedText(expectedText, &expectedTextLen, fplArrayCount(expectedText), chunkText, chunkTextLen);
		}
		ftAssertStringEquals(expectedText, result.output.text);
		fplProcessFreeResult(&result);
	}
	{
		// The none mode gives the child an immediate end-of-file, so nothing at all comes back
		fplProcessResult result = fplZeroInit;
		ftIsTrue(ProcessTestsRunCatChild(paths, fplProcessInputMode_None, fpl_null, 0, fpl_null, fpl_null, &result));
		ftIsTrue(result.hasExited);
		ftAssertS32Equals(processTestChildSuccessExitCode, result.exitCode);
		ftAssertSizeEquals(0, result.output.len);
		fplProcessFreeResult(&result);
	}
	{
		// The stream mode leaves the standard-input open, the caller writes into it and closes it
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.argumentLine = FPL_TEST_CHILD_ARGUMENT_CAT;
		context.captureFlags = fplProcessCaptureFlags_CaptureOutput;
		context.inputMode = fplProcessInputMode_Stream;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult startResult = fplZeroInit;
		ftIsTrue(fplProcessStart(&context, &handle, &startResult));
		size_t firstLen = fplGetStringLength("first input line\n");
		size_t writtenFirst = fplProcessWriteInput(&handle, "first input line\n", 0);
		size_t writtenSecond = fplProcessWriteInput(&handle, "second input line\n", 0);
		ftAssertSizeEquals(firstLen, writtenFirst);
		ftAssertSizeEquals(fplGetStringLength("second input line\n"), writtenSecond);
		// Without the close the child would wait for more input and never reach its end
		fplProcessCloseInput(&handle);
		fplProcessResult waitResult = fplZeroInit;
		ftIsTrue(fplProcessWait(&handle, FPL_TIMEOUT_INFINITE, &waitResult));
		ftAssertStringEquals(processTestInputText, waitResult.output.text);
		fplProcessFreeResult(&waitResult);
		fplProcessClose(&handle);
	}
	{
		// Writing into a standard-input that FPL feeds itself must be refused instead of interleaving
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.argumentLine = FPL_TEST_CHILD_ARGUMENT_CAT;
		context.captureFlags = fplProcessCaptureFlags_CaptureOutput;
		context.inputMode = fplProcessInputMode_Text;
		context.inputText = processTestInputText;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult startResult = fplZeroInit;
		ftIsTrue(fplProcessStart(&context, &handle, &startResult));
		ftAssertSizeEquals(0, fplProcessWriteInput(&handle, "ignored", 0));
		fplProcessResult waitResult = fplZeroInit;
		fplProcessWait(&handle, FPL_TIMEOUT_INFINITE, &waitResult);
		fplProcessFreeResult(&waitResult);
		fplProcessFreeResult(&startResult);
		fplProcessClose(&handle);
	}
	{
		// A process without any standard-input redirect has nothing to write to and nothing to close
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.argumentLine = FPL_TEST_CHILD_ARGUMENT_EXIT " 0";
		context.flags = fplProcessFlags_AutoWait;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		ftIsTrue(fplProcessStart(&context, &handle, &result));
		ftAssertSizeEquals(0, fplProcessWriteInput(&handle, "ignored", 0));
		fplProcessCloseInput(&handle);
		fplProcessFreeResult(&result);
		fplProcessClose(&handle);
	}
}

static void ProcessTestsInputFailures(const ProcessTestPaths *paths) {
	ftMsg("Test Process standard-input failures (the logged errors are expected)\n");
	{
		// The text mode without a text is wrong, not unimplemented
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.argumentLine = FPL_TEST_CHILD_ARGUMENT_CAT;
		context.inputMode = fplProcessInputMode_Text;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		ftIsFalse(fplProcessStart(&context, &handle, &result));
		ftAssert(result.type == fplProcessResultType_InvalidArguments);
		fplProcessClose(&handle);
	}
	{
		// The callback mode without a callback is wrong as well
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.argumentLine = FPL_TEST_CHILD_ARGUMENT_CAT;
		context.inputMode = fplProcessInputMode_Callback;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		ftIsFalse(fplProcessStart(&context, &handle, &result));
		ftAssert(result.type == fplProcessResultType_InvalidArguments);
		fplProcessClose(&handle);
	}
}

static void ProcessTestsCaptureFailures(const ProcessTestPaths *paths) {
	ftMsg("Test Process capture failures (the logged errors are expected)\n");
	{
		// Capture flags that select a stream but no target are wrong, not unimplemented
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.argumentLine = FPL_TEST_CHILD_ARGUMENT_BOTH;
		context.captureFlags = fplProcessCaptureFlags_Output;
		context.flags = fplProcessFlags_AutoWait;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		ftIsFalse(fplProcessStart(&context, &handle, &result));
		ftAssert(result.type == fplProcessResultType_InvalidArguments);
		fplProcessClose(&handle);
	}
	{
		// Capture flags that select a target but no stream are wrong as well
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.argumentLine = FPL_TEST_CHILD_ARGUMENT_BOTH;
		context.captureFlags = fplProcessCaptureFlags_ToBuffer;
		context.flags = fplProcessFlags_AutoWait;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		ftIsFalse(fplProcessStart(&context, &handle, &result));
		ftAssert(result.type == fplProcessResultType_InvalidArguments);
		fplProcessClose(&handle);
	}
	{
		// Asking for a callback without providing one is wrong
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.argumentLine = FPL_TEST_CHILD_ARGUMENT_BOTH;
		context.captureFlags = fplProcessCaptureFlags_RedirectOutput;
		context.flags = fplProcessFlags_AutoWait;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		ftIsFalse(fplProcessStart(&context, &handle, &result));
		ftAssert(result.type == fplProcessResultType_InvalidArguments);
		fplProcessClose(&handle);
	}
}

// Builds the command that starts the test executable through a shell. The path is put in quotes, because a
// shell splits an unquoted command at every space and FPL never quotes the name of a shell command line.
static void ProcessTestsBuildShellCommand(const ProcessTestPaths *paths, char *targetBuffer, const size_t maxTargetBufferLen) {
	fplStringFormat(targetBuffer, maxTargetBufferLen, "\"%s\"", paths->executableFilePath);
}

// Writes the script the shell tests execute, in the syntax of the shell of the platform
static bool ProcessTestsCreateScriptFile(const char *scriptFilePath) {
	char scriptContent[FPL_MAX_BUFFER_LENGTH];
#if defined(FPL_PLATFORM_WINDOWS)
	fplStringFormat(scriptContent, fplArrayCount(scriptContent), "@echo off\r\necho %s\r\nexit /b %d\r\n", processTestScriptText, processTestScriptExitCode);
#else
	fplStringFormat(scriptContent, fplArrayCount(scriptContent), "#!/bin/sh\necho %s\nexit %d\n", processTestScriptText, processTestScriptExitCode);
#endif
	fplFileHandle scriptFile = fplZeroInit;
	if (!fplFileCreateBinary(scriptFilePath, &scriptFile)) {
		return(false);
	}
	size_t scriptContentLen = fplGetStringLength(scriptContent);
	fplFileWriteBlock32(&scriptFile, scriptContent, (uint32_t)scriptContentLen);
	fplFileClose(&scriptFile);
	return(true);
}

// Starts a command through the default shell and waits for it, this is what the script tests need for
// the preparation steps
static bool ProcessTestsRunShellCommand(const char *command, fplProcessResult *outResult) {
	fplProcessContext context = fplZeroInit;
	context.name = command;
	context.shellMode = fplProcessShellMode_Default;
	context.flags = fplProcessFlags_AutoWait;
	fplProcessHandle handle = fplZeroInit;
	bool started = fplProcessStart(&context, &handle, outResult);
	fplProcessClose(&handle);
	return(started);
}

static void ProcessTestsShell(const ProcessTestPaths *paths) {
	ftMsg("Test Process shell execution\n");
	char shellCommand[FPL_MAX_PATH_LENGTH];
	ProcessTestsBuildShellCommand(paths, shellCommand, fplArrayCount(shellCommand));
	{
		// The default shell must start the program and hand its exit code back unchanged
		char argumentLine[FPL_MAX_BUFFER_LENGTH];
		fplStringFormat(argumentLine, fplArrayCount(argumentLine), "%s %d", FPL_TEST_CHILD_ARGUMENT_EXIT, processTestCustomExitCode);
		fplProcessContext context = fplZeroInit;
		context.name = shellCommand;
		context.argumentLine = argumentLine;
		context.shellMode = fplProcessShellMode_Default;
		context.flags = fplProcessFlags_AutoWait;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		ftIsTrue(fplProcessStart(&context, &handle, &result));
		ftAssert(result.type == fplProcessResultType_Success);
		ftAssertS32Equals(processTestCustomExitCode, result.exitCode);
		fplProcessFreeResult(&result);
		fplProcessClose(&handle);
	}
	{
		// Capturing works the same way through a shell, the shell itself must not add anything
		char argumentLine[FPL_MAX_BUFFER_LENGTH];
		fplStringFormat(argumentLine, fplArrayCount(argumentLine), "%s %s", FPL_TEST_CHILD_ARGUMENT_ECHO, processTestEchoText);
		fplProcessContext context = fplZeroInit;
		context.name = shellCommand;
		context.argumentLine = argumentLine;
		context.shellMode = fplProcessShellMode_Default;
		context.captureFlags = fplProcessCaptureFlags_CaptureOutput;
		context.flags = fplProcessFlags_AutoWait;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		ftIsTrue(fplProcessStart(&context, &handle, &result));
		ftIsNotNull(result.output.text);
		size_t expectedLen = fplGetStringLength(processTestEchoText) + 1;
		ftAssertSizeEquals(expectedLen, result.output.len);
		fplProcessFreeResult(&result);
		fplProcessClose(&handle);
	}
	{
		// The entries of an argument array must survive the quoting of FPL and the parsing of the shell
		const char *arguments[] = {
			FPL_TEST_CHILD_ARGUMENT_CHECK_ARGS,
			"3",
			processTestArgumentValues[0],
			processTestArgumentValues[1],
			processTestArgumentValues[2],
		};
		fplProcessContext context = fplZeroInit;
		context.name = shellCommand;
		context.arguments = arguments;
		context.argumentCount = fplArrayCount(arguments);
		context.shellMode = fplProcessShellMode_Default;
		context.flags = fplProcessFlags_AutoWait;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		ftIsTrue(fplProcessStart(&context, &handle, &result));
		ftAssertS32Equals(processTestChildSuccessExitCode, result.exitCode);
		fplProcessFreeResult(&result);
		fplProcessClose(&handle);
	}
	{
		// A custom shell is used instead of the default one, the interpreter is named by the caller
#if defined(FPL_PLATFORM_WINDOWS)
		const char *customShellPath = "cmd.exe";
#else
		const char *customShellPath = "/bin/sh";
#endif
		char argumentLine[FPL_MAX_BUFFER_LENGTH];
		fplStringFormat(argumentLine, fplArrayCount(argumentLine), "%s %d", FPL_TEST_CHILD_ARGUMENT_EXIT, processTestCustomExitCode);
		fplProcessContext context = fplZeroInit;
		context.name = shellCommand;
		context.argumentLine = argumentLine;
		context.shellMode = fplProcessShellMode_Custom;
		context.shellPath = customShellPath;
		context.flags = fplProcessFlags_AutoWait;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		ftIsTrue(fplProcessStart(&context, &handle, &result));
		ftAssert(result.type == fplProcessResultType_Success);
		ftAssertS32Equals(processTestCustomExitCode, result.exitCode);
		fplProcessFreeResult(&result);
		fplProcessClose(&handle);
	}
}

// Running a script is the reason the shell mode exists, so it gets its own group
static void ProcessTestsShellScript(const ProcessTestPaths *paths) {
	ftMsg("Test Process shell script\n");
	char scriptFilePath[FPL_MAX_PATH_LENGTH];
	fplPathCombine(scriptFilePath, fplArrayCount(scriptFilePath), 2, paths->executableDirectoryPath, processTestScriptFileName);
	if (!ProcessTestsCreateScriptFile(scriptFilePath)) {
		ftFail("Failed creating the script file '%s'", scriptFilePath);
		return;
	}
	char scriptCommand[FPL_MAX_PATH_LENGTH];
	fplStringFormat(scriptCommand, fplArrayCount(scriptCommand), "\"%s\"", scriptFilePath);
#if !defined(FPL_PLATFORM_WINDOWS)
	{
		// A POSIX shell only executes a file that is marked as executable, a batch file needs nothing like that
		char chmodCommand[FPL_MAX_PATH_LENGTH];
		fplStringFormat(chmodCommand, fplArrayCount(chmodCommand), "chmod +x \"%s\"", scriptFilePath);
		fplProcessResult chmodResult = fplZeroInit;
		ftIsTrue(ProcessTestsRunShellCommand(chmodCommand, &chmodResult));
		ftAssertS32Equals(0, chmodResult.exitCode);
		fplProcessFreeResult(&chmodResult);
	}
#endif
	{
		// The captured text of a script starts with what it has written, the line ending is up to the shell
		fplProcessContext context = fplZeroInit;
		context.name = scriptCommand;
		context.shellMode = fplProcessShellMode_Default;
		context.captureFlags = fplProcessCaptureFlags_CaptureBoth;
		context.flags = fplProcessFlags_AutoWait;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		ftIsTrue(fplProcessStart(&context, &handle, &result));
		ftAssertS32Equals(processTestScriptExitCode, result.exitCode);
		ftIsNotNull(result.output.text);
		size_t scriptTextLen = fplGetStringLength(processTestScriptText);
		ftIsTrue(result.output.len >= scriptTextLen);
		ftIsTrue(fplIsStringEqualLen(result.output.text, scriptTextLen, processTestScriptText, scriptTextLen));
		fplProcessFreeResult(&result);
		fplProcessClose(&handle);
	}
	fplFileDelete(scriptFilePath);
}

// The flags that change how a child is created must not change how it is started, waited for and reported
static void ProcessTestsCreationFlags(const ProcessTestPaths *paths) {
	ftMsg("Test Process creation flags\n");
	char argumentLine[FPL_MAX_BUFFER_LENGTH];
	fplStringFormat(argumentLine, fplArrayCount(argumentLine), "%s %d", FPL_TEST_CHILD_ARGUMENT_EXIT, processTestCustomExitCode);
	fplProcessFlags flagVariants[] = {
		fplProcessFlags_NoWindow,
		fplProcessFlags_Detached,
		fplProcessFlags_KillOnParentExit,
		fplProcessFlags_KillProcessTree,
	};
	for (size_t variantIndex = 0; variantIndex < fplArrayCount(flagVariants); ++variantIndex) {
		fplProcessResult result = fplZeroInit;
		bool started = ProcessTestsRunChild(paths, argumentLine, fpl_null, 0, fpl_null, flagVariants[variantIndex], &result);
		ftIsTrue(started);
		ftAssert(result.type == fplProcessResultType_Success);
		ftAssertS32Equals(processTestCustomExitCode, result.exitCode);
		fplProcessFreeResult(&result);
	}
	{
		// A graceful stop needs the process tree flag on Windows, because a console control event is
		// sent to the process group instead of a signal
		char sleepArgumentLine[FPL_MAX_BUFFER_LENGTH];
		fplStringFormat(sleepArgumentLine, fplArrayCount(sleepArgumentLine), "%s %d", FPL_TEST_CHILD_ARGUMENT_SLEEP, processTestLongSleepInMilliseconds);
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.argumentLine = sleepArgumentLine;
		context.flags = fplProcessFlags_KillProcessTree;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult startResult = fplZeroInit;
		ftIsTrue(fplProcessStart(&context, &handle, &startResult));
		ftIsTrue(fplProcessRequestStop(&handle));
		fplProcessResult stopResult = fplZeroInit;
		ftIsTrue(fplProcessWait(&handle, processTestStopTimeout, &stopResult));
		ftIsTrue(stopResult.hasExited);
#if !defined(FPL_PLATFORM_WINDOWS)
		ftAssertS32Equals(SIGTERM, stopResult.terminationSignal);
#endif
		fplProcessFreeResult(&stopResult);
		fplProcessClose(&handle);
	}
}

// Options that contradict each other or miss something they need must be rejected instead of being guessed
static void ProcessTestsFlagFailures(const ProcessTestPaths *paths) {
	ftMsg("Test Process shell and flag failures (the logged errors are expected)\n");
	{
		// A custom shell without a shell path cannot be started
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.shellMode = fplProcessShellMode_Custom;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		ftIsFalse(fplProcessStart(&context, &handle, &result));
		ftAssert(result.type == fplProcessResultType_InvalidArguments);
		fplProcessClose(&handle);
	}
	{
		// One flag lets the child survive this process, the other one kills it exactly then
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.flags = fplProcessFlags_Detached | fplProcessFlags_KillOnParentExit;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		ftIsFalse(fplProcessStart(&context, &handle, &result));
		ftAssert(result.type == fplProcessResultType_InvalidArguments);
		fplProcessClose(&handle);
	}
	{
		// Pumping a process without any redirected stream reports that there is nothing to pump
		fplProcessResult result = fplZeroInit;
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.argumentLine = processTestSilentChildArgumentLine;
		fplProcessHandle handle = fplZeroInit;
		if (fplProcessStart(&context, &handle, &result)) {
			ftIsFalse(fplProcessUpdate(&handle));
			fplProcessWait(&handle, FPL_TIMEOUT_INFINITE, fpl_null);
		}
		fplProcessFreeResult(&result);
		fplProcessClose(&handle);
	}
}

void FPLProcessTests_All(void) {
	ProcessTestPaths paths = fplZeroInit;
	if (!ProcessTestsQueryPaths(&paths)) {
		ftFail("Failed querying the paths of the test executable");
		return;
	}
	ftMsg("Using '%s' as the child program\n", paths.executableFilePath);
	ProcessTestsCurrentProcess();
	ProcessTestsInvalidArguments(&paths);
	ProcessTestsStartFailures();
	ProcessTestsWorkDirFailure(&paths);
	ProcessTestsExitCodes(&paths);
	ProcessTestsArguments(&paths);
	ProcessTestsWorkDir(&paths);
	ProcessTestsAsyncAndStop(&paths);
	ProcessTestsCapture(&paths);
	ProcessTestsCaptureSeparate(&paths);
	ProcessTestsCallbacks(&paths);
	ProcessTestsCaptureFailures(&paths);
	ProcessTestsInput(&paths);
	ProcessTestsInputFailures(&paths);
	ProcessTestsShell(&paths);
	ProcessTestsShellScript(&paths);
	ProcessTestsCreationFlags(&paths);
	ProcessTestsHandleLifetime(&paths);
	ProcessTestsFlagFailures(&paths);
}
