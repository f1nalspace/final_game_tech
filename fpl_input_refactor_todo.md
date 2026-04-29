# FPL Input Refactor — Progress TODO

Companion to `fpl_input_proposal_plan.md`. Tracks step-by-step status so a future session can resume cleanly.

Branch: `input-refactoring`
Last verified: builds clean on Linux/POSIX (FPL_Console, FPL_Window, FPL_OpenGL, FPL_Test). FPL_Input verified end-to-end on X11 with a Logitech F310 gamepad — connection + axis/button events flow correctly. Win32 path not re-verified after step 6/7 — needs a Windows build.

## Done

- [x] **Step 1 — Public types + `FPL_NO_INPUT`.** Commit `243b6ce6` ("Introduced a new input API"). Adds `fplInputSourceType`, `fplInputBackendType`, `fplInputBackendMask` (+ inline mask helpers), extended `fplInputSettings`, `fplInputDevice` family, `fplInputBackendSupport`, new public-API declarations + stub impls, extended `fplMouseButtonType` with X1/X2 + `fplMouseState.wheelDeltaY/X`, `FPL_NO_INPUT` + `FPL_NO_INPUT_*` per-backend switches, `FPL__SUPPORT_INPUT*` / `FPL__ENABLE_INPUT*`. Wayland scrubbed from code and plan.
- [x] **Step 2 — Skeleton stubs.** Commit `da7c623d` ("Input system stubs and bare-bone initialization"). Adds `fpl__InputContext`, `fpl__NativeInputEvent[Kind]`, `fpl__InputSystem_Init/Release/Update/HandleNativeEvent/IsEnabled/PollKeyboard/PollMouse/PollGamepad` stubs, wires init after audio and release before audio in `fplPlatformInit` / `fpl__ReleasePlatformStates`. Also reworked init flags: per-source `fplInitFlags_Keyboard`, `fplInitFlags_Mouse`, kept `fplInitFlags_GameController`; `fplInitFlags_Input` is now a composite alias = Keyboard | Mouse | GameController.
- [x] **Step 3 — XInput migration.** Commit `2c0de90b` ("Migrated XInput to new input system"). XInput types moved from `FPL__ENABLE_WINDOW` to `FPL__ENABLE_INPUT_XINPUT`. Renamed `fpl__Win32XInputState` → `fpl__InputBackendXInput` (with `isInitialized`, `api`). Removed `xinput` field from `fpl__Win32AppState`; added it to `fpl__InputContext`. New backend wrappers `fpl__InputBackendXInput_Init/Release/Update/PollGamepad`. `fpl__InputSystem_*` now dispatch to XInput. `fpl__Win32InitPlatform` / `fpl__Win32ReleasePlatform` no longer touch XInput. `fpl__Win32UpdateGameControllers` removed; Win32 `fplWindowUpdate` calls `fpl__InputSystem_Update`. Win32 `fplPollGamepadStates` delegates to `fpl__InputSystem_PollGamepad`. **Temporary:** `FPL__ENABLE_INPUT_XINPUT` requires `FPL__SUPPORT_WINDOW` for now — step 8 lifts this once gamepad event helpers are window-independent.
- [x] **Step 4 — Win32 keyboard/mouse migration.** New `fpl__InputBackendWin32` (in `fpl__InputContext.win32kbm`) with `Init/Release/PollKeyboard/PollMouse/HandleNativeEvent`. Win32 `fplPollKeyboardState` / `fplPollMouseState` delegate to `fpl__InputSystem_PollKeyboard` / `_PollMouse`. WndProc forwards `WM_SYSKEYDOWN/UP`, `WM_KEYDOWN/UP`, `WM_CHAR/SYSCHAR/UNICHAR/DEADCHAR`, `WM_*BUTTON*` (incl. X1/X2), `WM_MOUSEMOVE`, `WM_MOUSEWHEEL`, `WM_MOUSEHWHEEL` via `fpl__InputSystem_HandleNativeEvent` (payload = stack-allocated MSG). Backend handler does the original button/move/wheel work plus `SetCapture`/`ReleaseCapture` (both gated on `disabledEvents` and `enabledSources`). Back-compat tweak in `fpl__PlatformInit`: `fplInitFlags_Window` implies Keyboard+Mouse sources (Gamepad still requires explicit flag) so `FPL_Window` / `FPL_OpenGL` keep getting input. `FPL__ENABLE_INPUT_WIN32` gated on `FPL__SUPPORT_WINDOW` for now (lifted by step 8). `WM_MOUSEHWHEEL` is forwarded but discarded internally (no horizontal axis on `fpl__HandleMouseWheelEvent` yet — step 9). Linux build clean (FPL_Console / FPL_Window / FPL_OpenGL / FPL_Test / FPL_Input). **Win32 verified working** — needed forward declarations for `fpl__InputSystem_*` after the `fpl__InputContext` typedef (Win32 wndproc uses them ~15k lines before their definition at ~31k).
- [x] **Step 5 — X11Kbm migration.** New `fpl__InputBackendX11Kbm` (in `fpl__InputContext.x11kbm`, struct lives in `TYPES_X11`) with `Init/Release/PollKeyboard/PollMouse/HandleNativeEvent`. X11 `fplPollKeyboardState` / `fplPollMouseState` delegate to `fpl__InputSystem_PollKeyboard` / `_PollMouse`. `fpl__X11HandleEvent` collapses `KeyPress/KeyRelease/ButtonPress/ButtonRelease/MotionNotify` into a single forwarder that builds an `fpl__NativeInputEvent{X11Event, ev}` and calls `fpl__InputSystem_HandleNativeEvent`. Backend handler keeps the original `XEventsQueued`/`XPeekEvent` repeat-detection trick on `KeyRelease`. `FocusIn/FocusOut`, `EnterNotify`, `LeaveNotify` stay in window code (focus events aren't input events; enter/leave were never implemented and remain a separate task). `FPL__ENABLE_INPUT_X11` gated on `FPL__SUPPORT_WINDOW` for now (lifted by step 8). Linux build clean (FPL_Console / FPL_Window / FPL_OpenGL / FPL_Test / FPL_Input).
- [x] **Step 6 — Linux joystick migration (lift as-is).** New `fpl__InputBackendLinuxJoystick` (in `fpl__InputContext.linuxJoystick`, struct lives in `TYPES_LINUX` under `FPL__ENABLE_INPUT_LINUX_JOYSTICK`) owns the `fpl__LinuxGameController controllers[FPL__LINUX_MAX_GAME_CONTROLLER_COUNT]` array + `lastCheckTime`. Wrappers `Init/Release/Update/PollGamepad`. Detection/state helper renamed `fpl__LinuxJoystick_PollImpl(settings, backend, useEvents)` — same body, just operates on backend. `fpl__LinuxFreeGameControllers` folded into `_Release`. X11 `fplWindowUpdate` no longer calls the joystick directly; it calls `fpl__InputSystem_Update` (matching Win32). Linux `fplPollGamepadStates` delegates to `fpl__InputSystem_PollGamepad`. `fpl__LinuxReleasePlatform` cleared (release happens via `fpl__InputSystem_Release` before audio). Dropped `controllersState` field from `fpl__LinuxAppState`. **Temporary:** `FPL__ENABLE_INPUT_LINUX_JOYSTICK` gated on `FPL__SUPPORT_WINDOW` for now — step 8 lifts this once gamepad event helpers are window-independent. Linux build clean (FPL_Console / FPL_Window / FPL_OpenGL / FPL_Test).
- [x] **Step 7 — Linux joystick auto-detection + throttling (no udev).** Scan `/dev/input/js0..js31` from `fpl__LinuxJoystick_DetectControllers` (called by `_Update`), throttled by `gameControllers.detectionFrequency`. `_PollGamepad` no longer runs detection — mirrors XInput, just refreshes cached state via `fpl__LinuxJoystick_UpdateStates(false)` and copies. Added `fpl__InputBackendLinuxJoystick.triedSlot[FPL__LINUX_JOYSTICK_SCAN_COUNT]` for one-shot `FPL_LOG_DEBUG` per slot on (a) non-`ENOENT` open failures, (b) zero axes/buttons (virtual nodes), (c) missing init message. `ENOENT` skipped silently. Added `fpl__LinuxGameController.slotIndex` so `ENODEV` in read path can reset `triedSlot[slotIndex]` for hotplug re-announce. Linux build clean (FPL_Console / FPL_Window / FPL_OpenGL / FPL_Test).
- [x] **Bugfix — Linux joystick init-message guard.** Commit `aeb0a3a3`. Pre-existing bug exposed by step 7's scan-walking past virtual js0/js1 to the real F310 on js2: kernel ORs `JS_EVENT_INIT` (0x80) onto `JS_EVENT_AXIS`/`JS_EVENT_BUTTON` for the queued initial events, so `msg.type` is 0x81/0x82 — the strict equality `msg.type == JS_EVENT_AXIS` etc. never matched and every real gamepad failed detection. Fix masks `JS_EVENT_INIT` off before comparing in `fpl__LinuxJoystick_DetectControllers`. Verified with FPL_Input + Logitech F310.

## In progress

_None._

## Pending
- [ ] **Step 8 — Event plumbing.** Gamepad connect/disconnect/state events flow through `fpl__PushGamepadEvent` driven by backends, not the window loop. Verify `fplPollEvents` consumers still see `fplGamepadEvent` correctly across all platforms.
- [ ] **Step 9 — `FPL_NO_WINDOW` input support.** Add hidden `HWND_MESSAGE` window for Win32 (receives `WM_INPUT` / `WM_DEVICECHANGE`); add detached `Display*` for X11. Add `fplInputSettings.detachFromWindow` honoring. Lift the `FPL__SUPPORT_WINDOW` requirement from `FPL__ENABLE_INPUT_XINPUT` (and any other backend gates added in step 4/5/6) — likely requires moving `fpl__PushGamepadConnectionEvent` / `fpl__PushGamepadStateEvent` (and supporting public types) out of the `FPL__ENABLE_WINDOW` block, or splitting the event queue from the window state. Add a console-only demo that polls keyboard/gamepad to verify.
- [ ] **Step 10 — Multi-backend gamepad polling.** Make `fpl__InputSystem_PollGamepad` merge gamepad states across multiple active backends (XInput + future DInput, etc.). Add a unit test in `FPL_Test` that registers a fake second backend and verifies merge behavior.
- [ ] **Step 11 — Linux udev hotplug backend.** Add `fpl__InputBackendLinuxUdev` that opens a udev netlink socket, listens for `add`/`remove` events on the `input` subsystem, and pushes connect/disconnect signals into the `LinuxJoystick` (and later `LinuxEvdev`) backend so it can rebuild its slot list without polling. Falls back gracefully to step 7's polling scan when libudev is missing or `FPL_NO_INPUT_LINUX_UDEV` is set. Runtime-link `libudev.so.1` (no build-time dep). Ensures gamepads plugged in mid-session show up immediately, not after `detectionFrequency`.
- [ ] **Step 12 — New backends + docs.** Add `LinuxEvdev`, `RawInput`, `DInput`, `UnixGamepad` as descriptors + impl structs + function-table entries. Update `page_category_input_config` docs and changelog. (Plan §13 / §11 step 11.)

## Open questions parked from §12 of the plan

- `fplInputDevice.state` union: keep as union or expose pointers/arrays?
- Add a `fplInputBackendType_Custom` slot for user-supplied backends (matches `fplAudioBackendType_Custom`)?
- Expose `fplUpdateInputDevices()` publicly (already declared as a stub) or fold into `fplPollEvents`?

## Cross-platform validation checklist (run before/after each step)

- POSIX/Linux: `gcc -std=c99 -c -I. demos/FPL_Console/fpl_console.c` and FPL_Window / FPL_OpenGL (with `-Idemos/additions`) / FPL_Test (g++ -std=c++11).
- Windows: not buildable from this dev box (no mingw). Needs the user's Windows toolchain. FPL_Input demo is the canonical gamepad/keyboard/mouse smoke test.
- After step 5/6: run X11 demos (FPL_Window, FPL_OpenGL, FPL_Input).
- After step 8: console-only demo that polls keyboard or gamepad without `fplInitFlags_Window`.

## Resume instructions for the next session

1. `git checkout input-refactoring` (already there).
2. Re-read `fpl_input_proposal_plan.md` and this file.
3. Pick the next pending step. Mark it in_progress, implement, build-check on Linux at minimum, commit, mark done.
4. Each step should be one commit (per plan §11).

## Latest state (after step 7)

- Most recent commits on `input-refactoring`:
  - `aeb0a3a3` Fix Linux joystick init-message check
  - `c8174a26` Linux joystick auto-detection + log throttling
  - `d51a86ab` Migrated Linux joystick to new input system
- Linux gamepad path is now usable end-to-end: scan 0..31, throttled detection, cached poll, F310 verified.
- Step 8 is the next planned step — gamepad event helpers must move out of `FPL__ENABLE_WINDOW` before the `FPL__SUPPORT_WINDOW` gates on `FPL__ENABLE_INPUT_XINPUT` / `_WIN32` / `_X11` / `_LINUX_JOYSTICK` can be lifted in step 9.

## Known incidental issues found while testing

- `fplInputBackendMaskIsEnabled` / `_Enable` / `_Disable` are declared `fpl_inline` (= `inline`). With C99 `-O0`, GCC emits no out-of-line definition, producing `undefined reference` errors when linked from a single-translation-unit demo. Workaround: compile with `-O2`. Proper fix later: change `fpl_inline` to `static inline` on these helpers, or provide an extern definition. Not blocking step 8.
