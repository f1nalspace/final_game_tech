# Plan: Editor-Widget als `final_ui_texteditor.h`

Ziel: Ein richtiger Code- und Texteditor als eigenständiger Single-Header neben `final_ui.h` — mit Randspalte, Zeilennummern, Tabulatoren, sichtbarem Whitespace, Syntax-Einfärbung über einen Lexer, Diff-Einfärbung über ganze Zeilen, Überschreibmodus, Suchen und Ersetzen, Undo, und einem Dokument, das weiß, in welchem Encoding und mit welchem Zeilenende es angekommen ist. Und das alles bei einer Datei von einer Million Zeilen genauso flüssig wie bei einer von zwanzig.

Dieses Dokument beschreibt fünf Dinge: den **Stand** (1), die **Designentscheidungen** (2), die **öffentliche API** (3), den **inneren Aufbau** (4) und die **Iterationen** samt Demo- und Performance-Arbeit (5–8).

Der Auslöser steht in `docs_fpl/editor-widget.md`.

---

## 1. Stand

**Iterationen 0 bis 2 sind umgesetzt.** Was es gibt: das Dokument, und eine Ansicht darauf, die gelesen, gescrollt, markiert und kopiert werden kann. Damit ist der Read-Only-Editor fertig — alles Weitere ändert Text.

- `final_ui_texteditor.h` v0.3.0 — Gap-Buffer, Split-Zeilenindex, Encoding-Vtable mit UTF-8- und ASCII-Backend, dazu `fuiTextEditor` mit Randspalte, Zeilennummern, Tabstopps, Monospace-Schnellweg, beiden Scrollbalken, eigener Statusleiste, und Cursor, Auswahl, Tastatur, Maus und Kopieren. `fuiEditorConfig` mit `colors` / `metrics` / `toggles`.
- `final_ui.h` v0.9.8 — `fuiScrollbarHorizontal`, `fuiRegisterFocusable` und `fuiGetFrameTime` sind öffentlich. Damit sind die Zusätze, die dieser Add-on braucht, alle drin.
- `demos/FUI_Editor/` — zeigt `final_ui.h` selbst (über 14 000 Zeilen, 654 KB), mit Umschaltern für Zeilennummern, Statusleiste, aktuelle Zeile, Interaktivität, Tabbreite und Schriftschnitt, plus Auswahl und Kopieren.
- `--selftest` läuft mit **308 Prüfungen** sauber unter AddressSanitizer und UndefinedBehaviorSanitizer durch, davon ein kopfloser Rahmen, der eine Taste drückt und die Antwort zurückliest.

Was beim Bauen von Iteration 0 anders lief als geplant:

- **Die Lücke im Textpuffer darf sich nie ganz schließen.** `fuiEditorGetContiguousText` setzt seine terminierende Null an `bytes[textLength]` — steht die Lücke auf null, liegt das eine Stelle hinter dem Array. `fuiEditor__DocumentReserve` fordert deshalb grundsätzlich ein Byte mehr an, als der Aufrufer verlangt hat. Der Fehler wäre erst bei einer Einfügung aufgefallen, die die Lücke exakt auffüllt, also selten und dann unerklärlich.
- **Beide Reservierungen müssen vor der ersten Bewegung passieren.** `fuiEditorInsert` zählt die neuen Zeilen, reserviert Text *und* Zeilenindex, und fängt erst danach an, Lücken zu schieben. Eine fehlgeschlagene Reservierung mittendrin würde die beiden Indizes gegeneinander verstellt zurücklassen — und das wäre kein Absturz, sondern ein Dokument, das ab da falsche Zeilen meldet.
- **Reihenfolge von Lückenbewegung und `tailDelta`.** Das Verschieben der Lücke rechnet Einträge zwischen roher und verkürzter Form um, benutzt dabei also `tailDelta`. Es muss deshalb *vor* der Änderung von `tailDelta` laufen, nicht danach. Steht so als Kommentar an beiden Stellen.

Und was beim Bauen von Iteration 1 anders lief als geplant:

- **Der Tabstopp darf nicht auf dem Stift gerechnet werden.** Der erste Entwurf hat `penX - lineLeftX` durch die Tabbreite geteilt. `lineLeftX` trägt aber das x des Widgets, und bei ein paar hundert Pixeln hat ein `float` nicht mehr genug Stellen, um „genau einen Tabstopp weiter" zu sagen: die Division kommt bei 0,99999 heraus, der Stift wird auf den Stopp geschickt, auf dem er schon steht, und **der zweite Tabulator einer Zeile bewegt ihn gar nicht**. Im Demo war das sofort zu sehen — `final_ui.h` Zeile 12948 hat zwei Tabulatoren und stand eingerückt wie mit einem. Gerechnet wird jetzt auf dem Abstand zum Zeilenanfang, plus einer Toleranz am Stopp selbst. Beides ist nötig, und beides steht als Prüfung im `--selftest`.
- **`fuiEditorScrollToLine` kann nicht rechnen, wenn es gerufen wird.** Der Offset ist in Pixeln, und wie hoch eine Zeile ist, steht in der Schrift, die der *Kontext* trägt — von der ein Dokument nichts weiß. Aus der Schrifthöhe geraten ist um den Zeilenabstand der Schrift daneben. Der Aufruf merkt sich deshalb nur die Zeile, und der nächste Build löst sie auf.
- **Die Konfiguration wird gegen das Theme gehalten, aus dem sie aufgelöst wurde.** Geplant war „einmal beim Setzen auflösen". Damit wäre ein Kontext, der zwischen zwei Frames umgestylt wird, unbemerkt geblieben. Ein `memcmp` über das Theme je Frame kostet ein paar hundert Byte und nimmt die Falle ganz weg.
- **Die Breite für den waagerechten Balken ist die breiteste *gesehene* Zeile.** Jede Zeile jedes Frame zu messen wäre genau der Gang über das ganze Dokument, den das Widget vermeiden soll — bei einer proportionalen Schrift ein Glyphen-Lookup je Zeichen. Der Bereich wächst also beim Durchscrollen, und eine Änderung setzt ihn zurück. Scintilla macht es genauso.
- **Der Prüfrahmen wurde gegengeprüft, und zwar nicht absichtlich.** Drei echte Fehler und ein Stack-Overflow im Testcode selbst sind in dieser Iteration gemeldet worden, bevor sie behoben wurden. Zusätzlich wurde die Tabstopp-Toleranz einmal auf null gesetzt: die Prüfung wurde rot, wie sie sollte.

Und was bei Iteration 2 dazukam:

- **`final_ui.h` brauchte einen dritten Zusatz.** Geplant waren zwei; es wurde auch `fuiGetFrameTime`. Ein Add-on bekommt nur den Kontext gereicht, und ohne Zeitquelle kann es nichts takten — kein Blinken, keinen Doppelklick, kein Auto-Scrollen. Die Funktion gibt im Draw-Pass null zurück, damit ein Zwei-Pass-Aufrufer nichts doppelt laufen lässt.
- **Cursorposition und Zeichnen müssen aus derselben Quelle kommen.** Beides läuft jetzt über denselben Segment-Walk über eine Zeile: dieselben Stücke, dieselbe Tabstopp-Rechnung, dieselbe Messung. Zwei getrennte Rechnungen wären an jedem Tabulator und an jedem Kerning-Paar auseinandergelaufen.
- **Kerning ist nur als Differenz erreichbar.** `final_ui.h` gibt Messen heraus, nicht die Kerningtabelle. Der Vorschub eines Zeichens ist deshalb `Messung(Paar) − Messung(erstes)` — zwei O(1)-Messungen statt einer Präfixmessung je Kandidat, die quadratisch geworden wäre. Bei Monospace fällt beides weg.
- **Die Zeile des Cursors wird vor dem Layout gemessen.** Sonst kann eine Pfeiltaste, die in eine lange Zeile hineinläuft, nicht seitwärts zu ihr scrollen: der waagerechte Bereich kennt die Zeile ja noch nicht.

## 2. Designentscheidungen

Sechs Fragen entscheiden alles Weitere. Jede hat mehrere vertretbare Antworten, deswegen steht hier jeweils dabei, warum es diese wurde.

### 2.1 Wie wird der Text gespeichert?

| Variante | Wie | Dagegen spricht |
|---|---|---|
| **A: Gap-Buffer + Zeilenindex** | Ein Puffer mit einer Lücke an der Bearbeitungsstelle, dazu ein Array der Zeilenanfänge | Ein Sprung ans andere Ende kostet ein `memmove` |
| B: Flaches Array | Ein zusammenhängender Puffer, jede Änderung ein `memmove` des Rests | Ein Tastendruck am Dateianfang bewegt das ganze Dokument, jedes Mal |
| C: Zeilen-Array | Je Zeile ein eigener Puffer | Der Byteoffset im Dokument ist die Summe aller Vorzeilen — also entweder teuer oder ein zweiter Index |
| D: Piece Table | Original unverändert, Änderungen angehängt, Stückliste darüber | Jeder Lesezugriff geht durch die Stückliste; deutlich mehr Code als alle anderen |

**Es wird A.** Das ist, was Scintilla tut, und aus demselben Grund: Tippen ist die häufigste Operation und findet fast immer an derselben Stelle statt. Ein `memmove` je Cursorsprung ist ein Preis, der einmal pro *Sprung* anfällt, nicht einmal pro *Zeichen*.

### 2.2 Wem gehört das Dokument?

**Der Bibliothek.** Das ist die eine Stelle, an der dieser Add-on von der Hausregel abweicht, dass der Aufrufer alles besitzt.

Der Grund: Gap-Buffer, Zeilenindex und Style-Array sind keine Daten, sondern *Invarianten*. Sie müssen zueinander passen, und zwar nach jeder einzelnen Änderung. Sie dem Aufrufer zu geben hieße, ihm die Pflicht zu geben, sie konsistent zu halten — und das ist keine API, das ist eine Falle. Alles andere (Farben, Maße, Callbacks, Shortcuts) bleibt beim Aufrufer, so wie `fuiTreeDesc` es vormacht.

Der Zustand liegt aber trotzdem beim Aufrufer, nur eben als **undurchsichtiges `fuiEditor`, das er by value hält** — nicht in `fuiWidgetState`. Zwei Gründe:

1. `fuiWidgetState` (`final_ui.h:1959`) ist ein flaches Struct, das *jedes* Widget bezahlt. Der TreeView hat dort laut Changelog ~56 Bytes je Slot hinzugefügt, und das war schon der Rede wert. Ein Editor bringt mehr mit.
2. Cursor und Auswahl liegen heute auf dem **Kontext** (`caretOwner`, `caretPosition`, `selectionAnchor`, `final_ui.h:2276`), also *global über alle Textfelder*. Zwei Editoren nebeneinander bräuchten je einen eigenen. Mit eigenem Zustand ist das gelöst; nur der Tastaturfokus bleibt beim Kontext, wo er hingehört.

### 2.3 Wie eng koppelt der Add-on an `final_ui.h`?

**Nur über die öffentliche API.** Alle `fui__`-Interna liegen im `FUI_IMPLEMENTATION`-Block (`final_ui.h:4797`) und sind damit nur in derselben Übersetzungseinheit sichtbar. Ein Add-on, das sie benutzt, müsste zwingend dorthin — und wäre an Interna gebunden, die sich ohne Ankündigung ändern dürfen.

`final_ui.h` darf dafür Neues bekommen, aber nur, was auch anderen Widgets nützt. Bisher sind das genau zwei Dinge, beide in späteren Iterationen fällig:

| Was | Warum es fehlte | Wann |
|---|---|---|
| `fuiScrollbarHorizontal` ✅ | `fui__Scrollbar(..., false)` gab es intern längst, die ListView ruft es direkt auf; öffentlich war nur die vertikale Fassung | Iteration 1, drin seit `final_ui.h` v0.9.7 |
| `fuiRegisterFocusable` ✅ | `fui__RegisterFocusable` hängt ein Widget in die Tab-Kette; ohne öffentliche Fassung kann kein fremdes Widget daran teilnehmen | Iteration 2, drin seit `final_ui.h` v0.9.8 |
| `fuiGetFrameTime` ✅ | War nicht geplant. Ein Add-on bekommt nur den Kontext, und ohne Zeitquelle kann es nichts takten — Blinken, Doppelklick, Auto-Scrollen | Iteration 2, drin seit `final_ui.h` v0.9.8 |

`FUI_MAX_CLIPBOARD_TEXT` (1024) bleibt, wie es ist — das ist der Stapelpuffer des alten Textfelds. `fuiGetClipboardText`/`fuiSetClipboardText` nehmen die Puffergröße als Parameter, der Editor gibt einfach einen großen mit.

### 2.4 Wie weit reicht die Encoding-Abstraktion?

| Variante | Wie | Dagegen spricht |
|---|---|---|
| **A: Nur an den Rändern** | Dokument immer UTF-8, Vtable läuft beim Laden und Speichern | Ein 100-MB-UTF-16-File wird beim Laden einmal komplett konvertiert |
| B: Dokument bleibt im Quell-Encoding | Jeder Zugriff geht durch ein Codec | Bei UTF-7, dessen Bedeutung vom Vorangegangenen abhängt, hört „Byte Nummer n" auf, etwas zu heißen |

**Es wird A**, und zwar deutlich. B klingt sparsamer und ist es nicht: jeder Renderpfad, jede Cursorbewegung und jede Suche zahlt dann für eine Allgemeinheit, die an genau zwei Momenten im Leben eines Dokuments gebraucht wird. Scintilla macht es genauso.

Das Encoding und die Zeilenende-Art werden auf dem Editor gemerkt, damit Speichern zurückkonvertiert.

### 2.5 Wie wird eingefärbt?

**Zwei getrennte Schichten**, weil zwei sehr verschiedene Dinge gemeint sind.

| Schicht | Wofür | Braucht Zustand? |
|---|---|---|
| **Lexer** | Syntax-Highlighting für Code | Ja — ob Zeile 5000 im Blockkommentar steht, weiß nur, wer Zeile 4999 gesehen hat |
| **Dekoration** | Diffs, Fehlermarkierungen, Suchtreffer | Nein — ein Diff kennt seine Antwort ohne Vorgeschichte |

Der Lexer bekommt das Scintilla-Modell: ein `uint8_t` Style je Dokumentbyte, eine Style-Tabelle, ein Parser-Zustand je Zeile, und ein Wasserstand `styledUpToLine`. Gefragt wird nur, was fehlt — von dort bis zum Ende des sichtbaren Fensters.

Das löst den Widerspruch aus dem Prompt („nur sichtbare Bereiche einfärben, aber es muss zum Parsen passen"): der Lexer *darf* bei Zeile 5000 anfangen, weil er den Zustand von Zeile 4999 mitbekommt.

### 2.6 Bricht der Editor Zeilen um?

**Abschaltbar, standardmäßig aus.** Eine Dokumentzeile ist dann genau eine Bildschirmzeile, und Zeile N ist in O(1) erreichbar.

Wichtig ab Iteration 1, obwohl der Umbruch erst in Iteration 7 kommt: **Dokumentzeile und Bildschirmzeile werden von Anfang an getrennt benannt**, auch solange sie dasselbe sind. Sonst wird der Umbruch später eine Operation am offenen Herzen.

---

## 3. Öffentliche API

### 3.1 Was steht (Iteration 0 und 1)

```c
// Lebenszyklus
bool         fuiEditorInit(fuiEditor *editor, const fuiAllocator *allocator);
void         fuiEditorRelease(fuiEditor *editor);

// Füllen
bool         fuiEditorSetText(fuiEditor *editor, const char *text, const int32_t textLength);
bool         fuiEditorLoadFromMemory(fuiEditor *editor, const uint8_t *data, const int32_t dataLength, const fuiEditorEncoding *encoding);

// Lesen
int32_t      fuiEditorGetTextLength(const fuiEditor *editor);
char         fuiEditorGetByte(const fuiEditor *editor, const int32_t offset);
int32_t      fuiEditorCopyRange(const fuiEditor *editor, const int32_t offset, const int32_t byteCount, char *destination, const int32_t destinationCapacity);
int32_t      fuiEditorCopyText(const fuiEditor *editor, char *destination, const int32_t destinationCapacity);
const char  *fuiEditorGetContiguousText(fuiEditor *editor);

// Ändern
bool         fuiEditorInsert(fuiEditor *editor, const int32_t offset, const char *text, const int32_t textLength);
bool         fuiEditorErase(fuiEditor *editor, const int32_t offset, const int32_t byteCount);

// Zeilen
int32_t      fuiEditorGetLineCount(const fuiEditor *editor);
int32_t      fuiEditorGetLineStart(const fuiEditor *editor, const int32_t lineIndex);
int32_t      fuiEditorGetLineEnd(const fuiEditor *editor, const int32_t lineIndex);
int32_t      fuiEditorGetLineLength(const fuiEditor *editor, const int32_t lineIndex);
int32_t      fuiEditorCopyLine(const fuiEditor *editor, const int32_t lineIndex, char *destination, const int32_t destinationCapacity);
int32_t      fuiEditorGetLineOfOffset(const fuiEditor *editor, const int32_t offset);

// Codepoint-Grenzen
int32_t      fuiEditorNextCodepointOffset(const fuiEditor *editor, const int32_t offset);
int32_t      fuiEditorPreviousCodepointOffset(const fuiEditor *editor, const int32_t offset);
int32_t      fuiEditorSnapToCodepointStart(const fuiEditor *editor, const int32_t offset);

// Zeilenenden und Encodings
fuiEditorEol fuiEditorGetEol(const fuiEditor *editor);
void         fuiEditorSetEol(fuiEditor *editor, const fuiEditorEol eol);
const char  *fuiEditorEolGetName(const fuiEditorEol eol);
const char  *fuiEditorEolGetBytes(const fuiEditorEol eol, int32_t *outLength);
fuiEditorEncoding fuiEditorEncodingUtf8(void);
fuiEditorEncoding fuiEditorEncodingAscii(void);

// Das Widget und seine Konfiguration
fuiEditorConfig fuiEditorDefaultConfig(void);
void            fuiEditorSetConfig(fuiEditor *editor, const fuiEditorConfig *config);   // null ist erlaubt
const fuiEditorConfig *fuiEditorGetConfig(const fuiEditor *editor);
fuiEditorAction fuiTextEditor(fuiContext *context, const fuiRect rect, const char *id, fuiEditor *editor);

// Die Ansicht
void    fuiEditorScrollToLine(fuiEditor *editor, const int32_t documentLine);
int32_t fuiEditorGetFirstVisibleLine(const fuiEditor *editor);
int32_t fuiEditorGetVisibleLineCount(const fuiEditor *editor);

// Cursor und Auswahl
int32_t fuiEditorGetCaretOffset(const fuiEditor *editor);
void    fuiEditorSetCaretOffset(fuiEditor *editor, const int32_t offset, const bool extendSelection);
int32_t fuiEditorGetCaretLine(const fuiEditor *editor);
int32_t fuiEditorGetCaretColumn(const fuiEditor *editor);
void    fuiEditorSetCaretLine(fuiEditor *editor, const int32_t documentLine);
void    fuiEditorSetSelection(fuiEditor *editor, const int32_t anchorOffset, const int32_t caretOffset);
void    fuiEditorSelectAll(fuiEditor *editor);
void    fuiEditorClearSelection(fuiEditor *editor);
bool    fuiEditorHasSelection(const fuiEditor *editor);
int32_t fuiEditorGetSelectionStart(const fuiEditor *editor);
int32_t fuiEditorGetSelectionEnd(const fuiEditor *editor);
int32_t fuiEditorCopySelection(const fuiEditor *editor, char *destination, const int32_t destinationCapacity);
```

Zwei Konventionen, die überall gelten:

- **Kopieren sagt immer die volle Länge**, auch wenn es nicht gepasst hat. Also einmal mit `null` fragen, allozieren, nochmal fragen.
- **Ein `textLength` von 0 heißt „bis zur terminierenden Null"**, so wie `fuiMeasureText` es hält.

### 3.2 Was noch kommt

Alles Schreibende. Cursor und Auswahl (Iteration 2), Lexer und Dekoration (Iteration 3), Tippen, Undo, Suchen und Ersetzen, weitere Encodings und der Zeilenumbruch (Iterationen 4 bis 7). `fuiEditorConfig` bekommt dabei die beiden noch fehlenden Unterstrukturen `callbacks` (Iteration 4) und `shortcuts` (Iteration 8).

Zwei Abweichungen vom ursprünglichen Entwurf sind schon eingetreten und stehen so im Header:

- **`fuiEditorSetConfig` löst nicht selbst auf.** Es kopiert und markiert; aufgelöst wird beim nächsten Build, weil ein Theme zum *Kontext* gehört und ein Dokument keinen hat. Aufgelöst wird trotzdem nur **einmal** — genau das war der Punkt.
- **`fuiEditorScrollToLine` merkt sich nur die Zeile.** Aus demselben Grund: die Zeilenhöhe steht in der Schrift des Kontextes.

## 4. Innerer Aufbau

### 4.1 Der Gap-Buffer

```c
typedef struct fuiEditorDocument {
	char *bytes;          // [vorne][====Lücke====][hinten]
	int32_t capacity;
	int32_t gapStart;     // zugleich der Dokumentoffset, an dem die Lücke sitzt
	int32_t gapEnd;
	fuiEditorLineIndex lines;
} fuiEditorDocument;
```

Dokumentlänge ist `capacity - (gapEnd - gapStart)`. Byte an Position `p` liegt physisch bei `p < gapStart ? p : p + gapSize`.

**Die Lücke schließt sich nie ganz.** `fuiEditor__DocumentReserve` fordert immer ein Byte mehr an als verlangt, weil `fuiEditorGetContiguousText` dort seine Null hinschreibt.

### 4.2 Der Zeilenindex

Der nicht offensichtliche Teil, und der Grund, warum das Ganze bei großen Dateien trägt.

Ein naives `int32_t *lineStarts` müsste nach jeder Einfügung alle nachfolgenden Einträge um die Byte-Differenz verschieben — bei einer Million Zeilen also eine Million Additionen je Tastendruck. Stattdessen bekommt der Index dieselbe Lücke und **einen gemeinsamen Versatz für die hintere Hälfte**:

```c
typedef struct fuiEditorLineIndex {
	int32_t *starts;
	int32_t capacity;
	int32_t gapStart;
	int32_t gapEnd;
	int32_t tailDelta;   // auf jeden Eintrag HINTER der Lücke aufzuschlagen
} fuiEditorLineIndex;

// Physischer Slot von Eintrag i:  i < gapStart ? i : i + (gapEnd - gapStart)
// Wert von Eintrag i:             starts[slot] + (i < gapStart ? 0 : tailDelta)
```

Eine Änderung schiebt die Lücke hinter die betroffene Zeile und ändert `tailDelta` um die Byte-Differenz. Nur Zeilen, die wirklich dazugekommen oder weggefallen sind, bewegen sich.

**Einfügen** von N Bytes an Offset O in Zeile L:

1. Neue Zeilen zählen, Text *und* Index reservieren — beides vor der ersten Bewegung.
2. Textlücke auf O, Bytes hineinschreiben, `gapStart += N`.
3. Indexlücke auf `L+1` (noch mit dem **alten** `tailDelta`).
4. `tailDelta += N` — damit rückt alles hinter der Änderung.
5. Neue Zeilenanfänge als absolute Offsets vor der Lücke einfügen.

**Löschen** von N Bytes ab O, von Zeile L bis Zeile M:

1. Textlücke auf O, `gapEnd += N`.
2. Indexlücke auf `L+1` (altes `tailDelta`).
3. `gapEnd += (M - L)` — die Lücke frisst die entfallenen Zeilen.
4. `tailDelta -= N`.

**Reihenfolge ist kein Geschmack:** Schritt 3/4 und 2 sind vertauschbar aussehende Zeilen, die es nicht sind. Die Lückenbewegung rechnet mit `tailDelta`, muss also davor laufen.

`fuiEditorGetLineOfOffset` ist eine Binärsuche über die Einträge, kostet also den Logarithmus der Zeilenzahl.

### 4.3 Zeilenenden

**Nur ein Line Feed beendet eine Zeile.** Ein Carriage Return davor gehört zu der Zeile, die er beendet, und wird von `fuiEditorGetLineEnd` abgeschnitten — die Zeile *sagt* ihn nicht.

Folge: Ein Text aus reinen Carriage Returns (klassisches Mac) ist genau **eine** Zeile, bis er über ein Encoding geladen wird, das ihn normalisiert. Das ist Iteration 7. `fuiEditorGetEol` meldet, was gesehen wurde, und `fuiEditorEol_Mixed`, wenn es mehr als eine Art war.

### 4.4 Was noch kommt

- **Style-Array** (Iteration 3): ein `uint8_t` je Dokumentbyte, plus ein `int32_t` Parser-Zustand je Zeile und ein Wasserstand `styledUpToLine`. Eine Änderung in Zeile L setzt `styledUpToLine = min(styledUpToLine, L)`.

  Der Fall, an dem so etwas sonst stirbt — Cursor auf Zeile 500 000, Änderung in Zeile 3 — wird über **Zustandskonvergenz** abgefangen: das Nachfärben bricht ab, sobald der neu berechnete Ausgangszustand einer Zeile dem gespeicherten entspricht und man hinter der Änderung ist. Alles danach war schon richtig.

- **Tabulatoren** (Iteration 1): `fuiDrawText` kennt kein `\t`. Der Editor zerlegt jede Zeile an den Tabulatoren und setzt x auf den nächsten Tabstopp. Zusammen mit den Style-Läufen aus Iteration 3 sind die Segmente der Schnitt aus beidem.

- **Monospace-Schnellweg** (Iteration 1): Beim Setzen der Schrift wird `"W"` gegen `"i"` gemessen. Sind sie gleich breit, wird Spalte ↔ x eine Multiplikation statt einer Messschleife. Das umgeht nebenbei das O(n²), das `fui__ColumnFromCursorX` (`final_ui.h:9119`) bei langen Codezeilen hat.

---

## 5. Iterationen

Jede Iteration ist für sich abnahmefähig: sie compiliert, das Demo läuft, und es gibt etwas zu sehen.

### Iteration 0 — Fundament ✅

Dokument, Zeilenindex, Encoding-Seam, Demo-Gerüst, `--selftest`.

**Abnahme:** `FUI_Editor --selftest` liefert 0. — *Erfüllt.*

### Iteration 1 — Read-Only-Ansicht ✅

- `fuiTextEditor()` zeichnet: Rahmen, Randspalte mit **rechtsbündigen, nicht aufgefüllten** Zeilennummern und Trennstrich, Textbereich, nur die sichtbaren Zeilen.
- Vertikaler und horizontaler Scrollbalken, je `Auto` / `Always` / `Never`; Rad, Shift+Rad seitwärts. Voreinstellung: senkrecht `Always`, waagerecht `Auto`.
- Aktuelle Zeile mit Hintergrundfarbe, **die Zeilennummer eingeschlossen**.
- Eigene Statusleiste unter dem Editor — nicht `fuiBeginStatusBar`, die dockt ans Fensterende.
- `fuiEditorConfig`, `fuiEditorDefaultConfig()`, NULL erlaubt.
- Tabulator-Zerlegung, Monospace-Erkennung und -Schnellweg.
- Bitstream Vera Sans Mono **und** FiraCode über `apps/staticdatamaker` in `demos/additions/final_fonts.h`, im Demo umschaltbar.
- `fuiScrollbarHorizontal` in `final_ui.h` (v0.9.7).
- Zeilennummern und Text werden aus dem Puffer heraus gezeichnet, ohne Zwischenkopie — eine Zeile kann also nicht zu lang werden.

**Abnahme:** *Erfüllt.* Das Demo zeigt `final_ui.h` selbst — 14 357 Zeilen — und scrollt in beide Richtungen. Die Übereinstimmung der Zeilennummern mit der Datei ist als Prüfung im `--selftest` automatisiert (`[document against file]`): jede einzelne Zeile wird gegen einen rohen Scan der Datei nach Zeilenvorschüben gehalten, Anfang, Ende, Inhalt und der Rückweg über `fuiEditorGetLineOfOffset`. Dazu kommen `[widget layout]` und `[widget empty document]`, die das Widget kopflos gegen eine Schrift bekannter Maße bauen.

**Was noch aussteht und bewusst liegen bleibt:** Die aktuelle Zeile wird von `fuiEditorSetCaretLine` gesetzt, nicht von Maus oder Tastatur — das ist Iteration 2.

### Iteration 2 — Cursor, Auswahl, Tastatur ✅

- Cursor als Dokumentoffset plus Wunschspalte; Pfeile, Pos1/Ende, Ctrl+Pos1/Ende, Bild auf/ab, Ctrl+Links/Rechts wortweise, über `fuiKeyRepeat`.
- Maus: Klick, Ziehen über Zeilen, Auto-Scroll am Rand, Doppelklick=Wort, Dreifachklick=Zeile, Shift+Klick erweitert. Ein Ziehen, das auf einem Wort oder einer Zeile begann, bleibt auf ganzen.
- Ctrl+A, Ctrl+C. Der Puffer wird auf die Auswahl genau zugeschnitten alloziert, es wird also gar nichts abgeschnitten — auch nicht in der Mitte eines Codepoints.
- Auswahl zeichnen (Teilzeilen, ganze Zeilen, und der Zeilenumbruch als Leerzeichenbreite), Cursor blinken, Cursor ins Bild holen — **nur wenn er sich wirklich bewegt hat**.
- `fuiRegisterFocusable` und `fuiGetFrameTime` in `final_ui.h` (v0.9.8), Editor in der Tab-Kette.
- `fuiEditorConfig`: `colors.selectionBackground`, `colors.caret`, `metrics.caretWidth`, `toggles.isInteractive`.

**Abnahme:** *Erfüllt.* Als Prüfung im `--selftest` automatisiert (`[copy against file]`): `final_ui.h` wird geladen, komplett markiert, herauskopiert und byteweise gegen die Datei gehalten — plus eine Auswahl, die nicht bei null anfängt, damit die Offsets und nicht nur die Länge geprüft werden. `[wheel against caret]` fährt den Fallstrick ab: mit dem Rad wegscrollen, drei Frames nichts tun, der Offset muss stehen bleiben; dann eine Pfeiltaste, und er muss zurückkommen. `[keyboard]` drückt Tasten kopflos, `[line geometry]` prüft, dass Offset↔Position über Tabulatoren hinweg zueinander invers sind.

**Damit ist die Kerniteration Read-Only fertig.**

**Was noch aussteht:** Was `fuiSetClipboardText` mit dem Text macht, ist Sache der Plattform — FPLs X11-Backend kopiert ihn in `clipboardOut[FPL_MAX_BUFFER_LENGTH]` (2048 Bytes) und wirft den Rest weg. Der Editor selbst gibt die ganze Auswahl heraus; `fuiEditorCopySelection` ist der Weg, sie vollständig zu bekommen. Siehe Abschnitt 9.

### Iteration 3 — Whitespace und Einfärben

- Leerzeichen als `·`, Tabulator als Pfeil über die volle Tabstoppbreite, Zeilenende als `CR`/`LF`/`CRLF`. Abschaltbar.
- Style-Array, `fuiEditorStyleDef`-Tabelle, Lexer-Callback, `styledUpToLine`, Zeilenzustände, Konvergenzabbruch.
- Dekorationsschicht: Zeilenhintergrund und Randspaltenmarker, plus explizite Bereichsliste für Teilzeilen.
- Zeichnen in Style-Läufen, an Tabulatoren geschnitten.
- Demo: kleiner C-Lexer und eine Diff-Ansicht.

**Abnahme:** Der C-Lexer färbt `final_ui.h`. Ans Dateiende springen, Zeile 3 ändern — das Nachfärben bleibt unter einem Frame.

### Iteration 4 — Bearbeitungsmodus

- Tippen, Enter, Backspace und Entf **mit Berücksichtigung der Auswahl**.
- Einfg schaltet Einfügen/Überschreiben um, sichtbar am Cursor (Strich gegen Kasten).
- Ctrl+V, **mittlere Maustaste** (`FUI_MOUSE_MIDDLE` ist über `fui_input_fpl.h` verdrahtet; `fuiInteract` reagiert nur auf links, der Editor fragt den Knopf selbst ab), Ctrl+X (Auswahl, sonst die ganze Zeile), Ctrl+D löscht die Zeile.
- Geändert-Flag, `onChange`-Callback, `isReadOnly` sperrt jeden schreibenden Zweig.

**Abnahme:** Im Demo eine Datei laden, bearbeiten, speichern; das Ergebnis stimmt byteweise.

### Iteration 5 — Undo/Redo und Blockoperationen

- Undo-Stapel mit Zusammenfassen: eine getippte Wortfolge ist ein Schritt. Ctrl+Z, Ctrl+Y, Ctrl+Shift+Z, Cursor und Auswahl inklusive.
- Tab/Shift+Tab rücken eine Markierung ein und aus, Ctrl+Shift+D dupliziert, Alt+Hoch/Runter verschiebt, Enter übernimmt die Einrückung.

**Abnahme:** 200 gemischte Schritte, 200-mal rückgängig — byteweise der Ausgangszustand. Dann 200-mal vorwärts und wieder identisch.

### Iteration 6 — Suchen und Ersetzen

- Overlay im Editor (nicht modal), Ctrl+F suchen, Ctrl+R mit ausklappbarer Ersetzen-Zeile, Escape schließt. Die Felder sind `fuiTextInput`.
- F3 / Shift+F3, alle Treffer markieren, Groß-/Kleinschreibung, ganzes Wort, „Alle ersetzen" als **ein** Undo-Schritt.
- Gehe-zu-Zeile über Ctrl+G.

**Abnahme:** In `final_ui.h` nach `fui__` suchen — die Trefferzahl stimmt mit `grep -o | wc -l`. „Alle ersetzen" geht mit einem Ctrl+Z zurück.

### Iteration 7 — Encodings und Zeilenumbruch

- Backends: UTF-16LE, UTF-16BE, UTF-7, Latin-1/CP1252 (UTF-8 und ASCII stehen). BOM-Erkennung, Zeilenende-Normalisierung.
- `fuiEditorSaveToMemory` über die Vtable, mit dem gemerkten Ursprungs-Encoding.
- Optionaler Zeilenumbruch: zweiter Index Bildschirmzeile ↔ Dokumentzeile, bei Breitenänderung neu gebaut. Zeilennummer nur an der ersten Bildschirmzeile einer Dokumentzeile.

**Abnahme:** Eine UTF-16LE-Datei mit BOM laden, eine Zeile ändern, speichern — Bytes bis auf die Änderung identisch. Umbruch an/aus, ohne dass Cursor oder Auswahl springen.

### Iteration 8 — Shortcuts, Performance, Doku

- Shortcut-Tabelle in der Config, umbelegbar; fest bleiben nur Enter, Backspace, Entf und Ctrl+C/Ctrl+V. Vokabular von `fuiShortcut`/`fuiModifier` (`final_ui.h:3565–3585`).
- `PerfSubject_Editor` in `FUI_Performance`, im `--benchmark`-Modus messbar: Anzeigen, Scrollen, mit Lexer, mit Umbruch. Zahlen in Abschnitt 7.
- Changelog `v1.0.0` im Header, im Ton der bestehenden Einträge, mit den gemessenen Zahlen.
- `final_ui.h`: Changelog für die zwei Zusätze, Status-Absatz, und das stehengebliebene `@version v0.9.5` (`final_ui.h:185`) auf `FUI_VERSION_PATCH` bringen.
- `README.md`: Zeile für `final_ui_texteditor.h`, Versionsspalte von `final_ui.h` (steht auf `0.9.5-beta`).
- `final_game_tech.md`: Ordnerliste, Demoliste, Beschreibungen um `FUI_Editor`.

**Abnahme:** `--benchmark` liefert eine Zahlentabelle, alle Dokumentationsstellen stimmen.

---

## 6. Was der Aufrufer tut — das Muster

```c
#define FUI_IMPLEMENTATION
#include <final_ui.h>

#define FUI_TEXTEDITOR_IMPLEMENTATION
#include <final_ui_texteditor.h>

// Einmal
fuiEditor editor;
fuiEditorInit(&editor, fui_null);
fuiEditorLoadFromMemory(&editor, fileBytes, fileLength, fui_null);   // null = UTF-8

// Je Frame (ab Iteration 1)
fuiRect editorRect = fuiLayoutRemaining(&ui);
fuiEditorAction action = fuiTextEditor(&ui, editorRect, "source", &editor);
if(action.didChange) {
	MarkDocumentDirty();
}

// Einmal
fuiEditorRelease(&editor);
```

Der Editor gehört dem Aufrufer und wird nicht in `fuiWidgetState` gesucht — zwei Editoren nebeneinander sind zwei `fuiEditor` und teilen nichts.

---

## 7. Demo und Performance

### 7.1 `demos/FUI_Editor`

Eigenes Demo statt eines Panels in `FUI_Test`, weil ein Editor mit Randspalte, Statusleiste, Encoding-Umschalter, Lexer-Auswahl und Diff-Ansicht auf ein paar hundert Pixel nicht vorführbar ist.

Aufbau wie `FUI_Test`: FPL + legacy OpenGL, `fui_font_stbtt.h`, `fui_backend_gl1.h`, `fui_input_fpl.h`. Es füllt sein Dokument aus `final_ui.h` selbst — die größte Datei zur Hand und zugleich das, wogegen der Editor sich messen lassen muss.

### 7.2 Der `--selftest`-Modus

Kopflos nach dem Vorbild von `PerfRunBenchmark` (`fui_performance.c:2277`): `fplInitFlags_None`, kein Fenster, kein OpenGL, ein Exit-Code. Prüfmakros wie in `apps/mathtest/mathtest.c`.

Das ist der Modus, gegen den entwickelt wird, denn ein Gap-Buffer ist genau die Art Sache, die auf dem Bildschirm richtig aussieht und über der Lücke falsch ist — und ein Zeilenindex genau die Art Sache, die irgendwo in der Mitte einer Datei um eins danebenliegt, zu der niemand gescrollt hat. Einundzwanzig Gruppen: leeres Dokument, Zeilenindex, Einfügen, Löschen, Lückenbewegung, Wachstum, Zeilenenden, UTF-8, Encodings, Ansichtshelfer, zusammenhängende Läufe, Cursorzeile, Dokument gegen Datei, Widget-Layout, leeres Widget, Zeilengeometrie, Wörter, Auswahl, Tastatur, Rad gegen Cursor, und Kopieren gegen Datei.

Zu jeder Textprüfung gehören zwei Vergleiche — einmal stückweise über `fuiEditorCopyRange`, einmal zusammenhängend über `fuiEditorGetContiguousText`. Stimmen die nicht überein, sähen ein Lexer und eine Suche zwei verschiedene Dokumente.

```
cd demos/FUI_Editor
cmake --preset linux-x64-debug
cmake --build ../immediates/FUI_Editor/linux-x64-debug
../build/FUI_Editor/Linux-x64-Debug/FUI_Editor --selftest
```

Und mit Sanitizern, was bei einem Gap-Buffer die eigentliche Prüfung ist:

```
gcc -std=c99 -g -fsanitize=address,undefined demos/FUI_Editor/fui_editor_demo.c \
    -I . -I demos/additions -I demos/dependencies -o /tmp/fui_editor_asan -lm -ldl
/tmp/fui_editor_asan --selftest
```

### 7.3 Gemessen

*Wird in Iteration 8 gefüllt.*

| Fall | Zeilen | ms/Frame | Draw-Commands |
|---|---|---|---|
| — | — | — | — |

---

## 8. Bewusst offen

- **Code-Faltung.** Bräuchte einen zweiten, sichtbaren Zeilenindex über dem Dokumentindex und einen Faltungsgrad je Zeile vom Lexer. Machbar auf dem, was hier steht, aber eine Iteration für sich.
- **Klammer-Hervorhebung, Einrückungslinien, rechte Randlinie, Lesezeichen.** Reine Zeichenarbeit auf dem, was Iteration 1–3 ohnehin bauen. Bewusst nicht im Umfang.
- **Mehrfach-Cursor und Spaltenauswahl.** Ändert das Cursormodell von einem Paar auf eine Liste, und damit jede einzelne Bearbeitungsfunktion.
- **Autovervollständigung.** Wäre ein Callback plus ein Popup; hängt an nichts hier.
- **Klassisches Mac als Zeilenende im Dokument.** Nur ein Line Feed trennt Zeilen; ein reiner Carriage-Return-Text wird beim Laden normalisiert, nicht im Dokument verstanden.
- **FPLs Zwischenablage.** `fplSetClipboardText` kopiert unter X11 in einen festen Puffer von 2048 Bytes und schneidet still ab; `fplGetClipboardText` liest aus demselben. Für einen Editor ist das zu klein — eine markierte Datei sind schnell hunderte Kilobyte. Zu lösen ist das in `final_platform_layer.h` mit dynamischem Speicher für den Zwischenablagepuffer, nicht hier. Bewusst ein eigenes Thema: es ist eine Änderung an einer plattformübergreifenden Datei, die nichts mit dem Editor zu tun hat, und beim Setzen großer Selektionen kommt über X11 obendrein das INCR-Protokoll ins Spiel.
- **X11 PRIMARY selection.** Die mittlere Maustaste fügt aus der normalen Zwischenablage ein, weil FPL nur `CLIPBOARD` kennt und nicht `PRIMARY`. Unter Linux ist das nicht ganz die gewohnte Geste.

---

## 9. Risiken

| Risiko | Gegenmaßnahme |
|---|---|
| ~~FiraCode bläht `final_fonts.h` auf~~ ✅ | Beide Schnitte sind drin. `final_fonts.h` ist von 2,39 MB auf 3,22 MB gewachsen, also 830 KB für Bitstream Vera Sans Mono (49 KB Fontdaten) und FiraCode (290 KB). Das war tragbar, die befürchteten ~2 MB allein für FiraCode sind es nicht geworden |
| **Bestätigt:** FPLs X11-Zwischenablage hat eine feste Puffergrenze. `fplSetClipboardText` kopiert über `fplCopyString` in `clipboardOut[FPL_MAX_BUFFER_LENGTH]`, also 2048 Bytes, und wirft den Rest weg. Unter Windows gibt es die Grenze nicht, da wird passend alloziert | Der Editor macht seinen Teil vollständig: `fuiEditorCopySelection` gibt die ganze Auswahl heraus, und Ctrl+C alloziert genau dafür. Was die Plattform daraus macht, ist ihre Sache — und `fplSetClipboardText` meldet die Kürzung nicht, kann vom Editor also auch nicht erkannt werden. **Eigenes Thema: FPL braucht dort dynamisches Speichermanagement**, siehe Abschnitt 8 |
| Nachfärben nach einer Änderung weit über dem Sichtfenster | Zustandskonvergenz-Abbruch, in Iteration 3 mit genau diesem Fall abgenommen |
| Viele Style-Läufe je Zeile treiben die Draw-Commands hoch | `fuiSetDrawBatching`, Läufe gleicher Farbe zusammenfassen, in Iteration 8 messen |
| Rückwärtsscrollen mit Umbruch ist beim alten Textfeld O(Dokument) (`final_ui.h:9258`) | Der zweite Index wird einmal je Breite gebaut und gehalten, nicht je Frame |
| Die breiteste Zeile ist die breiteste *gesehene* — der waagerechte Bereich wächst also beim Durchscrollen | Bewusst so, und dokumentiert. Scintilla verhält sich genauso. Eine Änderung setzt ihn zurück |
| Der Cursor der Suchfelder und der des Editors stören sich | Der Editor hält seinen eigenen, `fuiTextInput` seinen auf dem Kontext. In Iteration 6 gegeneinander prüfen |
| Dokumente über 2 GB | `int32_t` durchgängig. Bewusst: die Grenze ist dokumentiert und für einen Texteditor keine |
