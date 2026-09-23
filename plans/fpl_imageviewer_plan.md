# Plan: FPL_ImageViewer — Skalierung, Pan & Zoom, final_ui

Ziel: Der `FPL_ImageViewer` wird zur eigenständigen App umgebaut, zu einem schnellen und brauchbaren Bildbetrachter. Dieser Plan legt die **technische Basis** dafür: Bilder werden beim Verkleinern **richtig heruntergerechnet** und beim Vergrößern mit einem umschaltbaren Filter **hochgerechnet**. Man kann **zoomen und verschieben**, ohne dass das mit dem Blättern kollidiert. `final_ui.h` ist **fest eingebaut** und zeichnet vorerst nur eine Info-Zeile. Bilder werden über **austauschbare Loader** gelesen, jeder mit seiner eigenen 128-Bit-Kennung. `stb_image` ist der Standard, und welcher Loader wofür zuständig ist, lässt sich einstellen.

Dieses Dokument beschreibt den **Stand samt Messungen** (1), die **Designentscheidungen** (2), den **inneren Aufbau** (3), die **Testbilder und den Messstand** (4) und die **Iterationen** mit Abnahmekriterien (5). Danach folgen Arbeitsregeln, Offenes und Risiken (6–8).

---

## 1. Stand

### 1.1 Was da ist

`demos/FPL_ImageViewer/fpl_imageviewer.cpp` (~1900 Zeilen, C++11 wegen `R"()"`), v0.5.6:

- Dateiliste aus einem Ordner (optional rekursiv), `.jpg/.jpeg/.png/.bmp`.
- `viewPicturesCapacity` = Preload + 1 Slots (Standard 17) um das aktive Bild herum. Die Lade-Threads (Standard: ein Thread pro Kern, hier 32) holen Aufträge aus einer lock-freien MPMC-Queue und dekodieren mit `stbi_load_from_callbacks`. Texturen werden im Hauptthread angelegt und freigegeben.
- OpenGL 3.3 **Core** mit 16× MSAA, sRGB-Framebuffer, Texturen als `GL_TEXTURE_RECTANGLE`. Dazu ein Legacy-Pfad hinter `FORCE_LEGACY_OPENGL`, der in Iteration 1 entfernt wird (Abschnitt 7).
- Sieben Filter-Shader (`shadersources.h`): Nearest, Bilinear, Bicubic (Triangular, Bell, B-Spline, Catmull-Rom), Lanczos3. Standard ist Bicubic (Triangular). Umschalten mit `T`.
- Darstellung: Das Bild wird eingepasst, wenn es größer als das Fenster ist, sonst 1:1 gezeigt. Dazu kommt eine Vorschau-Leiste mit einem Kästchen pro Slot.
- Tasten: Links/Rechts, Bild auf/ab (±10), Pos1/Ende, `F` Vollbild, `P` Vorschau, `R` neu laden, `T` Filter.

### 1.2 Gemessen

**Neu gemessen in Iteration 0 (2026-09-23).** Die Bildwerte stammen aus `tests/run_scaling_tests.sh` (Abschnitt 4.3): `--render-to` rendert offscreen in ein Framebuffer-Objekt **ohne MSAA**, verglichen wird mit ImageMagick-Referenzen **in linearem Licht** (4.2). Wo der Bildschirm mit seinem 16× MSAA etwas anderes zeigt, steht das dabei, gemessen per `xdotool` und `import -window`. Maschine: Ryzen 9 7950X, RTX 3090, NVIDIA 580, Release-Build. Der vollständige Bericht (483 Zeilen) liegt nach jedem Lauf in `demos/build/FPL_ImageViewer/tests/report.md`. Heute verletzen 378 Zeilen eine Schwelle, genau das soll Iteration 0 festhalten.

**1:1-Treue.** `checker_1px` (1-Pixel-Schachbrett 512×512) genau 1:1, einmal in einem 512×512-Fenster und einmal mittig in 1280×720:

| Filter | offscreen (`--render-to`, ohne MSAA) | Bildschirm (16× MSAA) |
|---|---|---|
| Nearest | 0 / 255 — **exakt** | exakt |
| Bilinear, Bicubic (Triangular, Bell, B-Spline, Catmull-Rom) | **flach 188** — das Schachbrett ist weg | flach 188 |
| Lanczos3 | 0 / 255 — **exakt** | 135 / 225 — verwaschen |

188 ist der sRGB-Wert von linear 0,5. Der Mittelwert wird also **in linearem Licht** gebildet, und die sRGB-Kette aus `GL_SRGB8_ALPHA8` und `GL_FRAMEBUFFER_SRGB` funktioniert, auch im sRGB-Farbanhang des Offscreen-Framebuffers. Das Verwaschen selbst ist ein **Halb-Texel-Versatz**: `a = fract(uv * size)` ist an jeder Pixelmitte 0,5, deshalb mischt jeder dieser Filter bei 100 % zwei Nachbarn zu gleichen Teilen (`shadersources.h:125`, `:146`). **Bei 100 % sieht man heute mit jedem Standardfilter ein unscharfes Bild.** Lanczos3 ist nur am Bildschirm verwaschen, die Ursache ist das MSAA (1.3).

**Aliasing beim Verkleinern.** `zoneplate_2048` wird eingepasst. Gemessen wird die Standardabweichung (8-Bit-Stufen) im Ring von 1,5 × Ausgabe-Nyquist bis 0,95 × Quell-Nyquist (Definition in 4.2). Dort muss ein korrekter Downscale **flach** sein, jede Schwankung ist Moiré. In Klammern steht die ImageMagick-Referenz **desselben** Kernels, korrekt verbreitert:

| Maßstab | Referenz Mitchell | Nearest | Bilinear | Triangular | Bell | B-Spline | Catmull-Rom | Lanczos3 |
|---|---|---|---|---|---|---|---|---|
| 0,5 (→1024) | 8,2 | 90,1 (90,1) | 90,1 (7,5) | 10,9 (3,1) | 16,2 (4,3) | 25,7 (5,2) | 90,1 (10,4) | 72,7 (11,1) |
| 0,35 (→717) | 4,0 | 90,2 (90,2) | 52,8 (4,4) | 13,2 (1,5) | 23,2 (1,9) | 31,3 (2,4) | 76,7 (5,2) | 86,3 (5,5) |
| 0,146 (→299) | **0,9** | 90,2 (90,2) | 60,4 (1,2) | 27,5 (0,2) | 36,1 (0,3) | 42,8 (0,5) | 80,7 (1,3) | 88,3 (1,3) |
| 0,1 (→205) | 0,4 | 89,9 (89,9) | 55,6 (0,7) | 28,6 (0,1) | 38,0 (0,3) | 44,5 (0,0) | 78,5 (0,6) | 86,5 (0,5) |

`zoneplate_4096` liefert dieselben Werte, bei 0,146 bis 0,35 auf ±0,5 und bei 0,1 auf ±4. Schon bei 0,35 aliast **jeder** Filter außer Nearest 9- bis 16-mal stärker als die Referenz desselben Kernels, bei 0,146 50- bis 140-mal. Der Standardfilter (Triangular) kommt bei 0,35 auf 13,2 statt 1,5. Die Kernel greifen immer ±2 Quelltexel ab, egal wie viele Quelltexel auf ein Bildschirmpixel fallen. Das ist der Kern von „es gibt kein richtiges Downscale“. Nearest aliast per Definition, dafür gibt es keine bessere Referenz.

Die Messung vor Iteration 0 kam auf andere Zahlen (8,2 / 1,1 für Mitchell), weil der Ring damals nicht festgelegt war. Die Zahlen oben ersetzen sie.

**Fotos unterscheiden die Filter kaum.** `IMG_8978.JPG` (4032×3024 → 960×720, Maßstab 0,238) gegen die Mitchell-Referenz: **alle sieben Filter liegen zwischen 35,2 und 38,6 dB PSNR**, Nearest 35,4, Catmull-Rom 35,2, Triangular 38,6. Die Screenshot-Messung mit MSAA kam auf 39,7–42,0 dB. Glatte Flächen dominieren die Metrik. Deshalb braucht es **synthetische Testbilder** (Abschnitt 4). Echte Fotos taugen hier nur zur Sichtprüfung.

**Weitere Befunde aus dem Testlauf** (alle mit dem heutigen Zeichenweg):

- `gamma_rows` bei 0,35, linke Hälfte (Soll 188): Triangular 187,9 ± 0,3 (ok), Bell 187,3 ± 8,5, B-Spline 186,2 ± 19,8, Bilinear 175,9 ± 58, Catmull-Rom und Lanczos3 ≈ 167 ± 77. Gemittelt wird in linearem Licht, aber zu schmal, deshalb bleiben die Zeilen stehen.
- `color_checker_1px` bei 0,35: Triangular und Bell treffen (188, 188, 0), Catmull-Rom und Lanczos3 kommen auf (179, 179, 0).
- `impulse` ×8: Asymmetrie (PAE gegen die gespiegelte Fassung) Bilinear 241, Catmull-Rom 253, B-Spline 123, Lanczos3 39 Stufen. Der Halb-Texel-Versatz wirkt also auch beim Vergrößern.
- `pixelart_32` mit Nearest bei ×1, ×2, ×3, ×4, ×8: **byteidentisch** zu `-filter Point`.
- `border_frame_odd` bei 0,35: Bilinear verliert die **obere Rahmenkante** (Rotwert 1 statt 140 in der Referenz), Lanczos3 schwächt oben und unten auf 62 statt 150.
- `alpha_disk` mit Lanczos3: **7,5 dB**, die transparenten Bereiche erscheinen knallgrün (1.3).

**CPU-Kosten.** Dekodieren und LOD-Erzeugung mit dem vorhandenen `stb_image_resize` v0.96 (sRGB-korrekt, Mitchell), Median aus 7 Läufen, `gcc -O2`:

| | 4032×3024 JPEG | 2154×1108 PNG |
|---|---|---|
| `stbi_load` v2.19 | 40,1 ms | 30,9 ms |
| `stbi_load` v2.30 | 41,4 ms | **23,2 ms** |
| LOD-Stufe 1 (½) | 137,8 ms | 27,4 ms |
| Stufen 2–4 | 34,8 + 8,8 + 2,3 ms | 6,9 + 1,8 + 0,5 ms |
| **Summe LOD** | **183,5 ms = 4,4× Dekodieren** | 36,6 ms |

LOD per `stb_image_resize` würde die Ladezeit eines Fotos verfünffachen. Deshalb bekommt die LOD-Erzeugung einen eigenen SIMD-Reducer (2.3). v2.30 entpackt PNG 25 % schneller, JPEG bleibt gleich.

**Speicher.** Ein 12-MP-Bild belegt als RGBA8 48,8 MB. 17 Slots ergeben **829 MB VRAM**, mit voller Mip-Kette (+33 %) **1,1 GB**. 16× MSAA kostet bei 4K allein für den Farbpuffer 531 MB und hilft dem Bildinneren nicht, es **schadet** ihm sogar: Lanczos3 ist bei 1:1 nur mit MSAA verwaschen (siehe oben). MSAA glättet Geometriekanten, und ein bildschirmfüllendes Rechteck hat keine.

### 1.3 Befunde im Code

- Halb-Texel-Versatz in Bilinear und allen Bicubic-Filtern (siehe oben). Lanczos3 berechnet die Texelmitte selbst und ist offscreen bei 1:1 exakt. **Im Fenster verwäscht ihn das 16× MSAA** (135/225, in Iteration 0 per Gegenprobe gefunden). Vermutlich wertet der Treiber die an der Pixelmitte unstetige Texelmitten-Rechnung (`mod(uv / texel, 1.0)`) für mehrere Abtastpunkte aus. Mit dem MSAA-Ausbau in Iteration 1 ist das erledigt.
- Lanczos3 startet die Summe mit `vec4(0,0,0,1)` (`shadersources.h:306`). Alpha ist danach ≈ 1 + Σw, geteilt durch Σw, also ≈ 1, und **voll transparente Pixel werden deckend**. Bei `alpha_disk` erscheint das versteckte Knallgrün. Der Shader wird in Iteration 2 ersetzt.
- Die CMake-Vorlage aller Demos setzt `CMAKE_CXX_STANDARD` erst **nach** `add_executable`. Die Zieleigenschaft wird aber beim Anlegen übernommen, deshalb bekommt der Viewer kein `-std=` und wird mit GCC 16 als **C++20** übersetzt (daher die `-Wvolatile`-Warnungen). Bei den C-Demos greift `target_compile_features`. Das betrifft alle C++-Demos und gehört nicht in diesen Plan.
- `FPL_Vulkan` baut schon vor dem stb-Update nicht (`fpl_vulkan.c:2083`: `fpl__X11WindowState` hat kein Feld `window`). Das hat nichts mit stb zu tun.
- `GL_TEXTURE_RECTANGLE` (`fpl_imageviewer.cpp:1119`): kann keine Mip-Stufen haben und braucht nicht-normalisierte Koordinaten. NPOT-`GL_TEXTURE_2D` ist seit GL 2.0 Kern.
- `GL_CLAMP` (`:530`) gibt es im Core-Profil nicht mehr → `GL_CLAMP_TO_EDGE`.
- Mip-Gerüst halb fertig und abgeschaltet: `MAX_PICTURE_MIPMAPS = 1` (`:212`), Größen `w / (2 * i)` statt `w >> i` (`:699`), jede Stufe wird aus Stufe 0 statt aus der vorherigen gerechnet (`:697`).
- Die Zweige in `UpdateAndRender` (`:1562–1581`) heißen „Upscaling“ und „Downscaling“, sind aber „Einpassen“ und „1:1“.
- Fortschritt: Die Leseposition läuft bis 1,0, danach setzt `:695` den Wert auf **0,75 zurück**. Bei PNG liest `stbi` erst die ganze Datei und entpackt dann, dann steht der Balken auf 100 %, während die eigentliche Arbeit noch läuft.
- ~~`ParseParameters` (`:882–891`): Das `switch` schickt alles außer `r` und `t` in `default: continue` → **`-p=` und `-f=` werden nie ausgewertet**.~~ Behoben in Iteration 0.
- ~~`LoadPicturesPath` (`:1050`): `startIndex = 0` setzt den Zeiger statt `*startIndex`. Der Fehler bleibt folgenlos, weil der Aufrufer vorbelegt.~~ Behoben in Iteration 0.
- ~~`preloadCount` wird erst **nach** der Verwendung auf gerade gerundet (`:1195`), die Rundung wirkt also nicht.~~ Behoben in Iteration 0. Dazu kommt eine Begrenzung auf die Slot-Anzahl, weil `-p=1000` jetzt, wo `-p` wirkt, sonst über das Feld `viewPictures[256]` hinauslaufen würde.
- Vorschau-Leiste: Sie zeichnet die volle Textur mit dem aktiven Filter in ein Kästchen von ~40 px und aliast deshalb massiv.
- `fui_input_fpl.h`: `fuiFplInputPumpEvents` leert die **ganze** Event-Queue. Der Viewer braucht die Events aber selbst (Drop, Tasten).
- `fui_backend_gl1.h` ist Fixed-Function und läuft auf einem Core-3.3-Kontext nicht. → **`fui_backend_gl3.h` wird gebaut** (Iteration 7).
- `fplX86CPUCapabilities.hasAVX512` prüft nur **AVX512F** (CPUID 7/EBX Bit 16, inklusive XCR0-Prüfung). BW/VL fehlen.
- Auf Apple Silicon setzt FPL nur `FPL_ARCH_APPLE_ARM64` und **nicht** `FPL_ARCH_ARM64` (`final_platform_layer.h:2170–2182`). Das ist kein Fehler, aber eine Falle für jede ARM-Weiche (2.4).

**Aufgefallen und inzwischen eingeplant:** 10 von 40 Stichproben-Fotos aus `202308` tragen EXIF-Orientierung 3 oder 6 und werden deshalb gedreht angezeigt, weil `stb_image` EXIF ignoriert. Das wird in Iteration 3 behoben. Außerdem ist `stb_image` auf v2.19 (2018), seitdem gab es mehrere Sicherheitskorrekturen. Das Update kommt in Iteration 0.

---

## 2. Designentscheidungen

### 2.1 Eine Resample-Pipeline für beide Richtungen

**Entscheidung:** Verkleinern und Vergrößern laufen durch **dieselbe** GPU-Pipeline: separabel in zwei Durchgängen (erst horizontal, dann vertikal), in linearem Licht, mit vormultipliziertem Alpha, über `texelFetch` mit geklemmten Integer-Koordinaten. Beim Verkleinern wird der Kernel um den Verkleinerungsfaktor **verbreitert** („correct downscaling“, wie mpv und ImageMagick es machen):

```
widen   = max(1, 1 / sL)                       // sL = display scale relative to the source level
xL      = (xSource + 0.5) / 2^L - 0.5          // pixel-center convention, level L
taps j  in [ceil(xL - radius*widen), floor(xL + radius*widen)]
weight  = Kernel((j - xL) / widen), normalized so that the weights sum to 1
```

Das entspricht der Resize-Mathematik von ImageMagick: gleiche Pixelmitten-Konvention, gleiche Randbehandlung („Edge“ = Klemmen), Normierung pro Ausgabepixel. Damit lässt sich jedes Ergebnis **direkt gegen ImageMagick messen**.

Warum nicht die Alternativen:

- **Mipmaps + trilinear:** Das ist Box-Filter plus Überblendung zweier Stufen, weich und nicht „richtig gerechnet“. Es bleibt nur für die Vorschau-Leiste.
- **Ein Durchgang 2D:** kostet `(2·r/s)²` Abgriffe pro Pixel, bei s = 0,15 und Mitchell über 700. Zwei Durchgänge kosten `2·r/s + 2·r/s`.
- **Exakte Neuberechnung auf der CPU bei jeder Ansichtsänderung:** Das wäre Latenz beim Zoomen, und eine GPU erledigt so etwas in Bruchteilen einer Millisekunde.

**Nur der sichtbare Ausschnitt** wird gerechnet. Durchgang 1 schreibt in eine RGBA16F-Zwischentextur (Ausgabebreite × benötigte Quellzeilen), Durchgang 2 in eine RGBA16F-Zieltextur in Fenstergröße. Das Ergebnis wird **zwischengespeichert** und pro Frame nur 1:1 auf den Bildschirm kopiert. Neu gerechnet wird nur, wenn sich Maßstab, Versatz, Filter, Fenstergröße oder Bild ändern. Der Speicher ist durch die Fenstergröße begrenzt und nicht durch das Bild.

**Genau 100 % bei ganzzahligem Ursprung** umgeht den Filter und kopiert die Pixel exakt. Das ist Industriestandard, und nicht-interpolierende Kernel (B-Spline, Mitchell) würden sonst selbst bei 1:1 weichzeichnen.

**Negative Keulen** (Catmull-Rom, Lanczos) erzeugen Werte außerhalb von [0, 1]. Die bleiben in RGBA16F erhalten und werden erst am Ende geklemmt, vormultipliziert auf `rgb ≤ a`.

### 2.2 Eine Kernel-Bibliothek, zwei Auswahlen

Alle vorhandenen Filter sind Produkte eindimensionaler Kernel, also separabel. Sie wandern als GLSL-Gewichtsfunktionen in **eine** Tabelle. Verkleinern und Vergrößern haben je **ihre eigene** aktive Auswahl aus derselben Liste:

| Kernel | Radius | interpolierend | Anmerkung |
|---|---|---|---|
| Nearest | 0,5 | ja | Pixelinspektion |
| Box | 0,5 | – | beim Verkleinern = Flächenmittel, ohne Überschwingen |
| Bilinear (Triangle) | 1 | ja | |
| Bicubic (Triangular) | 2 | – | vorhanden, weich |
| Bicubic (Bell) | 2 | – | vorhanden |
| Bicubic (B-Spline) | 2 | – | vorhanden, sehr weich |
| **Mitchell** (B = C = ⅓) | 2 | – | **neu**, ausgewogen |
| Catmull-Rom | 2 | ja | vorhanden, scharf |
| Lanczos3 | 3 | ja | vorhanden, am schärfsten, schwingt über |

**Standard beim Verkleinern: Mitchell.** Das ist der übliche Kompromiss zwischen Schärfe, Aliasing und Überschwingen, und auch `stb_image_resize` nimmt ihn als Standard zum Verkleinern. **Standard beim Vergrößern: Catmull-Rom**, interpolierend und scharf mit wenig Überschwingen. Beide Vorgaben sind Arbeitshypothesen. In Iteration 2 und 5 bekommt der Nutzer Vergleichsausschnitte (Fotos, Bildschirmfotos mit Text, synthetische Bilder) vorgelegt und legt die Standards selbst fest. Text in Screenshots ist dabei der Härtefall für Lanczos-Halos.

`T` schaltet den Filter der Richtung weiter, die **gerade wirkt**, `Shift+T` schaltet zurück. So bleibt die gewohnte Taste erhalten, und es gibt keine neuen Tasten. Titel und Info-Zeile zeigen „↓ Mitchell“ oder „↑ Catmull-Rom“.

### 2.3 LOD: auf der CPU, im Lade-Thread, mit SIMD

**Entscheidung:** Die LOD-Kette entsteht **im Lade-Thread**, wie vorgeschlagen, allerdings mit einem **eigenen SIMD-Reducer** statt `stb_image_resize`. Nach der Messung in 1.2 würde der die Ladezeit verfünffachen.

Wofür die LOD-Stufen gebraucht werden:

1. **Kosten der Resample-Pipeline begrenzen.** Quelle ist die kleinste Stufe, die noch mindestens doppelt so groß wie das Ziel ist. Der Maßstab relativ zur Quelle liegt dann in (¼, ½], die Verbreiterung bleibt ≤ 4 und die Abgriffe pro Achse ≤ 16 (Mitchell) bzw. ≤ 24 (Lanczos3), egal wie groß das Original ist. Der Abstand „mindestens doppelt“ sorgt dafür, dass das Übergangsband des 2:1-Vorfilters über der Nyquist-Grenze des Ziels liegt. So verfälscht die Doppelfilterung das Ergebnis messbar nicht (Abnahme in Iteration 4).
2. **Vorschau-Leiste:** trilinear aus der Kette, billig und ohne Moiré.
3. **Bilder über `GL_MAX_TEXTURE_SIZE`** (hier 32768): Stufen, die nicht passen, werden nicht hochgeladen. Die Basisstufe ist dann die erste, die passt. Es ist derselbe Code, kein Sonderfall.
4. **Schneller erster Eindruck:** Die kleinen Stufen werden zuerst hochgeladen, das Bild erscheint sofort und wird scharf, sobald Stufe 0 da ist.

Stufen entstehen bis die lange Seite ≤ 32 px ist. Bei 4032×3024 sind das 7 Stufen. Die ersten vier (2016, 1008, 504, 252) machen über 99 % der Arbeit aus, die restlichen kosten zusammen weniger als ein halbes Prozent.

**Der Reducer.** Eine Funktion `ReduceHalf` (RGBA8 sRGB → RGBA8 sRGB, Größe `floor(w/2) × floor(h/2)` wie in GL) arbeitet so:

1. **Dekodieren** über eine 256-Einträge-LUT: sRGB8 → linear u15 (0…32767). Danach wird Alpha vormultipliziert. Die LUT-Zugriffe bleiben skalar: 512 Byte liegen im L1, und Gather-Befehle sind auf Zen 4 nicht schneller.
2. **Filtern** in Festkomma: u15-Abtastwerte, Q14-Gewichte (vorzeichenbehaftet, Summe exakt 16384), int32-Akkumulator, runden, `>> 14`, auf [0, a] klemmen. Die Daten liegen planar vor (R, G, B, A getrennt). Dann ist der vertikale Durchgang ein reines Vektor-Multiply-Add über die Zeile, und der horizontale 2:1-Durchgang wird über eine Gerade/Ungerade-Aufteilung zu einer Summe verschobener Vektoren.
3. **Kodieren:** Alpha-Division herausrechnen (Schnellweg für opake Pixel), dann linear u15 → sRGB8 über eine LUT mit 32768 Einträgen (32 KB). Eine kleinere LUT wäre im Dunkeln zu grob.
4. Die nächste Stufe wird aus dem RGBA8-Ergebnis der vorherigen gerechnet, mit **derselben** Funktion. Ein Zeilen-Ringpuffer (Kernel-Höhe × Breite) statt einer vollen Zwischenkopie kostet pro Thread ~260 KB statt ~100 MB. Ob die wiederholte 8-Bit-Quantisierung stört, wird gemessen (Iteration 4).

**Kernel für 2:1:** Kandidaten sind Lanczos2 und Mitchell mit 8 Abgriffen pro Achse. Entschieden wird über die Zonenplatte: Aliasing-Wert und PSNR einer reinen 2:1-Reduktion gegen `magick -filter X -resize 50%` in linearem Licht.

**Ziele** (7950X, 4032×3024, alle Stufen zusammen): AVX2/AVX-512 ≤ 22 ms (≤ 50 % des Dekodierens), SSE2 ≤ 40 ms, skalar deutlich unter den 187 ms von `stb_image_resize`. Das sind Ziele, keine Zusagen. Die gemessenen Werte kommen nach Iteration 4 in Abschnitt 1.2.

**Hochladen:** eine `GL_TEXTURE_2D` mit echten Mip-Stufen `glTexImage2D(level i)`. `GL_TEXTURE_MAX_LEVEL` ist die letzte Stufe, `GL_TEXTURE_BASE_LEVEL` folgt der größten schon hochgeladenen. `glTexStorage2D` ist erst ab GL 4.2 Kern und wird deshalb nicht benutzt. Die Reihenfolge ist klein → groß. Ein Upload-Budget pro Frame kommt nur, wenn die Messung in Iteration 4 zeigt, dass `glTexImage2D` von 48 MB den Frame reißt.

**Alles ist Festkomma**, deshalb muss jede SIMD-Stufe auf jeder Architektur **bitidentisch** zur skalaren Referenz rechnen. Genau das prüft der Selbsttest, und es ist die stärkste Absicherung gegen SIMD-Fehler. Wie die Stufen gewählt und angeordnet werden, steht in 2.4.

### 2.4 SIMD-Architektur: FPL erkennt, eine Tabelle verteilt, ARM ist vorgesehen

**CPU-Erkennung ausschließlich über FPL.** `fplCPUGetCapabilities` liefert `fplCPUCapabilities` mit `type` (`X86` oder `ARM`) und dazu `x86.*` bzw. `arm.*`. Der Viewer liest nur diese Struktur und führt **kein** eigenes `cpuid`, `xgetbv` oder `getauxval` aus. Fehlt ein Flag, wird es **in FPL** ergänzt und nicht im Viewer nachgebaut. Heute betrifft das nur `hasAVX512BW`, und auch nur, falls die AVX-512-Stufe es braucht (siehe unten).

**Aufbau.** Der Treiber `ReduceHalf` (Ringpuffer, Ränder, Zeilenschleife, Abbruchprüfung, LUTs) wird **einmal** geschrieben und kennt keine Architektur. Er ruft die wenigen heißen Zeilenschleifen über eine Funktionstabelle auf:

```c
typedef struct ImagePyramidRowFunctions {
    const char *name;                    // "scalar", "sse2", "avx2", "avx512", "neon"
    SimdLevel level;
    DecodeRowToLinearFunction *decodeRowToLinear;         // sRGB8 RGBA -> planar u15, premultiplied
    FilterRowsVerticalFunction *filterRowsVertical;       // sum of taps rows * Q14 weights
    FilterRowHorizontalHalfFunction *filterRowHorizontalHalf; // 2:1 decimation via even/odd split
    EncodeRowToSRGBFunction *encodeRowToSRGB;             // planar u15 -> sRGB8 RGBA
} ImagePyramidRowFunctions;
```

`SimdLevel` gilt über alle Architekturen: `SimdLevel_Scalar`, `SimdLevel_X86_SSE2`, `SimdLevel_X86_AVX2`, `SimdLevel_X86_AVX512`, `SimdLevel_ARM_NEON`. Jede Architektur registriert ihre Stufen in derselben Tabelle. `ImagePyramidSelectRowFunctions(const fplCPUCapabilities *capabilities, SimdLevel requestedLevel)` nimmt die höchste Stufe, die **übersetzt** ist **und** die FPL meldet. `--simd=` kann nur unter den verfügbaren Stufen wählen. Eine nicht verfügbare Stufe fällt mit einer Log-Zeile auf die nächstniedrigere zurück.

**ARM später** heißt dann: die vier Zeilenfunktionen mit NEON schreiben, eine Tabellenzeile, eine Zeile in der Zuordnung zu `arm.hasNEON`. Am Treiber, an den LUTs, an den Tests und am Viewer ändert sich nichts. Das Datenformat ist bewusst neutral gewählt und nimmt nichts x86-Spezifisches an: planare u15-Zeilen, Q14-Gewichte, int32-Akkumulator.

| | x86 | ARM (Architektur vorbereitet, Umsetzung später) |
|---|---|---|
| Grundstufe | SSE2: auf x64 zur Übersetzungszeit garantiert, auf x86-32 Laufzeitprüfung `x86.hasSSE2` | NEON: auf AArch64 Pflicht, also zur Übersetzungszeit garantiert; auf ARM32 Laufzeitprüfung `arm.hasNEON` |
| Multiply-Add | `pmaddwd` / `vpmaddwd` | `vmull_s16` + `vmlal_s16` |
| Gerade/Ungerade-Aufteilung | Shift und Maske + `packs` (nur SSE2) | `vld2q_s16` trennt direkt beim Laden |
| Weitere Stufen | AVX2 `x86.hasAVX2`, AVX-512 `x86.hasAVX512` | SVE denkbar, nicht geplant |
| Ohne SIMD | skalar, z. B. x86-32 ohne SSE2 | skalar, z. B. ARM32 ohne NEON |

Die x86-Stufen im Einzelnen:

| Stufe | Wann (FPL) | Umsetzung |
|---|---|---|
| AVX-512 | `x86.hasAVX512` | nur **AVX512F**-Befehle (`vpmulld`, `vpaddd`, `vpsrad`, `vpmovzxbd`, `vpmovusdb`), weil FPL nur F meldet. Braucht es doch BW (`vpmaddwd` auf zmm), bekommt FPL `hasAVX512BW` (eigener Commit, eine Patch-Stufe über `develop`). |
| AVX2 | `x86.hasAVX2` | `vpmaddwd`, 16 Lanes à 16 Bit |
| SSE2 | x64 immer, x86-32 `x86.hasSSE2` | `pmaddwd`, **nur SSE2** — kein `packusdw` (SSE4.1), kein `pshufb` (SSSE3) |
| Skalar | immer | Referenzimplementierung, gegen die alle anderen bitidentisch sein müssen |

**Übersetzung.** Es gibt **kein** globales `-mavx2`, `-mfpu=neon` oder `-march=native`. Die AVX-Funktionen tragen `__attribute__((target("avx2")))` bzw. `target("avx512f")` (GCC/Clang). MSVC braucht für Intrinsics kein `/arch`. So läuft dieselbe Binärdatei auf jeder CPU ihrer Architektur. `<immintrin.h>` und `<arm_neon.h>` werden nur innerhalb ihrer Architekturweiche eingebunden.

**Architekturweichen nur über FPL-Makros**, gebündelt an **einer** Stelle:

```c
#if defined(FPL_ARCH_X64) || defined(FPL_ARCH_X86)
#	define IMAGE_PYRAMID_ARCH_X86
#elif defined(FPL_ARCH_ARM64) || defined(FPL_ARCH_APPLE_ARM64) || defined(FPL_ARCH_ARM32)
#	define IMAGE_PYRAMID_ARCH_ARM
#endif
```

**Falle:** Auf Apple Silicon setzt FPL **nur** `FPL_ARCH_APPLE_ARM64` und **nicht** `FPL_ARCH_ARM64` (`final_platform_layer.h:2170–2182`). Eine Weiche, die nur `FPL_ARCH_ARM64` abfragt, schaltet auf einem Mac stillschweigend auf skalar. Deshalb gibt es genau eine Weiche wie oben, und alles Weitere fragt `IMAGE_PYRAMID_ARCH_*`.

**Prüfbar ohne ARM-Hardware.** `--selftest` läuft **ohne Fenster und ohne GL-Kontext**, also vor `fplPlatformInit` mit Video. Ein für AArch64 übersetzter Viewer lässt sich deshalb unter `qemu-aarch64` testen, das hier installiert ist. Der Cross-Compiler (`aarch64-linux-gnu-gcc`) fehlt noch. Schon jetzt läuft auf x86 der **skalare** Pfad über dieselbe Tabelle wie später NEON. Ein Selbsttest mit `--simd=scalar` prüft also genau den Weg, den ein ARM-Build ohne NEON nimmt.

### 2.5 Koordinaten und `ViewTransform`

Heute rechnet der Viewer in einem mittig zentrierten, y-oben-Ortho-System. Maus und `final_ui` rechnen in **Fensterpixeln, y-unten, Ursprung oben links**. Der Viewer stellt auf die zweite Konvention um, dann braucht Pan & Zoom keine einzige Umrechnung.

`ComputeViewTransform(viewState, imageSize, viewportSize) → ViewTransform` ist eine **reine Funktion** ohne GL, und deshalb im Selbsttest prüfbar. Sie liefert:

- `scale`: Bildschirmpixel pro Bildpixel.
- `imageOrigin`: Lage des Bildes im Fenster, **auf ganze Pixel gerundet**. Nur so ist 100 % exakt, und beim Verschieben flimmert nichts.
- `visibleSourceRect`: Diesen Bereich rechnet die Pipeline.
- `sourceLevel`: nach der Regel in 2.3.

**Orientierung (ab Iteration 3):** Die EXIF-Orientierung (1–8) aus `PictureInfo` wird hier als Abbildung der Bildachsen angewendet: eine ganzzahlige 2×2-Matrix aus Drehung um 0/90/180/270° und Spiegelung, dazu ein Versatz. Die Pixel werden **nicht** umkopiert, LOD-Stufen und Texturen bleiben in gespeicherter Lage. `ComputeViewTransform` rechnet mit der **angezeigten** Größe (bei 90°/270° sind Breite und Höhe vertauscht). Die Resample-Pipeline bekommt die Achsabbildung mit, und Durchgang 1 filtert entlang der gespeicherten Achse, die auf die Bildschirm-x-Achse fällt. Die Info-Zeile zeigt die Größe so, wie das Bild angezeigt wird.

### 2.6 View-Modell für Pan & Zoom

```
ViewState {
    ZoomMode zoomMode;        // Fit, ActualSize (100 %), Custom
    float relativeScale;      // Custom only: scale / fitScale
    Vec2f normalizedCenter;   // image point at the viewport center, 0..1 per axis
}
```

Der Maßstab wird **relativ zur Einpassgröße** gespeichert und die Mitte **normiert**. Damit deckt **ein** Modus „Ansicht behalten“ beide Vergleichsfälle aus dem Auftrag ab:

- **Gleich große Bilder:** Die Einpassgröße ist gleich, also ist die Ansicht pixelidentisch.
- **Dasselbe Bild in verschiedenen Auflösungen:** Man sieht denselben Ausschnitt in derselben Anzeigegröße, und die kleinere Fassung wird entsprechend stärker hochgerechnet. Genau das will man sehen, um Qualität zu vergleichen.

`Fit` und `ActualSize` bleiben beim Wechsel **als Modus** erhalten, dann ist 100 % auch beim nächsten Bild 100 %. Dazu kommt ein dritter Modus **„absoluter Maßstab behalten“**: gleiche Bildpixel pro Bildschirmpixel und gleiche normierte Mitte, über verschiedene Auflösungen hinweg. So lassen sich zwei Auflösungen desselben Motivs etwa bei 100 % vergleichen.

**Einstellung:** `ViewPersistence_Reset` (Standard: jeder Wechsel beginnt eingepasst), `ViewPersistence_KeepRelative` oder `ViewPersistence_KeepAbsolute`. `K` schaltet in dieser Reihenfolge durch, auf der Kommandozeile heißt das `--keep-view=relative|absolute`. Titel und Info-Zeile zeigen den Modus.

**Mausrad-Modus:** `WheelMode_Zoom` (Standard: Rad zoomt um den Mauszeiger, `Strg`+Rad ebenso) oder `WheelMode_Navigate` (Rad blättert, `Strg`+Rad zoomt, wie IrfanView und XnView). Umschalten mit `W` oder `--wheel=zoom|navigate`.

**Pixelinspektion:** Ab einer einstellbaren Zoomstufe wird automatisch Nearest benutzt. Das ist standardmäßig **aus**, schaltbar mit `A` (Schwelle dann 400 %) oder `--nearest-from=<Prozent>`.

**Regeln:**

- **Zoom um einen Punkt:** Der Bildpunkt unter dem Mauszeiger bleibt, wo er ist. Bei Tastatur-Zoom ist der Punkt die Fenstermitte.
- **Klemmen pro Achse:** Ist das Bild in einer Achse kleiner als das Fenster, wird es in dieser Achse zentriert. Ist es größer, entsteht beim Verschieben kein Rand.
- **Grenzen:** ½ × Einpassgröße bis 32× (Pixelinspektion).
- **Magnet:** Das Mausrad rastet an „Einpassen“ und „100 %“ ein, wenn ein Rastschritt darüber hinweggehen würde.
- **Stufenleiter** für die Tasten: ⅛, ¼, ⅓, ½, ⅔, 1, 1½, 2, 3, 4, 6, 8, 12, 16, 24, 32, plus „Einpassen“ an seiner Stelle.
- **Mausrad:** Faktor `wheelZoomFactorPerNotch` (Start 1,2) hoch `wheelDelta`. Bruchteilige Deltas vom Touchpad zoomen dadurch stufenlos.
- **Fenstergröße/Vollbild:** `Fit` bleibt `Fit`, `Custom` behält Maßstab und Mitte.
- Beim Wechsel auf ein noch ladendes Bild gilt die behaltene Ansicht, sobald es da ist.

### 2.7 Belegung — ohne Kollision mit dem Blättern

**Links/Rechts blättern immer**, auch gezoomt. Das ist die ausdrückliche Anforderung, deshalb bekommt das Verschieben eigene Wege.

| Aktion | Tastatur | Maus |
|---|---|---|
| Vorheriges / nächstes Bild | ← / → (unverändert, auch gezoomt) | Seitentasten X1 / X2 |
| ±10, erstes / letztes | Bild ↑ / Bild ↓, Pos1 / Ende (unverändert) | – |
| Zoom (Stufenleiter) | `+` / `-` (Haupt- und Nummernblock), `Strg` + `+`/`-` | Mausrad um den Mauszeiger (auch `Strg`+Rad); im Blätter-Modus nur `Strg`+Rad |
| Einpassen | `0` (auch `Strg`+`0`) | Doppelklick: Einpassen ↔ 100 % am Mauszeiger |
| 100 % / 200 % | `1` (auch `Strg`+`1`) / `2` | |
| Verschieben | `Shift` + Pfeile (⅛ Fenster pro Druck, mit Wiederholung) | Linke oder mittlere Taste ziehen |
| Ansicht beim Wechsel: zurücksetzen → relativ → absolut | `K` | |
| Mausrad-Modus Zoom ↔ Blättern | `W` | im Blätter-Modus: Rad = vorheriges/nächstes Bild |
| Automatisch Nearest ab 400 % ein/aus | `A` | |
| Hintergrund: Schachbrett → Schwarz → Grau | `B` | |
| Filter der wirkenden Richtung | `T` / `Shift`+`T` | |
| Dateiname ↔ relativer Pfad | `N` | |
| Nächsten Loader für dieses Bild (2.10) | `L` | |
| Info-Zeile ein/aus (Standard an) | `I` | |
| Vorhanden | `F` Vollbild, `P` Vorschau, `R` neu laden | |

`Strg+0` und `Strg+1` sind die Photoshop- und Browser-Belegung, `+`/`-` und Ziehen zum Verschieben sind überall üblich. Den Doppelklick meldet FPL nicht selbst, er wird über Zeitstempel erkannt (benannte Konstanten für Zeit und Abstand). Die Links/Rechts-Behandlung muss `Shift` ausschließen, sonst blättert `Shift+←` zusätzlich.

### 2.8 `final_ui`: GL3-Backend, Eingabe, Schrift

- **`demos/additions/fui_backend_gl3.h`** (neu), aufgebaut wie `fui_backend_gl1.h` (About, Getting started, Usage, License), aber für **Core 3.3**:
  - Ein kleines Zustandsobjekt `fuiGL3Backend` mit Programm, VAO, VBO, IBO und 1×1-Weißtextur. `fuiGL3Init` und `fuiGL3Release`.
  - `fuiGL3UploadFontAtlas` legt den Atlas als `GL_R8` mit Swizzle `(ONE, ONE, ONE, RED)` an. `GL_ALPHA` und `GL_LUMINANCE_ALPHA` gibt es im Core-Profil nicht. Mit dem Swizzle bleibt es ein einziger Shader `texture × vertexColor`, und ungetexturierte Befehle binden die Weißtextur.
  - `fuiGL3UploadImageRGBA`, `fuiGL3DeleteTexture`.
  - `fuiGL3Render(backend, drawData)`: Vertices und Indizes werden pro Frame gestreamt (`glBufferData` mit Orphaning), Scissor pro Befehl nur bei Änderung wie im GL1-Backend, `FUI_USE_16BIT_INDICES` wird unterstützt.
  - **Zustand:** Das Backend setzt alles, was es braucht, und stellt danach wieder her: VAO, Programm, Puffer, Textur, Blend, Scissor und **`GL_FRAMEBUFFER_SRGB`**. Letzteres wird für die UI **abgeschaltet**, weil `final_ui`-Farben sRGB-Werte sind und wie im GL1-Pfad in sRGB gemischt werden. Sonst wäre die UI im Viewer heller als in jeder anderen Demo.
- **`fui_input_fpl.h`** bekommt `fuiFplInputBeginEvents(bridge)` und `fuiFplInputHandleEvent(bridge, const fplEvent *)`. `fuiFplInputPumpEvents` wird zu Begin + Schleife und bleibt für alle bisherigen Aufrufer unverändert. Der Viewer reicht jedes Event erst an die Bridge und dann an seine eigene Behandlung weiter.
- **Schrift:** Bitstream Vera Sans aus `final_fonts.h` (dieselbe UI-Schrift wie in `FUI_Editor`), gebacken mit `fuiStbttFontBake`, **Bereich U+0020–U+00FF**, damit Umlaute in Dateinamen erscheinen. Alles darüber (z. B. CJK) zeigt das Ersatzzeichen, siehe Abschnitt 7.
- **Fest integriert:** `fuiContext` lebt in `ViewerState`, und jeder Frame läuft durch `fuiBeginFrame` / `fuiEndFrame`. Gezeichnet wird vorerst **nur** über `fuiDrawRect` und `fuiDrawText`, ohne Panels und ohne Widgets. Pan & Zoom fragt schon jetzt `fuiWantsMouse` und `fuiWantsKeyboard` ab. Das ist heute immer falsch, aber sobald Widgets kommen, geht nichts doppelt.

### 2.9 Fortschritt

Der Fortschritt ist **streng monoton** (`max(alt, neu)`) und in feste Phasen mit benannten Gewichten aufgeteilt:

| Phase | Anteil | Quelle |
|---|---|---|
| Lesen + Dekodieren | 0–70 % | Standard ist die Leseposition der `ImageSource` (2.10). Ein Loader, der es besser weiß, übersteuert sie mit `reportProgress`. Beim stb-Loader: JPEG über die Dateiposition (`stbi` dekodiert beim Lesen), PNG über die Dateiposition nur bis 25 %, danach eine unbestimmte „läuft“-Animation im Rest der Phase, weil `stbi` erst nach dem Lesen entpackt |
| LOD-Stufen | 70–95 % | pro Stufe gewichtet nach Ausgabepixeln, Stufe 1 ≈ 75 % der LOD-Arbeit |
| Hochladen | 95–100 % | Hauptthread, pro Stufe |

Die Gewichte werden nach Iteration 4 anhand der Benchmark-Zahlen einmal nachgestellt. Der Balken wandert unter die Info-Zeile (Iteration 7), heute sitzt er genau dort, wo die Zeile hinkommt.

### 2.10 Bild-Loader: austauschbar, erkannt an einer 128-Bit-Kennung

**Entscheidung:** Das Laden läuft über eine Schnittstelle, und `stb_image` wird deren erste Implementierung und der Standard. Weitere Loader können **neue Formate** bringen oder **dieselben Formate anders** lesen, etwa schneller, vollständiger oder zum Vergleich. Jeder Loader trägt eine **feste 128-Bit-Kennung** (UUID v4: einmal zufällig erzeugt, danach nie mehr geändert). Name und Version sind nur Anzeige. Einstellungen, Kommandozeile und Log verweisen auf die Kennung.

Warum eine Kennung statt des Namens:
- Namen ändern sich.
- Zwei JPEG-Loader heißen schnell beide „jpeg“.
- Eine zufällige 128-Bit-Zahl braucht keine zentrale Vergabe und kollidiert praktisch nie.
- Eine Einstellung, die heute einen Loader festlegt, gilt auch nach jeder Umbenennung.

**Der Typ:**

```c
typedef struct ImageLoaderId {
    uint8_t bytes[16];   // RFC 4122 order, byte for byte identical to the textual form
} ImageLoaderId;
```

Die Textform ist `xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx`, mit oder ohne geschweifte Klammern, Groß- oder Kleinschreibung. Dazu kommen `ImageLoaderIdParse`, `ImageLoaderIdFormat` und `ImageLoaderIdIsEqual`. Die Byte-Reihenfolge ist **die Textreihenfolge** und bewusst **nicht** die Windows-`GUID`-Struktur, deren erste drei Felder little-endian liegen. So bedeutet dieselbe Kennung in einer Datei, auf der Kommandozeile und auf jeder Plattform dieselben 16 Bytes. FPL hat nur `fplInputDeviceGuid` (gleiches Layout, aber für Eingabegeräte gedacht) und keine Funktionen zum Parsen oder Formatieren. Deshalb bekommt der Viewer einen eigenen Typ.

**Die Schnittstelle:**

```c
typedef struct ImageLoader {
    uint32_t interfaceVersion;                 // IMAGE_LOADER_INTERFACE_VERSION, an unknown version is rejected
    uint32_t structSize;                       // sizeof(ImageLoader) as the loader was compiled with
    ImageLoaderId id;                          // the 128-bit signature, never changes
    const char *name;                          // display only, e.g. "stb_image"
    const char *version;                       // display only, e.g. "2.19"
    ImageLoaderFlags flags;                    // e.g. ImageLoaderFlags_NotThreadSafe
    const char *const *fileExtensions;         // null terminated list, e.g. ".jpg", ".jpeg"
    ImageLoaderProbeFunction *probe;           // first bytes of the file -> ImageLoaderMatch
    ImageLoaderReadInfoFunction *readInfo;     // header only -> PictureInfo
    ImageLoaderDecodeFunction *decode;         // full decode -> ImagePixels
    ImageLoaderReleaseFunction *releasePixels; // frees what decode returned
    void *userData;
} ImageLoader;
```

- **Quelle statt Datei.** Ein Loader öffnet nie selbst eine Datei. Er bekommt eine `ImageSource` mit `read`, `seek`, `tell`, `size`, `isCanceled` und `reportProgress`. Der Viewer stellt sie über `fplFile*` bereit, und die heutigen `stbi`-Callbacks werden genau zu dieser Quelle. Dadurch funktionieren Fortschritt und Abbruch bei **jedem** Loader gleich. Andere Quellen (Speicher, Archiv) kommen später ohne Änderung an einem Loader aus.
- **Fortschritt.** Standard ist die Leseposition. Ein Loader, der mehr weiß, meldet selbst `reportProgress(source, fraction)`. Das löst das PNG-Problem aus 2.9 für jeden Loader, dem es wichtig ist.
- **Ergebnis.** `ImagePixels { width, height, stride, format, pixels }` gehört dem Loader und wird nach LOD-Erzeugung und Upload über `releasePixels` zurückgegeben, wie heute `stbi_image_free`. `format` ist ein Enum. Nötig ist vorerst nur `ImagePixelFormat_RGBA8_SRGB`, `RGBA16` und `RGBA32F` sind für später reserviert (16-Bit-PNG in voller Tiefe, HDR).
- **`PictureInfo`** kommt aus `readInfo` und damit **vor** dem Dekodieren. Die Info-Zeile zeigt Größe und bpp also schon während des Ladens. Der Loader füllt `width`, `height`, `channelCount`, `bitsPerPixel`, `isPalette`, `formatName` („JPEG“, „PNG“) und `orientation`. Die Orientierung liest der stb-Loader selbst aus dem EXIF-Block (JPEG APP1, siehe 2.5), weil `stb_image` sie nicht liefert.
- **Rückgabe.** `ImageLoadResult` ist `Success`, `Unsupported` (dieser Loader lehnt diese Datei oder Variante ab), `Corrupt`, `OutOfMemory` oder `Canceled`, dazu eine kurze Meldung in einen Puffer des Aufrufers.
- **Threads.** Alle Lade-Threads rufen Loader gleichzeitig auf. Ein Loader muss deshalb **wiedereintrittsfähig** sein und darf pro Aufruf keinen globalen Zustand anfassen. Wer das nicht kann, setzt `ImageLoaderFlags_NotThreadSafe`, und die Registry serialisiert seine Aufrufe über eine eigene Mutex. `stb_image` v2.19 hat globale Schalter (Flip, Unpremultiply) und einen globalen Fehlertext. Der stb-Loader setzt die Schalter **einmal** bei der Registrierung (heute ruft jeder Thread `stbi_set_flip_vertically_on_load` auf) und liest den Fehlertext nur als Hinweis ohne Gewähr. Nach dem Update in Iteration 0 gibt es `STBI_THREAD_LOCAL` (ab 2.26). Dann ist der Fehlertext threadlokal, und die threadlokalen Schalter-Varianten werden genutzt.

**Registry und Auswahl:**

1. **Geordnete Liste** der registrierten Loader, `stb_image` zuerst. Das ist mit „Standard“ gemeint.
2. **Ordnerscan:** Die Vereinigung aller Endungen aller Loader ersetzt das fest verdrahtete `IsPictureFile`. Ein neuer Loader bringt seine Formate also automatisch in die Dateiliste.
3. **Pro Datei** werden die ersten 64 Bytes einmal gelesen, und jeder Loader bewertet sie mit `probe`: `ImageLoaderMatch_None`, `_Extension` (nur die Endung passt) oder `_Signature` (die Magic Bytes passen). Die höchste Bewertung gewinnt, bei Gleichstand die Reihenfolge. Der **Inhalt schlägt die Endung**: Ein PNG namens `.jpg` wird von einem PNG-fähigen Loader gelesen.
4. **Festlegen:** Ein Loader lässt sich global oder pro Endung festlegen. Er wird dann zuerst versucht, unabhängig von der Reihenfolge, aber nur wenn sein `probe` nicht `None` sagt.
5. **Rückfall:** Liefert der gewählte Loader `Unsupported` oder `Corrupt`, wird der nächste passende versucht. Das ist standardmäßig an und über `--no-loader-fallback` abschaltbar. So darf ein eigener Loader ein Format auch nur **teilweise** abdecken und den Rest an stb weitergeben.
6. Jedes Bild merkt sich, **welcher** Loader es gelesen hat. Das steht im Fenstertitel und im Log. `L` schaltet für das aktuelle Bild zum nächsten Loader, der es lesen kann, und lädt neu. So lassen sich zwei Loader an **derselben** Datei vergleichen, nach Ergebnis und nach Zeit.

**Einstellbar** über die Kommandozeile (die Speicherung in einer Einstellungsdatei steht in Abschnitt 7):

| Parameter | Bedeutung |
|---|---|
| `--loader=<Kennung\|Name>` | alle Formate zuerst mit diesem Loader versuchen (Rückfall bleibt) |
| `--loader-for=<Endung>:<Kennung\|Name>` | pro Endung, mehrfach erlaubt |
| `--loader-order=<Kennung>,<Kennung>,…` | Reihenfolge der Registry |
| `--no-loader-fallback` | kein Rückfall auf den nächsten Loader |
| `--list-loaders` | Tabelle aus Name, Kennung, Version und Endungen, dann beenden |

Maßgeblich ist die Kennung. Ein Name wird nur als Abkürzung angenommen, wenn er eindeutig ist, sonst gibt es einen Fehler mit der Liste der passenden Kennungen.

**Einkompiliert, aber DLL-fähig.** Vorerst werden die Loader einkompiliert und beim Start registriert. Die Schnittstelle ist aber reines C, hat `interfaceVersion` und `structSize` und keine C++-Typen. Ein späterer Plugin-Loader über `fplDynamicLibraryLoad` mit genau einer exportierten Funktion `ImageLoaderGetDescriptor` braucht deshalb keine Änderung an der Schnittstelle (Abschnitt 7).

**Zwei kleine eingebaute Loader beweisen die Architektur**, zusätzlich zu stb:

- **PNM/PAM** (`.ppm .pgm .pam`, Magic `P5`/`P6`/`P7`) — ein **neues Format**. Es hat auch über den Beweis hinaus Nutzen: Der Messstand schreibt PAM (`--render-to`), und der Viewer kann seine eigenen Testergebnisse dann direkt zeigen.
- **Referenz-BMP** (nur unkomprimiertes `BI_RGB` mit 24/32 Bit, bottom-up und top-down) — ein **Format, das stb auch kann**. Damit werden das Festlegen (`--loader-for=bmp:…`) und der Rückfall bewiesen: Ein RLE- oder Paletten-BMP gibt `Unsupported`, und stb übernimmt. Wo sich beide überschneiden, muss das Ergebnis byteidentisch zu stb sein. Er **bleibt** in der App registriert, steht in der Reihenfolge aber **hinter** stb. Er ändert also nie etwas, solange man ihn nicht festlegt, und dient Loader-Autoren als lebendes Beispiel.

---

## 3. Innerer Aufbau

Neue Dateien:

| Datei | Inhalt |
|---|---|
| `demos/FPL_ImageViewer/imagepyramid.h` | API, architekturfreier Treiber `ReduceHalf`, LUTs, skalare Zeilenfunktionen, `ImagePyramidRowFunctions`-Tabelle, `ImagePyramidSelectRowFunctions` über `fplCPUCapabilities`, die eine Architekturweiche. Header-only mit `IMAGE_PYRAMID_IMPLEMENTATION`, ohne GL. |
| `demos/FPL_ImageViewer/imagepyramid_x86.h` | Zeilenfunktionen SSE2, AVX2, AVX-512, nur unter `IMAGE_PYRAMID_ARCH_X86` |
| `demos/FPL_ImageViewer/imagepyramid_arm.h` | **später:** Zeilenfunktionen NEON, nur unter `IMAGE_PYRAMID_ARCH_ARM`. In diesem Plan wird nur der Platz vorgesehen, nicht die Datei. |
| `demos/FPL_ImageViewer/resamplepipeline.h` | Kernel-Tabelle, Shader-Erzeugung für horizontal und vertikal, Zwischen- und Zieltexturen, Framebuffer, Cache-Schlüssel |
| `demos/FPL_ImageViewer/viewtransform.h` | `ViewState`, `ViewTransform`, `ComputeViewTransform`, Zoom-/Pan-Operationen, Klemmen |
| `demos/FPL_ImageViewer/imageloader.h` | `ImageLoaderId` mit Parsen und Formatieren, `ImageLoader`, `ImageSource` über `fplFile*`, Registry, Auswahl, Rückfall, Serialisierung für nicht threadsichere Loader |
| `demos/FPL_ImageViewer/imageloader_stb.h` | Standard-Loader über `stb_image` inklusive Header-Blick für `PictureInfo` |
| `demos/FPL_ImageViewer/imageloader_pnm.h` | PNM/PAM-Loader (neues Format) |
| `demos/FPL_ImageViewer/imageloader_bmp.h` | Referenz-BMP-Loader (gleiches Format wie stb, nur unkomprimiert) |
| `demos/additions/fui_backend_gl3.h` | siehe 2.8 |
| `demos/FPL_ImageViewer/tests/` | Testbild-Generator, Testlauf, Auswertung (Abschnitt 4) |

Änderungen an Vorhandenem:

- `ImageData imageData[MAX_PICTURE_MIPMAPS]` wird zu `ImageLevel levels[MAX_PICTURE_LEVELS]` mit `levelCount` und `uploadedLevelCount`. Hinzu kommt `PictureInfo info`.
- `PictureInfo` kommt aus `readInfo` des Loaders (2.10). Der stb-Loader füllt sie aus `stbi_info_from_callbacks` und `stbi_is_16_bit_from_callbacks` und ergänzt sie um einen Header-Blick für PNG (IHDR-Farbtyp 3 → Palette, Bittiefe = bpp) und BMP (`biBitCount`), weil `stbi` allein ein Palette-PNG als 24/32 bpp meldet.
- `LoadPictureThreadProc` kennt stb nicht mehr. Die Reihenfolge ist: Quelle öffnen → `probe` über die Registry → `readInfo` → `decode` (mit Rückfall) → LOD → `ToUpload`. `IsPictureFile` fragt die Registry.
- Filter-Shader aus `shadersources.h` → Kernel-Gewichtsfunktionen in `resamplepipeline.h`. Uniform-Positionen werden einmal ermittelt statt bei jedem Zeichnen (`glGetUniformLocation` läuft heute pro Aufruf). Zwei Sampler-Objekte (nearest, trilinear) ersetzen das `glTexParameteri` pro Frame.
- Hauptschleife: Events → Bridge und Viewer, dann `fuiFplInputBuild`, Update, Bild (Pipeline oder Cache-Kopie), Vorschau-Leiste, Info-Zeile über `final_ui`, `fuiGL3Render`, Flip.
- **Parameter** werden neu geparst. Die alten Kurzformen `-r -t= -p= -f=` bleiben, repariert. Dazu kommen Langformen mit selbsterklärenden Namen:

| Parameter | Bedeutung |
|---|---|
| `--zoom=fit\|100\|<Prozent>` | Startzoom |
| `--center=<u>,<v>` | normierte Startmitte |
| `--down-filter=<Name>`, `--up-filter=<Name>` | Filterwahl |
| `--keep-view=relative\|absolute`, `--wheel=zoom\|navigate`, `--nearest-from=<Prozent>`, `--background=checker\|black\|gray` | Ansicht und Bedienung |
| `--relative-path`, `--no-preview`, `--no-info` | Anzeige |
| `--simd=scalar\|sse2\|avx2\|avx512\|neon` | SIMD-Stufe erzwingen; eine nicht verfügbare Stufe fällt mit Log-Zeile zurück |
| `--lod-source=auto\|0` | Pipeline-Quelle erzwingen, für Referenzvergleiche |
| `--render-to=<Datei.pam> --window=<B>x<H>` | ein Bild offscreen rendern, als PAM schreiben, beenden |
| `--bench-lod=<Datei>` | Dekodieren + LOD je SIMD-Stufe messen (Median aus N Läufen) |
| `--loader=`, `--loader-for=`, `--loader-order=`, `--no-loader-fallback`, `--list-loaders` | Loader-Auswahl, siehe 2.10 |
| `--decode-all=<Ordner>` | dekodiert alle Bilder des Ordners über Registry und Lade-Threads, ohne GL; meldet Fehler, Anzahl und Zeit je Loader |
| `--selftest` | ViewTransform-Mathematik, Bitidentität aller SIMD-Stufen, Kennungen parsen und formatieren, Loader-Auswahlregeln an Byte-Puffern. Läuft **ohne Fenster und GL**, also auch unter `qemu-aarch64`. |

---

## 4. Testbilder und Messstand

Die Filter werden gegen **selbst erzeugte** Bilder validiert, denn echte Fotos unterscheiden sie nachweislich nicht (1.2). Die Bilder sind **eingecheckt** unter `demos/FPL_ImageViewer/tests/images/` (Entscheidung vom 2026-09-23). Der Generator `tests/generate_testimages.sh` liegt daneben, dokumentiert, wie jedes Bild entstanden ist, und baut sie **byteidentisch** nach (keine Zeitstempel, kein Zufall). Nur die Ergebnisse der Testläufe (Renderings, Referenzen, `report.md`) landen in `demos/build/FPL_ImageViewer/tests/`, das ignoriert ist.

### 4.1 Die Bilder

| Bild | Größe | Prüft | Erwartung |
|---|---|---|---|
| `checker_1px` | 512² | 1:1-Treue; Mittelung in linearem Licht | bei 100 % **byteidentisch**; verkleinert flach **188 ± 1**, ohne Moiré |
| `zoneplate` | 2048², 4096² | Aliasing über der Nyquist-Grenze | Ring-Std ≤ Referenz + Toleranz; PSNR gegen dieselbe Kernel-Referenz |
| `siemens_star` | 2048² | Moiré, Sichtprüfung | glatte Grauzone zur Mitte hin, keine Scheinringe |
| `gamma_rows` | 1024² | Gamma-Korrektheit | Links 1-px-Zeilen schwarz/weiß, rechts flach 188 und ein Streifen 128. Verkleinert muss die linke Hälfte der 188 entsprechen und **nicht** der 128. |
| `color_checker_1px` | 512² | Gamma pro Kanal | rot/grün gemischt ≈ (188, 188, 0), nicht das dunkle (128, 128, 0) |
| `impulse` | 33², 1 weißes Pixel | Kernelform und -zentrierung, **ohne** Fremdreferenz | ×8 und ×7 vergrößert: Das Profil der Mittelzeile (linearisiert) entspricht dem abgetasteten Kernel, Maximum exakt mittig, symmetrisch. Jeder Halb-Texel-Versatz fällt auf. |
| `step_edge` | 256² | Überschwingen | Überschwinger je Filter wie in der Theorie: B-Spline/Mitchell ≈ 0, Catmull-Rom klein, Lanczos3 am größten |
| `lines_1px` | 2048² | Linien unter 0°, 7°, 45° und Linienpaare mit 1–8 px Abstand | verkleinert gleichmäßig grau statt gestrichelt; vergrößert gerade, ohne Treppenzittern |
| `text` | ~1200×800 | Halos an Schrift (ImageMagick `-annotate` mit DejaVu Sans, 9–24 pt, hell/dunkel) | Sichtprüfung; entscheidet mit über die Standardfilter |
| `pixelart_32` | 32² | Nearest bei ganzzahligem Zoom | exakt k×k-Blöcke, **byteidentisch** zu `magick -filter Point -resize` |
| `alpha_disk` | 1024², RGBA | vormultipliziertes Alpha | Voll transparente Pixel tragen Knallgrün als RGB. Ein **grüner Saum** nach dem Filtern ist ein Vormultiplikationsfehler. |
| `border_frame_odd` | 1023×767 | Ränder, ungerade Größen, LOD-Kette | 1-px-Rahmen rot, innen grün: bei Einpassen, 100 % und in allen vier Ecken nach dem Verschieben vollständig sichtbar |
| `extreme_aspect` | 40000×64 | Bild über `GL_MAX_TEXTURE_SIZE` (32768) | wird aus einer LOD-Stufe gezeigt, kein Absturz, 10 MB |
| `tiny_1x1`, `tiny_3x2` | | entartete Größen | korrekt, LOD-Kette endet sauber |
| `gradient16` | 4096×256, 16 Bit | Banding in den Zwischentexturen; bpp-Anzeige | kein sichtbares Banding; Info zeigt **48 bpp** |
| `palette8`, `gray8` | 256² | bpp-Anzeige | **8 bpp (Palette)**, **8 bpp** |
| `render_rgba.pam`, `.ppm`, `.pgm` | klein | PNM/PAM-Loader (neues Format) | erscheint im Ordnerscan, Pixel gleich der Quelle |
| `bmp24_bottomup`, `bmp32_topdown` | 257×131 | Referenz-BMP gegen stb | **byteidentisch** |
| `bmp_rle8` | 64² | Rückfall | Referenz-BMP lehnt ab, stb liest; das Log nennt beide |
| `png_named.jpg` | 64² | Signatur schlägt Endung | wird als PNG gelesen |
| `orientation_1` … `orientation_8` | 300×200 JPEG | EXIF-Orientierung | Ein asymmetrisches Motiv („F“ mit Farbecken, `orientation_upright.png`) wird für jede der 8 Orientierungen passend gedreht bzw. gespiegelt **gespeichert** und bekommt ein von Hand gebautes APP1-Segment mit genau dem Orientierungs-Tag, ungerade Werte big-endian (`MM`), gerade little-endian (`II`). `magick -orient` schreibt ohne vorhandenes EXIF-Profil nichts. Gegenprobe: `magick orientation_N.jpg -auto-orient` ergibt für alle 8 das aufrechte Motiv (64 dB). Alle 8 müssen **gleich** angezeigt werden, bei 100 % byteidentisch zur Referenz von `orientation_1` bis auf JPEG-Abweichungen (PSNR-Schwelle), eingepasst, gezoomt und in der Vorschau-Leiste. |
| `truncated.jpg`, `empty.png` (0 Byte) | | kaputte Dateien | Fehlerzustand, kein Absturz, alle passenden Loader versucht |
| `exif_broken.jpg`, `exif_broken_ifd_offset.jpg` | 64² | EXIF-Block mit Offsets und Längen außerhalb des Blocks: 256 angebliche Einträge in einem Block mit einem, Orientierung mit 65536 Werten an Offset 0xFFFFFFF0; bzw. IFD0-Offset weit hinter dem Blockende | wird als Orientierung 1 angezeigt, kein Absturz, kein Lesen über den Block hinaus (unter ASan geprüft) |
| `Übersicht_ä.png` | 320×200 | Umlaute im Dateinamen (Iteration 7) | Info-Zeile zeigt den Namen richtig |
| Fotos `202308`, `Bildschirmfotos` | | echte Motive | Sichtprüfung an vergrößerten Ausschnitten |

### 4.2 Referenzen und Metriken

- **Referenz:** `magick <in> -colorspace sRGB -colorspace RGB -filter <X> -resize <B>x<H>! -background black -alpha remove -alpha off -colorspace sRGB -depth 8 <ref>`, mit gleichem Kernel (`Box`, `Triangle`, `Mitchell`, `Catrom`, `Lanczos`, `Point`, B-Spline = `Spline`) und gleicher Pixelmittenkonvention. Das erste `-colorspace sRGB` ist nötig, damit Graustufen-PNGs vor dem Linearisieren als sRGB gelten. `-alpha remove` mischt Transparenz in linearem Licht auf Schwarz, so wie der Viewer heute zeigt. Die alten Kernel haben doch eine genaue Entsprechung (in Iteration 0 am Impulsprofil geprüft): **Triangular** = `-filter Triangle -define filter:blur=2` (Dreieck mit Radius 2), **Bell** = `-filter Quadratic -define filter:blur=1.3333333333` (quadratischer B-Spline, auf Radius 2 gestreckt). Damit hat jeder heutige Filter eine Referenz.
- **Randbehandlung von ImageMagick:** `-resize` klemmt am Rand **nicht**. Abgriffe außerhalb des Bildes fallen weg, und die restlichen Gewichte werden neu normiert (`resize.c`: `start = max(bisect − support + 0.5, 0)`, `stop = min(…, columns)`, danach durch `density` geteilt). Soll die Pipeline in Iteration 2 exakt vergleichbar sein, macht sie es genauso. Klemmen würde nur die Randpixel verändern, trifft aber genau `border_frame_odd`.
- **PSNR gegen dieselbe Kernel-Referenz: ≥ 45 dB** auf allen synthetischen Bildern. **Kalibriert in Iteration 0:** Die ImageMagick-Referenz als Q16-HDRI-Gleitkomma, auf [0, 1] geklemmt, gegen dieselbe Referenz in 8 Bit liegt über alle Testbilder, Maßstäbe (0,146 … 8) und Kernel (Mitchell, Lanczos3, Triangle, Catmull-Rom) bei **54,4 bis 84,7 dB**. Den Tiefstwert liefern flache Flächen: 187,5 wird zu 188, jedes Pixel liegt also 0,5 Stufen daneben. Das ist die Obergrenze des Erreichbaren. 45 dB lassen 9 dB (RMS ≈ 1,4 Stufen) für die GPU-Arithmetik (RGBA16F, `sin`/`pow`-Näherungen). Ohne Klemmen sinkt der Wert bei Lanczos/Catmull-Rom auf 15–40 dB, das misst aber das Überschwingen und nicht die Rundung. Die Referenz ist deshalb immer das geklemmte 8-Bit-Ergebnis.
- **Nearest** tastet nur ab. Liegt ein Abtastpunkt genau zwischen zwei Quellpixeln (etwa bei 512 → 179 für `x = 89`: 256·(2x+1)/179 ist ganzzahlig), ist die Rundung Sache der Implementierung. Beim Verkleinern ist der PSNR gegen `Point` deshalb nur informativ. Exakt geprüft wird Nearest bei 100 % und bei ganzzahligem Vergrößern, dort gibt es keine Gleichstände.
- **Aliasing-Wert:** Standardabweichung (8-Bit-Stufen) des Rotkanals im Ring über der Nyquist-Grenze der Zonenplatte. Die Zonenplatte `0.5 + 0.5·cos(π·r²/N)` hat die Ortsfrequenz `r/N` Perioden pro Quellpixel. Bei Maßstab `s` liegt die Nyquist-Grenze der Ausgabe im Ausgaberadius `0.5·s²·N`, die der Quelle bei `0.5·s·N`. Gemessen wird von **1,5 × Ausgabe-Nyquist** bis **0,95 × Quell-Nyquist**. Ein korrekter Filter ist dort flach. Der Ring braucht mindestens 4 Pixel Breite, bei 2048² also s ≤ 0,5. Toleranz: **+1,0 Stufen** über der Referenz desselben Kernels. Die 8-Bit-Quantisierung selbst trägt nur ≈ 0,3 Stufen bei (in Quadratur addiert: +0,05 bei einer Referenz von 0,9).
- **Flache Flächen** (`checker_1px` verkleinert): Mittelwert 188 ± 1, Standardabweichung ≤ 1. **`gamma_rows`, `color_checker_1px`:** ± 2. **Impuls:** Asymmetrie (PAE gegen die gespiegelte Fassung) und Abweichung vom abgetasteten Kernel je ≤ 1 Stufe. **Rahmen:** Jede der vier Kanten hat mindestens 50 % des Rotwerts, den dieselbe Kante in der Referenz hat. Das Nearest-Verkleinern ist davon ausgenommen, eine 1-px-Linie darf dabei verschwinden.
- **Exakte Tests** (100 %, Nearest bei ganzzahligem Zoom): `magick compare -metric AE` = 0. ImageMagick 7.1.2 meldet AE als kanalgewichteten Bruchteil, deshalb zählt jeder Wert über 0 als mindestens ein abweichendes Pixel.
- **Bitidentität der SIMD-Stufen:** `--selftest` vergleicht alle verfügbaren Stufen auf allen Testbildern und auf Zufallsbildern (Größen 1…130 × 1…130, Zufallsinhalt mit Alpha) Byte für Byte.

### 4.3 Werkzeuge

- `tests/generate_testimages.sh [Zielordner]`: Erzeugt alle Bilder aus 4.1 mit ImageMagick nach `tests/images/` (≈ 10 s). Das Skript ist deterministisch (zweimal erzeugt = byteidentisch) und hat englische Kommentare. PNG-Farbtypen werden erzwungen (`PNG24:` bzw. Graustufen 8 Bit), weil ImageMagick sonst eigenmächtig auf Palette oder 1-Bit-Grau reduziert.
- `tests/run_scaling_tests.sh [--viewer=] [--images=a,b] [--filters=1,7] [--quick] [--no-photo]`: Für jede Kombination aus Bild, Maßstab (je Bild eine passende Auswahl aus 0,1 / 0,146 / 0,238 / 0,35 / 0,5 / 0,7 / 1 / 1,5 / 2,3 / 4 / 7 / 8, dazu `1@1280x720` = 1:1 im Fenster der Screenshot-Messungen) und Filter rendert es mit `--render-to`, erzeugt die Referenz (zwischengespeichert), misst und gibt eine Markdown-Tabelle aus, zusätzlich als `report.md`. Der Exit-Code ist 1, wenn eine Schwelle verletzt ist. Ein voller Lauf dauert einige Minuten und öffnet pro Rendering kurz ein kleines Fenster.
- `--render-to=<Datei.pam> --window=<B>x<H>` ist der Kern des Messstands: Das Bild wird offscreen in ein Framebuffer-Objekt genau der angegebenen Größe gerendert (sRGB-Farbanhang wie der Standard-Framebuffer), ohne Vorschau-Leiste, ohne Info-Zeile, ohne Fensterdekoration und ohne Compositor, und per `glReadPixels` als PAM (RGB) geschrieben. Das ist deterministischer als jeder Screenshot. Ein Fenster entsteht trotzdem, weil FPL es für den GL-Kontext braucht, im Render-Modus nur 256×256 und **ohne MSAA**. Exit-Codes: 0 geschrieben, 1 Parameter, 2 kein Bild, 3 Laden fehlgeschlagen, 4 Zeitüberschreitung (60 s), 5 GL, 6 Schreiben, 7 Fenster geschlossen. Dazu `--zoom=fit|100|<Prozent>`: `fit` passt auch kleine Bilder ein, also mit Vergrößern.
- **Interaktion** (Tasten, Mausrad, Ziehen, Info-Zeile) wird per `xdotool` gesteuert und mit `import -window <id>` aufgenommen, wie bei den Messungen in 1.2. Nach jeder Zeichenänderung heißt es: Screenshot **und** hineinzoomen.

---

## 5. Iterationen

Die Reihenfolge folgt dem Auftrag: Zuerst die Technik und dort **zuerst das Verkleinern**, dann das Vergrößern, dann die Steuerung, am Ende die UI. Das Verkleinern ist ohne LOD-Stufen schon **richtig**, die LOD-Stufen machen es danach nur schneller und begrenzen die Kosten. Deshalb kommt die Qualität vor der Kette. Die Loader-Architektur steht vor der LOD-Kette, weil beide den Lade-Thread umbauen und die Kette auf dem Ergebnis des Loaders aufsetzt.

### Iteration 0 — Testbilder und Messstand

- **Vorab, eigener Commit:** `demos/dependencies/stb/stb_image.h` von v2.19 auf die aktuelle Version. Die Datei wird gemeinsam genutzt, außer vom Viewer von `final_assets.h`, `final_graphics.h` (und darüber den Spiele-Demos), `FPL_Emulator`, `FPL_Input`, `FPL_OpenGL` und `FPL_Vulkan`. Alle diese Demos werden danach gebaut. Das Update kommt **vor** allem anderen, damit jeder Messwert und die Abnahme „gleiche Pixel wie vorher“ in Iteration 3 schon mit der neuen Version entstehen.
- Die Messungen aus 1.2 werden mit der neuen Version wiederholt und dort ersetzt.
- `generate_testimages.sh` und `run_scaling_tests.sh`. `--render-to` wird vorgezogen und rendert vorerst mit dem **alten** Zeichenweg. Die Parameter werden repariert (`-p`, `-f`), und die Langformen aus Abschnitt 3 kommen dazu, soweit sie schon Sinn ergeben.
- Die Schwellen werden kalibriert (4.2).
- **Abnahme:** Der Testlauf läuft durch und dokumentiert den **heutigen** Stand als Tabelle. Er muss die Zahlen aus 1.2 reproduzieren, also 1:1 flach 188 bei allen außer Nearest und Aliasing bei 0,146.

**Stand (2026-09-23):**
- Erledigt: `--render-to`/`--window`/`--zoom`/`--no-preview`, Parser neu (`-p`, `-f` wirken, unbekannte oder kaputte Parameter enden mit Exit-Code 1), die drei Kleinfehler aus 1.3, v0.6.0 mit Changelog. Testbilder erzeugt und eingecheckt (4.1), `run_scaling_tests.sh` mit 483 Zeilen, Schwellen kalibriert (4.2), Messungen in 1.2 ersetzt.
- Abnahme erfüllt: 1:1 flach 188 bei Bilinear und allen Bicubic-Filtern, Nearest exakt. Aliasing bei 0,146 für jeden Filter 50- bis 140-fach über der Referenz. Einzige Abweichung von der alten Tabelle: Lanczos3 ist offscreen exakt, das Verwaschen am Bildschirm kommt vom MSAA (1.3).
- `stb_image` v2.30: Die Datei ist geladen und geprüft (Benchmark in 1.2 läuft damit), liegt aber noch **nicht** im Repo, weil das Einspielen fremden Codes eine Freigabe braucht. Danach: alle Nutzer neu bauen (Viewer, Emulator, Input, OpenGL, Vulkan, Software, GameTemplate, Crackout, Towadev, FUI_Test, FUI_Framework; vorher bauten alle außer Vulkan ohne Fehler) und `run_scaling_tests.sh` wiederholen.

### Iteration 1 — Fundament

- `GL_TEXTURE_RECTANGLE` → `GL_TEXTURE_2D`, `GL_CLAMP` → `GL_CLAMP_TO_EDGE`, 16× MSAA weg.
- Das halbe Mip-Gerüst kommt raus (`MAX_PICTURE_MIPMAPS`, `DownsampleImage`, die Auswahlschleife) und mit ihm `stb_image_resize`.
- **Legacy-GL-Pfad entfernen:** `FORCE_LEGACY_OPENGL`, alle Zweige `openGLMajor < 2` und das Fixed-Function-Zeichnen. Die App setzt GL 3.3 Core voraus. Kommt der Kontext nicht zustande, gibt es eine klare Fehlermeldung in Log und Konsole statt eines stillen `return -1`.
- **Leerlauf:** Gezeichnet wird nur, wenn etwas „schmutzig“ ist: Events, Ladefortschritt, Uploads, Fenstergröße, laufende Animationen. Sonst werden Zeichnen und Flip übersprungen, und der Thread schläft kurz (benannte Konstante, etwa 5 ms). FPL kennt kein blockierendes Warten auf Events, siehe Abschnitt 7.
- Umstellung auf y-unten-Fensterpixel; `viewtransform.h` mit `ComputeViewTransform` (vorerst nur `Fit` und `ActualSize`) und Ganzpixel-Rundung des Ursprungs. Die falsch benannten Zweige verschwinden dabei.
- Halb-Texel-Korrektur in den vorhandenen Filtern als Zwischenschritt: `p = uv·size − 0,5`, `base = floor(p)`, Gewichte aus `p − base`. So ist die Iteration für sich schon eine Verbesserung, auch wenn Iteration 2 die Shader ersetzt.
- `--selftest` mit den ersten ViewTransform-Prüfungen.
- **Abnahme:** `checker_1px` bei 100 % **byteidentisch** mit **jedem** interpolierenden Filter (Nearest, Bilinear, Catmull-Rom, Lanczos3). `border_frame_odd` hat bei 100 % und beim Einpassen alle vier Rahmenseiten. Es gibt keine GL-Fehler im Core-Profil. Die Fotos sehen nicht schlechter aus als vorher (Sichtprüfung, Vorher-Nachher-Ausschnitte). Im Leerlauf liegt die CPU-Last des Viewers unter 1 % (`pidstat` über 10 s), während des Ladens und Blätterns läuft er flüssig wie vorher. Kein `FORCE_LEGACY_OPENGL` und kein Fixed-Function-Aufruf mehr im Viewer (per `grep` geprüft).

### Iteration 2 — Richtiges Verkleinern

- `resamplepipeline.h`: Kernel-Tabelle (2.2) mit Box und Mitchell neu, Shader-Vorlage für horizontal und vertikal, verbreiterter Kernel, lineares Licht, vormultipliziertes Alpha, RGBA16F-Zwischen- und Zieltextur, nur der sichtbare Ausschnitt, Cache mit Schlüssel (Bild, Stufe, Maßstab, Versatz, Filter, Fenstergröße).
- Quelle ist vorerst immer Stufe 0.
- **Hintergrund** hinter transparenten Bildern: Schachbrett (Standard, feste Feldgröße in Bildschirmpixeln), Schwarz oder Grau. Umschalten mit `B` oder `--background=`. Das vormultiplizierte Ergebnis wird in linearem Licht darauf gemischt.
- Getrennte Filterwahl für unten und oben, `T` / `Shift+T` wirkt auf die aktive Richtung, der Fenstertitel zeigt Richtung und Filter.
- Ausblick: GPU-Zeit der Durchgänge per `GL_TIME_ELAPSED`-Abfrage, sichtbar im Log.
- **Abnahme:**
  - Zonenplatte bei 0,146 und 0,35: Aliasing-Wert höchstens Referenz + kalibrierte Toleranz (heute 15,9–64,3 bei einer Referenz von 1,1).
  - PSNR ≥ Schwelle gegen dieselbe Kernel-Referenz für Box, Triangle, Mitchell, Catmull-Rom und Lanczos3 auf allen synthetischen Bildern.
  - `gamma_rows` und `color_checker_1px` verkleinert: ± 2 zum Sollwert.
  - `alpha_disk`: kein grüner Saum. Auf Schwarz und Grau stimmt es mit `magick -background <Farbe> -flatten` überein (PSNR ≥ Schwelle), das Schachbrett wird per Sichtprüfung abgenommen.
  - Standardfilter zum Verkleinern: Der Nutzer bekommt Vergleichsausschnitte (`text`, `Bildschirmfotos`, Fotos, Zonenplatte) je Kandidat vorgelegt und legt fest.

### Iteration 3 — Loader-Architektur

- `imageloader.h` nach 2.10: Kennungstyp mit Parsen und Formatieren, Schnittstelle, `ImageSource` über `fplFile*`, Registry mit Reihenfolge, Probe, Festlegen, Rückfall und Serialisierung für `NotThreadSafe`.
- `imageloader_stb.h` als Standard. Der geplante Header-Blick für `PictureInfo` zieht hier als `readInfo` ein, und die globalen stb-Schalter werden nur noch einmal bei der Registrierung gesetzt.
- **EXIF-Orientierung:** Ein kleiner EXIF-Leser (JPEG APP1, TIFF-Header, nur Tag 0x0112, beide Byte-Reihenfolgen, mit Grenzprüfung gegen kaputte Dateien) füllt `PictureInfo.orientation`. `ViewTransform`, die Resample-Pipeline und die Vorschau-Leiste wenden sie an (2.5).
- `imageloader_pnm.h` und `imageloader_bmp.h` als Beweis für „neues Format“ und „gleiches Format“.
- Der Lade-Thread läuft nur noch über die Registry, und `IsPictureFile` fragt sie.
- Loader-Parameter, `L` zum Durchschalten, `--list-loaders`, `--decode-all`, Loader-Name im Fenstertitel.
- `--selftest`: Kennungen in beide Richtungen (mit und ohne Klammern, Groß- und Kleinschreibung, ungültige Eingaben) und Auswahlregeln an Byte-Puffern, also ohne Datei und ohne GL.
- **Abnahme:**
  - Mit der Standardeinstellung ändert sich sonst nichts: Der Testlauf aus Iteration 0 liefert für alle Bilder ohne Drehung dieselben Pixel wie vor dem Umbau.
  - `orientation_1` … `orientation_8` werden wie in 4.1 erwartet gleich angezeigt. Die gedrehten Fotos aus `202308` stehen aufrecht (Sichtprüfung an Stichproben mit Orientierung 3 und 6).
  - `--list-loaders` zeigt drei Loader mit Kennung. Eine unbekannte Kennung und ein mehrdeutiger Name führen zu einer klaren Fehlermeldung.
  - Die Loader-Testbilder aus 4.1 verhalten sich wie dort erwartet: BMP byteidentisch, RLE-Rückfall, Signatur vor Endung, kaputte Dateien ohne Absturz, PAM im Ordnerscan.
  - `L` schaltet ein BMP zwischen beiden Loadern um, und der Titel zeigt jeweils den richtigen.
  - `--decode-all=/home/final/Bilder/202308` liest alle 515 Fotos mit 32 Threads ohne Fehler.
  - `stbi_set_*` wird nur noch bei der Registrierung aufgerufen (per `grep` geprüft).

### Iteration 4 — LOD-Kette, SIMD-Reducer, Fortschritt

- `imagepyramid.h` und `imagepyramid_x86.h` nach 2.4:
  - Zuerst der architekturfreie Treiber mit Tabelle und skalaren Zeilenfunktionen.
  - Dann SSE2 → AVX2 → AVX-512, jede Stufe erst nach bitidentischem Selbsttest.
  - Die Auswahl läuft nur über `fplCPUGetCapabilities`, die Architekturweiche liegt an genau einer Stelle.
  - 2:1-Kernel per Zonenplatte gewählt, Zeilen-Ringpuffer, Abbruchprüfung zwischen Zeilenblöcken (`canceled`).
- Die Stufen entstehen im Lade-Thread, das Hochladen läuft klein → groß mit mitlaufender `BASE_LEVEL`, und Stufen über `GL_MAX_TEXTURE_SIZE` werden übersprungen.
- Die Pipeline wählt die Quellstufe nach der Regel in 2.3, und die Vorschau-Leiste samplet trilinear aus der Kette.
- Fortschritt nach 2.9, streng monoton.
- `--bench-lod`, und nur wenn die Messung es verlangt: Upload-Budget pro Frame.
- **Abnahme:**
  - Alle SIMD-Stufen bitidentisch (Testbilder + Zufallsbilder).
  - `--simd=neon` fällt auf x86 sauber auf die beste verfügbare Stufe zurück.
  - Im Viewer steht kein eigenes `cpuid`, und kein `#if` außerhalb der einen Weiche fragt `FPL_ARCH_*` ab (per `grep` geprüft).
  - Die `--bench-lod`-Ziele aus 2.3 sind erreicht oder die Abweichung ist begründet. Die Zahlen stehen in 1.2.
  - PSNR zwischen „Quelle LOD“ und „Quelle Stufe 0“ (`--lod-source=0`) ≥ 45 dB auf allen synthetischen Bildern, und der Aliasing-Wert ist unverändert. Die Doppelfilterung ist also unsichtbar.
  - Die Kette über 8-Bit-Zwischenstufen liegt bei Stufe 3 ≥ 45 dB gegen ImageMagick direkt aus Stufe 0 (sonst: u15-Zwischenstufen behalten).
  - `extreme_aspect` wird angezeigt.
  - Der Fortschrittsbalken springt nie zurück. Bei PNG steht er nicht auf 100 %, während noch dekodiert wird.

### Iteration 5 — Vergrößern

- Die Pipeline wird mit `widen = 1` für s > 1 genutzt, dieselben Kernel, Standard Catmull-Rom (Arbeitshypothese).
- Genau 100 % umgeht den Filter.
- Automatisch Nearest ab einer einstellbaren Zoomstufe (2.6), standardmäßig aus, `A` und `--nearest-from=`.
- **Abnahme:**
  - `impulse` ×7 und ×8: Profil ≤ 1 Stufe Abweichung vom abgetasteten Kernel, symmetrisch, Maximum mittig — für **alle** Kernel inklusive Triangular und Bell.
  - `pixelart_32` mit Nearest bei ×2 bis ×8 **byteidentisch**.
  - `step_edge`: Überschwinger je Filter dokumentiert und plausibel.
  - PSNR ≥ Schwelle gegen die ImageMagick-Referenz bei 1,5 / 2,3 / 4.
  - Die kleinen Screenshots (261×462 usw.) sehen eingepasst mit Hochrechnen sauber aus.
  - Mit `--nearest-from=400` ist `pixelart_32` bei ×4 byteidentisch zu Nearest, bei ×3 gilt der gewählte Filter.
  - Standardfilter zum Vergrößern: Der Nutzer bekommt Vergleichsausschnitte vorgelegt und legt fest.

### Iteration 6 — Pan & Zoom

- `ViewState` vollständig (`Custom`, `relativeScale`, `normalizedCenter`), Stufenleiter, Mausrad mit Magnet, Zoom um den Mauszeiger, Ziehen mit linker und mittlerer Taste, `Shift`+Pfeile, Doppelklick, `0`/`1`/`2`, `Strg`-Varianten, Seitentasten zum Blättern.
- `K` mit drei Modi und `--keep-view=`, Mausrad-Modus mit `W` und `--wheel=`, dazu `--zoom=` und `--center=` für reproduzierbare Aufnahmen.
- Zoom und Mitte stehen vorerst im Fenstertitel, ab Iteration 7 in der Info-Zeile.
- **Abnahme:**
  - `--selftest`: Zoom um einen Punkt lässt den Bildpunkt unter dem Zeiger auf ±0,5 px stehen. Klemmen: Ein Bild, das größer als das Fenster ist, zeigt nie einen Rand, ein kleineres bleibt zentriert. Behalten relativ: Gleich große Bilder ergeben pixelidentische Ansichten, dasselbe Motiv in 4032 und 1008 ergibt denselben Ausschnitt. Behalten absolut: Dasselbe Motiv in 4032 und 1008 steht beim Wechsel auf demselben Maßstab (z. B. 100 %) um dieselbe normierte Mitte.
  - Interaktiv per `xdotool` (Tasten, Rad, Ziehen): ← und → blättern auch gezoomt, `Shift+←` blättert **nicht**.
  - Beim Halten von → mit „Behalten“ bleibt die Ansicht stabil, ohne Springen.
  - Im Blätter-Modus (`W`) blättert das Rad, `Strg`+Rad zoomt. Im Zoom-Modus blättert das Rad nie.

### Iteration 7 — `final_ui`: GL3-Backend, Eingabe, Info-Zeile

- `fui_backend_gl3.h` (2.8). Eine GL3-Umschaltung in `FUI_Test` dient als Prüfstand.
- `fui_input_fpl.h`: Begin- und HandleEvent-Aufteilung.
- Font backen, `fuiContext` im Viewer, Hauptschleife umgebaut.
- **Info-Zeile** oben über die ganze Breite auf halbtransparentem Balken (benannte Konstanten für Höhe, Innenabstand und Deckkraft):
  - Links steht der **Dateiname** oder der **relative Pfad** zum geöffneten Ordner (`N` oder `--relative-path`). Bei Platzmangel wird **vorne** gekürzt (`…/unterordner/name.jpg`), weil der Name wichtiger ist als die Ordner. Das braucht eine kleine eigene Kürzung über `fuiMeasureText`, denn `fuiTruncateTextToWidth` kürzt hinten.
  - Rechtsbündig in derselben Zeile steht die **Originalgröße und bpp**: `4032 × 3024 · 24 bpp`. Dieser Teil hat Vorrang und wird nie gekürzt.
  - Die Zeile erscheint schon während des Ladens (aus `PictureInfo`).
  - Der Fortschrittsbalken rückt darunter.
  - `I` und `--no-info` blenden die Zeile aus, standardmäßig ist sie an.
- **Abnahme:**
  - `FUI_Test` über GL1 (Kompatibilitätskontext) und über GL3 (Core): Screenshots bis auf ≤ 1 Stufe gleich (`magick compare -metric AE` mit `-fuzz 1`).
  - Die Info-Zeile ist bei hellem und dunklem Bild lesbar (Screenshot + Zoom).
  - Umlaute in Dateinamen erscheinen richtig (Testdatei `Übersicht_ä.png`).
  - `gradient16` → 48 bpp, `palette8` → 8 bpp (Palette), Fotos → 24 bpp, Screenshots → 32 bpp.
  - Lange relative Pfade werden vorne gekürzt, die rechte Seite bleibt vollständig.
  - Drag & Drop und alle Tasten funktionieren weiter, obwohl die Bridge die Events jetzt mitliest.

---

## 6. Arbeitsregeln

- Code-Stil nach `CLAUDE.md`: benannte Zwischenvariablen statt verschachtelter Aufrufe, keine magischen Zahlen (Gewichte, Schwellen, Faktoren, Tastenzeiten sind benannte Konstanten), keine Umbrüche auf Spaltenbreite, geschweifte Klammern um **jeden** Bedingungskörper, Kommentare auf Englisch. Das gilt auch für die Shell-Skripte.
- **CMake ist maßgeblich:** `demos/FPL_ImageViewer/CMakeLists.txt` bekommt die neuen Header in `MY_HEADER_FILES`. `.vcxproj` wird nachgezogen.
- **Eine Versionsstufe pro Branch:** Der Viewer geht einmal von v0.5.6 auf v0.6.0 (`version.h` + Changelog im Dateikopf), und jede Iteration ergänzt nur diese Sektion. `final_ui.h` bleibt unberührt. `fui_backend_gl3.h` ist neu. `final_platform_layer.h` wird nur angefasst, falls `hasAVX512BW` nötig wird, und dann mit genau einer Patch-Stufe über `develop`.
- Commits macht der Nutzer selbst. Branch: `demo/image-viewer-improvements`.

---

## 7. Entscheidungen und Folgepunkte

### 7.1 Entschieden am 2026-09-23

| Punkt | Entscheidung | Wo im Plan |
|---|---|---|
| EXIF-Orientierung | wird eingebaut | 2.5, 2.10, Iteration 3 |
| `stb_image` aktualisieren | vor allem anderen, eigener Commit, alle abhängigen Demos bauen | Iteration 0 |
| Hintergrund für Transparenz | umschaltbar Schachbrett/Schwarz/Grau, Standard Schachbrett | 2.7, Iteration 2 |
| Legacy-GL-Pfad | **entfernt**, GL 3.3 Core ist Voraussetzung | Iteration 1 |
| Mausrad | einstellbar, Standard Zoom um den Mauszeiger, `W` schaltet auf Blättern | 2.6, 2.7, Iteration 6 |
| Verschieben per Tastatur | `Shift` + Pfeile | 2.7 |
| Ansicht behalten | drei Modi: zurücksetzen, relativ, absolut; `K` schaltet durch | 2.6, Iteration 6 |
| Automatisch Nearest | eingebaut, standardmäßig aus, `A` | 2.6, Iteration 5 |
| Neu zeichnen nur bei Bedarf | eingebaut | Iteration 1 |
| VRAM-Budget | später | 7.2 |
| LOD eines Bildes auf mehrere Threads | nur wenn die Benchmarks es verlangen | Iteration 4, 7.2 |
| Info-Zeile ausblendbar | `I`, Standard an | 2.7, Iteration 7 |
| Referenz-BMP-Loader | bleibt, hinter stb eingereiht | 2.10 |
| Plugin-Loader als DLL | später, eigener Plan | 7.2 |
| Weitere Loader | **WebP** als Folgeplan vorgemerkt, sonst keine | 7.2 |
| Kennungstyp | eigener `ImageLoaderId` im Viewer, kein `fplGuid` in FPL | 2.10 |
| Umzug nach `apps/` + Einstellungsdatei | nach diesem Plan, eigener Plan | 7.2 |
| NEON | später, die Architektur reicht vorerst | 2.4, 7.2 |
| Glyphen | Latin-1 (U+0020–U+00FF) reicht vorerst | 2.8 |
| Standardfilter | Mitchell ↓ / Catmull-Rom ↑ als Hypothese; der Nutzer entscheidet anhand vorgelegter Vergleichsausschnitte | 2.2, Iteration 2 und 5 |
| Testbilder | **eingecheckt** unter `demos/FPL_ImageViewer/tests/images/`, der Generator bleibt zum Nachbauen daneben | 4, 4.3 |
| PSNR-Schwelle | 45 dB statt 40 dB, nach Kalibrierung (Obergrenze ≥ 54,4 dB) | 4.2 |

### 7.2 Folgepunkte (nicht in diesem Plan)

- **App-Umbau:** Umzug nach `apps/FPL_ImageViewer` und eine Einstellungsdatei für Filter, Loader-Kennungen, Ansichtsmodus, Mausrad-Modus, Hintergrund und so weiter. Bis dahin gibt es alles als Parameter und Tasten.
- **WebP-Loader:** der erste Folgeplan für ein neues Format, über die Loader-Schnittstelle aus 2.10.
- **Plugin-Loader als DLL** über `fplDynamicLibraryLoad` und eine exportierte `ImageLoaderGetDescriptor`. Die Schnittstelle ist darauf ausgelegt, das Laden selbst ist ein eigenes Thema: Suche, Versionsprüfung, Entladen, Fehler in fremdem Code.
- **VRAM-Budget:** Nachbarn jenseits von ±N laden nur die Stufe in Bildschirmgröße hoch, Stufe 0 erst beim Hinsehen. Die LOD-Kette aus Iteration 4 macht das möglich.
- **NEON-Stufe:** die vier Zeilenfunktionen in `imagepyramid_arm.h`, eine Tabellenzeile und die Zuordnung zu `arm.hasNEON`. Dazu `aarch64-linux-gnu-gcc` installieren und `--selftest` unter `qemu-aarch64` laufen lassen. Vorher muss der Viewer insgesamt einmal auf ARM übersetzt werden.
- **LOD eines Bildes auf mehrere Threads:** nur falls die Benchmarks aus Iteration 4 es verlangen.
- **Blockierendes Warten in FPL** (etwa `fplWaitEvent(timeout)`: X11 über `select` auf die Verbindung, Win32 über `MsgWaitForMultipleObjects`): Damit käme der Leerlauf ohne Schlaf-Intervall und ohne dessen Eingabelatenz aus. Der Leerlauf in Iteration 1 funktioniert auch ohne.
- **Glyphen jenseits von Latin-1** (mehrere Bereiche oder ein dynamischer Glyphen-Cache in `fui_font_stbtt.h`), falls Dateinamen mit anderen Schriften eine Rolle spielen.

---

## 8. Risiken

- **RGBA16F-Zwischentexturen bei 4K:** 3840 × Quellzeilen × 8 B im ersten Durchgang. Zusammen mit der Zieltextur sind bei s ≈ 0,5 rund 100–130 MB Arbeitsspeicher auf der GPU zu erwarten. Falls das stört, wird der erste Durchgang in Streifen zerlegt.
- **Treiberunterschiede beim sRGB-Framebuffer:** Auf NVIDIA ist er nachweislich aktiv (188 in 1.2). Auf Mesa oder Intel wird er beim Start über `GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING` geprüft. Ist er nicht sRGB-fähig, kodiert der letzte Kopiervorgang selbst.
- **SIMD-Bitidentität** hängt an reiner Ganzzahlarithmetik. Sobald irgendwo Gleitkomma hineinrutscht (z. B. eine Alpha-Division), muss sie in **allen** Stufen skalar und gleich laufen. Der Selbsttest fängt das ab.
- **AVX-512 nur mit F:** Falls BW-Befehle deutlich schneller wären, heißt das eine FPL-Änderung. Getestet werden kann alles lokal, denn der 7950X hat F, BW und VL.
- **ARM-Weiche auf Apple:** Wer an der einen Weiche vorbei `FPL_ARCH_ARM64` allein abfragt, bekommt auf einem Mac stillschweigend den skalaren Pfad. Das fällt nur über die Leistung auf, nie über ein falsches Ergebnis. Abhilfe ist die `grep`-Prüfung in der Abnahme von Iteration 4.
- **ImageMagick-Referenz:** Die Kernel-Definitionen müssen exakt übereinstimmen (Mitchell B = C = ⅓, Lanczos mit 3 Keulen, Catmull-Rom B = 0, C = ½). Eine Abweichung sieht aus wie ein Fehler im Viewer. Der Impulstest ist die unabhängige Gegenprobe.
- **Fremde Loader und Threads:** Ein Loader, der fälschlich als threadsicher gilt, erzeugt seltene, schwer reproduzierbare Fehler. `--decode-all` mit 32 Threads über die Fotosammlung ist der Belastungstest, und im Zweifel wird ein neuer Loader zunächst als `NotThreadSafe` eingetragen.
- **`stb_image`-Update:** Die Datei ist eine gemeinsame Abhängigkeit mehrerer Demos. Eine geänderte API oder geänderte Warnungen fallen erst beim Bauen der anderen Demos auf, deshalb gehört das zur Abnahme von Iteration 0.
- **Ohne GL 3.3 Core kein Start:** Mit dem Legacy-Pfad fällt der letzte Weg für sehr alte Grafik weg. Hardware ab etwa 2010 kann 3.3 Core, und Mesa bietet es auch in Software (llvmpipe).
- **EXIF aus fremden Dateien:** Der EXIF-Leser liest nicht vertrauenswürdige Daten. Jeder Offset und jede Länge wird gegen die Blockgröße geprüft, und kaputte EXIF-Daten gelten als Orientierung 1. Dafür gibt es `truncated.jpg` und ein absichtlich kaputtes EXIF-Testbild.
- **Die Standardfilter** sind Arbeitshypothesen. Text in Bildschirmfotos kann die Wahl beim Verkleinern zu Mitchell oder Box verschieben, Fotos eher zu Lanczos3.
- **Größe der eingecheckten Testbilder:** zusammen ≈ 20 MB, davon `zoneplate_4096.png` 15 MB und `zoneplate_2048.png` 3,8 MB. Zonenplatten lassen sich kaum komprimieren, alle anderen Bilder zusammen haben ≈ 1,4 MB. Jede Neuerzeugung mit geändertem Inhalt landet erneut in der Git-Historie. Deshalb ist der Generator deterministisch, und die Zonenplatten werden nur bei echtem Bedarf geändert.
