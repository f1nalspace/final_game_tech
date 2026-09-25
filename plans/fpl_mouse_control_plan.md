# Plan: FPL — Tastatur-Grab, Maus-Grab, Warp und relativer Mausmodus

Ziel: FPL bekommt alles, was ein **qemu-Frontend** (wie `ui/sdl2.c` oder `ui/gtk.c`) von der Fenster- und Eingabeschicht braucht, um Tastatur und Maus an den Gast zu übergeben. Dazu gehören **Tastatur-Grab** (Alt+Tab, Super/Win, Alt+Esc und Strg+Esc landen im Fenster), **Maus einsperren**, **Mauszeiger setzen** (Warp) und ein **relativer Mausmodus** mit rohen, unbeschleunigten Deltas. Unterstützt werden **X11 und Win32** gleichwertig, hinter einer kleinen API, die ohne Plattformwissen benutzbar ist.

Dieses Dokument beginnt mit dem **Handover** für eine frische Session (0). Danach beschreibt es den **Stand** (1), die **Designentscheidungen** samt API (2), den **inneren Aufbau** (3), den **Prüfstand** (4) und die **Iterationen** mit Abnahmekriterien (5). Danach folgen Arbeitsregeln, Entscheidungen und Folgepunkte sowie Risiken (6–8).

Quellen der Anforderungen: qemu `master` und v10.1.0 (`ui/sdl2.c`, `ui/gtk.c`, `ui/input.c`, `ui/kbd-state.c`, `ui/win32-kbd-hook.c`, `include/ui/console.h`) sowie SDL2 (`src/video/x11/*`, `src/video/windows/*`), im Quelltext geprüft am 2026-09-24.

---

## 0. Handover (Stand 2026-09-25, nach Iteration 2)

Dieser Abschnitt reicht, um in einer frischen Session weiterzumachen. Der Rest des Plans ist die Begründung dazu.

### 0.1 Wo wir stehen

- **Branch `fpl/mouse-control`**, basiert auf `develop` (`edc19303`). Er war zuerst versehentlich vom Viewer-Branch abgezweigt und wurde vor dem ersten Commit auf `develop` gesetzt. Am 2026-09-25 wurde `develop` (`cc48021c`) hineingemergt (`47d32223`).
- **Commits auf dem Branch:** `3bb6aa02` Plan angelegt, `1231c892` Plan (Nutzer), `68eefab4` Demo + Testskript, `b5d674bb` Plan Iteration 0 + Entscheidungen, `199699f6` **FPL: Cursor warp, move deltas and key release on focus loss**, `d4f56021` Demo: Deltas, Grab-Tastenkürzel, Warp-Selbsttest, `204a55df` Plan Iteration 1, `0e614fdf` Plan Handover, `47d32223` Merge `develop`, `6eab900d` **FPL: Mouse grab for X11 and Win32**, `0c69ca45` Demo: Maus-Grab-Tests, danach der Plan-Commit zu Iteration 2. Nicht gepusht (Stand 2026-09-25).
- **Fertig:** Iteration 0 (Demo `demos/FPL_InputGrab`, Testskript, Ausgangsstand), Iteration 1 (API-Gerüst, Zustandsmodell, `fplWarpWindowCursor`, `deltaX`/`deltaY`, Loslassen bei Fokusverlust) und Iteration 2 (Maus-Grab unter X11 und Win32, Wiederholung, Warp-Begrenzung). Die Details stehen in den „Stand“-Blöcken unter 5.
- **Die API ist entschieden** (7.1): `fplSetWindowMouseGrab`/`fplIsWindowMouseGrabbed`, `fplSetWindowRelativeMouse`/`fplIsWindowRelativeMouse`, `fplSetWindowKeyboardGrab`/`fplIsWindowKeyboardGrabbed`, `fplWarpWindowCursor`. Der Maus-Grab ist freigeschaltet, die Setter für den relativen Modus und den Tastatur-Grab lehnen `true` noch mit `false` und einer Warnung ab. Jede Iteration schaltet ihren Teil frei.
- **Als Nächstes: Iteration 3 (relativer Modus),** die Schrittliste steht in 0.6.
- **Entschieden am 2026-09-24 (7.1):** Der Win32-Fehler „Events vom Fensteraufbau gehen verloren“ (1.3) wird **in einem eigenen Branch** behoben, etwa `fpl/win32-init-events` von `develop`. Der wird nach `develop` gemergt, danach `develop` in `fpl/mouse-control`. Gearbeitet wird dort in einem eigenen `git worktree`, nicht in diesem Arbeitsbaum. Der Branch existiert noch nicht. Die **Abnahme auf echtem Windows wird gesammelt in Iteration 6** gemacht, bis dahin gelten MinGW und wine als Win32-Prüfung.

### 0.2 Bauen

- Demo (CMake ist maßgeblich), die Ausgabe landet in `demos/build/FPL_InputGrab/Linux-x64-Release/FPL_InputGrab`:
  `cmake -S demos/FPL_InputGrab -B demos/FPL_InputGrab/build -G Ninja -DCMAKE_BUILD_TYPE=Release && ninja -C demos/FPL_InputGrab/build`
- **Build-Matrix pro Iteration** (6): `FPL_InputGrab`, `FPL_Window`, `FPL_Input`, `FPL_Test`, `FPL_NoRuntimeLinking`, `FPL_NoPlatformIncludes`, `FPL_ImGui`, je mit gcc/g++ und clang/clang++, jeweils `cmake -S demos/<Demo> -B <scratch>/<Demo>-<cc> -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=<cc> -DCMAKE_CXX_COMPILER=<cxx>` und `ninja`. Die Build-Verzeichnisse gehören ins Scratchpad, nicht ins Repo.
- **MinGW** (Win32-Build ohne Windows), aus `demos/`: `x86_64-w64-mingw32-gcc -std=c99 -I.. FPL_InputGrab/fpl_inputgrab.c -o <scratch>/FPL_InputGrab_x64.exe`, genauso mit `i686-w64-mingw32-gcc`, `FPL_Window` (C99) und `FPL_Input` (mit `x86_64-w64-mingw32-g++ -std=c++11 -Iadditions -Idependencies`). `FPL_Test` unter MinGW nur mit `-O2` und `-DFPL_LOGGING -DFT_IMPLEMENTATION`, ohne Optimierung linkt es schon auf `develop` nicht.
- **Neue Warnungen finden:** denselben Build einmal mit dem Header von `develop` (`git show develop:final_platform_layer.h > <scratch>/oldheader/final_platform_layer.h`, dann `-I<scratch>/oldheader` vor `-I..`) und einmal mit dem aktuellen, dann die Warnungen ohne Zeilennummern vergleichen. Der Header erzeugt mit `-Wall -Wextra` rund 70 alte Warnungen, gezählt werden nur neue.

### 0.3 Testen

- **Vor jedem Lauf, der Fenster öffnet, den Nutzer fragen.** Jedes neue Demo-Fenster bekommt den Fokus, und was der Nutzer gerade tippt, landet darin (ist in Iteration 0 passiert). Ansage mit Dauer, dann erst starten, nie in Schleifen.
- `demos/FPL_InputGrab/tests/run_grab_tests.sh [--tests=a,b] [--list] [--demo=<pfad>]`: 15 Tests, alle grün, etwa 2 Minuten. Logs und `report.md` gehen nach `demos/build/FPL_InputGrab/tests/`. Jede Demo läuft unter `timeout -s KILL`, wird per Signal beendet, und nach jedem Test prüft python-xlib, ob Tasten im X-Server hängen (Kapitel 8, Iteration 0).
- Ein neuer Test ist eine Funktion `Test_<name>` im Skript plus ein Eintrag in `allTests`. Die Helfer sind `StartDemo` (setzt `demoPid`, `demoWindow`, `demoLog`), `ActivateDemo`, `StopDemo <pid>`, `ExpectLogLine <log> <mark> <regex>`, `LineNumberOf`, `LogLineCount`, `Fail`, `Note`, `RunSelfTest <parameter>`. Für den Zeiger (seit Iteration 2): `ReadClientArea <fenster>` (setzt `areaLeft/Top/Width/Height` in Bildschirmkoordinaten), `ReadPointer` (setzt `pointerX/Y`), `ExpectPointerConfined <x> <y>` und `ExpectPointerFree <x> <y>` (bewegen den Zeiger per `xdotool mousemove` wiederholt bis zum Ergebnis oder Zeitlimit), `StartPointerHolder <sekunden>` (ein python-xlib-Client hält einen aktiven Pointer-Grab, Log in `holderLog`).
- Die Demo loggt mit `--log-events` eine Zeile pro Event (`t=… mouse move x= y= dx= dy=`, `key button state= code= key= mods=`, `window gotfocus|lostfocus|…`, `grab mouse requested= result=`). Mit `--log-fpl` kommen FPLs eigene Log-Meldungen bis Stufe Verbose dazu (`fpl level=verbose [X11] XGrabPointer failed with AlreadyGrabbed, trying again (…)`), ohne den Schalter nur Warnungen und Fehler. `--selftest` prüft den Warp selbst, Exit-Code 1 bei Fehler, und läuft auch ohne Fokus-Event los. Mit `--mouse-grab` warpt der Selbsttest zusätzlich auf zwei Ziele außerhalb des Fensters, die am Rand des Client-Bereichs ankommen müssen.
- **wine** (Win32-Vortest), im Scratchpad: `WINEDEBUG=-all timeout -s KILL 40 wine ./FPL_InputGrab_x64.exe --selftest --log-events --timeout=15`, dasselbe mit `--mouse-grab --log-fpl`. Vorsicht seit Iteration 2: wine setzt `ClipCursor` vermutlich über einen echten X-Pointer-Grab um, der betrifft dann den Desktop des Nutzers.
- **Build-Matrix als Skript:** Die Matrix aus 0.2 lässt sich in einem Rutsch bauen, das Skript der Iteration-2-Session lag im Scratchpad (`build_matrix.sh`) und ist nicht im Repo. Es baut die sieben Demos mit gcc und clang per CMake und `FPL_InputGrab`, `FPL_Window`, `FPL_Input` (g++) und `FPL_Test` (`gcc -std=c99 -O2 -DFPL_LOGGING -DFT_IMPLEMENTATION`) mit MinGW x64 und x86.
- **`FPL_Test` taugt nicht als Abnahme,** es ist schon auf `develop` rot (1.3). Wer es laufen lässt, vergleicht mit dem `develop`-Header. Es öffnet ein Fenster (`fplInitFlags_All`).

### 0.4 Arbeitsweise

- Code-Stil nach `CLAUDE.md` (benannte Zwischenwerte statt Aufrufketten, keine magischen Zahlen, keine Umbrüche auf Spaltenbreite, Klammern um jeden Bedingungskörper, Kommentare englisch). Der Plan bleibt deutsch.
- **Commits nur auf Zuruf**, klein und nach Thema getrennt: FPL-Header+Doku, Demo+Tests, Plan. Die Nachrichten sind englisch und kurz, mit `Co-Authored-By`, **nie** mit einer `Claude-Session`-Zeile. Plan-Commits heißen `Update fpl_mouse_control_plan.md: Iteration N done`.
- Der **Changelog** kommt in den unveröffentlichten Abschnitt **v1.0.1** (Window), keine neue Versionsstufe. Die Doku steht in `final_platform_layer.docs` (Seite `page_category_window_style`, Abschnitte `section_category_window_style_cursor…`).
- **premake nicht komplett neu erzeugen:** Das würde 41 fremde Projektdateien ändern. Neue Projekte in einer Kopie erzeugen und nur deren Einträge übernehmen (Iteration 0, Stand).
- Nach jeder Iteration: Block „Stand“ unter der Iteration, neue Befunde in 1.3, Folgepunkte in 7.2, dann den Merkzettel `project_fpl_mouse_control` im Memory nachziehen.

### 0.5 Code-Landkarte (`final_platform_layer.h`, nach Namen suchen, die Zeilen wandern)

- **Öffentliche API:** hinter `fplQueryCursorPosition` in der Gruppe `WindowBase`. `fplMouseEvent.deltaX/deltaY`.
- **Zustand:** `fpl__MouseLockState` und `fpl__InputGrabState` direkt vor `fpl__PlatformWindowState` (Abschnitt PLATFORM_STATES), dort auch `inputGrab`, `hasFocus`, `isMinimized`. Kapitel 3 beschreibt den Zielzustand. Felder wie `frozenX`, `remainderX` und `nextRetryTimeMilliseconds` kommen erst mit ihrer Iteration dazu.
- **Gemeinsame Logik** (COMMON, Block `FPL__COMMON_WINDOW_DEFINED`, direkt nach `fpl__IsFullscreenChangeAllowed`): `fpl__ReleaseAllPressedButtons`, `fpl__GetRequestedMouseLock`, die Vorwärtsdeklaration `fpl__PlatformApplyMouseLock`, `fpl__UpdateInputGrab` (der einzige Ort, der entscheidet, ruft die Plattform nur bei gewünscht ≠ angewendet und setzt bei Fehlschlag `isRetryPending` + `nextRetryTimeMilliseconds`), `fpl__RetryInputGrab` (beim Pumpen, `FPL__GRAB_RETRY_INTERVAL_MILLISECONDS` = 50), `fpl__LimitWarpToConfinedArea`, `fpl__HandleWindowFocusChanged`, `fpl__HandleWindowMinimizedChanged` und die sechs Setter/Getter (`fpl_common_api`). `isMouseLockSuspended` in `fpl__PlatformWindowState` setzt die Plattform (heute nur Win32), es wirkt nur auf die Maus. `fpl__ReleaseWindow` hebt den Grab vor dem Zerstören des Fensters auf.
- **Deltas:** `fpl__HandleMouseMoveEvent` rechnet, `fpl__PushMouseMoveEvent` trägt ein.
- **Win32:** `fpl__Win32WindowGotFocus/LostFocus` → `fpl__HandleWindowFocusChanged`, davor wird der Aktivierungsklick erkannt. Direkt vor `fpl__Win32MessageFiberProc` steht der Maus-Grab: `fpl__Win32GetClientScreenRect`, `fpl__Win32ReleaseClipCursor`, `fpl__Win32ClipCursorToClientArea`, `fpl__PlatformApplyMouseLock` (Win32), `fpl__Win32IsMouseLockSuspended`/`fpl__Win32UpdateMouseLockSuspension`, `fpl__Win32RefreshInputGrab` (beim Pumpen: Aktivierungsklick beenden, Clip alle `FPL__WIN32_CLIP_REFRESH_INTERVAL_MILLISECONDS` = 3 s neu setzen, Wiederholung). In `fpl__Win32MessageProc`: `WM_SIZE` (Minimieren), `WM_CLOSE/WM_DESTROY`, `WM_MOUSELEAVE`, `TrackMouseEvent` im Mausnachrichten-Block, `WM_ENTERSIZEMOVE/WM_ENTERMENULOOP` und die `EXIT`-Gegenstücke zählen `modalLoopDepth`, `WM_WINDOWPOSCHANGED` setzt den Clip neu. Die Win32-Version von `fplWarpWindowCursor` steht hinter `fplSetWindowCursorEnabled`. Der User32-Loader ist `fpl__Win32LoadApi`, mit Typedef `FPL__FUNC_WIN32_…`, Feld in `fpl__Win32Api.user` und `FPL__WIN32_GET_FUNCTION_ADDRESS`. `ClipCursor` und `GetClipCursor` sind geladen, `GetRawInputData` noch nicht.
- **X11:** `fpl__X11HandleEvent` (`FocusIn/FocusOut` → `fpl__HandleWindowFocusChanged`, `EnterNotify` setzt die Delta-Basis zurück, `PropertyNotify` → `fpl__HandleWindowMinimizedChanged`, `ClientMessage`/`wmDeleteWindow`). Die X11-Version von `fplWarpWindowCursor` steht hinter `fplSetWindowCursorEnabled`, direkt dahinter `FPL__X11_POINTER_GRAB_EVENT_MASK`, `fpl__X11GetGrabResultName` und `fpl__PlatformApplyMouseLock` (X11). `fplPollEvent` (wenn beide Queues leer sind), `fplPollEvents` und `fplWindowUpdate` rufen `fpl__RetryInputGrab`. Eine neue X11-Funktion braucht **vier Stellen**, Vorlage ist `XWarpPointer`: `FPL__FUNC_X11_…` + Typedef, `extern` im `FPL_NO_RUNTIME_LINKING`-Block, Feld in `fpl__X11Api`, `FPL__POSIX_GET_FUNCTION_ADDRESS` in `fpl__LoadX11Api`. Konstanten kommen **zweimal** in die X11-ABI: einmal als Weiterleitung (`#define FPL__X11_X X`) und einmal als Zahl für die Builds ohne X11-Header.
- **Nicht betroffen, aber wichtig:** Win32 pumpt nur in `fplWindowUpdate`/`fplPollEvent` (Message-Fiber), `fplWindowUpdate` leert die interne Queue.

### 0.6 Nächster Schritt: Iteration 3 (relativer Modus) als Schrittliste

1. **Zustand:** `frozenX`/`frozenY` und `remainderX`/`remainderY` (double) in `fpl__InputGrabState` (Kapitel 3). Win32: `rawMouseRegistered`, letzte absolute Rohposition. X11: XI2-Opcode, verfügbar, ausgewählt, Tabelle absoluter Quellgeräte samt Vorwerten, Warp-Mitte der Rückfallebene.
2. **Gemeinsam:** `fplSetWindowRelativeMouse(true)` freischalten. Beim Übergang nach `Relative` die Position einfrieren (letzte Move-Position, sonst `fplQueryCursorPosition` minus Client-Ursprung), beim Verlassen per Warp dorthin zurück (2.2). Solange relativ: `mouseX`/`mouseY` aller Maus-Events = eingefrorene Position, Deltas roh, Bruchteile im Rest sammeln. Der Cursor ist versteckt, danach gilt wieder `fplSetWindowCursorEnabled`. `fplWarpWindowCursor` versetzt im relativen Modus nur die eingefrorene Position (2.6). `fpl__PlatformApplyMouseLock` bekommt die Übergänge Free↔Relative und Confined↔Relative.
3. **Win32:** `GetRawInputData` (und `GetMessageExtraInfo`) in den User32-Loader. Einschalten: `RegisterRawInputDevices({0x01, 0x02, 0, hwnd})`, `ClipCursor` auf 1×1 in der Client-Mitte (über `fpl__Win32ClipCursorToClientArea` verallgemeinern, sonst schlägt die Eigentumsprüfung in `fpl__Win32ReleaseClipCursor` fehl). `case WM_INPUT` → Stack-Puffer, `MOUSE_MOVE_RELATIVE` direkt, `MOUSE_MOVE_ABSOLUTE` von 0..65535 (benannte Konstante) auf den (virtuellen) Desktop umrechnen und gegen den Vorwert rechnen, das erste Paket setzt nur die Basis, Touch-Pakete (`GetMessageExtraInfo() & 0x80`, benannte Konstante) überspringen, danach `DefWindowProc`. `WM_MOUSEMOVE` im relativen Modus ignorieren. Ausschalten: `RIDEV_REMOVE`. Das Raw-Input-Überbleibsel in `fpl__Win32SetCursorState` fällt weg (1.3). Aussetzen im modalen Loop gilt schon über `isMouseLockSuspended`.
4. **X11-ABI und Loader:** aus libX11 `XQueryExtension`, `XGetEventData`, `XFreeEventData` (vier Stellen, 0.5). libXi **optional** wie Xinerama (`fpl__LoadXineramaApi` als Vorlage, `libXi.so.6`, `libXi.so`): `XIQueryVersion`, `XISelectEvents`, `XIQueryDevice`, `XIFreeDeviceInfo`. Für `FPL_NO_PLATFORM_INCLUDES` Typen `XGenericEventCookie`, `XIEventMask`, `XIRawEvent`, `XIValuatorState`, `XIDeviceInfo`/`XIValuatorClassInfo` und Konstanten `XI_RawMotion` (17), `XIAllMasterDevices` (1), `XI_RawMotionMask`, `XIMaskLen`, `XIValuatorClass`, `XIModeAbsolute` selbst definieren. Neues Define `FPL_NO_X11_XINPUT2` erzwingt die Rückfallebene, `FPL_NoRuntimeLinking` wird mit und ohne gebaut (ohne braucht es `-lXi` in der CMake-Datei).
5. **X11 anwenden:** `XISelectEvents(root, XIAllMasterDevices, XI_RawMotion)` nur solange relativ, dazu `XGrabPointer` mit `confine_to=window` und dem unsichtbaren Cursor (`cursor.invisibleCursor`, ggf. erst anlegen), dann Warp in die Mitte. Event-Schleife: `GenericEvent` → `XGetEventData` → Opcode und `XI_RawMotion` → `raw_values` der Valuatoren 0/1 nach gesetzten Maskenbits → Delta (`cur - prev` bei absoluten Geräten) → `XFreeEventData`. Core-`MotionNotify` im relativen Modus ignorieren. Ausschalten: leere Maske, `XUngrabPointer` oder zurück auf Confined, Warp auf die eingefrorene Position.
6. **X11-Rückfallebene:** Warp zur Mitte, `MotionNotify` genau auf der Mitte ist das Echo und wird verworfen, sonst `delta = position - mitte` und erneuter Warp. Beschleunigt, das kommt in die Doku.
7. **Demo:** Fadenkreuz im relativen Modus an einer virtuellen Position (Deltasumme, am Rand gestoppt), `--log-core-motion` (4.1).
8. **Tests** (vorher fragen, sperren den Zeiger): zuerst prüfen, ob XTEST überhaupt `XI_RawMotion` erzeugt (python-xlib oder `xinput test-xi2 --root`). Dann `relative_mouse_delta_sum` (Folge `xdotool mousemove_relative` mit bekannter Summe = Summe von `dx`/`dy`), `relative_mouse_keeps_pointer` (Zeiger bleibt im Fenster), `relative_mouse_returns_to_start`, `relative_mouse_released_on_focus_loss`, dieselben mit einer Demo, die mit `FPL_NO_X11_XINPUT2` gebaut ist (zweites CMake-Ziel oder `--demo=`).
9. **Doku und Changelog:** Abschnitt „Relative mouse mode“, Changelog-Zeilen, Handprüfliste 4.4 (1600-DPI-Maus, qemu-Gast mit usb-tablet).
10. Build-Matrix, Tests (mit Freigabe), Abschnitt „Stand“ unter Iteration 3, dann fragen, ob committet werden soll.

### 0.7 Fallen, die schon Zeit gekostet haben

- `xdotool key --window W` geht bei einem fokussierten Fenster über XTEST. Beendet sich das Ziel beim Tastendruck, bleiben Tasten im X-Server gedrückt. Deshalb Demos nur per Signal beenden.
- FPL setzt unter X11 kein `WM_NAME`, `xdotool search --name` findet die Fenster nicht. Gesucht wird mit `--pid` (Kind-PID des `timeout`-Wrappers über `pgrep -P`).
- KWin kann eine Fenster-ID wiederverwenden (X-Fehler bei schnellen Neustarts). Das Skript wiederholt einen Test mit X-Fehler bis zu dreimal.
- Win32 verliert die Events vom Fensteraufbau (1.3), unter wine kommt deshalb kein erstes `gotfocus`. Tests dürfen sich unter Win32 nicht auf das erste `gotfocus` verlassen.
- Ein Test, der schneller fertig ist, als das Skript eine Aktivierung bestätigen kann, darf die Aktivierung nicht prüfen (warp_selftest).
- clangd zeigt im Editor Fehler für `final_platform_layer.h` und die Demo (`Unknown type name 'uint32_t'`, `-fdiagnostics-plain-output`). Das liegt an der `compile_commands.json` mit gcc-Flags im Build-Verzeichnis und ist kein echter Fehler, maßgeblich ist der Build.
- gcc meldet bei `-O3` seit Iteration 1 `-Wstringop-overflow` in `fplAtomicLoadU64` (inlined in `fpl__WaitForThreadsStopped`). Das ist ein Fehlalarm durch verschobenes Inlining, der Code ist unverändert (7.2).
- **Neue Warnungen gegen den Header von `HEAD` vergleichen, nicht gegen `develop`:** Die Demo benutzt die neue API und baut mit dem `develop`-Header nicht, der Vergleich zählt dann Fehler mit. Für `sort`/`comm` `LC_ALL=C` setzen, sonst meldet `comm` unsortierte Eingaben.
- **Bewegt jemand während eines Testlaufs die echte Maus**, kann ein anderes Fenster den Fokus bekommen, und ein Test ohne jeden Bezug zum Grab wird rot (Iteration 2: `no_stuck_alt_after_alt_tab`, im Log fließende Mausbewegungen, danach ging der Fokus an ein drittes Fenster). Erst ins Log schauen, dann den einen Test einmal wiederholen.
- `ActivateDemo` wartet bis zu 5 s auf `gotfocus`, wenn das Fenster den Fokus schon hat (erst dann fragt es das aktive Fenster ab). Tests werden dadurch langsamer, nicht rot.
- KDE hat Aktionen auf Bildschirmecken (oben links: Übersicht). Zeigerziele in Tests deshalb nie auf Bildschirmrand oder -ecke legen, sondern relativ zum Client-Bereich (`outsideDistance`).

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
- **Tastenzustand nach Fokusverlust (in Iteration 0 bestätigt):** Wird eine Taste losgelassen, während das Fenster keinen Fokus hat, sieht FPL das `Release` nie. `fpl__HandleKeyboardButtonEvent` (`:14533`) meldet den nächsten Druck dieser Taste dann als `Repeat`. Nachgestellt mit Alt+Tab weg und zurück: Das Fenster sieht Alt gedrückt, KWin übernimmt mit Tab die Tastatur, das Loslassen von Alt geht an KWin, und der nächste echte Alt-Druck kommt als `state=repeat` an (Test `no_stuck_alt_after_alt_tab`). SDL gleicht das beim Fokusverlust aus (`SDL_ResetKeyboard`), qemu-gtk mit `qkbd_state_lift_all_keys`.
- **X11-Modifier** (`fpl__X11TranslateModifierFlags`, `:28149`): Ist Strg, Umschalt, Alt oder Super gedrückt, setzt FPL immer **beide** Flags (links und rechts), weil X11 im Zustandsfeld keine Seite kennt. `LockMask` und `Mod2Mask` werden gar nicht ausgewertet, `CapsLock` und `NumLock` fehlen unter X11 also in `modifiers`. Für qemu ist das egal, weil qemu den Tastenzustand selbst führt. Für eine eigene Modifier-Führung (2.5) muss man es aber wissen.
- **Win32 verliert die Events vom Fensteraufbau** (gefunden in Iteration 1 unter wine, aus dem Code belegt): `fpl__Win32InitWindow` ruft `ShowWindow`, `SetForegroundWindow` und `SetFocus` (`:19366`), die `WM_ACTIVATEAPP` und `WM_SIZE` synchron in die Fensterprozedur schicken. Die Events landen in der internen Queue, und das erste `fplWindowUpdate()` der App leert sie mit `fpl__ClearInternalEvents()`, bevor `fplPollEvent()` sie liest. Das anfängliche `GotFocus` (und vermutlich `Resized`/`Shown`) kommt unter Win32 nie an, unter X11 schon, weil `FocusIn` erst über die X-Queue kommt. Der interne Fokus-Zustand der Grabs ist davon nicht betroffen, er wird direkt in `fpl__HandleWindowFocusChanged` gesetzt. Ein qemu-Frontend, das den Fokus aus den Events führt, hält das Fenster aber anfangs für unfokussiert (7.2).
- **`FPL_Test` ist auf `develop` rot** (gefunden in Iteration 1, mit dem Header von `develop` gegengeprüft): Mit `-O3` (CMake-Release) bricht es in den Security-Tests mit `double free or corruption` ab, mit `-O2` scheitert eine Prüfung in „Test Process handle and result lifetime“. Mit MinGW ohne Optimierung linkt es nicht (`ForceInlineTest`). Nichts davon hängt mit diesem Plan zusammen (7.2).
- **X11 setzt kein `WM_NAME`** (gefunden in Iteration 0): Der Fenstertitel steht nur in `_NET_WM_NAME`, `WM_CLASS` wird einmal aus dem Starttitel gesetzt. Werkzeuge, die nach ICCCM `WM_NAME` lesen, finden das Fenster nicht über den Titel, darunter `xdotool search --name`. Das Testskript sucht deshalb über die Prozess-ID. Eine kleine FPL-Korrektur, die nicht zu diesem Plan gehört (7.2).
- **Win32 `eventCallback` bekommt `MSG**` statt `MSG*`** (gefunden in Iteration 2, im Code belegt): `fpl__Win32HandleMessage` übergibt `&msg`, wobei `msg` schon ein `MSG *` ist. Die Doku von `fpl_window_event_callback` verspricht „MSG for Win32“, X11 übergibt korrekt das `XEvent *`. Wer unter Win32 den rohen Callback benutzt, liest Müll (7.2).
- **X11-Loader:** Weder `XGrabPointer`, `XUngrabPointer`, `XGrabKeyboard`, `XUngrabKeyboard` und `XWarpPointer` noch `XQueryExtension`, `XGetEventData` und `XFreeEventData` werden geladen, libXi gar nicht. (Stand Iteration 2: `XWarpPointer`, `XGrabPointer` und `XUngrabPointer` sind geladen.) Für `FPL_NO_PLATFORM_INCLUDES` müssen alle Konstanten (`GrabModeAsync`, `GrabSuccess`, `AlreadyGrabbed`, `GrabNotViewable`, `GrabFrozen`, `XI_RawMotion`, `XIAllMasterDevices`) und Typen (`XGenericEventCookie`, `XIEventMask`, `XIRawEvent`, `XIValuatorState`) selbst definiert werden, wie beim Rest der X11-ABI.

---

## 2. Designentscheidungen

### 2.1 Die API

**Entschieden am 2026-09-24: getrennte Schalter wie bei SDL.** Drei Schalter und ein Warp, jeder Schalter mit Getter:

```c
//! Keeps the cursor inside the client area of the window.
fpl_platform_api bool fplSetWindowMouseGrab(const bool enabled);
fpl_platform_api bool fplIsWindowMouseGrabbed(void);

//! Hides and locks the cursor, move events carry raw unaccelerated deltas. Locks the cursor regardless of the mouse grab.
fpl_platform_api bool fplSetWindowRelativeMouse(const bool enabled);
fpl_platform_api bool fplIsWindowRelativeMouse(void);

//! System shortcuts (Alt+Tab, Super/Win, Alt+Esc, Ctrl+Esc, Alt+F4) go to the window instead of the window manager or the shell.
fpl_platform_api bool fplSetWindowKeyboardGrab(const bool enabled);
fpl_platform_api bool fplIsWindowKeyboardGrabbed(void);

//! Moves the cursor to a position in window coordinates, the same coordinates the mouse events use.
fpl_platform_api bool fplWarpWindowCursor(const int32_t x, const int32_t y);
```

Dazu bekommt `fplMouseEvent` zwei Felder:

```c
	//! Movement since the previous move event in pixels, or the raw device counts in relative mode.
	int32_t deltaX;
	//! Movement since the previous move event in pixels, or the raw device counts in relative mode.
	int32_t deltaY;
```

**Zusammenspiel der beiden Maus-Schalter:** Der relative Modus gewinnt. Solange er an ist, ist der Cursor versteckt und festgehalten, egal was der Maus-Grab sagt. Wird er ausgeschaltet, gilt wieder der Maus-Grab, der Cursor ist also eingesperrt oder frei. Intern wird aus beiden Schaltern genau ein wirksamer Zustand (frei, eingesperrt, relativ), und nur diesen kennen die Plattformteile (3). So gibt es trotz zweier Schalter keine ungültige Kombination.

Warum diese Form:
- **Getrennte Schalter wie SDL** (`SDL_SetWindowMouseGrab`, `SDL_SetWindowRelativeMouseMode`, `SDL_SetWindowKeyboardGrab`, `SDL_WarpMouseInWindow`): Wer SDL kennt, findet sich sofort zurecht. Ein Frontend kann den relativen Modus ein- und ausschalten, ohne sich den Grab-Zustand zu merken.
- **Tastatur-Grab getrennt**, weil er unabhängig ist: qemu im absoluten Modus (usb-tablet) greift nur die Tastatur, ein Spiel im relativen Modus meistens nur die Maus.
- **Warp in Fensterkoordinaten**, weil qemu (`dpy_mouse_set`) und die Warp-Rückfallebene genau die brauchen. `fplQueryCursorPosition` bleibt bei Bildschirmkoordinaten.
- **Deltas in jedem Move-Event**, nicht nur im relativen Modus. Wer nur `mouseX`/`mouseY` liest, merkt nichts.
- **Die Getter liefern den angeforderten Zustand**, nicht den wirksamen (2.2).

So sieht ein qemu-Frontend damit aus:

```c
// Grab after a click or Ctrl+Alt+G, the guest has a relative mouse (PS/2)
fplSetWindowKeyboardGrab(true);
fplSetWindowRelativeMouse(true);

// Inside the event loop
if (ev.mouse.type == fplMouseEventType_Move && isGrabbed) {
	qemu_input_queue_rel(console, INPUT_AXIS_X, ev.mouse.deltaX);
	qemu_input_queue_rel(console, INPUT_AXIS_Y, ev.mouse.deltaY);
	qemu_input_event_sync();
}

// Ungrab
fplSetWindowRelativeMouse(false);
fplSetWindowKeyboardGrab(false);
```

### 2.2 Zustandsmodell: angefordert und wirksam

- FPL speichert den **angeforderten** Zustand, und die Getter liefern ihn. **Wirksam** ist er nur, solange das Fenster den Fokus hat, sichtbar und nicht minimiert ist. Das ist das Modell von SDL (`SDL_UpdateWindowGrab`).
- **Fokusverlust, Minimieren, Verstecken, der modale Verschiebe-/Größen-Loop unter Win32 und `fplWindowShutdown()`** heben den Grab beim Betriebssystem auf, die Anforderung bleibt. Kommt der Fokus zurück, stellt FPL den Grab wieder her. Wer die qemu-SDL-Semantik will (Fokusverlust beendet den Grab), schaltet bei `LostFocus` selbst alle drei Schalter aus.
- Eine **zentrale Funktion** `fpl__UpdateInputGrab()` gleicht angefordert und wirksam ab. Aufgerufen wird sie von den Settern, bei Fokus, Minimieren/Wiederherstellen, Zeigen/Verstecken, Vollbild, Größe/Position (Win32-Clip-Rechteck) und beim Beenden. Eine zweite Stelle, die Grabs anfasst, gibt es nicht.
- **Vorübergehende Fehlschläge werden wiederholt:** Direkt nach Alt+Tab hält KWin die Tastatur oft noch selbst, `XGrabKeyboard` liefert dann `AlreadyGrabbed`. SDL wiederholt blockierend 100 × 50 ms. FPL wiederholt stattdessen **nicht blockierend** in `fplWindowUpdate()`/`fplPollEvent()`, höchstens alle `grabRetryIntervalMilliseconds` (50 ms), solange angefordert und fokussiert.
- **Rückgabewert der Setter:** `false` nur, wenn der Wunsch grundsätzlich nicht erfüllbar ist (kein Fenster, ungültiger Wert, Plattform ohne Umsetzung). Ein vorübergehender Fehlschlag gibt `true` zurück und wird wiederholt.
- **Fokusverlust lässt alle gedrückten Tasten und Maustasten los** (Release-Events, wie SDL), und zwar **für alle Apps**, nicht nur mit Grab (entschieden am 2026-09-24). Sonst bleiben nach einem aufgehobenen Grab Strg oder Alt im Gast hängen, und nach Alt+Tab kommt die nächste Alt-Taste als `Repeat` an (Iteration 0).
- **Cursor-Sichtbarkeit:** Der relative Modus versteckt den Cursor, egal was `fplSetWindowCursorEnabled` sagt. Beim Verlassen gilt wieder dessen Wert. Der Maus-Grab lässt die Sichtbarkeit, wie sie ist.
- **Verlassen des relativen Modus:** Der Cursor erscheint an der Stelle, an der der Modus begonnen hat (eingefrorene Position). So macht es qemu-gtk (`gd_ungrab_pointer`), und so erwarten es Spiele.

### 2.3 Maus einsperren (Maus-Grab)

**X11:** `XGrabPointer(display, window, owner_events=True, ButtonPress|ButtonRelease|PointerMotion, GrabModeAsync, GrabModeAsync, confine_to=window, cursor=None, CurrentTime)`. `GrabSuccess` heißt wirksam. `AlreadyGrabbed`, `GrabFrozen` und `GrabInvalidTime` führen zur Wiederholung (2.2). `GrabNotViewable` (Fenster nicht abgebildet) wird bei `Shown`/`Restored` erneut versucht. Ändert sich die Fenstergeometrie, hält der X-Server den Zeiger von selbst im Fenster. Aufheben mit `XUngrabPointer` + `XFlush`.

**Win32:** `ClipCursor` auf das Client-Rechteck in Bildschirmkoordinaten. Wie bei SDL wird nur gesetzt, wenn sich das Rechteck von `GetClipCursor` unterscheidet, und `ClipCursor(NULL)` nur gerufen, wenn das aktuelle Rechteck das eigene ist. Neu gesetzt wird bei Fokus, `WM_WINDOWPOSCHANGED`, `WM_EXITSIZEMOVE`, Vollbildwechsel und zusätzlich alle `clipRefreshIntervalMilliseconds` (3 s, wie SDL) beim Pumpen, weil andere Programme das Rechteck zurücksetzen können. Während eines Klicks auf die Titelleiste und im modalen Verschiebe-Loop wird nicht eingesperrt.

### 2.4 Relativer Modus

Gemeinsam: Der Cursor ist versteckt und festgehalten. Move-Events tragen in `deltaX`/`deltaY` die **rohen Gerätezählwerte ohne Beschleunigung**. `mouseX`/`mouseY` bleiben auf der eingefrorenen Position, ebenso bei Tasten- und Rad-Events. Bruchteile (XI2 liefert Festkommawerte, absolute Geräte werden umgerechnet) sammelt FPL in einem Rest, damit nichts verloren geht. Jedes Rohpaket wird ein eigenes Move-Event, zusammengefasst wird nicht.

**Win32:**
- Einschalten: Position merken (`GetCursorPos`), Cursor verstecken, `RegisterRawInputDevices({0x01, 0x02, dwFlags=0, hwndTarget=window})`. Ohne `RIDEV_NOLEGACY` (Tasten-Nachrichten, `SetCapture` und die Titelleiste funktionieren weiter) und ohne `RIDEV_INPUTSINK` (nur im Vordergrund). `ClipCursor` auf ein 1×1-Rechteck in der Client-Mitte, damit Klicks im Fenster landen und der unsichtbare Cursor nirgends hin kann.
- `WM_INPUT`: `GetRawInputData` in einen Stack-Puffer. `MOUSE_MOVE_RELATIVE` liefert `lLastX`/`lLastY` direkt. **`MOUSE_MOVE_ABSOLUTE`** (RDP, VM-Tablets wie qemu usb-tablet oder VMware, Stifte) wird von 0..65535 auf den (virtuellen) Desktop umgerechnet (`MOUSE_VIRTUAL_DESKTOP`) und gegen die vorige Position gerechnet. Das erste Paket setzt nur die Basis. Aus Touch erzeugte Eingaben (`GetMessageExtraInfo() & 0x80`) werden übersprungen. `WM_INPUT` geht danach weiter an `DefWindowProc`, wie verlangt.
- `WM_MOUSEMOVE` wird im relativen Modus ignoriert, Tasten und Rad kommen weiter über die normalen Nachrichten, mit der eingefrorenen Position.
- Ausschalten: `RIDEV_REMOVE`, Clip-Rechteck zurück (frei oder eingesperrt, je nach Maus-Grab), `SetCursorPos` auf die eingefrorene Position, Sichtbarkeit wiederherstellen.

**X11, bevorzugt XInput2:**
- Beim Fensteraufbau wird libXi **optional** geladen (`libXi.so.6`, `libXi.so`, wie Xinerama). Danach folgen `XQueryExtension("XInputExtension")` für den Opcode und `XIQueryVersion` ≥ 2.0. Fehlt etwas, gilt die Rückfallebene.
- Einschalten: `XISelectEvents(root, XIAllMasterDevices, XI_RawMotion)`, und zwar **nur während der relative Modus wirksam ist**. SDL wählt es dauerhaft aus, das erzeugt ohne Nutzen Verkehr für jede Mausbewegung auf dem ganzen Bildschirm. Dazu `XGrabPointer` mit `confine_to=window` und dem unsichtbaren Cursor, dann ein Warp in die Fenstermitte.
- Event-Schleife: `GenericEvent` → `XGetEventData` → Opcode und `XI_RawMotion` prüfen → `raw_values` der Valuatoren 0 und 1 (Reihenfolge nach gesetzten Maskenbits) → Delta. **Absolute Valuatoren** (VM-Tablets, Grafiktabletts) werden pro `sourceid` über `XIQueryDevice` erkannt und gegen den Vorwert gerechnet. SDL rechnet dort `prev - cur`, das Vorzeichen sieht verdreht aus, FPL rechnet `cur - prev`. Danach `XFreeEventData`. Core-`MotionNotify` wird im relativen Modus ignoriert.
- Ausschalten: leere Maske, `XUngrabPointer` (oder weiter eingesperrt, wenn der Maus-Grab an ist), `XWarpPointer` auf die eingefrorene Position.

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

`fplWarpWindowCursor(x, y)` nimmt Fensterkoordinaten, der Ursprung liegt oben links wie bei den Maus-Events. Unter Win32 folgen `ClientToScreen` und `SetCursorPos`, unter X11 `XWarpPointer(display, None, window, 0, 0, 0, 0, x, y)` und `XFlush`. Das System schickt danach selbst eine Bewegung. FPL setzt seine „letzte Position“ vorher auf das Ziel, sodass dieses Move-Event das Delta 0 hat. Die SDL-Maßnahmen gegen Win32-Zittern (dreifaches `SetCursorPos`, verspätete `WM_MOUSEMOVE` verwerfen) werden nur übernommen, wenn der Test das Problem zeigt. Mit Maus-Grab wird das Ziel auf den Client-Bereich begrenzt. **Im relativen Modus wird nicht wirklich gewarpt**, es wird nur die eingefrorene Position versetzt, an der der Cursor beim Verlassen wieder erscheint. qemu warpt so bei `dpy_mouse_set`. Gibt es kein Fenster oder ist es versteckt oder minimiert, liefert die Funktion `false`.

### 2.7 Deltas in Move-Events

- Ohne relativen Modus (frei oder eingesperrt): Differenz zum vorigen Move-Event des Fensters. Nach Enter, Fokusgewinn und Warp ist das Delta 0.
- Im relativen Modus: rohe Zählwerte (2.4).
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
// The one mouse state the platform parts know, made from the mouse grab and the relative mouse switch
typedef enum fpl__MouseLockState {
	fpl__MouseLockState_Free = 0,
	fpl__MouseLockState_Confined,
	fpl__MouseLockState_Relative,
} fpl__MouseLockState;

typedef struct fpl__InputGrabState {
	// Requested by the user, kept while the window has no focus
	fpl_b32 requestedMouseGrab;
	fpl_b32 requestedRelativeMouse;
	fpl_b32 requestedKeyboardGrab;
	// What is currently applied at the operating system
	fpl__MouseLockState appliedMouseLock;
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
- `fpl__UpdateInputGrab(appState)` ist der einzige Ort, der wirksame Grabs setzt oder aufhebt. Er vergleicht angefordert mit wirksam unter der Bedingung „fokussiert, sichtbar, nicht minimiert“ und ruft die Plattformfunktionen `fpl__Win32ApplyMouseLock`/`fpl__X11ApplyMouseLock` und `…ApplyKeyboardGrab`. Den gewünschten `fpl__MouseLockState` rechnet eine kleine plattformneutrale Funktion aus den beiden Maus-Schaltern aus (relativ vor Grab).
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
- Tasten nach qemu-Art: Strg+Alt+G schaltet den vollen Grab (Tastatur + relativ) um, Strg+Alt+M den Maus-Grab, Strg+Alt+R den relativen Modus, Strg+Alt+K den Tastatur-Grab, Strg+Alt+W warpt zur Mitte, Strg+Alt+Q beendet. Die Tastenkürzel selbst werden nicht als „Gast-Taste“ geloggt.
- Parameter: `--mouse-grab`, `--relative-mouse`, `--keyboard-grab`, `--timeout=<s>` (beendet sich selbst, Sicherheitsnetz), `--selftest` (Warp-Prüfungen ohne Nutzer), `--no-grab` (zum Debuggen, 8), `--log-core-motion` (im relativen Modus zusätzlich die beschleunigte Core-Bewegung, Iteration 3), `--stall=<ms>` (künstliche Pause pro Frame für den Hook-Test, 4.4).
- CMake ist maßgeblich, `premake5.lua`, `Makefile` und `.vcxproj` werden nachgezogen.

### 4.2 Automatische Tests unter X11

`demos/FPL_InputGrab/tests/run_grab_tests.sh` treibt die Demo mit `xdotool` (XTEST-Eingaben unterliegen Grabs und Confinement wie echte). Jeder Lauf läuft unter `timeout -s KILL`, denn stirbt der Client, gibt der X-Server alle Grabs frei. Demos werden per Signal beendet, nie über ihr Tastenkürzel, und nach jedem Test wird geprüft, dass keine Taste im X-Server gedrückt geblieben ist (Iteration 0, Stand). Ein X-Fehler wird wie im Viewer bis zu dreimal wiederholt. **Das Skript greift für einige Sekunden die echte Tastatur und Maus des Nutzers.** Es sagt das vor dem Start an und läuft nicht in Schleifen.

### 4.3 Win32

- **Bauen:** `x86_64-w64-mingw32-gcc` und `i686-w64-mingw32-gcc` sind lokal da, `clang-cl` + `lld-link` ebenfalls. MSVC über `.vcxproj` beim Nutzer.
- **Rauchtest:** `wine` ist installiert. Dort laufen ClipCursor, Raw Input und LL-Hooks grundsätzlich, aber nicht in jeder Einzelheit wie unter Windows, deshalb zählt wine nur als Vortest.
- **Abnahme auf echtem Windows** mit einer Checkliste pro Iteration (4.4). Dazu kommt ein Windows-Gast in qemu mit `-device usb-tablet`: Der liefert `MOUSE_MOVE_ABSOLUTE`-Rohdaten und prüft damit genau den VM-Pfad aus 2.4.

### 4.4 Handprüfliste Win32 (wächst mit den Iterationen)

Die Liste steht vollständig in der Demo-Readme, hier die Kernpunkte: Das Clip-Rechteck folgt beim Verschieben, bei Größenänderung und im Vollbild. Maus-Grab (Iteration 2, Demo mit `--mouse-grab --log-events`): Ein Klick auf die Titelleiste des inaktiven Fensters lässt sich zum Verschieben ziehen, erst nach dem Loslassen ist der Zeiger gefangen. Ein Aktivierungsklick auf Minimieren oder Schließen wirkt. Alt+Leertaste öffnet das Systemmenü, der Zeiger erreicht es. Nach Strg+Alt+Entf (das setzt den Clip zurück) ist der Zeiger spätestens nach 3 s wieder gefangen. Strg+Alt+M schaltet den Grab sichtbar ein und aus. Nach dem Beenden der Demo ohne `fplWindowShutdown()` ist der Zeiger frei. Nach Alt+Tab ist die Maus frei und kommt danach wieder in den Käfig. Im relativen Modus steht der Cursor still und ist unsichtbar, Klicks bleiben im Fenster, eine 1600-DPI-Maus liefert Rohzählwerte, und beim Verlassen erscheint der Cursor am Ausgangspunkt. Beim Tastatur-Grab öffnet Win kein Startmenü, Alt+Tab wechselt nicht, Alt+F4 schließt nicht, AltGr+Q liefert `@`, Strg+Alt+Entf funktioniert, und danach hängt keine Taste. Mit `--stall=500` geht der Hook nicht verloren, oder er wird neu installiert.

---

## 5. Iterationen

Die Reihenfolge geht vom Fundament (Zustandsmodell, Warp, Deltas) über das Einsperren zum relativen Modus, weil der auf dem Einsperren aufsetzt. Der Tastatur-Grab ist unabhängig davon und steht nach der Maus, weil er unter Win32 das meiste Risiko trägt. Jede Iteration ist für sich lieferbar: Modi, die noch fehlen, lehnt der Setter mit `false` ab.

### Iteration 0 — Branch, Demo, Ausgangsstand

- Branch auf `develop` setzen (Abschnitt 0).
- Demo-Gerüst aus 4.1 mit heutiger FPL: Events loggen, Fadenkreuz, Titelzeile, `--timeout`, `--log-events`. Testskript mit den Fällen, die heute schon gehen (Move, Tasten).
- Ausgangsstand festhalten: Alt+Tab und Super gehen an KWin (aktives Fenster wechselt, per `xdotool getactivewindow`). Den Verdacht aus 1.3 prüfen (nach Alt+Tab weg und zurück meldet der nächste Alt-Druck `Repeat`?), das Ergebnis kommt in 1.3.
- **Abnahme:** Die Demo baut mit gcc und clang unter Linux und mit MinGW x64/x86, das Testskript läuft durch und dokumentiert den Ausgangsstand.

**Stand (2026-09-24):**
- Erledigt: Branch auf `develop` gesetzt (Abschnitt 0). `demos/FPL_InputGrab` (C99, Software-Backbuffer): Fadenkreuz, Fokusrahmen, Titelzeile mit Fokus und letzter Taste, `--log-events`, `--timeout`, `--stall`, `--window`, `--title`, Strg+Alt+Q. CMake maßgeblich. `premake5.lua`, `Makefile` und `.vcxproj` stammen aus premake und sind bis auf die GUID gleich denen von `FPL_Window`. Im Workspace-`Makefile`, in der Solution und in `demos_final_platform_layer_premake5.lua` stehen nur die Einträge der neuen Demo.
- Builds: gcc und clang (`-Wall -Wextra`), MinGW x64 und x86, keine Warnung aus der Demo. Die übrigen Warnungen kommen aus `final_platform_layer.h` und sind nicht neu. Unter wine startet die x64-Version, loggt Events und endet über `--timeout`. Unter wine kam allerdings kein `gotfocus`, das wird in Iteration 1 mit angesehen.
- `tests/run_grab_tests.sh` mit fünf Tests, Ergebnis: `mouse_move`, `key_press_release`, `focus_switch` und `alt_tab_without_grab_switches` grün. **`no_stuck_alt_after_alt_tab` rot:** Der erste Alt-Druck nach der Rückkehr kommt als `repeat`, der Verdacht aus 1.3 ist damit bestätigt. Der Test bleibt rot, bis Iteration 1 das Loslassen bei Fokusverlust einbaut.
- Ausgangsstand festgehalten: Ohne Grab wechselt KWin bei Alt+Tab das Fenster, die Demo sieht nur Alt gedrückt und dann den Fokusverlust.
- Beim Testen gefunden und im Skript behoben: `xdotool key --window` geht bei einem **fokussierten** Fenster über XTEST. Beendet sich die Demo beim Q-Druck ihres Strg+Alt+Q, bricht xdotool am zerstörten Fenster ab und lässt Strg, Alt und Q nie los. Die Tasten blieben im X-Server gedrückt (Strg+Q lief als Autorepeat in den nächsten Test). Das Skript beendet Demos jetzt per Signal, prüft nach jedem Test mit python-xlib (`query_keymap`), ob Tasten hängen, lässt sie über XTEST los und wertet den Test dann als rot.
- Beim Testen gefunden, gehört nicht zu diesem Plan: FPL setzt unter X11 kein `WM_NAME` (1.3, 7.2), und das Workspace-`Makefile` ist gegenüber premake veraltet (`FPL_Process` fehlt). Ein komplettes Neuerzeugen per premake würde 41 Projektdateien ändern.

### Iteration 1 — API, Zustandsmodell, Warp, Deltas

- Öffentliche Deklarationen aus 2.1 mit Doxygen, Abschnitt „Maus- und Tastatursteuerung“ in `final_platform_layer.docs` (neben `section_category_window_style_cursor`).
- `fpl__InputGrabState`, `fpl__UpdateInputGrab` mit allen Aufrufstellen aus 3. Die drei Schalter lehnen `true` vorerst mit `false` ab, bis ihre Iteration sie umsetzt.
- `fplWarpWindowCursor` unter Win32 und X11 (`XWarpPointer` in den Loader).
- `deltaX`/`deltaY` in Move-Events (2.7).
- Loslassen aller gedrückten Tasten und Maustasten bei Fokusverlust, für alle Apps (2.2). Damit wird `no_stuck_alt_after_alt_tab` grün.
- `--selftest`: Warp auf fünf Positionen (Ecken, Mitte) → `fplQueryCursorPosition` = Client-Ursprung + Ziel, das nächste Move-Event hat Delta 0.
- **Abnahme:** Der Selbsttest ist unter X11 grün, unter wine und auf Windows ebenso. Bei Handbewegung ist die Summe der Deltas gleich der Positionsdifferenz (Testskript mit `xdotool mousemove`). Kein hängender Modifier nach Alt+Tab. Die Build-Matrix aus 6 ist grün.

**Stand (2026-09-24):**
- Erledigt: Deklarationen samt Doxygen für `fplWarpWindowCursor`, `fplSetWindowMouseGrab`/`fplIsWindowMouseGrabbed`, `fplSetWindowRelativeMouse`/`fplIsWindowRelativeMouse` und `fplSetWindowKeyboardGrab`/`fplIsWindowKeyboardGrabbed`. Die drei Schalter sind `fpl_common_api` und lehnen `true` vorerst mit `false` und einer Warnung ab. `fpl__InputGrabState` und `fpl__MouseLockState` liegen in `fpl__PlatformWindowState`, dazu `hasFocus` und `isMinimized`. `fpl__UpdateInputGrab` wird bei Fokus, Minimieren/Wiederherstellen, Zeigen/Verstecken, Schließen und `fplWindowShutdown` gerufen (Win32 und X11) und wendet noch nichts an.
- `fplWarpWindowCursor`: Win32 über `ClientToScreen` + `SetCursorPos`, X11 über `XWarpPointer` (neu im Loader, auch als `extern` für `FPL_NO_RUNTIME_LINKING`). Die „letzte Position“ wird auf das Ziel gesetzt, das folgende Move-Event hat Delta 0.
- `deltaX`/`deltaY` in `fplMouseEvent`, berechnet in `fpl__HandleMouseMoveEvent`. Die Basis fällt weg bei Fokusgewinn, bei X11 `EnterNotify` (neue ABI-Konstanten `FPL__X11_EnterNotify`/`LeaveNotify`) und bei Win32 `WM_MOUSELEAVE` (neu: `TrackMouseEvent`, bei jedem ersten `WM_MOUSEMOVE` neu gespannt).
- Fokusverlust: `fpl__HandleWindowFocusChanged` schickt für jede noch gedrückte Taste und Maustaste ein Release-Event und danach `LostFocus`, für alle Apps. Win32 und X11 schieben ihre Fokus-Events nur noch darüber.
- Doku: Warp-Abschnitt und Hinweise zu Deltas und Fokusverlust in `final_platform_layer.docs`, drei Zeilen im Changelog v1.0.1.
- Demo: Deltas im Log, Tastenkürzel Strg+Alt+G/M/R/K/W, Parameter `--mouse-grab`, `--relative-mouse`, `--keyboard-grab` und `--selftest`. Der Selbsttest startet nach dem Fokus oder spätestens nach 2 s, warpt auf vier Ecken und die Mitte und prüft Zielposition, Delta 0 und einen gleichbleibenden Bildschirm-Ursprung.
- **Abnahme:** X11: alle 9 Tests grün (`mouse_move`, `move_deltas`, `no_delta_after_reenter`, `warp_selftest`, `key_press_release`, `focus_switch`, `focus_loss_releases_keys`, `alt_tab_without_grab_switches`, `no_stuck_alt_after_alt_tab`), der rote Test aus Iteration 0 ist damit behoben. Win32 unter wine: Selbsttest grün (alle fünf Ziele, Delta 0). Die Win32-Abnahme auf echtem Windows kommt gesammelt in Iteration 6 (7.1). Build-Matrix grün: `FPL_InputGrab`, `FPL_Window`, `FPL_Input`, `FPL_Test`, `FPL_NoRuntimeLinking`, `FPL_NoPlatformIncludes`, `FPL_ImGui` mit gcc und clang, MinGW x64/x86 für `FPL_InputGrab`, `FPL_Window`, `FPL_Input` und `FPL_Test` (`-O2`, siehe 1.3). Keine neue Warnung aus dem neuen Code.
- Gefunden: Win32 verliert die Events vom Fensteraufbau, `FPL_Test` ist auf `develop` rot (beides 1.3 und 7.2). Im Testskript lief der Selbsttest schneller durch, als das Skript die Aktivierung bestätigen konnte. Das Skript prüft die Aktivierung dort nicht mehr.

### Iteration 2 — Maus einsperren

- X11 `XGrabPointer`/`XUngrabPointer` mit Konstanten, Ergebnisbehandlung und Wiederholung. Win32 `ClipCursor` nach 2.3.
- **Abnahme X11 (automatisch):** Nach `xdotool mousemove` in eine Bildschirmecke liegt der Zeiger im Client-Rechteck (`xdotool getmouselocation`). Nach einem Fokuswechsel auf ein anderes Fenster ist er frei, nach der Rückkehr wieder eingesperrt. Nach einer Größenänderung folgt der Käfig. Ein zweiter Client mit gehaltenem Pointer-Grab (`AlreadyGrabbed`) wird überstanden: Der Grab greift, sobald der andere loslässt. Win32 nach Handprüfliste.

**Stand (2026-09-25):**
- Erledigt: `fplSetWindowMouseGrab(true)` ist freigeschaltet. `fpl__UpdateInputGrab` ruft die neue Plattformfunktion `fpl__PlatformApplyMouseLock`, sobald gewünscht und angewendet auseinanderliegen, und merkt sich den angewendeten Zustand nur bei Erfolg. Bei einem Fehlschlag setzt es `isRetryPending` und `nextRetryTimeMilliseconds`, `fpl__RetryInputGrab` versucht es beim Pumpen (`fplWindowUpdate`, `fplPollEvents`, `fplPollEvent` bei leerer Queue) höchstens alle 50 ms erneut. Die erste Ablehnung einer Serie wird auf Stufe Verbose geloggt. `fpl__ReleaseWindow` hebt den Grab vor dem Zerstören des Fensters auf, auch ohne `fplWindowShutdown()`.
- X11: `XGrabPointer(window, owner_events=True, Button/Motion, Async, Async, confine_to=window, None, CurrentTime)` und `XUngrabPointer` + `XFlush`, neu im Loader (vier Stellen) samt sechs ABI-Konstanten in beiden Zweigen. Größenänderung und Verschieben verfolgt der X-Server selbst.
- Win32: `ClipCursor` auf das Client-Rechteck in Bildschirmkoordinaten, nur wenn es sich von `GetClipCursor` unterscheidet (neu im Loader). Freigegeben wird nur, wenn der aktuelle Clip im eigenen Rechteck liegt (Ecken-Prüfung wie SDL, weil das System das Rechteck auf den Bildschirm beschneidet). Neu gesetzt bei `WM_WINDOWPOSCHANGED` und alle 3 s beim Pumpen. Ein leerer Client-Bereich gibt den Clip frei und zählt trotzdem als angewendet, der nächste `WM_WINDOWPOSCHANGED` setzt ihn wieder.
- Aussetzen (neues gemeinsames Flag `isMouseLockSuspended`, wirkt nur auf die Maus): Win32 zählt `WM_ENTERSIZEMOVE`/`WM_ENTERMENULOOP` und ihre `EXIT`-Gegenstücke in `modalLoopDepth`. Dazu kommt der **Aktivierungsklick** nach SDL-Vorbild (`focus_click_pending`): Ist beim Fokusgewinn die linke oder rechte Taste gedrückt, wartet der Grab, bis beide losgelassen sind (`GetAsyncKeyState`, beim Pumpen geprüft). Das deckt Klicks auf Titelleiste und Rahmenknöpfe des inaktiven Fensters ab. Ein eigenes „Klick auf den Rahmen“-Flag (`WM_NCLBUTTONDOWN` bis `WM_CAPTURECHANGED`) wurde bewusst weggelassen: Solange der Grab wirkt, erreicht der Zeiger den Rahmen nicht, und bleibt `WM_CAPTURECHANGED` aus, wäre der Grab für immer ausgesetzt.
- `fplWarpWindowCursor` begrenzt das Ziel auf `[0, Breite−1] × [0, Höhe−1]`, solange der Grab wirkt (`fpl__LimitWarpToConfinedArea`), damit die Delta-Basis der echten Position entspricht.
- Doku: Abschnitt `section_category_window_style_cursor_grab` („Grabbing the mouse“) mit Beispiel und Hinweisen zu Fokus, Wiederholung, Win32-Rahmen und Debugger, Hinweis im Warp-Abschnitt, Doxygen der Setter. Changelog v1.0.1: zwei Zeilen.
- Demo: `--log-fpl` leitet FPLs Log (bis Verbose) als `fpl level=…`-Zeilen ins Event-Log, dafür ist `FPL_LOGGING` gesetzt. Der Selbsttest warpt mit `--mouse-grab` zusätzlich nach (−50, −50) und (Breite+49, Höhe+49) und erwartet (0, 0) und (Breite−1, Höhe−1).
- Testskript: sechs neue Tests `mouse_grab_confines`, `mouse_grab_released_on_focus_loss`, `mouse_grab_restored_on_focus_gain`, `mouse_grab_follows_resize`, `mouse_grab_retries_when_already_grabbed` (ein python-xlib-Client hält den Pointer 2 s, im Demo-Log muss `XGrabPointer failed with AlreadyGrabbed` stehen, danach muss der Käfig greifen) und `warp_limited_by_mouse_grab` (Selbsttest mit `--mouse-grab`).
- **Abnahme:** X11: alle 15 Tests grün. Im ersten Lauf war `no_stuck_alt_after_alt_tab` rot, weil während des Tests jemand die echte Maus bewegte und ein drittes Fenster den Fokus bekam (0.7). Einzeln wiederholt war er grün. Der Wiederholungstest hat den Pfad belegt (`AlreadyGrabbed` im Log, danach gefangen). Win32 unter wine: Selbsttest grün, mit `--mouse-grab` auch die beiden Ziele außerhalb (0, 0) und (639, 399). Das zeigt, dass FPL den Clip als angewendet führt und begrenzt. Ob `ClipCursor` den Zeiger unter wine physisch hält, wurde nicht geprüft, das gehört zur Windows-Abnahme in Iteration 6. Build-Matrix grün (sieben Demos mit gcc und clang, MinGW x64/x86 für `FPL_InputGrab`, `FPL_Window`, `FPL_Input`, `FPL_Test`). Keine neue Warnung aus dem neuen Code, verglichen gegen den Header von `HEAD` mit gcc, clang, MinGW x64/x86 (C) und g++, clang++, MinGW g++ (C++).
- Gefunden: Win32 `eventCallback` bekommt `MSG**` (1.3, 7.2).

### Iteration 3 — Relativer Modus

- Win32 Raw Input nach 2.4, das Raw-Input-Überbleibsel aus `fpl__Win32SetCursorState` entfällt.
- X11: libXi optional laden, XI2-Pfad, Rückfallebene, `FPL_NO_X11_XINPUT2`.
- **Abnahme X11 (automatisch):** Eine Folge `xdotool mousemove_relative` mit bekannten Summen ergibt exakt dieselbe Deltasumme. Dass die Deltas roh sind, wird von Hand belegt: Die Demo loggt mit `--log-core-motion` zusätzlich die Core-Bewegung, und bei schneller Bewegung mit einer echten Maus und eingeschalteter Beschleunigung ist die Core-Summe größer als die XI2-Summe. Der Zeiger ist unsichtbar und bleibt im Fenster. Nach dem Verlassen steht er am Ausgangspunkt. Bei Fokusverlust kommen keine Deltas mehr, und der Zeiger ist frei. Die Rückfallebene besteht dieselben Tests bis auf die Beschleunigung. Ob XTEST überhaupt `XI_RawMotion` erzeugt, wird zuerst geprüft, sonst wird der XI2-Teil von Hand abgenommen.
- **Abnahme Win32:** Handprüfliste sowie der qemu-Gast mit usb-tablet (absoluter Rohpfad).

### Iteration 4 — Tastatur-Grab

- X11 `XGrabKeyboard`/`XUngrabKeyboard` nach 2.5, Fokusverhalten während des Grabs prüfen.
- Win32-Hook nach 2.5: SDL-Tastenmenge, eigene Modifier-Führung, AltGr, einmaliges Durchreichen, `DefWindowProc`-Sperre, Neuinstallation.
- **Abnahme X11 (automatisch):** Mit Grab loggt die Demo `alt+Tab`, `super` und `alt+F4`, und das aktive Fenster wechselt nicht. Ohne Grab wechselt es. Text-Events kommen während des Grabs weiter an. Win32 nach Handprüfliste.

### Iteration 5 — Was qemu darüber hinaus braucht

- `scanCode` (Satz 1) unter Win32 und X11, evdev→Satz-1-Tabelle.
- X11-Maustasten 8/9 → X1/X2, horizontales Rad auf beiden Plattformen.
- `fplMouseEventType_Enter`/`_Leave`.
- **Abnahme:** Dieselbe physische Taste liefert unter `setxkbmap us` und `setxkbmap de` denselben `scanCode`. Alle Tasten einer 105er-Tastatur werden gegen eine Erwartungstabelle geprüft, auch Ziffernblock-Enter, `<>`, Pause und Druck. Die Keycodes schickt python-xlib über XTEST, weil `xdotool key` nur Keysyms kennt. Die Win32-Scancodes stimmen mit denselben Erwartungswerten überein (Handprüfung mit der Demo). Enter/Leave erscheinen genau einmal pro Übergang und nicht bei eigenen Grabs.

### Iteration 6 — Abschluss

- Changelog vollständig, Doku-Seite mit Beispiel (qemu-artige Schleife aus 2.1), Readme der Demo mit Handprüfliste.
- `FPL_Test`: reine Hilfsfunktionen ohne Fenster prüfen (Umrechnung absolut → relativ, Restsammlung, evdev→Satz-1-Tabelle ohne Doppelungen, `sizeof(fplMouseEvent)` und `sizeof(fplEvent)` in `TestSizes`).
- Build-Matrix aus 6 und die **gesammelte Win32-Abnahme auf echtem Windows** für alle Iterationen (7.1): Handprüfliste 4.4 und der Selbsttest der Demo.

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

### 7.1 Entschieden am 2026-09-24

Nach Iteration 0 abgestimmt. Die ersten vier Punkte hat der Nutzer ausdrücklich entschieden, die übrigen sind wie empfohlen übernommen.

| Punkt | Entscheidung | Wo im Plan |
|---|---|---|
| API-Form | **getrennte Schalter** wie SDL: `fplSetWindowMouseGrab`, `fplSetWindowRelativeMouse`, `fplSetWindowKeyboardGrab`, `fplWarpWindowCursor` (statt eines Modus-Enums). Der relative Modus gewinnt über den Maus-Grab. | 2.1 |
| Tasten bei Fokusverlust loslassen | **ja, für alle Apps** (Release-Events) | 2.2, Iteration 1 |
| Iteration 5 | **gehört in diesen Plan** | 2.8, Iteration 5 |
| Format von `scanCode` | **PC Satz 1**, erweiterte Tasten als `0xE0xx`, Pause als `0xE11D`, 0 = unbekannt | 2.8 |
| Alt+F4 beim Tastatur-Grab | **geht an die App**, das Fenster schließt nicht | 2.5 |
| Grab bei Fokusverlust | aussetzen und wiederherstellen, die Anforderung bleibt | 2.2 |
| Relative Deltas | roh und unbeschleunigt, beschleunigt nur in der X11-Rückfallebene | 2.4 |
| Win32-Hook | Fensterthread, SDL-Tastenmenge inklusive Modifier | 2.5, 8 |
| Cursorbild | eigener Folgeplan | 7.2 |
| Win32-Events vom Fensteraufbau (1.3) | **eigener Branch** von `develop` (`fpl/win32-init-events`), danach `develop` in diesen Branch mergen | 0.1, 7.2 |
| Abnahme auf echtem Windows | **gesammelt in Iteration 6**, bis dahin MinGW + wine | 4.3, Iteration 6 |

### 7.2 Folgepunkte (nicht in diesem Plan)

- **Cursorbild aus RGBA mit Hotspot** (`fplSetWindowCursorImage`) für `dpy_cursor_define`. qemu ist beim Kanalformat selbst uneinheitlich (`ui/cursor.c` baut `0xAABBGGRR`, sdl2 liest mit ARGB-Masken). Die FPL-Funktion bekommt deshalb ein ausdrückliches Pixelformat mit geradem Alpha. X11 braucht libXcursor (optional) mit 1-Bit-Rückfall, Win32 `CreateIconIndirect` mit 32-Bit-DIB.
- **qemu-Frontend selbst** (`ui/fpl.c`, meson, `DisplayChangeListenerOps`, Grab-Logik nach sdl2 oder gtk, Pumpen in `dpy_refresh`). Dafür wird vorher geprüft: **Win32-Fibers gegen qemu-Coroutinen.** FPL ruft beim Fensteraufbau `ConvertThreadToFiber` (`:19034`), qemus `coroutine-win32.c` macht das im Hauptthread auch und behandelt „schon ein Fiber“ vermutlich nicht. FPL sollte `IsThreadAFiber()`/`GetCurrentFiber()` benutzen und nur zurückwandeln, was es selbst gewandelt hat. Ob das allein reicht, hängt von der Init-Reihenfolge in qemu ab.
- **Deltas in `fplMouseState`** (Polling), zusammen mit den nie geschriebenen `wheelDeltaX/Y`.
- **Tastenzustand bei Fokusgewinn abgleichen** (X11 `XQueryKeymap`, Win32 `GetKeyboardState`) wie SDL, falls das Loslassen bei Fokusverlust nicht reicht.
- **RDP-Sonderfälle** im relativen Modus (SDL zentriert bei `SM_REMOTESESSION` nach und verwirft große Sprünge).
- **Mäuse mit hoher Abtastrate** (4–8 kHz): `GetRawInputBuffer` statt einer `WM_INPUT` pro Paket. SDL3 liest Raw Input dafür in einem eigenen Thread.
- **Wayland nativ:** FPL hat kein Wayland-Backend. Unter XWayland greift der Pointer nur über die Pointer-Constraints des Compositors, und `XGrabKeyboard` blockiert Compositor-Kürzel wohl nicht (ungeprüft).
- **Mehrere Fenster** (qemu Multi-Head): FPL kennt nur ein Fenster.
- **Win32-Events vom Fensteraufbau** nicht verwerfen (1.3): etwa das erste `fpl__ClearInternalEvents()` nach der Initialisierung auslassen, oder die Queue erst leeren, wenn die App sie einmal gelesen hat. Das ändert das Verhalten von `fplWindowUpdate()`. **Entschieden:** eigener Branch `fpl/win32-init-events` von `develop` (7.1).
- **`FPL_Test` wieder grün bekommen** (1.3): `double free` bei `-O3`, Prozess-Lebensdauer-Test bei `-O2`, `ForceInlineTest` unter MinGW.
- **`-Wstringop-overflow` in `fpl__WaitForThreadsStopped`**: gcc meldet bei `-O3` einen Fehlalarm in `fplAtomicLoadU64`, seit sich das Inlining durch Iteration 1 verschoben hat. Der Code ist unverändert.
- **`WM_NAME` unter X11** zusätzlich zu `_NET_WM_NAME` setzen (`XStoreName` bzw. `XSetWMName`), damit ältere Fenstermanager und Werkzeuge wie `xdotool search --name` den Titel sehen (1.3).
- **Win32 `eventCallback` mit `MSG*` aufrufen** statt `&msg` (1.3). Eine Zeile, aber eine Verhaltensänderung für jeden, der sich auf das Falsche eingestellt hat, deshalb mit Changelog-Eintrag und am besten zusammen mit `fpl/win32-init-events`.
- **Tastatur-Grab im modalen Win32-Loop:** `isMouseLockSuspended` setzt nur die Maus aus. Ob der Hook während Verschieben/Größenänderung aktiv bleiben soll, wird in Iteration 4 entschieden.
- **X11: vom Server aufgehobener Grab.** Wird das Fenster unabbildbar, hebt der X-Server einen Pointer-Grab von selbst auf. FPL merkt das nur über Minimieren, Verstecken oder Fokusverlust, die in der Praxis immer mitkommen. Ein Fall ohne diese Events ist nicht bekannt.

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
