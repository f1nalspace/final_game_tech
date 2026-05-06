# FPL Code Security & Stability Analysis

**Target:** `final_platform_layer.h` (v1.0.0, ~31074 lines)
**Date:** 2026-05-06
**Scope:** Stability, security, correctness, and API consistency.
**Out of scope:** Anything stubbed-out / not yet implemented.

This document is structured by severity. Each entry lists the affected
location(s), the problem, and a proposed fix.

---

## 1. Critical Bugs (data corruption / crashes / security)

### 1.1 `fplFileCopy` writes source path into target buffer
- **Location:** lines 16360–16369 (Win32 impl)
- **Problem:** Second `fplUTF8StringToWideString` call uses `sourceFilePath`
  for both source and target instead of `targetFilePath`. `CopyFileW` is
  invoked with the same wide string for source and destination,
  silently making the API non-functional on Windows.
- **Fix:** Replace the second call's `sourceFilePath` argument with
  `targetFilePath`:
  ```c
  fplUTF8StringToWideString(targetFilePath, fplGetStringLength(targetFilePath),
                            targetFilePathWide, fplArrayCount(targetFilePathWide));
  ```

### 1.2 `fplFileMove` writes source path into target buffer
- **Location:** lines 16371–16380 (Win32 impl)
- **Problem:** Same copy/paste mistake as 1.1. `MoveFileW` is called with
  source==dest, so the call is a no-op or fails.
- **Fix:** Replace second call's `sourceFilePath` with `targetFilePath`.

### 1.3 `fplFileReadBlock32` / `fplFileReadBlock64` ignore `maxTargetBufferSize`
- **Location:** lines 16072–16117, 16089–16117 (Win32)
- **Problem:** The function signature accepts `maxTargetBufferSize`, but
  the implementation never compares it against `sizeToRead`. A caller
  that passes `sizeToRead > maxTargetBufferSize` triggers an unbounded
  write past the user buffer (heap/stack corruption).
- **Fix:** Clamp before reading:
  ```c
  uint32_t actualToRead = (sizeToRead > maxTargetBufferSize) ? maxTargetBufferSize : sizeToRead;
  ```
  Apply the same change in 32-bit and 64-bit variants. The same applies
  to the POSIX variants (verify against `fpl__POSIXFileHandle` block).

### 1.4 `fplFileReadBlock64` / `fplFileWriteBlock64` lose accumulated bytes
- **Location:** lines 16108–16115 and 16149–16161 (Win32)
- **Problem:** Inside the chunk loop, `result = bytesRead;` (and
  `result = bytesWritten;`) overwrite the running total. After more
  than one chunk, `result` is just the size of the last chunk. Caller
  cannot detect a partial transfer from a short final read.
- **Fix:** `result += bytesRead;` (and `result += bytesWritten;`).

### 1.5 `fplFileGetPosition64` returns hardcoded 0
- **Location:** line 16228 (Win32)
- **Problem:** Function computes `result` correctly via
  `SetFilePointerEx`, then returns the literal `0` instead of `result`.
  Callers always see position 0.
- **Fix:** `return result;` instead of `return 0;`.

### 1.6 `fplFileClose` calls `CloseHandle(0)` on uninitialized handles
- **Location:** lines 16241–16247 (Win32)
- **Problem:** Guard is
  `internalHandle.win32FileHandle != INVALID_HANDLE_VALUE`. After
  `fplClearStruct()` the handle is `0` (NULL), not `INVALID_HANDLE_VALUE`
  (`-1`). A handle that was never opened compares unequal to
  `INVALID_HANDLE_VALUE`, so `CloseHandle(NULL)` is invoked and may
  crash / set last-error.
- **Fix:** Test both: `handle != fpl_null && handle != INVALID_HANDLE_VALUE`,
  *or* initialize `internalHandle.win32FileHandle` to
  `INVALID_HANDLE_VALUE` on every open path.
- The same pattern appears across other Win32 file functions
  (`fplFileSetPosition32/64`, `fplFileGetPosition32/64`,
  `fplFileGetSizeFromHandle32/64`, `fplFileFlush`,
  `fplFileGetTimestampsFromHandle`). All inherit this latent bug.

### 1.7 POSIX `fplGetExecutableFilePath` undefined behaviour
- **Location:** lines 19129–19161
- **Problems:**
  1. `readlink()` does **not** NUL-terminate. The code immediately calls
     `fplGetStringLength(buf)`, reading uninitialised memory beyond the
     written length.
  2. `if (readlink(procName, buf, ...))` treats the return value as
     boolean. `readlink` returns `-1` on error, which is **truthy**, so
     errors are mistreated as success and a garbage `buf` is read.
- **Fix:**
  ```c
  ssize_t n = readlink(procName, buf, sizeof(buf) - 1);
  if (n > 0) {
      buf[n] = '\0';
      ...
  }
  ```
  Use `n` instead of `fplGetStringLength`.

### 1.8 POSIX `fplGetHomePath` may dereference NULL
- **Location:** lines 19164–19178
- **Problem:** When `getenv("HOME")` returns NULL, the code calls
  `getpwuid(...)` and then `userPwd->pw_dir` without checking
  `userPwd != NULL`. `getpwuid` can fail (e.g. NSS error), causing a
  segfault.
- **Fix:** Check `userPwd != fpl_null && userPwd->pw_dir != fpl_null`,
  return 0 otherwise.

### 1.9 `fpl__X11Window` opaque type truncates X11 IDs
- **Location:** line 3354 (opaque branch)
- **Problem:** `typedef int fpl__X11Window;` — but the X11 `Window` type
  is `unsigned long` (an `XID`). On 64-bit Linux the opaque variant
  truncates window IDs to 32-bit when `FPL_OPAQUE_HANDLES` /
  `FPL_NO_PLATFORM_INCLUDES` is used. Any valid window with an ID above
  2³¹ will fail to match.
- **Fix:** `typedef unsigned long fpl__X11Window;` (or `uint64_t` to be
  safe across LP64 and LLP64). Update size comment ("4 bytes") to
  match.

### 1.10 POSIX UTF-8 conversion uses non-NUL-terminated input
- **Location:** lines 19320–19345 (`STD_STRINGS` impl)
- **Problem:** `wcstombs(NULL, src, n)` and `mbstowcs(NULL, src, n)` are
  defined for **NUL-terminated** input strings; the `n` parameter is
  the *output* limit, not an input length. The code passes
  `wideSourceLen` / `utf8SourceLen` as `n` and assumes the input is
  measured. If the input is not NUL-terminated, behaviour is undefined
  (read overrun).
  Additionally, `wcstombs`/`mbstowcs` return `(size_t)-1` on failure,
  but the code stores the result unchecked and later does
  `result + 1`, producing 0/wrap-around used as a length (large
  allocation downstream).
- **Fix:**
  - Use a temporary NUL-terminated copy of the input (or switch to
    `wcsrtombs`/`mbsrtowcs` which take an explicit src pointer and
    state and respect lengths better).
  - Check for `(size_t)-1` and return 0 with `FPL__ERROR(...)`.

### 1.11 Win32 UTF-8 conversion truncates buffer size cast
- **Location:** line 16733 (`WideCharToMultiByte`) and 16745
  (`MultiByteToWideChar`)
- **Problem:** `(int)maxUtf8DestLen` casts a `size_t` to `int`. If
  `maxUtf8DestLen > INT_MAX` (rare but possible for huge buffers),
  the cast becomes negative or zero, the API may overrun or
  silently fail. There is also no error check on the conversion
  (returns 0 on failure; the code keeps going).
- **Fix:**
  - Clamp to `INT_MAX` (or convert in chunks if larger).
  - On `WideCharToMultiByte` returning 0, push an `FPL__ERROR` and
    return 0 from the FPL wrapper.

---

## 2. Major Bugs

### 2.1 `fplGetStringLength` returns truncated length
- **Location:** lines 11700–11708
- **Problem:** Internal counter is `uint32_t` but the return type is
  `size_t`. On 64-bit systems with strings larger than 4 GiB the
  length wraps. Several callers feed this value into
  `+ 1`/`*sizeof(...)`/`fplCopyStringLen`, propagating the underflow.
- **Fix:** Use `size_t` for the counter:
  ```c
  size_t result = 0;
  ```

### 2.2 `fplStringFormatArgs` ambiguous return on truncation
- **Location:** lines 11732–11766
- **Problems:**
  1. After `vsnprintf`, when `charCount >= maxDestBufferLen` the code
     returns 0 via `FPL__CheckArgumentMin` — which is the same value as
     "format error" (`charCount < 0`). Callers cannot distinguish
     truncation from invalid format.
  2. When destination is too small, the buffer is left in whatever
     state `vsnprintf` produced (most C runtimes truncate and write
     a NUL — but old MSVC `_vsnprintf` did **not**). FPL should
     guarantee a NUL terminator at `destBuffer[maxDestBufferLen-1]`
     in all paths.
  3. The error message string is grammatically wrong:
     `"Format parameter are '%s' are invalid"`.
- **Fix:**
  - Return the number of *required* characters (excluding NUL) even on
    truncation, mirroring the C99 `vsnprintf` contract.
  - Always force `destBuffer[fplMin(maxDestBufferLen-1, charCount)] = 0`.
  - Fix message to `"Format parameter '%s' is invalid"`.

### 2.3 `fplCopyString*` / `fplStringAppend*` API contract is broken
- **Location:** lines 6683–6720, 11679–11730
- **Problem:** The change-log (`#74`) states these functions "return the
  number of characters instead of a char-pointer", and individual doc
  comments add: "allows to pass null-pointer as output argument to
  return the number of characters only". The actual signatures still
  return `char *`, and the implementation returns `fpl_null` whenever
  `dest == fpl_null`. Therefore:
  - You cannot ask "how many bytes do I need?" — calling with
    `dest == NULL` returns `NULL`, no count.
  - The returned `char *` is the *end* pointer, not a count, so
    callers cannot compute the written length except via subtraction.
- **Fix:** Choose one direction (recommendation: align with the rest of
  the API and return `size_t`):
  ```c
  fpl_common_api size_t fplCopyString(const char *source, char *dest, size_t maxDestLen);
  fpl_common_api size_t fplCopyStringLen(const char *source, size_t sourceLen, char *dest, size_t maxDestLen);
  fpl_common_api size_t fplStringAppend(const char *appended, char *buffer, size_t maxBufferLen);
  fpl_common_api size_t fplStringAppendLen(const char *appended, size_t appendedLen, char *buffer, size_t maxBufferLen);
  ```
  Allow `dest/buffer == NULL` to compute the required size.
  Update all internal call sites that use the returned `char *`
  (e.g. `fpl__PushError_Formatted`, `fplGetWindowTitle`,
  several path/version builders).

### 2.4 `fplStringAppendLen` off-by-one in remaining-size formula
- **Location:** line 11689
- **Problem:**
  ```c
  size_t remainingBufferSize = maxBufferLen - (curBufferLen > 0 ? curBufferLen + 1 : 0);
  ```
  Once a NUL is overwritten by the new copy at `buffer[curBufferLen]`,
  the remaining usable capacity is `maxBufferLen - curBufferLen`
  (not `… - curBufferLen - 1`). The off-by-one shrinks the available
  area by one byte. Combined with `fplCopyStringLen`'s strict
  `requiredLen > maxDestLen` check, fully-fitting appends are
  rejected.
- **Fix:**
  ```c
  size_t remainingBufferSize = maxBufferLen - curBufferLen;
  ```

### 2.5 `fplPathCombine` leaks `va_list` and writes a partial path
- **Location:** lines 12798–12834
- **Problems:**
  1. The `FPL__CheckArgumentMin` macro inside the loop returns 0
     immediately, **without** calling `va_end(vargs)`. On systems where
     `va_list` is heap-backed (some 64-bit ABIs) this is a leak/UB.
  2. A partial write may already have happened in earlier iterations,
     leaving the destination buffer in an inconsistent / non-terminated
     state.
  3. `*currentDestPtr = 0;` is reached even when destPath is null
     (`currentDestPtr` keeps the original null), guarded only by an
     `!= fpl_null` check at the very end.
- **Fix:** Compute the required length in a first pass (no writes),
  validate it once against `maxDestPathLen`, and only then write in a
  second pass. Always call `va_end` before returning.

### 2.6 `fplStringToS32Len` overflow & ambiguous failure
- **Location:** lines 11825–11850
- **Problems:**
  1. `value *= 10; value += v;` has no overflow check. Strings like
     `"99999999999999"` silently produce wrong values.
  2. Returns `0` for invalid input *and* for the literal string `"0"`.
     Caller cannot tell the two apart.
  3. No leading-whitespace handling, no `+`-sign handling.
  4. `fplStringToS32` calls `fplGetStringLength`, then
     `fplStringToS32Len` and re-walks the string — wasted work.
- **Fix:** Provide an out-parameter for success, e.g.
  ```c
  bool fplTryStringToS32Len(const char *str, size_t len, int32_t *outValue);
  ```
  or define a sentinel error return (e.g. a struct with `bool ok`).
  Detect overflow with `if (value > (uint32_t)INT32_MAX + 1) ...`.

### 2.7 `fplFileGetSizeFromPath32` / `…Handle32` cannot signal failure
- **Location:** lines 16249–16289
- **Problem:** `GetFileSize` returns `INVALID_FILE_SIZE` (`0xFFFFFFFF`)
  on error, and the code stores it as the file size verbatim. A
  caller may treat that as a real ~4 GiB file. `0` is also returned
  on legitimate empty files but is not distinguishable from
  `CreateFileW` failure.
- **Fix:** Use `GetFileSizeEx` always (even for the 32-bit wrapper),
  detect failure with the documented two-step (`GetLastError() != NO_ERROR`),
  and surface a boolean `outOk` parameter or sentinel value.

### 2.8 `fplFileSetPosition32` does not check `INVALID_SET_FILE_POINTER`
- **Location:** lines 16165–16181
- **Problem:** `SetFilePointer` returns `INVALID_SET_FILE_POINTER`
  (`0xFFFFFFFF`) on error. The code casts it directly to `uint32_t`
  and returns it, indistinguishable from a legitimate position at
  4 GiB - 1.
- **Fix:** Follow Microsoft's documented two-step error check
  (`r == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR`), or
  switch to `SetFilePointerEx` and convert.

### 2.9 `fpl__PushError_Formatted` is not thread-safe
- **Location:** lines 11343–11374
- **Problem:** `fpl__global__LastErrorState` (an array of strings plus
  a counter) is mutated without any synchronization. Concurrent
  errors from worker threads (audio, video) corrupt the buffer and
  the counter.
- **Fix:** Either:
  - Wrap mutations in `fplAtomicAddAndFetchU32` for the index and use
    a single-writer-per-slot pattern; or
  - Guard with a small `fplMutexHandle` that is initialised at
    `fplPlatformInit` time.

### 2.10 `fplStringFormatArgs` MSVC pre-C99 vsnprintf hazard
- **Location:** line 11752
- **Problem:** Pre-2015 MSVC `vsnprintf` returns `-1` on truncation
  (instead of the required size). The code already routes through
  `FPL_USERFUNC_vsnprintf` for `FPL_NO_CRT`, but the regular CRT path
  treats `-1` as a hard format error. On Vista-era runtimes the
  function will report failures for any oversized string.
- **Fix:** When `charCount < 0`, fall back to `_vscprintf(format, listCopy)`
  on MSVC ≤ 1900 to obtain the required size (or use `vsnprintf_s`).

### 2.11 `fplDirectoryListBegin` retains caller pointers (lifetime)
- **Location:** lines 19052–19068 (POSIX, similar issue exists in Win32 logic)
- **Problem:** `fplFileEntry::internalRoot` stores the caller's
  `path` and `filter` pointers verbatim. Subsequent
  `fplDirectoryListNext` reads them. If the caller frees the strings
  after `Begin` (a normal pattern with stack buffers freed by going
  out of scope), the next iteration reads dangling memory.
- **Fix:** Copy `path`/`filter` into fixed-length buffers inside the
  `fplFileEntry` structure (similar to `name[FPL_MAX_FILENAME_LENGTH]`).

### 2.12 `fplPathNormalize` (Windows & POSIX) drops required-size info
- **Location:** lines 16558–16575 (Win32), 19180–19195 (POSIX)
- **Problem:** When the destination buffer is too small, the function
  returns 0, hiding the required size. Other FPL output-buffer
  functions consistently return the required count when destination
  is `null` or too small. `GetFullPathNameW` on Windows already
  returns the required length when the buffer is too small — that
  information is discarded.
- **Fix:** On Windows: if the first `GetFullPathNameW` returns a value
  greater than `maxDestLen`, return that value (still error out for
  user). On POSIX: pre-allocate enough scratch via `realpath(NULL)`
  (PATH_MAX-free variant) and report the required count.

### 2.13 Audio backend chunk write API uses outdated reader semantics
- **Location:** comment block near line 26698
- **Problem:** Comment notes `fpl__ReadAudioFramesFromClient always
  fills the full frame count`. This is now the documented contract
  (see project memory `feedback_read_audio_frames_contract.md`), but
  not all audio backends respect it (the loop `fpl__ReadAudioFramesFromClient`
  call sites should not gate on the return value). A wider audit of
  every call site is required.
- **Fix:** Add an `fplAssert(read == frameCount)` after every internal
  call, then remove any conditional logic that branches on a partial
  read.

---

## 3. API Consistency Issues

### 3.1 Output-buffer return-type pattern is mixed
- Functions that should "return required/written characters" but
  return `char *` instead:
  - `fplCopyString`, `fplCopyStringLen`
  - `fplStringAppend`, `fplStringAppendLen`
  - `fplEnforcePathSeparator`, `fplEnforcePathSeparatorLen`
- Functions that already use `size_t`:
  - `fplStringFormat`, `fplStringFormatArgs`,
    `fplWideStringToUTF8String`, `fplUTF8StringToWideString`,
    `fplGetExecutableFilePath`, `fplGetHomePath`,
    `fplExtractFilePath`, `fplChangeFileExtension`, `fplPathCombine`,
    `fplPathNormalize`, `fplCPUGetName`, `fplSessionGetUsername`,
    `fplS32ToString`.
- **Fix:** Convert *all* output-buffer string functions to:
  ```c
  size_t fpl<Foo>(<inputs>, char *dest, size_t maxDestLen);
  ```
  Contract:
  1. Returns required characters excluding NUL.
  2. Always NUL-terminates if `dest != NULL && maxDestLen > 0`.
  3. `dest == NULL` is allowed and just returns the required size.
  4. Returns 0 only on hard errors (NULL input, invalid format, etc.).

### 3.2 `fplOpenGLCompabilityFlags` typo: "Compability" → "Compatibility"
- **Location:** lines 4762–4774, 79, 84, 1389, 1642, all examples and
  changelog references.
- **Problem:** Persistent misspelling across enum, settings field,
  and documentation (`compabilityFlags`, `fplOpenGLCompabilityFlags_*`).
- **Fix:** Add an alias for back-compat *only* if you value the older
  name; otherwise rename in 1.0 since this is a release boundary:
  ```c
  typedef enum fplOpenGLCompatibilityFlags {
      fplOpenGLCompatibilityFlags_Legacy = 0,
      fplOpenGLCompatibilityFlags_Core = 1 << 1,
      fplOpenGLCompatibilityFlags_Compatibility = 1 << 2,
      fplOpenGLCompatibilityFlags_Forward = 1 << 3,
  } fplOpenGLCompatibilityFlags;
  ```
  Rename `fplOpenGLSettings::compabilityFlags` → `compatibilityFlags`.

### 3.3 Date-time documentation typos
- **Location:** lines 3667–3671 (declaration of `fplDateTimeCreate`)
- **Problems:**
  - `@param minute[in] The minute in range of 0-23.` — should be
    `0-59`.
  - `@param second[in] The minute in range of 0-59.` — should be
    `The second in range of 0-59`.
- **Fix:** Correct both lines.

### 3.4 `fplFormatString` → `fplStrngFormat` typo in changelog
- **Location:** lines 392–393, 766–767
- **Problem:** Changelog entries say `fplFormatString()` was renamed to
  `fplStrngFormat()` — almost certainly a typo for `fplStringFormat`,
  the actual current name. This is still confusing to readers.
- **Fix:** Update the changelog text to `fplStringFormat`.

### 3.5 Naming inconsistency: noun-verb vs verb-noun
- The codebase has a mix:
  - "Get prefix": `fplGetStringLength`, `fplGetExecutableFilePath`,
    `fplGetHomePath`, `fplGetWindowTitle`.
  - "Object first" (newer convention): `fplFileOpenBinary`,
    `fplFileGetPosition`, `fplPathNormalize`, `fplPathCombine`,
    `fplCPUGetName`, `fplSessionGetUsername`.
- **Fix:** Pick one convention consistently. Recommendation:
  "namespace-object first" (e.g. `fplStringGetLength`,
  `fplPathGetExecutable`, `fplPathGetHome`, `fplWindowGetTitle`).
  Provide one-cycle deprecation aliases.

### 3.6 `fpl_b32` vs `bool` mixed in public structs
- `fplFileHandle::isValid` is `fpl_b32`; many other booleans use `bool`.
- **Fix:** Standardise. `bool` matches stdbool.h and is what consumers
  expect; reserve `fpl_b32` for atomic-friendly ABI structs.

### 3.7 `fplFileGetSizeFromPath32` and friends conflate "0" with "error"
- See 2.7. `0` is documented as both "empty file" and "error". A
  separate `bool fplFileTryGetSizeFromPath(...)` returning size via
  out-parameter would resolve this.

### 3.8 `fplStringFormat` doc claims "wrapper to vsnprintf"
- **Location:** lines 6751, 6762
- **Problem:** Doc says "This is most likely just a wrapper call to
  vsnprintf()". This is implementation detail leaking into the
  public contract. The internal `FPL_USERFUNC_vsnprintf` and
  `FPL_NO_CRT` paths invalidate that statement.
- **Fix:** Drop the note or rephrase to "Format follows the standard
  C99 vsnprintf format specifiers".

---

## 4. Type / Opaque-Handle Issues

### 4.1 Opaque handle size comments wrong
- **Location:** lines 3300, 3338–3343, 3340 (POSIX & Win32 opaque branch)
- **Problem:** Comments such as `min 80 bytes`, `min 40 bytes`,
  `min 32 bytes`, `min: 48 bytes` claim minimum sizes that are
  smaller than the buffers actually declared (`uint64_t[16]` is 128
  bytes, `uint64_t[8]` is 64 bytes). The comments also do not
  reflect the *real* underlying type sizes on different libcs:
  - glibc Linux x86_64: `pthread_mutex_t` 40 B, `pthread_cond_t` 48 B,
    `sem_t` 32 B.
  - musl: `pthread_mutex_t` 40 B, `pthread_cond_t` 48 B, `sem_t` 32 B.
  - macOS/BSD differ.
- **Fix:** Replace each comment with the real maximum across supported
  platforms, and add a `fplStaticAssert(sizeof(pthread_mutex_t) <= sizeof(fpl__POSIXMutexHandle));`
  in the non-opaque code path so the buffer can never be undersized.

### 4.2 `fpl__X11Display`, `fpl__X11Visual`, `fpl__X11GC`, `fpl__X11Image`
  declared as `void`
- **Location:** lines 3352, 3356, 3358, 3360, 3362
- **Problem:** `typedef void fpl__X11Display;` is legal but unusual
  and the comment `(opaque, 4/8 bytes)` is misleading — the type is
  not pointer-sized; users access it as `fpl__X11Display *` everywhere.
  Internally this works, but `sizeof(fpl__X11Display)` is invalid C.
- **Fix:** Make the opaque types pointer-typed for clarity:
  ```c
  typedef struct fpl__X11Display fpl__X11Display; /* incomplete struct */
  typedef struct fpl__X11Visual  fpl__X11Visual;
  ...
  ```
  Update the size comment ("opaque incomplete type, accessed through pointer").

### 4.3 `fpl__POSIXThreadHandle` width
- **Location:** lines 3337, 3417
- **Problem:** Opaque definition is `uint64_t fpl__POSIXThreadHandle`,
  matching glibc's `pthread_t` on x86_64. On macOS `pthread_t` is
  pointer-typed; on AIX it is a struct. The opaque branch is unsafe
  on those platforms.
- **Fix:** Document the platform constraints (or expand to
  `uint8_t buffer[16]` and use `fplStaticAssert(sizeof(pthread_t) <= 16)`).

### 4.4 `fpl__Win32Console` / `fpl__Win32Guid` opaque mismatch
- **Location:** line 3284–3289 (opaque) vs line 3378 (real)
- **Problem:** Opaque `fpl__Win32Guid` uses `uint32_t Data1` but the
  real `GUID` uses `unsigned long Data1`. On 64-bit Windows
  `unsigned long` is still 32-bit, so this happens to match — but
  using `uint32_t` is more portable and matches the size
  comment. Recommend keeping `uint32_t` (current opaque) and
  asserting `sizeof(fpl__Win32Guid) == sizeof(GUID)` in the real path.

### 4.5 `fplDateTime::epoch` cannot represent dates before 1970
- **Location:** lines 3619–3626
- **Problem:** `epoch` is `uint64_t`. Pre-1970 dates and tests of
  conversion functions cannot go negative. Even though the docs say
  "since 1970-01-01", users may pass earlier values and silently
  underflow.
- **Fix:** Either change to `int64_t` (preferred — matches `time_t`
  semantics and Windows FILETIME conversion) and update the
  `fplDateTimeCreate` validation accordingly, or document explicitly
  that pre-1970 is unsupported and reject in `fplDateTimeCreate`.

---

## 5. Other Stability / Correctness Issues

### 5.1 `fplExtractFilePath` truncates large paths via `int`
- **Location:** line 12677
- **Problem:** `int pathLen = 0;` and the cast
  `pathLen = (int)(chPtr - sourcePath);`. Paths > 2 GiB (rare but
  possible for symlink chains and synthetic paths) overflow.
- **Fix:** `size_t pathLen = 0;` and remove the `(int)` cast.

### 5.2 `fplGetExecutableFilePath` (Win32) ignores truncation
- **Location:** lines 16540–16546
- **Problem:** `GetModuleFileNameW(NULL, modulePath, MAX_PATH)` returns
  `MAX_PATH` when the path is exactly that long *or* longer (with
  `ERROR_INSUFFICIENT_BUFFER`). The buffer may not be NUL-terminated.
  `lstrlenW` then walks past the buffer.
- **Fix:** Capture the return value, set `modulePath[result] = 0;` if
  `result < MAX_PATH`, otherwise retry with a larger buffer
  (`HeapAlloc`/`fplMemoryAllocate`) until the result is < requested
  capacity.

### 5.3 `fplGetHomePath` (Win32) ignores `SHGetFolderPathW` failure
- **Location:** lines 16548–16556
- **Problem:** The `HRESULT` is dropped. On failure, `homePath` is
  uninitialised, then `lstrlenW` is called on it.
- **Fix:** Check `SUCCEEDED(hr)`; on failure return 0.

### 5.4 `fplDateTimeQuery` (POSIX) ignores `gettimeofday` errors
- **Location:** lines 19201–19219
- **Problem:** `gettimeofday` may return -1 (rare). `localtime` may
  return NULL. The code dereferences `localTime->tm_gmtoff` without
  a NULL check. `localtime` is also not thread-safe — use
  `localtime_r`.
- **Fix:**
  ```c
  if (gettimeofday(&tv, NULL) != 0) return fplZeroInit;
  struct tm tmp;
  struct tm *localTime = localtime_r(&rawtime, &tmp);
  if (!localTime) return fplZeroInit;
  ```

### 5.5 `fplDirectoriesCreate` (POSIX) ignores Windows path separators
- **Location:** lines 18954–18992
- **Problem:** Hardcoded `/` separator. Paths created with
  `\\` (e.g. via cross-platform string concatenation that didn't go
  through `FPL_PATH_SEPARATOR`) break.
- **Fix:** Treat both `/` and `\\` as separators in path scanning.

### 5.6 `fplFormatDateTime` (POSIX) declared without API tag
- **Location:** line 19221
- **Problem:** Declaration in the public header carries
  `fpl_platform_api`; the implementation omits it.
- **Fix:** Add `fpl_platform_api` to the implementation.

### 5.7 `fplPathCombine` allows separator drift
- **Location:** lines 12798–12834
- **Problem:** If a path argument already ends with a separator, the
  function still adds another one. The result has a duplicate
  separator (e.g. `"a/" + "b"` → `"a//b"`).
- **Fix:** When appending the separator, check
  `path[pathLen-1] != FPL_PATH_SEPARATOR`; otherwise, skip writing the
  separator.

### 5.8 `fplDirectoryListNext` skips first match on filter
- **Location:** lines 19070–19094 (POSIX)
- **Problem:** Function calls `readdir()` once before entering the
  do-while, then `readdir()` again at the bottom. If the *very first*
  directory entry doesn't match the filter, it is read but its
  matching is checked, then re-read. Logic is OK but easy to break;
  refactor to a single `for (;;) { dp = readdir(...); ... }` loop.

### 5.9 `fpl__PushError_Formatted` circular buffer lossy
- **Location:** lines 11353–11366
- **Problem:** When the ring overflows, oldest errors are silently
  dropped and the user has no way to know the oldest entry rolled.
- **Fix:** Track a separate `dropped` counter and expose it via a new
  `fplGetLastErrorCount(int *outDropped)` accessor (or document the
  limitation).

### 5.10 `fplStringFormat` may write zero bytes in early-exit
- **Location:** lines 11738–11742
- **Problem:** `destBuffer[0] = 0;` is written before format-checks.
  If `format == NULL`, `FPL__CheckArgumentNull` returns 0 *before*
  this line — OK. But if `maxDestBufferLen == 0` and
  `destBuffer != NULL`, the assignment is performed *and then* the
  `FPL__CheckArgumentMin(maxDestBufferLen, 1, 0);` rejects. So we
  wrote into a zero-byte buffer.
- **Fix:** Move the `FPL__CheckArgumentMin(maxDestBufferLen, 1, 0)`
  before the `destBuffer[0] = 0;` write.

### 5.11 Win32 path APIs miss long-path handling
- **Location:** all `wchar_t filePathWide[FPL_MAX_PATH_LENGTH];`
  buffers in Win32 file/path code (16027, 16043, 16059, etc.)
- **Problem:** `FPL_MAX_PATH_LENGTH = MAX_PATH * 2 = 520`. Win32 long
  paths (`\\?\…`, opt-in long path support) can be up to ~32767
  chars. Silent truncation when called with long paths.
- **Fix:** Either:
  - Document that paths must be ≤ FPL_MAX_PATH_LENGTH.
  - Switch the path-conversion buffers to dynamic allocation when the
    UTF-8 source is over a threshold.
  - For Win32, prepend `\\?\` automatically and use the Unicode
    long-path forms of CreateFileW etc.

### 5.12 `fplOSGetVersionInfos` / `fplVersionInfo` static-buffer
  overflow risk
- **Location:** lines 4363–4382
- **Problem:** `fplVersionNumberPart` is exactly 5 chars (`"4 + 1"`).
  Version strings such as build numbers ≥ 10000 (`"19045"`,
  Windows 10 22H2) need 5 + NUL = 6 chars. The struct truncates the
  string silently on populate.
- **Fix:** Increase to `[6 + 1]` or define
  `FPL_MAX_VERSION_PART_LENGTH (8)`.

### 5.13 `fplStringFormatArgs` uses argList copy but copies before format check
- **Location:** lines 11735–11741
- **Problem:** `va_copy(listCopy, argList);` is performed before
  `FPL__CheckArgumentMin(maxDestBufferLen, 1, 0);` returns 0 on bad
  destination. The copy must be matched by `va_end(listCopy)`. Early
  return path skips it.
- **Fix:** Move the `va_copy` after the early returns, or call
  `va_end(listCopy)` before the early return.

### 5.14 `fplDirectoryListBegin` re-uses null-pointer filter
- **Location:** lines 19059–19061
- **Problem:** When `filter == NULL`, the code reassigns
  `filter = "*"` (string literal). Later stored in
  `entry->internalRoot.filter` — fine. But the function also
  silently rewrites the caller's "no filter" intention into "*", which
  may surprise callers that introspect the entry. Document this
  behavior.

---

## 6. Documentation / Comments

### 6.1 Outdated TODO comment about X11
- **Location:** line 3349
- **Problem:** `// @TODO(final): Opaque X11 Display is not correct, ...`
  Has been there for several versions. Either fix or remove.

### 6.2 Doxygen tag inconsistencies
- `fplPathNormalize` doc uses `@param sourcePath` (line 7330) instead
  of `@param[in] sourcePath` like neighbours.
- `fplDateTimeCreate` doc uses `@param year[in]` (line 3666) — the
  Doxygen-friendly form is `@param[in] year`. Both forms parse, but
  consistency helps tooling.
- **Fix:** Mass-format `@param` tags into `@param[in]` / `@param[out]`.

### 6.3 Misleading comment on `fpl__PushError_Formatted` thread-safety
- **Location:** lines 190–191 (changelog) say "Improved thread-safety in
  the event system" but the error-state ring buffer remains unguarded.
  Update the changelog or fix as per 2.9.

### 6.4 Min sizes in opaque handle comments contradict actual buffer sizes
- See 4.1.

### 6.5 `fplFileReadBlock` doc "Returns the number of bytes read or zero" is ambiguous
- Zero may be EOF *or* error. Document explicitly that the caller
  must use `fplFileGetPosition()` / `fplFileGetSize()` to disambiguate
  or, better, change the API to return `size_t` plus a `bool *outEOF`.

---

## 7. Suggested Workplan

Order of attack (highest impact first):

1. **Fix data-corruption bugs first (Section 1).**
   `fplFileCopy`, `fplFileMove`, `fplFileReadBlock*`,
   `fplFileGetPosition64`, POSIX `fplGetExecutableFilePath`,
   POSIX `fplGetHomePath`, opaque `fpl__X11Window`, POSIX UTF-8
   conversion.

2. **Buffer-bounds correctness (Section 1.3, 1.11, 2.4, 5.2).**
   Even when the data is right, missing bounds checks are unbounded
   writes.

3. **Thread-safety (Section 2.9).**
   Required before any production use of audio + worker threads.

4. **Public API contract overhaul (Section 2.3, 3.1).**
   Single biggest "breaking change" remaining for 1.0. Best done now
   while the version bump is fresh.

5. **Type/Opaque review (Section 4).**
   Add `fplStaticAssert` size guards in the non-opaque path so the
   opaque path can never silently mis-size a kernel structure.

6. **Documentation cleanup (Section 6).**
   Mostly mechanical; do as the last sweep before release.

---

## 8. Quick Reference Checklist (for todo generation)

- [ ] **CRITICAL** Fix `fplFileCopy` source/target swap (16365–16366)
- [ ] **CRITICAL** Fix `fplFileMove` source/target swap (16376–16377)
- [ ] **CRITICAL** Fix `fplFileGetPosition64` returning hardcoded 0 (16228)
- [ ] **CRITICAL** Accumulate `result` in `fplFileReadBlock64` and
      `fplFileWriteBlock64` (16108, 16155)
- [ ] **CRITICAL** Honour `maxTargetBufferSize` in
      `fplFileReadBlock32`/`64`
- [ ] **CRITICAL** POSIX `fplGetExecutableFilePath` readlink
      NUL-termination + return-value check (19139)
- [ ] **CRITICAL** POSIX `fplGetHomePath` getpwuid NULL check (19164)
- [ ] **CRITICAL** Opaque `fpl__X11Window` width fix (3354)
- [ ] **CRITICAL** POSIX `wcstombs`/`mbstowcs` non-NUL input UB +
      missing failure check (19320–19345)
- [ ] **CRITICAL** Win32 `WideCharToMultiByte` int cast / error check (16726–16749)
- [ ] **CRITICAL** Win32 `fplFileClose` valid-handle test (16241)
- [ ] **MAJOR** `fplGetStringLength` use `size_t` counter (11700)
- [ ] **MAJOR** `fplStringFormatArgs` truncation/error semantics + always-NUL (11732–11766)
- [ ] **MAJOR** Switch `fplCopyString*`, `fplStringAppend*`, `fplEnforcePathSeparator*`
      to return `size_t` + null-dest support (6683–6720, 11679–11730)
- [ ] **MAJOR** `fplStringAppendLen` off-by-one (11689)
- [ ] **MAJOR** `fplPathCombine` two-pass + va_end (12798–12834)
- [ ] **MAJOR** `fplStringToS32Len` overflow + ambiguous error (11825–11850)
- [ ] **MAJOR** `fplFileGetSizeFromPath32`/`Handle32` failure detection (16249–16289)
- [ ] **MAJOR** `fplFileSetPosition32` `INVALID_SET_FILE_POINTER` check (16165)
- [ ] **MAJOR** Thread-safe error ring (`fpl__PushError_Formatted`) (11343–11374)
- [ ] **MAJOR** MSVC `vsnprintf` -1 fallback path (11752)
- [ ] **MAJOR** `fplDirectoryListBegin` lifetime — copy path/filter into entry
- [ ] **MAJOR** `fplPathNormalize` return required size on overflow
- [ ] **API** Rename `Compability` → `Compatibility`
- [ ] **API** Fix date-time parameter doc ranges (3670, 3671)
- [ ] **API** Fix `fplStrngFormat` typo in changelog
- [ ] **API** Document/standardise output-buffer convention
- [ ] **API** Decide on `fpl_b32` vs `bool` consistency
- [ ] **TYPE** Add `fplStaticAssert` for every opaque vs real
      handle's size (Win32 + POSIX + X11)
- [ ] **TYPE** Make X11 opaque types incomplete-struct typedefs
- [ ] **TYPE** Decide pre-1970 support for `fplDateTime` (epoch sign)
- [ ] **STAB** `fplExtractFilePath` use `size_t` for pathLen (12677)
- [ ] **STAB** Win32 `fplGetExecutableFilePath` long-path handling
- [ ] **STAB** Win32 `fplGetHomePath` HRESULT check
- [ ] **STAB** POSIX `fplDateTimeQuery` use `localtime_r` + handle errors
- [ ] **STAB** `fplPathCombine` skip duplicate separators
- [ ] **STAB** `fplFormatDateTime` add `fpl_platform_api` to def (19221)
- [ ] **STAB** Long-path / `\\?\` Win32 support for `FPL_MAX_PATH_LENGTH`
- [ ] **STAB** `fplVersionNumberPart` extend to 8 chars
- [ ] **STAB** `fplStringFormat` move zero-buffer write past arg checks
- [ ] **DOC** Remove or address X11 Display TODO (3349)
- [ ] **DOC** Normalise `@param[in]/@param[out]` everywhere
- [ ] **DOC** Update opaque-handle size comments to match buffers / real types
- [ ] **DOC** Disambiguate `fplFileReadBlock` "0 = EOF or error" return
