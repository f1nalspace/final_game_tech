# Handout: `final_ui_texteditor.h`

Übergabe für die nächste Session. Der **Plan** steht in [`fui_texteditor_plan.md`](fui_texteditor_plan.md) — Designentscheidungen, API, innerer Aufbau, alle acht Iterationen samt Abnahmekriterien. Dieses Dokument ergänzt ihn um das, was beim Bauen gelernt wurde und nirgends sonst steht.

**Kurzfassung:** Iterationen 0–5 sind fertig. Der Editor liest, schreibt und **nimmt zurück**. Als Nächstes kommt **Iteration 6 — Suchen und Ersetzen**. Alles ist grün, nichts hängt.

---

## 1. Stand

| | |
|---|---|
| Branch | `ui/editor-widget`, Basis `develop` |
| Arbeitsverzeichnis | Iteration 5 ist **nicht committet** — der Nutzer committet selbst |
| `final_ui.h` | **v0.9.7** (`develop` steht auf v0.9.6), von Iteration 5 **nicht angefasst** |
| `final_ui_texteditor.h` | **v0.6.0**, ~6560 Zeilen |
| `demos/FUI_Editor/` | ~4450 Zeilen, davon der größere Teil Selbsttest |
| Selbsttest | **809 Prüfungen, 0 Fehler**, sauber unter ASan und UBSan |

### Was der Editor kann

Dokument (Gap-Buffer, Split-Zeilenindex, Encoding-Seam) · Ansicht mit Randspalte, Zeilennummern, Tabstopps, Monospace-Schnellweg, beiden Scrollbalken und eigener Statusleiste · Cursor, Auswahl, Tastatur, Maus, Kopieren · Lexer mit inkrementellem Nachfärben · Dekorationsschicht · sichtbarer Whitespace und Zeilenenden · Tippen, Enter, Backspace, Entf, Überschreibmodus, Ausschneiden, Einfügen (auch mittlere Maustaste), Zeilenlöschen, Geändert-Flag, `onChange`, `isReadOnly` · **Undo/Redo mit Zusammenfassen, Gruppen, Speicherbudget und Speicherpunkt · Tab/Shift+Tab, Ctrl+Shift+D, Alt+Hoch/Runter, Auto-Einrückung.**

**Was er nicht kann:** suchen und ersetzen, andere Encodings als UTF-8 und ASCII, Zeilenumbruch. Das sind Iteration 6 bis 8.

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

Vorhanden sind `import` (ImageMagick), `magick`, `xwininfo`, `spectacle`, `ffmpeg`. **Kein** `xdotool`, **kein** `wmctrl` — es lässt sich also nichts anklicken.

**Was man deshalb macht:** die zu zeigende Situation per **temporärem Patch** im Demo vorsetzen und ihn danach zurücknehmen. In Iteration 4 waren das drei Zeilen in `DemoInit` (Überschreibmodus an, Cursor auf eine sichtbare Zeile, ein `fuiEditorInsert`) plus zwei in `BuildUserInterface` (`fuiSetFocusedId` und `caretBlinkTime = 0`, sonst ist gar kein Cursor zu sehen — er ist unfokussiert unsichtbar und blinkt). Vorher eine Kopie der Datei wegsichern und hinterher **mit `diff -q` prüfen**, dass sie wirklich wieder da ist.

**Das ist keine Nebensache.** Der Scrollbalken-Bug war drei Iterationen unentdeckt, weil Screenshots gemacht, aber nicht genau genug hingesehen wurde. Bei jeder Zeichenänderung: Screenshot **und** hineinzoomen.

---

## 3. Arbeitsregeln, die der Nutzer gesetzt hat

1. **Versionen.** Ein Feature-Branch dreht die Version einer Bibliothek **genau einmal** hoch — eine Patch-Stufe über `develop` — und bleibt dort. Alles, was der Branch dazu beiträgt, kommt als weitere `- New:`-Zeile in dieselbe Changelog-Sektion. Keine neue Sektion je Iteration. Gilt für `final_ui.h` (v0.9.7, bleibt — auch die drei Zusätze aus Iteration 4 sind dort einsortiert). `final_ui_texteditor.h` existiert auf `develop` nicht, hat also keine Basis, über der es stehen könnte — dort läuft je Iteration eine Minor-Stufe (aktuell v0.6.0). Bisher unbeanstandet; im Zweifel nachfragen.

2. **FPLs Zwischenablage ist ein eigenes Thema.** Nicht im Editor umgehen. Details in Abschnitt 6.

3. **Commits macht der Nutzer selbst**, oft parallel aus einer Git-GUI. Nicht committen, wenn nicht danach gefragt wird — und nicht überrascht sein, wenn `git status` zwischendurch sauber wird.

4. Code-Stil steht in `CLAUDE.md` und wird streng genommen: benannte Zwischenvariablen statt verschachtelter Aufrufe, keine magischen Zahlen, keine Zeilenumbrüche auf Spaltenbreite, geschweifte Klammern um **jeden** Bedingungskörper, Kommentare auf Englisch.

---

## 4. Was beim Bauen anders lief als geplant

Chronologisch. Jeder Punkt ist ein Fehler, der Geld gekostet hat, oder eine Entscheidung, die vom Plan abweicht. Iterationen 0–3 stehen ausführlich im Plan, Abschnitt 1; hier nur ihre Kurzform, dann Iteration 4 vollständig.

### Iterationen 0 bis 3, in einem Satz je Punkt

- Die Lücke im Textpuffer darf sich nie ganz schließen (`fuiEditorGetContiguousText` schreibt an `bytes[textLength]`).
- Beide Reservierungen vor der ersten Lückenbewegung, sonst stehen Text- und Zeilenindex gegeneinander verstellt.
- Tabstopps auf dem **Abstand zum Zeilenanfang** rechnen, nicht auf dem Stift — sonst bewegt der zweite Tabulator einer Zeile gar nichts.
- Zeichnen und Cursorposition kommen aus **einem** Segment-Walk; zwei Rechnungen laufen an jedem Tabulator auseinander.
- Kein Style-Array je Byte, sondern ein Parser-Zustand je Zeile in denselben Slots wie die Zeilenanfänge.
- Beide Scrollbalken waren seit Iteration 1 unsichtbar — der Hintergrund wurde **nach** ihnen gezeichnet. Gefunden hat es der Nutzer, nicht der Test.

### Iteration 4 — Bearbeitungsmodus

- **Der Cursor wird von der Änderung bewegt, nicht vom Zweig, der sie gemacht hat.** `fuiEditorInsert` und `fuiEditorErase` rücken Cursor, Auswahlanker und Drag-Anker über `fuiEditor__PositionAfterChange` selbst. Damit stimmt es für *jeden* Weg ins Dokument, den programmatischen des Aufrufers eingeschlossen — und kein schreibender Zweig kann es vergessen.
- **`fuiEditorSetText` hatte damit sofort einen Fehler.** Es setzte den Cursor auf null und füllte danach; der Insert hat ihn prompt ans Dateiende getragen. Der Cursor wird jetzt **nach** dem Füllen zurückgesetzt.
- **Drei Zusätze in `final_ui.h`, nicht einer.** Vorhergesehen war `fuiMouseButtonWentDown` (die mittlere Maustaste), dazu kam das Gegenstück `fuiIsMouseButtonDown` und `fuiConsumeKey` — ohne Letzteres committet ein Dialog auf demselben Enter, aus dem der Editor gerade einen Zeilenumbruch gemacht hat. Das mehrzeilige Textfeld in `final_ui.h` griff dafür bisher direkt in `context->keys[...]` und benutzt jetzt die neue Funktion.
- **Ein Ausschneiden, dessen Kopie fehlgeschlagen ist, löscht nicht.** Es gibt bis Iteration 5 keinen Undo-Stapel, und FPLs Hook verweigert oberhalb von zwei Kilobyte.
- **Überschreiben ist durch das definiert, was es *nicht* frisst:** kein Zeilenende. `fuiEditorGetLineEnd` lässt das Ende weg, also ist das CR gleich mit geschützt. Ein Text mit Zeilenvorschub darin überschreibt gar nichts.
- **Backspace und Entf behandeln CR+LF als *ein* Ende.** Sonst bleibt ein Carriage Return am Ende der verbundenen Zeile stehen — nichts zeichnet ihn, nichts markiert ihn, niemand findet ihn.
- **Enter schreibt bei `Eol_Cr` und `Eol_Mixed` ein Line Feed**, nicht das gemeldete Ende: im Dokumentmodell beendet *nur* ein Line Feed eine Zeile, ein eingefügtes `\r` hätte gar keine neue gemacht.
- **`onChange` läuft bei `SetText`/`LoadFromMemory` nicht.** Das Füllen geht intern durch dasselbe `fuiEditorInsert`, also braucht es ein internes `isReplacingDocument`.
- **Getippte Zeichen werden zu einem Insert gesammelt** — festgehalten über die **Anzahl der Versionssprünge**, nicht über den Text danach.
- **Der Überschreib-Cursor ist ein Umriss, kein Block.** Ein gefüllter Kasten deckt genau das Zeichen zu, um das es geht, und eine Glyphe invertieren kann man von hier aus nicht.

### Iteration 5 — Undo/Redo und Blockoperationen

- **`fuiEditorCopyRange` schreibt IMMER eine terminierende Null.** Ein Puffer von genau `byteCount` Bytes bekommt deshalb `byteCount − 1` Bytes und eine Null — bei einem Backspace also null Bytes und eine Null. Der Undo-Stapel legte prompt `\0` statt des Zeichens zurück: Dokument richtig lang, falsch gefüllt. Es gibt jetzt **`fuiEditor__CopyRangeRaw`** ohne Null, und jede Stelle, die Dokumentbytes in einen exakt passenden Puffer kopiert, benutzt sie. Betroffen waren sechs Stellen — alle neu, alle in derselben Stunde geschrieben, alle mit demselben Fehler. **Wer Dokumentbytes in einen exakt passenden Puffer kopiert, nimmt `fuiEditor__CopyRangeRaw`.**
- **Der vorhergesagte siebte Zusatz zu `final_ui.h` war keiner.** `tabWasConsumedThisFrame` braucht keine öffentliche Fassung: `fui__RegisterFocusable` und der Umlauf in `fuiEndFrame` fragen beide über `fuiKeyWentDown`, und genau das nullt `fuiConsumeKey`. Ein `fuiConsumeKey(context, fuiKey_Tab)` reicht.
- **Was Tab bedeutet, entscheidet nicht die Taste, sondern wer den Fokus hatte, als der Build anfing.** `fuiRegisterFocusable` kann dem Editor die Tastatur *in diesem Build* geben — danach wäre `isFocused` wahr und die Taste noch ungenutzt, und ein Tab in den Editor hinein hätte gleichzeitig eingerückt. Der Fokus wird deshalb **vor** dem Eintragen in die Kette gelesen (`alreadyHadTheKeyboard`).
- **Zusammenfassen ist doch keine Zeitfrage.** Der Plan sah Scintillas Pause vor. `fuiEditorInsert`/`fuiEditorErase` sind ohne jeden Frame aufrufbar, ein programmatischer Aufrufer hat also keine Frame-Zeit — der Stapel läge für ihn anders als für die Tastatur. Die Regel ist jetzt: **gleiche Stelle, gleiche Art, klein genug für einen Tastendruck (64 Byte), kein Zeilenvorschub darin, Cursor unbewegt.** Der Cursorsprung ist ohnehin das schärfere Signal.
- **Der Löschlauf sammelt rückwärts.** Backspace nimmt vor dem Cursor, also gehört das, was der zweite Druck nimmt, **vor** das des ersten. Anhängen dreht den Text beim Zurücknehmen von innen nach außen — richtig lang, falsch sortiert, und erst mit drei Drücken hintereinander zu sehen.
- **Alt musste den Pfeiltasten weggenommen werden.** Der Cursor-Zweig steht vor dem Zeilen-Zweig, ein Alt+Hoch hätte sonst erst den Cursor bewegt und dann die Zeile unter ihm weggeschoben. Festgehalten über den **Cursor-Offset danach**, nicht über den Text — der stimmt fast.
- **Der Speicherpunkt ist eine Position im Stapel, keine Fahne.** `fuiEditorClearModified` merkt sich, wo gespeichert wurde; Undo/Redo rechnen `isModified` daraus. Und er wird **ungültig**, sobald der Zweig, auf dem er lag, weggeworfen wird — speichern, ein Schritt zurück, etwas anderes schreiben: gleich lange Historie, andere Schritte.
- **Das Budget wirft nur ganze Schritte weg**, und auch ein **zusammengefasster** Lauf wird daran gemessen — er legt keinen Datensatz an, wächst aber die Arena.
- **Zwei Sicherheitsnetze wurden wieder ausgebaut**, weil keine Prüfung sie rot bekommen konnte (`mayCoalesce = false` in `fuiEditorBeginUndoGroup` und `fuiEditorEndUndoGroup` — beide überflüssig neben der Gruppenabfrage in `fuiEditor__UndoTryCoalesce`). Toter Code, den keine Prüfung sehen kann, ist schlechter als kein Code.

---

## 5. Zusätze in `final_ui.h` (alle in der v0.9.7-Sektion)

| Was | Warum |
|---|---|
| `fuiScrollbarHorizontal` | `fui__Scrollbar(..., false)` gab es intern längst, öffentlich war nur die vertikale Fassung |
| `fuiRegisterFocusable` | Ohne öffentliche Fassung ist ein fremdes Widget das Einzige, was Tab überspringt |
| `fuiGetFrameTime` | Ohne Zeitquelle kann ein Add-on nichts takten. Gibt im Draw-Pass null zurück |
| `fuiTheme.scrollTrackColor` | Die Rinne ging im Feld daneben unter |
| `fuiIsMouseButtonDown`, `fuiMouseButtonWentDown` | `fuiInteract` antwortet nur für links |
| `fuiConsumeKey` | Eine Taste, die dieses Widget beantwortet hat, muss aufgebraucht werden |

**Iteration 5 hat nichts hinzugefügt.** Erwartet war ein siebter Zusatz für die Tab-Taste; er war nicht nötig (siehe Abschnitt 4).

Der Editor benutzt **ausschließlich** die öffentliche API von `final_ui.h` — kein `fui__`-Interna. Wenn etwas fehlt, kommt es öffentlich dazu, aber nur, wenn es auch anderen Widgets nützt. Das war bisher jedes Mal der Fall.

---

## 6. Offene Punkte

### FPLs Zwischenablage (eigenes Thema, nicht im Editor lösen)

`fplSetClipboardText` kopiert unter X11 über `fplCopyString` in `clipboardOut[FPL_MAX_BUFFER_LENGTH]` (2048 Bytes). `fplCopyString` schreibt bei zu kleinem Ziel **gar nichts** und liefert null — der Selection-Owner wird trotzdem übernommen und liefert null Bytes aus. Die Zwischenablage ist danach **leer**, samt dem, was vorher darin stand, und der Aufruf meldet Erfolg. Unter Windows gibt es die Grenze nicht.

Der Nutzer will das in `final_platform_layer.h` mit dynamischem Speicher gelöst haben, nicht hier. Beim Setzen großer Selektionen kommt über X11 obendrein das INCR-Protokoll ins Spiel.

Solange: Die Größengrenze gehört in den Plattform-Hook. `demos/FUI_Editor` macht das in `DemoSetClipboardText` — was nicht hineinpasst, wird verweigert (`return false`), und die vorhandene Zwischenablage bleibt in Ruhe. Der Editor zieht daraus jetzt eine Konsequenz: **ein Ctrl+X, dessen Kopie verweigert wurde, löscht nichts.**

**Beim Einfügen** ist es die andere Richtung: `fuiGetClipboardText` schreibt in einen Puffer der Größe, die man ihm *sagt*, und lässt sich nicht fragen, wie viel wirklich da ist. Also muss eine Zahl vor dem Lesen feststehen — `FUI_TEXTEDITOR_MAX_PASTE_BYTES`, per Default 65536.

### Kleinigkeiten

- ~~Tab rückt nicht ein.~~ Erledigt in Iteration 5. `fuiConsumeKey(context, fuiKey_Tab)` genügt: `tabWasConsumedThisFrame` musste nicht öffentlich werden.
- Eine **echte Diff-Ansicht** braucht zwei Fassungen. Das Demo vergleicht positionsweise gegen die geladene Datei — welche Zeilen sich *verschoben* haben, findet ein Diff heraus, und das ist Sache des Aufrufers.
- **`fuiEditorGetCaretColumn` zählt Codepoints**, nicht die Spalten, die ein Tabulator überspannt.
- **Bildschirmzeile und Dokumentzeile sind überall getrennt benannt**, obwohl sie noch dasselbe sind. Der Zeilenumbruch (Iteration 7) macht sie verschieden.
- **Das Layout eines Frames wird vor der Eingabe gerechnet.** Ein Enter, das eine Zeile dazutut, wird von den Scrollbalken erst im nächsten Frame gesehen. Selbstkorrigierend und ohne Zugriff außerhalb des Puffers, aber es ist bekannt.

---

## 7. Die Tests

`--selftest` läuft kopflos: kein Fenster, kein OpenGL, ein Exit-Code. 42 Gruppen, 809 Prüfungen. Wichtig sind vor allem:

| Gruppe | Was sie festhält |
|---|---|
| `[edits against a plain buffer]` | **Die schärfste.** 400 gemischte Änderungen an pseudozufälligen Stellen von `final_ui.h`, einmal durch den Editor und einmal per `memmove` über einen flachen Puffer, am Ende byteweise verglichen — plus der Zeilenindex gegen einen rohen Scan nach Zeilenvorschüben |
| `[document against file]` | Jede der 14 000+ Zeilen gegen einen rohen Scan der Datei — Anfang, Ende, Inhalt und der Rückweg über `fuiEditorGetLineOfOffset` |
| `[copy against file]` | Alles markieren, herauskopieren, byteweise vergleichen |
| `[typing]` | Zählt die **Versionssprünge**: ein Frame voller Tasten ist *ein* Insert |
| `[cut, paste and the line commands]` | Unter anderem eine Zwischenablage, die **alles verweigert** — und ein Ausschneiden, das dann nichts löscht |
| `[an edit moves the caret]` | Cursor und beide Auswahlenden über Einfügen davor, dahinter und mittendurch |
| `[wheel against caret]` | Mit dem Rad wegscrollen, drei Frames nichts tun — der Offset muss stehen bleiben |
| `[incremental colouring]` | **Zählt die Lexer-Aufrufe.** 2000 Zeilen einmal = 2000, nochmal fragen = 0, Änderung in Zeile 3 danach ≤ 2 |
| `[scrollbar is not painted over]` | Geht über die **Reihenfolge**, in der die Geometrie ausgegeben wird |
| `[two hundred steps, back and forward again]` | **Die Abnahme von Iteration 5.** 200 gemischte Schritte über `final_ui.h`, einzeln zurückgenommen und byteweise gegen die Datei gehalten, dann 200-mal vorwärts und byteweise gegen den bearbeiteten Stand. Zählt die Schritte in beide Richtungen mit |
| `[a run of typing is one step]` | Zählt, **wie oft Ctrl+Z gedrückt werden muss**. Der Text stimmt so oder so; die Anzahl ist das, was Zusammenfassen überhaupt entscheidet |
| `[tab against the focus chain]` | Baut den Frame **von Hand** mit einem `fuiRegisterFocusable` HINTER dem Editor — nur so ist zu sehen, ob die Taste aufgebraucht wurde oder weitergelaufen ist |
| `[the undo budget]` | Schritte aus **je zwei** Datensätzen, damit „halb weggeworfen" von „ganz" zu unterscheiden ist. Liest zum Schluss `undo.arenaLength` selbst, weil ein zusammengefasster Lauf am Schrittzähler unsichtbar ist |
| `[the keys they are on]` | Ctrl+Z/Y/Shift+Z, Ctrl+D gegen Ctrl+Shift+D, Alt+Pfeil gegen Pfeil, Shift+Tab — dass die Funktionen stimmen, sagt nichts darüber, ob die Taste sie erreicht |

Der kopflose Rahmen `EditorTestHarness` kann inzwischen: Tasten mit **allen drei Modifiern** drücken (`HarnessPressChord`, `HarnessPressKey` ist der Aufsatz ohne Alt), **tippen** (`HarnessTypeText` füllt `input.textInput`), **mit der mittleren Maustaste klicken** (`HarnessClickMiddleAt` — baut erst einen Frame, weil Hovern gegen den *vorigen* Build aufgelöst wird) und hat eine **eigene Zwischenablage**, die sich auf Kommando weigert.

**Die Lehre, die sich in Iteration 4 bestätigt hat:** Prüfungen über das *Ergebnis* allein finden zu wenig. Was findet, sind Prüfungen über **Kosten** (wie viele Versionssprünge, wie viele Lexer-Aufrufe), über **Reihenfolge** (welche Geometrie zuletzt) und gegen eine **zweite, dumme Implementierung**.

Und: **jede neue Absicherung wurde absichtlich kaputtgemacht und die Suite dabei rot gesehen.** Iteration 4: Cursor-Nachführung (21 Fehler), Ausschneiden ohne Kopie (3), CR vor LF (3), Überschreiben über das Zeilenende hinaus (7), ein Insert je getipptem Zeichen (1). Iteration 5: 28 weitere Eingriffe, von der Reihenfolge im Löschlauf (2) über Tab in den Editor hinein (4) und das Zeilenende beim Verschieben am Dateiende (8) bis zur rohen Kopie (9) und dem Aufzeichnen vor dem Löschen (21).

**Das ist nicht Zierrat, das findet etwas.** In Iteration 5 waren **zwei** frisch geschriebene Prüfungen blind — der Speicherpunkt (weil kein Test `fuiEditorClearModified` überhaupt aufrief) und das Budget (weil ein Schritt aus einem Datensatz „halb weggeworfen" nicht von „ganz" unterscheiden kann). Beide wurden erst durch den Eingriff sichtbar und danach geschärft. Eine Prüfung, die man nicht rot gesehen hat, prüft nichts.

**Und wenn ein Eingriff die Suite hängen lässt statt sie rot zu machen** (`fuiEditorUndo`, das zurückgibt „ja" ohne den Cursor zu bewegen, dreht die `while(fuiEditorUndo(...))`-Schleifen endlos), ist das dasselbe Signal — nur mit `timeout` davor laufen lassen.

---

## 8. Als Nächstes: Iteration 6 — Suchen und Ersetzen

Aus dem Plan:

- Overlay im Editor (nicht modal), Ctrl+F suchen, Ctrl+R mit ausklappbarer Ersetzen-Zeile, Escape schließt. Die Felder sind `fuiTextInput`.
- F3 / Shift+F3, alle Treffer markieren, Groß-/Kleinschreibung, ganzes Wort, „Alle ersetzen" als **ein** Undo-Schritt.
- Gehe-zu-Zeile über Ctrl+G.

**Abnahme:** In `final_ui.h` nach `fui__` suchen — die Trefferzahl stimmt mit `grep -o | wc -l`. „Alle ersetzen" geht mit einem Ctrl+Z zurück.

### Was dabei absehbar wehtut

- **„Alle ersetzen" als ein Schritt ist schon fertig.** `fuiEditorBeginUndoGroup` / `fuiEditorEndUndoGroup` sind genau dafür da und werden von fünf Operationen im Header bereits so benutzt. Von hinten nach vorne ersetzen, dann bleiben die Offsets vor der Schreibstelle stehen.
- **Zwei Cursor auf einem Kontext.** Der Editor hält seinen eigenen, `fuiTextInput` seinen auf dem Kontext (`caretOwner`, `caretPosition`, `selectionAnchor`). Ein Suchfeld im Overlay hat also den Fokus, und der Editor darf in dem Frame keine Tasten fressen — `result.isFocused` ist die Stelle, an der das entschieden wird. Gegeneinander prüfen, wie im Plan unter Risiken vermerkt.
- **Escape und Enter im Overlay müssen aufgebraucht werden**, sonst schließt der Dialog darüber mit. `fuiConsumeKey` ist da; der Editor macht es beim Enter bereits vor.
- **Suchen geht über `fuiEditorGetContiguousText`** — ein `memmem` über den zusammenhängenden Puffer, nicht ein Gang über den Gap-Buffer. Das kostet **eine** Lückenbewegung und danach nichts mehr, solange nicht geschrieben wird.
- **Die Treffermarkierung ist eine Dekoration**, keine neue Zeichenschicht: `fuiEditorRangeDecoration` gibt es seit Iteration 3, und die Arrays gehören dem Aufrufer. Sortiert und überlappungsfrei übergeben, das ist die Bedingung.
- **Das Demo braucht die Zeile mit der Trefferzahl** und der Selbsttest den Vergleich gegen `grep -o 'fui__' final_ui.h | wc -l`.
