# Plan: Farbige Row-Icons in Listbox und ListView (`final_ui.h`)

Ziel: Ein Sheet mit **farbigen** Icons vor den Zeilen einer Liste zeichnen — bisher ging über den GL1-Referenz-Backend nur einkanalige Coverage, die von der Vertexfarbe eingefärbt wird.

**Abschnitt 1 ist erledigt:** `fuiGL1UploadImageRGBA` in `demos/additions/fui_backend_gl1.h` lädt ein vierkanaliges Bild als Textur hoch, mit wählbarem Filter (linear für skalierte Artwork, nearest für Pixelart). Der Weg über `final_ui_adapter.h` konnte RGBA schon immer, weil `RenderPushTexture` die Kanalzahl als Parameter nimmt.

Dieses Dokument beschreibt die drei restlichen Schichten: was der **Aufrufer** tut (2), welcher **Widget-Aufruf** es ist (3), und was die **Bibliothek** damit macht bzw. wo sie dafür noch nachziehen sollte (4).

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

1. **Zweierpotenz in beiden Achsen.** Der GL1-Backend ist bewusst OpenGL 1.1 ohne Extension-Prüfung — non-power-of-two-Texturen sind dort nicht garantiert und kommen je nach Treiber schwarz heraus. Das Demo-Sheet ist 128×32 und damit zufällig richtig; ein Sheet aus 5 Zellen à 48px (240×48) wäre es nicht. → Auf 256×64 aufrunden und die überzähligen Zellen leer lassen.
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

`fuiSetDrawBatching` ist per Default **aus** und hilft hier ohnehin kaum: pro Zeile wechselt die Textur zwischen Icon-Sheet und Font-Atlas, und gemerged wird nur bei gleicher Textur und gleichem Clip (`final_ui.h:4904`). Eine Icon-Liste kostet also grob **zwei Draw-Commands pro sichtbarer Zeile** statt einem. Bei den ~20–40 Zeilen, die in einen Viewport passen, ist das egal — relevant erst, wenn `FUI_Performance` eine Icon-Variante bekommt.

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

**4.2.1 Inkonsistenz bei `rowIndex >= cellForRowCount`** — *klein, sollte gefixt werden*
Eine Zeile hinter dem Ende der Tabelle bekommt `rowRect` unverändert zurück, also **keine** Einrückung, während eine Zeile mit negativer Zelle eingerückt wird. In einer Liste, deren Tabelle kürzer ist als die Zeilenzahl, springt der Text ab da nach links. → Entweder auch dort einrücken, oder in der Doku festhalten, dass `cellForRowCount` immer der Zeilenzahl entsprechen soll.

**4.2.2 Tint pro Zeile** — *klein*
Heute `fuiColorRGBA(1,1,1,1)` fest verdrahtet (`final_ui.h:11089`). Ein `fuiColor tint` im Struct (0 = weiß) reicht schon für „Liste ausgegraut"; ein optionales `const fuiColor *tintForRow` für Zustandsfarben pro Zeile. Rückwärtskompatibel, weil `fplZeroInit` weiterhin das alte Verhalten ergibt — sofern eine transparente Null als „nicht gesetzt" gelesen wird.

**4.2.3 Padding aus dem Theme** — *klein*
`FUI__LIST_ICON_PADDING` ist ein 2.0f-Define. Bei einem Theme mit größeren Zeilen ist das optisch zu eng. → `theme->listIconPadding`.

**4.2.4 Icon in beliebiger Spalte** — *mittel*
`columnIndex == 0` in `final_ui.h:12461` durch ein Feld `int32_t column` in `fuiListIcons` ersetzen (Default 0). Zu klären: verträgt sich das mit der Button-Spalte (`rowButtons->column`), wenn beide auf dieselbe Spalte zeigen — dann gewinnt der Button, das Icon fällt weg. Braucht einen expliziten Vorrang und eine Zeile Doku.

**4.2.5 Mehrere Sheets / ungleich große Zellen** — *groß*
Heute genau ein Sheet mit gleichmäßigem Raster. Ein Sprung auf „Zelle ist ein Rechteck im Sheet" (`const fuiRect *cellRects` statt `columns`/`rows`) würde beliebige Atlanten erlauben, macht aber `fui__ListIconCellUv` und das Struct deutlich schwerer. → Nur angehen, wenn ein echter Anwendungsfall auftaucht; mehrere Sheets lassen sich vorher durch Zusammenkopieren in eins lösen.

**4.2.6 Icon-only-Modus und Icon im Spaltenkopf** — *mittel*
Beides ist heute unmöglich: der Text-Rect wird immer reserviert, und der Header zeichnet nur Strings. Für eine Werkzeugliste ohne Beschriftung bräuchte es ein Flag „kein Text-Rect", für Sortier-Pfeile im Kopf ein zweites Tabellenfeld `cellForColumn`.

### 4.3 Test und Nachweis

- `FUI_Test`: das prozedurale Sheet bleibt als Coverage-Beispiel, zusätzlich ein RGBA-Sheet über den neuen Upload, damit beide Pfade in einem Demo laufen.
- `FUI_Performance`: eine Icon-Variante der großen Liste, um den Command-Zuwachs aus 3.3 an Zahlen zu haben.
- Der Non-Power-of-Two-Fall aus 2.2 gehört einmal bewusst ausprobiert und dokumentiert, statt später als „Icons sind schwarz" wieder aufzutauchen.
