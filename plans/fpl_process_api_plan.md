# Plan: Prozess-API für FPL (`fplProcess*`)

Ziel: Executables **und** Skripte starten, mit einer einzigen Signatur + einem Steuerungs-Struct, auf Windows und Linux/Unix (POSIX). Referenz-Vorbild: `System.Diagnostics.Process.Start`, aber ohne .NET-Ballast und ohne dessen Einschränkung "UseShellExecute schließt Redirect aus".

Rechte-Modell wie gewünscht: **keine** Privilege-Manipulation. Kein `runas`, kein `setuid`, kein `LOGON_WITH_PROFILE`. Der Kindprozess erbt exakt den Security-Kontext der Session (root bleibt root, User bleibt User).

---

## 1. Was am Entwurf gefehlt hat (und warum es rein muss)

Das ist der wichtigste Abschnitt: die Punkte, die im Sketch fehlen und ohne die die API entweder nicht funktioniert oder leakt.

### 1.1 Es fehlt eine Wait-Funktion
`autoWait` deckt nur den synchronen Fall ab. Startet man asynchron (`autoWait = false`), gibt es keinen Weg mehr, auf das Ende zu warten oder den Exit-Code abzuholen — `fplProcessIsRunning()` liefert nur ja/nein.
→ **`fplProcessWait(handle, timeout, outResult)`** ist Pflicht. Mit Timeout, weil ein Wait ohne Timeout die App bei einem hängenden Kind für immer blockiert (klassischer Aufhänger bei Tool-Aufrufen).

### 1.2 Es fehlt ein Close/Release für das Handle
Ein Prozess-Handle ist eine **Ressource**, kein Wert:
- Windows: `CreateProcess` liefert zwei Handles (Prozess + Primär-Thread). Beide müssen mit `CloseHandle` freigegeben werden, sonst bleibt das Kernel-Objekt (und damit ein Eintrag in der Prozess-Tabelle) am Leben, auch nachdem das Kind längst beendet ist.
- POSIX: Ein beendetes Kind bleibt **Zombie**, bis der Parent `waitpid()` macht. Ohne Release sammelt eine App, die alle 5 Sekunden ein Tool aufruft, Zombies bis zum Prozess-Limit.
- Dazu die Pipes: Kapselt man Streams, hängen Datei-Deskriptoren/Handles am Handle, die geschlossen werden müssen.

→ **`fplProcessClose(handle)`** ist Pflicht. `fplProcessStop()` ersetzt das nicht: Kill ≠ Reap.

### 1.3 `fplProcessHandle` mit nur `id` reicht technisch nicht
Auf Windows ist die PID **nicht** stabil identifizierend — PIDs werden wiederverwendet. Ein `fplProcessStop()` das über die PID ein Handle nachöffnet, kann einen fremden, zwischenzeitlich gestarteten Prozess killen. Deshalb muss das native `HANDLE` im Struct liegen. Gleiches Pattern wie `fplFileHandle`/`fplThreadHandle`: ein `fplInternalProcessHandle`-Union.
`id` (PID) bleibt drin, aber als *Informations*-Feld (Loggen, Anzeigen).

### 1.4 Es fehlt eine Free-Funktion für die Result-Buffer
Alles was FPL alloziert, braucht ein Gegenstück (Vorbild im Header: `fplWindowDropFiles.internalMemory`). → **`fplProcessFreeResult(result)`**.
**Entschieden:** `fplProcessBuffer *outputBuffer` (Zeiger) wird zu **`fplProcessBuffer output`** (by value), und das Struct bleibt öffentlich minimal (`text`, `len`) — die Buchhaltung liegt in einem **versteckten Prefix-Header direkt vor dem Text** im selben Speicherblock (Details in 7.1). Damit sieht der Nutzer nichts Internes, es gibt keine zweite Allokation und keinen Null-Check auf einem Struct-Zeiger.

### 1.5 Es fehlt eine Obergrenze für Capture
Ein Kind kann unbegrenzt auf stdout schreiben (`yes`, ein Build-Log, ein kaputtes Tool). Ohne Limit wächst der Buffer bis OOM.
→ **`maxCaptureSize`** im Context (0 = unbegrenzt) + `isTruncated` im Result.

### 1.6 Es fehlt Argument-Array (nicht nur Argument-String)
Der Sketch hat nur `const char *arguments` (eine Zeile, .NET-Stil). Das ist auf POSIX ein Problem: `execv` will `argv[]`. FPL müsste die Zeile selbst zerlegen — inklusive Quoting-Regeln — und das geht bei Pfaden mit Leerzeichen zwangsläufig irgendwann schief. Genau der Fall "Skript starten" hat oft Pfade mit Leerzeichen.
→ Beide Formen anbieten: `argumentLine` (bequem) **und** `arguments[]`/`argumentCount` (exakt, gewinnt wenn gesetzt). FPL macht die plattformrichtige Konvertierung in beide Richtungen (auf Windows argv→Kommandozeile mit korrekter Backslash/Quote-Verdopplung, auf POSIX Zeile→argv).

### 1.7 Es fehlt Shell-Unterstützung — sonst laufen "Skripte" nicht
Der Wunsch "oder sogar Skripte" ist ohne Shell-Flag nicht erfüllbar:
- Windows: `.bat`/`.cmd` sind **keine** ausführbaren Images. `CreateProcess` scheitert daran. Es braucht `cmd.exe /c`. `.ps1` braucht `powershell -File`, `.py` braucht den registrierten Interpreter (den kennt nur die Shell-Assoziation).
- POSIX: Ein Skript läuft mit `execv` nur, wenn es Shebang **und** x-Bit hat. Sonst `ENOEXEC`/`EACCES`.

→ Eigenes Feld **`fplProcessContext.shellMode`** (plus optional `shellPath`/`shellArgument`), siehe 1.7.1: Windows `cmd.exe /c <cmdline>`, POSIX `/bin/sh -c <cmdline>`.
Wichtiger Unterschied zu .NET: Da schließt `UseShellExecute = true` Redirection aus, weil .NET dann `ShellExecuteEx` benutzt. Wir benutzen `cmd /c` als normalen Child-Prozess → Capture/Redirect funktioniert weiterhin. Das ist ein echter Vorteil und sollte so dokumentiert werden.
Sicherheitshinweis in die Doku: mit Shell-Modus ist die Kommandozeile Shell-Syntax, also Command-Injection möglich, wenn man User-Eingaben reinreicht.

### 1.7.1 Warum `shellMode` (Enum + Pfad) statt `bool useShell`
Ein `bool` kann nur "die eine" Shell ausdrücken. Genau das reicht in der Praxis nicht:
- Windows: `.bat`/`.cmd` brauchen `cmd.exe /c`, `.ps1` braucht `powershell.exe -File` bzw. `pwsh -File`. Mit einem Bool kann man PowerShell-Skripte nie starten.
- POSIX: `/bin/sh` ist auf vielen Systemen `dash`, nicht `bash`. Ein Skript mit Bashismen (`[[ ]]`, Arrays) läuft mit `sh -c` schlicht nicht. Ebenso will man manchmal `python3 -c`, `perl -e`, `busybox sh`.
- Ein Bool zwingt den Nutzer sonst dazu, den Interpreter selbst in `name` zu schreiben und die Kommandozeile selbst zu quoten — also genau die fehleranfällige Arbeit, die die API abnehmen soll.

Das Enum bleibt für den einfachen Fall trotzdem eine einzige Zuweisung (`context.shellMode = fplProcessShellMode_Default;`), deckt aber den Custom-Interpreter mit ab. Deshalb liegt es als **Feld** im Context und nicht als Flag in `fplProcessFlags`: Doppelzustand (Flag *und* Pfadfeld) wäre mehrdeutig, sobald nur eins von beidem gesetzt ist.

### 1.8 Environment-Variablen — zurückgestellt
Werkzeug-Aufrufe brauchen oft ein modifiziertes Environment (`PATH`, `LANG`, `LC_ALL`, `CC`). Das wäre `environment[]` (`"KEY=VALUE"`) + `environmentCount` + ein Inherit-Flag gewesen.
→ **Bewusst auf später verschoben** (siehe 12). Das Kind erbt vorerst immer das Environment des Parents. Die Erweiterung ist additiv: zwei Felder im Context plus ein Flag, ohne Bruch an der bestehenden Signatur. Bis dahin ist der Workaround `setenv()`/`SetEnvironmentVariable()` im Parent — mit dem Nachteil, dass es nicht thread-safe ist und den eigenen Prozess dauerhaft verändert.

### 1.9 Es fehlt "kein Konsolenfenster"
Eine GUI-App, die ein Konsolen-Tool startet, lässt auf Windows ein schwarzes Fenster aufblitzen. Das ist der sichtbarste Bug einer Prozess-API überhaupt.
→ `fplProcessFlags_NoWindow` (`CREATE_NO_WINDOW`).

### 1.10 Es fehlt Prozess-Baum-Semantik
`fplProcessStop()` killt nur das direkte Kind. Mit `UseShell` ist das direkte Kind aber `cmd.exe`/`sh` — das eigentliche Tool überlebt und läuft weiter. Ebenso bei Build-Skripten mit Sub-Prozessen.
→ `fplProcessFlags_KillProcessTree`: Windows über ein **Job Object** (`JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE`), POSIX über eigene Prozessgruppe (`setsid()`/`setpgid()` im Kind, `kill(-pgid, sig)` beim Stoppen).
→ Ergänzend `fplProcessFlags_KillOnParentExit`: Kind stirbt mit dem Parent (Windows: dasselbe Job Object; Linux: `prctl(PR_SET_PDEATHSIG, SIGKILL)` im Kind; andere BSDs: `PROC_PDEATHSIG_CTL` wo vorhanden, sonst dokumentiert nicht unterstützt).

### 1.11 Es fehlt "graceful stop"
`fplProcessStop()` = hart (SIGKILL / `TerminateProcess`) ist richtig so wie beschrieben, aber hart bedeutet: keine Cleanup-Chance, halbgeschriebene Dateien, keine Flush.
→ Zusätzlich **`fplProcessRequestStop()`**: POSIX `SIGTERM`, Windows `GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT)` an die Prozessgruppe bzw. `WM_CLOSE` an Top-Level-Fenster als Fallback.

### 1.12 Es fehlt die Unterscheidung "beendet durch Signal" vs "Exit-Code"
Auf POSIX liefert `waitpid` einen Status, kein Exit-Code. Bei `WIFSIGNALED` gibt es gar keinen Exit-Code. Ohne eigenes Feld ist "durch SIGSEGV gekillt" nicht von "regulär mit 11 beendet" unterscheidbar.
→ `terminationSignal` im Result, `exitCode = 128 + signal` (Shell-Konvention) und `fplProcessResultType_Terminated`.

### 1.13 Die Result-Typen sind zu grob
`FailedToStart` ist der häufigste Fehler und hat zwei völlig verschiedene Ursachen, die der Aufrufer unterscheiden will: Datei nicht gefunden (`ENOENT`/`ERROR_FILE_NOT_FOUND`) vs. keine Ausführrechte / kein x-Bit (`EACCES`). Dazu Timeout.
→ `_NotFound`, `_AccessDenied`, `_Timeout`, `_Terminated` ergänzen, plus `nativeErrorCode` (roher `errno`/`GetLastError`) für alles andere.

### 1.14 `FailedWithExitCode` als Default ist falsch
Sehr viele Programme benutzen Exit-Codes als *Information*, nicht als Fehler (`grep` = 1 bei "nichts gefunden", `diff` = 1 bei "unterschiedlich"). Wenn FPL das als Fehler meldet, muss jeder Aufrufer das wieder ausbauen.
→ `type` bleibt `Success`, sobald der Prozess *gelaufen und normal beendet* ist — unabhängig vom Exit-Code. `FailedWithExitCode` nur, wenn der Aufrufer `fplProcessFlags_TreatNonZeroExitAsError` setzt.

### 1.15 Das Deadlock-Problem beim Capture (der eigentliche Knackpunkt)
Wenn stdout und stderr getrennt gecaptured werden und man sie **nacheinander** ausliest, blockiert das Kind, sobald die andere Pipe voll ist (64 KB Pipe-Buffer auf Linux, ähnlich auf Windows) → Deadlock, Anwendung hängt.
Genauso tödlich: erst `WaitForSingleObject`/`waitpid`, dann Pipes leeren. Beim Draining muss **gleichzeitig** gewartet werden.
Konsequenzen für das Design, siehe Abschnitt 4:
- Merged-Capture (`2>&1`) braucht nur eine Pipe und eine Leseschleife → einfachster und sicherster Fall.
- Getrenntes Capture braucht `poll()` (POSIX) bzw. `PeekNamedPipe`-Polling (Windows), oder Reader-Threads.

### 1.16 stdin-Vererbung ist ein Fallstrick
Erbt das Kind stdin des Parents (Default), kann ein Tool, das interaktiv nachfragt, in einer GUI-App **ewig** blockieren, ohne dass irgendwer die Frage sieht.
→ Es braucht explizit `fplProcessInputMode_None` (`/dev/null` bzw. `NUL`), und das sollte man in der Doku als Empfehlung für GUI-Apps nennen.

### 1.17 SIGPIPE (POSIX)
Schreibt der Parent in stdin eines bereits beendeten Kindes, bekommt **der Parent** SIGPIPE und stirbt per Default. Muss behandelt werden (siehe 6.6).

### 1.18 `STILL_ACTIVE`-Falle (Windows)
`GetExitCodeProcess` liefert `STILL_ACTIVE` (259) für "läuft noch" — ein Prozess, der tatsächlich mit 259 endet, ist davon nicht unterscheidbar. "Läuft noch" muss immer über `WaitForSingleObject(handle, 0)` bestimmt werden, nie über den 259-Vergleich.

### 1.19 Kleinigkeiten, die man später vermisst
- `fplProcessGetCurrentId()` — eigene PID, für Logs/Lockfiles/temporäre Dateinamen. Ein Dreizeiler pro Plattform.
- `fplProcessFlags_LineBuffered` — sonst baut jeder Callback-Nutzer den gleichen Zeilen-Splitter samt Chunk-Grenzen-Bug nach.
- Zeichenkodierung: FPL transkodiert **nicht**. Auf Windows liefert ein Konsolen-Tool typischerweise die OEM-Codepage, nicht UTF-8. Muss dokumentiert werden; optional `fplProcessFlags_ForceUtf8Console` (setzt `chcp 65001` bei `UseShell`) — würde ich erst mal weglassen und nur dokumentieren.

---

## 2. Öffentliche API (Vorschlag, final)

Platzierung im Header: neue `@defgroup Process` direkt nach `@defgroup DynamicLibrary` (beides "OS-Dienste", kein Window/Video/Audio-Bezug).

```c
// ----------------------------------------------------------------------------
/**
* @defgroup Process Process functions
* @brief This category contains functions for starting and controlling child processes
* @{
*/
// ----------------------------------------------------------------------------

// Forward declaration; the documented definition lives below.
typedef struct fplProcessHandle fplProcessHandle;

/**
* @enum fplProcessStreamType
* @brief An enumeration of process stream types.
*/
typedef enum fplProcessStreamType {
	//! No stream.
	fplProcessStreamType_None = 0,
	//! The standard-output stream.
	fplProcessStreamType_Output,
	//! The standard-error stream.
	fplProcessStreamType_Error,
} fplProcessStreamType;

/**
* @def FPL_FUNC_PROCESS_OUTPUT
* @brief A function definition for receiving redirected output/error text from a process.
*/
#define FPL_FUNC_PROCESS_OUTPUT(name) void name(const fplProcessHandle *process, const fplProcessStreamType stream, const char *text, const size_t textLen, void *userData)
//! A callback that receives redirected output/error text. The text is always null-terminated.
typedef FPL_FUNC_PROCESS_OUTPUT(fpl_process_output_callback);

/**
* @def FPL_FUNC_PROCESS_INPUT
* @brief A function definition for providing standard-input text for a process.
*/
#define FPL_FUNC_PROCESS_INPUT(name) size_t name(const fplProcessHandle *process, char *targetBuffer, const size_t maxTargetBufferLen, void *userData)
//! A callback that fills the target buffer with the next input chunk. Returning zero closes the standard-input (EOF).
typedef FPL_FUNC_PROCESS_INPUT(fpl_process_input_callback);

/**
* @enum fplProcessCaptureFlags
* @brief An enumeration of capture/redirect flags for the output/error streams.
*/
typedef enum fplProcessCaptureFlags {
	//! Do not capture anything, the child inherits the streams of the parent.
	fplProcessCaptureFlags_None = 0,
	//! Capture the standard-output stream.
	fplProcessCaptureFlags_Output = 1 << 0,
	//! Capture the standard-error stream.
	fplProcessCaptureFlags_Error = 1 << 1,
	//! Store the captured text in the buffers of @ref fplProcessResult.
	fplProcessCaptureFlags_ToBuffer = 1 << 2,
	//! Push the captured text into the @ref fpl_process_output_callback.
	fplProcessCaptureFlags_ToCallback = 1 << 3,
	//! Merge the error stream into the output stream, the same as "2>&1".
	fplProcessCaptureFlags_Merged = 1 << 4,

	//! Both streams.
	fplProcessCaptureFlags_Both = fplProcessCaptureFlags_Output | fplProcessCaptureFlags_Error,
	//! Capture only the output stream into the output buffer.
	fplProcessCaptureFlags_CaptureOutput = fplProcessCaptureFlags_Output | fplProcessCaptureFlags_ToBuffer,
	//! Capture only the error stream into the error buffer.
	fplProcessCaptureFlags_CaptureError = fplProcessCaptureFlags_Error | fplProcessCaptureFlags_ToBuffer,
	//! Capture the output stream into the output buffer and the error stream into the error buffer.
	fplProcessCaptureFlags_CaptureSeparate = fplProcessCaptureFlags_Both | fplProcessCaptureFlags_ToBuffer,
	//! Capture both streams merged into the output buffer.
	fplProcessCaptureFlags_CaptureBoth = fplProcessCaptureFlags_Both | fplProcessCaptureFlags_ToBuffer | fplProcessCaptureFlags_Merged,
	//! Redirect only the output stream to the callback.
	fplProcessCaptureFlags_RedirectOutput = fplProcessCaptureFlags_Output | fplProcessCaptureFlags_ToCallback,
	//! Redirect only the error stream to the callback.
	fplProcessCaptureFlags_RedirectError = fplProcessCaptureFlags_Error | fplProcessCaptureFlags_ToCallback,
	//! Redirect both streams merged to the callback.
	fplProcessCaptureFlags_RedirectBoth = fplProcessCaptureFlags_Both | fplProcessCaptureFlags_ToCallback | fplProcessCaptureFlags_Merged,
} fplProcessCaptureFlags;
FPL_ENUM_AS_FLAGS_OPERATORS(fplProcessCaptureFlags);

/**
* @enum fplProcessInputMode
* @brief An enumeration of standard-input modes.
*/
typedef enum fplProcessInputMode {
	//! The child inherits the standard-input of the parent.
	fplProcessInputMode_Inherit = 0,
	//! The child gets an immediate end-of-file on the standard-input.
	fplProcessInputMode_None,
	//! Write the text from @ref fplProcessContext.inputText into the standard-input and close it afterwards.
	fplProcessInputMode_Text,
	//! Pull the standard-input in chunks from the @ref fpl_process_input_callback until it returns zero.
	fplProcessInputMode_Callback,
	//! Keep the standard-input open, so the caller can write with @ref fplProcessWriteInput().
	fplProcessInputMode_Stream,
} fplProcessInputMode;

/**
* @enum fplProcessShellMode
* @brief An enumeration of shell execution modes.
*/
typedef enum fplProcessShellMode {
	//! Execute the process directly, without any shell.
	fplProcessShellMode_None = 0,
	//! Execute the command line through the default system shell (cmd.exe on Windows, /bin/sh on POSIX).
	fplProcessShellMode_Default,
	//! Execute the command line through the shell or interpreter from @ref fplProcessContext.shellPath.
	fplProcessShellMode_Custom,
} fplProcessShellMode;

/**
* @enum fplProcessFlags
* @brief An enumeration of process creation/control flags.
*/
typedef enum fplProcessFlags {
	//! No flags.
	fplProcessFlags_None = 0,
	//! Wait for the process to be terminated, before returning from @ref fplProcessStart().
	fplProcessFlags_AutoWait = 1 << 0,
	//! Do not create a console window for the child process (Windows only).
	fplProcessFlags_NoWindow = 1 << 1,
	//! Detach the child, so it survives the exit of the parent.
	fplProcessFlags_Detached = 1 << 2,
	//! Stop the entire process tree instead of the direct child only.
	fplProcessFlags_KillProcessTree = 1 << 3,
	//! Kill the child automatically, when the parent exits.
	fplProcessFlags_KillOnParentExit = 1 << 4,
	//! Split the captured/redirected text into complete lines.
	fplProcessFlags_LineBuffered = 1 << 5,
	//! Report a non-zero exit code as @ref fplProcessResultType_FailedWithExitCode.
	fplProcessFlags_TreatNonZeroExitAsError = 1 << 6,
} fplProcessFlags;
FPL_ENUM_AS_FLAGS_OPERATORS(fplProcessFlags);

/**
* @struct fplProcessContext
* @brief Stores the start parameters for @ref fplProcessStart().
*/
typedef struct fplProcessContext {
	//! Filename or path of the executable or script to run.
	const char *name;
	//! The full argument line, ignored when @ref fplProcessContext.arguments is set.
	const char *argumentLine;
	//! The arguments, without the executable itself.
	const char **arguments;
	//! The work directory path or fpl_null for using the current directory.
	const char *workDir;
	//! The path of the shell or interpreter, used for @ref fplProcessShellMode_Custom (for example "/bin/bash" or "powershell.exe").
	const char *shellPath;
	//! The argument that tells the custom shell to execute the command line or fpl_null for using the default argument ("-c" on POSIX, "/c" on Windows).
	const char *shellArgument;
	//! The text written into the standard-input, used for @ref fplProcessInputMode_Text.
	const char *inputText;
	//! Callback that receives the redirected output/error text.
	fpl_process_output_callback *outputCallback;
	//! Callback that provides the standard-input text, used for @ref fplProcessInputMode_Callback.
	fpl_process_input_callback *inputCallback;
	//! The user data passed to the callbacks.
	void *userData;
	//! Number of arguments.
	size_t argumentCount;
	//! Number of characters in @ref fplProcessContext.inputText or zero for computing the length automatically.
	size_t inputTextLen;
	//! Maximum number of bytes captured per buffer or zero for no limit.
	size_t maxCaptureSize;
	//! Output/Error capture mode.
	fplProcessCaptureFlags captureFlags;
	//! Standard-input mode.
	fplProcessInputMode inputMode;
	//! Shell execution mode.
	fplProcessShellMode shellMode;
	//! Process creation/control flags.
	fplProcessFlags flags;
	//! Timeout in milliseconds for @ref fplProcessFlags_AutoWait or zero for waiting infinitely.
	uint32_t waitTimeout;
} fplProcessContext;

/**
* @enum fplProcessResultType
* @brief An enumeration of process result types.
*/
typedef enum fplProcessResultType {
	//! Unknown result or error.
	fplProcessResultType_Unknown = -1,
	//! The process was started and has exited normally, regardless of the exit code.
	fplProcessResultType_Success = 0,
	//! Any argument passed to a fplProcess* function is invalid (not the arguments of the process!).
	fplProcessResultType_InvalidArguments,
	//! The executable or script was not found.
	fplProcessResultType_NotFound,
	//! The executable or script is not allowed to be executed.
	fplProcessResultType_AccessDenied,
	//! The process failed to start.
	fplProcessResultType_FailedToStart,
	//! The process exited with a non-zero exit code and @ref fplProcessFlags_TreatNonZeroExitAsError was set.
	fplProcessResultType_FailedWithExitCode,
	//! The process was terminated by a signal (POSIX only).
	fplProcessResultType_Terminated,
	//! The process did not exit within the timeout.
	fplProcessResultType_Timeout,
	//! Not enough memory available.
	fplProcessResultType_OutOfMemory,
} fplProcessResultType;

/**
* @struct fplProcessBuffer
* @brief Stores a captured text block, allocated by FPL and released by @ref fplProcessFreeResult().
*/
typedef struct fplProcessBuffer {
	//! The null-terminated text or fpl_null when nothing was captured.
	char *text;
	//! The number of characters, without the null-terminator.
	size_t len;
} fplProcessBuffer;

/**
* @struct fplProcessResult
* @brief Stores the result of a process start/wait.
*/
typedef struct fplProcessResult {
	//! The captured standard-output.
	fplProcessBuffer output;
	//! The captured standard-error.
	fplProcessBuffer error;
	//! The raw exit code from the process.
	int32_t exitCode;
	//! The signal number that has terminated the process or zero (POSIX only).
	int32_t terminationSignal;
	//! The raw error code from the operating system (errno or GetLastError).
	uint32_t nativeErrorCode;
	//! The result type.
	fplProcessResultType type;
	//! The process has exited normally.
	fpl_b32 hasExited;
	//! The captured text was truncated, because @ref fplProcessContext.maxCaptureSize was exceeded.
	fpl_b32 isTruncated;
} fplProcessResult;

/**
* @union fplInternalProcessHandle
* @brief A union containing the internal process handle for any platform.
*/
typedef union fplInternalProcessHandle {
#if defined(FPL_PLATFORM_WINDOWS)
	//! Win32 specifics.
	struct {
		//! The process handle.
		fpl__Win32Handle processHandle;
		//! The handle of the primary thread.
		fpl__Win32Handle threadHandle;
		//! The job object handle, used for the process tree control.
		fpl__Win32Handle jobHandle;
	} win32;
#elif defined(FPL_SUBPLATFORM_POSIX)
	//! POSIX specifics.
	struct {
		//! The process id.
		int32_t pid;
		//! The process group id.
		int32_t pgid;
	} posix;
#endif
} fplInternalProcessHandle;

/**
* @struct fplProcessHandle
* @brief The process handle structure.
*/
typedef struct fplProcessHandle {
	//! The internal process handle.
	fplInternalProcessHandle internalHandle;
	//! The internal stream state, do not touch.
	struct fpl__ProcessStreams *streams;
	//! The id of the process.
	uint64_t id;
	//! The cached exit code, valid when the process has exited.
	int32_t exitCode;
	//! The process handle is valid.
	fpl_b32 isValid;
	//! The process has exited and the exit code is cached.
	fpl_b32 hasExited;
} fplProcessHandle;

/**
* @brief Starts a process with the specified context.
*/
fpl_platform_api bool fplProcessStart(const fplProcessContext *context, fplProcessHandle *outHandle, fplProcessResult *outResult);
/**
* @brief Pumps the redirected streams of the specified process without blocking.
*/
fpl_platform_api bool fplProcessUpdate(fplProcessHandle *handle);
/**
* @brief Waits for the specified process to be terminated, while pumping the redirected streams.
*/
fpl_platform_api bool fplProcessWait(fplProcessHandle *handle, const uint32_t timeout, fplProcessResult *outResult);
/**
* @brief Is the specified process still running?
*/
fpl_platform_api bool fplProcessIsRunning(fplProcessHandle *handle);
/**
* @brief Gets the exit code from the specified process, when it has exited already.
*/
fpl_platform_api bool fplProcessTryGetExitCode(fplProcessHandle *handle, int32_t *outExitCode);
/**
* @brief Writes the specified text into the standard-input of the process, used for @ref fplProcessInputMode_Stream.
*/
fpl_platform_api size_t fplProcessWriteInput(fplProcessHandle *handle, const char *text, const size_t textLen);
/**
* @brief Closes the standard-input of the process, so the process gets an end-of-file.
*/
fpl_platform_api void fplProcessCloseInput(fplProcessHandle *handle);
/**
* @brief Asks the process to stop gracefully.
*/
fpl_platform_api bool fplProcessRequestStop(const fplProcessHandle *handle);
/**
* @brief Tries to forcefully stop the process from the specified handle.
*/
fpl_platform_api bool fplProcessStop(const fplProcessHandle *handle);
/**
* @brief Releases all resources of the specified process handle.
*/
fpl_platform_api void fplProcessClose(fplProcessHandle *handle);
/**
* @brief Releases the captured buffers from the specified result.
*/
fpl_common_api void fplProcessFreeResult(fplProcessResult *result);
/**
* @brief Gets the id of the current process.
*/
fpl_platform_api uint64_t fplProcessGetCurrentId(void);

/** @} */
```

### Warum Flags-Enum statt ordinalem `fplProcessCaptureMode`
Der Wunsch war "Callback **oder** String-Block **oder beides**". Mit einem ordinalen Enum bräuchte man dafür Werte wie `CaptureAndRedirectBoth` — bei 2 Streams × 2 Senken × merged/separat sind das 12+ Werte. Als Flags-Enum ist alles kombinierbar, und die **Preset-Werte heißen genau wie im Sketch** (`fplProcessCaptureFlags_CaptureOutput`, `_RedirectBoth`, ...), d.h. der einfache Fall bleibt eine einzige Zuweisung:

```c
context.captureFlags = fplProcessCaptureFlags_CaptureBoth;                                    // wie im Sketch
context.captureFlags = fplProcessCaptureFlags_CaptureBoth | fplProcessCaptureFlags_ToCallback; // Buffer UND Callback
```
FPL hat dafür Präzedenz: `fplInitFlags`, `fplLogWriterFlags`, `fplInputSourceType` — inkl. `FPL_ENUM_AS_FLAGS_OPERATORS` für C++.
Falls das ordinale Enum trotzdem bevorzugt wird: dann fehlt zwingend ein Wert `CaptureSeparate` (stdout→output, stderr→error), sonst lassen sich beide Streams nicht getrennt einsammeln.

---

## 3. Anwendungsbeispiele (Zielbild)

**Der Ein-Zeiler (synchron, alles einsammeln):**
```c
fplProcessContext context = fplZeroInit;
context.name = "git";
context.argumentLine = "rev-parse --short HEAD";
context.captureFlags = fplProcessCaptureFlags_CaptureBoth;
context.flags = fplProcessFlags_AutoWait;

fplProcessHandle handle = fplZeroInit;
fplProcessResult result = fplZeroInit;
if (fplProcessStart(&context, &handle, &result)) {
	fplConsoleOut(result.output.text);
}
fplProcessFreeResult(&result);
fplProcessClose(&handle);
```

**Skript starten (plattformneutral):**
```c
fplProcessContext context = fplZeroInit;
context.name = "./build.sh";                      // auf Windows z.B. "build.bat"
context.workDir = "/home/final/_projects/fpl";
context.shellMode = fplProcessShellMode_Default;   // cmd.exe /c   bzw.   /bin/sh -c
context.flags = fplProcessFlags_AutoWait | fplProcessFlags_NoWindow;
context.captureFlags = fplProcessCaptureFlags_CaptureBoth;
```

**Asynchron mit Live-Ausgabe (Pump-Modell, Callback läuft im Aufrufer-Thread):**
```c
static FPL_FUNC_PROCESS_OUTPUT(OnProcessOutput) {
	fplConsoleOut(text);
}

fplProcessContext context = fplZeroInit;
context.name = "cmake";
context.argumentLine = "--build .";
context.captureFlags = fplProcessCaptureFlags_RedirectBoth;
context.flags = fplProcessFlags_LineBuffered;
context.outputCallback = OnProcessOutput;

fplProcessHandle handle = fplZeroInit;
fplProcessResult startResult = fplZeroInit;
if (fplProcessStart(&context, &handle, &startResult)) {
	while (fplProcessIsRunning(&handle)) {
		fplProcessUpdate(&handle);   // nicht-blockierend, feuert die Callbacks
		DoOtherFrameWork();
	}
	fplProcessResult exitResult = fplZeroInit;
	fplProcessWait(&handle, 0, &exitResult);
	fplProcessClose(&handle);
}
```

---

## 4. Capture/Redirect-Design im Detail

### 4.1 Kein interner Thread als Default — Pump-Modell
Zwei Möglichkeiten für asynchrones Lesen:

| | Interner Reader-Thread | Pump (`fplProcessUpdate`) |
|---|---|---|
| Callback-Thread | FPL-Thread | Aufrufer-Thread |
| Locking nötig | ja (Buffer, Handle-State) | nein |
| Reentrancy-Regeln für User-Callback | müssen dokumentiert+eingehalten werden | trivial |
| Latenz | sofort | vom Pump-Intervall abhängig |
| Passt zu FPL | mittel | ja (`fplPollEvents`-Stil) |

→ **Empfehlung: Pump-Modell als einziger Mechanismus** in v1. Der Nutzer ruft `fplProcessUpdate()` in seiner Schleife, Callbacks feuern garantiert im Aufrufer-Thread, es gibt keinerlei Thread-Safety-Vertrag zu erklären. Ein `fplProcessFlags_AsyncReaderThread` kann später ergänzt werden, ohne die API zu brechen.
Im synchronen Fall (`AutoWait`) pumpt `fplProcessStart` intern in der Warteschleife — sonst genau der Deadlock aus 1.15.

### 4.2 Merged vs. Separat
- `Merged`: eine Pipe, beide Enden des Kindes (`stdout`, `stderr`) zeigen auf dieselbe Write-Seite. Eine Leseschleife, kein Multiplexing, keine Reihenfolge-Probleme. Das ist der robusteste Modus und sollte in der Doku empfohlen werden.
- `Separat`: zwei Pipes. POSIX über `poll()` auf beide FDs. Windows über `PeekNamedPipe` + `ReadFile` je Pipe (siehe 5.4).

### 4.3 Buffer-Wachstum
Capture-Buffer wächst geometrisch (Start 4 KB, Verdopplung, Cap = `maxCaptureSize`), Allokation über `fpl__AllocateDynamicMemory()` — damit greifen die `fplMemorySettings`-Callbacks des Nutzers. Bei Erreichen des Caps: weiterlesen, aber verwerfen (**nicht** abbrechen — Pipe muss weiter geleert werden, sonst blockiert das Kind), `isTruncated = true`.
Das gecapturete Textende bekommt immer einen Null-Terminator (`len + 1` alloziert), damit `result.output.text` direkt an `printf`/`fplConsoleOut` gereicht werden kann.

### 4.4 Zeilenpufferung
Bei `fplProcessFlags_LineBuffered` hält FPL pro Stream einen Rest-Buffer und ruft den Callback nur mit vollständigen Zeilen (`\n`, `\r\n` normalisiert auf `\n`, ohne Zeilenende im Text). Beim Stream-Ende wird ein evtl. unvollständiger Rest als letzte Zeile ausgeliefert. Max. Zeilenlänge begrenzt (z.B. 64 KB), danach wird hart gesplittet — sonst ist es ein unbegrenzter Puffer mit Angriffsfläche.

### 4.5 Kodierung
FPL reicht rohe Bytes durch, kein Transcoding. In die Doku: auf Windows liefert ein Konsolen-Tool i.d.R. die aktive OEM-Codepage; wer UTF-8 will, setzt es im Kind (`chcp 65001` bzw. Tool-Option).

---

## 5. Windows-Implementierung

Abschnitt `// > WIN32_PROCESS` in der Win32-Plattform-Sektion. **Kein neues Runtime-Linking nötig**: `CreateProcessW`, `CreatePipe`, `ReadFile`, `WriteFile`, `PeekNamedPipe`, `WaitForSingleObject`, `TerminateProcess`, Job-Objects sind alle in kernel32, und FPL ruft kernel32 bereits direkt auf (`CreateFileW` in `fplFileOpenBinary`). Nur wenn wir `ShellExecuteEx` wollten, käme shell32 ins Spiel — brauchen wir dank `cmd /c` nicht.

### 5.1 Kommandozeile bauen
- `name` + `arguments[]` → eine Kommandozeile mit den MSVCRT-Quoting-Regeln: Argument quoten, wenn es Space/Tab/Quote enthält; Backslashes **vor** einem Quote verdoppeln; abschließende Backslashes vor dem schließenden Quote verdoppeln. (Der Klassiker, an dem selbstgebaute Implementierungen scheitern.)
- `fplProcessShellMode_Default`: `cmd.exe /c "<cmdline>"`, wobei `cmd.exe` aus der Umgebungsvariable `ComSpec` kommt (Fallback `cmd.exe`, dann PATH-Suche).
- `fplProcessShellMode_Custom`: `<shellPath> <shellArgument|"/c"> "<cmdline>"`. Achtung `cmd.exe`-Eigenheit: bei `/c "..."` frisst `cmd` unter bestimmten Bedingungen die äußeren Quotes; robuster ist `cmd.exe /s /c "<cmdline>"`, das dokumentiert die äußeren Quotes exakt einmal entfernt. Wir benutzen `/s /c`.
- UTF-8 → UTF-16 über `fplUTF8StringToWideString()`. Wichtig: `lpCommandLine` von `CreateProcessW` muss **beschreibbar** sein → in eigenen Scratch-Buffer kopieren, kein String-Literal.

### 5.2 Pipes
- `CreatePipe` mit `SECURITY_ATTRIBUTES{ bInheritHandle = TRUE }`.
- Die **Parent-Seite** danach per `SetHandleInformation(h, HANDLE_FLAG_INHERIT, 0)` nicht-vererbbar machen. Ohne das behält das Kind die Write-Seite offen → EOF kommt nie → Leseschleife hängt für immer.
- Die Child-Seite nach `CreateProcess` im Parent sofort schließen (gleicher Grund).

### 5.3 STARTUPINFO
- `STARTF_USESTDHANDLES` setzen und **alle drei** Handles füllen. Setzt man nur `hStdOutput` und lässt die anderen auf NULL, bekommt das Kind ungültige Handles — Tools, die stdin/stderr anfassen, brechen dann mit obskuren Fehlern ab. Für nicht umgeleitete Streams: `GetStdHandle(...)` bzw. bei `fplProcessInputMode_None` ein Handle auf `NUL` (`CreateFileW(L"NUL", ...)`).
- `dwCreationFlags`: `CREATE_NO_WINDOW` (NoWindow), `CREATE_NEW_PROCESS_GROUP` (für CTRL_BREAK bei RequestStop), `DETACHED_PROCESS` (Detached), `EXTENDED_STARTUPINFO_PRESENT` (siehe 5.5).
- `lpCurrentDirectory` = `workDir` (wide).

### 5.4 Nicht-blockierendes Lesen
Anonyme Pipes unterstützen **kein** Overlapped-IO. `ReadFile` blockiert also, sobald nichts anliegt. Deshalb:
`PeekNamedPipe(readHandle, fpl_null, 0, fpl_null, &availableBytes, fpl_null)` → nur wenn `availableBytes > 0`, dann `ReadFile` mit exakt so vielen Bytes. Funktioniert auch auf anonymen Pipes.
EOF-Erkennung: `ReadFile` liefert `FALSE` mit `ERROR_BROKEN_PIPE`, oder `PeekNamedPipe` schlägt mit `ERROR_BROKEN_PIPE` fehl.
(Alternative für echtes Async: manuell erzeugte Named Pipes mit `FILE_FLAG_OVERLAPPED` — deutlich mehr Code, nur nötig wenn wir später Reader-Threads/echtes Async wollen. Für v1 nicht.)

### 5.5 Handle-Vererbung eingrenzen
`bInheritHandles = TRUE` vererbt **alle** vererbbaren Handles des Parents an das Kind — inklusive Sockets, geöffneter Dateien anderer Threads. Folge: das Kind hält z.B. einen Port belegt, obwohl der Parent den Socket längst geschlossen hat.
→ `InitializeProcThreadAttributeList` + `PROC_THREAD_ATTRIBUTE_HANDLE_LIST` mit exakt unseren Pipe-Handles, `EXTENDED_STARTUPINFO_PRESENT`. Vista+ (also überall verfügbar). Fallback auf die einfache Variante, wenn die Attributliste fehlschlägt.

### 5.6 Warten, Exit-Code, Stop
- Läuft noch: `WaitForSingleObject(processHandle, 0) == WAIT_TIMEOUT`. **Nicht** über `STILL_ACTIVE` (1.18).
- Exit-Code: `GetExitCodeProcess` erst *nachdem* der Wait Signalisierung meldet, dann in `handle.exitCode` cachen.
- `fplProcessWait` mit Timeout: `WaitForSingleObject` in kleinen Scheiben (z.B. 15 ms), dazwischen Streams pumpen; oder `WaitForMultipleObjects` sobald wir Named Pipes mit Events hätten. Für v1 reicht Slice-Wait + Peek.
- `fplProcessStop`: `TerminateProcess(handle, 1)`; mit `KillProcessTree` stattdessen `CloseHandle(jobHandle)` (Job mit `JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE`) bzw. `TerminateJobObject`.
- `fplProcessRequestStop`: `GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, processGroupId)` — geht nur mit `CREATE_NEW_PROCESS_GROUP`; sonst `false` mit Warnung.
- `fplProcessClose`: Pipes schließen, `CloseHandle` auf Prozess-/Thread-/Job-Handle, Struct nullen.

### 5.7 Environment
Zurückgestellt (1.8). `CreateProcessW` bekommt `lpEnvironment = fpl_null`, das Kind erbt damit das Environment des Parents. `CREATE_UNICODE_ENVIRONMENT` wird erst gesetzt, wenn wir einen eigenen Block bauen.

---

## 6. POSIX-Implementierung (Linux/Unix)

Abschnitt `// > POSIX_PROCESS` in der POSIX-Subplattform. Zusätzliche Includes: `<sys/wait.h>` (waitpid, WIFEXITED), `<poll.h>` (poll), `<spawn.h>` nur falls wir doch posix_spawn nehmen, `<sys/prctl.h>` (Linux, nur für `KillOnParentExit`). `signal.h`, `unistd.h`, `fcntl.h`, `errno.h` sind bereits da.

### 6.1 `fork()` + `execv*` statt `posix_spawn`
Grund: wir brauchen im Kind `chdir`, `dup2`, `setsid`/`setpgid`, `prctl`, das Schließen fremder FDs **und** einen sauberen Fehlerkanal. `posix_spawn` kann davon vieles über File-Actions, aber nicht alles portabel, und die Fehlermeldung ist schlechter.

### 6.2 Die Regel für alles zwischen fork und exec
In einem multithreaded Prozess darf das Kind nach `fork()` nur **async-signal-safe** Funktionen aufrufen. Kein `malloc`, kein `snprintf`, kein FPL-Logging (das alloziert/lockt). Sonst Deadlock, wenn ein anderer Thread beim Fork gerade den Allocator-Lock hielt.
→ **Alle** Strings (Pfad, `argv[]`, `envp[]`, workDir) werden **vor** dem Fork fertig gebaut. Im Kind laufen nur: `dup2`, `close`, `chdir`, `setsid`/`setpgid`, `prctl`, `sigprocmask`, `execv`/`execvp`, `write`, `_exit`.

### 6.3 Der exec-Fehler-Kanal (Pflicht)
`fork()` gelingt immer, auch wenn die Datei gar nicht existiert — der Fehler entsteht erst im `execv` im Kind. Ohne Rückkanal kann der Parent "Start fehlgeschlagen" nicht von "gestartet und mit 127 beendet" unterscheiden.
→ Zusätzliche Pipe mit `FD_CLOEXEC` auf der Write-Seite:
- exec erfolgreich → Kernel schließt den FD → Parent liest 0 Bytes → Start OK.
- exec fehlgeschlagen → Kind schreibt `errno` (4 Bytes) und `_exit(127)` → Parent liest den Wert und mappt: `ENOENT` → `_NotFound`, `EACCES`/`EPERM` → `_AccessDenied`, `ENOEXEC` → `_FailedToStart` (mit Hinweis auf `UseShell`), sonst `_FailedToStart` + `nativeErrorCode`.

### 6.4 FD-Hygiene
- Parent-Seiten aller Pipes mit `FD_CLOEXEC` (bzw. `pipe2(O_CLOEXEC)` wo verfügbar) markieren, damit sie nicht in weitere Kinder lecken.
- Im Kind nach `dup2(readFd, STDIN_FILENO)` etc. **beide** Originale schließen.
- Im Parent die Child-Seiten sofort nach dem Fork schließen. Bleibt eine Write-Seite im Parent offen, kommt nie EOF und `fplProcessWait` hängt ewig.
- `fplProcessInputMode_None` → `open("/dev/null", O_RDONLY)` auf stdin dup'en.

### 6.5 Shell und Skripte
- `fplProcessShellMode_Default` → `execl("/bin/sh", "sh", "-c", commandLine, (char *)NULL)`.
- `fplProcessShellMode_Custom` → `execv(shellPath, {shellPath, shellArgument ? shellArgument : "-c", commandLine, NULL})`, also z.B. `/bin/bash -c ...` oder `/usr/bin/python3 -c ...`. Damit sind Bashismen und fremde Interpreter abgedeckt, ohne dass der Nutzer selbst quoten muss.
- In beiden Shell-Modi wird `name` + Argumente vorher zu **einer** Kommandozeile zusammengesetzt (mit POSIX-Shell-Quoting: einfache Anführungszeichen, enthaltene `'` als `'\''` escaped), weil `sh -c` genau ein Kommando-String erwartet.
- Ohne Shell: `execvp` wenn `name` **keinen** Slash enthält (PATH-Suche, wie .NET/Shell es tun), sonst `execv` mit dem Pfad direkt. Das ist ein bewusster Unterschied und gehört dokumentiert.

### 6.6 SIGPIPE
Schreiben in stdin eines toten Kindes → SIGPIPE → Parent stirbt (Default-Disposition). Optionen:
1. `send(MSG_NOSIGNAL)` — geht nur bei Sockets, nicht bei Pipes.
2. `SIGPIPE` prozessweit ignorieren — globaler Nebeneffekt.
3. Signal nur im schreibenden Thread per `pthread_sigmask` blocken und nach dem Write mit `sigtimedwait` abholen — korrekt, aber Linux-lastig und fummelig.

→ **Empfehlung:** beim ersten `fplProcessStart` mit stdin-Redirect prüfen, ob die aktuelle Disposition `SIG_DFL` ist; nur dann auf `SIG_IGN` setzen und das per `FPL__WARNING` loggen. Eine vom Nutzer bereits gesetzte Disposition wird **nie** überschrieben. Zusätzlich in die Doku. `EPIPE` aus `write()` wird als "Kind hat stdin geschlossen" behandelt, nicht als Fehler.

### 6.7 Warten, Zombies, Exit-Code
- `fplProcessIsRunning` → `waitpid(pid, &status, WNOHANG)`. Liefert `pid` → Kind ist beendet: Status **sofort im Handle cachen** (`hasExited`, `exitCode`, Signal). Ein zweiter `waitpid` liefert `ECHILD`, der Status wäre dann für immer weg.
- Status auswerten: `WIFEXITED` → `WEXITSTATUS`; `WIFSIGNALED` → `terminationSignal = WTERMSIG(status)`, `exitCode = 128 + signal`, `type = _Terminated`.
- `ECHILD` behandeln: Wenn die App `SIGCHLD` auf `SIG_IGN` gesetzt hat, reapt der Kernel automatisch und `waitpid` schlägt fehl. Dann melden wir "beendet, Exit-Code unbekannt" (`hasExited = true`, `exitCode = 0`, Warnung) statt einen Fehler zu werfen.
- Timeout-Wait: `waitpid` kann portabel nicht mit Timeout. Umsetzung: Schleife aus `poll()` auf die Capture-FDs (Timeout-Scheiben) + `waitpid(WNOHANG)`; ohne Capture-FDs kurze `fplThreadSleep`-Scheiben (1 ms). Linux-Optimierung optional später: `pidfd_open()` (5.3+) und den FD mit in `poll()` hängen — sauber und ohne Polling, mit Fallback.

### 6.8 Stop / Prozessgruppe
- Kind ruft `setpgid(0, 0)` (bzw. `setsid()` bei `Detached`), damit es eine eigene Prozessgruppe hat.
- `fplProcessStop`: `kill(pid, SIGKILL)`, mit `KillProcessTree` `kill(-pgid, SIGKILL)`.
- `fplProcessRequestStop`: dasselbe mit `SIGTERM`.
- `KillOnParentExit`: im Kind `prctl(PR_SET_PDEATHSIG, SIGKILL)` (Linux). Auf anderen Unixen nicht portabel → dokumentieren, dass das Flag dort ignoriert wird (mit `FPL__WARNING`).

### 6.9 Environment
Zurückgestellt (1.8). Es wird `execv`/`execvp` benutzt (kein `execve`), das Kind erbt damit `environ` des Parents unverändert.

---

## 7. Interne Strukturen

### 7.1 Der versteckte Prefix-Header der Capture-Buffer
`fplProcessBuffer` bleibt öffentlich auf `text` + `len` beschränkt. Alles was FPL zum Wachsen und Freigeben braucht, liegt **vor** dem Text im selben Block:

```c
// Layout eines Capture-Buffers:
//   [fpl__ProcessBufferHeader][text ... '\0']
//    ^ allocPtr                ^ buffer->text
typedef struct fpl__ProcessBufferHeader {
	//! Magic value, used to detect a foreign or already released buffer.
	uint64_t magic;
	//! Number of bytes usable for the text, without the null-terminator.
	size_t capacity;
	//! Total number of bytes of the entire block.
	size_t totalSize;
} fpl__ProcessBufferHeader;

#define FPL__PROCESS_BUFFER_MAGIC 0x50524F4342554653ULL  // "PROCBUFS"
#define FPL__PROCESS_BUFFER_ALIGNMENT 16
```

Regeln:
- Allokation: `allocPtr = fpl__AllocateDynamicMemory(sizeof(fpl__ProcessBufferHeader) + capacity + 1, FPL__PROCESS_BUFFER_ALIGNMENT)`, danach `text = (char *)allocPtr + sizeof(fpl__ProcessBufferHeader)`.
- Freigabe: `header = (fpl__ProcessBufferHeader *)((uint8_t *)buffer->text - sizeof(fpl__ProcessBufferHeader))`, Magic prüfen, dann `fpl__ReleaseDynamicMemory(header)` und `buffer` nullen. Doppelter Aufruf ist damit erkennbar und wird ignoriert (idempotent, wie `fplFileClose`).
- **Allokator-Regel:** Es wird ausschließlich `fpl__AllocateDynamicMemory()` / `fpl__ReleaseDynamicMemory()` benutzt, nie `fplMemoryAlignedAllocate()`/`fplMemoryAlignedFree()` oder `fplMemoryAllocate()` direkt. Nur so greifen die `fplMemorySettings.dynamic`-Callbacks des Nutzers, und der eigene Allocator bekommt jede Prozess-Allokation zu sehen. Das gilt für die Capture-Buffer, die Zeilen-Buffer, `fpl__ProcessStreams` und die vor dem Fork gebauten argv-Blöcke.
- **Deshalb ist der zurückgegebene Zeiger opak:** Wo der Speicher wirklich beginnt, weiß nur der jeweilige Allokator. Der Default-Pfad landet in `fplMemoryAlignedAllocate()`, das intern *bereits* denselben Prefix-Trick benutzt (Basiszeiger vor dem ausgerichteten Zeiger), und ein benutzerdefinierter Callback kann beliebige eigene Buchhaltung davor legen. Unser Header muss deshalb **auf** dem gelieferten Zeiger liegen — niemals davor gerechnet werden — und freigegeben wird immer exakt der Zeiger, den der Allokator zurückgegeben hat.
- Wachstum: FPL hat kein `realloc`. Beim Vergrößern wird ein neuer Block alloziert, `header + text` per `fplMemoryCopy` übernommen und der alte Block freigegeben. Startkapazität 4 KB, danach Verdopplung, gedeckelt durch `maxCaptureSize`.
- `text` ist immer null-terminiert (deshalb das `+ 1`), auch wenn `len == 0`. `text` ist nur dann `fpl_null`, wenn der Stream gar nicht gecaptured wurde.

### 7.2 Stream-State

```c
typedef struct fpl__ProcessStream {
	fplProcessBuffer capture;       // wächst über den Prefix-Header aus 7.1
	char *lineBuffer;               // nur bei LineBuffered
	size_t lineBufferLen;
	size_t lineBufferCapacity;
#if defined(FPL_PLATFORM_WINDOWS)
	fpl__Win32Handle readHandle;
#else
	int readFd;
#endif
	fplProcessStreamType type;
	fpl_b32 isEOF;
} fpl__ProcessStream;

typedef struct fpl__ProcessStreams {
	fpl__ProcessStream output;
	fpl__ProcessStream error;
	fpl_process_output_callback *outputCallback;
	fpl_process_input_callback *inputCallback;
	void *userData;
	size_t maxCaptureSize;
	size_t inputOffset;
#if defined(FPL_PLATFORM_WINDOWS)
	fpl__Win32Handle inputHandle;
#else
	int inputFd;
#endif
	fplProcessCaptureFlags captureFlags;
	fplProcessFlags flags;
	fpl_b32 isTruncated;
} fpl__ProcessStreams;
```
Wird nur alloziert, wenn tatsächlich gecaptured/umgeleitet wird (`fpl__AllocateDynamicMemory`), und in `fplProcessClose()` freigegeben. Damit kostet der einfache "fire and forget"-Start keine einzige Allokation.

### 7.3 Besitzübergang der Capture-Buffer
Gecaptured wird in `fpl__ProcessStream.capture`. Sobald `fplProcessStart` (bei `AutoWait`) oder `fplProcessWait` ein Result füllt, wird der Buffer **verschoben, nicht kopiert**: `result.output = streams->output.capture;` und die Quelle wird genullt. Damit gilt eindeutig:
- Nach dem Move gehört der Speicher dem Result → `fplProcessFreeResult()` gibt ihn frei.
- `fplProcessClose()` gibt nur noch das frei, was *nicht* übergeben wurde (z.B. wenn der Aufrufer `outResult` als `fpl_null` übergibt oder nie gewartet hat).
- Ruft man `fplProcessWait` mehrfach, ist der Buffer beim zweiten Mal leer statt doppelt vergeben.

`fplProcessFreeResult()` prüft die Magic aus 7.1, gibt beide Buffer frei und nullt das Result — doppelter Aufruf ist erlaubt (idempotent), so wie `fplFileClose`.

---

## 8. Fehlerbehandlung, Logging, Konventionen

- Neues Modul: `#define FPL__MODULE_PROCESS "Process"` bei den anderen `FPL__MODULE_*`.
- Argument-Checks über die bestehenden Makros: `FPL__CheckArgumentNull(context, false)`, `FPL__CheckArgumentNull(outHandle, false)`, `FPL__CheckArgumentNull(context->name, false)` → `outResult->type = fplProcessResultType_InvalidArguments`.
- Fehler über `FPL__ERROR(FPL__MODULE_PROCESS, "...")`, Warnungen über `FPL__WARNING`.
- `outResult` ist optional (`fpl_null` erlaubt) — für den "starte und vergiss"-Fall.
- Kein Platform-Init nötig: Die Prozess-API hängt an nichts (kein Window/Video/Audio). `FPL__CheckPlatform` **nicht** verwenden, damit sie auch ohne `fplPlatformInit` benutzbar ist — konsistent mit `fplFileOpenBinary`/`fplConsoleOut`? (→ prüfen, wie die Datei-API es genau hält, und identisch handhaben.)
- **Kein** `FPL_NO_PROCESS` (so entschieden): die Prozess-API ist auf allen unterstützten Plattformen immer vorhanden. Konsequenzen, die dadurch zur Pflicht werden: die zusätzlichen POSIX-Includes (`<sys/wait.h>`, `<poll.h>`) müssen auf *allen* POSIX-Zielen (Linux, BSD, macOS) vorhanden sein — sind sie, sie sind POSIX.1-Standard; `<sys/prctl.h>` wird nur innerhalb `#if defined(FPL_PLATFORM_LINUX)` eingebunden, weil es Linux-spezifisch ist. Kein `#if`-Rauschen um die Deklarationen herum.
- `FPL_NO_CRT`-Kompatibilität: keine CRT-Funktionen benutzen (kein `snprintf`, kein `malloc`), nur `fplStringFormat`, `fplCopyString`, `fpl__AllocateDynamicMemory`. `demos/FPL_NoCRT` muss weiter bauen.
- `FPL_NO_PLATFORM_INCLUDES`-Kompatibilität: `fplInternalProcessHandle` nutzt ausschließlich die opaken Vorab-Typen (`fpl__Win32Handle`, `int32_t`), nie `HANDLE`/`pid_t` direkt — exakt wie `fplInternalFileHandle`.
- `FPL_NO_RUNTIME_LINKING`: keine neuen dynamischen Ladepfade nötig (Win32 = kernel32 direkt, POSIX = libc direkt). Ausdrücklich verifizieren mit `demos/FPL_NoRuntimeLinking`.

---

## 9. Änderungen an Dateien

| Datei | Änderung |
|---|---|
| `final_platform_layer.h` | Neue `@defgroup Process` nach `DynamicLibrary`; opake Handle-Typen; `FPL__MODULE_PROCESS`; Sektionsmarker `// > WIN32_PROCESS` und `// > POSIX_PROCESS` in der Marker-Liste oben ergänzen; Common-Teil (`fplProcessFreeResult`, argv↔cmdline-Helper) im `// > COMMON`-Block; Changelog-Eintrag unter v1.0.1 |
| `final_platform_layer.docs` | Neue `@section section_category_process_*` (Overview, Start, Capture, Redirect, Input, Wait/Stop, Notes) mit Beispielen; in die Kategorie-Übersicht oben eintragen |
| `demos/FPL_Process/` (neu) | `fpl_process.c`, `CMakeLists.txt`, `Makefile`, `premake5.lua`, `.vcxproj(+.filters/.user)` — analog zu `demos/FPL_Console` |
| `demos/FPL_Test/fpl_test.c` | Neue Testgruppe `TestProcess()` |
| `demos/demos_final_platform_layer_premake5.lua`, `.sln` | Neues Projekt eintragen |
| `README.md` / `final_game_tech.md` | Feature-Liste ergänzen |

CMake ist maßgeblich (`MY_COMPILER`, `MY_TRANSLATION_UNITS`, `MY_HEADER_FILES`), `.vcxproj` wird nachgezogen.

---

## 10. Tests

Problem: Prozess-Tests brauchen ein Zielprogramm, und `ls`/`dir` sind nicht plattformgleich.
**Lösung: Das Testprogramm startet sich selbst.** `fpl_test.c` bekommt einen Kind-Modus über `argv`:

```
FPL_Test --child-echo <text>       gibt <text> auf stdout aus, Exit 0
FPL_Test --child-error <text>      gibt <text> auf stderr aus, Exit 0
FPL_Test --child-exit <code>       beendet sich sofort mit <code>
FPL_Test --child-cat               liest stdin bis EOF und spiegelt es auf stdout
FPL_Test --child-spam <bytes>      schreibt <bytes> Bytes auf stdout (Deadlock/Truncate-Test)
FPL_Test --child-sleep <ms>        schläft (Timeout-/Stop-Tests)
```
Der eigene Pfad kommt aus `fplGetExecutableFilePath()`. Keine externen Abhängigkeiten, identisches Verhalten auf allen Plattformen.

Testfälle:
1. Start + AutoWait, Exit-Code 0 und Exit-Code 42.
2. Capture stdout / stderr getrennt und merged.
3. Redirect per Callback, Chunk- und Line-Modus.
4. Buffer + Callback gleichzeitig.
5. `--child-spam 8000000` mit `CaptureSeparate` → darf nicht deadlocken (der eigentliche Regressionstest für 1.15).
6. `maxCaptureSize` → `isTruncated`, Kind darf nicht hängenbleiben.
7. stdin: Text-Modus, Callback-Modus, Stream-Modus mit `fplProcessWriteInput` + `fplProcessCloseInput` gegen `--child-cat`.
8. Nicht existierendes Programm → `_NotFound`; Datei ohne x-Bit (POSIX) → `_AccessDenied`.
9. `--child-sleep 10000` + `fplProcessWait(handle, 100, ...)` → `_Timeout`; danach `fplProcessStop` → beendet.
10. Async: Start ohne AutoWait, `fplProcessIsRunning` pollen, danach Wait + Exit-Code.
11. `workDir` wirkt (Kind gibt CWD aus).
13. Skript: temporäres `.sh` bzw. `.bat` schreiben, mit `fplProcessShellMode_Default` starten, Ausgabe prüfen.
13b. Custom-Shell: dasselbe Skript mit `fplProcessShellMode_Custom` + `/bin/bash` (POSIX) bzw. `cmd.exe` mit explizitem `shellPath` (Windows) starten.
14. Argumente mit Leerzeichen und Anführungszeichen kommen unverfälscht an (argv-Array-Pfad!).
15. Kein Leak: 200× Start/Close in einer Schleife, danach `fplMemoryGetUsage` und (POSIX) Zombie-Zählung prüfen.

---

## 11. Phasenplan

| Phase | Inhalt | Ergebnis |
|---|---|---|
| **0** | Typen + Deklarationen + Doku im Header, keine Implementierung | API steht, kompiliert auf beiden Plattformen (Stubs) |
| **1** | POSIX-Kern: fork/exec, exec-Fehlerpipe, Wait, IsRunning, Stop, Close, `workDir`, argv | `FPL_Process`-Demo startet Programme unter Linux |
| **2** | Win32-Kern: CreateProcessW, Quoting, Wait, ExitCode, Stop, Close | Feature-Gleichstand Windows |
| **3** | Capture merged (eine Pipe) auf beiden Plattformen + Pump/AutoWait-Schleife | `CaptureBoth` funktioniert, Deadlock-Test grün |
| **4** | Capture separat (poll / PeekNamedPipe), Callbacks, LineBuffered, maxCaptureSize | volle Capture/Redirect-Matrix |
| **5** | stdin: None/Text/Callback/Stream, SIGPIPE-Behandlung | Input komplett |
| **6** | shellMode (Default+Custom), NoWindow, Detached, KillProcessTree, KillOnParentExit, Timeouts, Handle-Liste (Win32) | Flags komplett |
| **7** | Demo, Tests, `.docs`, Changelog, Build-Dateien, NoCRT/NoRuntimeLinking/NoPlatformIncludes verifizieren | Release-fertig |

Phasen 1 und 2 sind unabhängig, 3–6 sollten pro Feature immer beide Plattformen zusammen abschließen — sonst driften die Semantiken auseinander.

---

## 12. Bewusst NICHT enthalten (v1)

- **Privilege-Escalation / anderer User** (`runas`, `CreateProcessAsUser`, `setuid`) — laut Vorgabe ausdrücklich nicht.
- **Interner Reader-Thread** — Pump-Modell reicht, kann später additiv kommen.
- **`ShellExecuteEx` / Datei mit Standard-Anwendung öffnen / URLs** — anderes Feature (`fplOpenURL`), andere Semantik (kein Handle, kein Exit-Code). Wäre ein sinnvoller separater Nachschlag.
- **PTY / Terminal-Emulation** (`forkpty`, ConPTY) — für Tools, die bei einer Pipe ihr Verhalten ändern (Farben, Interaktivität). Großes eigenes Thema.
- **Environment-Variablen** — das Kind erbt das Environment des Parents. Nachrüstbar als `environment[]` + `environmentCount` + Inherit-Flag, ohne Bruch der Signatur (Windows: `lpEnvironment`-Block in UTF-16, doppelt null-terminiert und sortiert; POSIX: `execve` mit gemergtem `environ`).
- **Prozess-Enumeration** (laufende Prozesse auflisten, nach Namen suchen) — eigenes Feature.
- **Prioritäten/Affinity** — leicht nachrüstbar über ein zusätzliches Feld im Context.

---

## 13. Entschieden (interaktiv geklärt, 2026-08-23)

| # | Frage | Entscheidung |
|---|---|---|
| 1 | Capture-Steuerung | **`fplProcessCaptureFlags`** als Flags-Enum mit Preset-Werten, die genau wie im Sketch heißen. Buffer und Callback gleichzeitig sowie getrennte Streams sind damit darstellbar. |
| 2 | Result-Buffer | **`fplProcessBuffer output`/`error` by value**, öffentlich nur `text` + `len`. Die Buchhaltung liegt in einem versteckten Prefix-Header direkt vor dem Text (7.1). |
| 3 | Optionen | **`fplProcessFlags flags`** als Flags-Feld statt einzelner Bools. `autoWait` wird zu `fplProcessFlags_AutoWait`. |
| 4 | Naming | **`fplProcessStart` / `Update` / `Wait` / `IsRunning` / `RequestStop` / `Stop` / `Close`** wie im Sketch und wie .NET. |
| 5 | Compile-Switch | **Kein `FPL_NO_PROCESS`** — die API ist immer aktiv. |
| 6 | Shell | **`fplProcessShellMode`** (`None`/`Default`/`Custom`) + `shellPath`/`shellArgument` statt `bool useShell`. Deckt `cmd.exe`, PowerShell, `bash` statt `dash` und beliebige Interpreter ab; FPL quotet in allen Fällen selbst. |

Damit ist die API-Oberfläche festgelegt und Phase 0 (Typen + Deklarationen + Doku im Header) kann beginnen.
