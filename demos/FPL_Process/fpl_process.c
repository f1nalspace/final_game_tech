/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Process

Description:
	A Console Application demonstrating how to start and control child processes

Requirements:
	- C99 Compiler
	- Final Platform Layer

Author:
	Torsten Spaete

Changelog:
	## 2026-08-23
	- Added the separate capture and the output callback scenarios
	- Fixed the argument array scenario, it started an interactive cmd.exe on Windows and hung there
	- Added the capture scenarios
	- Initial creation of this demo

License:
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#define FPL_NO_WINDOW
#define FPL_NO_VIDEO
#define FPL_NO_AUDIO
#include <final_platform_layer.h>

//
// The demo runs the same scenarios on every platform, only the programs differ
//
#if defined(FPL_PLATFORM_WINDOWS)
#	define DemoEchoProgram "cmd.exe"
#	define DemoEchoArgumentLine "/c echo Hello from the child process"
#	define DemoWorkDirProgram "cmd.exe"
#	define DemoWorkDirArgumentLine "/c cd"
#	define DemoWorkDirPath "C:\\Windows"
#	define DemoExitCodeProgram "cmd.exe"
#	define DemoExitCodeArgumentLine "/c exit 3"
#	define DemoSleepProgram "cmd.exe"
#	define DemoSleepArgumentLine "/c ping -n 6 127.0.0.1"
#else
#	define DemoEchoProgram "echo"
#	define DemoEchoArgumentLine "Hello from the child process"
#	define DemoWorkDirProgram "pwd"
#	define DemoWorkDirArgumentLine fpl_null
#	define DemoWorkDirPath "/tmp"
#	define DemoExitCodeProgram "sh"
#	define DemoExitCodeArgumentLine "-c \"exit 3\""
#	define DemoSleepProgram "sleep"
#	define DemoSleepArgumentLine "5"
#endif

#define DemoMissingProgram "this_program_does_not_exist_xyz"

// Argument that makes this demo write a lot of text, used to show that a big output is captured without deadlocking
#define DemoSpamArgument "--spam"

// Argument that makes this demo print every argument it received, used to show that an argument array arrives unchanged
#define DemoPrintArgumentsArgument "--print-args"

// Argument that makes this demo write a few lines to both standard streams, used for the separate capture and the callback
#define DemoWriteStreamsArgument "--write-streams"

// Milliseconds we wait in the timeout scenario, the child runs a lot longer than that
static const fplTimeoutValue demoShortWaitTimeout = 250;
// Milliseconds slept between two polls of fplProcessIsRunning()
static const uint32_t demoPollIntervalInMilliseconds = 100;
// Number of polls before the demo gives up and stops the child
static const int demoMaxPollCount = 5;
// Number of characters written in one go by the spam mode
static const size_t demoSpamChunkSize = 1024;
// Number of characters the spam mode writes in total
static const int32_t demoSpamTotalSize = 1000000;
// Number of characters the capture is limited to in the truncation scenario
static const size_t demoCaptureLimit = 256;
// Number of characters of the captured text the demo prints at most
static const size_t demoMaxPrintedCaptureSize = 400;
// Number of lines the child writes to each standard stream
static const int demoStreamLineCount = 3;

static const char *GetProcessResultTypeName(const fplProcessResultType type) {
	switch (type) {
		case fplProcessResultType_Success:
			return("Success");
		case fplProcessResultType_InvalidArguments:
			return("InvalidArguments");
		case fplProcessResultType_NotFound:
			return("NotFound");
		case fplProcessResultType_AccessDenied:
			return("AccessDenied");
		case fplProcessResultType_FailedToStart:
			return("FailedToStart");
		case fplProcessResultType_FailedWithExitCode:
			return("FailedWithExitCode");
		case fplProcessResultType_Terminated:
			return("Terminated");
		case fplProcessResultType_Timeout:
			return("Timeout");
		case fplProcessResultType_OutOfMemory:
			return("OutOfMemory");
		case fplProcessResultType_NotImplemented:
			return("NotImplemented");
		default:
			return("Unknown");
	}
}

static void PrintProcessBuffer(const char *name, const fplProcessBuffer *buffer, const bool isTruncated) {
	if ((buffer->text == fpl_null) || (buffer->len == 0)) {
		return;
	}
	fplConsoleFormatOut("  -> %s (%zu bytes%s):\n", name, buffer->len, isTruncated ? ", truncated" : "");
	if (buffer->len <= demoMaxPrintedCaptureSize) {
		fplConsoleFormatOut("%s", buffer->text);
	} else {
		// A big capture is only shown in part, the interesting thing here is the size
		char preview[512];
		fplCopyStringLen(buffer->text, demoMaxPrintedCaptureSize, preview, fplArrayCount(preview));
		fplConsoleFormatOut("%s\n     ... (%zu more bytes)\n", preview, buffer->len - demoMaxPrintedCaptureSize);
	}
	if (buffer->text[buffer->len - 1] != '\n') {
		fplConsoleOut("\n");
	}
}

static void PrintProcessResult(const fplProcessResult *result) {
	const char *typeName = GetProcessResultTypeName(result->type);
	fplConsoleFormatOut("  -> type: %s, exited: %s, exit code: %d", typeName, result->hasExited ? "yes" : "no", result->exitCode);
	if (result->terminationSignal != 0) {
		fplConsoleFormatOut(", signal: %d", result->terminationSignal);
	}
	if (result->nativeErrorCode != 0) {
		fplConsoleFormatOut(", native error: %lu", (unsigned long)result->nativeErrorCode);
	}
	fplConsoleOut("\n");
	PrintProcessBuffer("output", &result->output, result->isTruncated);
	PrintProcessBuffer("error", &result->error, result->isTruncated);
}

static void PrintScenarioHeader(const char *title) {
	fplConsoleFormatOut("\n== %s ==\n", title);
}

// Runs a program and waits for it to be finished
static void RunAndWait(const char *title, const fplProcessContext *context) {
	PrintScenarioHeader(title);
	fplProcessHandle handle = fplZeroInit;
	fplProcessResult result = fplZeroInit;
	bool started = fplProcessStart(context, &handle, &result);
	if (started) {
		fplConsoleFormatOut("  started '%s' with the process id %llu\n", context->name, (unsigned long long)handle.id);
	} else {
		fplConsoleFormatOut("  could not start '%s'\n", context->name);
	}
	PrintProcessResult(&result);
	fplProcessFreeResult(&result);
	fplProcessClose(&handle);
}

static void RunSimpleScenario(void) {
	fplProcessContext context = fplZeroInit;
	context.name = DemoEchoProgram;
	context.argumentLine = DemoEchoArgumentLine;
	context.captureFlags = fplProcessCaptureFlags_CaptureBoth;
	context.flags = fplProcessFlags_AutoWait;
	RunAndWait("Start a program and capture what it writes", &context);
}

// Prints every argument the demo was started with, so the parent can see that each one arrived unchanged
static void RunPrintArgumentsMode(const int argumentCount, char **arguments) {
	fplConsoleFormatOut("got %d arguments\n", argumentCount);
	for (int argumentIndex = 0; argumentIndex < argumentCount; ++argumentIndex) {
		fplConsoleFormatOut("  [%d] '%s'\n", argumentIndex, arguments[argumentIndex]);
	}
}

static void RunArgumentArrayScenario(void) {
	// The argument array is the exact way to pass arguments, because nothing has to be quoted or parsed.
	// The demo starts itself here, because Windows has no standalone echo program and a shell started
	// without a command would wait for user input forever instead of exiting.
	char executablePath[FPL_MAX_PATH_LENGTH];
	fplGetExecutableFilePath(executablePath, fplArrayCount(executablePath));
	const char *arguments[] = { DemoPrintArgumentsArgument, "first argument with spaces", "second", "third with \"quotes\"" };
	fplProcessContext context = fplZeroInit;
	context.name = executablePath;
	context.arguments = arguments;
	context.argumentCount = fplArrayCount(arguments);
	context.captureFlags = fplProcessCaptureFlags_CaptureBoth;
	context.flags = fplProcessFlags_AutoWait;
	RunAndWait("Pass arguments as an array", &context);
}

static void RunWorkDirScenario(void) {
	fplProcessContext context = fplZeroInit;
	context.name = DemoWorkDirProgram;
	context.argumentLine = DemoWorkDirArgumentLine;
	context.workDir = DemoWorkDirPath;
	context.captureFlags = fplProcessCaptureFlags_CaptureBoth;
	context.flags = fplProcessFlags_AutoWait;
	RunAndWait("Run in a different work directory", &context);
}

static void RunExitCodeScenario(void) {
	// A non-zero exit code is a normal result, many programs use it to report a state instead of an error
	fplProcessContext contextWithoutErrorFlag = fplZeroInit;
	contextWithoutErrorFlag.name = DemoExitCodeProgram;
	contextWithoutErrorFlag.argumentLine = DemoExitCodeArgumentLine;
	contextWithoutErrorFlag.captureFlags = fplProcessCaptureFlags_CaptureBoth;
	contextWithoutErrorFlag.flags = fplProcessFlags_AutoWait;
	RunAndWait("Exit code is not an error by default", &contextWithoutErrorFlag);

	// Only when the caller asks for it, a non-zero exit code becomes a failure
	fplProcessContext contextWithErrorFlag = contextWithoutErrorFlag;
	contextWithErrorFlag.flags = fplProcessFlags_AutoWait | fplProcessFlags_TreatNonZeroExitAsError;
	RunAndWait("Exit code as an error, when asked for", &contextWithErrorFlag);
}

static void RunMissingProgramScenario(void) {
	fplProcessContext context = fplZeroInit;
	context.name = DemoMissingProgram;
	context.flags = fplProcessFlags_AutoWait;
	RunAndWait("Start a program that does not exist", &context);
}

static void RunTimeoutScenario(void) {
	PrintScenarioHeader("Wait with a timeout");
	fplProcessContext context = fplZeroInit;
	context.name = DemoSleepProgram;
	context.argumentLine = DemoSleepArgumentLine;
	context.flags = fplProcessFlags_AutoWait;
	context.waitTimeout = demoShortWaitTimeout;
	fplProcessHandle handle = fplZeroInit;
	fplProcessResult result = fplZeroInit;
	if (fplProcessStart(&context, &handle, &result)) {
		fplConsoleFormatOut("  waited %lu ms for the process id %llu\n", (unsigned long)demoShortWaitTimeout, (unsigned long long)handle.id);
		PrintProcessResult(&result);
		fplConsoleOut("  stopping the process, because it did not finish in time\n");
		fplProcessStop(&handle);
		fplProcessResult stopResult = fplZeroInit;
		fplProcessWait(&handle, FPL_TIMEOUT_INFINITE, &stopResult);
		PrintProcessResult(&stopResult);
		fplProcessFreeResult(&stopResult);
	}
	fplProcessFreeResult(&result);
	fplProcessClose(&handle);
}

static void RunAsyncScenario(void) {
	PrintScenarioHeader("Start without waiting and poll the state");
	fplProcessContext context = fplZeroInit;
	context.name = DemoSleepProgram;
	context.argumentLine = DemoSleepArgumentLine;
	fplProcessHandle handle = fplZeroInit;
	fplProcessResult startResult = fplZeroInit;
	if (!fplProcessStart(&context, &handle, &startResult)) {
		PrintProcessResult(&startResult);
		fplProcessClose(&handle);
		return;
	}
	fplConsoleFormatOut("  started the process id %llu, doing other work while it runs\n", (unsigned long long)handle.id);
	int pollCount = 0;
	while (fplProcessIsRunning(&handle) && (pollCount < demoMaxPollCount)) {
		++pollCount;
		fplConsoleFormatOut("  poll %d: the process is still running\n", pollCount);
		fplThreadSleep(demoPollIntervalInMilliseconds);
	}
	if (fplProcessIsRunning(&handle)) {
		fplConsoleOut("  asking the process to stop gracefully\n");
		fplProcessRequestStop(&handle);
	}
	fplProcessResult exitResult = fplZeroInit;
	fplProcessWait(&handle, FPL_TIMEOUT_INFINITE, &exitResult);
	PrintProcessResult(&exitResult);
	fplProcessFreeResult(&exitResult);
	fplProcessClose(&handle);
}

// Writes a lot of text, so the capture scenarios have something to chew on
static void RunSpamMode(const int32_t totalSize) {
	char chunk[1025];
	for (size_t charIndex = 0; charIndex < demoSpamChunkSize; ++charIndex) {
		chunk[charIndex] = 'x';
	}
	chunk[demoSpamChunkSize] = 0;
	int32_t remainingSize = totalSize;
	while (remainingSize >= (int32_t)demoSpamChunkSize) {
		fplConsoleOut(chunk);
		remainingSize -= (int32_t)demoSpamChunkSize;
	}
	if (remainingSize > 0) {
		chunk[remainingSize] = 0;
		fplConsoleOut(chunk);
	}
}

// Starts this very demo again, so the scenario works the same way on every platform
static void BuildSpamContext(fplProcessContext *outContext, char *executablePath, const size_t maxExecutablePathLen, char *argumentLine, const size_t maxArgumentLineLen) {
	fplGetExecutableFilePath(executablePath, maxExecutablePathLen);
	fplStringFormat(argumentLine, maxArgumentLineLen, "%s %d", DemoSpamArgument, demoSpamTotalSize);
	fplClearStruct(outContext);
	outContext->name = executablePath;
	outContext->argumentLine = argumentLine;
	outContext->flags = fplProcessFlags_AutoWait;
}

static void RunLargeCaptureScenario(void) {
	PrintScenarioHeader("Capture a lot more than one pipe buffer holds");
	char executablePath[FPL_MAX_PATH_LENGTH];
	char argumentLine[FPL_MAX_BUFFER_LENGTH];
	fplProcessContext context;
	BuildSpamContext(&context, executablePath, fplArrayCount(executablePath), argumentLine, fplArrayCount(argumentLine));
	context.captureFlags = fplProcessCaptureFlags_CaptureBoth;

	fplProcessHandle handle = fplZeroInit;
	fplProcessResult result = fplZeroInit;
	fplMilliseconds startTime = fplMillisecondsQuery();
	bool started = fplProcessStart(&context, &handle, &result);
	fplMilliseconds elapsedTime = fplMillisecondsQuery() - startTime;
	if (started) {
		fplConsoleFormatOut("  captured %zu bytes in %llu ms, truncated: %s\n", result.output.len, (unsigned long long)elapsedTime, result.isTruncated ? "yes" : "no");
	}
	PrintProcessResult(&result);
	fplProcessFreeResult(&result);
	fplProcessClose(&handle);
}

static void RunCaptureLimitScenario(void) {
	PrintScenarioHeader("Limit how much is captured");
	char executablePath[FPL_MAX_PATH_LENGTH];
	char argumentLine[FPL_MAX_BUFFER_LENGTH];
	fplProcessContext context;
	BuildSpamContext(&context, executablePath, fplArrayCount(executablePath), argumentLine, fplArrayCount(argumentLine));
	context.captureFlags = fplProcessCaptureFlags_CaptureBoth;
	context.maxCaptureSize = demoCaptureLimit;

	fplProcessHandle handle = fplZeroInit;
	fplProcessResult result = fplZeroInit;
	if (fplProcessStart(&context, &handle, &result)) {
		fplConsoleFormatOut("  the child wrote %d bytes, the capture kept %zu of them\n", demoSpamTotalSize, result.output.len);
	}
	PrintProcessResult(&result);
	fplProcessFreeResult(&result);
	fplProcessClose(&handle);
}

// Writes a few lines to both standard streams, so the separate capture and the callback have something to show
static void RunWriteStreamsMode(void) {
	char lineText[128];
	for (int lineIndex = 0; lineIndex < demoStreamLineCount; ++lineIndex) {
		fplStringFormat(lineText, fplArrayCount(lineText), "output line %d\n", lineIndex);
		fplConsoleOut(lineText);
		fplStringFormat(lineText, fplArrayCount(lineText), "error line %d\n", lineIndex);
		fplConsoleError(lineText);
	}
}

static void RunSeparateCaptureScenario(void) {
	// Without the merge flag each stream gets its own pipe and its own buffer in the result
	char executablePath[FPL_MAX_PATH_LENGTH];
	fplGetExecutableFilePath(executablePath, fplArrayCount(executablePath));
	fplProcessContext context = fplZeroInit;
	context.name = executablePath;
	context.argumentLine = DemoWriteStreamsArgument;
	context.captureFlags = fplProcessCaptureFlags_CaptureSeparate;
	context.flags = fplProcessFlags_AutoWait;
	RunAndWait("Capture the output and the error stream separately", &context);
}

// Receives one complete line at a time, because the scenario asks for fplProcessFlags_LineBuffered
static FPL_FUNC_PROCESS_OUTPUT(OnProcessOutputLine) {
	(void)process;
	(void)textLen;
	int *receivedLineCount = (int *)userData;
	++(*receivedLineCount);
	const char *streamName = (stream == fplProcessStreamType_Error) ? "error" : "output";
	fplConsoleFormatOut("  [%s] %s\n", streamName, text);
}

static void RunOutputCallbackScenario(void) {
	PrintScenarioHeader("Redirect both streams into a callback, line by line");
	char executablePath[FPL_MAX_PATH_LENGTH];
	fplGetExecutableFilePath(executablePath, fplArrayCount(executablePath));
	int receivedLineCount = 0;
	fplProcessContext context = fplZeroInit;
	context.name = executablePath;
	context.argumentLine = DemoWriteStreamsArgument;
	// The two redirect presets together keep the streams apart, so the callback can tell them apart as well.
	// Only the order inside one stream is kept, the order between them is not - a merged capture is the way to get one interleaved text.
	context.captureFlags = fplProcessCaptureFlags_RedirectOutput | fplProcessCaptureFlags_RedirectError;
	context.outputCallback = OnProcessOutputLine;
	context.userData = &receivedLineCount;
	context.flags = fplProcessFlags_AutoWait | fplProcessFlags_LineBuffered;
	fplProcessHandle handle = fplZeroInit;
	fplProcessResult result = fplZeroInit;
	if (fplProcessStart(&context, &handle, &result)) {
		fplConsoleFormatOut("  the callback received %d lines, nothing was stored in the result\n", receivedLineCount);
	}
	PrintProcessResult(&result);
	fplProcessFreeResult(&result);
	fplProcessClose(&handle);
}

// Runs the program the user passed on the command line, so the demo can start anything
static void RunUserProgram(const int argumentCount, char **arguments) {
	const char *programName = arguments[0];
	const char **programArguments = (const char **)(arguments + 1);
	size_t programArgumentCount = (size_t)(argumentCount - 1);
	fplProcessContext context = fplZeroInit;
	context.name = programName;
	context.arguments = programArguments;
	context.argumentCount = programArgumentCount;
	context.captureFlags = fplProcessCaptureFlags_CaptureBoth;
	context.flags = fplProcessFlags_AutoWait;
	RunAndWait("Run the program from the command line", &context);
}

int main(int argc, char *args[]) {
	if (!fplPlatformInit(fplInitFlags_All, fpl_null)) {
		return(-1);
	}

	// This demo starts itself for the capture scenarios, that mode replaces the whole run
	if ((argc > 2) && fplIsStringEqual(args[1], DemoSpamArgument)) {
		int32_t spamSize = fplStringToS32(args[2]);
		RunSpamMode(spamSize);
		fplPlatformRelease();
		return(0);
	}

	// The argument array scenario starts the demo with this mode, that mode replaces the whole run as well
	if ((argc > 1) && fplIsStringEqual(args[1], DemoPrintArgumentsArgument)) {
		RunPrintArgumentsMode(argc - 2, args + 2);
		fplPlatformRelease();
		return(0);
	}

	// The separate capture and the callback scenario both start the demo with this mode
	if ((argc > 1) && fplIsStringEqual(args[1], DemoWriteStreamsArgument)) {
		RunWriteStreamsMode();
		fplPlatformRelease();
		return(0);
	}

	uint64_t currentProcessId = fplProcessGetCurrentId();
	fplConsoleFormatOut("FPL process demo, running as process id %llu\n", (unsigned long long)currentProcessId);

	if (argc > 1) {
		RunUserProgram(argc - 1, args + 1);
	} else {
		RunSimpleScenario();
		RunArgumentArrayScenario();
		RunWorkDirScenario();
		RunExitCodeScenario();
		RunMissingProgramScenario();
		RunLargeCaptureScenario();
		RunCaptureLimitScenario();
		RunSeparateCaptureScenario();
		RunOutputCallbackScenario();
		RunTimeoutScenario();
		RunAsyncScenario();
		fplConsoleOut("\nPass a program and its arguments to this demo, to run it directly\n");
	}

	fplPlatformRelease();
	return(0);
}
