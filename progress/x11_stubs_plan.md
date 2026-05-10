# X11 Stub Implementation Plan

Scope: implement the X11 platform stubs in `final_platform_layer.h` between lines 23362 and 23543 (and `fplSetWindowCursorEnabled` at 23283, `fplIsWindowResizable`/`fplSetWindowResizeable` at 23311–23318).

References:
- X11 reference functions: `fplSetWindowDecorated` (23340), `fplGetWindowPosition` (23459).
- Win32 reference functions: `fplQueryCursorPosition` (19645), display query block (19674–19846).

## Rules (recap)

- Optional libs (XRandR, Xinerama) get their own `fpl__XrandRApi` / `fpl__XineramaApi`. Load failure must not error platform init.
- Custom types prefixed `fpl__`, defines `FPL__`.
- No headers for extensions — define types/prototypes locally.
- No heap allocations. Use stack or fields in `fpl__X11WindowState`. For temp buffers > stack, use `fpl__AllocateTemporaryMemory` + `fpl__ReleaseTemporaryMemory`.
- X extensions go in their own struct, NOT in `fpl__X11Api` (per rule example XRANDR).
- Plain `libX11` calls newly needed (XIconifyWindow etc.) go in `fpl__X11Api`.
- Block-style braces around all conditional bodies.
- Single-line comments only. No long doc comments.

## New optional API structs

### `fpl__XrandRApi`
- Library files: `libXrandr.so.2`, `libXrandr.so`.
- Lives in `fpl__X11SubplatformState` as `xrandr` field.
- Custom types (ABI-compatible with libXrandr):
  - `fpl__RROutput`, `fpl__RRCrtc`, `fpl__RRMode` (`unsigned long`)
  - `fpl__XRRModeInfo` (id, width, height, dotClock, hTotal, vTotal, name, nameLength, …)
  - `fpl__XRRScreenResources` (timestamps, ncrtc/crtcs, noutput/outputs, nmode/modes)
  - `fpl__XRRCrtcInfo` (timestamp, x, y, width, height, mode, rotation, …)
  - `fpl__XRROutputInfo` (timestamp, crtc, name, nameLen, mm_width, mm_height, connection, subpixel_order, ncrtc/crtcs, nclone/clones, nmode/modes, npreferred)
- Function pointers:
  - `XRRQueryExtension`
  - `XRRGetScreenResourcesCurrent` / `XRRFreeScreenResources`
  - `XRRGetCrtcInfo` / `XRRFreeCrtcInfo`
  - `XRRGetOutputInfo` / `XRRFreeOutputInfo`
  - `XRRGetOutputPrimary`

### `fpl__XineramaApi`
- Library files: `libXinerama.so.1`, `libXinerama.so`.
- Lives in `fpl__X11SubplatformState` as `xinerama` field.
- Custom types:
  - `fpl__XineramaScreenInfo { int screen_number; short x_org, y_org; short width, height; }`
- Function pointers:
  - `XineramaQueryExtension`
  - `XineramaIsActive`
  - `XineramaQueryScreens` (result freed with `XFree`)

## Additions to `fpl__X11Api`

- `XIconifyWindow` — minimize
- `XAllocSizeHints`, `XSetWMNormalHints`, `XGetWMNormalHints` — resizable
- `XDefineCursor`, `XUndefineCursor`, `XFreeCursor`, `XCreateBitmapFromData`, `XCreatePixmapCursor` — cursor hide
- `XSetSelectionOwner`, `XGetSelectionOwner`, `XCheckTypedWindowEvent` — clipboard
- `XTranslateCoordinates` — window→root coords for window-display lookup

## Additions to `fpl__X11WindowState`

- `Cursor invisibleCursor` — lazy-built 1x1 cursor for hide
- `bool cursorEnabled` — current state
- `Atom netWMStateAbove` — floating
- `Atom clipboardAtom`, `targetsAtom`, `incrAtom`, `selectionPropAtom` — clipboard
- `char clipboardOut[FPL_MAX_BUFFER_LENGTH]` — text we own as selection owner
- `size_t clipboardOutLen`

## Init/Release wiring

In `fpl__X11InitSubplatform`:
1. `fpl__LoadX11Api` (required — keep current behavior).
2. `fpl__LoadXrandRApi` (optional — log warn on miss).
3. `fpl__LoadXineramaApi` (optional — log warn on miss).

Mirror unloads in `fpl__X11ReleaseSubplatform`.

## Three-tier display strategy

1. **XRandR** — full info. Modes, refresh, names, primary.
2. **Xinerama** — per-monitor rects only. Synthetic id. No real modes.
3. **Root** — single virtual display, root window dims. Last resort.

Tier picked at call time based on which APIs loaded + extensions active.

| Function | RandR | Xinerama | Root |
|---|---|---|---|
| `fplGetDisplayCount` | connected outputs with crtc | screen count | 1 |
| `fplGetDisplays` | full info | rect + synthetic id | root rect, id="default" |
| `fplGetPrimaryDisplay` | `XRRGetOutputPrimary` | screen containing (0,0) | root |
| `fplGetWindowDisplay` | crtc containing window center | screen containing window center | root |
| `fplGetDisplayFromPosition` | crtc containing pt | screen containing pt | root |
| `fplGetDisplayModes` | full mode list | 1 entry, current | 1 entry, current |

Xinerama id format: `"XINERAMA-%d"` via `fplStringFormat`. Refresh = 0, colorBits = root depth.

## Per-function plan

### Window styles (23362–23379, 23311–23325)

- **fplIsWindowFloating** — `XGetWindowProperty` on `_NET_WM_STATE`, scan Atom array for `netWMStateAbove`. Free via `XFree`.
- **fplSetWindowFloating** — `XSendEvent` ClientMessage to root: msg_type=`netWMState`, data[0]=add/remove, data[1]=`netWMStateAbove`. Mirrors fullscreen pattern.
- **fplGetWindowState** — read `_NET_WM_STATE`:
  - Hidden → `fplWindowState_Iconify`
  - Vert+Horz → `fplWindowState_Maximize`
  - Fullscreen → `fplWindowState_Fullscreen`
  - else if `XGetWindowAttributes.map_state == IsViewable` → `fplWindowState_Normal`
  - else → `fplWindowState_Hidden`
- **fplSetWindowState** — switch:
  - Iconify → `XIconifyWindow`
  - Maximize → ClientMessage add MaxVert+MaxHorz
  - Normal → ClientMessage remove all + `XMapWindow`
  - Fullscreen → reuse `fplSetWindowFullscreenSize(true,...)`
  - Hidden → `XUnmapWindow`
- **fplIsWindowResizable** — return `appState->currentSettings.window.isResizable` (matches Win32 cache).
- **fplSetWindowResizeable** — `XAllocSizeHints`, lock min=max=current size when false, clear PMinSize/PMaxSize when true. `XSetWMNormalHints`. `XFree` hints. Update cached setting.
- **fplIsWindowDecorated** — already implemented (23320). No-op.

### Cursor (23283, 23540)

- **fplSetWindowCursorEnabled** — first-call lazy: `XCreateBitmapFromData` 1x1 zero pixmap + `XCreatePixmapCursor` (transparent). Cache in `windowState->invisibleCursor`. `XDefineCursor` to hide / `XUndefineCursor` to show. Update `cursorEnabled`.
- **fplQueryCursorPosition** — `XQueryPointer(display, window, …)`, return `win_x_return` / `win_y_return` (matches Win32 default branch which returns global p.x/p.y — but Win32 commented alternate path subtracts monitor origin; we go window-relative for parity with Win32 behavior).

### Display query (23381–23409)

Implement three-tier path described above. Helper internals:

- `fpl__X11FillDisplayInfoFromCrtc(outputInfo, crtcInfo, primaryOutputId, outInfo)`
- `fpl__X11FillDisplayInfoFromXinerama(screen, outInfo)`
- `fpl__X11FillDisplayInfoFromRoot(display, screen, outInfo)`
- `fpl__X11ResolveDisplayBackend(subplatform)` returns enum `RandR | Xinerama | Root`.

Free returned RandR / Xinerama structs on every path.

### Clipboard (23507–23515)

X11 selection model. Use `CLIPBOARD` atom. `PRIMARY` not used.

- **fplSetClipboardText** — copy to `clipboardOut`. `XSetSelectionOwner(display, clipboardAtom, window, CurrentTime)`. Verify owner. Real reply happens in event loop on `SelectionRequest`:
  - target=`TARGETS` → reply property = [TARGETS, UTF8_STRING, STRING].
  - target=`UTF8_STRING` or `STRING` → write `clipboardOut` to requestor property, send `SelectionNotify`.
  - other → reply `None`.
  - Extend `fpl__X11HandleEvent`.
- **fplGetClipboardText** — if we own selection → copy from `clipboardOut`. Else `XConvertSelection(display, clipboardAtom, UTF8_STRING, selectionPropAtom, window, CurrentTime)`, `XFlush`, then poll `XCheckTypedWindowEvent` for `SelectionNotify` with timeout (~500ms via `fplGetTimeInMilliseconds` loop with short sleep). On notify: `XGetWindowProperty` UTF8_STRING. Copy. `XFree`. `XDeleteProperty`.
- For long property reads, use `fpl__AllocateTemporaryMemory` / `fpl__ReleaseTemporaryMemory`. INCR not implemented — single-shot.

### Optional fplSetWindowFullscreenRect (23437)

X11 has no pixel-rect fullscreen. Plan: send `_NET_WM_FULLSCREEN_MONITORS` ClientMessage with monitor indices computed from rect via Xinerama/RandR. Toggle fullscreen ON. Comment limitation: spans monitors, not pixel rect.

## Sub-session split

Multi-session implementation. See `x11_stubs_todo.md` for ordered checkboxes.
