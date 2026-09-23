# Plan: FPL_ImageViewer — Skalierung, Pan & Zoom, final_ui

Ziel: Der `FPL_ImageViewer` wird zur eigenständigen App umgebaut, zu einem schnellen und brauchbaren Bildbetrachter. Dieser Plan legt die **technische Basis** dafür: Bilder werden beim Verkleinern **richtig heruntergerechnet** und beim Vergrößern mit einem umschaltbaren Filter **hochgerechnet**. Man kann **zoomen und verschieben**, ohne dass das mit dem Blättern kollidiert. `final_ui.h` ist **fest eingebaut** und zeichnet vorerst nur eine Info-Zeile.

Dieses Dokument beschreibt den **Stand samt Messungen** (1), die **Designentscheidungen** (2), den **inneren Aufbau** (3), die **Testbilder und den Messstand** (4) und die **Iterationen** mit Abnahmekriterien (5). Danach folgen Arbeitsregeln, Offenes und Risiken (6–8).

---

## 1. Stand

### 1.1 Was da ist

`demos/FPL_ImageViewer/fpl_imageviewer.cpp` (~1900 Zeilen, C++11 wegen `R"()"`), v0.5.6:

- Dateiliste aus einem Ordner (optional rekursiv), `.jpg/.jpeg/.png/.bmp`.
- `viewPicturesCapacity` = Preload + 1 Slots (Standard 17) um das aktive Bild herum. Die Lade-Threads (Standard: ein Thread pro Kern, hier 32) holen Aufträge aus einer lock-freien MPMC-Queue und dekodieren mit `stbi_load_from_callbacks`. Texturen werden im Hauptthread angelegt und freigegeben.
- OpenGL 3.3 **Core** mit 16× MSAA, sRGB-Framebuffer, Texturen als `GL_TEXTURE_RECTANGLE`. Dazu ein Legacy-Pfad hinter `FORCE_LEGACY_OPENGL`.
- Sieben Filter-Shader (`shadersources.h`): Nearest, Bilinear, Bicubic (Triangular, Bell, B-Spline, Catmull-Rom), Lanczos3. Standard ist Bicubic (Triangular). Umschalten mit `T`.
- Darstellung: Das Bild wird eingepasst, wenn es größer als das Fenster ist, sonst 1:1 gezeigt. Dazu kommt eine Vorschau-Leiste mit einem Kästchen pro Slot.
- Tasten: Links/Rechts, Bild auf/ab (±10), Pos1/Ende, `F` Vollbild, `P` Vorschau, `R` neu laden, `T` Filter.

### 1.2 Gemessen

Alle Messungen stammen von dieser Maschine: Ryzen 9 7950X, RTX 3090, NVIDIA 580, Fenster 1280×720, Build mit `g++ -O2`. Die Screenshots wurden per `xdotool` und `import -window` gemacht und mit `magick compare` ausgewertet. Die Referenzen sind mit ImageMagick **in linearem Licht** gerechnet (`-colorspace RGB -filter X -resize … -colorspace sRGB`). `xdotool` ist inzwischen installiert (im Handout des Editors hieß es noch „fehlt“), der Viewer lässt sich also fernsteuern.

**1:1-Treue.** Ein 1-Pixel-Schachbrett 512×512 wird genau 1:1 und pixelgenau ausgerichtet gezeigt:

| Filter | Ergebnis (sRGB-Werte, Bildmitte) |
|---|---|
| Nearest | 0 / 255 — **exakt** |
| Bilinear, Bicubic (Triangular, Bell, B-Spline, Catmull-Rom) | **flach 188** — das Schachbrett ist weg |
| Lanczos3 | 135 / 225 — stark verwaschen |

188 ist der sRGB-Wert von linear 0,5. Der Mittelwert wird also **in linearem Licht** gebildet, und die sRGB-Kette aus `GL_SRGB8_ALPHA8` und `GL_FRAMEBUFFER_SRGB` funktioniert. Das Verwaschen selbst ist ein **Halb-Texel-Versatz**: `a = fract(uv * size)` ist an jeder Pixelmitte 0,5, deshalb mischt jeder dieser Filter bei 100 % zwei Nachbarn zu gleichen Teilen (`shadersources.h:125`, `:146`). **Bei 100 % sieht man heute mit jedem Standardfilter ein unscharfes Bild.**

**Aliasing beim Verkleinern.** Eine Zonenplatte 2048×2048 wird eingepasst. Gemessen wird die Standardabweichung in einem Ring, dessen Frequenz über der Nyquist-Grenze der Ausgabe liegt. Dort muss ein korrekter Downscale **flach** sein, jede Schwankung dort ist Moiré:

| Maßstab | Referenz Mitchell | Triangular | Bell | B-Spline | Catmull-Rom | Lanczos3 | Nearest | Bilinear |
|---|---|---|---|---|---|---|---|---|
| 0,35 (→720) | 8,2 | 9,3 | 10,5 | 12,1 | 22,8 | 21,5 | 50,2 | 20,6 |
| 0,146 (→300) | **1,1** | 15,9 | 24,3 | 30,1 | 53,4 | 57,7 | 64,3 | 43,1 |

Bei 0,35 hält der Standardfilter noch mit, weil er ohnehin weich ist. Bei 0,146 aliast **jeder** Filter, 15- bis 60-mal stärker als die Referenz. Die Kernel greifen immer ±2 Quelltexel ab, egal wie viele Quelltexel auf ein Bildschirmpixel fallen. Das ist der Kern von „es gibt kein richtiges Downscale“.

**Fotos unterscheiden die Filter kaum.** `IMG_8978.JPG` (4032×3024 → 960×720, Maßstab 0,238) im Vergleich zur Mitchell-Referenz: **alle sieben Filter liegen zwischen 39,7 und 42,0 dB PSNR, Nearest eingeschlossen (40,5)**. Glatte Flächen dominieren die Metrik. Deshalb braucht es **synthetische Testbilder** (Abschnitt 4). Echte Fotos taugen hier nur zur Sichtprüfung.

**CPU-Kosten.** Dekodieren und LOD-Erzeugung mit dem vorhandenen `stb_image_resize` v0.96 (sRGB-korrekt, Mitchell):

| | 4032×3024 JPEG | 2154×1108 PNG |
|---|---|---|
| `stbi_load` | 43,9 ms | 32,0 ms |
| LOD-Stufe 1 (½) | 140,3 ms | 27,8 ms |
| Stufen 2–4 | 35,4 + 9,0 + 2,3 ms | 7,0 + 1,9 + 0,5 ms |
| **Summe LOD** | **187 ms = 4,3× Dekodieren** | 37 ms |

LOD per `stb_image_resize` würde die Ladezeit eines Fotos verfünffachen. Deshalb bekommt die LOD-Erzeugung einen eigenen SIMD-Reducer (2.3).

**Speicher.** Ein 12-MP-Bild belegt als RGBA8 48,8 MB. 17 Slots ergeben **829 MB VRAM**, mit voller Mip-Kette (+33 %) **1,1 GB**. 16× MSAA kostet bei 4K allein für den Farbpuffer 531 MB und verbessert **das Bildinnere nicht**. Der Fragment-Shader läuft trotzdem nur einmal pro Pixel, MSAA glättet nur Geometriekanten, und ein bildschirmfüllendes Rechteck hat keine.

### 1.3 Befunde im Code

- Halb-Texel-Versatz in Bilinear und allen Bicubic-Filtern (siehe oben). Lanczos3 berechnet die Texelmitte zwar, ist bei 1:1 aber trotzdem nicht exakt. Die Ursache wurde nicht untersucht, weil der Shader ohnehin ersetzt wird.
- `GL_TEXTURE_RECTANGLE` (`fpl_imageviewer.cpp:1119`): kann keine Mip-Stufen haben und braucht nicht-normalisierte Koordinaten. NPOT-`GL_TEXTURE_2D` ist seit GL 2.0 Kern.
- `GL_CLAMP` (`:530`) gibt es im Core-Profil nicht mehr → `GL_CLAMP_TO_EDGE`.
- Mip-Gerüst halb fertig und abgeschaltet: `MAX_PICTURE_MIPMAPS = 1` (`:212`), Größen `w / (2 * i)` statt `w >> i` (`:699`), jede Stufe wird aus Stufe 0 statt aus der vorherigen gerechnet (`:697`).
- Die Zweige in `UpdateAndRender` (`:1562–1581`) heißen „Upscaling“ und „Downscaling“, sind aber „Einpassen“ und „1:1“.
- Fortschritt: Die Leseposition läuft bis 1,0, danach setzt `:695` den Wert auf **0,75 zurück**. Bei PNG liest `stbi` erst die ganze Datei und entpackt dann, dann steht der Balken auf 100 %, während die eigentliche Arbeit noch läuft.
- `ParseParameters` (`:882–891`): Das `switch` schickt alles außer `r` und `t` in `default: continue` → **`-p=` und `-f=` werden nie ausgewertet**.
- `LoadPicturesPath` (`:1050`): `startIndex = 0` setzt den Zeiger statt `*startIndex`. Der Fehler bleibt folgenlos, weil der Aufrufer vorbelegt.
- `preloadCount` wird erst **nach** der Verwendung auf gerade gerundet (`:1195`), die Rundung wirkt also nicht.
- Vorschau-Leiste: Sie zeichnet die volle Textur mit dem aktiven Filter in ein Kästchen von ~40 px und aliast deshalb massiv.
- `fui_input_fpl.h`: `fuiFplInputPumpEvents` leert die **ganze** Event-Queue. Der Viewer braucht die Events aber selbst (Drop, Tasten).
- `fui_backend_gl1.h` ist Fixed-Function und läuft auf einem Core-3.3-Kontext nicht. → **`fui_backend_gl3.h` wird gebaut** (Iteration 6).
- `fplX86CPUCapabilities.hasAVX512` prüft nur **AVX512F** (CPUID 7/EBX Bit 16, inklusive XCR0-Prüfung). BW/VL fehlen.
- Auf Apple Silicon setzt FPL nur `FPL_ARCH_APPLE_ARM64` und **nicht** `FPL_ARCH_ARM64` (`final_platform_layer.h:2170–2182`). Das ist kein Fehler, aber eine Falle für jede ARM-Weiche (2.4).

**Aufgefallen, aber nicht Teil des Auftrags:** 10 von 40 Stichproben-Fotos aus `202308` tragen EXIF-Orientierung 3 oder 6 und werden deshalb gedreht angezeigt, weil `stb_image` EXIF ignoriert. Außerdem ist `stb_image` auf v2.19 (2018), und seitdem gab es mehrere Sicherheitskorrekturen. Beides steht in Abschnitt 7. Das View-Modell reserviert der Orientierung schon jetzt ihren Platz.

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

- **Mipmaps + trilinear:** Das ist Box-Filter plus Überblendung zweier Stufen, weich und nicht „richtig gerechnet“. Es bleibt der Fallback für den Legacy-Pfad und für die Vorschau-Leiste.
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

**Standard beim Verkleinern: Mitchell.** Das ist der übliche Kompromiss zwischen Schärfe, Aliasing und Überschwingen, und auch `stb_image_resize` nimmt ihn als Standard zum Verkleinern. **Standard beim Vergrößern: Catmull-Rom**, interpolierend und scharf mit wenig Überschwingen. Beide Vorgaben sind Arbeitshypothesen. Iteration 2 und 4 entscheiden anhand der Testbilder und der Bildschirmfoto-Sammlung, denn Text in Screenshots ist der Härtefall für Lanczos-Halos.

`T` schaltet den Filter der Richtung weiter, die **gerade wirkt**, `Shift+T` schaltet zurück. So bleibt die gewohnte Taste erhalten, und es gibt keine neuen Tasten. Titel und Info-Zeile zeigen „↓ Mitchell“ oder „↑ Catmull-Rom“.

### 2.3 LOD: auf der CPU, im Lade-Thread, mit SIMD

**Entscheidung:** Die LOD-Kette entsteht **im Lade-Thread**, wie vorgeschlagen, allerdings mit einem **eigenen SIMD-Reducer** statt `stb_image_resize`. Nach der Messung in 1.2 würde der die Ladezeit verfünffachen.

Wofür die LOD-Stufen gebraucht werden:

1. **Kosten der Resample-Pipeline begrenzen.** Quelle ist die kleinste Stufe, die noch mindestens doppelt so groß wie das Ziel ist. Der Maßstab relativ zur Quelle liegt dann in (¼, ½], die Verbreiterung bleibt ≤ 4 und die Abgriffe pro Achse ≤ 16 (Mitchell) bzw. ≤ 24 (Lanczos3), egal wie groß das Original ist. Der Abstand „mindestens doppelt“ sorgt dafür, dass das Übergangsband des 2:1-Vorfilters über der Nyquist-Grenze des Ziels liegt. So verfälscht die Doppelfilterung das Ergebnis messbar nicht (Abnahme in Iteration 3).
2. **Vorschau-Leiste** und **Legacy-Pfad:** trilinear aus der Kette, billig und ohne Moiré.
3. **Bilder über `GL_MAX_TEXTURE_SIZE`** (hier 32768): Stufen, die nicht passen, werden nicht hochgeladen. Die Basisstufe ist dann die erste, die passt. Es ist derselbe Code, kein Sonderfall.
4. **Schneller erster Eindruck:** Die kleinen Stufen werden zuerst hochgeladen, das Bild erscheint sofort und wird scharf, sobald Stufe 0 da ist.

Stufen entstehen bis die lange Seite ≤ 32 px ist. Bei 4032×3024 sind das 7 Stufen. Die ersten vier (2016, 1008, 504, 252) machen über 99 % der Arbeit aus, die restlichen kosten zusammen weniger als ein halbes Prozent.

**Der Reducer.** Eine Funktion `ReduceHalf` (RGBA8 sRGB → RGBA8 sRGB, Größe `floor(w/2) × floor(h/2)` wie in GL) arbeitet so:

1. **Dekodieren** über eine 256-Einträge-LUT: sRGB8 → linear u15 (0…32767). Danach wird Alpha vormultipliziert. Die LUT-Zugriffe bleiben skalar: 512 Byte liegen im L1, und Gather-Befehle sind auf Zen 4 nicht schneller.
2. **Filtern** in Festkomma: u15-Abtastwerte, Q14-Gewichte (vorzeichenbehaftet, Summe exakt 16384), int32-Akkumulator, runden, `>> 14`, auf [0, a] klemmen. Die Daten liegen planar vor (R, G, B, A getrennt). Dann ist der vertikale Durchgang ein reines Vektor-Multiply-Add über die Zeile, und der horizontale 2:1-Durchgang wird über eine Gerade/Ungerade-Aufteilung zu einer Summe verschobener Vektoren.
3. **Kodieren:** Alpha-Division herausrechnen (Schnellweg für opake Pixel), dann linear u15 → sRGB8 über eine LUT mit 32768 Einträgen (32 KB). Eine kleinere LUT wäre im Dunkeln zu grob.
4. Die nächste Stufe wird aus dem RGBA8-Ergebnis der vorherigen gerechnet, mit **derselben** Funktion. Ein Zeilen-Ringpuffer (Kernel-Höhe × Breite) statt einer vollen Zwischenkopie kostet pro Thread ~260 KB statt ~100 MB. Ob die wiederholte 8-Bit-Quantisierung stört, wird gemessen (Iteration 3).

**Kernel für 2:1:** Kandidaten sind Lanczos2 und Mitchell mit 8 Abgriffen pro Achse. Entschieden wird über die Zonenplatte: Aliasing-Wert und PSNR einer reinen 2:1-Reduktion gegen `magick -filter X -resize 50%` in linearem Licht.

**Ziele** (7950X, 4032×3024, alle Stufen zusammen): AVX2/AVX-512 ≤ 22 ms (≤ 50 % des Dekodierens), SSE2 ≤ 40 ms, skalar deutlich unter den 187 ms von `stb_image_resize`. Das sind Ziele, keine Zusagen. Die gemessenen Werte kommen nach Iteration 3 in Abschnitt 1.2.

**Hochladen:** eine `GL_TEXTURE_2D` mit echten Mip-Stufen `glTexImage2D(level i)`. `GL_TEXTURE_MAX_LEVEL` ist die letzte Stufe, `GL_TEXTURE_BASE_LEVEL` folgt der größten schon hochgeladenen. `glTexStorage2D` ist erst ab GL 4.2 Kern und wird deshalb nicht benutzt. Die Reihenfolge ist klein → groß. Ein Upload-Budget pro Frame kommt nur, wenn die Messung in Iteration 3 zeigt, dass `glTexImage2D` von 48 MB den Frame reißt.

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

Die EXIF-Orientierung hätte hier ihren Platz als Transformation der Bildachsen, sobald sie kommt (Abschnitt 7).

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

`Fit` und `ActualSize` bleiben beim Wechsel **als Modus** erhalten, dann ist 100 % auch beim nächsten Bild 100 %. Ein dritter Modus „absoluter Maßstab behalten“ (gleiche Bildpixel pro Bildschirmpixel über verschiedene Auflösungen) kostet eine Zeile, wird aber nur gebaut, wenn er gebraucht wird (Abschnitt 7).

**Einstellung** `ViewPersistence_Reset` (Standard: jeder Wechsel beginnt eingepasst) oder `ViewPersistence_Keep`. Umschalten mit `K` oder `--keep-view`.

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
| Zoom (Stufenleiter) | `+` / `-` (Haupt- und Nummernblock), `Strg` + `+`/`-` | Mausrad um den Mauszeiger (auch `Strg`+Rad) |
| Einpassen | `0` (auch `Strg`+`0`) | Doppelklick: Einpassen ↔ 100 % am Mauszeiger |
| 100 % / 200 % | `1` (auch `Strg`+`1`) / `2` | |
| Verschieben | `Shift` + Pfeile (⅛ Fenster pro Druck, mit Wiederholung) | Linke oder mittlere Taste ziehen |
| Ansicht beim Wechsel behalten | `K` | |
| Filter der wirkenden Richtung | `T` / `Shift`+`T` | |
| Dateiname ↔ relativer Pfad | `N` | |
| Info-Zeile ein/aus | `I` (Vorschlag) | |
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
| Lesen + Dekodieren | 0–70 % | JPEG: Dateiposition (`stbi` dekodiert beim Lesen). PNG: Dateiposition nur 0–25 %, danach eine unbestimmte „läuft“-Animation im Rest der Phase, weil `stbi` erst nach dem Lesen entpackt |
| LOD-Stufen | 70–95 % | pro Stufe gewichtet nach Ausgabepixeln, Stufe 1 ≈ 75 % der LOD-Arbeit |
| Hochladen | 95–100 % | Hauptthread, pro Stufe |

Die Gewichte werden nach Iteration 3 anhand der Benchmark-Zahlen einmal nachgestellt. Der Balken wandert unter die Info-Zeile (Iteration 6), heute sitzt er genau dort, wo die Zeile hinkommt.

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
| `demos/additions/fui_backend_gl3.h` | siehe 2.8 |
| `demos/FPL_ImageViewer/tests/` | Testbild-Generator, Testlauf, Auswertung (Abschnitt 4) |

Änderungen an Vorhandenem:

- `ImageData imageData[MAX_PICTURE_MIPMAPS]` wird zu `ImageLevel levels[MAX_PICTURE_LEVELS]` mit `levelCount` und `uploadedLevelCount`. Hinzu kommt `PictureInfo info`.
- `PictureInfo { width, height, channelCount, bitsPerPixel, isPalette }`: vor dem Dekodieren aus `stbi_info_from_callbacks` und `stbi_is_16_bit_from_callbacks`, ergänzt um einen Header-Blick für PNG (IHDR-Farbtyp 3 → Palette, Bittiefe = bpp) und BMP (`biBitCount`). Die Info-Zeile kann dadurch Größe und bpp schon **während des Ladens** zeigen. `stbi` allein meldet ein Palette-PNG als 24/32 bpp.
- Filter-Shader aus `shadersources.h` → Kernel-Gewichtsfunktionen in `resamplepipeline.h`. Uniform-Positionen werden einmal ermittelt statt bei jedem Zeichnen (`glGetUniformLocation` läuft heute pro Aufruf). Zwei Sampler-Objekte (nearest, trilinear) ersetzen das `glTexParameteri` pro Frame.
- Hauptschleife: Events → Bridge und Viewer, dann `fuiFplInputBuild`, Update, Bild (Pipeline oder Cache-Kopie), Vorschau-Leiste, Info-Zeile über `final_ui`, `fuiGL3Render`, Flip.
- **Parameter** werden neu geparst. Die alten Kurzformen `-r -t= -p= -f=` bleiben, repariert. Dazu kommen Langformen mit selbsterklärenden Namen:

| Parameter | Bedeutung |
|---|---|
| `--zoom=fit\|100\|<Prozent>` | Startzoom |
| `--center=<u>,<v>` | normierte Startmitte |
| `--down-filter=<Name>`, `--up-filter=<Name>` | Filterwahl |
| `--keep-view`, `--relative-path`, `--no-preview`, `--no-info` | Einstellungen |
| `--simd=scalar\|sse2\|avx2\|avx512\|neon` | SIMD-Stufe erzwingen; eine nicht verfügbare Stufe fällt mit Log-Zeile zurück |
| `--lod-source=auto\|0` | Pipeline-Quelle erzwingen, für Referenzvergleiche |
| `--render-to=<Datei.pam> --window=<B>x<H>` | ein Bild offscreen rendern, als PAM schreiben, beenden |
| `--bench-lod=<Datei>` | Dekodieren + LOD je SIMD-Stufe messen (Median aus N Läufen) |
| `--selftest` | ViewTransform-Mathematik + Bitidentität aller SIMD-Stufen. Läuft **ohne Fenster und GL**, also auch unter `qemu-aarch64`. |

---

## 4. Testbilder und Messstand

Die Filter werden gegen **selbst erzeugte** Bilder validiert, denn echte Fotos unterscheiden sie nachweislich nicht (1.2). Der Generator ist ein Skript und kein eingechecktes Bildarchiv. Die Bilder sind reproduzierbar und landen in `demos/build/FPL_ImageViewer/tests/`, das bereits ignoriert ist.

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
| Fotos `202308`, `Bildschirmfotos` | | echte Motive | Sichtprüfung an vergrößerten Ausschnitten |

### 4.2 Referenzen und Metriken

- **Referenz:** `magick <in> -colorspace RGB -filter <X> -resize <B>x<H>! -colorspace sRGB -depth 8 <ref>`, mit gleichem Kernel (`Box`, `Triangle`, `Mitchell`, `Catrom`, `Lanczos`, `Point`, B-Spline = `Spline`) und gleicher Pixelmitten- und Randkonvention. Für die beiden alten Kernel Triangular und Bell hat ImageMagick keine genaue Entsprechung. Sie werden nur über den Impulstest geprüft.
- **PSNR gegen dieselbe Kernel-Referenz:** Startziel ≥ 40 dB auf allen synthetischen Bildern. Die Schwelle wird in Iteration 0 kalibriert: Wie weit liegen ImageMagick selbst in Q16 und die auf 8 Bit gerundete Fassung auseinander? Das ist die Obergrenze des Erreichbaren.
- **Aliasing-Wert:** Standardabweichung im Ring über der Nyquist-Grenze (Zonenplatte), gegen die Werte der Referenz. Die heutigen Werte stehen in 1.2.
- **Exakte Tests** (100 %, Nearest bei ganzzahligem Zoom): `magick compare -metric AE` = 0.
- **Bitidentität der SIMD-Stufen:** `--selftest` vergleicht alle verfügbaren Stufen auf allen Testbildern und auf Zufallsbildern (Größen 1…130 × 1…130, Zufallsinhalt mit Alpha) Byte für Byte.

### 4.3 Werkzeuge

- `tests/generate_testimages.sh`: Erzeugt alle Bilder aus 4.1 mit ImageMagick. Das Skript ist deterministisch und hat englische Kommentare.
- `tests/run_scaling_tests.sh`: Für jede Kombination aus Bild, Maßstab (0,1 / 0,146 / 0,238 / 0,35 / 0,5 / 0,7 / 1 / 1,5 / 2,3 / 4 / 8) und Filter rendert es mit `--render-to`, erzeugt die Referenz, misst und gibt eine Tabelle aus. Der Exit-Code ist ungleich 0, wenn eine Schwelle verletzt ist.
- `--render-to` ist der Kern des Messstands: Das Bild wird offscreen in ein Framebuffer-Objekt genau der angegebenen Größe gerendert, ohne Vorschau-Leiste, ohne Info-Zeile, ohne Fensterdekoration und ohne Compositor, und per `glReadPixels` als PAM geschrieben. Das ist deterministischer als jeder Screenshot. Ein Fenster entsteht trotzdem, weil FPL es für den GL-Kontext braucht.
- **Interaktion** (Tasten, Mausrad, Ziehen, Info-Zeile) wird per `xdotool` gesteuert und mit `import -window <id>` aufgenommen, wie bei den Messungen in 1.2. Nach jeder Zeichenänderung heißt es: Screenshot **und** hineinzoomen.

---

## 5. Iterationen

Die Reihenfolge folgt dem Auftrag: Zuerst die Technik und dort **zuerst das Verkleinern**, dann das Vergrößern, dann die Steuerung, am Ende die UI. Das Verkleinern ist ohne LOD-Stufen schon **richtig**, die LOD-Stufen machen es danach nur schneller und begrenzen die Kosten. Deshalb kommt die Qualität vor der Kette.

### Iteration 0 — Testbilder und Messstand

- `generate_testimages.sh` und `run_scaling_tests.sh`. `--render-to` wird vorgezogen und rendert vorerst mit dem **alten** Zeichenweg. Die Parameter werden repariert (`-p`, `-f`), und die Langformen aus Abschnitt 3 kommen dazu, soweit sie schon Sinn ergeben.
- Die Schwellen werden kalibriert (4.2).
- **Abnahme:** Der Testlauf läuft durch und dokumentiert den **heutigen** Stand als Tabelle. Er muss die Zahlen aus 1.2 reproduzieren, also 1:1 flach 188 bei allen außer Nearest und Aliasing bei 0,146.

### Iteration 1 — Fundament

- `GL_TEXTURE_RECTANGLE` → `GL_TEXTURE_2D`, `GL_CLAMP` → `GL_CLAMP_TO_EDGE`, 16× MSAA weg.
- Das halbe Mip-Gerüst kommt raus (`MAX_PICTURE_MIPMAPS`, `DownsampleImage`, die Auswahlschleife).
- Umstellung auf y-unten-Fensterpixel; `viewtransform.h` mit `ComputeViewTransform` (vorerst nur `Fit` und `ActualSize`) und Ganzpixel-Rundung des Ursprungs. Die falsch benannten Zweige verschwinden dabei.
- Halb-Texel-Korrektur in den vorhandenen Filtern als Zwischenschritt: `p = uv·size − 0,5`, `base = floor(p)`, Gewichte aus `p − base`. So ist die Iteration für sich schon eine Verbesserung, auch wenn Iteration 2 die Shader ersetzt.
- `--selftest` mit den ersten ViewTransform-Prüfungen.
- **Abnahme:** `checker_1px` bei 100 % **byteidentisch** mit **jedem** interpolierenden Filter (Nearest, Bilinear, Catmull-Rom, Lanczos3). `border_frame_odd` hat bei 100 % und beim Einpassen alle vier Rahmenseiten. Es gibt keine GL-Fehler im Core-Profil. Die Fotos sehen nicht schlechter aus als vorher (Sichtprüfung, Vorher-Nachher-Ausschnitte).

### Iteration 2 — Richtiges Verkleinern

- `resamplepipeline.h`: Kernel-Tabelle (2.2) mit Box und Mitchell neu, Shader-Vorlage für horizontal und vertikal, verbreiterter Kernel, lineares Licht, vormultipliziertes Alpha, RGBA16F-Zwischen- und Zieltextur, nur der sichtbare Ausschnitt, Cache mit Schlüssel (Bild, Stufe, Maßstab, Versatz, Filter, Fenstergröße).
- Quelle ist vorerst immer Stufe 0.
- Getrennte Filterwahl für unten und oben, `T` / `Shift+T` wirkt auf die aktive Richtung, der Fenstertitel zeigt Richtung und Filter.
- Ausblick: GPU-Zeit der Durchgänge per `GL_TIME_ELAPSED`-Abfrage, sichtbar im Log.
- **Abnahme:**
  - Zonenplatte bei 0,146 und 0,35: Aliasing-Wert höchstens Referenz + kalibrierte Toleranz (heute 15,9–64,3 bei einer Referenz von 1,1).
  - PSNR ≥ Schwelle gegen dieselbe Kernel-Referenz für Box, Triangle, Mitchell, Catmull-Rom und Lanczos3 auf allen synthetischen Bildern.
  - `gamma_rows` und `color_checker_1px` verkleinert: ± 2 zum Sollwert.
  - `alpha_disk`: kein grüner Saum.
  - Standardfilter per Sichtprüfung an `text`, `Bildschirmfotos` und Fotos festgelegt.

### Iteration 3 — LOD-Kette, SIMD-Reducer, Fortschritt

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

### Iteration 4 — Vergrößern

- Die Pipeline wird mit `widen = 1` für s > 1 genutzt, dieselben Kernel, Standard Catmull-Rom (Arbeitshypothese).
- Genau 100 % umgeht den Filter.
- Optional, nur wenn die Sichtprüfung es nahelegt: Ab einer einstellbaren Zoomstufe (etwa 400 %) wird automatisch Nearest benutzt, wie Bildbearbeitungen es zur Pixelinspektion tun. Standardmäßig aus.
- **Abnahme:**
  - `impulse` ×7 und ×8: Profil ≤ 1 Stufe Abweichung vom abgetasteten Kernel, symmetrisch, Maximum mittig — für **alle** Kernel inklusive Triangular und Bell.
  - `pixelart_32` mit Nearest bei ×2 bis ×8 **byteidentisch**.
  - `step_edge`: Überschwinger je Filter dokumentiert und plausibel.
  - PSNR ≥ Schwelle gegen die ImageMagick-Referenz bei 1,5 / 2,3 / 4.
  - Die kleinen Screenshots (261×462 usw.) sehen eingepasst mit Hochrechnen sauber aus.

### Iteration 5 — Pan & Zoom

- `ViewState` vollständig (`Custom`, `relativeScale`, `normalizedCenter`), Stufenleiter, Mausrad mit Magnet, Zoom um den Mauszeiger, Ziehen mit linker und mittlerer Taste, `Shift`+Pfeile, Doppelklick, `0`/`1`/`2`, `Strg`-Varianten, Seitentasten zum Blättern.
- `K` und `--keep-view` für Behalten oder Zurücksetzen, dazu `--zoom=` und `--center=` für reproduzierbare Aufnahmen.
- Zoom und Mitte stehen vorerst im Fenstertitel, ab Iteration 6 in der Info-Zeile.
- **Abnahme:**
  - `--selftest`: Zoom um einen Punkt lässt den Bildpunkt unter dem Zeiger auf ±0,5 px stehen. Klemmen: Ein Bild, das größer als das Fenster ist, zeigt nie einen Rand, ein kleineres bleibt zentriert. Behalten: Gleich große Bilder ergeben pixelidentische Ansichten, dasselbe Motiv in 4032 und 1008 ergibt denselben Ausschnitt.
  - Interaktiv per `xdotool` (Tasten, Rad, Ziehen): ← und → blättern auch gezoomt, `Shift+←` blättert **nicht**.
  - Beim Halten von → mit „Behalten“ bleibt die Ansicht stabil, ohne Springen.

### Iteration 6 — `final_ui`: GL3-Backend, Eingabe, Info-Zeile

- `fui_backend_gl3.h` (2.8). Eine GL3-Umschaltung in `FUI_Test` dient als Prüfstand.
- `fui_input_fpl.h`: Begin- und HandleEvent-Aufteilung.
- Font backen, `fuiContext` im Viewer, Hauptschleife umgebaut.
- **Info-Zeile** oben über die ganze Breite auf halbtransparentem Balken (benannte Konstanten für Höhe, Innenabstand und Deckkraft):
  - Links steht der **Dateiname** oder der **relative Pfad** zum geöffneten Ordner (`N` oder `--relative-path`). Bei Platzmangel wird **vorne** gekürzt (`…/unterordner/name.jpg`), weil der Name wichtiger ist als die Ordner. Das braucht eine kleine eigene Kürzung über `fuiMeasureText`, denn `fuiTruncateTextToWidth` kürzt hinten.
  - Rechtsbündig in derselben Zeile steht die **Originalgröße und bpp**: `4032 × 3024 · 24 bpp`. Dieser Teil hat Vorrang und wird nie gekürzt.
  - Die Zeile erscheint schon während des Ladens (aus `PictureInfo`).
  - Der Fortschrittsbalken rückt darunter.
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

## 7. Bewusst offen

- **EXIF-Orientierung:** 10 von 40 Stichproben-Fotos sind betroffen. Die Lösung wäre ein kleiner JPEG-APP1-Parser beim Header-Blick und die Orientierung als Achsentransformation in `ViewTransform`. Der Platz dafür ist reserviert, der Aufwand ist klein. **Empfehlung: direkt nach diesem Plan.**
- **`stb_image` aktualisieren** (v2.19 → aktuell): Das bringt Sicherheitskorrekturen für eine App, die beliebige Dateien öffnet. Danach die Benchmarks wiederholen.
- **VRAM-Budget:** 17 Slots × 65 MB = 1,1 GB sind auf der 3090 kein Thema, auf einer iGPU schon. Nachbarn jenseits von ±N bräuchten nur die Stufe in Bildschirmgröße, Stufe 0 erst beim Hinsehen. Die Kette aus Iteration 3 macht das möglich, es ist aber nicht Teil dieses Plans.
- **Absoluter Maßstab behalten** als dritter Modus (2.6).
- **Schachbrett-Hintergrund** für transparente Bilder: Heute ist er schwarz.
- **Neu zeichnen nur bei Bedarf:** Heute läuft die Schleife mit 60 Hz, auch wenn sich nichts ändert. Für eine eigenständige App gehört das in den Leerlauf.
- **NEON-Stufe:** Die Architektur ist vorbereitet (2.4). Der Aufwand sind die vier Zeilenfunktionen in `imagepyramid_arm.h`, eine Tabellenzeile und die Zuordnung zu `arm.hasNEON`. Geprüft wird per Cross-Compile und `--selftest` unter `qemu-aarch64`, dafür muss `aarch64-linux-gnu-gcc` noch installiert werden. Bis dahin läuft ARM skalar. Auch der übrige Viewer müsste auf ARM erst einmal übersetzt werden, bevor man sich darauf verlassen kann. Nur die SIMD-Seite ist hier schon ausgelegt.
- **Der erste Bildaufruf** ist heute nicht parallel: Ein Bild wird von einem Thread dekodiert, und nur die Nachbarn laufen parallel. Die LOD-Stufe 1 ließe sich in Zeilenblöcken auf mehrere Threads verteilen.
- **Glyphen außerhalb von U+00FF** (CJK usw.) erscheinen als Ersatzzeichen.
- **Legacy-GL-Pfad:** Er bleibt übersetzbar und zeigt Nearest/Bilinear/trilinear aus der Kette, ohne Qualitätszusage. Ob er für eine eigenständige App überhaupt bleiben soll, ist zu entscheiden.
- **Umzug nach `apps/`** und Einstellungen in einer Datei speichern: gehört zum App-Umbau, nicht zur technischen Basis.

---

## 8. Risiken

- **RGBA16F-Zwischentexturen bei 4K:** 3840 × Quellzeilen × 8 B im ersten Durchgang. Zusammen mit der Zieltextur sind bei s ≈ 0,5 rund 100–130 MB Arbeitsspeicher auf der GPU zu erwarten. Falls das stört, wird der erste Durchgang in Streifen zerlegt.
- **Treiberunterschiede beim sRGB-Framebuffer:** Auf NVIDIA ist er nachweislich aktiv (188 in 1.2). Auf Mesa oder Intel wird er beim Start über `GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING` geprüft. Ist er nicht sRGB-fähig, kodiert der letzte Kopiervorgang selbst.
- **SIMD-Bitidentität** hängt an reiner Ganzzahlarithmetik. Sobald irgendwo Gleitkomma hineinrutscht (z. B. eine Alpha-Division), muss sie in **allen** Stufen skalar und gleich laufen. Der Selbsttest fängt das ab.
- **AVX-512 nur mit F:** Falls BW-Befehle deutlich schneller wären, heißt das eine FPL-Änderung. Getestet werden kann alles lokal, denn der 7950X hat F, BW und VL.
- **ARM-Weiche auf Apple:** Wer an der einen Weiche vorbei `FPL_ARCH_ARM64` allein abfragt, bekommt auf einem Mac stillschweigend den skalaren Pfad. Das fällt nur über die Leistung auf, nie über ein falsches Ergebnis. Abhilfe ist die `grep`-Prüfung in der Abnahme von Iteration 3.
- **ImageMagick-Referenz:** Die Kernel-Definitionen müssen exakt übereinstimmen (Mitchell B = C = ⅓, Lanczos mit 3 Keulen, Catmull-Rom B = 0, C = ½). Eine Abweichung sieht aus wie ein Fehler im Viewer. Der Impulstest ist die unabhängige Gegenprobe.
- **Die Standardfilter** sind Arbeitshypothesen. Text in Bildschirmfotos kann die Wahl beim Verkleinern zu Mitchell oder Box verschieben, Fotos eher zu Lanczos3.
