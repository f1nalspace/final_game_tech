# X11 Stub Implementation TODO

Tracking file. Tick boxes as work progresses across sessions. See `x11_stubs_plan.md` for design.

## Session 1 — Infrastructure (no behavior changes yet)

- [ ] Add new function-pointer typedefs + slots in `fpl__X11Api`: `XIconifyWindow`, `XAllocSizeHints`, `XSetWMNormalHints`, `XGetWMNormalHints`, `XDefineCursor`, `XUndefineCursor`, `XFreeCursor`, `XCreateBitmapFromData`, `XCreatePixmapCursor`, `XSetSelectionOwner`, `XGetSelectionOwner`, `XCheckTypedWindowEvent`, `XTranslateCoordinates`.
- [ ] Update `fpl__LoadX11Api` / `fpl__UnloadX11Api` for new entries.
- [ ] Define `fpl__XrandRApi` struct, types (`fpl__RROutput`, `fpl__RRCrtc`, `fpl__RRMode`, `fpl__XRRModeInfo`, `fpl__XRRScreenResources`, `fpl__XRRCrtcInfo`, `fpl__XRROutputInfo`).
- [ ] Implement `fpl__LoadXrandRApi` / `fpl__UnloadXrandRApi` (libXrandr.so.2, libXrandr.so). Optional. No init failure.
- [ ] Define `fpl__XineramaApi` struct + `fpl__XineramaScreenInfo`.
- [ ] Implement `fpl__LoadXineramaApi` / `fpl__UnloadXineramaApi` (libXinerama.so.1, libXinerama.so). Optional. No init failure.
- [ ] Add `xrandr` and `xinerama` fields to `fpl__X11SubplatformState`.
- [ ] Wire load/unload into `fpl__X11InitSubplatform` / `fpl__X11ReleaseSubplatform`.
- [ ] Extend `fpl__X11WindowState` with: `invisibleCursor`, `cursorEnabled`, `netWMStateAbove`, `clipboardAtom`, `targetsAtom`, `incrAtom`, `selectionPropAtom`, `clipboardOut[]`, `clipboardOutLen`.
- [ ] Intern new atoms in `fpl__X11InitWindow`.
- [ ] Build verification: compile a demo, ensure init still works without Xrandr/Xinerama.

## Session 2 — Window styles

- [ ] `fplIsWindowFloating`
- [ ] `fplSetWindowFloating`
- [ ] `fplGetWindowState`
- [ ] `fplSetWindowState`
- [ ] `fplIsWindowResizable`
- [ ] `fplSetWindowResizeable`
- [ ] Manual test: window state cycling, resizable toggle, floating toggle.

## Session 3 — Cursor

- [ ] `fplSetWindowCursorEnabled` (lazy invisible cursor + define/undefine)
- [ ] `fplQueryCursorPosition` (`XQueryPointer`, window-relative)
- [ ] Free cursor in `fpl__X11ReleaseWindow` if created.
- [ ] Manual test: hide/show cursor, query position inside window.

## Session 4 — Display query (RandR + Xinerama + root fallback)

- [ ] Helper `fpl__X11ResolveDisplayBackend`.
- [ ] Helper `fpl__X11FillDisplayInfoFromCrtc`.
- [ ] Helper `fpl__X11FillDisplayInfoFromXinerama`.
- [ ] Helper `fpl__X11FillDisplayInfoFromRoot`.
- [ ] `fplGetDisplayCount`
- [ ] `fplGetDisplays`
- [ ] `fplGetPrimaryDisplay`
- [ ] `fplGetWindowDisplay`
- [ ] `fplGetDisplayFromPosition`
- [ ] `fplGetDisplayModes` (RandR mode list, Xinerama/root single current)
- [ ] Manual test: single-monitor + multi-monitor (if available).

## Session 5 — Clipboard

- [ ] `fplGetClipboardText` (XConvertSelection + SelectionNotify polling).
- [ ] `fplSetClipboardText` (XSetSelectionOwner + outgoing buffer).
- [ ] Extend `fpl__X11HandleEvent` for `SelectionRequest` (TARGETS, UTF8_STRING, STRING).
- [ ] Manual test: copy from another app + paste; copy from FPL window + paste in another app.

## Session 6 — Optional + cleanup

- [ ] `fplSetWindowFullscreenRect` via `_NET_WM_FULLSCREEN_MONITORS`.
- [ ] Sweep for leftover `@IMPLEMENT(final/X11)` markers in target range.
- [ ] Final build + run a window/display demo end-to-end.
- [ ] Update changelog block at top of `final_platform_layer.h`.

## Notes

- INCR clipboard transfer not in scope — single-shot only. Document as limitation.
- `fplSetWindowFullscreenRect` cannot be pixel-perfect on X11. Document as limitation.
- Resume next session by reading this file + `x11_stubs_plan.md`.
