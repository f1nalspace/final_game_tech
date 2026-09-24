# Plan: FPL — Tastatur-Grab, Maus-Grab, Warp und relativer Mausmodus

Ziel: FPL bekommt alles, was ein **qemu-Frontend** (wie `ui/sdl2.c` oder `ui/gtk.c`) von der Fenster- und Eingabeschicht braucht, um Tastatur und Maus an den Gast zu übergeben. Dazu gehören **Tastatur-Grab** (Alt+Tab, Super/Win, Alt+Esc und Strg+Esc landen im Fenster), **Maus einsperren**, **Mauszeiger setzen** (Warp) und ein **relativer Mausmodus** mit rohen, unbeschleunigten Deltas. Unterstützt werden **X11 und Win32** gleichwertig, hinter einer kleinen API, die ohne Plattformwissen benutzbar ist.

Dieses Dokument beschreibt den **Stand** (1), die **Designentscheidungen** samt API (2), den **inneren Aufbau** (3), den **Prüfstand** (4) und die **Iterationen** mit Abnahmekriterien (5). Danach folgen Arbeitsregeln, Entscheidungen und Folgepunkte sowie Risiken (6–8).

Quellen der Anforderungen: qemu `master` und v10.1.0 (`ui/sdl2.c`, `ui/gtk.c`, `ui/input.c`, `ui/kbd-state.c`, `ui/win32-kbd-hook.c`, `include/ui/console.h`) sowie SDL2 (`src/video/x11/*`, `src/video/windows/*`), im Quelltext geprüft am 2026-09-24.

---

## 0. Vorab: Branch-Basis

**`fpl/mouse-control` steht auf `3f1c1df4`, dem Kopf von `demo/image-viewer-improvements`, und nicht auf `develop`.** Der Branch trägt damit alle 39 Viewer-Commits, die noch nicht in `develop` sind (der älteste ist `51aa141c`), dazu `plans/` und `prompts/`. Ein späterer Merge nach `develop` würde den halben Bildbetrachter mitziehen. Der Branch hat noch keinen eigenen Commit, und der Arbeitsbaum ist sauber. Deshalb wird er **vor dem ersten Commit** auf `develop` gesetzt: `git switch fpl/mouse-control && git reset --hard develop`. Diese Plandatei ist bis dahin nicht eingecheckt und übersteht den Reset.

---

## 1. Stand

### 1.1 Was FPL heute kann (v1.0.1 auf `develop`)

| Fähigkeit | Win32 | X11 |
|---|---|---|
| Cursor ein-/ausblenden, `fplSetWindowCursorEnabled` | setzt nur ein Flag, wirksam beim nächsten `WM_SETCURSOR` | unsichtbarer Pixmap-Cursor über `XDefineCursor` |
| Cursorposition abfragen, `fplQueryCursorPosition` (Bildschirmkoordinaten) | `GetCursorPos` | `XQueryPointer`, Root-Koordinaten |
| Maus-Events Move/Button/Wheel (Fensterkoordinaten, oben links) | `WM_MOUSEMOVE`, `WM_*BUTTON*`, `WM_MOUSEWHEEL` | `MotionNotify`, `ButtonPress/Release` 1–5 |
| Maus festhalten, solange eine Taste gedrückt ist | `SetCapture`/`ReleaseCapture` | impliziter passiver Grab des X-Servers |
| Fokus-Events `GotFocus`/`LostFocus` | `WM_ACTIVATEAPP` | `FocusIn`/`FocusOut`, `NotifyGrab`/`NotifyUngrab` werden ignoriert |
| Tasten-Events `keyCode` + `mappedKey` | `keyCode` = VK-Code aus `wParam` | `keyCode` = X-Keycode |
| Rohe Plattform-Events | `fplWindowCallbacks.eventCallback` (MSG) | `fplWindowCallbacks.eventCallback` (XEvent) |

### 1.2 Was fehlt

| Fähigkeit | Win32 | X11 | FPL heute |
|---|---|---|---|
| Tastatur-Grab | `WH_KEYBOARD_LL`-Hook | `XGrabKeyboard` | — |
| Maus einsperren | `ClipCursor` | `XGrabPointer` mit `confine_to` | — |
| Mauszeiger setzen (Warp) | `SetCursorPos` | `XWarpPointer` | — |
| Relativer Modus, roh und unbeschleunigt | Raw Input (`WM_INPUT`) | XInput2 `XI_RawMotion` (libXi), Rückfall: Warp zur Mitte | — |
| Bewegungsdeltas im Maus-Event | — | — | — |
| Grab folgt Fokus, Minimieren, Verstecken | — | — | — |

Ein qemu-Frontend braucht darüber hinaus **physische Tastencodes**, die Seitentasten, das horizontale Mausrad und Enter/Leave-Events (2.8).

### 1.3 Befunde im Code

- **Win32-Cursor-Verstecken über Raw Input** (`fpl__Win32SetCursorState`, `final_platform_layer.h:18237`): Beim Verstecken wird die Maus per `RegisterRawInputDevices` (Usage Page 1, Usage 2, Flags 0) für das Fenster angemeldet. Der Kommentar sagt „remove the mouse device entirely“. Ohne `RIDEV_NOLEGACY` wird aber nichts entfernt, und `WM_INPUT` wird nirgends behandelt: Es gibt keinen `case WM_INPUT`, die Nachrichten landen ungenutzt in `DefWindowProc`. Beim Einblenden meldet `RIDEV_REMOVE` die Maus wieder ab. Genau das würde eine Anmeldung des relativen Modus zerstören. **Der relative Modus wird der einzige Besitzer von Raw Input, das Überbleibsel fällt weg** (Iteration 3).
- **Win32 `fplSetWindowCursorEnabled`** (`:22464`) setzt nur `isCursorActive`. Unsichtbar wird der Cursor erst beim nächsten `WM_SETCURSOR`, also erst, wenn sich die Maus bewegt.
- **Win32 `WM_SYSKEYDOWN`/`WM_SYSKEYUP` gehen an `DefWindowProc`** (`:18444`): Alt allein und F10 aktivieren das Fenstermenü (die nächsten Tasten gehen verloren), Alt+Leertaste öffnet das Systemmenü, Alt+F4 schließt das Fenster. Mit Tastatur-Grab muss das alles bei der App ankommen.
- **Win32 pumpt Nachrichten nur in `fplWindowUpdate()`/`fplPollEvent()`** (Message-Fiber, `:18323`). Ein Low-Level-Hook wird im installierenden Thread während dieses Pumpens aufgerufen. Stockt die Hauptschleife länger als `LowLevelHooksTimeout`, wird jede Taste systemweit verzögert, und seit Windows 7 entfernt das System einen Hook, der das Zeitlimit überschreitet, stillschweigend (8).
- **Win32 `keyCode` ist der VK-Code** (`:20092`) und damit **layoutabhängig**: Auf einer deutschen Tastatur liefert die Taste rechts neben T `VK_Z`. Den Scancode (`lParam` Bits 16–23, Erweiterungsbit 24) wirft FPL weg. Ziffernblock-Enter und Enter sind nicht unterscheidbar, und die ISO-Taste `<>` (`VK_OEM_102`) hat kein `fplKey`. Unter X11 ist `keyCode` der X-Keycode (evdev + 8). **Es gibt keinen plattformneutralen physischen Tastencode**, qemu braucht aber genau den (2.8).
- **Enter/Leave:** X11 wählt `EnterWindowMask`/`LeaveWindowMask` aus (`:27950`), behandelt `EnterNotify`/`LeaveNotify` aber nicht. Win32 hat kein `TrackMouseEvent`/`WM_MOUSELEAVE`. FPL meldet also nie, dass die Maus das Fenster betritt oder verlässt.
- **X11-Maustasten 8/9** werden nicht auf `fplMouseButtonType_X1`/`X2` abgebildet, das gibt es nur unter Win32 (schon im Viewer-Plan 7.2 vermerkt).
- **Horizontales Mausrad:** `WM_MOUSEHWHEEL` ist ein Platzhalter („Step 9“, `:20176`), X11-Tasten 6/7 werden ignoriert, und `fplMouseEvent` hat nur ein `wheelDelta`.
- **`fplMouseState.wheelDeltaX/Y`** sind als „accumulated since last poll“ dokumentiert, werden aber nirgends geschrieben.
- **Tastenzustand nach Fokusverlust (Verdacht, wird in Iteration 0 geprüft):** Wird eine Taste losgelassen, während das Fenster keinen Fokus hat, sieht FPL das `Release` nie. `fpl__HandleKeyboardButtonEvent` (`:14533`) meldet den nächsten Druck dieser Taste dann als `Repeat`. Typischer Fall: Alt+Tab weg und zurück. SDL gleicht das beim Fokusverlust aus (`SDL_ResetKeyboard`), qemu-gtk mit `qkbd_state_lift_all_keys`.
- **X11-Loader:** Weder `XGrabPointer`, `XUngrabPointer`, `XGrabKeyboard`, `XUngrabKeyboard` und `XWarpPointer` noch `XQueryExtension`, `XGetEventData` und `XFreeEventData` werden geladen, libXi gar nicht. Für `FPL_NO_PLATFORM_INCLUDES` müssen alle Konstanten (`GrabModeAsync`, `GrabSuccess`, `AlreadyGrabbed`, `GrabNotViewable`, `GrabFrozen`, `XI_RawMotion`, `XIAllMasterDevices`) und Typen (`XGenericEventCookie`, `XIEventMask`, `XIRawEvent`, `XIValuatorState`) selbst definiert werden, wie beim Rest der X11-ABI.

---

## 2. Designentscheidungen

### 2.1 Die API

Drei Stellschrauben, jede mit Setter und Getter nach dem Muster der vorhandenen Fensterfunktionen:

```c
/**
* @enum fplWindowMouseMode
* @brief How the mouse cursor behaves inside the window.
*/
typedef enum fplWindowMouseMode {
	//! The cursor moves freely, mouse events carry window positions (Default).
	fplWindowMouseMode_Free = 0,
	//! The cursor cannot leave the client area, mouse events carry window positions.
	fplWindowMouseMode_Confined,
	//! The cursor is hidden and locked, move events carry raw unaccelerated deltas.
	fplWindowMouseMode_Relative,
} fplWindowMouseMode;

fpl_platform_api bool fplSetWindowMouseMode(const fplWindowMouseMode mode);
fpl_platform_api fplWindowMouseMode fplGetWindowMouseMode(void);

//! System shortcuts (Alt+Tab, Super/Win, Alt+Esc, Ctrl+Esc, Alt+F4) go to the window instead of the window manager or the shell.
fpl_platform_api bool fplSetWindowKeyboardGrabEnabled(const bool enabled);
fpl_platform_api bool fplIsWindowKeyboardGrabEnabled(void);

//! Moves the cursor to a position in window coordinates, the same coordinates the mouse events use.
fpl_platform_api bool fplSetWindowCursorPosition(const int32_t x, const int32_t y);
```

Dazu bekommt `fplMouseEvent` zwei Felder:

```c
	//! Movement since the previous move event in pixels, or the raw device counts in relative mode.
	int32_t deltaX;
	//! Movement since the previous move event in pixels, or the raw device counts in relative mode.
	int32_t deltaY;
```

Warum diese Form:
- **Ein Modus statt zweier Schalter für Maus-Grab und relativ.** Relativ ohne Einsperren ergibt nirgends einen Sinn, und relativ mit sichtbarem Cursor auch nicht. Mit einem Enum gibt es keine ungültigen Kombinationen.
- **Tastatur-Grab getrennt**, weil er unabhängig ist: qemu im absoluten Modus (usb-tablet) greift nur die Tastatur, ein Spiel im relativen Modus meistens nur die Maus.
- **Warp in Fensterkoordinaten**, weil qemu (`dpy_mouse_set`) und die Warp-Rückfallebene genau die brauchen. `fplQueryCursorPosition` bleibt bei Bildschirmkoordinaten.
- **Deltas in jedem Move-Event**, nicht nur im relativen Modus. Wer nur `mouseX`/`mouseY` liest, merkt nichts.

So sieht ein qemu-Frontend damit aus:

```c
// Grab after a click or Ctrl+Alt+G, the guest has a relative mouse (PS/2)
fplSetWindowKeyboardGrabEnabled(true);
fplSetWindowMouseMode(fplWindowMouseMode_Relative);

// Inside the event loop
if (ev.mouse.type == fplMouseEventType_Move && isGrabbed) {
	qemu_input_queue_rel(console, INPUT_AXIS_X, ev.mouse.deltaX);
	qemu_input_queue_rel(console, INPUT_AXIS_Y, ev.mouse.deltaY);
	qemu_input_event_sync();
}

// Ungrab
fplSetWindowMouseMode(fplWindowMouseMode_Free);
fplSetWindowKeyboardGrabEnabled(false);
```

### 2.2 Zustandsmodell: angefordert und wirksam

- FPL speichert den **angeforderten** Zustand, und die Getter liefern ihn. **Wirksam** ist er nur, solange das Fenster den Fokus hat, sichtbar und nicht minimiert ist. Das ist das Modell von SDL (`SDL_UpdateWindowGrab`).
- **Fokusverlust, Minimieren, Verstecken, der modale Verschiebe-/Größen-Loop unter Win32 und `fplWindowShutdown()`** heben den Grab beim Betriebssystem auf, die Anforderung bleibt. Kommt der Fokus zurück, stellt FPL den Grab wieder her. Wer die qemu-SDL-Semantik will (Fokusverlust beendet den Grab), setzt bei `LostFocus` selbst `Free` und `false`.
- Eine **zentrale Funktion** `fpl__UpdateInputGrab()` gleicht angefordert und wirksam ab. Aufgerufen wird sie von den Settern, bei Fokus, Minimieren/Wiederherstellen, Zeigen/Verstecken, Vollbild, Größe/Position (Win32-Clip-Rechteck) und beim Beenden. Eine zweite Stelle, die Grabs anfasst, gibt es nicht.
- **Vorübergehende Fehlschläge werden wiederholt:** Direkt nach Alt+Tab hält KWin die Tastatur oft noch selbst, `XGrabKeyboard` liefert dann `AlreadyGrabbed`. SDL wiederholt blockierend 100 × 50 ms. FPL wiederholt stattdessen **nicht blockierend** in `fplWindowUpdate()`/`fplPollEvent()`, höchstens alle `grabRetryIntervalMilliseconds` (50 ms), solange angefordert und fokussiert.
- **Rückgabewert der Setter:** `false` nur, wenn der Wunsch grundsätzlich nicht erfüllbar ist (kein Fenster, ungültiger Wert, Plattform ohne Umsetzung). Ein vorübergehender Fehlschlag gibt `true` zurück und wird wiederholt.
- **Fokusverlust lässt alle gedrückten Tasten und Maustasten los** (Release-Events, wie SDL). Sonst bleiben nach einem aufgehobenen Grab Strg oder Alt im Gast hängen (Entscheidung 7.1).
- **Cursor-Sichtbarkeit:** Der relative Modus versteckt den Cursor, egal was `fplSetWindowCursorEnabled` sagt. Beim Verlassen gilt wieder dessen Wert. `Confined` lässt die Sichtbarkeit, wie sie ist.
- **Verlassen des relativen Modus:** Der Cursor erscheint an der Stelle, an der der Modus begonnen hat (eingefrorene Position). So macht es qemu-gtk (`gd_ungrab_pointer`), und so erwarten es Spiele.

### 2.3 Maus einsperren (`Confined`)

**X11:** `XGrabPointer(display, window, owner_events=True, ButtonPress|ButtonRelease|PointerMotion, GrabModeAsync, GrabModeAsync, confine_to=window, cursor=None, CurrentTime)`. `GrabSuccess` heißt wirksam. `AlreadyGrabbed`, `GrabFrozen` und `GrabInvalidTime` führen zur Wiederholung (2.2). `GrabNotViewable` (Fenster nicht abgebildet) wird bei `Shown`/`Restored` erneut versucht. Ändert sich die Fenstergeometrie, hält der X-Server den Zeiger von selbst im Fenster. Aufheben mit `XUngrabPointer` + `XFlush`.

**Win32:** `ClipCursor` auf das Client-Rechteck in Bildschirmkoordinaten. Wie bei SDL wird nur gesetzt, wenn sich das Rechteck von `GetClipCursor` unterscheidet, und `ClipCursor(NULL)` nur gerufen, wenn das aktuelle Rechteck das eigene ist. Neu gesetzt wird bei Fokus, `WM_WINDOWPOSCHANGED`, `WM_EXITSIZEMOVE`, Vollbildwechsel und zusätzlich alle `clipRefreshIntervalMilliseconds` (3 s, wie SDL) beim Pumpen, weil andere Programme das Rechteck zurücksetzen können. Während eines Klicks auf die Titelleiste und im modalen Verschiebe-Loop wird nicht eingesperrt.

### 2.4 Relativer Modus

Gemeinsam: Der Cursor ist versteckt und festgehalten. Move-Events tragen in `deltaX`/`deltaY` die **rohen Gerätezählwerte ohne Beschleunigung**. `mouseX`/`mouseY` bleiben auf der eingefrorenen Position, ebenso bei Tasten- und Rad-Events. Bruchteile (XI2 liefert Festkommawerte, absolute Geräte werden umgerechnet) sammelt FPL in einem Rest, damit nichts verloren geht. Jedes Rohpaket wird ein eigenes Move-Event, zusammengefasst wird nicht.

**Win32:**
- Einschalten: Position merken (`GetCursorPos`), Cursor verstecken, `RegisterRawInputDevices({0x01, 0x02, dwFlags=0, hwndTarget=window})`. Ohne `RIDEV_NOLEGACY` (Tasten-Nachrichten, `SetCapture` und die Titelleiste funktionieren weiter) und ohne `RIDEV_INPUTSINK` (nur im Vordergrund). `ClipCursor` auf ein 1×1-Rechteck in der Client-Mitte, damit Klicks im Fenster landen und der unsichtbare Cursor nirgends hin kann.
- `WM_INPUT`: `GetRawInputData` in einen Stack-Puffer. `MOUSE_MOVE_RELATIVE` liefert `lLastX`/`lLastY` direkt. **`MOUSE_MOVE_ABSOLUTE`** (RDP, VM-Tablets wie qemu usb-tablet oder VMware, Stifte) wird von 0..65535 auf den (virtuellen) Desktop umgerechnet (`MOUSE_VIRTUAL_DESKTOP`) und gegen die vorige Position gerechnet. Das erste Paket setzt nur die Basis. Aus Touch erzeugte Eingaben (`GetMessageExtraInfo() & 0x80`) werden übersprungen. `WM_INPUT` geht danach weiter an `DefWindowProc`, wie verlangt.
- `WM_MOUSEMOVE` wird im relativen Modus ignoriert, Tasten und Rad kommen weiter über die normalen Nachrichten, mit der eingefrorenen Position.
- Ausschalten: `RIDEV_REMOVE`, Clip-Rechteck zurück (frei oder `Confined`), `SetCursorPos` auf die eingefrorene Position, Sichtbarkeit wiederherstellen.

**X11, bevorzugt XInput2:**
- Beim Fensteraufbau wird libXi **optional** geladen (`libXi.so.6`, `libXi.so`, wie Xinerama). Danach folgen `XQueryExtension("XInputExtension")` für den Opcode und `XIQueryVersion` ≥ 2.0. Fehlt etwas, gilt die Rückfallebene.
- Einschalten: `XISelectEvents(root, XIAllMasterDevices, XI_RawMotion)`, und zwar **nur während der relative Modus wirksam ist**. SDL wählt es dauerhaft aus, das erzeugt ohne Nutzen Verkehr für jede Mausbewegung auf dem ganzen Bildschirm. Dazu `XGrabPointer` mit `confine_to=window` und dem unsichtbaren Cursor, dann ein Warp in die Fenstermitte.
- Event-Schleife: `GenericEvent` → `XGetEventData` → Opcode und `XI_RawMotion` prüfen → `raw_values` der Valuatoren 0 und 1 (Reihenfolge nach gesetzten Maskenbits) → Delta. **Absolute Valuatoren** (VM-Tablets, Grafiktabletts) werden pro `sourceid` über `XIQueryDevice` erkannt und gegen den Vorwert gerechnet. SDL rechnet dort `prev - cur`, das Vorzeichen sieht verdreht aus, FPL rechnet `cur - prev`. Danach `XFreeEventData`. Core-`MotionNotify` wird im relativen Modus ignoriert.
- Ausschalten: leere Maske, `XUngrabPointer` (oder zurück zu `Confined`), `XWarpPointer` auf die eingefrorene Position.

**X11-Rückfallebene ohne XI2 (Warp zur Mitte):** Pointer-Grab und unsichtbarer Cursor wie oben. Jedes `MotionNotify` außerhalb der Mitte ergibt `delta = position - mitte`, danach folgt ein Warp zur Mitte. Ein `MotionNotify` genau auf der Mitte ist das Echo des Warps und wird verworfen. Diese Deltas sind **beschleunigt**, das wird dokumentiert. Die Rückfallebene lässt sich mit dem neuen Define `FPL_NO_X11_XINPUT2` erzwingen. Das braucht der Test, und `FPL_NO_RUNTIME_LINKING`-Builds kommen damit ohne `-lXi` aus.

### 2.5 Tastatur-Grab

**X11:** `XGrabKeyboard(display, window, owner_events=True, GrabModeAsync, GrabModeAsync, CurrentTime)`, Ergebnis und Wiederholung wie beim Pointer. Der aktive Grab schlägt die passiven Grabs des Fenstermanagers. Deshalb kommen Alt+Tab, Super, Alt+F4 und Druck beim Fenster an, und Text-Events funktionieren weiter. **Nicht abfangbar** sind Aktionen, die der X-Server selbst ausführt, bevor ein Client etwas sieht (VT-Wechsel mit Strg+Alt+F1…F12, Strg+Alt+Rücktaste, falls aktiviert), sowie Magic SysRq. Fokuswechsel, die während des eigenen Grabs entstehen, kommen als `NotifyGrab`/`NotifyUngrab` und werden wie bisher ignoriert. Ein echter Fokusverlust setzt den Grab aus (2.2), das wird in Iteration 4 geprüft.

**Win32:** Ein `WH_KEYBOARD_LL`-Hook, **im Fensterthread**, installiert nur, solange der Grab wirksam ist. So machen es SDL und qemu, und so bleibt die Reihenfolge der Tasten erhalten (8).
- Der Hook fängt, wenn das eigene Fenster im Vordergrund ist, **LWin, RWin, LAlt, RAlt, LStrg, RStrg, Tab und Esc** ab. Er meldet sie direkt über `fpl__HandleKeyboardButtonEvent` und gibt `1` zurück. Die Modifier gehören dazu, damit Alt und Tab denselben Weg in derselben Reihenfolge nehmen. Das ist die SDL-Menge. qemu reicht die Modifier dagegen durch und riskiert damit vertauschte Reihenfolgen.
- Alle anderen Tasten laufen normal über `WM_KEYDOWN`, samt Text-Events. Weil Alt und Strg geschluckt werden, kennt das System sie nicht als gedrückt. FPL führt den Zustand dieser Modifier deshalb selbst und mischt ihn in die `modifiers` jedes Tasten-Events. Nebeneffekt: Alt+F4 schließt nicht, F10 und Alt+Buchstabe aktivieren kein Menü.
- Das **falsche linke Strg von AltGr** (Scancode `0x21D`) wird geschluckt, AltGr kommt als RAlt an.
- Eine Taste, die schon vor dem Einschalten gedrückt war, bekommt ihr Loslassen einmal durchgereicht, damit das System keine hängende Taste behält (SDL).
- Solange der Grab wirksam ist, gehen `WM_SYSKEYDOWN`, `WM_SYSKEYUP`, `WM_SYSCHAR` und `WM_SYSCOMMAND/SC_KEYMENU` nicht an `DefWindowProc`.
- **Nicht abfangbar:** Strg+Alt+Entf (Secure Attention Sequence) und Win+L. Danach sorgt 2.2 (Loslassen bei Fokusverlust) dafür, dass Strg und Alt nicht hängen bleiben.
- **Hook verloren:** Windows entfernt einen Hook, der das Zeitlimit überschreitet, ohne Nachricht. Kommt bei wirksamem Grab ein `WM_KEYDOWN` für LWin/RWin oder Tab mit Alt an, war der Hook weg, und FPL installiert ihn neu.

### 2.6 Warp

`fplSetWindowCursorPosition(x, y)` nimmt Fensterkoordinaten, der Ursprung liegt oben links wie bei den Maus-Events. Unter Win32 folgen `ClientToScreen` und `SetCursorPos`, unter X11 `XWarpPointer(display, None, window, 0, 0, 0, 0, x, y)` und `XFlush`. Das System schickt danach selbst eine Bewegung. FPL setzt seine „letzte Position“ vorher auf das Ziel, sodass dieses Move-Event das Delta 0 hat. Die SDL-Maßnahmen gegen Win32-Zittern (dreifaches `SetCursorPos`, verspätete `WM_MOUSEMOVE` verwerfen) werden nur übernommen, wenn der Test das Problem zeigt. Bei `Confined` wird das Ziel auf den Client-Bereich begrenzt. **Im relativen Modus wird nicht wirklich gewarpt**, es wird nur die eingefrorene Position versetzt, an der der Cursor beim Verlassen wieder erscheint. qemu warpt so bei `dpy_mouse_set`. Gibt es kein Fenster oder ist es versteckt oder minimiert, liefert die Funktion `false`.

### 2.7 Deltas in Move-Events

- `Free` und `Confined`: Differenz zum vorigen Move-Event des Fensters. Nach Enter, Fokusgewinn und Warp ist das Delta 0.
- `Relative`: rohe Zählwerte (2.4).
- `fplMouseState` (Polling) bekommt vorerst keine Deltas, qemu arbeitet mit Events (7.2).

### 2.8 Was ein qemu-Frontend darüber hinaus braucht

| Bedarf | Wofür in qemu | FPL heute | Vorschlag | Wo |
|---|---|---|---|---|
| **Physischer Tastencode** `fplKeyboardEvent.scanCode` | `qkbd_state_key_event` erwartet in `master` Linux-evdev-Codes, in v10.1 QCodes, beide aus der **physischen** Taste. Umrechnungstabellen `atset1_to_linux` und `usb_to_linux` gibt es in qemu. | VK-Code bzw. X-Keycode | **PC-Scancode Satz 1**, erweiterte Tasten als `0xE0xx`, Pause als `0xE11D`, 0 = unbekannt. Win32 liefert ihn direkt aus `lParam`, X11 rechnet evdev (`keycode - 8`) über eine Tabelle um (1–83 sind identisch, der Rest ≈ 40 Einträge). | Iteration 5 |
| Maustasten 8/9 unter X11 | `INPUT_BUTTON_SIDE`/`EXTRA` | nur Win32 | X1/X2 zuordnen | Iteration 5 |
| Horizontales Rad | `WHEEL_LEFT`/`WHEEL_RIGHT` | fehlt | `fplMouseEvent.wheelDeltaX`, Win32 `WM_MOUSEHWHEEL`, X11-Tasten 6/7 | Iteration 5 |
| Enter/Leave | absoluter Modus: Grab folgt dem Zeiger (sdl2), „Grab On Hover“ (gtk) | fehlt | `fplMouseEventType_Enter`/`_Leave`. X11 `EnterNotify`/`LeaveNotify` ohne Grab/Ungrab/Inferior, Win32 `TrackMouseEvent` + `WM_MOUSELEAVE` | Iteration 5 |
| Cursorbild aus RGBA mit Hotspot | `dpy_cursor_define` (Gast-Hardwarecursor von virtio-gpu, vmware-svga, qxl) | nur ein/aus | `fplSetWindowCursorImage`, Win32 `CreateIconIndirect`, X11 libXcursor mit 1-Bit-Rückfall | Folgeplan (7.2) |
| Lock-LEDs abgleichen | wird weder von sdl2 noch von gtk gemacht | Lock-Zustand in `modifiers` | nichts | — |
| Tastenwiederholung | `qkbd_state` gibt Wiederholungen als Autorepeat weiter | `fplButtonState_Repeat` | nichts | — |
| Event-Pumpe im qemu-Hauptloop | sdl2 pumpt in `dpy_refresh` (Standard 30 ms) | vorhanden | nichts, wie sdl2 | — |

---

## 3. Innerer Aufbau

Plattformneutraler Zustand in `fpl__PlatformWindowState`:

```c
typedef struct fpl__InputGrabState {
	// Requested by the user, kept while the window has no focus
	fplWindowMouseMode requestedMouseMode;
	fpl_b32 requestedKeyboardGrab;
	// What is currently applied at the operating system
	fplWindowMouseMode appliedMouseMode;
	fpl_b32 appliedKeyboardGrab;
	// Position where relative mode started, the cursor returns there
	int32_t frozenX;
	int32_t frozenY;
	// Last window position of a move event, base for the deltas in free and confined mode
	int32_t lastMoveX;
	int32_t lastMoveY;
	fpl_b32 hasLastMove;
	// Fractional parts of raw deltas that are not reported yet
	double remainderX;
	double remainderY;
	// Non-blocking retry of a failed grab
	uint64_t nextRetryTimeMilliseconds;
} fpl__InputGrabState;
```

Plattformteile:
- **Win32** (`fpl__Win32WindowState`): `HHOOK keyboardHook`, `bool rawMouseRegistered`, `RECT appliedClipRect`, `bool ownsClip`, `uint64_t nextClipRefreshTime`, der selbst geführte Zustand der geschluckten Modifier, das Bitfeld „vor dem Hook gedrückt“, die letzte absolute Rohposition.
- **X11** (`fpl__X11WindowState`): `bool pointerGrabbed`, `bool keyboardGrabbed`. XI2-Zustand: Opcode, verfügbar, ausgewählt, Tabelle absoluter Quellgeräte, vorige absolute Werte. Dazu die Warp-Mitte der Rückfallebene. `fpl__X11SubplatformState` bekommt `fpl__XInput2Api xinput2` neben `xrandr` und `xinerama`.

Ablauf:
- `fpl__UpdateInputGrab(appState)` ist der einzige Ort, der wirksame Grabs setzt oder aufhebt. Er vergleicht angefordert mit wirksam unter der Bedingung „fokussiert, sichtbar, nicht minimiert“ und ruft die Plattformfunktionen `fpl__Win32ApplyMouseMode`/`fpl__X11ApplyMouseMode` und `…ApplyKeyboardGrab`.
- Win32: `WM_INPUT` wird im Fensterproc an `fpl__InputBackendWin32_HandleNativeEvent` weitergereicht, wie die übrigen Maus-Nachrichten. Der Hook-Callback ruft direkt `fpl__HandleKeyboardButtonEvent`.
- X11: `GenericEvent` kommt als neuer Fall in die Event-Schleife neben `KeyPress`/`MotionNotify` und wird an `fpl__InputBackendX11Kbm_HandleNativeEvent` weitergereicht.
- Die Wiederholung aus 2.2 und die Clip-Auffrischung aus 2.3 laufen am Ende von `fplWindowUpdate()` und `fplPollEvent()`.
- Loslassen bei Fokusverlust: Über `keyStates` und `mouseStates` wird für alles, was gedrückt ist, ein Release-Event geschickt, bevor `LostFocus` in die Queue geht.

Neue X11-Funktionen im Loader: `XGrabPointer`, `XUngrabPointer`, `XGrabKeyboard`, `XUngrabKeyboard`, `XWarpPointer`, `XQueryExtension`, `XGetEventData`, `XFreeEventData` (alle aus libX11). Dazu `XIQueryVersion`, `XISelectEvents` und `XIQueryDevice`/`XIFreeDeviceInfo` aus libXi. Jede davon kommt in alle vier Stellen: Typedef, `extern` für `FPL_NO_RUNTIME_LINKING`, Api-Struct, Loader.

---

## 4. Prüfstand

### 4.1 Demo `demos/FPL_InputGrab`

C99, Software-Backbuffer (kein GL nötig):
- Die Demo zeichnet ein Fadenkreuz an der Mausposition, im relativen Modus an einer virtuellen Position, die aus den Deltas aufsummiert und am Rand gestoppt wird. Die Titelzeile zeigt Modus, Tastatur-Grab, Fokus und die letzte Taste.
- `--log-events` schreibt jedes Event als eine maschinenlesbare Zeile auf stdout (`move x=… y=… dx=… dy=…`, `key down vk=… scan=… mods=…`, `focus lost`). Die Testskripte werten genau das aus.
- Tasten nach qemu-Art: Strg+Alt+G schaltet den vollen Grab (Tastatur + relativ) um, Strg+Alt+M den Mausmodus, Strg+Alt+K den Tastatur-Grab, Strg+Alt+W warpt zur Mitte, Strg+Alt+Q beendet. Die Tastenkürzel selbst werden nicht als „Gast-Taste“ geloggt.
- Parameter: `--mouse-mode=free|confined|relative`, `--keyboard-grab`, `--timeout=<s>` (beendet sich selbst, Sicherheitsnetz), `--selftest` (Warp-Prüfungen ohne Nutzer), `--no-grab` (zum Debuggen, 8), `--log-core-motion` (im relativen Modus zusätzlich die beschleunigte Core-Bewegung, Iteration 3), `--stall=<ms>` (künstliche Pause pro Frame für den Hook-Test, 4.4).
- CMake ist maßgeblich, `premake5.lua`, `Makefile` und `.vcxproj` werden nachgezogen.

### 4.2 Automatische Tests unter X11

`demos/FPL_InputGrab/tests/run_grab_tests.sh` treibt die Demo mit `xdotool` (XTEST-Eingaben unterliegen Grabs und Confinement wie echte). Jeder Lauf läuft unter `timeout -s KILL`, denn stirbt der Client, gibt der X-Server alle Grabs frei. Ein X-Fehler wird wie im Viewer bis zu dreimal wiederholt. **Das Skript greift für einige Sekunden die echte Tastatur und Maus des Nutzers.** Es sagt das vor dem Start an und läuft nicht in Schleifen.

### 4.3 Win32

- **Bauen:** `x86_64-w64-mingw32-gcc` und `i686-w64-mingw32-gcc` sind lokal da, `clang-cl` + `lld-link` ebenfalls. MSVC über `.vcxproj` beim Nutzer.
- **Rauchtest:** `wine` ist installiert. Dort laufen ClipCursor, Raw Input und LL-Hooks grundsätzlich, aber nicht in jeder Einzelheit wie unter Windows, deshalb zählt wine nur als Vortest.
- **Abnahme auf echtem Windows** mit einer Checkliste pro Iteration (4.4). Dazu kommt ein Windows-Gast in qemu mit `-device usb-tablet`: Der liefert `MOUSE_MOVE_ABSOLUTE`-Rohdaten und prüft damit genau den VM-Pfad aus 2.4.

### 4.4 Handprüfliste Win32 (wächst mit den Iterationen)

Die Liste steht vollständig in der Demo-Readme, hier die Kernpunkte: Das Clip-Rechteck folgt beim Verschieben, bei Größenänderung und im Vollbild. Nach Alt+Tab ist die Maus frei und kommt danach wieder in den Käfig. Im relativen Modus steht der Cursor still und ist unsichtbar, Klicks bleiben im Fenster, eine 1600-DPI-Maus liefert Rohzählwerte, und beim Verlassen erscheint der Cursor am Ausgangspunkt. Beim Tastatur-Grab öffnet Win kein Startmenü, Alt+Tab wechselt nicht, Alt+F4 schließt nicht, AltGr+Q liefert `@`, Strg+Alt+Entf funktioniert, und danach hängt keine Taste. Mit `--stall=500` geht der Hook nicht verloren, oder er wird neu installiert.

---

## 5. Iterationen

Die Reihenfolge geht vom Fundament (Zustandsmodell, Warp, Deltas) über das Einsperren zum relativen Modus, weil der auf dem Einsperren aufsetzt. Der Tastatur-Grab ist unabhängig davon und steht nach der Maus, weil er unter Win32 das meiste Risiko trägt. Jede Iteration ist für sich lieferbar: Modi, die noch fehlen, lehnt der Setter mit `false` ab.

### Iteration 0 — Branch, Demo, Ausgangsstand

- Branch auf `develop` setzen (Abschnitt 0).
- Demo-Gerüst aus 4.1 mit heutiger FPL: Events loggen, Fadenkreuz, Titelzeile, `--timeout`, `--log-events`. Testskript mit den Fällen, die heute schon gehen (Move, Tasten).
- Ausgangsstand festhalten: Alt+Tab und Super gehen an KWin (aktives Fenster wechselt, per `xdotool getactivewindow`). Den Verdacht aus 1.3 prüfen (nach Alt+Tab weg und zurück meldet der nächste Alt-Druck `Repeat`?), das Ergebnis kommt in 1.3.
- **Abnahme:** Die Demo baut mit gcc und clang unter Linux und mit MinGW x64/x86, das Testskript läuft durch und dokumentiert den Ausgangsstand.

### Iteration 1 — API, Zustandsmodell, Warp, Deltas

- Öffentliche Deklarationen aus 2.1 mit Doxygen, Abschnitt „Maus- und Tastatursteuerung“ in `final_platform_layer.docs` (neben `section_category_window_style_cursor`).
- `fpl__InputGrabState`, `fpl__UpdateInputGrab` mit allen Aufrufstellen aus 3. Die Setter nehmen vorerst nur `Free` und `false` an.
- `fplSetWindowCursorPosition` unter Win32 und X11 (`XWarpPointer` in den Loader).
- `deltaX`/`deltaY` in Move-Events (2.7).
- Loslassen aller gedrückten Tasten und Maustasten bei Fokusverlust (falls in 7.1 so entschieden).
- `--selftest`: Warp auf fünf Positionen (Ecken, Mitte) → `fplQueryCursorPosition` = Client-Ursprung + Ziel, das nächste Move-Event hat Delta 0.
- **Abnahme:** Der Selbsttest ist unter X11 grün, unter wine und auf Windows ebenso. Bei Handbewegung ist die Summe der Deltas gleich der Positionsdifferenz (Testskript mit `xdotool mousemove`). Kein hängender Modifier nach Alt+Tab. Die Build-Matrix aus 6 ist grün.

### Iteration 2 — Maus einsperren

- X11 `XGrabPointer`/`XUngrabPointer` mit Konstanten, Ergebnisbehandlung und Wiederholung. Win32 `ClipCursor` nach 2.3.
- **Abnahme X11 (automatisch):** Nach `xdotool mousemove` in eine Bildschirmecke liegt der Zeiger im Client-Rechteck (`xdotool getmouselocation`). Nach einem Fokuswechsel auf ein anderes Fenster ist er frei, nach der Rückkehr wieder eingesperrt. Nach einer Größenänderung folgt der Käfig. Ein zweiter Client mit gehaltenem Pointer-Grab (`AlreadyGrabbed`) wird überstanden: Der Grab greift, sobald der andere loslässt. Win32 nach Handprüfliste.

### Iteration 3 — Relativer Modus

- Win32 Raw Input nach 2.4, das Raw-Input-Überbleibsel aus `fpl__Win32SetCursorState` entfällt.
- X11: libXi optional laden, XI2-Pfad, Rückfallebene, `FPL_NO_X11_XINPUT2`.
- **Abnahme X11 (automatisch):** Eine Folge `xdotool mousemove_relative` mit bekannten Summen ergibt exakt dieselbe Deltasumme. Dass die Deltas roh sind, wird von Hand belegt: Die Demo loggt mit `--log-core-motion` zusätzlich die Core-Bewegung, und bei schneller Bewegung mit einer echten Maus und eingeschalteter Beschleunigung ist die Core-Summe größer als die XI2-Summe. Der Zeiger ist unsichtbar und bleibt im Fenster. Nach dem Verlassen steht er am Ausgangspunkt. Bei Fokusverlust kommen keine Deltas mehr, und der Zeiger ist frei. Die Rückfallebene besteht dieselben Tests bis auf die Beschleunigung. Ob XTEST überhaupt `XI_RawMotion` erzeugt, wird zuerst geprüft, sonst wird der XI2-Teil von Hand abgenommen.
- **Abnahme Win32:** Handprüfliste sowie der qemu-Gast mit usb-tablet (absoluter Rohpfad).

### Iteration 4 — Tastatur-Grab

- X11 `XGrabKeyboard`/`XUngrabKeyboard` nach 2.5, Fokusverhalten während des Grabs prüfen.
- Win32-Hook nach 2.5: SDL-Tastenmenge, eigene Modifier-Führung, AltGr, einmaliges Durchreichen, `DefWindowProc`-Sperre, Neuinstallation.
- **Abnahme X11 (automatisch):** Mit Grab loggt die Demo `alt+Tab`, `super` und `alt+F4`, und das aktive Fenster wechselt nicht. Ohne Grab wechselt es. Text-Events kommen während des Grabs weiter an. Win32 nach Handprüfliste.

### Iteration 5 — Was qemu darüber hinaus braucht (Umfang nach 7.1)

- `scanCode` (Satz 1) unter Win32 und X11, evdev→Satz-1-Tabelle.
- X11-Maustasten 8/9 → X1/X2, horizontales Rad auf beiden Plattformen.
- `fplMouseEventType_Enter`/`_Leave`.
- **Abnahme:** Dieselbe physische Taste liefert unter `setxkbmap us` und `setxkbmap de` denselben `scanCode`. Alle Tasten einer 105er-Tastatur werden gegen eine Erwartungstabelle geprüft, auch Ziffernblock-Enter, `<>`, Pause und Druck. Die Keycodes schickt python-xlib über XTEST, weil `xdotool key` nur Keysyms kennt. Die Win32-Scancodes stimmen mit denselben Erwartungswerten überein (Handprüfung mit der Demo). Enter/Leave erscheinen genau einmal pro Übergang und nicht bei eigenen Grabs.

### Iteration 6 — Abschluss

- Changelog vollständig, Doku-Seite mit Beispiel (qemu-artige Schleife aus 2.1), Readme der Demo mit Handprüfliste.
- `FPL_Test`: reine Hilfsfunktionen ohne Fenster prüfen (Umrechnung absolut → relativ, Restsammlung, evdev→Satz-1-Tabelle ohne Doppelungen, `sizeof(fplMouseEvent)` und `sizeof(fplEvent)` in `TestSizes`).
- Build-Matrix aus 6 und die Win32-Abnahme auf echtem Windows.

---

## 6. Arbeitsregeln

- Code-Stil nach `CLAUDE.md`: benannte Zwischenvariablen statt verschachtelter Aufrufe, keine magischen Zahlen (Wiederholungsintervall, Clip-Auffrischung, Rohwert-Normierung 65535, Scancode-Präfixe sind benannte Konstanten), keine Umbrüche auf Spaltenbreite, geschweifte Klammern um jeden Bedingungskörper, Kommentare auf Englisch, auch in Shell-Skripten.
- **FPL-Änderungen nur auf diesem `fpl/*`-Branch**, der danach nach `develop` gemergt wird. Das qemu-Frontend selbst entsteht woanders und holt sich `develop`.
- **Version:** Kein neuer Versionsabschnitt, alles kommt in den noch unveröffentlichten Abschnitt **v1.0.1** (Window, Input, X11), wie die anderen `fpl/*`-Branches (`master` steht auf v1.0.0).
- **CMake ist maßgeblich** für die neue Demo, `.vcxproj` und premake werden nachgezogen.
- **Build-Matrix pro Iteration:** `FPL_InputGrab`, `FPL_Window`, `FPL_Input`, `FPL_Test`, `FPL_NoRuntimeLinking` (mit und ohne `FPL_NO_X11_XINPUT2`), `FPL_NoPlatformIncludes`, eine C++-Demo (`FPL_ImGui`), jeweils gcc und clang, dazu MinGW x64/x86 für die Win32-relevanten Demos.
- **Tests, die greifen, nur mit Ansage**, immer unter `timeout -s KILL`, nie in Schleifen mit sichtbaren Fenstern ohne Warnung.
- Commits macht der Nutzer (oder auf Zuruf), klein und nach Thema getrennt.

---

## 7. Entscheidungen und Folgepunkte

### 7.1 Zu entscheiden (mit Empfehlung)

| Punkt | Empfehlung | Alternative | Wo |
|---|---|---|---|
| API-Form | ein Maus-Modus-Enum + Tastatur-Schalter + Warp | getrennte Schalter für Grab und relativ (SDL-Stil) | 2.1 |
| Namen | `fplSetWindowMouseMode`, `fplSetWindowKeyboardGrabEnabled`, `fplSetWindowCursorPosition` | `…MouseGrab`, `fplWarpWindowCursor` | 2.1 |
| Grab bei Fokusverlust | aussetzen und wiederherstellen (Anforderung bleibt) | Anforderung löschen (qemu-sdl2-Semantik), das kann die App selbst tun | 2.2 |
| Tasten bei Fokusverlust loslassen | ja, für alle Apps (Release-Events) | nur, solange ein Grab angefordert ist | 2.2, Iteration 1 |
| Relative Deltas | roh und unbeschleunigt, beschleunigt nur in der X11-Rückfallebene | zusätzlicher Modus „relativ beschleunigt“ | 2.4 |
| Win32-Hook | Fensterthread, SDL-Tastenmenge inklusive Modifier | eigener Hook-Thread, der alle Tasten abfängt (keine Text-Events während des Grabs) | 2.5, 8 |
| Alt+F4 beim Tastatur-Grab | geht an die App (wie qemu/SDL) | schließt weiterhin | 2.5 |
| Iteration 5 in diesem Plan | ja, sonst ist das Frontend unter Win32 nicht benutzbar (VK statt physischer Taste) | eigener Folgeplan | 2.8 |
| Format von `scanCode` | PC Satz 1 (`0xE0xx`), passt zu Win32 und PC-Emulatoren (86Box, 8086sim), qemu hat `atset1_to_linux` | Linux-evdev (X11 direkt) oder USB-HID (SDL-Stil) | 2.8 |
| Cursorbild | eigener Folgeplan | in Iteration 5 | 7.2 |

### 7.2 Folgepunkte (nicht in diesem Plan)

- **Cursorbild aus RGBA mit Hotspot** (`fplSetWindowCursorImage`) für `dpy_cursor_define`. qemu ist beim Kanalformat selbst uneinheitlich (`ui/cursor.c` baut `0xAABBGGRR`, sdl2 liest mit ARGB-Masken). Die FPL-Funktion bekommt deshalb ein ausdrückliches Pixelformat mit geradem Alpha. X11 braucht libXcursor (optional) mit 1-Bit-Rückfall, Win32 `CreateIconIndirect` mit 32-Bit-DIB.
- **qemu-Frontend selbst** (`ui/fpl.c`, meson, `DisplayChangeListenerOps`, Grab-Logik nach sdl2 oder gtk, Pumpen in `dpy_refresh`). Dafür wird vorher geprüft: **Win32-Fibers gegen qemu-Coroutinen.** FPL ruft beim Fensteraufbau `ConvertThreadToFiber` (`:19034`), qemus `coroutine-win32.c` macht das im Hauptthread auch und behandelt „schon ein Fiber“ vermutlich nicht. FPL sollte `IsThreadAFiber()`/`GetCurrentFiber()` benutzen und nur zurückwandeln, was es selbst gewandelt hat. Ob das allein reicht, hängt von der Init-Reihenfolge in qemu ab.
- **Deltas in `fplMouseState`** (Polling), zusammen mit den nie geschriebenen `wheelDeltaX/Y`.
- **Tastenzustand bei Fokusgewinn abgleichen** (X11 `XQueryKeymap`, Win32 `GetKeyboardState`) wie SDL, falls das Loslassen bei Fokusverlust nicht reicht.
- **RDP-Sonderfälle** im relativen Modus (SDL zentriert bei `SM_REMOTESESSION` nach und verwirft große Sprünge).
- **Mäuse mit hoher Abtastrate** (4–8 kHz): `GetRawInputBuffer` statt einer `WM_INPUT` pro Paket. SDL3 liest Raw Input dafür in einem eigenen Thread.
- **Wayland nativ:** FPL hat kein Wayland-Backend. Unter XWayland greift der Pointer nur über die Pointer-Constraints des Compositors, und `XGrabKeyboard` blockiert Compositor-Kürzel wohl nicht (ungeprüft).
- **Mehrere Fenster** (qemu Multi-Head): FPL kennt nur ein Fenster.

---

## 8. Risiken

- **Tests greifen den echten Desktop.** Ein hängender (nicht toter) Client hält Tastatur und Maus fest. Abhilfe: `timeout -s KILL` in jedem Testlauf (ein toter Client verliert alle Grabs), `--timeout` in der Demo, Ansage vor jedem Lauf. Notausgang von Hand: Konsole mit Strg+Alt+F2, dort `pkill FPL_InputGrab`.
- **Debugger-Falle:** Hält ein Breakpoint die App an, während sie die Tastatur greift, ist unter X11 die Tastatur für den ganzen Desktop weg (Maus-Grab ebenso). Abhilfe: `--no-grab` in der Demo, ein Hinweis in der Doku, Notausgang wie oben. Unter Win32 friert der Hook alle Tastatureingaben bis zum Timeout ein.
- **Win32-Hook-Timeout:** Pumpt die App seltener als `LowLevelHooksTimeout` (ab Windows 10 1709 höchstens 1 s, der tatsächliche Standard wird in Iteration 4 nachgesehen), verzögert sich jede Taste systemweit, und Windows entfernt den Hook stillschweigend. Abhilfe: Doku („mindestens alle 100 ms pumpen“, qemu pumpt alle 30 ms), Neuinstallation nach 2.5. Ein eigener Hook-Thread würde das lösen, bringt aber Reihenfolgeprobleme gegenüber der normalen Eingabe, weil gepostete Nachrichten vor Eingabenachrichten gelesen werden.
- **Reihenfolge Hook gegen Normalweg:** Tasten aus dem Hook und Tasten über `WM_KEYDOWN` können sich vertauschen, wenn mehrere zwischen zwei Pump-Durchgängen liegen. Das kann passieren, wenn die abgefangene Taste nach einer durchgereichten kommt. Die SDL-Menge hält Modifier und ihre Kombitasten (Tab, Esc) auf demselben Weg, übrig bleibt der seltene Fall „Buchstabe, dann Modifier loslassen“ innerhalb eines Pump-Intervalls.
- **XTEST und Rohdaten:** Falls XTEST-Bewegungen keine `XI_RawMotion` erzeugen, ist der XI2-Pfad nicht automatisch testbar, dann gilt die Handprüfung.
- **Fenstermanager:** Die Abfolge von Fokus-Events rund um eigene Grabs (`NotifyGrab`, `NotifyWhileGrabbed`) wird unter KWin gemessen, andere Fenstermanager können abweichen. SDL enthält dafür Sonderwege (etwa den Mutter-Workaround bei `LeaveNotify`), die nur bei Bedarf übernommen werden.
- **ABI:** `fplMouseEvent` wächst um 8 Byte (Iteration 5: plus `wheelDeltaX`), `fplKeyboardEvent` um 4 Byte. `fplEvent` ist eine Union mit dem größeren `fplGamepadEvent` und sollte gleich groß bleiben. `TestSizes` prüft das.
- **Wine ist nicht Windows:** Ein grüner wine-Test ersetzt die Abnahme auf echtem Windows nicht, besonders beim Hook und bei `ClipCursor`.
