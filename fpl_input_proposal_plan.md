# FPL Input Backend Refactoring Proposal

Status: Proposal
Target file: `final_platform_layer.h`
Scope: Decouple input handling from the window subsystems (Win32 / X11) and introduce a formal input backend system modeled after the existing audio backend system.

---

## 1. Motivation

Today, input handling in FPL is entangled with the window subsystems:

- Gamepads (XInput on Win32, `/dev/input/jsX` on Linux) live inside `fpl__Win32AppState` / `fpl__LinuxAppState` and are only compiled in behind `FPL__ENABLE_WINDOW`.
- `fplPollKeyboardState` / `fplPollMouseState` / `fplPollGamepadStates` are implemented directly against `Win32Api` or `X11Api` handles stored inside the window state.
- Gamepad detection runs from inside the window event loop (`fpl__Win32UpdateGameControllers`, `fpl__LinuxPollGameControllers` called from `fpl__X11HandleEvent` neighborhood).
- `fplInputSettings` is small and only covers game controllers + an `disabledEvents` flag; there is no user-facing way to choose, enable or disable individual input sources.
- Linux gamepad code is locked to the joystick node range, scans every second, and cannot be extended to evdev/udev easily.

Consequences:

- `FPL_NO_WINDOW` kills input entirely, even though polling XInput / `/dev/input/js0` / raw Win32 keyboard does not require a window.
- Running XInput **and** DirectInput side by side is impossible — there is only one gamepad code path per platform.
- Adding DirectInput, evdev or udev input requires touching windowing internals.
- Event dispatching from the window backends cannot be cleanly forwarded to a neutral input layer.

## 2. Goals

1. Separate "input" into its own subsystem with its own on/off switch (`FPL_NO_INPUT`), mirroring `FPL_NO_AUDIO`.
2. Make input independent of the window subsystem — allow `FPL_NO_WINDOW` together with full input support.
3. Support **multiple input backends running at once** (e.g. XInput + DirectInput + RawInput on Windows).
4. Intelligent mapping of controllers, to not conflict against each other input system.
5. Allow user-controlled enable/disable of individual backends through `fplInputSettings`.
6. Define a small, uniform backend interface with: load, init, release, device enumeration, per-device polling, event handling.
7. Let the window subsystem **forward** native events to the input subsystem (for device-hotplug notification, keyboard/mouse polling shortcuts, etc.) instead of implementing input itself.
8. Stay simple: fewer layers than the audio backend, no async main loop, no threading requirement.
9. Keep the public API (`fplPollKeyboardState`, `fplPollMouseState`, `fplPollGamepadStates`, `fplEvent`, `fplKeyboardEvent`, `fplMouseEvent`, `fplGamepadEvent`) source-compatible.
10. Extent the public event API to introduce a device-id that is unique across all input systems.

## 3. Non-Goals

- No new high-level input mapping / action system.
- No new haptics / rumble API (reserved for later; keep the door open in feature flags).
- No change to `fplKey` mapping tables.
- No public backend-specific structs in the header — backends stay fully internal.

---

## 4. High-Level Architecture

```
+-----------------------------------------------------------+
|                 fpl (public API, unchanged)               |
|    fplPollKeyboardState / fplPollMouseState /             |
|    fplPollGamepadStates / fplPollEvents                   |
+-----------------------------------------------------------+
                         |
                         v
+--------------------------------------------------------------------+
|                   fpl__InputSystem                                 |
|   - list of active backends                                        |
|   - aggregated device table                                        |
|   - event router                                                   |
+--------------------------------------------------------------------+
    |              |              |              |               |
    v              v              v              v               v
 XInput         DirectInput    RawInput      X11/Evdev/...     Win32
 backend         backend        backend       backend         backend
 (fplInputBackend instance, one per type, isolated data)
```

- `fpl__InputSystem` is owned by `fpl__PlatformAppState` at the same level as the audio context — not inside `fpl__PlatformWindowState`.
- Each backend is a self-contained struct instance with its own runtime-loaded API table and its own device list.
- The public polling functions iterate over all active backends that advertise support for the requested input type and merge results into the caller's output struct.
- Windowing code is reduced to: "translate native messages into neutral input events and call `fpl__InputSystem_PushEvent`".

---

## 5. New Public Types and Settings

All of this lives next to the existing `fplInputSettings` block (around line 5496 of `final_platform_layer.h`).

### 5.1 Input Source Type

```c
typedef enum fplInputSourceType {
    fplInputSourceType_None    = 0,
    fplInputSourceType_Keyboard = 1 << 0,
    fplInputSourceType_Mouse    = 1 << 1,
    fplInputSourceType_Gamepad  = 1 << 2,
    fplInputSourceType_All      = fplInputSourceType_Keyboard
                                | fplInputSourceType_Mouse
                                | fplInputSourceType_Gamepad,
} fplInputSourceType;
FPL_ENUM_AS_FLAGS_OPERATORS(fplInputSourceType);
```

### 5.2 Input Backend Type

```c
typedef enum fplInputBackendType {
    fplInputBackendType_None = 0,
    fplInputBackendType_Auto,         // choose sensible defaults per platform

    // Cross-platform / vendor APIs
    fplInputBackendType_XInput,       // gamepads (Windows)
    fplInputBackendType_DInput,       // gamepads (DirectInput8, Windows)
    fplInputBackendType_RawInput,     // keyboard/mouse via RAWINPUT (Windows)

    // Linux / Unix
    fplInputBackendType_LinuxJoystick,// /dev/input/jsX (legacy joydev)
    fplInputBackendType_LinuxEvdev,   // /dev/input/eventX (modern)
    fplInputBackendType_LinuxUdev,    // hotplug notifications (companion)
    fplInputBackendType_UnixGamepad,  // BSD / generic uhid

    // X11 bridge
    fplInputBackendType_X11Kbm,       // keyboard/mouse via X11

    // Win32 fallback (keyboard/mouse via WM_* + GetKeyState)
    fplInputBackendType_Win32,

    fplInputBackendType_First = fplInputBackendType_None,
    fplInputBackendType_Last  = fplInputBackendType_Win32,
} fplInputBackendType;
```

### 5.3 Backend Enable Mask

A bitmask indexed by `fplInputBackendType`. Keeps it simple and allows the user to disable a single backend without touching the others.

```c
#define FPL_MAX_INPUT_BACKEND_COUNT 16

typedef struct fplInputBackendMask {
    // One bit per backend type. Default: all bits set. Backends that the
    // platform does not support are simply ignored at init time.
    uint32_t bits;
} fplInputBackendMask;

fpl_common_api bool fplInputBackendMaskIsEnabled(const fplInputBackendMask *mask, fplInputBackendType type);
fpl_common_api void fplInputBackendMaskEnable   (fplInputBackendMask *mask, fplInputBackendType type);
fpl_common_api void fplInputBackendMaskDisable  (fplInputBackendMask *mask, fplInputBackendType type);
```

### 5.4 Extended `fplInputSettings`

```c
typedef struct fplInputSettings {
    // Existing fields (kept for compatibility)
    fplGameControllersSettings gameControllers;
    fpl_b32 disabledEvents;

    // New fields
    fplInputSourceType enabledSources;   // default: fplInputSourceType_All
    fplInputBackendMask enabledBackends; // default: all bits set
    fpl_b32 allowMultipleGamepadBackends;// default: true -> XInput + DInput together
    fpl_b32 pollFromWindowEventLoop;     // default: true  -> pipe X11/Win32 events
    fpl_b32 detachFromWindow;            // default: false -> true needed for FPL_NO_WINDOW
} fplInputSettings;
```

### 5.5 `fplSettings` stays unchanged structurally — still has `fplInputSettings input;`.

### 5.6 New Init Flag

```c
typedef enum fplInitFlags {
    ...
    fplInitFlags_GameController = 1 << 4, // retained as an alias for Input + gamepad source
    fplInitFlags_Input          = 1 << 5, // NEW: bring up input subsystem
    ...
} fplInitFlags;
```

Behavior:

- `fplInitFlags_Input` initializes the input system regardless of `fplInitFlags_Window`.
- `fplInitFlags_GameController` implies `fplInitFlags_Input` with `enabledSources |= fplInputSourceType_Gamepad` (for back-compat).
- New compile switch `FPL_NO_INPUT` disables the whole thing just like `FPL_NO_AUDIO` does.
- Per-backend compile switches: `FPL_NO_INPUT_XINPUT`, `FPL_NO_INPUT_DINPUT`, `FPL_NO_INPUT_RAWINPUT`, `FPL_NO_INPUT_LINUX_JOYSTICK`, `FPL_NO_INPUT_LINUX_EVDEV`, `FPL_NO_INPUT_X11`, `FPL_NO_INPUT_WIN32`.

### 5.7 Public Device Struct

```c
#define FPL_MAX_INPUT_DEVICE_NAME 64

typedef struct fplInputDeviceGuid {
    uint8_t bytes[16];
} fplInputDeviceGuid;

typedef enum fplInputDeviceFeatureFlags {
    fplInputDeviceFeatureFlags_None        = 0,
    fplInputDeviceFeatureFlags_Polling     = 1 << 0,
    fplInputDeviceFeatureFlags_Events      = 1 << 1,
    fplInputDeviceFeatureFlags_Hotplug     = 1 << 2,
    fplInputDeviceFeatureFlags_Rumble      = 1 << 3, // reserved
    fplInputDeviceFeatureFlags_Battery     = 1 << 4, // reserved
} fplInputDeviceFeatureFlags;
FPL_ENUM_AS_FLAGS_OPERATORS(fplInputDeviceFeatureFlags);

typedef enum fplInputConnectionState {
    fplInputConnectionState_Unknown = 0,
    fplInputConnectionState_Disconnected,
    fplInputConnectionState_Connected,
} fplInputConnectionState;

typedef struct fplInputDevice {
    fplInputDeviceGuid         guid;
    char                       name[FPL_MAX_INPUT_DEVICE_NAME];
    uint32_t                   index;       // backend-local index
    fplInputSourceType         sourceType;  // exactly one bit set
    fplInputBackendType        backend;     // which backend owns it
    fplInputDeviceFeatureFlags featureFlags;
    fplInputConnectionState    connection;
    union {
        fplKeyboardState keyboard;
        fplMouseState    mouse;
        fplGamepadState  gamepad;
    } state;
} fplInputDevice;
```

### 5.8 Backend Support Query

```c
typedef struct fplInputBackendSupport {
    fplInputBackendType type;
    const char         *name;
    fplInputSourceType  supportedSources;
    fpl_b32             supportsEvents;
    fpl_b32             supportsPolling;
    fpl_b32             supportsHotplug;
} fplInputBackendSupport;

fpl_common_api uint32_t fplGetInputBackendSupport(
    fplInputBackendSupport *outSupports, uint32_t maxCount);

fpl_common_api bool fplGetInputBackendSupportByType(
    fplInputBackendType type, fplInputBackendSupport *outSupport);
```

### 5.9 Device Enumeration API

```c
fpl_common_api uint32_t fplGetInputDevices(
    fplInputSourceType sourceFilter,
    fplInputDevice *outDevices,
    uint32_t maxDevices);

fpl_common_api bool fplFindInputDevice(
    const fplInputDeviceGuid *guid,
    fplInputDevice *outDevice);

fpl_common_api uint32_t fplGetKeyboardDevices(fplInputDevice *out, uint32_t max);
fpl_common_api uint32_t fplGetMouseDevices   (fplInputDevice *out, uint32_t max);
fpl_common_api uint32_t fplGetGamepadDevices (fplInputDevice *out, uint32_t max);
```

`fplPollKeyboardState` / `fplPollMouseState` / `fplPollGamepadStates` keep the same signatures and become thin wrappers that ask the input system to merge states across enabled backends.

### 5.10 Mouse: Five Buttons + Wheel

Extend the existing `fplMouseButtonType` from 3 to 5 buttons and add wheel deltas to `fplMouseState`:

```c
typedef enum fplMouseButtonType {
    fplMouseButtonType_None   = -1,
    fplMouseButtonType_Left   = 0,
    fplMouseButtonType_Right  = 1,
    fplMouseButtonType_Middle = 2,
    fplMouseButtonType_X1     = 3, // back
    fplMouseButtonType_X2     = 4, // forward
    fplMouseButtonType_MaxCount,
} fplMouseButtonType;

typedef struct fplMouseState {
    fplButtonState buttonStates[fplMouseButtonType_MaxCount];
    int32_t x;
    int32_t y;
    float   wheelDeltaY; // vertical wheel
    float   wheelDeltaX; // horizontal wheel
} fplMouseState;
```

---

## 6. Internal Architecture

### 6.1 Function Table

Modeled after `fplAudioBackendFunctionTable` but considerably smaller.

```c
struct fpl__InputContext;
struct fpl__InputBackend;

#define FPL_INPUT_BACKEND_LOAD_FUNC(name)    bool name(struct fpl__InputContext *ctx, struct fpl__InputBackend *backend)
#define FPL_INPUT_BACKEND_UNLOAD_FUNC(name)  void name(struct fpl__InputContext *ctx, struct fpl__InputBackend *backend)

#define FPL_INPUT_BACKEND_INIT_FUNC(name)    bool name(struct fpl__InputContext *ctx, struct fpl__InputBackend *backend, const fplInputSettings *settings)
#define FPL_INPUT_BACKEND_RELEASE_FUNC(name) void name(struct fpl__InputContext *ctx, struct fpl__InputBackend *backend)

#define FPL_INPUT_BACKEND_HAS_SUPPORT_FUNC(name) void name(struct fpl__InputContext *ctx, struct fpl__InputBackend *backend, fplInputBackendSupport *outSupport)

#define FPL_INPUT_BACKEND_ENUM_DEVICES_FUNC(name) uint32_t name(struct fpl__InputContext *ctx, struct fpl__InputBackend *backend, fplInputSourceType sourceFilter, fplInputDevice *outDevices, uint32_t maxDevices)

#define FPL_INPUT_BACKEND_FIND_DEVICE_FUNC(name)  bool name(struct fpl__InputContext *ctx, struct fpl__InputBackend *backend, const fplInputDeviceGuid *guid, fplInputDevice *outDevice)

#define FPL_INPUT_BACKEND_UPDATE_FUNC(name) void name(struct fpl__InputContext *ctx, struct fpl__InputBackend *backend) // hotplug scan, per-tick bookkeeping

#define FPL_INPUT_BACKEND_POLL_KEYBOARD_FUNC(name) bool name(struct fpl__InputContext *ctx, struct fpl__InputBackend *backend, uint32_t deviceIndex, fplKeyboardState *outState)
#define FPL_INPUT_BACKEND_POLL_MOUSE_FUNC(name)    bool name(struct fpl__InputContext *ctx, struct fpl__InputBackend *backend, uint32_t deviceIndex, fplMouseState *outState)
#define FPL_INPUT_BACKEND_POLL_GAMEPAD_FUNC(name)  bool name(struct fpl__InputContext *ctx, struct fpl__InputBackend *backend, uint32_t deviceIndex, fplGamepadState *outState)

// Window bridge: called by window backends with a neutral wrapped native event.
#define FPL_INPUT_BACKEND_HANDLE_NATIVE_EVENT_FUNC(name) bool name(struct fpl__InputContext *ctx, struct fpl__InputBackend *backend, const fpl__NativeInputEvent *ev)

typedef struct fpl__InputBackendFunctionTable {
    fpl_input_backend_load_func              *load;
    fpl_input_backend_unload_func            *unload;
    fpl_input_backend_init_func              *init;
    fpl_input_backend_release_func           *release;
    fpl_input_backend_has_support_func       *hasSupport;
    fpl_input_backend_enum_devices_func      *enumDevices;
    fpl_input_backend_find_device_func       *findDevice;
    fpl_input_backend_update_func            *update;
    fpl_input_backend_poll_keyboard_func     *pollKeyboard;
    fpl_input_backend_poll_mouse_func        *pollMouse;
    fpl_input_backend_poll_gamepad_func      *pollGamepad;
    fpl_input_backend_handle_native_event_func *handleNativeEvent;
} fpl__InputBackendFunctionTable;
```

### 6.2 Descriptor + Backend Instance

Same two-part shape that the audio system uses (`fplAudioBackendDescriptor` + `fplAudioBackend`), intentionally trimmed down.

```c
typedef struct fpl__InputBackendDescriptorHeader {
    fplInputBackendType type;
    const char         *name;
    uint32_t            backendSize;  // size of concrete impl struct
    fplInputSourceType  defaultSources;
    bool                isValid;
} fpl__InputBackendDescriptorHeader;

typedef struct fpl__InputBackendDescriptor {
    fpl__InputBackendDescriptorHeader header;
    fpl__InputBackendFunctionTable    table;
} fpl__InputBackendDescriptor;

typedef struct fpl__InputBackend {
    fplInputBackendType  type;
    fplInputBackendSupport support;  // filled during init
    bool                 isInitialized;
    bool                 isRuntimeLinked;
    void                *libraryHandle; // dlopen / LoadLibrary
    // Concrete backend data starts directly after this struct with
    // the same padding trick used by the audio system.
} fpl__InputBackend;

#define FPL_INPUT_BACKEND_DATA_PADDING 16
#define FPL_INPUT_BACKEND_DATA_OFFSET  (sizeof(fpl__InputBackend) + FPL_INPUT_BACKEND_DATA_PADDING)
#define FPL_GET_INPUT_BACKEND_IMPL(backend, type) \
    (type *)(((uint8_t *)(backend) + FPL_INPUT_BACKEND_DATA_OFFSET))
```

### 6.3 Input Context / System

```c
typedef struct fpl__InputContext {
    fplInputSettings         settings;
    fplInitFlags             initFlags;
    fpl__InputBackend       *backends[FPL_MAX_INPUT_BACKEND_COUNT];
    uint32_t                 backendCount;

    // Aggregated device cache, refreshed by update() + native events.
    fplInputDevice           devices[FPL_MAX_INPUT_DEVICE_COUNT];
    uint32_t                 deviceCount;

    // Event queue shared with windowing (re-uses existing fplEvent pipe)
    // No separate queue: see section 6.5.
} fpl__InputContext;
```

Stored on `fpl__PlatformAppState` next to the audio context and **outside** `fpl__PlatformWindowState`, so it survives `FPL_NO_WINDOW`.

### 6.4 Global Descriptor Registry

Same pattern as `fpl__global_audioBackendDirectShowDescriptor` et al. One static descriptor per backend, guarded by the corresponding `FPL_NO_INPUT_*` switch, collected into a small static array that `fpl__InputSystem_Init` walks.

```c
fpl_globalvar fpl__InputBackendDescriptor *fpl__global_inputBackendDescriptors[] = {
#if !defined(FPL_NO_INPUT_XINPUT) && defined(FPL_PLATFORM_WINDOWS)
    &fpl__global_inputBackendXInputDescriptor,
#endif
#if !defined(FPL_NO_INPUT_DINPUT) && defined(FPL_PLATFORM_WINDOWS)
    &fpl__global_inputBackendDInputDescriptor,
#endif
#if !defined(FPL_NO_INPUT_RAWINPUT) && defined(FPL_PLATFORM_WINDOWS)
    &fpl__global_inputBackendRawInputDescriptor,
#endif
#if !defined(FPL_NO_INPUT_X11) && defined(FPL_SUBPLATFORM_X11)
    &fpl__global_inputBackendX11KbmDescriptor,
#endif
#if !defined(FPL_NO_INPUT_LINUX_JOYSTICK) && defined(FPL_PLATFORM_LINUX)
    &fpl__global_inputBackendLinuxJoystickDescriptor,
#endif
#if !defined(FPL_NO_INPUT_LINUX_EVDEV) && defined(FPL_PLATFORM_LINUX)
    &fpl__global_inputBackendLinuxEvdevDescriptor,
#endif
#if !defined(FPL_NO_INPUT_WIN32) && defined(FPL_PLATFORM_WINDOWS)
    &fpl__global_inputBackendWin32Descriptor,
#endif
    fpl_null,
};
```

### 6.5 Window ↔ Input Event Bridge

A tiny neutral struct carries native events from the window backend to the input system. No new event queue — events that should become public are pushed onto the existing `fplEvent` ring via the existing `fpl__PushWindowEvent` / `fpl__PushGamepadEvent` helpers.

```c
typedef enum fpl__NativeInputEventKind {
    fpl__NativeInputEventKind_Win32Msg,    // payload = MSG / WPARAM / LPARAM
    fpl__NativeInputEventKind_X11Event,    // payload = XEvent*
    fpl__NativeInputEventKind_Custom,
} fpl__NativeInputEventKind;

typedef struct fpl__NativeInputEvent {
    fpl__NativeInputEventKind kind;
    void *payload;
} fpl__NativeInputEvent;
```

Windowing code keeps only its own translations (focus, resize, DnD). Everything input-shaped is forwarded as follows:

```c
// Inside fpl__X11HandleEvent:
case KeyPress: case KeyRelease:
case ButtonPress: case ButtonRelease:
case MotionNotify: case EnterNotify: case LeaveNotify:
case FocusIn: case FocusOut:
    fpl__InputSystem_HandleNativeEvent(&fpl__NativeInputEvent{ X11Event, ev });
    break;
```

```c
// Inside the Win32 WndProc:
case WM_KEYDOWN: case WM_KEYUP: case WM_SYSKEYDOWN: case WM_SYSKEYUP:
case WM_CHAR:    case WM_DEADCHAR:
case WM_LBUTTONDOWN: case WM_LBUTTONUP: case WM_RBUTTONDOWN: case WM_RBUTTONUP:
case WM_MBUTTONDOWN: case WM_MBUTTONUP: case WM_XBUTTONDOWN: case WM_XBUTTONUP:
case WM_MOUSEMOVE:   case WM_MOUSEWHEEL: case WM_MOUSEHWHEEL:
case WM_INPUT:       // raw input
    fpl__InputSystem_HandleNativeEvent(&fpl__NativeInputEvent{ Win32Msg, &msg });
    break;
```

`fpl__InputSystem_HandleNativeEvent` dispatches to every registered backend whose `supportedSources` intersects the event type. Backends that don't care return false — cheap.

### 6.6 Polling Paths

Public polling functions delegate to the input system:

```c
fpl_platform_api bool fplPollKeyboardState(fplKeyboardState *outState) {
    FPL__CheckPlatform(false);
    FPL__CheckArgumentNull(outState, false);
    return fpl__InputSystem_PollKeyboard(&fpl__global__AppState->input, outState);
}
```

`fpl__InputSystem_PollKeyboard` walks all enabled backends that declare `supportedSources & Keyboard`, OR-merges their state into the out struct, and returns true if any one succeeded. This gives the "XInput + DInput at the same time" behavior the user asked for without any special-casing at call sites.

### 6.7 Hotplug

Three strategies, chosen per backend:

1. **Event-driven (preferred)** — `handleNativeEvent` catches device-added/removed messages:
   - Win32: `WM_DEVICECHANGE` forwarded by the window (or a hidden message-only window when `FPL_NO_WINDOW` is set — see section 8).
   - Linux: udev netlink socket polled with the existing epoll/select loop.
2. **Periodic scan** — `update()` is called once per `fplPollEvents` or once per user-driven `fplUpdateInputDevices()`, throttled by `gameControllers.detectionFrequency`.
3. **Manual** — user calls `fplUpdateInputDevices()` themselves. Useful in console apps.

Device add/remove still produces public `fplGamepadEvent` entries on the existing event queue, so the user-visible API does not change.

### 6.8 Simplifications Compared to the Audio System

| Audio backend system                        | Input backend system (this proposal)     |
| -------------------------------------------- | ---------------------------------------- |
| Async main loop per backend                  | No main loop — pure poll + event router  |
| Device state machine (Start/Stop/Starting…)  | Just `isInitialized`                     |
| Per-backend thread                           | Runs on the calling thread               |
| `isAsync` flag, client read callback         | None                                     |
| Channel maps, formats                        | None                                     |
| Single active backend                        | **Many** active backends at once         |

Keep the descriptor/header/table shape, drop everything that was audio-stream-specific.

---

## 7. Migration of Existing Code

### 7.1 Win32

- **XInput** code in `fpl__Win32XInputState`, `fpl__Win32XInput_*` → move into a new translation unit block guarded by `FPL__ENABLE_INPUT_XINPUT`. Wrap as `fpl__InputBackendXInput` implementing the full function table. `fpl__Win32UpdateGameControllers` / `fpl__Win32XInput_CreateEventsForStates` become the backend's `update()`. Gamepad events keep flowing through `fpl__PushGamepadEvent`.
- **Win32 WM_ keyboard/mouse handling** currently inside the WndProc becomes `fpl__InputBackendWin32`. WndProc only calls `fpl__InputSystem_HandleNativeEvent`.
- **`fpl__Win32IsKeyDown` / `MapVirtualKeyW` polling** in `fplPollKeyboardState` moves into `fpl__InputBackendWin32::pollKeyboard`. `fplPollMouseState` moves into the same backend's `pollMouse`.
- **RAWINPUT** bits (`RegisterRawInputDevices`) become `fpl__InputBackendRawInput` — opt-in.
- **DirectInput** (future) becomes `fpl__InputBackendDInput`. Can coexist with XInput because the input system allows multiple active backends.

### 7.2 X11

- `fpl__X11HandleEvent` stops dispatching keyboard/mouse logic; it forwards native events through the bridge.
- `fplPollKeyboardState` / `fplPollMouseState` current X11 implementations move into `fpl__InputBackendX11Kbm::pollKeyboard` / `pollMouse`.
- Window-state field access (`appState->window.x11.display`, `.window`) stays — the backend receives the `fpl__X11SubplatformState*` at init time through the context.

### 7.3 Linux Gamepad

- Existing `fpl__LinuxGameControllersState` / `fpl__LinuxPollGameControllers` becomes `fpl__InputBackendLinuxJoystick` — same `/dev/input/jsX` path, still useful as a minimal baseline.
- A new `fpl__InputBackendLinuxEvdev` is added: opens `/dev/input/eventX` nodes, supports full button/axis sets, proper multi-gamepad, and reads via epoll instead of 1-second scans.
- A companion `fpl__InputBackendLinuxUdev` provides hotplug by listening on a udev netlink socket; it owns no devices of its own but pushes device-add/remove events into the context so the Evdev backend can rebuild its device list without polling.
- The input system enables `LinuxEvdev + LinuxUdev` by default; `LinuxJoystick` is opt-in via `fplInputBackendMask`.

### 7.4 Unix

- A new `fpl__InputBackendUnixGamepad` handles uhid / `/dev/uhidX` on BSD-family systems. Same function table, isolated from the Linux backends.

### 7.5 Call Site Changes

- All `fplIsMaskSet(appState->initFlags, fplInitFlags_GameController)` checks inside window code become `fpl__InputSystem_IsEnabled(ctx, fplInputSourceType_Gamepad)`.
- `fpl__Win32UpdateGameControllers(...)` call in the Win32 event pump → `fpl__InputSystem_Update(&appState->input)`.
- `fpl__LinuxPollGameControllers(...)` call near `fpl__X11HandleEvent` → same `fpl__InputSystem_Update`.
- `fpl__LinuxFreeGameControllers` → removed, lives inside the Linux backend's `release()`.
- `FPL__ENABLE_WINDOW` guards around gamepad code are replaced by `FPL__ENABLE_INPUT`.

---

## 8. `FPL_NO_WINDOW` Interaction

To make input work without a window on Win32 we need a surface to receive `WM_INPUT` and `WM_DEVICECHANGE`. Options:

1. **Hidden message-only window** (`HWND_MESSAGE`) owned by the input system. Cheap, no visible surface, works without `fplInitFlags_Window`. This is the recommended approach.
2. **Polling-only mode** — set `fplInputSettings.pollFromWindowEventLoop = false` and rely on XInput polling + a timer thread for DirectInput. Simpler but no hotplug.

The input system picks the first available: if there is a user window, use it; else create the message-only window; else fall back to polling. On X11, the input system can run without a visible window by opening its own `Display*` for polling keyboard/mouse state when `detachFromWindow` is set. On Linux gamepads this is a non-issue — evdev lives entirely outside X.

---

## 9. Lifetime and Init Order

New sequence inside `fpl__PlatformInit`:

```
1. Platform API load (existing)
2. Window init        (if fplInitFlags_Window)
3. Video init         (if fplInitFlags_Video)
4. Audio init         (if fplInitFlags_Audio)           <-- existing order
5. Input init         (if fplInitFlags_Input or GameController)  <-- new, moved out of window init
      for each descriptor in fpl__global_inputBackendDescriptors:
          if mask disables it          -> skip
          if platform mismatches       -> skip
          allocate fpl__InputBackend + impl (platform allocator)
          call load() (runtime link)
          call init(settings)
          call hasSupport() -> cache into backend->support
          if everything OK             -> push to ctx->backends[]
          else                         -> unload()+free
```

Release in reverse order. `fpl__InputSystem_Release` calls `release()` + `unload()` on every active backend, frees its memory, clears the context.

---

## 10. Default Mappings

- **Windows**: XInput + Win32 (keyboard/mouse via `GetKeyState`/WM_*). DInput and RawInput are off by default but enabled with a single mask bit.
- **Linux/X11**: X11Kbm + LinuxEvdev + LinuxUdev. Joystick node backend off by default; can be re-enabled for retro-compat.
- **Unix**: X11Kbm + UnixGamepad.

`fplInputBackendType_Auto` in settings means "use the defaults above". Users who want multiple gamepad backends just set:

```c
fplSettings s = fplMakeDefaultSettings();
fplInputBackendMaskEnable(&s.input.enabledBackends, fplInputBackendType_DInput);
// XInput is already on by default; DInput is now also on
fplPlatformInit(fplInitFlags_All, &s);
```

---

## 11. Step-By-Step Implementation Plan

Each step is independently compilable and testable.

1. **Types**: add `fplInputSourceType`, `fplInputBackendType`, `fplInputBackendMask`, `fplInputDevice`, `fplInputBackendSupport`, extended `fplInputSettings`, new `fplInitFlags_Input`, `FPL_NO_INPUT` switch, extended `fplMouseButtonType` + wheel fields. No behavior change yet.
2. **Skeleton**: add `fpl__InputContext` to `fpl__PlatformAppState`. Add `fpl__InputSystem_Init` / `Release` / `Update` / `HandleNativeEvent` / `PollKeyboard` / `PollMouse` / `PollGamepad` as empty stubs. Wire init/release into `fpl__PlatformInit` / `fpl__PlatformRelease` behind `FPL_NO_INPUT`.
3. **XInput migration**: wrap existing XInput code as `fpl__InputBackendXInput`. Switch `fplPollGamepadStates` to go through the context. Delete `fpl__Win32XInputState` from `fpl__Win32AppState`. Verify `FPL_Input` demo still shows gamepad input.
4. **Win32 keyboard/mouse migration**: move the current `fplPollKeyboardState` / `fplPollMouseState` Win32 implementations into `fpl__InputBackendWin32`. Route WndProc keyboard/mouse messages through `HandleNativeEvent`. Verify all Win32 demos.
5. **X11Kbm migration**: same for X11. Route `XEvent` from `fpl__X11HandleEvent` through the bridge. Verify X11 demos.
6. **Linux Joystick migration**: lift existing `/dev/input/jsX` code into `fpl__InputBackendLinuxJoystick`. Remove the 1-second scan; let `fpl__InputSystem_Update` throttle using `gameControllers.detectionFrequency`.
7. **Event plumbing**: gamepad connect/disconnect events keep flowing through `fpl__PushGamepadEvent`, now driven by the backends instead of the window loop.
8. **FPL_NO_WINDOW support**: implement the hidden Win32 message-only window and the detached X11 display. Add `fplInputSettings.detachFromWindow`. Verify with a console-only demo that polls a keyboard or gamepad.
9. **Multi-backend polling**: make `fpl__InputSystem_PollGamepad` merge across backends so XInput + (future) DInput coexist. Add a small unit test in `FPL_Test` that walks a fake second backend.
10. **New backends (future PRs)**: LinuxEvdev, LinuxUdev, RawInput, DInput, UnixGamepad. Each is one descriptor + impl struct + function table entry; nothing else in the core changes.
11. **Docs / changelog**: update category `page_category_input_config`, note the new init flag, mask helpers and `FPL_NO_INPUT` switch.

Each step is ~1 commit. Steps 3-6 should keep the public behavior identical; only step 8 and step 9 introduce new capabilities.

---

## 12. Open Questions

- Should `fplInputDevice.state` be kept as a union in the public API, or should the public API return plain pointers/arrays? Union matches the spec but duplicates state structs.
- Does the project want a `fplInputBackendType_Custom` slot (to match `fplAudioBackendType_Custom`) for user-supplied backends? Simple to add, but adds one more public function.
- Do we expose `fplUpdateInputDevices()` as a public call, or fold it into `fplPollEvents`? Folding is simpler; exposing helps `FPL_NO_WINDOW` console loops.
---

## 13. Summary

- New public types: `fplInputSourceType`, `fplInputBackendType`, `fplInputBackendMask`, `fplInputDevice`, `fplInputBackendSupport`, extended `fplInputSettings`, new `fplInitFlags_Input`, new `FPL_NO_INPUT`, extended mouse state (5 buttons + wheel).
- New internal layer: `fpl__InputContext` + `fpl__InputBackend` + `fpl__InputBackendDescriptor` + small function table, modeled on the audio backend system but with no async main loop and no device state machine.
- Windowing code is reduced to forwarding native events; all input logic moves into dedicated backends.
- Multiple backends run simultaneously — the input system merges results.
- `FPL_NO_WINDOW` works with input; `FPL_NO_INPUT` kills input entirely.
- Existing XInput, Win32 keyboard/mouse, X11 keyboard/mouse, Linux joystick code is migrated as-is to the new shape; new backends (DInput, RawInput, Evdev, Udev, Unix) slot in without touching the core.
