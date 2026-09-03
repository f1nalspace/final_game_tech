# Handout: `final_ui_texteditor.h`

Übergabe für die nächste Session. Der **Plan** steht in [`fui_texteditor_plan.md`](fui_texteditor_plan.md) — Designentscheidungen, API, innerer Aufbau, alle acht Iterationen samt Abnahmekriterien. Dieses Dokument ergänzt ihn um das, was beim Bauen gelernt wurde und nirgends sonst steht.

**Kurzfassung:** Iterationen 0–3 sind fertig und committet. Als Nächstes kommt **Iteration 4 — Bearbeitungsmodus**. Alles ist grün, nichts hängt.

---

## 1. Stand

| | |
|---|---|
| Branch | `ui/editor-widget`, Basis `develop` |
| Letzter Commit | `4ed637e4 Scrollbar background color wired` |
| Arbeitsverzeichnis | sauber, alles committet |
| `final_ui.h` | **v0.9.7** (`develop` steht auf v0.9.6) |
| `final_ui_texteditor.h` | **v0.4.0**, 4496 Zeilen, 98 öffentliche Funktionen |
| `demos/FUI_Editor/` | 2796 Zeilen, davon der größere Teil Selbsttest |
| Selbsttest | **354 Prüfungen, 0 Fehler**, sauber unter ASan und UBSan |

### Was der Editor kann

Dokument (Gap-Buffer, Split-Zeilenindex, Encoding-Seam) · Ansicht mit Randspalte, Zeilennummern, Tabstopps, Monospace-Schnellweg, beiden Scrollbalken und eigener Statusleiste · Cursor, Auswahl, Tastatur, Maus, Kopieren · Lexer mit inkrementellem Nachfärben · Dekorationsschicht · sichtbarer Whitespace und Zeilenenden.

**Was er nicht kann:** schreiben. Tippen, Undo, Suchen/Ersetzen, weitere Encodings und Zeilenumbruch sind Iteration 4 bis 8.

---

## 2. Bauen, testen, hinschauen

```sh
# Schnell, mit Sanitizern - das ist der Weg für die Entwicklung
gcc -std=c99 -g -Wall -Wextra -Wno-unused-parameter -fsanitize=address,undefined \
    demos/FUI_Editor/fui_editor_demo.c -I . -I demos/additions -I demos/dependencies \
    -o /tmp/fui_editor_asan -lm -ldl
/tmp/fui_editor_asan --selftest

# Regulär
cd demos/FUI_Editor && cmake --preset linux-x64-debug
cmake --build ../immediates/FUI_Editor/linux-x64-debug
./demos/build/FUI_Editor/Linux-x64-Debug/FUI_Editor --selftest
```

Vor jedem Abschluss zusätzlich durchgeprüft: `gcc -std=c17 -O2`, `clang -std=c99` und `g++ -std=c++11` (Letzteres über eine winzige TU, die beide Header mit `IMPLEMENTATION` einbindet). Alle warnungsfrei.

### Screenshot vom laufenden Demo

Ein Screenshot des **Root-Fensters** erwischt oft das falsche Fenster. Über die Fenster-ID geht es zuverlässig:

```sh
timeout 30 bash -c '
./demos/build/FUI_Editor/Linux-x64-Debug/FUI_Editor > /dev/null 2>&1 &
DEMOPID=$!
sleep 5
WINID=$(xwininfo -root -tree | grep "final_ui_texteditor.h demo" | head -1 | awk "{print \$1}")
if [ -n "$WINID" ]; then import -window "$WINID" /tmp/shot.png; fi
kill $DEMOPID 2>/dev/null
wait $DEMOPID 2>/dev/null
'
magick /tmp/shot.png -crop 900x180+30+415 +repage -resize 250% /tmp/zoom.png
```

Vorhanden sind `import` (ImageMagick), `magick`, `xwininfo`, `spectacle`, `ffmpeg`. **Kein** `xdotool`, **kein** `wmctrl` — es lässt sich also nichts anklicken. Wer eine Interaktion sehen will, setzt sie per temporärem Patch im Demo vor und nimmt ihn danach zurück.

**Das ist keine Nebensache.** Der Scrollbalken-Bug (siehe unten) war drei Iterationen unentdeckt, weil ich Screenshots gemacht, aber nicht genau genug hingesehen habe. Bei jeder Zeichenänderung: Screenshot **und** hineinzoomen.

---

## 3. Arbeitsregeln, die der Nutzer gesetzt hat

1. **Versionen.** Ein Feature-Branch dreht die Version einer Bibliothek **genau einmal** hoch — eine Patch-Stufe über `develop` — und bleibt dort. Alles, was der Branch dazu beiträgt, kommt als weitere `- New:`-Zeile in dieselbe Changelog-Sektion. Keine neue Sektion je Iteration. Gilt für `final_ui.h` (v0.9.7, bleibt). `final_ui_texteditor.h` existiert auf `develop` nicht, hat also keine Basis, über der es stehen könnte — dort läuft je Iteration eine Minor-Stufe (aktuell v0.4.0). Bisher unbeanstandet; im Zweifel nachfragen.

2. **FPLs Zwischenablage ist ein eigenes Thema.** Nicht im Editor umgehen. Details in Abschnitt 6.

3. **Commits macht der Nutzer selbst**, oft parallel aus einer Git-GUI. Nicht committen, wenn nicht danach gefragt wird — und nicht überrascht sein, wenn `git status` zwischendurch sauber wird.

4. Code-Stil steht in `CLAUDE.md` und wird streng genommen: benannte Zwischenvariablen statt verschachtelter Aufrufe, keine magischen Zahlen, keine Zeilenumbrüche auf Spaltenbreite, geschweifte Klammern um **jeden** Bedingungskörper, Kommentare auf Englisch.

---

## 4. Was beim Bauen anders lief als geplant

Chronologisch. Jeder Punkt ist ein Fehler, der Geld gekostet hat, oder eine Entscheidung, die vom Plan abweicht.

### Iteration 0 — Fundament

- **Die Lücke im Textpuffer darf sich nie ganz schließen.** `fuiEditorGetContiguousText` schreibt seine terminierende Null an `bytes[textLength]`. `fuiEditor__DocumentReserve` fordert deshalb grundsätzlich ein Byte mehr an als verlangt.
- **Beide Reservierungen vor der ersten Bewegung.** Eine fehlgeschlagene Reservierung mittendrin ließe Text- und Zeilenindex gegeneinander verstellt zurück — kein Absturz, sondern falsche Zeilen ab dieser Stelle.
- **Lückenbewegung vor `tailDelta`.** Das Verschieben rechnet mit `tailDelta`, muss also davor laufen. Steht als Kommentar an beiden Stellen.

### Iteration 1 — Read-Only-Ansicht

- **Tabstopps dürfen nicht auf dem Stift gerechnet werden.** `penX - lineLeftX` durch die Tabbreite geteilt kommt bei großem `lineLeftX` als 0,99999 heraus — der zweite Tabulator einer Zeile bewegt den Stift dann gar nicht. Gerechnet wird auf dem **Abstand zum Zeilenanfang**, plus einer Toleranz am Stopp (`FUI_TEXTEDITOR__TAB_STOP_EPSILON`). Beides ist nötig.
- **`fuiEditorScrollToLine` kann beim Aufruf nicht rechnen.** Die Zeilenhöhe steht in der Schrift, die der *Kontext* trägt. Der Aufruf merkt sich nur die Zeile, der nächste Build löst sie auf.
- **Die Konfiguration wird gegen das Theme gehalten, aus dem sie aufgelöst wurde** (ein `memcmp` je Frame). Sonst bliebe ein umgestylter Kontext unbemerkt.
- **Die waagerechte Reichweite ist die breiteste *gesehene* Zeile.** Jede Zeile jedes Frame zu messen wäre der Gang über das ganze Dokument, den das Widget vermeiden soll. Scintilla macht es genauso.

### Iteration 2 — Cursor, Auswahl, Tastatur

- **`fuiGetFrameTime` war nicht geplant.** Ein Add-on bekommt nur den Kontext gereicht und hatte damit keine Zeitquelle — kein Blinken, kein Doppelklick, kein Auto-Scrollen.
- **Zeichnen und Cursorposition kommen aus derselben Quelle** — einem Segment-Walk über die Zeile. Zwei getrennte Rechnungen liefen an jedem Tabulator und jedem Kerning-Paar auseinander.
- **Kerning ist nur als Differenz erreichbar:** `Messung(Paar) − Messung(erstes)`. `final_ui.h` gibt das Messen heraus, nicht die Kerningtabelle. Zwei O(1)-Messungen je Zeichen statt einer Präfixmessung je Kandidat, die quadratisch würde. Bei Monospace fällt beides weg.
- **Die Zeile des Cursors wird vor dem Layout gemessen.** Sonst kann eine Pfeiltaste, die in eine lange Zeile hineinläuft, nicht seitwärts zu ihr scrollen.

### Iteration 3 — Whitespace und Einfärben

- **Kein Style-Array je Dokumentbyte** — abweichend vom Plan. Gebaut ist ein `int32_t` **Parser-Zustand je Zeile**, in denselben Slots wie die Zeilenanfänge; die Style-Bytes werden für die sichtbaren Zeilen in einen Scratch gelext. Verhalten identisch, spart einen zweiten Gap-Buffer über 650 KB. Scintilla speichert die Styles, weil seine Lexer extern sind; hier ist der Lexer ein Callback, der die sichtbaren Zeilen ohnehin je Frame durchläuft.
- **Die Konvergenz-Schwelle war falsch:** `lineCount` statt höchster Zeilenindex, Abfrage `>=` statt `>`. Folge: nach einem vollständigen Durchlauf wurde sie nie zurückgesetzt, und jede spätere Änderung färbte das ganze Dokument neu — genau das, was das Verfahren verhindern soll. **Gefunden nur, weil der Test die Anzahl der Lexer-Aufrufe zählt** statt das Ergebnis zu prüfen.
- **Nur wirklich neue Zeilen sind „ungeschrieben".** Eine Einfügung ohne Zeilenvorschub legt keinen Zustandsslot an und darf die Schwelle nicht anheben.
- **Läufe werden an Stilgrenzen zerschnitten, aber als Präfix gemessen** — die Breiten summieren sich teleskopisch zu genau dem, was das ganze Stück misst. Ohne das liefe eine eingefärbte Zeile pro Stilgrenze um ein Kerning-Paar gegen den Cursor davon.

### Nachträglich vom Nutzer gemeldet

- **Beide Scrollbalken waren unsichtbar — seit Iteration 1.** Der Hintergrund deckt den ganzen Rahmen und wurde *nach* ihnen gezeichnet. Am Layout war nichts falsch, jede Prüfung, die Geometrie *zählt*, war grün. Die Prüfung, die es jetzt festhält (`[scrollbar is not painted over]`), geht über die **Reihenfolge**, in der die Geometrie ausgegeben wird.
- **Die Scrollrinne hatte keine eigene Farbe.** Sie wurde in `widgetTrackColor` gezeichnet — derselben Farbe, mit der sich ein scrollender Container selbst füllt. Neu: `fuiTheme.scrollTrackColor`, zwischen versenktem Feld und Daumen.
- **Die Zeilenende-Marke saß ein Leerzeichen zu weit rechts.** Ein Abstand dort ist ein Zeichen, das nicht im Dokument steht, liest sich aber wie eines. Steht jetzt bündig; was sie trennt, ist ihre Farbe.

---

## 5. Zusätze in `final_ui.h` (alle in der v0.9.7-Sektion)

| Was | Warum |
|---|---|
| `fuiScrollbarHorizontal` | `fui__Scrollbar(..., false)` gab es intern längst, öffentlich war nur die vertikale Fassung |
| `fuiRegisterFocusable` | Ohne öffentliche Fassung ist ein fremdes Widget das Einzige, was Tab überspringt |
| `fuiGetFrameTime` | Ohne Zeitquelle kann ein Add-on nichts takten. Gibt im Draw-Pass null zurück |
| `fuiTheme.scrollTrackColor` | Die Rinne ging im Feld daneben unter |

Der Editor benutzt **ausschließlich** die öffentliche API von `final_ui.h` — kein `fui__`-Interna. Wenn etwas fehlt, kommt es öffentlich dazu, aber nur, wenn es auch anderen Widgets nützt. Das war bisher jedes Mal der Fall.

---

## 6. Offene Punkte

### FPLs Zwischenablage (eigenes Thema, nicht im Editor lösen)

`fplSetClipboardText` kopiert unter X11 über `fplCopyString` in `clipboardOut[FPL_MAX_BUFFER_LENGTH]` (2048 Bytes). `fplCopyString` schreibt bei zu kleinem Ziel **gar nichts** und liefert null — der Selection-Owner wird trotzdem übernommen und liefert null Bytes aus. Die Zwischenablage ist danach **leer**, samt dem, was vorher darin stand, und der Aufruf meldet Erfolg. Unter Windows gibt es die Grenze nicht.

Der Nutzer will das in `final_platform_layer.h` mit dynamischem Speicher gelöst haben, nicht hier. Beim Setzen großer Selektionen kommt über X11 obendrein das INCR-Protokoll ins Spiel.

Solange: Die Größengrenze gehört in den Plattform-Hook. `demos/FUI_Editor` macht das in `DemoSetClipboardText` — was nicht hineinpasst, wird verweigert (`return false`), und die vorhandene Zwischenablage bleibt in Ruhe.

### Kleinigkeiten

- Eine **echte Diff-Ansicht** braucht zwei Fassungen. Das Demo vergleicht positionsweise gegen die geladene Datei — welche Zeilen sich *verschoben* haben, findet ein Diff heraus, und das ist Sache des Aufrufers.
- **`fuiEditorGetCaretColumn` zählt Codepoints**, nicht die Spalten, die ein Tabulator überspannt. Bewusst: welche der beiden eine Statusleiste zeigen soll, ist Geschmack des Aufrufers.
- **Bildschirmzeile und Dokumentzeile sind überall getrennt benannt**, obwohl sie noch dasselbe sind. Der Zeilenumbruch (Iteration 7) macht sie verschieden.

---

## 7. Die Tests

`--selftest` läuft kopflos: kein Fenster, kein OpenGL, ein Exit-Code. 26 Gruppen, 354 Prüfungen. Wichtig sind vor allem:

| Gruppe | Was sie festhält |
|---|---|
| `[document against file]` | Jede der 14 000+ Zeilen von `final_ui.h` gegen einen rohen Scan der Datei — Anfang, Ende, Inhalt und der Rückweg über `fuiEditorGetLineOfOffset` |
| `[copy against file]` | Alles markieren, herauskopieren, byteweise vergleichen. Plus eine Auswahl, die nicht bei null anfängt |
| `[wheel against caret]` | Mit dem Rad wegscrollen, drei Frames nichts tun — der Offset muss stehen bleiben. Dann eine Pfeiltaste, und er muss zurückkommen |
| `[incremental colouring]` | **Zählt die Lexer-Aufrufe.** 2000 Zeilen einmal = 2000, nochmal fragen = 0, Änderung in Zeile 3 danach ≤ 2 — und die Gegenprobe, dass ein geöffneter Blockkommentar *nicht* früh abbricht |
| `[scrollbar is not painted over]` | Geht über die **Reihenfolge** der ausgegebenen Geometrie, nicht über ihre Menge |
| `[widget layout]`, `[keyboard]` | Bauen das Widget kopflos gegen eine Schrift bekannter Maße und drücken Tasten (`EditorTestHarness`) |

**Die Lehre aus dieser Runde:** Prüfungen, die nur das *Ergebnis* ansehen, haben zwei der drei ernsten Fehler nicht gesehen. Was sie gefunden hat, waren Prüfungen über **Kosten** (wie viele Lexer-Aufrufe) und über **Reihenfolge** (welche Geometrie zuletzt). Bei allem, was „inkrementell" oder „wird gezeichnet" heißt: nicht das Was prüfen, sondern das Wie viel und das Wann.

Der Prüfrahmen selbst ist mehrfach gegengeprüft worden — teils absichtlich (Toleranz auf null, Zeichenreihenfolge zurückgedreht), teils weil er echte Fehler und einen Stack-Overflow im Testcode gemeldet hat.

---

## 8. Als Nächstes: Iteration 4 — Bearbeitungsmodus

Aus dem Plan:

- Tippen, Enter, Backspace und Entf **mit Berücksichtigung der Auswahl**.
- Einfg schaltet Einfügen/Überschreiben um, sichtbar am Cursor (Strich gegen Kasten).
- Ctrl+V, **mittlere Maustaste**, Ctrl+X (Auswahl, sonst die ganze Zeile), Ctrl+D löscht die Zeile.
- Geändert-Flag, `onChange`-Callback, `isReadOnly` sperrt jeden schreibenden Zweig.

**Abnahme:** Im Demo eine Datei laden, bearbeiten, speichern; das Ergebnis stimmt byteweise.

### Was dabei absehbar wehtut

- **Die mittlere Maustaste braucht wahrscheinlich einen fünften Zusatz in `final_ui.h`.** `fuiInteract` reagiert nur auf links, und es gibt keine öffentliche Abfrage für die anderen Knöpfe (`context->mouseIsDown[]` ist intern). Wenn es so kommt: als weitere `- New:`-Zeile in die **bestehende** v0.9.7-Sektion, Version bleibt.
- **`fuiEditorConfig` bekommt die Unterstruktur `callbacks`.** `shortcuts` folgt erst in Iteration 8.
- **Jeder schreibende Zweig muss über `fuiEditorInsert`/`fuiEditorErase` laufen**, denn nur die halten Zeilenindex, Lexer-Wasserstand und `version` nach. Nichts an `document.bytes` vorbei.
- **Der Undo-Stapel kommt erst in Iteration 5.** Iteration 4 schreibt also ohne Netz — das ist so gewollt, aber das Demo sollte es sagen.
- **Das Demo hat schon eine Baseline** der geladenen Datei (`DemoTakeBaseline`) und die Ansicht der geänderten Zeilen. Beides wird ab Iteration 4 von selbst interessant, weil das Dokument wirklich divergiert.
