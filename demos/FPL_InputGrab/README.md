# FPL_InputGrab

Test bench for the keyboard grab, the mouse grab, the cursor warp and the relative mouse mode of FPL, together with the scan codes, the side mouse buttons, both mouse wheels and the mouse enter/leave events.
It draws a crosshair at the mouse position and writes every event as one machine readable line, which the test script in `tests/` evaluates on X11 and a person compares by hand on Windows.

The API it tests is described in `final_platform_layer.docs`, page "Window style & layout", sections "Moving the cursor", "Grabbing the mouse", "Relative mouse mode", "Grabbing the keyboard" and "Capturing keyboard and mouse for a virtual machine".

## Building

CMake is the main build system, the executables go to `demos/build/FPL_InputGrab/<System>-<Arch>-<BuildType>/`.

```sh
cmake -S demos/FPL_InputGrab -B demos/FPL_InputGrab/build -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja -C demos/FPL_InputGrab/build
```

On Linux this also builds `FPL_InputGrab_NoXInput2` with `FPL_NO_X11_XINPUT2`, the relative mouse mode then uses the warp fallback instead of XInput2.

Windows without Visual Studio, from the `demos` folder:

```sh
x86_64-w64-mingw32-gcc -std=c99 -I.. FPL_InputGrab/fpl_inputgrab.c -o FPL_InputGrab_x64.exe
i686-w64-mingw32-gcc -std=c99 -I.. FPL_InputGrab/fpl_inputgrab.c -o FPL_InputGrab_x86.exe
```

`FPL_InputGrab.vcxproj` is the Visual Studio project. It links as a Windows application without a console, so its log only shows up when the standard output is redirected (see "Windows hand checklist"). The CMake and MinGW builds are console applications and write the log into the console.

## What the window shows

- A crosshair at the mouse position. In the relative mouse mode it follows the sum of the deltas instead and stops at the edges of the window.
- A green frame while the window has the focus, a gray frame without it. The crosshair is gray without focus as well.
- The title: `<prefix> - Focus: yes|no - Mouse grab: on|off, Relative: on|off, Keyboard grab: on|off - Last key: <key> <state>`. The grab states are the requested ones, a grab is only active while the window has the focus.

## Hotkeys

The hotkeys work like the ones of qemu and are never logged as a normal key.

| Hotkey | Action |
|---|---|
| Ctrl+Alt+G | Toggle the full grab: keyboard grab and relative mouse mode together |
| Ctrl+Alt+M | Toggle the mouse grab |
| Ctrl+Alt+R | Toggle the relative mouse mode |
| Ctrl+Alt+K | Toggle the keyboard grab |
| Ctrl+Alt+W | Warp the cursor to the window center |
| Ctrl+Alt+Q | Quit |

## Parameters

| Parameter | Meaning |
|---|---|
| `--log-events` | Write every event as one line to the standard output |
| `--log-fpl` | Also write the log messages of FPL down to the verbose level, like a refused grab that is tried again. Without it only the FPL warnings and errors are written. |
| `--log-core-motion` | X11 only: also write the cursor motion of the X server (`mouse core x= y=`), which is accelerated, next to the raw deltas of the relative mode |
| `--mouse-grab` | Request the mouse grab at the start |
| `--relative-mouse` | Request the relative mouse mode at the start |
| `--keyboard-grab` | Request the keyboard grab at the start |
| `--selftest` | Warp the cursor to the four corners and the center and check position, delta and screen origin of every move event, the exit code is 1 when a check fails. With `--mouse-grab` it also warps to two positions outside of the window, which must end at the edges of the client area. |
| `--timeout=<seconds>` | Quit by itself after this time, the safety net for everything that grabs the input |
| `--stall=<ms>` | Sleep this long in every frame, simulates a main loop that pumps the events rarely |
| `--window=<w>x<h>` | Inner size of the window, default 640x400 |
| `--title=<text>` | Window title prefix, tells several instances apart |
| `--help` | Print the parameters |

## Log format

One event per line, `t=<milliseconds since start> <category> <name> [key=value ...]`:

| Line | Meaning |
|---|---|
| `demo ready w= h=`, `demo timeout`, `demo quit` | Start, `--timeout` reached, end |
| `window gotfocus`, `window lostfocus`, `window resized w= h=`, `window moved x= y=`, `window minimized`, `window maximized`, `window restored`, `window shown`, `window hidden`, `window closed` | Window events |
| `key button state=press\|repeat\|release code= scan=0x key= mods=0x` | Key event: `code` is the platform key code (Win32 virtual key, X11 key code), `scan` the PC set 1 scan code, `key` the mapped `fplKey`, `mods` the modifier flags below |
| `key input code=` | Text input, the Unicode code point |
| `mouse move x= y= dx= dy=` | Position in window coordinates and the delta, in the relative mode the raw device movement |
| `mouse button name=left\|right\|middle\|x1\|x2 state= x= y=` | Mouse button |
| `mouse wheel dx= dy= x= y=` | Vertical wheel in `dy`, horizontal wheel in `dx` (each event has only one of them) |
| `mouse enter x= y=`, `mouse leave x= y=` | The cursor entered or left the client area |
| `mouse core x= y=` | X11 core motion, only with `--log-core-motion` |
| `grab mouse\|relative\|keyboard requested=on\|off result=1\|0` | A switch was set by a parameter or a hotkey, `result` is the return value of the setter |
| `warp x= y= result=` | Ctrl+Alt+W |
| `hotkey quit` | Ctrl+Alt+Q |
| `selftest ...` | Self test steps, ends with `selftest pass` or `selftest fail` (written even without `--log-events`) |
| `fpl level=<level> <message> (<function>:<line>)` | FPL log message (written even without `--log-events`) |

Modifier flags in `mods`: `0x1` left Alt, `0x2` right Alt, `0x4` left Ctrl, `0x8` right Ctrl, `0x10` left Shift, `0x20` right Shift, `0x40` left Super/Win, `0x80` right Super/Win, `0x100` Caps Lock, `0x200` Num Lock, `0x400` Scroll Lock. On X11 both sides are set when one is pressed, the X server does not tell them apart.

Example:

```
t=1520 key button state=press code=38 scan=0x1e key=A mods=0x0
t=1733 mouse move x=100 y=50 dx=3 dy=-1
```

## Automatic tests (X11)

```sh
demos/FPL_InputGrab/tests/run_grab_tests.sh [--tests=a,b] [--list] [--demo=<path>] [--fallback-demo=<path>]
```

33 tests (`--list`), around five minutes. Logs and `report.md` go to `demos/build/FPL_InputGrab/tests/`.
It needs `xdotool`, `python-xlib`, `setxkbmap` and `xkbcomp`.

**The tests take over the real desktop for the whole run.** They open small windows that get the focus, move the real pointer, send keys to the focused window (including Alt+Tab, Alt+F4 and Super), hold the pointer or the keyboard for a few seconds, and the scan code test switches the X keyboard layout to `us` and `de` (the saved keymap is restored afterwards, also when the script is interrupted). Do not type or move the mouse during a run: a moved mouse makes the first mouse tests fail, run them again alone then.
Every demo runs under `timeout -s KILL`, an X client that is killed loses all its grabs.

## Emergency exit

A grab ends when the demo ends. Every run that grabs should use `--timeout`.

- X11: switch to a text console (Ctrl+Alt+F2), `pkill FPL_InputGrab`, and back.
- Windows: Ctrl+Alt+Del always works, then end the demo in the task manager.

## Windows hand checklist

Wine runs the Win32 code, but it is not Windows (clip cursor, low level hook and raw input behave differently), so every item below is checked on a real Windows 10 or 11.

### Preparation

- Build with CMake or MinGW for a console application, or redirect the log of the Visual Studio build in `cmd.exe`: `FPL_InputGrab.exe --log-events > log.txt`, and follow it in PowerShell with `Get-Content log.txt -Wait`.
- Start every run with `--log-events --timeout=120` (longer if needed), add `--log-fpl` for the grab items.
- A mouse with side buttons, a tilt wheel (or a touchpad with horizontal scrolling) and a mouse with a high resolution (1600 DPI or more) for the raw delta item.
- A German keyboard layout (or another one with dead keys) for the dead key item, the other items work with any layout.

### 1. Warp, deltas and focus loss

- [ ] `--selftest --log-events --timeout=15` ends with `selftest pass` and exit code 0 (`echo %ERRORLEVEL%` after a console build, the Visual Studio build needs `start /wait` for it).
- [ ] `--selftest --mouse-grab --log-events --timeout=15` ends with `selftest pass`, the two targets outside of the window arrive at (0, 0) and (width-1, height-1).
- [ ] Moving the mouse slowly from one position to another: the sum of `dx`/`dy` equals the difference of `x`/`y`.
- [ ] Ctrl+Alt+W puts the cursor at the window center, the next `mouse move` has `dx=0 dy=0`.
- [ ] Hold A, Alt+Tab to another window, release A there, come back: the log shows `key button state=release ... key=A` before `window lostfocus`. The next Alt press after coming back is `state=press`, never `repeat`.

### 2. Mouse grab

Start with `--mouse-grab --log-events --log-fpl`.

- [ ] The pointer can not leave the client area, not at any edge, also not over the title bar.
- [ ] The cage follows when the window is moved (Win+Arrow keys), resized and in fullscreen or maximized.
- [ ] Ctrl+Alt+M releases and grabs again, visibly.
- [ ] Alt+Tab to another window: the pointer is free. Back to the demo: it is caged again.
- [ ] A click on the title bar of the inactive demo window can be dragged to move it, the pointer is only caged after the button is released.
- [ ] An activating click on the minimize or close button of the inactive window works.
- [ ] Alt+Space opens the system menu and the pointer can reach it.
- [ ] After Ctrl+Alt+Del and Esc (which resets the cursor clip), the pointer is caged again after 3 seconds at the latest.
- [ ] Quit through Ctrl+Alt+Q, through the close button and through `--timeout=10`: the pointer is free afterwards every time.

### 3. Relative mouse mode

Start with `--relative-mouse --log-events --log-fpl`.

- [ ] The cursor is invisible and does not move, clicks stay in the window (the frame stays green), and every `mouse move` carries the same `x`/`y`.
- [ ] Deltas are raw: with "Enhance pointer precision" switched on and a fast movement, the relative deltas are not bigger than the same movement done slowly (the cursor movement without the relative mode is). A 1600 DPI mouse gives about 1600 counts per inch.
- [ ] Ctrl+Alt+R ends the mode: the cursor appears again where the mode started and is visible.
- [ ] Ctrl+Alt+W during the mode moves the `x`/`y` of the events, and the cursor appears there when the mode ends.
- [ ] Alt+Tab to another window: the cursor is visible and free, no `mouse move` with deltas while the demo has no focus. Back to the demo: no jump in the first delta.
- [ ] Mouse buttons and the wheel work and carry the frozen position.
- [ ] Windows guest in qemu with `-device usb-tablet` (the tablet sends absolute raw input): in the relative mode the deltas follow the host mouse without jumps, the first movement gives no big delta.
- [ ] Optional: over Remote Desktop (absolute raw input as well) the same.

### 4. Keyboard grab

Start with `--keyboard-grab --log-events --log-fpl`.

- [ ] Win alone: no start menu, the log shows `key=LeftSuper` press and release.
- [ ] Alt+Tab: no window switch, the log shows Alt and Tab.
- [ ] Alt+Esc and Ctrl+Esc: nothing happens outside of the window, the keys are in the log.
- [ ] Alt+F4: the window stays open, the keys are in the log.
- [ ] Print: no snipping tool, `scan=0xe037` in the log.
- [ ] Alt alone and F10 do not open a menu: the next letter is text input (`key input`) as usual.
- [ ] Ctrl+A gives `key input code=1` and `mods=0x4` in the key line.
- [ ] AltGr+Q on a German layout gives `@` (`key input code=64`), no extra left Ctrl press in the log.
- [ ] Ctrl+Alt+Del works (it can not be grabbed), afterwards no key is stuck: the next Ctrl, Alt and Del presses are `state=press`.
- [ ] Ctrl+Alt+K ends the grab while Ctrl and Alt are held, after releasing them they are not stuck (Win opens the start menu again, Alt+Tab switches again).
- [ ] With `--stall=500` the grab keeps working, or the hook is installed again after the first Win or Alt+Tab that got through (`The low level keyboard hook was lost, installing it again` with `--log-fpl`).
- [ ] Alt+Tab while the grab is off (Ctrl+Alt+K) switches windows, with the grab on again it does not.

### 5. Scan codes, buttons, wheels, enter and leave

Start with `--log-events`.

- [ ] Every key of the keyboard gives the scan code of the table below, press and release.
- [ ] Alt+Print gives `scan=0xe037`, Ctrl+Pause gives `scan=0xe11d`, Pause gives `scan=0xe11d`, Num Lock gives `scan=0x45`, the menu key gives `scan=0xe05d`.
- [ ] Enter and keypad Enter are different (`0x1c` and `0xe01c`), so are the left and right Ctrl, Alt and Shift.
- [ ] A dead key (`^` on a German layout) comes as a key press and release, `^` then `1` gives one text input of `¹`.
- [ ] With `--keyboard-grab` the hooked keys give the same codes: left Win `0xe05b`, right Win `0xe05c`, left Ctrl `0x1d`, right Ctrl `0xe01d`, left Alt `0x38`, right Alt `0xe038`, Tab `0xf`, Esc `0x1`.
- [ ] The side buttons come as `name=x1` (back) and `name=x2` (forward).
- [ ] The horizontal wheel comes as `mouse wheel dx=` with a positive value to the right, the vertical wheel as `mouse wheel dy=`, both with window coordinates in `x`/`y` (0,0 is the top left corner of the client area).
- [ ] `mouse enter` and `mouse leave` come once per crossing of the client area edge, also when dragging with a pressed button out of the window.
- [ ] With the mouse grab or the relative mode on, there is no enter or leave while the cursor is held, only the enter when the grab pulls the cursor in.

### Scan code table (PC set 1, as the log writes it)

The key names are the ones of a US keyboard, the scan code belongs to the place of the key: the key that is Z on a US keyboard is Y on a German one and gives `0x2c` on both.

| Row | Keys and scan codes |
|---|---|
| Function row | Esc `0x1`, F1-F10 `0x3b`-`0x44`, F11 `0x57`, F12 `0x58`, Print `0xe037`, Scroll Lock `0x46`, Pause `0xe11d` |
| Number row | `` ` `` `0x29`, 1-9 `0x2`-`0xa`, 0 `0xb`, - `0xc`, = `0xd`, Backspace `0xe` |
| Top row | Tab `0xf`, Q-P `0x10`-`0x19`, [ `0x1a`, ] `0x1b`, Enter `0x1c` |
| Home row | Caps Lock `0x3a`, A-L `0x1e`-`0x26`, ; `0x27`, ' `0x28`, # (ISO, left of Enter) or \\ (US, above Enter) `0x2b` |
| Bottom row | Left Shift `0x2a`, <> (ISO) `0x56`, Z-M `0x2c`-`0x32`, , `0x33`, . `0x34`, / `0x35`, Right Shift `0x36` |
| Space row | Left Ctrl `0x1d`, Left Win `0xe05b`, Left Alt `0x38`, Space `0x39`, Right Alt/AltGr `0xe038`, Right Win `0xe05c`, Menu `0xe05d`, Right Ctrl `0xe01d` |
| Navigation | Insert `0xe052`, Home `0xe047`, Page Up `0xe049`, Delete `0xe053`, End `0xe04f`, Page Down `0xe051` |
| Arrows | Up `0xe048`, Left `0xe04b`, Down `0xe050`, Right `0xe04d` |
| Keypad | Num Lock `0x45`, / `0xe035`, * `0x37`, - `0x4a`, 7 `0x47`, 8 `0x48`, 9 `0x49`, + `0x4e`, 4 `0x4b`, 5 `0x4c`, 6 `0x4d`, 1 `0x4f`, 2 `0x50`, 3 `0x51`, Enter `0xe01c`, 0 `0x52`, . `0x53` |

The same table is `scanCodeTable` in `tests/run_grab_tests.sh`, where the X11 test checks all 105 keys.

### Known Win32 limits, not failures

- The first `window gotfocus` is missing, Win32 loses the events of the window creation.
- The key state is kept per virtual key: both Shift keys (and Enter and keypad Enter) share one state. Pressing the second while the first is held gives `state=repeat`, and releasing both may give only one release. The scan codes are right.
- Under wine only: the menu key comes as `0x5f`, right Ctrl as `0x1d` followed by `0xe01d`, and the horizontal wheel has screen coordinates. Windows must not show any of that.
