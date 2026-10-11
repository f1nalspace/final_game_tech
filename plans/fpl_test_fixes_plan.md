# Plan: FPL — Befunde aus dem Gesamttest beheben (vor dem Release v1.0.1)

Branch `fpl/test-fixes` von `develop` (08e6c56a), Worktree `../final_game_tech_fpl-test-fixes`. Grundlage ist der Gesamttest vom 2026-09-28: `develop` (v1.0.1) gegen `master` (v1.0.0).

---

## 0. Handover

### 0.1 Wo wir stehen (Handover 2026-09-28 abends)

- **Iteration 0 und 1 sind fertig und abgenommen** (Stand-Blöcke in Abschnitt 5). Basislinie: `demos/immediates/fpl_test_runs/baseline/`.
- **Iteration 2 ist im Code weitgehend fertig, die Abnahme läuft noch:**
  - A6/A7 Thread-Umbau nach 3.4 ist umgesetzt: `fpl__GetFreeThread` vergibt reihum (`nextSlotNumber`), zählt `generation` hoch und schließt unter Win32 das Handle des Vorgängers. Win32 erzeugt mit `CREATE_SUSPENDED` + `ResumeThread`, die Thread-Prozedur und `fplThreadTerminate` schließen kein Handle mehr. POSIX: `pthread_create` in eine lokale Variable, `isValid` ist die Startschranke in `fpl__PosixThreadProc`. Die Waits vergleichen `generation` statt `id`. Der Fehlschlagpfad heißt `fpl__ReleaseReservedThread`. POSIX `fplAtomicStore*` laufen über `__atomic_store_n` (SEQ_CST). Neues Feld `fplThreadHandle::generation`.
  - Ergebnisse bisher:
    - TSan `FPL_Test`: **0 Meldungen** (vorher 272).
    - gcc-debug: bestanden.
    - Wine32-Stress 20/20 (Basislinie: Absturz in Runde 3–5).
  - A1 (`XDisplayKeycodes` gibt `int` zurück), A3 (lokale Zeigerkopie, direktes Lesen des Zustands in `fpl__WaitForThreadsStopped`), A4 (`(void)` in den leeren Linux-Funktionen) und A5 (`FPL_NoRuntimeLinking`: `premake5.lua`, `Makefile`, Kommentar; `make config=debug_linuxx64` linkt) sind erledigt.
  - C5 (neuer Testfehler, erledigt): `ConditionThreadsTest` wartete ohne gehaltenes Mutex und ohne Prädikat. Der Slave wartet jetzt unter dem Mutex auf `isSignaled`, der Master setzt das Flag unter dem Mutex.
  - Changelog-Zeilen für alles oben stehen in v1.0.1 (Threading, X11) und im Kopf von `fpl_test.c`.
  - `build_all iter2`: alle 42 Projekte bauen, jedes hat 6 FPL-Warnungen weniger, 0 neue Warnstellen.
  - `matrix iter2`: dieselben 12 Fehlschläge. Neu sind nur 2 `-Wpedantic`-Meldungen am neuen pthread-Makro (B15-Klasse).
- **Läuft im Hintergrund (vom 28.09. abends):** `threadstress.sh iter2 100 wine32 wine64 linux tsan`, Ausgabe in `demos/immediates/fpl_test_runs/threadstress_iter2_all.out` und `iter2/threadstress/results.tsv`. Beim Handover stand Wine32 bei 11 Runden. Das Skript beendet sein Xvfb selbst.
- **Nächste Schritte:**
  1. Ergebnis des Stresses lesen. Abnahme Iteration 2: je 100 Runden Linux, TSan, Wine64 und Wine32 ohne Hänger oder Absturz. Bei einem Befund liegt der Backtrace in `iter2/threadstress/<ziel>/`.
  2. `fpltest.sh iter2 asan clang-o2 mingw64-o2 mingw32-o2` laufen lassen. Unter Wine scheitert weiter `TestTimes` (B4/C3, Iteration 3).
  3. **A2 mit dem Nutzer klären** (7.2 Punkt 1): `fpl_extern_inline` wird von FPL nirgends benutzt, ist aber ein öffentliches Makro aus #183. Die Doku-Notiz „jedes C-TU muss `FPL_MULTI_TRANSLATION_UNIT` definieren“ ist in jedem Fall falsch (`final_platform_layer.docs` Zeile ~284 und Tabelle ~5032). Offen ist nur, ob der Standard in C `static inline` wird und der Schalter entfällt.
  4. Stand-Block Iteration 2 in Abschnitt 5 schreiben, dann Commit-Vorschlag an den Nutzer (siehe 0.2).
  5. Weiter mit Iteration 3.
- Die Offen-Liste in 7.2 ist unverändert, der Nutzer hat mit „loslegen“ die Empfehlungen für den Prüfstand (Punkt 4) freigegeben, die übrigen Punkte fragen, wenn sie dran sind.

### 0.2 Uncommittete Arbeit und Sicherung

Nichts ist committet, der Nutzer committet selbst. Gesichert wird mit einem versteckten Ref (`refs/backup/fpl-test-fixes-plan`), per temporärem Index unter `.git/worktrees/final_game_tech_fpl-test-fixes/` mit literalem Pfad. Ein `rm` mit einem Pfad aus einer Variable blockt die Sicherheitsprüfung.

Vorschlag für kleine Commits, alle englisch und einzeilig:
1. `tools/fpl_test` + Plan: `Add FPL test bench and fpl_test_fixes_plan.md`
2. `FPL: Resolve pthread functions through the global scope first` (B16)
3. `FPL_Test: Fix memory, semaphore and condition tests` (C1, C2, C5)
4. `FPL: Publish threads before they run, generation counter for waits` (A6/A7, A3, Atomics)
5. `FPL: Fix XDisplayKeycodes prototype and unused Linux parameters` (A1, A4)
6. `FPL_NoRuntimeLinking: Complete the link lists` (A5)

Die Aufteilung der Header-Änderungen auf Commits 2, 4 und 5 geht nur per `git add -p`.

### 0.3 Arbeitsweise

- **Nur in diesem Worktree.** Das Hauptverzeichnis bleibt unberührt. `fpl/test-fixes` in Gittyup **nicht** im Hauptverzeichnis auswählen: Gittyup schreibt dann Dateien und Index um, ohne HEAD umzustellen (am 2026-09-28 passiert).
- Testläufe schreiben nach `demos/immediates/fpl_test_runs/` im Worktree. Das Verzeichnis ist ignoriert (`/demos/immediates` in `.gitignore`), also kein `/tmp` und keine Spuren im Repo.
- Fenster-Tests laufen auf Xvfb (`Xvfb :77 -screen 0 1600x1000x24 +extension GLX`), nie auf dem Desktop. Xvfb über die gemerkte PID beenden.
- Wine-Läufe nie parallel im selben Verzeichnis: Die Sicherheitstests schreiben Fixtures neben die EXE und kollidieren sonst (am 2026-09-28 passiert).
- **Wine nur über `fpl_test_wine`** aus `tools/fpl_test/common.sh`. Das nutzt ein eigenes Prefix (`demos/immediates/fpl_test_runs/wineprefix`, Absturzdialog aus) und das private Display. Ein Absturz schreibt dann seinen Backtrace ins Log. Am 2026-09-28 ist ein Wine-Absturzdialog auf dem Desktop des Nutzers aufgegangen, weil ein Lauf ohne Display-Umleitung gestartet war.

### 0.4 Prüfstand (`tools/fpl_test/`)

| Skript | Zweck |
|---|---|
| `build_all.sh <label>` | alle FPL-Projekte bauen, danach `compare_builds.py --summary` |
| `compare_builds.py <basis> <label>` | Build-Ergebnisse und FPL-Warnungen nach Flag und Quelltext vergleichen |
| `matrix.py run <label> [--only RE]`, `matrix.py compare <basis> <label>` | Compile-Matrix (135 Jobs), Teilläufe ergänzen die Ergebnisse |
| `fpltest.sh <label> [variante …]` | `FPL_Test`: gcc-debug, clang-o2, asan, tsan, mingw64-o2, mingw32-o2, mingw64-o0 |
| `threadstress.sh <label> <runden> [ziel …]` | `threadloop.c` auf linux, tsan, wine64, wine32. Ein Wächter meldet Hänger und Abstürze und sichert Backtraces. |

Die Ergebnisse liegen unter `demos/immediates/fpl_test_runs/<label>/`, mit einer Kopie des getesteten Headers für den Warnungsvergleich.

### 0.5 Code-Landkarte

Nach Namen suchen, die Zeilen wandern. Die Zeilennummern gelten für `develop` 08e6c56a.

| Thema | Wo |
|---|---|
| Compiler-Erkennung, Inline, Ausrichtung, Asm | `FPL_COMPILER_MINGW` (~2386), `fpl__m_Asm` (~2551), `fpl__MinAlignment` (~2570), `fpl__m_AlignAs` (~2594), `fpl__m_force_inline` (~2801), `fpl_extern_inline` (~2672) |
| API-Linkage | `fpl__m_api`, `fpl__m_platform_api`, `fpl__m_common_api` (~2732–2760) |
| Crash bei Fehlern | `fpl__HandleError` (~15287) |
| Thread-Warten | `fpl__WaitForThreadsStopped` (~16748), Win32 `thread->id = threadId` (~22283), POSIX (~26096) |
| Win32-Zeitmessung | `fplTimestampQuery`, `fplTimestampElapsed` im Block „Win32 Timings“ (~23423) |
| POSIX-Speicher | `fplMemoryAllocate`/`fplMemoryFree` (~26585) |
| X11-Funktionstabelle | `FPL__FUNC_X11_XDisplayKeycodes` (~13399), Extern-Block für `FPL_NO_RUNTIME_LINKING` (~13521) |
| GLX | `legacyRenderingContext` in der X11-OpenGL-Initialisierung (~34815, Aufräumen ~34955) |
| Vulkan | `#include <vulkan/vulkan.h>` (~35403), Debug-Messenger (~35710), `surfaceKHR` (~35990) |
| Linux-Plattform | `fpl__LinuxInitPlatform`/`fpl__LinuxReleasePlatform` (~32703) |
| Input-Schalter | `FPL__ENABLE_INPUT_RAWINPUT` (~3230), `FPL__ENABLE_INPUT_UNIX_GAMEPAD` (~3242) |
| Tests | `demos/FPL_Test/fpl_test.c`, `security_tests.c`, `process_tests.c` |

---

## 1. Stand

### 1.1 Getestet am 2026-09-28

- **Alle 42 CMake-Projekte**, die FPL nutzen (`-Wall -Wextra`, Debug), gegen `develop` und `master`:
  - Alle bauen auf `develop` bis auf `FPL_NoCRT`, das auf Linux schon immer scheitert (auf `master` identisch, nur für Windows).
  - `FontRendering` baut auf `develop` wieder.
  - FPL-Warnungen sind insgesamt weniger als auf `master`. Neu ist nur A1.
- **Compile-Matrix:** 205 Konfigurationen × 2 Stände.
  - Compiler: gcc 16, clang 22, g++, clang++, `gcc -m32`, MinGW 32/64.
  - Standards: C99, C11, C17, gnu99, C++11, C++17, C++20.
  - Jeder dokumentierte Schalter einzeln, dazu Kombinationen, `gcc -fanalyzer` und `clang --analyze`.
  - Nichts, was auf `master` baut, bricht auf `develop`. `FPL_LOG_MULTIPLE_WRITERS` baut wieder.
- **`FPL_Test`:**
  - gcc Debug und clang `-O2`: bestanden.
  - ASan, UBSan und LeakSanitizer: sauber, sobald C1 und C2 korrigiert sind.
  - MinGW 64/32 unter Wine: bis auf B4, C3, C4 und den Hänger A6 bestanden.
- **FUI_Test und FPL_ImageViewer** mit den neuen `final_ui`-Headern: byteidentisch, siehe `ui/gl3-backend`.

### 1.2 Nicht getestet (holt Iteration 8 nach)

- Rauchtest aller Fenster-, Video- und Input-Demos auf Xvfb.
- Audio-Demos (brauchen stille Ausgabe, siehe 3.10).
- MSVC und echtes Windows. Das prüft der Nutzer anhand der Handprüfliste in 4.4.

---

## 2. Befunde

„Geprüft“ heißt, dass der Befund am Code oder mit einem eigenen Lauf bestätigt ist. „Agent“ heißt, dass er nur aus der Compile-Matrix stammt und in seiner Iteration zuerst bestätigt wird.

### 2.1 Neu in `develop`

| ID | Befund | Wirkung | Stand |
|---|---|---|---|
| A1 | `FPL__FUNC_X11_XDisplayKeycodes` ist mit Rückgabe `fpl__X11_KeySym *` deklariert, Xlib hat `int XDisplayKeycodes(Display*, int*, int*)`. Kam mit Fix #191. | Zur Laufzeit harmlos, weil der Rückgabewert ignoriert wird. `-Wcast-function-type` in `FPL_NO_RUNTIME_LINKING`, und die Extern-Deklaration widerspricht Xlib. | geprüft |
| A2 | `fpl_extern_inline` ist ein öffentliches Makro (#183), FPL selbst benutzt es nirgends. Die Doku (`final_platform_layer.docs`, „@note In a C build, every translation-unit …“ und die Tabelle der Compiler-Optionen) verlangt trotzdem, dass jedes C-TU `FPL_MULTI_TRANSLATION_UNIT` setzt. Der C-Standard ist `extern inline`, das über mehrere TUs doppelte Symbole erzeugt. | Irreführende Doku, unsicherer Standard | geprüft |
| A3 | `-Wstringop-overflow` bei gcc `-O2` und MinGW in `fpl__WaitForThreadsStopped`: Die Thread-Zeiger werden nach der Null-Prüfung aus dem Array des Aufrufers neu geladen. | Fehlalarm, aber in jedem optimierten gcc-Build | geprüft |
| A4 | `fpl__LinuxInitPlatform`/`fpl__LinuxReleasePlatform` sind leer, seit der setlocale-Code weg ist. `-Wunused-parameter initState`. | Warnung in jedem Linux-Build | geprüft |
| A5 | `demos/FPL_NoRuntimeLinking`: `Makefile`, `premake5.lua` und der Kommentar im Quelltext linken nur `m pthread GL vulkan asound X11`. Es fehlen `Xi` (neu), `Xrandr`, `Xinerama` sowie pulse und pipewire, je nachdem welche Header gefunden werden. | Makefile-Build linkt nicht, CMake schon | teils geprüft |
| A6 | **Wine 32 Bit: Absturz** in „Concurrent thread create/teardown 8×100“. Zuerst sah das wie ein Hänger aus, weil der Absturz im Wine-Debugger stehen blieb. Am 2026-09-28 in Runde 5 von 100 reproduziert: `Unhandled page fault on execute access to FFBB113C`. Der abgestürzte Thread startet an `0xffbb113c` mit Parameter 0, ohne einen FPL-Rahmen im Stack. `0xffbb113c` ist die untere Hälfte einer Adresse im 64-Bit-`ntdll` von Wine (`0x6fffffbb113c`), FPL übergibt `CreateThread` dagegen seine eigene Prozedur samt Kontext. Das spricht für einen Fehler in Wines WoW64-Schicht. Ausgelöst werden könnte er durch ein FPL-Verhalten, etwa durch `TerminateThread` in `fplThreadTerminate`. Zusätzlich verdächtig, aber nicht als Ursache belegt: `thread->id = threadId` ist unter Win32 ein **nicht atomarer 64-Bit-Store**, und `fpl__WaitForThreadsStopped` nutzt die **OS-Thread-ID als Generationsmarke**, obwohl Windows Thread-IDs wiederverwendet. Beleg: `demos/immediates/fpl_test_runs/baseline/threadstress/wine32-run1-desktop/`. | Absturz, Ursache offen (Wine oder FPL) | reproduziert |
| A7 | **ThreadSanitizer (seit B16 möglich): 272 Meldungen in `FPL_Test`.** Am 2026-09-28 in Iteration 1 gefunden, Einzelheiten unter der Tabelle. | Speichermodell- und Lebensdauerfehler im Thread-Code | gefunden |

Einzelheiten zu A7, Beleg in `demos/immediates/fpl_test_runs/iter1/fpltest/tsan/run.log`:
- **Rennen in FPL:** `fplThreadCreateWithParameters` (103, an mehreren Stellen), `fplMemoryClear` auf Thread-Slots (37), `fplSignalDestroy` gegen `close` (11), `fplConditionDestroy`/`fplMutexDestroy` (8), `fplAtomicStoreU64`/`LoadU32` gegen nicht atomare Zugriffe.
- **36-mal „unlock of an unlocked mutex“** aus `fplConditionWait`: FPL oder der Test ruft `pthread_cond_wait` ohne gehaltenes Mutex.
- **Rennen im Test**, die in Wahrheit FPL-Synchronisation betreffen: Der Hauptthread schreibt Speicher, den ein bereits gestoppter Worker gelesen hat, und TSan sieht dazwischen nur „as if synchronized via sleep“. Das Warten (`fplThreadWaitForAll`/`One`, Poll-Schleife über Zustand und ID) stellt also **keine Happens-before-Beziehung** her. Auf x86 fällt das kaum auf, auf ARM wäre es ein echter Fehler.

### 2.2 Alt (auch in `master`)

| ID | Befund | Stand |
|---|---|---|
| B1 | `FPL_CRASH_ON_ERROR`/`FPL_CRASH_ON_WARNING` sind verdreht: `level >= minErrorLevel` bei Critical=0, Error=1, Warning=2. Dadurch stürzt `CRASH_ON_ERROR` bei Warnungen und Infos ab, bei Critical nie. Richtig ist `<=`. | geprüft |
| B2 | GLX: Im Zweig „No visual info or frame buffer config defined!“ springt der Code zu `failed_x11_glx`, und das nie gesetzte `legacyRenderingContext` landet in `glXDestroyContext`. | geprüft |
| B3 | **MinGW gilt nicht als GCC.** `FPL_COMPILER_MINGW` fällt an diesen Weichen in den Fallback: `fpl__m_Asm`, `fpl__MinAlignment`, `fpl__m_AlignAs` (Ausrichtung wirkt nicht), `fpl__m_force_inline`/`fpl__m_no_inline`, `#include <cpuid.h>`, CPUID/XGETBV/RDTSC und die Warnungs-Pragmas am Anfang und Ende (~11709, ~45479). Die Folgen: `fpl_force_inline` wird zu einem nackten C99-`inline` (Linkfehler bei `-O0`, geprüft), und `fplCPUGetCapabilities` meldet in MinGW-Builds keine SIMD-Stufen. | geprüft |
| B4 | Win32 `fplTimestampQuery`/`fplTimestampElapsed` hängen am Init-Zustand. Ohne `fplPlatformInit` fallen sie auf `GetTickCount64` zurück, mit etwa 15,6 ms Auflösung auf echtem Windows. Ein vor dem Init genommener Zeitstempel wird danach als QPC-Wert verrechnet. | geprüft |
| B5 | 32-Bit-Linux mit Vulkan kompiliert nicht: Nicht dispatchbare Handles (`uint64_t` auf 32 Bit) werden mit `fpl_null` verglichen und gesetzt, `surfaceKHR` wird nach `void *` gecastet. | teils geprüft |
| B6 | `FPL_NO_MEMORY_MACROS` auf Linux: `memset`/`memcpy` ohne `<string.h>`. | Agent |
| B7 | `FPL_API_AS_PRIVATE` in C++: `fpl__m_platform_api` wird zu `extern "C" static`, das ist ungültig. | geprüft |
| B8 | `FPL_NO_RUNTIME_LINKING` + `FPL_NO_PLATFORM_INCLUDES` in C++: Dem X11/GLX-Extern-Block fehlt `extern "C"`, die Symbole werden gemangelt und der Link scheitert. XRandR, Xinerama und XInput2 haben es. `develop` bringt 23 weitere betroffene X11-Symbole. | geprüft |
| B9 | `FPL_NO_RUNTIME_LINKING` mit echten PipeWire-Headern im Include-Pfad: C scheitert an `pw_thread_loop` ohne `struct`, C++ an Callback-Umwandlungen. | Agent |
| B10 | `FPL_NO_RUNTIME_LINKING` bindet `<vulkan/vulkan.h>` ohne `fplHasInclude`-Prüfung ein. MinGW ohne Vulkan-SDK kompiliert dann nicht. | teils geprüft |
| B11 | Ungeprüfte Allokationen: POSIX `fplMemoryAllocate` prüft `mmap` nicht auf `MAP_FAILED`, `fpl__ParseTextFile` prüft `line` nicht, `fpl__X11LoadWindowIcon` prüft `data` nicht. | geprüft |
| B12 | `FPL_NO_INPUT_RAWINPUT` wirkt nicht: Der relative Mausmodus nutzt Raw Input immer. `FPL_NO_INPUT_UNIX_GAMEPAD` schaltet ein Backend ab, das es nicht gibt. | geprüft |
| B13 | Doku: Diese Schalter fehlen: `FPL_NO_MEMORY_MACROS`, `FPL_FORCE_MEMORY_MACROS`, `FPL_NO_UNDEF`, `FPL_LOG_KEY_EVENTS`, `FPL_DLLEXPORT`/`FPL_DLLIMPORT`, `FPL_MAX_THREAD_COUNT` und `FPL_NO_INPUT_UNIX_GAMEPAD`. C++11 ist als Minimum genannt, aber C++98 scheitert mit 48 kryptischen Fehlern statt einem `#error`. | Agent |
| B14 | FPL-eigene Warnungen bei `-Wall -Wextra`: `-Wmissing-braces`, `-Wsign-compare`, `-Wtype-limits`, gesetzt aber unbenutzt (`eventsResult` in der POSIX-Signalwarte, `deviceIndex` in der ALSA-Geräteliste), `-Wignored-qualifiers`, `-Wmissing-field-initializers` (`padding`, C++), interne Funktionen mit externer Bindung (`fpl__PosixThreadProc`, `fpl__PosixWriteFileBlock`, `fpl__Win32MessageProc`, XInput-Stubs, `FPL__IID_IDirectSoundNotify_Guid`) sowie unbenutzte Parameter und Variablen unter MinGW, weil dort die Pragmas fehlen (B3). | Agent |
| B15 | `-Wpedantic` (C99): `__FUNCTION__`, Casts zwischen Objekt- und Funktionszeigern (dlsym/GetProcAddress), variadische Makros ohne Argument, `;` nach `FPL_ENUM_AS_FLAGS_OPERATORS(...)`, Typedef-Neudefinition (`fplProcessHandle`, `fpl__ProcessStreams`) und anonyme Structs/Unions in öffentlichen Typen (C11). | Agent |
| B16 | **ThreadSanitizer unbrauchbar:** FPL holt `pthread_create` und die übrigen pthread-Funktionen per `dlsym` aus dem Handle von `libpthread.so(.0)` und umgeht damit die Interceptoren von TSan. Neue Threads laufen dann ohne TSan-Zustand, `FPL_Test` unter TSan stirbt im ersten Thread mit `SEGV on unknown address 0x18`. Andere Interposer (Profiler, `LD_PRELOAD`-Werkzeuge) umgeht FPL genauso. Lösung: zuerst über `dlsym(RTLD_DEFAULT, …)` auflösen, das Bibliotheks-Handle nur als Rückfall. Gefunden am 2026-09-28 in Iteration 0. | geprüft |

### 2.3 Fehler in `FPL_Test` (alt)

| ID | Befund | Stand |
|---|---|---|
| C1 | `TestMemory`, drei Schleifen: `uint8_t value = *mem++;`, danach `fplMemoryFree(mem)` mit einem um 100 Bytes verschobenen Zeiger. UBSan meldet einen falsch ausgerichteten Zugriff, `munmap` scheitert still, der Speicher leckt. | geprüft |
| C2 | `SyncThreadsTestSemaphores`: Das Semaphor startet mit `numWriters - 1` statt 1, bis zu 30 Threads sind gleichzeitig im ungeschützten Lesen-Ändern-Schreiben. Die Folge ist ein Datenrennen, unter ASan einmal fehlgeschlagen („expected int32 1, got 2“). | geprüft |
| C3 | `TestTimes` läuft ohne `fplPlatformInit` und scheitert unter Windows und Wine an B4 (gemessen 0,485 s für 500 ms). | geprüft |
| C4 | `process_tests.c` (~514): Beim Start eines Verzeichnisses (`C:\Windows`) wird `AccessDenied` oder `FailedToStart` erwartet. Wine liefert `ERROR_FILE_NOT_FOUND` (2), echtes Windows vermutlich `ERROR_ACCESS_DENIED` (5), POSIX `EACCES`. | teils geprüft |

---

## 3. Designentscheidungen

Jeweils mit Empfehlung. Die offenen Punkte stehen gesammelt in 7.2.

### 3.1 MinGW als GCC-kompatibel (B3)

- `FPL_COMPILER_MINGW` bleibt als öffentliche Kennung erhalten, an jeder der Weichen aus B3 kommt `|| defined(FPL_COMPILER_MINGW)` dazu. Kein neues internes Sammel-Makro, damit jede Stelle beim Lesen eindeutig bleibt.
- clang für MinGW definiert `__clang__` und ist schon `FPL_COMPILER_CLANG`.
- Die Pragmas für MinGW sind die GCC-Pragmas.
- Danach meldet `fplCPUGetCapabilities` unter MinGW SSE/AVX. Das ist ein Prüfpunkt, weil der Viewer darauf seine SIMD-Stufe wählt.
- **Folge:** `fplAlignAs` wirkt unter MinGW erstmals. Das ändert das Layout ausgerichteter Structs zwischen alten und neuen MinGW-Builds, ist also ein Changelog-Eintrag unter „Changed“.

### 3.2 Win32-Zeitmessung ohne Init (B4, C3)

- `fplTimestampQuery` misst immer mit `QueryPerformanceCounter`. Die Frequenz holt `fplTimestampElapsed` einmal lazy über `QueryPerformanceFrequency` in eine interne statische Variable. Das Rennen beim ersten Aufruf ist harmlos, weil beide Threads denselben Wert schreiben.
- Seit Windows XP scheitert das nicht mehr, der `GetTickCount64`-Zweig entfällt.
- Das öffentliche Feld `fplTimestamp.win32.ticks` bleibt aus Kompatibilität stehen, wird aber nicht mehr gefüllt. Die Doku sagt das.
- `TestTimes` bleibt **ohne** Init und ist damit der Regressionstest für diesen Fix.

### 3.3 `fpl_extern_inline` und `FPL_MULTI_TRANSLATION_UNIT` (A2)

Beides ist in v1.0.1 neu und noch unveröffentlicht, also frei änderbar.
- **Empfehlung:** In C ist `fpl_extern_inline` immer `static inline`, das ist sicher in jeder Anzahl von TUs. `FPL_MULTI_TRANSLATION_UNIT` entfällt samt Changelog-Zeile.
- Die Doku-Notiz „every translation-unit … must define FPL_MULTI_TRANSLATION_UNIT“ wird gestrichen, weil sie für FPL selbst nie galt.
- Falls #183 ausdrücklich `extern inline` für Unity-Builds wollte, gibt es die Gegenvariante: Standard `static inline`, und ein Schalter `FPL_EXTERN_INLINE` wählt `extern inline`.

### 3.4 Thread-Erzeugung, Slot-Wiederverwendung und Warten (A6, A7)

**Befund am Code (2026-09-28), Ursache von A6 sehr wahrscheinlich:**
- **Veröffentlichungsrennen:** Unter Win32 und POSIX läuft der neue Thread schon, **bevor** der Ersteller `isValid`, `id` und unter Win32 das Handle in den Slot schreibt: `CreateThread` ohne `CREATE_SUSPENDED` bzw. `pthread_create`, danach erst die Zuweisungen. Ein kurzer Thread kann schon fertig und sein Slot neu vergeben sein (`fpl__GetFreeThread` nimmt jeden `Stopped`-Slot). Der erste Ersteller überschreibt dann `id`/`isValid`/Handle des neuen Bewohners.
- **Win32 schließt ein fremdes Handle:** `fpl__Win32ThreadProc` liest beim Beenden `internalHandle.win32ThreadHandle` und ruft `CloseHandle`. Bei einem wiederverwendeten Slot steht dort, falls der Ersteller noch nicht geschrieben hat, das längst geschlossene Handle des Vorgängers. Windows vergibt Handle-Werte neu. `CloseHandle` kann so ein fremdes Objekt schließen, etwa einen gerade startenden Thread, was zum Wine-Absturz passt.
- **Handle als Slot:** `fplThreadTerminate`, `fplGetThreadState` und das Warten arbeiten auf dem Slot, den der Aufrufer weiter in der Hand hält, auch wenn der Slot längst einem anderen Thread gehört. `fplThreadTerminate` kann so einen fremden Thread beenden.
- **Nicht atomare Zustandswechsel:** `thread->currentState = fplThreadState_Starting` im Ersteller, `thread->isValid = false` im Thread. TSan sieht beim Warten keine Happens-before-Kante (A7).

**Lösung (Umsetzung in Iteration 2, danach Stress als Beleg):**
- Alles, was der Thread oder ein Wartender liest, steht im Slot, **bevor** der Thread läuft:
  - **Win32:** `CREATE_SUSPENDED`, dann Handle, ID und `isValid` schreiben, dann `ResumeThread`.
  - **POSIX:** Der Ersteller schreibt vor `pthread_create`. Die `id` setzt der Thread selbst als Erstes (`pthread_self`), der Ersteller schreibt danach nichts mehr in den Slot.
- **Das Handle schließt nicht der Thread selbst**, sondern wer den Slot neu vergibt (`fpl__GetFreeThread` unter dem Lock), `fplThreadTerminate` oder das Plattform-Release. Ein Handle wird so genau einmal geschlossen.
- **Generationszähler je Slot** (`uint32_t`, atomar), beim Neuvergeben hochgezählt. Wartende und `fplThreadTerminate` prüfen die Generation, die zum Zeitpunkt der Erzeugung galt. Dafür hält der Slot die Generation und `fplThreadHandle` bleibt, wie er ist. Die öffentliche API ändert sich nicht, der Vergleich läuft über die ID des Threads, die nun eindeutig pro Generation ist.
- Alle Zustandsübergänge laufen atomar mit Release/Acquire (Store im Thread, Load im Wartenden), damit TSan und schwach geordnete CPUs die Kante sehen.

Die frühere Idee, nur einen Generationszähler statt der ID einzuführen, bleibt Teil der Lösung, reicht allein aber nicht.
- Jeder Thread-Slot bekommt einen `uint32_t`-Generationszähler, der beim Wiederverwenden eines Slots atomar hochzählt.
- `fpl__WaitForThreadsStopped` merkt sich die Generation statt der OS-Thread-ID.
- `thread->id` wird unter Win32 atomar geschrieben (`fplAtomicStoreU64`).
- Die Thread-Zeiger lädt die Funktion **einmal** in ein lokales Array. Das beseitigt nebenbei A3.

### 3.5 Crash bei Fehlern (B1)

- Die Bedingung wird `level <= minErrorLevel`: `CRASH_ON_ERROR` stürzt bei Critical und Error ab, `CRASH_ON_WARNING` zusätzlich bei Warning.
- Der absichtliche Nullzeiger-Schreibzugriff wird zu `fplDebugBreak()` oder `__builtin_trap`. Ein Schreiben nach Adresse 0 ist undefiniertes Verhalten und darf wegoptimiert werden. Über die genaue Form wird entschieden, wenn die Stelle angefasst wird (7.2).

### 3.6 Raw Input und Unix-Gamepad-Schalter (B12)

- **Empfehlung:** `FPL_NO_INPUT_RAWINPUT` schaltet den Raw-Input-Pfad des relativen Mausmodus ab. Der Modus fällt dann unter Win32 auf das Zurücksetzen zur Mitte zurück, wie X11 ohne XI2, mit beschleunigten Deltas.
- Das ist neuer Code. Wenn er zu groß wird, wird der Schalter stattdessen als „reserviert, derzeit ohne Wirkung“ dokumentiert (7.2).
- `FPL_NO_INPUT_UNIX_GAMEPAD` wird als reserviert dokumentiert, bis es ein Backend gibt.

### 3.7 Prozessstart eines Verzeichnisses (C4)

- FPL vereinheitlicht das Ergebnis: Ist der Pfad ein Verzeichnis, liefern Win32 (`GetFileAttributesW`) und POSIX `fplProcessResultType_AccessDenied`. So verhält sich `execve` unter POSIX ohnehin.
- Der Test bleibt streng.
- Vorher prüft der Nutzer, was echtes Windows heute liefert (Handprüfliste 4.4).

### 3.8 Umfang der Warnungsbereinigung (B14, B15)

- **Empfehlung:** Ziel sind **0 FPL-eigene Warnungen bei `-Wall -Wextra`** in der Standardkonfiguration, mit gcc, clang, g++, clang++ und MinGW, jeweils C99 und C++11. Das umfasst alles aus B14.
- Aus B15 werden nur die billigen Punkte bereinigt:
  - `;` nach `FPL_ENUM_AS_FLAGS_OPERATORS`.
  - `__FUNCTION__` wird zu einem internen Makro mit `__func__` in C99.
  - Variadische Makros bekommen ein Pflichtargument.
  - Die Typedef-Neudefinitionen fallen weg.
- Anonyme Structs/Unions bleiben, weil sie Teil der API sind. Die Doku sagt, dass `-pedantic-errors` mit C99 nicht geht (C11 oder GNU-Erweiterungen nötig).

### 3.9 C++98 (B13)

Eine klare Meldung am Anfang: `#if defined(FPL_IS_CPP) && !defined(FPL_IS_CPP11)` führt zu `#error "Final Platform Layer requires C++11 or newer"`.

### 3.10 Prüfstand im Repo oder außerhalb

- **Empfehlung:** Die Skripte kommen nach `tools/fpl_test/` ins Repo, weil dieser Gesamttest vor jedem Release wiederholt wird. Sie schreiben nur nach `demos/immediates/fpl_test_runs/`. Ergebnisse werden nie eingecheckt.
- Die Skripte:
  - `build_all.sh`: alle FPL-Projekte bauen und die FPL-Warnungen nach Quelltext mit einer Basislinie vergleichen.
  - `matrix.py`: Compile-Matrix.
  - `fpltest.sh`: `FPL_Test` in allen Varianten.
  - `smoke.sh`: Demos auf Xvfb.
- **Audio-Demos:** Sie laufen stumm über eine vorübergehend geladene PipeWire-Null-Senke (`pactl load-module module-null-sink`, danach `unload-module`). Die Streams werden per `PULSE_SINK` bzw. Ziel-Node dorthin gelenkt.
- Vor dem ersten Lauf wird mit `pactl list sink-inputs` geprüft, dass wirklich nichts auf den echten Ausgang geht. Bei Wine-Audio genauso.

---

## 4. Prüfstand

### 4.1 Builds

- `build_all.sh`: alle 42 FPL-Projekte, `-Wall -Wextra`, Debug, gegen die Basislinie von `develop` 08e6c56a aus Iteration 0.
- Verglichen werden Build-Ergebnis und FPL-Warnungen nach Flag und Quelltext der Zeile, weil die Zeilennummern wandern.

### 4.2 Compile-Matrix

Gekürzt auf die Konfigurationen, die ein Befund berührt, plus die Standardkonfiguration mit allen Compilern. Der volle Lauf kommt nur in Iteration 8.

### 4.3 Laufzeit

- **`FPL_Test`** in diesen Varianten:
  - gcc Debug, clang `-O2`.
  - gcc mit ASan, UBSan und LSan.
  - **gcc mit TSan (neu)**, für die Thread-Tests.
  - MinGW x64/x86 unter Wine, `-O2` und nach B3 auch `-O0`.
- **Thread-Stress:** Schleife über `ThreadStorageTest` und `ThreadConcurrentCreateTest`, je 100 Runden auf Linux, Wine64 und Wine32.
- **Rauchtest:** alle lauffähigen Demos auf Xvfb, je etwa 5 s. Geprüft werden Exit-Code, Screenshot, GL-Fehler (`MESA_DEBUG=1`) und Sanitizer-Meldungen.

### 4.4 Handprüfliste Windows (für den Nutzer, MSVC)

- `FPL_Test` x64 und x86, Debug und Release, vollständig durchlaufen lassen. Insbesondere `TestTimes` (ohne Init) und den Prozessstart eines Verzeichnisses (C4) prüfen.
- Thread-Stress x86: 50 Runden.
- `FPL_Window`, `FPL_OpenGL`, `FPL_Vulkan`, `FPL_Input` und `FPL_InputGrab` kurz starten.
- `FPL_Test` einmal mit `FPL_CRASH_ON_ERROR` bauen und prüfen, dass ein absichtlicher Fehler abstürzt und eine Warnung nicht.

---

## 5. Iterationen

Jede Iteration endet mit einem sauberen Build aller Projekte (4.1) ohne neue FPL-Warnungen und mit den Laufzeittests, die ihre Befunde berühren. Die Changelog-Zeilen kommen in den offenen Abschnitt v1.0.1, ohne neue Versionsstufe. Sie sind kurz: `- Fixed: …` / `- Changed: …`.

### Iteration 0 — Prüfstand und Basislinie

- Skripte nach 3.10 (Ort nach Entscheidung 7.2).
- Basislinie von `develop` 08e6c56a festhalten: Build-Ergebnis und FPL-Warnungen je Projekt, die Matrix-Konfigurationen aus 4.2 und die Laufzeiten von `FPL_Test`.
- Die Thread-Stress-Schleife (A6) wird früh gestartet, weil sie lange läuft.
- **Abnahme:** Die Basislinie reproduziert die Zahlen vom 2026-09-28: 42 Projekte, `FPL_NoCRT` scheitert, dazu die Warnungszahlen je Projekt aus dem Gesamttest.

**Stand (2026-09-28):**
- Prüfstand unter `tools/fpl_test/` (0.4), Basislinie `baseline` von `develop` 08e6c56a.
- `build_all`: 42 Projekte, nur `FPL_NoCRT` scheitert. Die Warnungszahlen je Projekt sind identisch mit dem Gesamttest (etwa `FPL_Test` 19, `FPL_Window` 99, `FPL_Emulator` 961). Es gibt 56 verschiedene FPL-Warnstellen.
- `matrix`: 135 Jobs, 12 scheitern, jeder an einem bekannten Befund:
  - B3: MinGW `-O0` x86/x64, `MatrixInlineHelper` ohne Definition.
  - B5: `gcc -m32` C und C++.
  - B6: `FPL_NO_MEMORY_MACROS` C und C++.
  - B7: `FPL_API_AS_PRIVATE` C++.
  - B8: `NO_RUNTIME_LINKING` + `NO_PLATFORM_INCLUDES` C++ (Link).
  - B9: PipeWire-Header C und C++.
  - B10: MinGW `NO_RUNTIME_LINKING`.
  - B13: C++98.
  - Über alle Jobs gibt es 728 verschiedene FPL-Warnstellen.
- `fpltest`:
  - gcc-debug und clang-o2: bestanden.
  - asan: bestanden bis auf 1 Meldung (C1).
  - tsan: stirbt im ersten Thread (B16).
  - mingw64-o2 und mingw32-o2: scheitern an `TestTimes` (B4/C3).
  - mingw64-o0: Linkfehler (B3).
- `threadstress`: Wine32 ist in Runde 5 abgestürzt (A6). Der Absturzdialog ging dabei noch auf dem Desktop des Nutzers auf, seitdem gibt es das eigene Prefix (0.3). Linux, Wine64 und eine zweite Wine32-Runde laufen noch.

### Iteration 1 — Testsuite zuverlässig machen

- B16: FPL löst die pthread-Funktionen zuerst über `dlsym(RTLD_DEFAULT, …)` auf. Ohne das ist die TSan-Abnahme hier und in Iteration 2 nicht möglich.
- C1: Die drei Schleifen lesen `mem[i]`, freigegeben wird der Originalzeiger.
- C2: Das Semaphor startet mit 1 (gegenseitiger Ausschluss), die erwartete Summe bleibt.
- C4: vorerst nur dokumentieren. Der Fix kommt mit 3.7 in Iteration 3.
- **Abnahme:** `FPL_Test` unter ASan, UBSan und LSan meldet in zehn Läufen hintereinander nichts. TSan läuft durch, ohne im ersten Thread zu sterben. Seine Befunde gehören zu A7.

**Stand (2026-09-28):**
- B16: `fpl__PThreadGetProcAddress` sucht zuerst über `dlsym(RTLD_DEFAULT, …)`, das Bibliotheks-Handle ist nur noch der Rückfall. Die 33 pthread-Funktionen laufen über `FPL__PTHREAD_GET_FUNCTION_ADDRESS(_OPTIONAL)`. Ohne Runtime Linking bleibt alles wie bisher. `FPL_Test` läuft unter TSan jetzt vollständig durch, 120 s.
- C1: Die drei Speicherschleifen lesen `mem[i]`. C2: Das Semaphor startet mit 1.
- Changelog: `- Fixed: [POSIX] ThreadSanitizer and other interposers see the pthread calls` (v1.0.1, Threading) und zwei Zeilen im Changelog von `FPL_Test`.
- Build-Vergleich gegen `baseline`: unverändert, 0 neue FPL-Warnstellen.
- Matrix: dieselben 12 Fehlschläge. Neu sind nur 2 `-Wpedantic`-Meldungen „object pointer to function pointer“ am neuen Makro, dieselbe Klasse wie die vorhandenen `dlsym`-Casts (B15).
- **Abnahme erfüllt:** ASan, UBSan und LSan in 10 Läufen hintereinander ohne Meldung und ohne fehlgeschlagene Assertion. Thread-Stress Linux (Basislinie) 100/100 Runden sauber.

### Iteration 2 — Neu in `develop`, Thread-Absturz (A6) und TSan-Befunde (A7)

1. **A6 eingrenzen:** Zuerst ein reiner Win32-Stress ohne FPL (`CreateThread`/`WaitForSingleObject`/`CloseHandle` mit demselben Muster) unter Wine32. Stürzt der ab, liegt der Fehler bei Wine. Sonst wird geprüft, welches FPL-Verhalten ihn auslöst: `TerminateThread` in `fplThreadTerminate`, die Wiederverwendung von Slots und Handles, die ID als Generationsmarke. Danach Thread-Stress auf Wine32 bis zum Absturz. Beim Hänger mit `winedbg --command "bt all"` alle Threads sichern und feststellen, worauf `fpl__WaitForThreadsStopped` wartet (Slot, Zustand, ID). Parallel TSan auf Linux über die Thread-Tests. Dann den Fix nach 3.4, sofern die Ursache dort liegt. Liegt sie in Wine, wird das mit Beleg festgehalten und der Test auf Wine entsprechend markiert.
2. **A1:** `FPL__FUNC_X11_XDisplayKeycodes` gibt `int` zurück.
3. **A2:** nach Entscheidung 3.3.
4. **A3:** ist mit 3.4 erledigt. Sonst lädt `fpl__WaitForThreadsStopped` die Zeiger einmal in ein lokales Array.
5. **A4:** `fplNotUsed` für die Parameter der leeren Linux-Funktionen, oder die Aufrufe entfallen, falls nichts mehr kommt.
6. **A5:** `Makefile`, `premake5.lua` und der Kommentar in `fpl_noruntimelinking.cpp` bekommen die vollständige Link-Liste. Die Doku zu `FPL_NO_RUNTIME_LINKING` nennt `Xrandr`, `Xinerama`, `Xi` und die Audio-Bibliotheken samt der Regel, dass die Audio-Backends über `fplHasInclude` automatisch dazukommen.

- **Abnahme:**
  - Thread-Stress: je 100 Runden auf Linux, Wine64 und Wine32 ohne Hänger, TSan sauber.
  - `FPL_NoRuntimeLinking` baut mit CMake und Makefile.
  - Keine `-Wcast-function-type`-, `-Wstringop-overflow`- oder `initState`-Warnung mehr.

### Iteration 3 — Kernfehler

- B1 nach 3.5.
- B2: `GLXContext legacyRenderingContext = fpl_null;`.
- B4 nach 3.2.
- B11: `MAP_FAILED` führt zu `fpl_null` samt Fehler. `fpl__ParseTextFile` und `fpl__X11LoadWindowIcon` kehren sauber zurück, wenn die Allokation scheitert.
- C4 nach 3.7.
- **Abnahme:**
  - `FPL_Test` unter Wine64 und Wine32 läuft komplett durch, auch `TestTimes` ohne Init.
  - Ein kleiner Test mit `FPL_CRASH_ON_ERROR` bricht bei einem absichtlichen Fehler ab und bei einer Warnung nicht. Mit `FPL_CRASH_ON_WARNING` bricht er bei beiden ab.
  - `-fanalyzer -fanalyzer-call-summaries` meldet die drei echten Befunde nicht mehr.

### Iteration 4 — MinGW als GCC (B3)

- Alle Weichen aus B3 nach 3.1.
- **Abnahme:**
  - MinGW x64/x86 `-O0` linkt `FPL_Test`.
  - Unter Wine meldet `fplCPUGetCapabilities` SSE2/AVX2 wie der native Build.
  - MinGW zeigt keine der etwa 30 Extra-Warnungen mehr, die durch die fehlenden Pragmas entstanden.
  - `FPL_ImageViewer` für MinGW gebaut: Der Selbsttest wählt unter Wine die AVX2-Stufe.

### Iteration 5 — Konfigurationen

- B5: Die Vergleiche und Zuweisungen der nicht dispatchbaren Vulkan-Handles laufen über `VK_NULL_HANDLE`, `surfaceKHR` über eine Hilfsfunktion, die auf 32 Bit nicht abschneidet. Dazu wird die öffentliche Doku von `surfaceKHR` geprüft.
- B6: `<string.h>` bei `FPL_NO_MEMORY_MACROS` auf Nicht-Windows.
- B7: Mit `FPL_API_AS_PRIVATE` in C++ entfällt `extern "C"`.
- B8: `extern "C"` um den X11/GLX-Extern-Block.
- B9: PipeWire mit echten Headern. Entweder werden die FPL-Typen sauber auf die echten abgebildet, oder die Header werden bei `FPL_NO_RUNTIME_LINKING` bewusst nicht benutzt und FPL deklariert selbst, wie in der Demo. Entschieden wird in der Iteration nach dem, was kleiner ist.
- B10: `<vulkan/vulkan.h>` nur mit `fplHasInclude`. Ohne Header wird Vulkan bei `FPL_NO_RUNTIME_LINKING` mit Log-Hinweis abgeschaltet, wie bei den Audio-Backends.
- B12 nach 3.6.
- B13: C++98-`#error` nach 3.9, fehlende Schalter in die Doku-Tabelle.
- **Abnahme:**
  - Die Matrix-Konfigurationen, die heute scheitern, bauen: `gcc -m32` Standard, `FPL_NO_MEMORY_MACROS`, `FPL_API_AS_PRIVATE` in C++ (ein TU), `NO_RUNTIME_LINKING` + `NO_PLATFORM_INCLUDES` in C++, `NO_RUNTIME_LINKING` mit PipeWire-Headern und MinGW `NO_RUNTIME_LINKING` ohne Vulkan-SDK.
  - C++98 bricht mit genau einer klaren Meldung ab.

### Iteration 6 — Warnungen

- B14 komplett, B15 im Umfang von 3.8.
- Jede `-Wsign-compare`- und `-Wtype-limits`-Stelle wird einzeln gelesen, nicht mechanisch gecastet. Einige sind echte Grenzfälle, etwa `int` gegen `uint32_t` bei Fenstergrößen und Zeitouts.
- **Abnahme:** 0 FPL-Warnungen bei `-Wall -Wextra` in der Standardkonfiguration mit gcc, clang, g++, clang++ und MinGW (C99, C++11). Die 42 Projekte bauen ohne FPL-Warnung.

### Iteration 7 — Doku und Changelog

- Doku zu allen geänderten Schaltern, zur Zeitmessung ohne Init, zur Ausrichtung unter MinGW und zu den Link-Listen.
- Die Changelog-Zeilen von v1.0.1 werden auf Vollständigkeit geprüft.
- **Abnahme:** Die Doxygen-Erzeugung läuft ohne neue Warnungen, sofern sie lokal eingerichtet ist.

### Iteration 8 — Rest-Laufzeittests und Abschluss

- Rauchtest aller Demos auf Xvfb.
- Audio-Demos stumm nach 3.10.
- Volle Matrix, voller Build-Vergleich, `FPL_Test` in allen Varianten.
- Windows-Handprüfliste 4.4 durch den Nutzer.
- Danach wird der Plan in einem eigenen Commit gelöscht, und der Branch geht nach `develop`.
- **Abnahme:** Alles aus 4.1–4.4 ist grün. Offene Punkte stehen in 7.3.

---

## 6. Arbeitsregeln

- **Code-Stil nach `CLAUDE.md`:**
  - benannte Zwischenvariablen, keine magischen Zahlen, keine Umbrüche auf Spaltenbreite,
  - geschweifte Klammern um jeden Bedingungskörper,
  - Kommentare auf Englisch.
- **FPL nur auf diesem Branch**, im Worktree. Nach dem Merge wird `develop` in die Feature-Branches gemergt.
- **Vor jedem Commit:**
  - Build-Vergleich aller Projekte (4.1).
  - Bei API- oder Event-Änderungen jeden Nutzer lesen (`demos/additions/*`, `final_ui*.h`, `FPL_Emulator`, `FPL_ImGui`, `FPL_Input`, FTT …). Das betrifft hier B12 und gegebenenfalls B5.
- **Kein Versionssprung:** v1.0.1 ist auf `develop` noch unveröffentlicht, die Zeilen kommen in diesen Abschnitt.
- **Commits** macht der Nutzer, oder ich auf Zuruf: klein, nach Thema, englisch, eine Zeile. Die Plan-Datei wird vor dem Merge in einem eigenen Commit gelöscht.

---

## 7. Entscheidungen und Folgepunkte

### 7.1 Entschieden am 2026-09-28

| Punkt | Entscheidung |
|---|---|
| Umfang | Alle Befunde aus Abschnitt 2 inklusive der `FPL_Test`-Fehler |
| Branch | `fpl/test-fixes` von `develop`, eigener Worktree |

### 7.2 Offen (Empfehlung in Klammern)

1. `fpl_extern_inline` / `FPL_MULTI_TRANSLATION_UNIT` (3.3: in C immer `static inline`, Schalter streichen).
2. `FPL_NO_INPUT_RAWINPUT` (3.6: Rückfall auf das Zurücksetzen zur Mitte, sonst als reserviert dokumentieren).
3. Umfang der Warnungsbereinigung (3.8: 0 bei `-Wall -Wextra`, von `-Wpedantic` nur die billigen Punkte).
4. Prüfstand im Repo unter `tools/fpl_test/` (3.10: ja).
5. Audio-Tests stumm über eine PipeWire-Null-Senke (3.10: ja) oder kurz hörbar.
6. Form des Absturzes bei `FPL_CRASH_ON_*` (3.5: `fplDebugBreak()` bzw. `__builtin_trap` statt Schreiben nach Adresse 0).

### 7.3 Folgepunkte (nicht in diesem Plan)

- `FPL_NoCRT` unter Linux: Die Demo ist nur für Windows gedacht und gehört aus der Linux-Liste der CMake-Projekte.
- Offene Punkte aus früheren Arbeiten (siehe Speicher `project_pending_fpl_fixes`): Win32-Leak und `fplWaitEvent`. MinGW-CPUID erledigt dieser Plan.

---

## 8. Risiken

- **MinGW-Ausrichtung (B3):** `fplAlignAs` wirkt erstmals. Ausgerichtete Structs ändern ihr Layout zwischen alten und neuen MinGW-Builds. Das spielt nur eine Rolle, wenn Binärteile gemischt werden, und kommt in den Changelog.
- **Thread-Code (A6):** Der Warte- und Slot-Mechanismus ist neu in v1.0.1. Ein Fix auf Verdacht könnte den Fehler verdecken. Deshalb erst reproduzieren und belegen, dann ändern, mit TSan und je 100 Runden Stress als Abnahme.
- **Warnungsbereinigung (B14):** Viele kleine Änderungen über den ganzen Header. Vorzeichenkorrekturen können Verhalten ändern, etwa Vergleiche mit -1 als Sentinel. Das Gegenmittel sind Einzelprüfung, der volle Build-Vergleich und `FPL_Test` in allen Varianten nach der Iteration.
- **Win32-Zeitmessung (B4):** Wer heute Zeitstempel vor dem Init nimmt und danach misst, bekam bisher Unsinn und künftig richtige Werte. Das verbessert das Verhalten, ändert es aber.
- **Crash-Semantik (B1):** Wer `FPL_CRASH_ON_WARNING` benutzt, stürzt heute bei Infos ab, künftig bei Fehlern. Das kommt in den Changelog.
- **Wine ist kein Windows:** Befunde nur unter Wine (C4, A6) werden auf echtem Windows gegengeprüft, bevor FPL ihretwegen geändert wird.
