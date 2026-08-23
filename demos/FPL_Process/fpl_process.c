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
#	define DemoArgumentProgram "cmd.exe"
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
#	define DemoArgumentProgram "echo"
#	define DemoWorkDirProgram "pwd"
#	define DemoWorkDirArgumentLine fpl_null
#	define DemoWorkDirPath "/tmp"
#	define DemoExitCodeProgram "sh"
#	define DemoExitCodeArgumentLine "-c \"exit 3\""
#	define DemoSleepProgram "sleep"
#	define DemoSleepArgumentLine "5"
#endif

#define DemoMissingProgram "this_program_does_not_exist_xyz"

// Milliseconds we wait in the timeout scenario, the child runs a lot longer than that
static const fplTimeoutValue demoShortWaitTimeout = 250;
// Milliseconds slept between two polls of fplProcessIsRunning()
static const uint32_t demoPollIntervalInMilliseconds = 100;
// Number of polls before the demo gives up and stops the child
static const int demoMaxPollCount = 5;

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
	context.flags = fplProcessFlags_AutoWait;
	RunAndWait("Start a program and wait for it", &context);
}

static void RunArgumentArrayScenario(void) {
	// The argument array is the exact way to pass arguments, because nothing has to be quoted or parsed
	const char *arguments[] = { "first argument with spaces", "second", "third" };
	fplProcessContext context = fplZeroInit;
	context.name = DemoArgumentProgram;
	context.arguments = arguments;
	context.argumentCount = fplArrayCount(arguments);
	context.flags = fplProcessFlags_AutoWait;
	RunAndWait("Pass arguments as an array", &context);
}

static void RunWorkDirScenario(void) {
	fplProcessContext context = fplZeroInit;
	context.name = DemoWorkDirProgram;
	context.argumentLine = DemoWorkDirArgumentLine;
	context.workDir = DemoWorkDirPath;
	context.flags = fplProcessFlags_AutoWait;
	RunAndWait("Run in a different work directory", &context);
}

static void RunExitCodeScenario(void) {
	// A non-zero exit code is a normal result, many programs use it to report a state instead of an error
	fplProcessContext contextWithoutErrorFlag = fplZeroInit;
	contextWithoutErrorFlag.name = DemoExitCodeProgram;
	contextWithoutErrorFlag.argumentLine = DemoExitCodeArgumentLine;
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

// Runs the program the user passed on the command line, so the demo can start anything
static void RunUserProgram(const int argumentCount, char **arguments) {
	const char *programName = arguments[0];
	const char **programArguments = (const char **)(arguments + 1);
	size_t programArgumentCount = (size_t)(argumentCount - 1);
	fplProcessContext context = fplZeroInit;
	context.name = programName;
	context.arguments = programArguments;
	context.argumentCount = programArgumentCount;
	context.flags = fplProcessFlags_AutoWait;
	RunAndWait("Run the program from the command line", &context);
}

int main(int argc, char *args[]) {
	if (!fplPlatformInit(fplInitFlags_All, fpl_null)) {
		return(-1);
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
		RunTimeoutScenario();
		RunAsyncScenario();
		fplConsoleOut("\nPass a program and its arguments to this demo, to run it directly\n");
	}

	fplPlatformRelease();
	return(0);
}
