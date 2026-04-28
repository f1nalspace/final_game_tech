# FPL Input Refactor — Progress TODO

Companion to `fpl_input_proposal_plan.md`. Tracks step-by-step status so a future session can resume cleanly.

Branch: `input-refactoring`
Last verified: builds clean on Linux/POSIX (FPL_Console, FPL_Window, FPL_OpenGL, FPL_Test). Win32 path not yet verified — needs a Windows build.

## Done

- [x] **Step 1 — Public types + `FPL_NO_INPUT`.** Commit `243b6ce6` ("Introduced a new input API"). Adds `fplInputSourceType`, `fplInputBackendType`, `fplInputBackendMask` (+ inline mask helpers), extended `fplInputSettings`, `fplInputDevice` family, `fplInputBackendSupport`, new public-API declarations + stub impls, extended `fplMouseButtonType` with X1/X2 + `fplMouseState.wheelDeltaY/X`, `FPL_NO_INPUT` + `FPL_NO_INPUT_*` per-backend switches, `FPL__SUPPORT_INPUT*` / `FPL__ENABLE_INPUT*`. Wayland scrubbed from code and plan.
- [x] **Step 2 — Skeleton stubs.** Commit `da7c623d` ("Input system stubs and bare-bone initialization"). Adds `fpl__InputContext`, `fpl__NativeInputEvent[Kind]`, `fpl__InputSystem_Init/Release/Update/HandleNativeEvent/IsEnabled/PollKeyboard/PollMouse/PollGamepad` stubs, wires init after audio and release before audio in `fplPlatformInit` / `fpl__ReleasePlatformStates`. Also reworked init flags: per-source `fplInitFlags_Keyboard`, `fplInitFlags_Mouse`, kept `fplInitFlags_GameController`; `fplInitFlags_Input` is now a composite alias = Keyboard | Mouse | GameController.
- [x] **Step 3 — XInput migration.** Commit `2c0de90b` ("Migrated XInput to new input system"). XInput types moved from `FPL__ENABLE_WINDOW` to `FPL__ENABLE_INPUT_XINPUT`. Renamed `fpl__Win32XInputState` → `fpl__InputBackendXInput` (with `isInitialized`, `api`). Removed `xinput` field from `fpl__Win32AppState`; added it to `fpl__InputContext`. New backend wrappers `fpl__InputBackendXInput_Init/Release/Update/PollGamepad`. `fpl__InputSystem_*` now dispatch to XInput. `fpl__Win32InitPlatform` / `fpl__Win32ReleasePlatform` no longer touch XInput. `fpl__Win32UpdateGameControllers` removed; Win32 `fplWindowUpdate` calls `fpl__InputSystem_Update`. Win32 `fplPollGamepadStates` delegates to `fpl__InputSystem_PollGamepad`. **Temporary:** `FPL__ENABLE_INPUT_XINPUT` requires `FPL__SUPPORT_WINDOW` for now — step 8 lifts this once gamepad event helpers are window-independent.

## In progress

_None._

## Pending

- [ ] **Step 4 — Win32 keyboard/mouse migration.** Move `fplPollKeyboardState` / `fplPollMouseState` Win32 impls into `fpl__InputBackendWin32`. Route `WM_KEY*`, `WM_*BUTTON*`, `WM_MOUSEMOVE`, `WM_MOUSEWHEEL`, `WM_MOUSEHWHEEL`, `WM_CHAR`, `WM_DEADCHAR` through `fpl__InputSystem_HandleNativeEvent`. WndProc keeps focus/resize/DnD only.
- [ ] **Step 5 — X11Kbm migration.** Move X11 keyboard/mouse polling and event dispatch (currently inside `fpl__X11HandleEvent`) into `fpl__InputBackendX11Kbm`. Forward `KeyPress/Release`, `ButtonPress/Release`, `MotionNotify`, `EnterNotify`, `LeaveNotify`, `FocusIn/Out` via the bridge.
- [ ] **Step 6 — Linux joystick migration.** Lift `/dev/input/jsX` code (`fpl__LinuxGameControllersState`, `fpl__LinuxPollGameControllers`, `fpl__LinuxFreeGameControllers`) into `fpl__InputBackendLinuxJoystick`. Throttle via `gameControllers.detectionFrequency` from `fpl__InputSystem_Update`. Update Linux `fplPollGamepadStates` to delegate.
- [ ] **Step 7 — Event plumbing.** Gamepad connect/disconnect/state events flow through `fpl__PushGamepadEvent` driven by backends, not the window loop. Verify `fplPollEvents` consumers still see `fplGamepadEvent` correctly across all platforms.
- [ ] **Step 8 — `FPL_NO_WINDOW` input support.** Add hidden `HWND_MESSAGE` window for Win32 (receives `WM_INPUT` / `WM_DEVICECHANGE`); add detached `Display*` for X11. Add `fplInputSettings.detachFromWindow` honoring. Lift the `FPL__SUPPORT_WINDOW` requirement from `FPL__ENABLE_INPUT_XINPUT` (and any other backend gates added in step 4/5/6) — likely requires moving `fpl__PushGamepadConnectionEvent` / `fpl__PushGamepadStateEvent` (and supporting public types) out of the `FPL__ENABLE_WINDOW` block, or splitting the event queue from the window state. Add a console-only demo that polls keyboard/gamepad to verify.
- [ ] **Step 9 — Multi-backend gamepad polling.** Make `fpl__InputSystem_PollGamepad` merge gamepad states across multiple active backends (XInput + future DInput, etc.). Add a unit test in `FPL_Test` that registers a fake second backend and verifies merge behavior.
- [ ] **Step 10/11 — New backends + docs.** Add `LinuxEvdev`, `LinuxUdev`, `RawInput`, `DInput`, `UnixGamepad` as descriptors + impl structs + function-table entries. Update `page_category_input_config` docs and changelog. (Plan §13 / §11 step 11.)

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
