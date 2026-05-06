# FPL Security & Stability TODO

**Source:** `analysis/fpl-code-security-analysis.md`
**Target file:** `final_platform_layer.h`
**Approach:** Iterative phases. Critical → Major → API/Type/Stability → Docs.

## Global Conventions

These rules apply to every task that touches a string/buffer copy, format, or append function.

- Functions that return a `*` pointer for buffer ops must be converted to return `size_t` — the **total characters required** (excluding NUL).
- If `dest == NULL` or `maxDestLen == 0`: return required size, do NOT write.
- If buffer too small (`required > maxDestLen`): return `0`. Do NOT leave partial buffer. Use the matching `FPL__CheckArgument*` macro.
- If `dest != NULL && maxDestLen > 0`: always NUL-terminate.
- Return `0` for hard errors (NULL input, format error). Document this contract in each function's doxygen block.
- Leave functions as it is and just fix the underlaying issues, with as less changes as possible and do not allocate heap memory or use large stack memory.
- Returning a value directly is never intented, store it first into a local variable called "result" and return this instead.

Functions impacted (full list):
- `fplCopyString`, `fplCopyStringLen`
- `fplStringAppend`, `fplStringAppendLen`
- `fplEnforcePathSeparator`, `fplEnforcePathSeparatorLen`
- `fplStringFormat`, `fplStringFormatArgs` (already `size_t`, fix semantics only)
- `fplPathCombine`, `fplPathNormalize` (already `size_t`, fix overflow return)

---

## Phase 1 — Critical (data corruption / crashes / security)

Goal: stop active corruption first. Each task: edit → build → run `FPL_Test` → commit.

### 1.1 Win32 file ops — source/target swap
- [x] Fix `fplFileCopy` (16365–16366): pass `targetFilePath` to second `fplUTF8StringToWideString`.
- [x] Fix `fplFileMove` (16376–16377): same swap fix.
- [ ] Smoke test: copy + move file in `FPL_Test`. (deferred — Win32-only path, not testable on Linux dev box)

### 1.2 Win32 `fplFileGetPosition64`
- [x] Replace `return 0;` → `return result;` (16228).

### 1.3 Win32 `fplFileReadBlock64` / `fplFileWriteBlock64` accumulation
- [x] `result += bytesRead;` (Win32 + POSIX).
- [x] `result += bytesWritten;` (Win32 + POSIX).
- [x] Break loop on `bytesRead == 0` / `bytesWritten == 0` to avoid infinite loop.

### 1.4 Bound `maxTargetBufferSize` in `fplFileReadBlock32` / `fplFileReadBlock64`
- [x] Clamp `actualToRead = min(sizeToRead, maxTargetBufferSize)` before reading.
- [x] Apply Win32 + POSIX variants.
- [x] Add `FPL__CheckArgumentZero(maxTargetBufferSize, 0)` guard on read functions.

### 1.5 Win32 `fplFileClose` invalid handle guard
- [x] Added `FPL__WIN32_IS_VALID_FILE_HANDLE` macro covering both NULL and INVALID_HANDLE_VALUE.
- [x] Applied across all Win32 file functions and `fplFileEntry` directory iteration.

### 1.6 POSIX `fplGetExecutableFilePath`
- [x] Capture `readlink` return as `ssize_t n`.
- [x] Treat `n <= 0` as failure.
- [x] NUL-terminate `buf[n] = '\0';`.
- [x] Use `n` instead of `fplGetStringLength`.

### 1.7 POSIX `fplGetHomePath`
- [x] After `getpwuid`, check `userPwd != fpl_null && userPwd->pw_dir != fpl_null`.
- [x] Return 0 on NULL.

### 1.8 Opaque `fpl__X11Window` width
- [x] Changed to `typedef unsigned long fpl__X11Window;` (line 3354).
- [x] Updated size comment.

### 1.9 POSIX UTF-8 conversion (`STD_STRINGS`)
- [x] Use NUL-terminated temp copy of input (stack buf if fits, else heap).
- [x] Check `(size_t)-1` failure → push `FPL__ERROR` + return 0.
- [x] Applied to both `wcstombs` and `mbstowcs` paths.

### 1.10 Win32 UTF-8 conversion size cast
- [x] Clamp `maxUtf8DestLen`/`maxWideDestLen` to `INT_MAX` before passing to API.
- [x] Reject input length > INT_MAX with FPL__ERROR.
- [x] Check return value `<= 0` → push error, return 0.

**Phase 1 exit criteria:** `FPL_Test` builds clean. Linux-side (POSIX/X11) tests pass through 1.x. Win32-only fixes verified by code review only on this host.

---

## Phase 2 — Major (correctness / API contract)

### 2.1 String length / format core fixes
- [x] `fplGetStringLength`: counter is now `size_t`.
- [x] `fplStringFormatArgs`:
  - [x] On too-small buffer: return `0` (per user policy: no partial buffers).
  - [x] On NULL dest: return required size (query mode).
  - [x] Always NUL-terminate at `destBuffer[requiredLen]` when fits, or `destBuffer[0] = 0` on too-small.
  - [x] Fixed error message to `"Format parameter '%s' is invalid"`.
  - [x] `va_copy` paired with `va_end` on every path.
- [x] `fplStringFormat`: `FPL__CheckArgumentMin` runs before any buffer write.

### 2.2 Convert string-buffer functions to `size_t` return
Applied Global Conventions; updated all docstrings and call sites.

- [x] `fplCopyString` → `size_t` return.
- [x] `fplCopyStringLen` → `size_t` return.
- [x] `fplStringAppend` → `size_t` return.
- [x] `fplStringAppendLen` → `size_t` return + off-by-one fix.
- [x] `fplEnforcePathSeparator` → `size_t` return.
- [x] `fplEnforcePathSeparatorLen` → `size_t` return.
- [x] `dest == NULL` allowed (returns required size) for all of the above.
- [x] Buffer too small returns `0` via `FPL__CheckArgumentMin`.
- [x] `fplGetWindowTitle` updated to `size_t`.
- [x] `fpl__PushError_Formatted` already discards return — no change required.

### 2.3 `fplPathCombine` two-pass + va_list hygiene
- [x] Pass 1 computes total required length (no writes).
- [x] Pass 2 writes the buffer.
- [x] `va_end(vargs)` reached on every return path.
- [x] Duplicate separators skipped (path already ending in `FPL_PATH_SEPARATOR` does not get an extra one inserted).

### 2.4 `fplPathNormalize` return required size on overflow
- [x] Decision: keep returning 0 on too-small per user policy. NULL-dest query mode returns required size — that is the supported way to size the buffer.
- [ ] (Optional) Win32 `GetFullPathNameW` query path — covered by NULL-dest contract; revisit if Win32 needs special handling.

### 2.5 `fplStringToS32Len` overflow + ambiguous failure
- [x] Overflow detection (clamp to INT32_MAX/INT32_MIN).
- [x] Added `bool fplTryStringToS32Len(...)` and `bool fplTryStringToS32(...)` for unambiguous parsing.
- [x] Leading whitespace + `+` sign handled.
- [x] `fplStringToS32` delegates through `fplTryStringToS32`.

### 2.6 Win32 file size / position error detection
- [ ] `fplFileGetSizeFromPath32` / `…Handle32`: switch to `GetFileSizeEx`. Add `bool fplFileTryGetSize…(…, uint64_t *outSize);` variant.
- [ ] `fplFileSetPosition32`: use Microsoft's two-step `INVALID_SET_FILE_POINTER` + `GetLastError() != NO_ERROR` check, or switch to `SetFilePointerEx`.

### 2.7 Thread-safe error ring (`fpl__PushError_Formatted`) (11343–11374)
- [ ] Add `fplMutexHandle fpl__global__LastErrorMutex` initialized at `fplPlatformInit`.
- [ ] Lock around index increment + slot write.
- [ ] Optional: track `dropped` counter for ring overflow (5.9).

### 2.8 MSVC pre-C99 vsnprintf hazard (11752)
- [ ] When `charCount < 0` on MSVC ≤ 1900: fall back to `_vscprintf(format, listCopy)` to obtain required size.
- [ ] Or use `vsnprintf_s`.

### 2.9 `fplDirectoryListBegin` lifetime fix (19052–19068)
- [ ] Copy `path` and `filter` into fixed buffers inside `fplFileEntry` (mirror `name[FPL_MAX_FILENAME_LENGTH]`).
- [ ] Caller may now free strings safely after `Begin`.

### 2.10 Audio backend chunk write audit (~26698)
- [ ] Add `fplAssert(read == frameCount)` after every internal `fpl__ReadAudioFramesFromClient` call.
- [ ] Remove conditional logic that branches on partial-read return.

**Phase 2 exit criteria:** all `FPL_Test`, `FMEM_Test`, `FXML_Test`, `FOGL_Test` pass. Audio demos run without buffer underruns.

---

## Phase 3 — API Consistency

### 3.1 Rename typo `Compability` → `Compatibility`
- [ ] Rename enum `fplOpenGLCompabilityFlags` → `fplOpenGLCompatibilityFlags`.
- [ ] Rename enum values `_Legacy`, `_Core`, `_Compatibility`, `_Forward`.
- [ ] Rename `fplOpenGLSettings::compabilityFlags` → `compatibilityFlags`.
- [ ] Update all callers in demos.
- [ ] Optionally provide back-compat aliases (1.0 boundary — recommended to drop).

### 3.2 Date-time doc range fixes (3670, 3671)
- [ ] `@param minute` 0-23 → 0-59.
- [ ] `@param second` "The minute…" → "The second in range of 0-59".

### 3.3 Changelog typo (392–393, 766–767)
- [ ] `fplStrngFormat` → `fplStringFormat`.

### 3.4 Output-buffer convention documented
- [ ] Add a top-of-file convention block describing the Global Conventions above.
- [ ] Reference from each affected function's doxygen.

### 3.5 `fpl_b32` vs `bool`
- [ ] `fplFileHandle::isValid` → `bool`.
- [ ] Audit other public structs; reserve `fpl_b32` for atomic-friendly ABI structs only.

### 3.6 `fplFileGetSizeFromPath` family — try-style API
- [ ] Add `bool fplFileTryGetSizeFromPath(const char *path, uint64_t *outSize);` (resolves 3.7).

### 3.7 `fplStringFormat` doc cleanup (6751, 6762)
- [ ] Drop "wrapper to vsnprintf" claim. Replace with "Follows C99 vsnprintf format specifiers".

---

## Phase 4 — Type / Opaque-Handle Hardening

### 4.1 Static assertions for opaque-vs-real handle sizes
- [ ] Add `fplStaticAssert(sizeof(pthread_mutex_t) <= sizeof(fpl__POSIXMutexHandle));` (and equivalents) in non-opaque code path.
- [ ] Same for `pthread_cond_t`, `sem_t`, `pthread_t`, `GUID`, `Window`.

### 4.2 X11 opaque types as incomplete structs
- [ ] `typedef struct fpl__X11Display fpl__X11Display;` (and Visual / GC / Image).
- [ ] Update size comments → "opaque incomplete type, accessed via pointer".

### 4.3 Opaque handle size comments (3300, 3338–3343, 3340)
- [ ] Replace each `min N bytes` comment with the real maximum across supported libcs.

### 4.4 `fpl__POSIXThreadHandle` portability (3337, 3417)
- [ ] Either expand to `uint8_t buffer[16]` with `fplStaticAssert(sizeof(pthread_t) <= 16)`,
- [ ] Or document constraint that opaque branch is glibc/musl-x86_64 only.

### 4.5 `fplDateTime::epoch` signedness (3619–3626)
- [ ] Decide: change `uint64_t epoch` → `int64_t` (preferred — matches `time_t`),
- [ ] Or document pre-1970 unsupported and reject in `fplDateTimeCreate`.

---

## Phase 5 — Other Stability / Correctness

### 5.1 `fplExtractFilePath` int → size_t (12677)
- [ ] `int pathLen` → `size_t pathLen`. Drop `(int)` cast.

### 5.2 Win32 `fplGetExecutableFilePath` truncation (16540–16546)
- [ ] Capture `GetModuleFileNameW` return value.
- [ ] If `result < MAX_PATH`: NUL-terminate.
- [ ] Else retry with growing dynamic buffer until `result < capacity`.

### 5.3 Win32 `fplGetHomePath` HRESULT check (16548–16556)
- [ ] Check `SUCCEEDED(hr)`; on failure return 0.

### 5.4 POSIX `fplDateTimeQuery` (19201–19219)
- [ ] Check `gettimeofday` return.
- [ ] Use `localtime_r` (thread-safe).
- [ ] Check `localtime_r` NULL.

### 5.5 POSIX `fplDirectoriesCreate` separator (18954–18992)
- [ ] Treat both `/` and `\\` as separators.

### 5.6 POSIX `fplFormatDateTime` API tag (19221)
- [ ] Add `fpl_platform_api` to implementation.

### 5.7 POSIX `fplDirectoryListNext` loop refactor (19070–19094)
- [ ] Refactor to single `for (;;) { dp = readdir(...); ... }` loop.

### 5.8 Win32 long-path support (`\\?\`)
- [ ] Decide: document `FPL_MAX_PATH_LENGTH` cap, OR
- [ ] Switch path-conversion buffers to dynamic alloc when source > threshold + prepend `\\?\`.

### 5.9 `fplVersionNumberPart` extend
- [ ] Increase to `[8 + 1]` or define `FPL_MAX_VERSION_PART_LENGTH (8)`.

### 5.10 `fplDirectoryListBegin` filter behavior doc (19059–19061)
- [ ] Document that NULL filter is rewritten to `"*"` and stored as such.

---

## Phase 6 — Documentation & Comments

### 6.1 Outdated TODO (3349)
- [ ] Address or remove the X11 Display TODO comment.

### 6.2 Doxygen `@param` form
- [ ] Mass-format to `@param[in]` / `@param[out]` everywhere.
- [ ] Specifically: `fplPathNormalize` (7330), `fplDateTimeCreate` (3666).

### 6.3 Changelog thread-safety claim (190–191)
- [ ] Update or fix the "Improved thread-safety in the event system" line to match Phase 2.7.

### 6.4 `fplFileReadBlock` 0-return ambiguity
- [ ] Document explicitly: 0 means EOF or error; caller disambiguates via `fplFileGetPosition` / `fplFileGetSize`.
- [ ] Or: change API to `size_t` + `bool *outEOF` (breaking — defer or include in 1.0 sweep).

---

## Iteration Rhythm

For each task:
1. Read affected lines.
2. Make edit.
3. Build all affected demos (at minimum `FPL_Test`).
4. Run unit tests.
5. Commit with focused message (no `Co-Authored-By` trailer).
6. Move on.

For renames / API breaks (Phase 2.2, 3.1): bundle the rename + all caller updates + test build into one atomic commit.
