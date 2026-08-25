# Plan: Farbige Row-Icons in Listbox und ListView (`final_ui.h`)

Ziel: Ein Sheet mit **farbigen** Icons vor den Zeilen einer Liste zeichnen — bisher ging über den GL1-Referenz-Backend nur einkanalige Coverage, die von der Vertexfarbe eingefärbt wird.

**Abschnitt 1 ist erledigt:** `fuiGL1UploadImageRGBA` in `demos/additions/fui_backend_gl1.h` lädt ein vierkanaliges Bild als Textur hoch, mit wählbarem Filter (linear für skalierte Artwork, nearest für Pixelart). Der Weg über `final_ui_adapter.h` konnte RGBA schon immer, weil `RenderPushTexture` die Kanalzahl als Parameter nimmt.

Dieses Dokument beschreibt die drei restlichen Schichten: was der **Aufrufer** tut (2), welcher **Widget-Aufruf** es ist (3), und was die **Bibliothek** damit macht bzw. wo sie dafür noch nachziehen sollte (4).

---

## Stand

Umgesetzt sind 4.2.1, 4.2.2, 4.2.3, 4.2.4, 4.2.6 und der komplette Nachweis aus 4.3. **4.2.5 bleibt bewusst offen**, so wie der Plan es vorsieht — beliebige Zell-Rechtecke lohnen erst, wenn ein echter Anwendungsfall auftaucht.

`fuiListIcons` hat dadurch sechs neue Felder: `tint`, `tintForRow`, `column`, `iconOnly`, `cellForColumn` und `cellForColumnCount`. Alle sind rückwärtskompatibel, weil ein genullter Wert überall das alte Verhalten bedeutet — bei den Farben ist die Regel *voll transparent = nicht gesetzt*. Dazu kommt `fuiTheme.listIconPadding`, das den festen 2px-Define `FUI__LIST_ICON_PADDING` ersetzt.

Was sich beim Lesen des Codes noch gezeigt hat: der Doc-Kommentar `//! Draws one row's icon …` stand im Original über `fui__ListVisibleRange` statt über der Icon-Funktion. Beide haben jetzt ihre eigene Überschrift.

---

## 2. Aufrufer-Seite: das Sheet und die `fuiListIcons`-Tabelle

### 2.1 Woher das Sheet kommt

Drei Wege, alle schon im Repo vorhanden:

| Weg | Womit | Wann |
|---|---|---|
| Prozedural gemalt | eigener Pixel-Buffer, siehe `demos/FUI_Test/fui_test.c:198` | Demos, kein Asset zum Ausliefern |
| Aus Datei geladen | `TextureDataLoadFromFile` (`demos/additions/final_assets.h:206`) | echte Anwendung, PNG neben der Exe |
| Eingebettet | `TextureDataLoadFromMemory` (`final_assets.h:265`) | Single-File-Deploy |

Beide Loader zwingen stb_image auf `forcedComponentCount = 4`, liefern also genau das Layout, das `fuiGL1UploadImageRGBA` erwartet: RGBA, **straight alpha**, nicht premultipliziert.

```c
TextureData sheet = fplZeroInit;
if(TextureDataLoadFromFile(allocator, &sheet, "assets/list_icons.png")) {
	const bool useLinearFilter = true;
	uint32_t sheetTexture = 0;
	if(fuiGL1UploadImageRGBA(sheet.data, sheet.width, sheet.height, useLinearFilter, &sheetTexture)) {
		demo->iconSheet = (fuiTextureId)sheetTexture;
		demo->iconSheetSize = fuiV2((float)sheet.width, (float)sheet.height);
	}
	TextureDataFree(allocator, &sheet);
}
```

Die Pixel werden beim Upload kopiert, der `TextureData` darf also sofort danach weg. Am Ende `fuiGL1DeleteTexture(sheetTexture)`.

### 2.2 Wie das Sheet aussehen muss

Drei Regeln, und alle drei sind unbequem genug, dass sie in die Doku gehören:

1. **Zweierpotenz in beiden Achsen.** Der GL1-Backend ist bewusst OpenGL 1.1 ohne Extension-Prüfung — non-power-of-two-Texturen sind dort formal nicht garantiert. Das Demo-Sheet ist 128×32 und damit richtig; ein Sheet aus 5 Zellen à 48px (240×48) wäre es nicht. → Auf 256×64 aufrunden und die überzähligen Zellen leer lassen. **Nachgemessen** (siehe 4.3): auf einem aktuellen Treiber geht 240×48 einwandfrei durch, weil die Extension seit OpenGL 2.0 Kern ist. Die Regel bleibt als Gewohnheit für wirklich alte Implementierungen sinnvoll, ist aber nichts, woran heute jemand scheitert.
2. **Der Rand gehört INS Zelleninnere.** `fui__ListIconCellUv` (`final_ui.h:11003`) teilt das Sheet in ein exaktes Raster, `uv = spalte/columns .. (spalte+1)/columns`. Es gibt keinen Gutter und kein Halbtexel-Inset. Mit linearem Filter greift die Randsample also in die Nachbarzelle. → Artwork mit ein bis zwei Texeln Luft innerhalb der eigenen Zelle zeichnen, nicht Zellen auseinanderrücken.
3. **Zellgröße auf die Zeilenhöhe abstimmen.** Die Icon-Box ist `rowHeight - 2 * FUI__LIST_ICON_PADDING`, mit `rowHeight = theme->menuItemHeight * rowScale`. Bei Default-Theme (`menuItemHeight = 24`, `final_ui.h:4613`) und `rowScale = 2.0` sind das **44 px**; mit `rowScale = 1.5` sind es **32 px**. Zellen kleiner als die Box werden hochskaliert und matschen.

### 2.3 Die `cellForRow`-Tabelle

```c
fuiListIcons icons = fplZeroInit;
icons.sheet         = demo->iconSheet;
icons.sheetSize     = demo->iconSheetSize;   // nur fürs Aspect-Fit, 0 => Icon füllt die Box quadratisch
icons.columns       = 4;
icons.rows          = 2;                      // Zellindex = spalte + zeile * columns
icons.cellForRow    = demo->iconForRow;       // int32_t je Eintrag
icons.cellForRowCount = rowCount;
icons.rowScale      = 1.5f;                   // 0 => 2.0

// Alles ab hier ist optional, genullt ergibt jedes Feld das alte Verhalten
icons.tint          = fuiColorRGBA(1, 1, 1, 0.5f);  // ganzes Sheet, voll transparent => nicht gesetzt
icons.tintForRow    = demo->iconTintForRow;         // so lang wie cellForRow
icons.column        = 1;                            // nur ListView, 0 ist die erste Spalte
icons.iconOnly      = false;                        // true => kein Text-Rect neben dem Icon
icons.cellForColumn = demo->iconForColumn;          // ein Icon je Spaltenkopf, negativ => keins
icons.cellForColumnCount = columnCount;
```

- Indiziert wird mit dem **Quell-Zeilenindex des Aufrufers**, nicht mit der Anzeigeposition (`final_ui.h:12411` löst über `fui__ListSourceRow` auf). Beim Sortieren wandern die Icons also von allein mit — die Tabelle muss nicht mitsortiert werden.
- **Negativer Eintrag** = diese Zeile hat kein Icon, rückt den Text aber trotzdem ein, damit die Beschriftungsspalte eine gerade Kante behält.
- Die Bibliothek **kopiert nichts** — weder die Tabelle noch die Zellen-Strings. Beide müssen leben, solange die Liste gebaut wird.
- Das `fuiListIcons`-Struct selbst darf ein Stack-Temporary sein; es wird nur während des Aufrufs gelesen.

### 2.4 Wann das Handle steht

Bei einem Renderer mit **verzögertem Upload** (`RenderPushTexture` stellt nur in die Queue) ist das Texture-Handle zur Init-Zeit noch null, und eine Liste, die dann gebaut wird, zeichnet gar keine Icons. Muster dafür steht in `demos/FUI_Adapter_FPL/fui_adapter_fpl.c:918` (`WireQueuedTextures`): am Anfang jedes Frames prüfen und nach dem ersten echten Handle nichts mehr tun.

---

## 3. Der Widget-Aufruf

### 3.1 Welche Funktion

| Ohne Icons | Mit Icons |
|---|---|
| `fuiListBox` | `fuiListBoxEx` (`final_ui.h:3972`) |
| `fuiListView` | `fuiListViewEx` (`final_ui.h:4189`) |
| — | `fuiListViewButtons` (`final_ui.h:4208`), wenn zusätzlich eine Button-Spalte gebraucht wird |

Die Kurzformen reichen intern nur `fui_null` als Icons durch, es gibt also keinen Verhaltensunterschied außer dem Icon selbst. `fuiFileDialog` und `fuiFileBrowser` nehmen dasselbe Struct entgegen — dort ist die Tabelle typischerweise „0 = Ordner, 1 = Datei".

### 3.2 Was sich durch Icons am Verhalten ändert

- **Die Zeilenhöhe wächst** auf `menuItemHeight * rowScale`, in der ListView aber **nicht die Kopfzeile** (`final_ui.h:12272`). Ein Layout, das die sichtbaren Zeilen ausrechnet, muss das nachziehen.
- **Nur Spalte 0** der ListView bekommt ein Icon (`final_ui.h:12461`), an derselben Stelle, an der die Listbox es zeichnet.
- Auswahl, Doppelklick (`outWasActivated`) und Sortierung bleiben unberührt.

### 3.3 Batching

`fuiSetDrawBatching` ist per Default **aus** und hilft hier ohnehin kaum: pro Zeile wechselt die Textur zwischen Icon-Sheet und Font-Atlas, und gemerged wird nur bei gleicher Textur und gleichem Clip (`final_ui.h:4904`). Eine Icon-Liste kostet also grob **zwei Draw-Commands pro sichtbarer Zeile** statt einem.

**Gemessen** (`FUI_Performance --benchmark`, 1600×940, `rowScale = 1.0` damit beide Fälle exakt gleich viele Zeilen zeigen — 36 sichtbare Zeilen):

| Fall | Commands | Vertices | Median ms |
|---|---|---|---|
| `listview 100K` | 302 | 16300 | 0.037 |
| `listview 100K icons` | **338** | 16759 | 0.039 |
| `listview 100K icn bat` | 332 | 16759 | 0.039 |
| `listbox 50K` | 41 | 7024 | 0.015 |
| `listbox 50K icons` | **77** | 7168 | 0.016 |
| `listbox 50K icn bat` | 73 | 7168 | 0.016 |

Die Schätzung stimmt auf den Command genau: **+36 Commands bei 36 sichtbaren Zeilen**, also exakt einer pro Zeile. Bei der Listbox ist das buchstäblich die Verdopplung aus der Vermutung (41 → 77, eine Zeile war vorher ein einziger Text-Command); beim ListView mit sieben Spalten sind es dagegen nur ~12 % obendrauf, weil eine Zeile dort schon vorher ~8 Commands kostete.

Batching bringt fast nichts, genau wie vermutet: 338 → 332 und 77 → 73, also ganze 6 bzw. 4 gemergte Commands, weil die Textur pro Zeile hin und her springt. Die Bauzeit bewegt sich um +0.002 ms und damit im Rauschen.

---

## 4. Bibliotheks-Seite: was sie tut, und wo sie nachziehen sollte

### 4.1 Der bestehende Pfad

`fui__ListBoxDrawRowIcon` (`final_ui.h:11053`) macht in dieser Reihenfolge:

1. Abbruch, wenn kein Sheet, keine Tabelle, oder `rowIndex >= cellForRowCount`
2. Bei negativer Zelle: nur das eingerückte Text-Rechteck zurückgeben
3. UVs aus dem Raster, Aspect-Fit der Zelle in die quadratische Box, zentriert
4. `fuiDrawImage` mit **hart weißem Tint**
5. Rückgabe des Rest-Rechtecks für den Text

Mit einem RGBA-Sheet ist Schritt 4 genau richtig: weiß moduliert die Textur nicht.

### 4.2 Kandidaten für Änderungen (nach Aufwand sortiert)

**4.2.1 Inkonsistenz bei `rowIndex >= cellForRowCount`** — *erledigt*
Eine Zeile hinter dem Ende der Tabelle bekam `rowRect` unverändert zurück, also **keine** Einrückung, während eine Zeile mit negativer Zelle eingerückt wird. In einer Liste, deren Tabelle kürzer ist als die Zeilenzahl, sprang der Text ab da nach links. Beides ist jetzt derselbe Fall: kein Icon, aber die Einrückung, die die Beschriftungsspalte gerade hält.

**4.2.2 Tint pro Zeile** — *erledigt*
War `fuiColorRGBA(1,1,1,1)` fest verdrahtet. Jetzt gibt es `fuiColor tint` fürs ganze Sheet und `const fuiColor *tintForRow` für Zustandsfarben pro Zeile, aufgelöst in `fui__ListIconSheetTint` und `fui__ListIconRowTint`. Die Regel ist wie vorgeschlagen **voll transparent = nicht gesetzt**, also ergibt `fplZeroInit` weiterhin exakt das alte Verhalten. `tintForRow` ist so lang wie `cellForRow` und teilt sich dessen `cellForRowCount`.

Im Demo hängt der Tint an der Zeile, die gerade „spielt" — die trägt die Akzentfarbe, alle anderen Einträge bleiben transparent.

**4.2.3 Padding aus dem Theme** — *erledigt*
`FUI__LIST_ICON_PADDING` ist weg, `fuiTheme.listIconPadding` ist an seine Stelle getreten und steht in `fuiDefaultTheme` auf denselben 2.0f. `fui__ListIconPadding` klemmt negative Werte auf null, damit ein von Hand zusammengebautes Theme nichts kaputt macht.

**4.2.4 Icon in beliebiger Spalte** — *erledigt*
`columnIndex == 0` ist durch `icons->column` ersetzt, Default 0. Die offene Frage ist so entschieden: eine Spalte **ausserhalb** der Liste heisst „diese Liste hat keine Icons" — dieselbe Prüfung, die `rowButtons->column` schon immer bekam — statt still auf Spalte 0 zurückzufallen. Zeigen Icons und Buttons auf dieselbe Spalte, **gewinnt der Button**, weil er die ganze Zelle ersetzt; die Zeilen behalten trotzdem ihre Icon-Höhe, damit eine Liste über einen Aufruferfehler nicht die Form wechselt. Steht so am `fuiListViewButtons`-Doxygen.

**4.2.5 Mehrere Sheets / ungleich große Zellen** — *groß*
Heute genau ein Sheet mit gleichmäßigem Raster. Ein Sprung auf „Zelle ist ein Rechteck im Sheet" (`const fuiRect *cellRects` statt `columns`/`rows`) würde beliebige Atlanten erlauben, macht aber `fui__ListIconCellUv` und das Struct deutlich schwerer. → Nur angehen, wenn ein echter Anwendungsfall auftaucht; mehrere Sheets lassen sich vorher durch Zusammenkopieren in eins lösen.

**4.2.6 Icon-only-Modus und Icon im Spaltenkopf** — *erledigt*
`bool iconOnly` reserviert keinen Text-Rect mehr — in der Listbox ist das die ganze Zeile, im ListView die Icon-Spalte allein. Eine so gezeichnete Listbox braucht gar keine `items` mehr; der ListView braucht seine `cells` weiter, weil die anderen Spalten davon leben. Das Icon bleibt dabei, wo es auch mit Text sitzt, damit das Umschalten nichts verrückt.

`cellForColumn` / `cellForColumnCount` setzen ein Icon vor einen Spaltentitel, gezeichnet von `fui__ListDrawHeaderIcon` in Kopfzeilenhöhe. Die Tabelle wird eigenständig gelesen, eine Liste kann also Icons im Kopf tragen und keine in den Zeilen. Ein Kopf ohne Icon wird **nicht** eingerückt: zwei Titel in verschiedenen Spalten haben keine gemeinsame Kante, an der sie ausgerichtet werden müssten. Der Sortierpfeil sitzt rechts in der Zelle und kommt dem Icon nicht in die Quere.

### 4.3 Test und Nachweis — *erledigt*

**`FUI_Test`** zeichnet jetzt beide Sheets. Die vier Formen liegen in `DemoIconTexelIsInk`, damit Coverage- und Farbversion garantiert dasselbe Bild sind und sich nur im Anstrich unterscheiden. `DemoDrawIconSheet` malt weiter einkanalig und geht über `fuiGL1UploadFontAtlas`, `DemoDrawColorIconSheet` malt vierkanalig und geht über `fuiGL1UploadImageRGBA`. Die Farbzellen tragen einen senkrechten Verlauf von einer hellen Oberkante zur vollen Kindfarbe — genau das, was ein Vertex-Tint prinzipiell nicht kann, und damit der eigentliche Beweis, dass der RGBA-Weg trägt.

Über der Entity-Tabelle sitzt eine Schalterreihe, ein Schalter je neuem Feld: **RGBA sheet** (Sheet-Wechsel), **Header icons** (`cellForColumn`), **On Kind** (`column`) und **Icons only** (`iconOnly`). `tintForRow` läuft dauerhaft mit und färbt die spielende Zeile.

**`FUI_Performance`** hat die Icon-Variante samt Zahlen — siehe die Tabelle in 3.3. Toolstrip-Toggle **Icons** plus Menüeintrag, vier neue Benchmark-Fälle (`listview 100K icons`, `listbox 50K icons` und je eine gebatchte Zwillingsvariante). Die Icon-Tabelle hängt an `PerfDataSet` und wird mit den Zeilen erzeugt; die Listbox liest den vorderen Teil derselben Tabelle, das kostet also keine zweite Allokation. Im Headless-Lauf gibt es keine Textur, dort steht ein Platzhalter-Handle — es muss nur ungleich null und ungleich dem Atlas-Handle sein, sonst würde der Batching-Fall Icon und Label zusammenlegen und die Zählung verfälschen.

**Der Non-Power-of-Two-Fall** wurde ausprobiert, und das Ergebnis widerspricht der Vermutung aus 2.2. Eine Sonde hat je ein 128×32-, ein 240×48- und ein 256×48-Sheet durch `fuiGL1UploadImageRGBA` geschickt, jedes auf ein Quad gezeichnet und den Mitteltexel zurückgelesen:

```
GL_VERSION  : 4.6.0 NVIDIA 580.173.02
power of two    128x32   uploaded=1  uploadError=0x0000  drawError=0x0000  centerTexel=(255,0,0,255)
npot            240x48   uploaded=1  uploadError=0x0000  drawError=0x0000  centerTexel=(255,0,0,255)
npot one axis   256x48   uploaded=1  uploadError=0x0000  drawError=0x0000  centerTexel=(255,0,0,255)
```

Alle drei kommen korrekt heraus, kein `glGetError`, nichts schwarz. `GL_ARB_texture_non_power_of_two` ist seit OpenGL 2.0 Kernbestandteil, und ein Compatibility-Context liefert es mit — auf irgendeinem Desktop-Treiber von heute ist das kein Problem. Der Treiber liess sich hier auch nicht zum Versagen überreden: die Mesa-Overrides greifen am proprietären NVIDIA-GLX nicht.

Damit ist die Aufrundungsregel eine **Portabilitätsgewohnheit für echte OpenGL-1.1-Implementierungen** und kein Fehler, den jemand real trifft. So steht sie jetzt auch an `fuiGL1UploadImageRGBA` und in 2.2 — statt als „Icons sind schwarz" wiederzukommen, ist sie als das dokumentiert, was sie ist. Die Sonde selbst war ein Wegwerfprogramm und liegt nicht im Repo.
