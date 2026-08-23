/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Process Tests

Description:
	Tests for the fplProcess* API, including the failure paths.

	The tests need a program to start. Instead of depending on any system tool,
	the test executable starts itself with one of the "--child-" arguments and
	uses the exit code of that child as the answer. This behaves identically on
	every platform and needs no external files.

	Because the capture/redirect support is not implemented yet, every child
	reports its result through the exit code only. When capture lands, the
	echo/error children can be verified through the captured text as well.

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

// Exit code a child reports when everything was as expected
static const int32_t processTestChildSuccessExitCode = 0;
// Exit code a child reports when the expectation was not met
static const int32_t processTestChildFailureExitCode = 1;
// Exit code the exit-code tests ask the child for, a value no runtime uses on its own
static const int32_t processTestCustomExitCode = 42;
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
	ftMsg("Test Process invalid arguments\n");
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
	ftMsg("Test Process start failures\n");
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
	ftMsg("Test Process work directory failure\n");
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
		bool started = ProcessTestsRunChild(paths, FPL_TEST_CHILD_ARGUMENT_ECHO " hello", fpl_null, 0, fpl_null, fplProcessFlags_None, &result);
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
		bool started = ProcessTestsRunChild(paths, FPL_TEST_CHILD_ARGUMENT_ECHO " twice", fpl_null, 0, fpl_null, fplProcessFlags_None, &result);
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
			bool started = ProcessTestsRunChild(paths, FPL_TEST_CHILD_ARGUMENT_ECHO " cycle", fpl_null, 0, fpl_null, fplProcessFlags_None, &result);
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

static void ProcessTestsCaptureFailures(const ProcessTestPaths *paths) {
	ftMsg("Test Process capture failures\n");
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

// A callback that does nothing, used to prove that the redirect is rejected because it is not implemented
// and not because the callback was missing
static FPL_FUNC_PROCESS_OUTPUT(ProcessTestsOnOutput) {
	(void)process;
	(void)stream;
	(void)text;
	(void)textLen;
	(void)userData;
}

// Every option that is not implemented yet must be reported instead of being ignored silently.
// @TODO(final): Remove each block below, as soon as the matching phase is implemented.
static void ProcessTestsNotImplemented(const ProcessTestPaths *paths) {
	ftMsg("Test Process not implemented options\n");
	{
		// Shell execution (Phase 6)
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.shellMode = fplProcessShellMode_Default;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		ftIsFalse(fplProcessStart(&context, &handle, &result));
		ftAssert(result.type == fplProcessResultType_NotImplemented);
		fplProcessClose(&handle);
	}
	{
		// Capturing the output and the error stream into separate buffers (Phase 4)
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.captureFlags = fplProcessCaptureFlags_CaptureSeparate;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		ftIsFalse(fplProcessStart(&context, &handle, &result));
		ftAssert(result.type == fplProcessResultType_NotImplemented);
		fplProcessClose(&handle);
	}
	{
		// Redirecting a stream into a callback (Phase 4)
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.captureFlags = fplProcessCaptureFlags_RedirectBoth;
		context.outputCallback = ProcessTestsOnOutput;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		ftIsFalse(fplProcessStart(&context, &handle, &result));
		ftAssert(result.type == fplProcessResultType_NotImplemented);
		fplProcessClose(&handle);
	}
	{
		// Splitting the captured text into lines (Phase 4)
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.captureFlags = fplProcessCaptureFlags_CaptureOutput;
		context.flags = fplProcessFlags_LineBuffered;
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		ftIsFalse(fplProcessStart(&context, &handle, &result));
		ftAssert(result.type == fplProcessResultType_NotImplemented);
		fplProcessClose(&handle);
	}
	{
		// Standard-input (Phase 5)
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.inputMode = fplProcessInputMode_Text;
		context.inputText = "hello";
		fplProcessHandle handle = fplZeroInit;
		fplProcessResult result = fplZeroInit;
		ftIsFalse(fplProcessStart(&context, &handle, &result));
		ftAssert(result.type == fplProcessResultType_NotImplemented);
		fplProcessClose(&handle);
	}
	{
		// Pumping a process without any redirected stream reports that there is nothing to pump
		fplProcessResult result = fplZeroInit;
		fplProcessContext context = fplZeroInit;
		context.name = paths->executableFilePath;
		context.argumentLine = FPL_TEST_CHILD_ARGUMENT_ECHO " update";
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
	ProcessTestsCaptureFailures(&paths);
	ProcessTestsHandleLifetime(&paths);
	ProcessTestsNotImplemented(&paths);
}
