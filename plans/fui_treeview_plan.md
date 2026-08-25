# Plan: TreeView-Widget für `final_ui.h`

Ziel: Ein Widget für **hierarchische Items** — Ordner/Datei-Explorer, Szenengraph, Layer-Baum, Asset-Browser. Es soll sich anfühlen wie `fuiListBox`/`fuiListView` (gleiche Zeilenhöhe, gleiche Icons, gleiches Scrollen, gleiche Auswahl-Semantik) und es soll bei einer Million Knoten genauso wenig kosten wie die Liste seit v0.9.3.

Dieses Dokument beschreibt vier Dinge: die **Designentscheidungen** (1), die **öffentliche API** (2), den **inneren Aufbau** (3) und die **Phasen** samt Demo- und Performance-Arbeit (4–7).

---

## Stand

**Phasen 1 bis 7 sind umgesetzt**, einschließlich Demo-Panel und Performance-Szenario. Was beim Bauen anders lief als geplant:

- **`fuiTreeComputeDescendants` ist in Phase 1 gerutscht**, weil das Widget ohne den Helfer nicht testbar ist — der Aufrufer müsste die Teilbaumgrößen sonst von Hand eintragen.
- **Ein Fallstrick, der im Plan fehlte:** `fuiBeginStackAt` schiebt einen Id-Scope. `fuiTreeInvalidate` und `fuiTreeReveal` aus einem Button *innerhalb* einer Stack-Zeile heraus lösen die Id also in einem anderen Scope auf und tun still gar nichts. Der Performance-Tab merkt sich deswegen nur, was der Button wollte, und handelt erst nach `fuiEndStack`. Das steht jetzt so in der Doku beider Funktionen und als Kommentar im Demo-Code.
- **Der Tiefen-Fall war zuerst falsch geschnitten.** Eine reine Kette hat nirgends ein Geschwister, und eine Führungslinie wird nur gezeichnet, wo eins folgt — die Kette hätte also zwei Linien je Zeile gezeichnet, egal wie tief sie ist, und damit nichts gemessen. Sie ist jetzt eine *Leiter*: auf jeder Ebene geht ein Knoten weiter nach unten und ein Blatt steht daneben. Damit zeichnet die tiefste Zeile ihre 63 Linien wirklich, und der Fall ging von 144 auf 705 Draw-Commands.
- **`demos/FUI_Adapter_FPL` spiegelt den Baum ebenfalls.** Der Final-Framework-Weg über `final_ui_adapter.h` lebt, also darf er nicht hinter FUI_Test zurückfallen. Zwei Unterschiede zur FUI_Test-Fassung, beide durch den Adapter bedingt: das Sheet ist dort einkanalige Coverage, Ordner und Dateien werden also über `fuiListIcons.tintForRow` eingefärbt statt über gemalte Farbe; und die Fenstergröße steht nicht in der `GameConfiguration`, das Demo verlangt sie in `GameInit` über `fplSetWindowSize`.
- **Was bewusst offen blieb**, steht unverändert in Abschnitt 8.

Die gemessenen Zahlen stehen in 7.4.

---

## 1. Designentscheidungen

Vier Fragen entscheiden alles Weitere. Jede hat mehrere vertretbare Antworten, deswegen steht hier jeweils dabei, warum es diese wurde.

### 1.1 Wie beschreibt der Aufrufer den Baum?

| Variante | Wie | Dagegen spricht |
|---|---|---|
| **A: Preorder-Array mit Tiefe** | `{ label, depth, descendantCount }`, flach, in Anzeigereihenfolge | Aufrufer muss flach halten |
| B: Begin/End rekursiv | `if(fuiBeginTreeNode(..)) { .. fuiEndTreeNode(..); }` | Ein offener Baum mit 100 K sichtbaren Knoten wird jeden Frame komplett durchlaufen — keine Virtualisierung möglich |
| C: Callback-Modell | Aufrufer gibt `childCount`/`childAt`-Callbacks | Callback pro Zeile, pro Frame; passt zu keinem anderen Widget dieser Bibliothek |

**Es wird A.** Das ist dasselbe Modell, das `fuiListView` schon hat: der Aufrufer besitzt ein Array, die Bibliothek liest es und kopiert nichts. Ein Ordnerbaum fällt beim rekursiven Einlesen ohnehin als Preorder an. B ist bequemer für 30 Knoten und wird als bewusst offener Punkt in 8. geführt — es lässt sich später auf denselben Zeichenhelfern aufsetzen.

### 1.2 Wem gehört der Auf-/Zugeklappt-Zustand?

**Dem Aufrufer**, als `bool *isExpanded` parallel zum Knoten-Array.

Das ist die Regel des Hauses — `DemoState` sagt es wörtlich: *„Die Bibliothek hält NICHTS davon"*. Eine Checkbox bekommt `bool *value`, eine Liste `int32_t *selectedIndex`, der Baum bekommt sein Flag-Array. Die Alternative — ein Id-basiertes Hash-Set im Widget-State — bräuchte eine Id je Knoten, wäre bei einer Million Knoten unbegrenzt groß und würde beim Neuladen der Daten stillschweigend falsche Knoten offen halten.

Nebeneffekt, der viel wert ist: *Alle ausklappen*, *Alle zuklappen*, *Zustand speichern/laden* sind damit Aufrufer-Code über ein `bool`-Array und brauchen keine API.

### 1.3 Wie wird virtualisiert?

Der Baum hält im Widget-State einen **Sichtbarkeitsindex**: ein `int32_t`-Array, das je Anzeigezeile den Quell-Knotenindex nennt. Gebaut wird er nur, wenn er ungültig ist — genau wie `fuiWidgetState.sortOrder` der ListView (`final_ui.h:1846`).

Warum überhaupt ein Cache: um beim Scrollstand *S* die erste sichtbare Zeile zu finden, muss man von oben zählen, welche Knoten sichtbar sind. Ohne Cache ist das O(n) pro Frame — exakt der Fehler, den v0.9.3 bei Liste und ListView beseitigt hat („1,47 ms → 0,05 ms"). Mit Cache ist es `visible[scroll / rowHeight]`, also O(1).

`descendantCount` macht den **Neuaufbau** billig: ein zugeklappter Knoten wird mit `i += 1 + descendantCount` in einem Schritt übersprungen statt Kind für Kind. Ein Baum mit einer Million Knoten und zwölf offenen Wurzeln kostet damit zwölf Schleifendurchläufe statt einer Million.

### 1.4 Wie sieht der Aufruf aus?

Wie bei den Listen: eine **kurze Form** für den Normalfall und eine **Ex-Form mit Desc-Struct** für alles andere. `fuiListBox`/`fuiListBoxEx` und `fuiImage`/`fuiImageDesc` sind die Vorbilder.

---

## 2. Öffentliche API

### 2.1 Der Knoten

```c
/**
* @struct fuiTreeNode
* @brief Ein Knoten des Baumes, in PREORDER - also genau in der Reihenfolge, in der die Zeilen erscheinen, wenn alles offen ist.
*/
typedef struct fuiTreeNode {
	//! Was die Zeile sagt
	const char *label;
	//! Wie tief der Knoten sitzt, null ist eine Wurzel
	int32_t depth;
	//! Wie viele Knoten des eigenen Teilbaums IM ARRAY direkt hinter diesem folgen. Null ist ein Blatt
	int32_t descendantCount;
} fuiTreeNode;
```

`descendantCount` füllt der Aufrufer nicht von Hand, sondern mit `fuiTreeComputeDescendants` (2.4) — ein linearer Durchlauf nach dem Einlesen. Ob ein Knoten Kinder hat, steht damit schon da; ein zusätzliches `hasChildren` gibt es nicht.

16 Bytes je Knoten. Eine Million Knoten sind 16 MB plus die Beschriftungen — dieselbe Größenordnung wie die Zellen der ListView im Workbench.

### 2.2 Die Beschreibung

```c
typedef struct fuiTreeDesc {
	//! Die Knoten in Preorder, im Besitz des Aufrufers und NICHT kopiert
	const fuiTreeNode *nodes;
	//! Wie viele Knoten nodes hält
	int32_t nodeCount;
	//! Ein Flag je Knoten, vom Widget geschrieben, wenn ein Expander geklickt wird. Null ist ein Baum, der immer ganz offen steht
	bool *isExpanded;
	//! Das Icon-Sheet @ref fuiListIcons, indiziert mit dem KNOTENINDEX. Null ist ein Baum aus reinem Text
	const fuiListIcons *icons;
	//! Wie weit eine Ebene gegenüber ihrer Elternebene einrückt. Null nimmt fuiTheme.treeIndentWidth
	float indentWidth;
	//! Führungslinien von einem Elternknoten zu seinen Kindern zeichnen
	bool showGuides;
	//! Ein EINFACHER Klick meldet schon eine Aktivierung, statt erst der Doppelklick
	bool activateOnSingleClick;
	//! Die Pfeiltasten bewegen die Auswahl, solange der Baum den Fokus hat
	bool keyboardIsEnabled;
} fuiTreeDesc;
```

Genullt ergibt jedes Feld das schlichteste Verhalten — dieselbe Regel wie bei `fuiImageDesc` und `fuiListIcons`.

### 2.3 Was ein Build zurückgibt

```c
typedef struct fuiTreeAction {
	//! OUT: Welcher Knoten aktiviert wurde (Doppelklick oder Enter), minus eins wenn keiner
	int32_t activatedNode;
	//! OUT: Welcher Knoten diesen Frame auf- oder zugeklappt wurde, minus eins wenn keiner
	int32_t toggledNode;
	//! OUT: Auf welchen Knoten die rechte Maustaste ging, minus eins wenn auf keinen. Das ist der Knoten, den ein Kontextmenü meint
	int32_t contextNode;
} fuiTreeAction;
```

Vorbild ist `fuiListRowButtons.clickedRow` (`final_ui.h:4180`): ein Struct mit OUT-Feldern, das direkt nach dem Aufruf gelesen wird wie ein Rückgabewert. `contextNode` ist drin, weil ein Explorer ohne Rechtsklick-auf-Zeile keiner ist und der Aufrufer sonst selbst treffen müsste, was das Widget schon getroffen hat.

### 2.4 Die Funktionen

```c
//! Der Baum, kurze Form
fui_api bool fuiTreeView(fuiContext *context, const fuiRect rect, const char *id, const fuiTreeNode *nodes, const int32_t nodeCount, bool *isExpanded, int32_t *selectedIndex);

//! Der Baum, mit allem
fui_api bool fuiTreeViewEx(fuiContext *context, const fuiRect rect, const char *id, const fuiTreeDesc *desc, int32_t *selectedIndex, fuiTreeAction *outAction);
```

Beide geben `true` an dem Frame zurück, an dem sich die Auswahl geändert hat — dieselbe Zusage wie `fuiListBox`. `selectedIndex` ist immer der **Knotenindex des Aufrufers**, nie eine Anzeigeposition; Zuklappen verschiebt also, wo eine Zeile gezeichnet wird, und nie, was sie ist. (Ist der ausgewählte Knoten zugeklappt worden, bleibt er ausgewählt und ist nur nicht sichtbar — das ist, was ein Explorer auch tut.)

Reine Helfer, ohne Kontext, die auf den Arrays des Aufrufers arbeiten:

```c
//! Füllt descendantCount aus depth, in einem Durchlauf. Nach jedem Neuaufbau der Knoten aufrufen
fui_api void fuiTreeComputeDescendants(fuiTreeNode *nodes, const int32_t nodeCount);

//! Klappt jeden Vorfahren eines Knotens auf, damit er sichtbar werden KANN
fui_api bool fuiTreeExpandToNode(const fuiTreeNode *nodes, const int32_t nodeCount, bool *isExpanded, const int32_t nodeIndex);

//! Klappt den ganzen Baum auf oder zu
fui_api void fuiTreeSetExpandedAll(const fuiTreeNode *nodes, const int32_t nodeCount, bool *isExpanded, const bool expandedValue);

//! Der Elternknoten, oder minus eins für eine Wurzel
fui_api int32_t fuiTreeParentOf(const fuiTreeNode *nodes, const int32_t nodeCount, const int32_t nodeIndex);
```

Und drei, die den Widget-State meinen und darum — wie `fuiListViewSetSort` — im selben Id-Scope wie das Widget stehen müssen:

```c
//! Scrollt so, dass ein Knoten im Kasten steht. Wirkt beim NÄCHSTEN Build des Baumes
fui_api void fuiTreeReveal(fuiContext *context, const char *id, const int32_t nodeIndex);

//! Wirft den Sichtbarkeitsindex weg. Nur nötig, wenn ein Baum mit mehr als FUI_TREE_VERIFY_NODES Knoten seine Flags SELBST geändert hat
fui_api void fuiTreeInvalidate(fuiContext *context, const char *id);

//! Wie viele Zeilen der Baum gerade zeigen würde, für eine Statuszeile oder ein eigenes Layout
fui_api int32_t fuiTreeGetVisibleCount(fuiContext *context, const char *id);
```

### 2.5 Theme und Defines

| Neu | Default | Wofür |
|---|---|---|
| `fuiTheme.treeIndentWidth` | `16.0f` | Einrückung je Ebene |
| `fuiTheme.treeExpanderSize` | `10.0f` | Kantenlänge des Dreiecks, quadratische Trefferfläche ist die Zeilenhöhe |
| `fuiTheme.treeGuideColor` | wie `panelBorderColor`, halbtransparent | Führungslinien |
| `FUI_MAX_TREE_DEPTH` | `64` | Tiefe, bis zu der Führungslinien und Helfer arbeiten |
| `FUI_TREE_VERIFY_NODES` | `4096` | Bis hierher merkt der Baum selbst, dass Flags sich geändert haben |
| `FUI_MAX_TREE_NODES` | `1000000` | Obergrenze dessen, was ein Baum den Kontext allokieren lässt |

Die letzten beiden sind eins zu eins `FUI_LIST_SORT_VERIFY_ROWS` und `FUI_MAX_SORTABLE_ROWS` nachgebaut, inklusive Begründung: ein kurzer Baum prüft sich selbst, ein langer verlangt eine ausdrückliche Ansage.

---

## 3. Innerer Aufbau

### 3.1 Neue Felder in `fuiWidgetState`

```c
	//! Ein Quell-Knotenindex je Anzeigezeile, aus der Kontext-Arena, oder null solange nichts gebaut wurde
	int32_t *treeVisibleNodes;
	//! Eine Bitmaske je Anzeigezeile: welche Ahnenebene noch ein späteres Geschwister hat, und also eine Linie braucht. Nur belegt, wenn Führungslinien an sind
	uint64_t *treeGuideMasks;
	//! Wieviel Platz beide Arrays haben, was nur wächst
	int32_t treeVisibleCapacity;
	//! Wie viele Zeilen wirklich sichtbar sind
	int32_t treeVisibleCount;
	//! Woraus der Index gebaut wurde, per ADRESSE verglichen
	const void *treeNodesAddress;
	const void *treeExpandedAddress;
	int32_t treeSourceNodeCount;
	//! Hash der Klapp-Flags, für einen Baum, der kurz genug ist ihn jeden Frame zu nehmen
	fuiId treeExpandFingerprint;
	bool treeExpandWasFingerprinted;
	//! Ob der Index überhaupt fertig ist, was eine Invalidierung löscht
	bool treeVisibleIsBuilt;
	//! Welcher Knoten beim nächsten Build ins Bild gescrollt werden soll, minus eins für keiner
	int32_t treeRevealNode;
```

**Kosten, ehrlich benannt:** `fuiWidgetState` ist ein Struct für *alle* Widgets, jeder Slot wächst also um ~56 Bytes, auch der eines Buttons. Das Haus macht das schon so (`columnWidths[12]` sind allein 48 Bytes), und die Tabelle wächst nach Bedarf statt fest zu sein — trotzdem gehört es in die Changelog-Zeile.

### 3.2 Der Sichtbarkeitsindex

Aufbau, ein Durchlauf:

```c
int32_t visibleCount = 0;
int32_t nodeIndex = 0;
while(nodeIndex < nodeCount) {
	visibleNodes[visibleCount] = nodeIndex;
	visibleCount += 1;

	const fuiTreeNode *node = &nodes[nodeIndex];
	bool hasChildren = (node->descendantCount > 0);
	bool isOpen = hasChildren && (isExpanded == fui_null || isExpanded[nodeIndex]);
	if(isOpen) {
		nodeIndex += 1;
	} else {
		nodeIndex += 1 + node->descendantCount;
	}
}
```

Kosten: O(sichtbare Zeilen + zugeklappte Wurzeln). Ein zugeklappter Baum kostet so viel wie seine Wurzeln, egal wie viel darunter hängt — das ist die Zusage, die das Performance-Szenario in 7. nachweisen muss.

**Wann er neu gebaut wird.** Ungültig ist er, wenn eins davon nicht mehr stimmt: die Adresse von `nodes`, die Adresse von `isExpanded`, `nodeCount` — oder, bei bis zu `FUI_TREE_VERIFY_NODES` Knoten, der Hash der Flags. Ein Klick auf einen Expander setzt `treeVisibleIsBuilt = false` selbst, der übliche Weg braucht also gar keinen Hash. Übrig bleibt der Aufrufer, der die Flags *selbst* umlegt (Alles auf/zu, Zustand geladen) — für einen langen Baum ist das `fuiTreeInvalidate`. Genau dieselbe Arbeitsteilung wie bei `fuiListViewInvalidateSort`, und die Demo führt sie vor, statt sie zur Falle werden zu lassen.

**Speicher.** Über `fui__ArenaPushExact` wie `fui__EnsureListSortOrder` (`final_ui.h:12115`), Kapazität in Zweierpotenzen ab 256, Deckel `FUI_MAX_TREE_NODES`. Die Arena gibt nichts zurück, ein Wachsen lässt den alten Block also liegen — deswegen Zweierpotenzen und nicht exakte Größen.

**Kein Speicher.** Schlägt die Allokation fehl, fällt der Baum auf den ungecachten Durchlauf zurück: von oben zählen, bis der Scrollstand erreicht ist. Das ist langsam und korrekt, statt schnell und leer — dieselbe Haltung wie beim OOM-Sink der Arena.

### 3.3 Eine Zeile

Höhe: `theme->menuItemHeight`, mit Icons `theme->menuItemHeight * icons->rowScale` — die Regel der Listbox, unverändert.

Aufbau von links nach rechts:

```
[ depth * indentWidth ][ Expander ][ Icon ][ Beschriftung .................. ]
```

- **Expander** nur, wenn `descendantCount > 0`. Gezeichnet mit `fuiDrawCollapseGlyph` (`final_ui.h:7040`) — dasselbe Dreieck, das eine eingeklappte Panel-Titelleiste trägt, also nichts Neues zu gestalten. Die Trefferfläche ist ein Quadrat in Zeilenhöhe und **schluckt den Klick**: ein Klick auf den Expander klappt um und wählt *nicht* aus. Vorbild ist die Button-Spalte der ListView, wo „der Button diesen Klick besitzt".
- **Icon** über `fui__ListBoxDrawRowIcon` (`final_ui.h:11173`), mit dem Knotenindex als Zeilenindex und dem bereits eingerückten Rechteck. Damit gelten alle Regeln aus `fui_list_icons_plan.md` unverändert weiter, inklusive negativer Zelle = kein Icon, aber Einrückung. Es entsteht **kein** neuer Icon-Code.
- **Beschriftung** über `fui__DrawTextInRect`, das seit v0.9.3 die Glyphen außerhalb des Clips ohnehin überspringt.

Auswahl- und Hover-Wisch über die ganze Zeilenbreite, `menuHighlightColor` und `widgetHoveredColor` wie in der Listbox.

### 3.4 Führungslinien

Die Maske je Zeile fällt beim Aufbau des Index gratis an: beim Preorder-Durchlauf ist bekannt, ob der Knoten auf Ebene *d* noch ein späteres Geschwister hat. Bit *d* gesetzt heißt „auf dieser Ebene läuft die Linie weiter". Gezeichnet wird je gesetztem Bit eine senkrechte Linie in der Mitte der Einrückung, plus ein kurzer Winkel zur eigenen Zeile.

Kosten: 8 Bytes je Zeile zusätzlich, und die Arrays werden nur belegt, wenn `showGuides` an ist. Zeichenkosten sind zwei bis drei Linien je sichtbarer Zeile — was das Performance-Szenario als eigenen Fall misst, weil Linien im Tessellator teurer sind als Rechtecke.

### 3.5 Maus

| Geste | Wirkung |
|---|---|
| Klick auf Expander | Auf/zu, Index ungültig, kein Auswahlwechsel |
| Klick auf Zeile | Auswahl, Fokus auf den Baum |
| Doppelklick auf Ordner | Auf/zu — was der Explorer tut |
| Doppelklick auf Blatt | `activatedNode` |
| Rechtsklick auf Zeile | Auswahl **und** `contextNode`, damit ein Kontextmenü weiß, worauf es sich bezieht |
| Rad | Scrollen, `FUI__SCROLL_WHEEL_ROWS` Zeilen wie überall |

Doppelklick über `state->lastClickTime`/`lastClickIndex` und `FUI__DOUBLE_CLICK_SECONDS`, exakt wie in `fuiListBoxEx`.

### 3.6 Tastatur

Nur wenn `keyboardIsEnabled` gesetzt ist **und** der Baum den Fokus hat (`context->focused`, den der erste Klick setzt).

| Taste | Wirkung |
|---|---|
| Runter / Hoch | Auswahl eine sichtbare Zeile weiter, danach ins Bild scrollen |
| Rechts | Zugeklappt: aufklappen. Offen: auf das erste Kind |
| Links | Offen: zuklappen. Zugeklappt oder Blatt: auf den Elternknoten |
| Pos1 / Ende | Erste / letzte sichtbare Zeile |
| Bild auf / ab | Um so viele Zeilen, wie in den Kasten passen |
| Enter | `activatedNode` |

Über `fuiKeyRepeat`, damit eine gehaltene Taste läuft.

> **Inkonsistenz, bewusst:** Listbox und ListView haben heute *keine* Tastaturnavigation. Der Baum bekommt sie, weil ein Baum ohne Pfeiltasten kaum bedienbar ist. Damit das keine Sonderlocke bleibt, wird das Bewegen-und-Sichtbarmachen als interner Helfer (`fui__RowNavigate`, `fui__ScrollRowIntoView`) geschrieben, den die beiden Listen später ohne Umbau übernehmen können. Das ist eine eigene Aufgabe und steht in 8.

---

## 4. Phasen

Jede Phase ist für sich abnahmefähig: sie compiliert, die Demo läuft, und es gibt etwas zu sehen.

### Phase 1 — Kern

- `fuiTreeNode`, `fuiTreeDesc`, `fuiTreeAction` in den Header, im Abschnitt hinter „List view".
- Felder in `fuiWidgetState`, Defines, Theme-Felder plus Werte in `fuiDefaultTheme`.
- `fui__TreeBuildVisible`, `fui__TreeFingerprintExpanded`, `fui__EnsureTreeVisible`.
- `fuiTreeViewEx` mit: Rahmen, Scrollbalken, Rad, Sichtbarkeitsfenster über `fui__ListVisibleRange`, Zeile aus Einrückung + Expander + Text, Auswahl, Doppelklick, Rechtsklick.
- `fuiTreeView` als kurze Form, die intern eine genullte Desc füllt.

**Abnahme:** Ein Baum von 20 Knoten in FUI_Test klappt auf und zu, wählt aus, scrollt.

### Phase 2 — Helfer

`fuiTreeComputeDescendants`, `fuiTreeSetExpandedAll`, `fuiTreeExpandToNode`, `fuiTreeParentOf`, `fuiTreeReveal`, `fuiTreeInvalidate`, `fuiTreeGetVisibleCount`.

`fuiTreeComputeDescendants` läuft rückwärts und springt über schon berechnete Teilbäume, ist also linear:

```c
for(int32_t nodeIndex = nodeCount - 1; nodeIndex >= 0; --nodeIndex) {
	int32_t ownDepth = nodes[nodeIndex].depth;
	int32_t total = 0;
	int32_t childIndex = nodeIndex + 1;
	while(childIndex < nodeCount && nodes[childIndex].depth > ownDepth) {
		int32_t subtreeSize = 1 + nodes[childIndex].descendantCount;
		total += subtreeSize;
		childIndex += subtreeSize;
	}
	nodes[nodeIndex].descendantCount = total;
}
```

**Abnahme:** *Alles aufklappen* auf einem großen Baum tut, was es sagt, und `fuiTreeReveal` holt einen tief vergrabenen Knoten ins Bild.

### Phase 3 — Icons

`fuiListIcons` durchreichen, `fui__ListBoxDrawRowIcon` mit dem Knotenindex aufrufen. Nichts Neues, nur verdrahten. `iconOnly` ergibt für einen Baum keinen Sinn (die Beschriftung ist der Ordnername) — in der Doku als „wird ignoriert" festhalten.

**Abnahme:** Ordner-zu / Ordner-auf / Datei aus dem Demo-Sheet, wobei die Zelle beim Aufklappen wechselt — womit gleich vorgeführt ist, dass `cellForRow` dem Aufrufer gehört.

### Phase 4 — Führungslinien und Tastatur

`treeGuideMasks`, `showGuides`, `treeGuideColor`; danach die Tastaturtabelle aus 3.6 samt `fui__ScrollRowIntoView`.

**Abnahme:** Mit Pfeiltasten durch einen 100-K-Baum laufen, ohne dass die Bildrate einbricht.

### Phase 5 — Demo (siehe 6.)
### Phase 6 — Performance (siehe 7.)

### Phase 7 — Doku

- Changelog-Block **v0.9.4** oben im Header, im Ton der bestehenden Einträge, mit gemessenen Zahlen aus Phase 6.
- Den Absatz „Status" im Kopf des Headers um den Baum ergänzen.
- `README.md`: Versionsspalte für `final_ui.h` (steht dort noch auf `0.9.0-beta`, ist also ohnehin überfällig).
- `final_game_tech.md`: Zeile zu FUI_Test/FUI_Performance um das Baum-Panel ergänzen.
- **Kein** CMake-Eingriff: es kommt keine neue Übersetzungseinheit dazu, die `CMakeLists.txt` der drei Demos bleiben, wie sie sind.

---

## 5. Was der Aufrufer tut — das Muster

So sieht die Nutzung aus, und so steht sie später im Header-Kommentar:

```c
// Einmal, nach dem Einlesen des Verzeichnisses
fuiTreeComputeDescendants(explorer->nodes, explorer->nodeCount);

// Jeden Frame
fuiListIcons icons = fplZeroInit;
icons.sheet = demo->iconSheet;
icons.sheetSize = demo->iconSheetSize;
icons.columns = DEMO_ICON_CELL_COUNT;
icons.rows = 1;
icons.cellForRow = explorer->iconForNode;
icons.cellForRowCount = explorer->nodeCount;
icons.rowScale = DEMO_TREE_ICON_ROW_SCALE;

fuiTreeDesc desc = fplZeroInit;
desc.nodes = explorer->nodes;
desc.nodeCount = explorer->nodeCount;
desc.isExpanded = explorer->isExpanded;
desc.icons = &icons;
desc.showGuides = demo->treeShowsGuides;
desc.keyboardIsEnabled = true;

fuiTreeAction action = fplZeroInit;
bool selectionChanged = fuiTreeViewEx(ui, treeRect, "explorer", &desc, &explorer->selectedNode, &action);
if(action.activatedNode >= 0) {
	const fuiTreeNode *activated = &explorer->nodes[action.activatedNode];
	DemoSayFormat(demo, "Geöffnet: %s", activated->label);
}
if(action.toggledNode >= 0) {
	// Das Icon des Ordners folgt seinem Zustand, weil die Tabelle dem Aufrufer gehört
	bool isOpen = explorer->isExpanded[action.toggledNode];
	explorer->iconForNode[action.toggledNode] = isOpen ? DEMO_ICON_FOLDER_OPEN : DEMO_ICON_FOLDER_CLOSED;
}
if(action.contextNode >= 0) {
	explorer->contextNode = action.contextNode;
	fuiOpenContextMenu(ui, "treemenu");
}
```

Nichts wird kopiert: Knoten, Beschriftungen, Flags und Icon-Tabelle müssen den Aufruf überleben — dieselbe Zusage wie überall in dieser Bibliothek.

---

## 6. Demo-Einbau: `demos/FUI_Test/fui_test.c`

Ein neues Panel **„Project"**, in der Machart des vorhandenen „Entity table"-Panels: das Widget in echt, und daneben Schalter für jedes Feld, das das Bild ändert.

### 6.1 Der Datenbestand

Ein handgeschriebener, plausibler Projektbaum von ~40 Knoten, statisch im Quelltext — kein Verzeichnis wird gelesen, denn die Bibliothek liest keins und die Demo soll das zeigen:

```
assets/
  fonts/        sprites/       audio/
levels/
  gardens-of-ash.lvl           the-drowned-mill.lvl
scripts/
readme.txt
```

Dazu `iconForNode` (Ordner zu / Ordner auf / Datei / Level), `isExpanded`, `selectedNode`.

### 6.2 Das Panel

- Kopfzeile mit vier kleinen Knöpfen: *Alle auf*, *Alle zu*, *Auswahl zeigen*, *Zufälligen Knoten zeigen* — letzteres führt `fuiTreeExpandToNode` + `fuiTreeReveal` zusammen vor.
- Der Baum selbst im Rest des Panels.
- Fußzeile wie beim Tabellen-Panel: Pfad des ausgewählten Knotens, sonst ein Satz, der die Bedienung erklärt.
- Checkboxen: *Führungslinien*, *Icons*, *Tastatur*, plus ein Slider für `indentWidth`. Das ist bewusst dieselbe Idee wie `tableShowsHeaderIcons` und Freunde: jedes Desc-Feld einmal anfassbar.

### 6.3 Verdrahtung

- `bool showTreePanel` in `DemoState`, Eintrag im View-Menü und in *„Bring the panels back"* im Kontextmenü.
- Ein eigenes Kontextmenü **auf dem Baum**, gespeist aus `action.contextNode`: *Aufklappen*, *Zuklappen*, *Teilbaum aufklappen*, *Umbenennen…* (öffnet den vorhandenen `fuiInputBox`-Dialog).
- Statuszeile meldet Auswahl und Aktivierung, wie die anderen Panels auch.

### 6.4 `demos/FUI_Adapter_FPL/fui_adapter_fpl.c`

Die Adapter-Demo spiegelt FUI_Test fast eins zu eins. Das Baum-Panel gehört dort ebenfalls hinein, damit beide Demos denselben Funktionsumfang zeigen — aber **erst nachdem FUI_Test steht**, und als eigener Commit. Wenn es zeitlich klemmt, ist das der Punkt, der ohne Schaden liegen bleiben kann; dann muss es aber ausdrücklich hier vermerkt werden.

---

## 7. Performance-Szenario: `demos/FUI_Performance/fui_performance.c`

Der Workbench fragt „was KOSTET das". Für den Baum sind drei Kosten interessant, und das Szenario muss sie auseinanderhalten:

1. **Ein zugeklappter Baum** — die zentrale Zusage. 1 M Knoten, zwölf offen: muss so viel kosten wie zwölf Zeilen, nicht wie eine Million.
2. **Ein offener Baum, ans Ende gescrollt** — der Fall, der einen linearen Durchlauf entlarvt. Das Gegenstück zu `textbox 200K at end`.
3. **Der Neuaufbau des Index** — was ein Klick auf einen großen Ordner kostet. Das Gegenstück zu `listview resort`.

### 7.1 Der Datensatz

`PerfTreeData` neben `PerfDataSet`, aus demselben deterministischen xorshift und derselben `PerfStringArena` — also kein neuer Allokator und keine neue Zufallsquelle:

- Ein Dateisystem-artiger Baum: Verzeichnisse bis `PERF_TREE_MAX_DEPTH` (6), je Verzeichnis 2–8 Kinder, Blätter mit Dateinamen samt Endung.
- Erzeugt breitenweise, bis die Knotenzahl der Skalenstufe erreicht ist, dann abgeschnitten — so trifft jede Stufe ihre Zahl **genau**.
- Skalenstufen sind die vorhandenen `g_perfScaleRowCounts` (1 K … 1 M), es kommt also keine zweite Achse dazu.
- `iconForNode` fällt beim Erzeugen mit an (Ordner/Datei), damit der Icon-Fall nichts extra bauen muss.
- Zusätzlich ein **entarteter** Baum: eine Kette von `FUI_MAX_TREE_DEPTH` Ebenen, für den Tiefen-Fall.

Die Statuszeile meldet nach dem Erzeugen Knotenzahl, Tiefe und Speicher, wie sie es für Tabelle und Text schon tut.

### 7.2 Tab und Bedienung

- Neuer Tab **„Tree view"**, `PERF_TAB_TREE`, in `g_perfTabNames`.
- `PerfBuildTreeTab` mit dem Baum über die volle Fläche und einer Kopfzeile: *Alle auf*, *Alle zu*, *Nur Wurzeln*, dazu die Schalter für Icons und Führungslinien.
- Die Metrik-Anzeige bekommt zwei Zeilen dazu: **sichtbare Zeilen** (`fuiTreeGetVisibleCount`) und **Index-Neuaufbauten** seit dem letzten Zurücksetzen. Für Letzteres ein Zähler `fuiContext.treeRebuildCount` — der Kontext ist ausdrücklich „öffentlich zur Ansicht", das passt.
- Der Tab **„All at once"** nimmt den Baum als vierten Bereich auf. **Achtung:** damit verschieben sich die Zahlen der Fälle `everything 10K`/`everything 100K` gegenüber dem, was in `fui_list_icons_plan.md` steht. Die Baseline wird in Phase 6 neu aufgenommen und der alte Wert dabei ausdrücklich als „vor dem Baum" gekennzeichnet.

### 7.3 Benchmark-Fälle

Neue Subjects in `PerfSubject`, neue Zeilen in `g_perfCases`. Jede Zeile variiert genau eine Sache:

| Fall | Was er beantwortet |
|---|---|
| `tree 1K collapsed` … `tree 1M collapsed` | Kostet ein zugeklappter Baum wirklich nur seine Wurzeln? Die Kurve muss **flach** sein |
| `tree 1K expanded` … `tree 1M expanded` | Der offene Baum am Anfang — muss so flach sein wie `listview` |
| `tree 1M expanded at end` | Ans Ende gescrollt. Gleich teuer wie am Anfang, sonst zählt irgendwo jemand von vorn |
| `tree 100K toggle` | Jeden Frame ein Ordner um, also Index-Neuaufbau je Frame. Die Zahl, gegen die der Cache sich rechtfertigen muss |
| `tree 1M expand all` | Der eine teure Frame nach *Alles auf*, als worst-frame gemeldet statt im Median versteckt |
| `tree 100K icons` / `tree 100K icn bat` | Der zweite Draw-Command je Zeile, mit und ohne Batching — genau wie bei den Listen |
| `tree 100K guides` | Was die Linien kosten |
| `tree deep 64` | Einrückung und Maske bei voller Tiefe |
| `everything 100K` (neu gemessen) | Der Baum neben Tabelle, Liste und Text |

Der Fall `tree 1M expand all` braucht im Ablauf eine Besonderheit: `PerfBuildBenchmarkFrame` bekommt für ihn im ersten Frame ein `fuiTreeSetExpandedAll` + `fuiTreeInvalidate`, so wie `PerfSubject_MenuPopup` sein `fuiOpenContextMenu` im ersten Frame bekommt.

### 7.4 Gemessen

`FUI_Performance --benchmark`, 1600×940, 8 Aufwärm- und 41 gemessene Frames, Median der Build-Zeit. Die Zeit ist reine Bibliothekszeit zwischen `fuiBeginFrame` und `fuiEndFrame`. Alle Zahlen aus **einem** Lauf; zwischen zwei Läufen schwanken sie um wenige Prozent, die dritte Nachkommastelle ist also Rauschen.

**Die Zusage aus 1.3.** Ein zugeklappter Baum kostet seine offenen Wurzeln, ein offener seine sichtbaren Zeilen — beides unabhängig davon, wie viele Knoten dahinter hängen:

| Fall | Median ms | Commands | Vertices |
|---|---|---|---|
| `tree 1K folded` | 0,006 | 75 | 1740 |
| `tree 10K folded` | 0,005 | 75 | 1732 |
| `tree 100K folded` | 0,006 | 95 | 2174 |
| `tree 1M folded` | **0,007** | 95 | 2250 |
| `tree 1K open` | 0,008 | 70 | 3219 |
| `tree 10K open` | 0,008 | 71 | 3114 |
| `tree 100K open` | 0,008 | 70 | 3199 |
| `tree 1M open` | **0,007** | 72 | 3113 |
| `tree 1M open at end` | **0,008** | 68 | 3322 |

Die Kurve ist flach — über drei Größenordnungen. `tree 1M open at end` ist der Fall, der einen linearen Durchlauf entlarvt hätte: ans Ende gescrollt kostet genauso viel wie oben.

**Was der Index wert ist.** `toggling` klappt jeden Frame eine Wurzel um, wirft den Index also jeden Frame weg. Das ist die Zahl, gegen die sich der Cache rechtfertigen muss:

| Fall | Median ms | gegenüber `open` |
|---|---|---|
| `tree 10K toggling` | 0,022 | 2,8× |
| `tree 100K toggling` | 0,225 | 28× |
| `tree 1M toggling` | **2,353** | **336×** |

2,35 ms ist auch das, was *ein* Klick auf einen Ordner in einem vollständig offenen Millionen-Knoten-Baum kostet — einmal, nicht pro Frame. Das gehört so in die Doku und nicht wegerklärt.

**Icons und Führungslinien** kosten Draw-Commands, nicht Build-Zeit — genau wie bei den Listen:

| Fall | Median ms | Commands |
|---|---|---|
| `tree 100K open` | 0,008 | 70 |
| `tree 100K icons` | 0,009 | 100 |
| `tree 100K icn bat` | 0,009 | 81 |
| `tree 100K guides` | 0,011 | **250** |
| `tree deep 64` | 0,018 | **705** |

Führungslinien verdreifachen die Commands, weil jede Ebene einer Zeile eine eigene Linie ist und Linien nicht batchbar sind. Deswegen sind sie per Default aus und müssen angefordert werden. `tree deep 64` ist die Leiter aus dem Stand-Abschnitt: 64 Ebenen, die tiefste sichtbare Zeile zieht ihre Ahnenlinien alle einzeln.

**Neben den anderen Widgets.** Zum Vergleich aus demselben Lauf — die `everything`-Fälle bauen jetzt vier Widgets statt drei, ihre Zahlen sind also **nicht** mit denen aus `fui_list_icons_plan.md` vergleichbar:

| Fall | Median ms | Commands |
|---|---|---|
| `listview 1M` | 0,036 | 302 |
| `listbox 500K` | 0,015 | 41 |
| `textbox 200K` | 0,099 | 59 |
| `tree 1M open` | 0,007 | 72 |
| `everything 10K` | 0,035 | 237 |
| `everything 100K` | 0,055 | 237 |
| `listview resort 100K` | 40,953 | 303 |

Der Baum ist das billigste der vier Widgets, weil er weder sortiert noch Spalten misst noch Text umbricht — er zeichnet Zeilen.

---

## 8. Bewusst offen

Nicht vergessen, sondern verschoben, bis es einen echten Anwendungsfall gibt:

- **Begin/End-Form** (`fuiBeginTreeNode`/`fuiEndTreeNode`) für kleine, im Code gebaute Bäume. Setzt auf denselben Zeichenhelfern auf, hat aber keine Virtualisierung.
- **Tree-Table**: Baumspalte plus weitere Spalten mit Kopfzeile, also Explorer-Detailansicht. Größter Brocken; deswegen wird das Zeichnen einer Zeile in Phase 1 so herausgezogen, dass eine Spaltenvariante sie später benutzen kann.
- **Mehrfachauswahl** mit Strg und Umschalt.
- **Checkboxen je Knoten**, dreiwertig (an / aus / teilweise).
- **Waagerechtes Scrollen.** `fuiWidgetState.scrollX` gibt es schon, ein `fuiScrollbarHorizontal` nicht — das ist eine eigene Aufgabe, von der auch die ListView etwas hätte.
- **Ziehen und Ablegen** von Knoten.
- **Tastaturnavigation für Listbox und ListView**, aus den Helfern von Phase 4.
- **`fuiFileBrowser` auf Baum umstellen** — eine echte Ordner-Baum-Spalte neben der Dateiliste. Verlockend, aber es ändert eine bestehende API, und das gehört nicht in denselben Schritt wie ein neues Widget.

---

## 9. Risiken

| Risiko | Gegenmaßnahme |
|---|---|
| `fuiWidgetState` wächst für jedes Widget | Bewusst in Kauf genommen und in der Changelog benannt; Alternative wäre eine eigene Tabelle nur für Bäume, was mehr Code für weniger Nutzen wäre |
| Ein langer Baum, dessen Flags der Aufrufer selbst umlegt, zeigt einen veralteten Index | `FUI_TREE_VERIFY_NODES` deckt den kurzen Fall automatisch ab, `fuiTreeInvalidate` den langen — und die Demo führt genau das vor |
| Arena gibt nichts zurück, häufiges Wachsen fragmentiert | Kapazität in Zweierpotenzen, Deckel `FUI_MAX_TREE_NODES` |
| Doppelklick auf Ordner ist zweideutig (öffnen oder umklappen?) | Ordner klappt um, Blatt aktiviert; `activateOnSingleClick` für alles, was das anders will |
| Tastatur nur im Baum | Helfer so schneiden, dass die Listen nachziehen können, und als offener Punkt geführt |
| `everything`-Baseline verschiebt sich | Neu messen und die alte Zahl als „vor dem Baum" kennzeichnen |
