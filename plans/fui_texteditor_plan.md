# Plan: Editor-Widget als `final_ui_texteditor.h`

Ziel: Ein richtiger Code- und Texteditor als eigenständiger Single-Header neben `final_ui.h` — mit Randspalte, Zeilennummern, Tabulatoren, sichtbarem Whitespace, Syntax-Einfärbung über einen Lexer, Diff-Einfärbung über ganze Zeilen, Überschreibmodus, Suchen und Ersetzen, Undo, und einem Dokument, das weiß, in welchem Encoding und mit welchem Zeilenende es angekommen ist. Und das alles bei einer Datei von einer Million Zeilen genauso flüssig wie bei einer von zwanzig.

Dieses Dokument beschreibt fünf Dinge: den **Stand** (1), die **Designentscheidungen** (2), die **öffentliche API** (3), den **inneren Aufbau** (4) und die **Iterationen** samt Demo- und Performance-Arbeit (5–8).

Der Auslöser steht in `docs_fpl/editor-widget.md`.

---

## 1. Stand

**Iterationen 0 bis 6 sind umgesetzt, dazu der Encoding-Teil von Iteration 7.** Was es gibt: das Dokument, eine Ansicht darauf, die gelesen, gescrollt, markiert, kopiert und eingefärbt werden kann, die seit Iteration 4 auch schreibt, seit Iteration 5 einen Weg zurück aus allem hat, was sie schreibt — und seit Iteration 6 gefragt werden kann, wo etwas steht. Damit sind **beide Kerniterationen** und **beide Komfortiterationen** fertig.

- `final_ui_texteditor.h` v0.8.0 — Gap-Buffer, Split-Zeilenindex, Encoding-Vtable mit **sieben** Backends (UTF-8, ASCII, UTF-16LE, UTF-16BE, UTF-7, Latin-1, CP1252), `fuiEditorSaveToMemory` und `fuiEditorDetectEncoding`, dazu `fuiTextEditor` mit Randspalte, Zeilennummern, Tabstopps, Monospace-Schnellweg, beiden Scrollbalken, eigener Statusleiste, Cursor, Auswahl, Tastatur, Maus, Kopieren, Lexer, Dekorationen, sichtbarem Whitespace, Tippen, Enter, Backspace, Entf, Überschreibmodus, Ausschneiden, Einfügen, Zeilenlöschen, Geändert-Flag, `onChange`, Undo/Redo mit Zusammenfassen, Gruppen, Speicherbudget und Speicherpunkt, Tab/Shift+Tab, Ctrl+Shift+D, Alt+Hoch/Runter und Auto-Einrückung — und **Suchen, Ersetzen und Gehe-zu-Zeile**: eine Suchleiste im Widget, alle Treffer markiert, „n von m", Groß-/Kleinschreibung, ganzes Wort, F3, „Alle ersetzen" als ein Undo-Schritt — jedes der drei über `toggles.canFind` / `canReplace` / `canGoToLine` **einzeln abschaltbar**. `fuiEditorConfig` mit `colors` / `metrics` / `toggles` / `limits` / `callbacks`.
- `final_ui.h` v0.9.7 — `fuiScrollbarHorizontal`, `fuiRegisterFocusable`, `fuiGetFrameTime`, `fuiIsMouseButtonDown`, `fuiMouseButtonWentDown` und `fuiConsumeKey` sind öffentlich, und `fuiTheme.scrollTrackColor` gibt der Scrollrinne eine eigene Farbe. **Iteration 5 und Iteration 6 haben nichts hinzugefügt** — die ganze Suchleiste ist `fuiTextInput`, `fuiButton`, `fuiCheckbox` und `fuiLabel` über die öffentliche API. Die Version bleibt bei einer Patch-Stufe über `develop` und wird nicht je Iteration weitergedreht.
- `demos/FUI_Editor/` — zeigt `final_ui.h` selbst (über 14 000 Zeilen, 654 KB), mit Umschaltern für Zeilennummern, Statusleiste, aktuelle Zeile, Interaktivität, Tabbreite, Schriftschnitt, C-Lexer, Whitespace, Zeilenenden, geänderte Zeilen, Nur-Lesen, Auto-Einrückung und Einrücken mit Leerzeichen, plus Auswahl, Kopieren, Undo/Redo mit Schrittzähler, Duplizieren, einer Zeile, die jede Änderung meldet, einem **„Save & verify"**, das schreibt und byteweise zurückliest — und **Knöpfen für Suchen, Ersetzen und Gehe-zu-Zeile samt einem „Count against the file"**, das die Trefferzahl des Editors gegen einen flachen Lauf über die gelesene Datei hält.
- `--selftest` läuft mit **1200 Prüfungen** sauber unter AddressSanitizer und UndefinedBehaviorSanitizer durch, davon ein kopfloser Rahmen, der Tasten mit allen drei Modifiern drückt, tippt, mit der mittleren und mit der linken Maustaste klickt und die Antwort zurückliest.

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

Und was bei Iteration 3 dazukam:

- **Kein Style-Array je Dokumentbyte.** Der Plan sah eins vor, nach Scintillas Vorbild. Gebaut ist stattdessen ein `int32_t` **Parser-Zustand je Zeile**, im selben Split-Array wie die Zeilenanfänge, und die Style-Bytes werden für die sichtbaren Zeilen in einen Scratch gelext. Das Verhalten ist dasselbe — die Inkrementalität hängt an den *Zuständen*, nicht an den Styles — aber es spart einen zweiten Gap-Buffer über 650 KB, der bei jeder Einfügung mitgezogen werden müsste. Scintilla speichert die Styles, weil seine Lexer extern sind; hier ist der Lexer ein Callback, der die sichtbaren Zeilen ohnehin je Frame durchläuft.
- **Die Konvergenz-Schwelle war zuerst falsch.** Sie stand auf `lineCount` statt auf dem höchsten Zeilenindex, und die Abfrage war `>=` statt `>`. Folge: nach einem vollständigen Durchlauf wurde sie nie zurückgesetzt, und die nächste Änderung hat wieder das ganze Dokument neu gefärbt — genau das, was das Verfahren verhindern soll. Aufgefallen ist es nur, weil der Test die **Anzahl der Lexer-Aufrufe zählt** statt nur das Ergebnis zu prüfen.
- **Nur wirklich neue Zeilen sind „ungeschrieben".** Eine Einfügung ohne Zeilenvorschub legt keinen neuen Zustandsslot an und darf die Schwelle deshalb auch nicht anheben.
- **Ein Zeilenstück wird an Stilgrenzen zerschnitten, aber als Präfix gemessen.** Jeder Lauf wird von *Stückanfang* bis Laufende gemessen und die Differenz genommen, sodass die Breiten sich teleskopisch zu genau dem aufsummieren, was das ganze Stück misst. Ohne das liefe eine eingefärbte Zeile pro Stilgrenze um ein Kerning-Paar gegen den Cursor davon.
- **Beide Scrollbalken waren unsichtbar — seit Iteration 1.** Der Hintergrund deckt den ganzen Rahmen und wurde *nach* ihnen gezeichnet, hat sie also übermalt, sobald sie da waren. Am Layout war nichts falsch, die Balken wurden wirklich gebaut, und jede Prüfung, die Geometrie *zählt*, war grün — nur hingeschaut hatte niemand. Aufgefallen ist es dem Nutzer. Die Prüfung, die es jetzt festhält, geht deshalb über die **Reihenfolge**, in der die Geometrie ausgegeben wird: der Daumen trägt die Widget-Farbe, der Hintergrund die Track-Farbe, und der letzte Vertex der einen muss vor dem letzten der anderen liegen. Mit dem alten Code wird sie rot — gegengeprüft.
- **Die Scrollrinne hatte keine eigene Farbe.** Sie wurde in `widgetTrackColor` gezeichnet — genau der Farbe, mit der sich ein scrollender Container selbst füllt. Sichtbar war der Balken damit auch nach dem Reihenfolge-Fix kaum: die Rinne ging im Feld daneben unter, und der Daumen schien frei im Inhalt zu schweben. `fuiTheme.scrollTrackColor` liegt jetzt zwischen dem versenkten Feld und dem Daumen, sodass beide Kanten der Rinne auf einen Blick lesbar sind. Das ist eine Änderung an `final_ui.h`, von der ListView und Scroll-Panel genauso profitieren.
- **Die Zeilenende-Marke saß ein Leerzeichen zu weit rechts.** Ein Abstand dort ist ein Zeichen, das nicht im Dokument steht, liest sich aber wie eines — der Cursor kann nicht hinein, markieren lässt es sich auch nicht. Die Marke steht jetzt bündig am letzten Zeichen; was sie vom Text trennt, ist ihre Farbe.
- **Ein Marker mit `*` und `/` darin ist ein schlechter Marker.** Der „Zeile 3 ändern"-Knopf hängte zuerst einen Blockkommentar an — der den Kommentar schließt, in dem der Dateikopf von `final_ui.h` steht, und damit den halben Bildschirm umfärbt. Korrektes C, verwirrendes Demo. (Der Kommentar, der das erklärt, musste aus demselben Grund umgeschrieben werden.)

Und was bei Iteration 4 dazukam:

- **Der Cursor wird von der Änderung bewegt, nicht vom Zweig, der sie gemacht hat.** Geplant war das nirgends. `fuiEditorInsert` und `fuiEditorErase` rücken Cursor, Auswahlanker und Drag-Anker jetzt selbst — und damit stimmen sie für *jeden* Weg ins Dokument: Tastendruck, Einfügen aus der Zwischenablage, mittlere Maustaste, und den programmatischen `fuiEditorInsert` des Aufrufers gleich mit. Die Alternative wäre gewesen, es in jedem schreibenden Zweig einzeln zu tun, und einer davon hätte es vergessen.
- **`fuiEditorSetText` hatte damit sofort einen Fehler.** Es setzt den Cursor auf null und füllt danach — und ein Insert, der jede Position hinter sich mitzieht, hat ihn prompt ans Dateiende getragen. Der Cursor wird jetzt **nach** dem Füllen zurückgesetzt. Gefunden von `[an edit moves the caret]`, das genau diese Zeile prüft.
- **`final_ui.h` brauchte drei Zusätze, nicht einen.** Vorhergesehen war die mittlere Maustaste (`fuiMouseButtonWentDown`, dazu das Gegenstück `fuiIsMouseButtonDown`). Dazu kam `fuiConsumeKey`: das mehrzeilige Textfeld in `final_ui.h` greift dafür seit jeher direkt in `context->keys[...]`, und ohne öffentliche Fassung kann ein fremdes Widget das Enter, aus dem es einen Zeilenumbruch gemacht hat, nicht aufbrauchen — der Dialog darüber committet dann auf demselben Tastendruck. Das Textfeld benutzt jetzt selbst die neue Funktion.
- **Ein Ausschneiden, dessen Kopie fehlgeschlagen ist, löscht nicht.** FPLs Hook verweigert oberhalb von zwei Kilobyte, und einen Undo-Stapel gibt es erst in Iteration 5 — ein Ctrl+X, das trotzdem gelöscht hätte, wäre ein Löschen ohne Rückweg. Als Prüfung drin, mit einer Zwischenablage, die *alles* verweigert.
- **Der Überschreibmodus ist durch das definiert, was er *nicht* frisst.** Ein überschriebener Zeilenumbruch verbindet zwei Zeilen, und das ist nicht, was „ein Zeichen ersetzen" heißt. Also: Halt am Zeilenende, und ein Text mit Zeilenvorschub darin überschreibt gar nichts.
- **Backspace und Entf behandeln CR+LF als *ein* Ende.** Nur das LF zu nehmen ließe ein Carriage Return am Ende der verbundenen Zeile stehen — ein Zeichen, das nichts zeichnet, nichts markiert und niemand findet, in einer Datei, die völlig richtig aussieht.
- **Enter schreibt das Ende, mit dem das Dokument angekommen ist** — aber bei `fuiEditorEol_Cr` und `_Mixed` ein Line Feed, weil im Dokumentmodell *nur* ein Line Feed eine Zeile beendet. Ein „\r" einzufügen hätte gar keine neue Zeile gemacht.
- **`onChange` läuft bei `fuiEditorSetText` und `fuiEditorLoadFromMemory` nicht.** Die ersetzen das Dokument, statt es zu ändern, und der Aufrufer war es selbst. Dafür gibt es ein internes `isReplacingDocument`, weil das Füllen intern durch dasselbe `fuiEditorInsert` läuft.
- **Getippte Zeichen werden zu *einem* Insert gesammelt.** Was das festhält, ist nicht der Text danach — der stimmt so oder so — sondern die **Anzahl der Versionssprünge**. Genau die Art Prüfung, die letzte Iteration den Fehler im Lexer-Wasserstand gefunden hat.
- **Die schärfste Prüfung ist eine zweite, dumme Implementierung.** `[edits against a plain buffer]` fährt 400 gemischte Einfügungen und Löschungen an pseudozufälligen Stellen über `final_ui.h` — einmal durch den Editor, einmal per `memmove` über einen flachen `malloc`-Puffer — und vergleicht am Ende byteweise. Danach wird der Zeilenindex noch einmal gegen einen rohen Scan nach Zeilenvorschüben gehalten, denn Bytes, die stimmen, sagen nichts über Zeilen, die es nicht tun.
- **Jede neue Absicherung wurde absichtlich kaputtgemacht und die Suite dabei rot gesehen** — Cursor-Nachführung (21 Fehler), Ausschneiden ohne Kopie (3), CR vor LF (3), Überschreiben über das Zeilenende hinaus (7), ein Insert je Zeichen (1). Eine Prüfung, die man nicht rot gesehen hat, prüft nichts.

Und was bei Iteration 5 dazukam:

- **`fuiEditorCopyRange` schreibt immer eine terminierende Null.** Ein Puffer von genau `byteCount` Bytes bekommt deshalb `byteCount − 1` Bytes und eine Null obendrauf — bei einem Backspace also *null* Bytes und eine Null. Der Undo-Stapel hat sein erstes Zeichen prompt als `\0` zurückgeschrieben, und das Dokument war danach richtig *lang* und falsch *gefüllt*. Gefunden hat es keine Prüfung über den Text, sondern das Ausdrucken des Datensatzes selbst. Es gibt jetzt `fuiEditor__CopyRangeRaw` ohne Null, und **jede** Stelle, die Dokumentbytes in einen exakt passenden Puffer kopiert, benutzt sie — die drei in `fuiEditor__SwapAdjacentLineRuns`, die zwei in `fuiEditorDuplicate` und die eine in der Auto-Einrückung waren alle sechs betroffen, ohne dass es aufgefallen wäre.
- **Der siebte Zusatz zu `final_ui.h` war keiner.** Erwartet war, dass die Fokuskette eine öffentliche Fassung von `tabWasConsumedThisFrame` braucht. Braucht sie nicht: `fui__RegisterFocusable` und der Umlauf am Frameende fragen beide über `fuiKeyWentDown`, und `fuiConsumeKey` nullt genau das. Ein `fuiConsumeKey(context, fuiKey_Tab)` reicht also.
- **Was Tab bedeutet, entscheidet nicht die Taste, sondern wer den Fokus hatte, als der Build anfing.** `fuiRegisterFocusable` kann dem Editor die Tastatur *in diesem Build* geben, und danach wäre `isFocused` wahr und die Taste noch ungenutzt — ein Tab in den Editor hinein hätte also gleichzeitig eingerückt. Der Fokus wird deshalb **vor** dem Eintragen in die Kette gelesen und als `alreadyHadTheKeyboard` weitergereicht.
- **Zusammenfassen ist doch keine Zeitfrage.** Der Plan sah Scintillas Regel vor: gleiche Stelle, kein Cursorsprung, keine Pause. Die Pause ist gestrichen. `fuiEditorInsert` und `fuiEditorErase` sind ohne jeden Frame aufrufbar — ein Aufrufer, der programmatisch schreibt, hat keine Frame-Zeit, und der Stapel läge dann für ihn anders als für die Tastatur. Was bleibt: **gleiche Stelle, gleiche Art, klein genug für einen Tastendruck, kein Zeilenvorschub darin, und der Cursor hat sich nicht bewegt.** Der Cursorsprung ist ohnehin das schärfere Signal — er ist genau der Moment, in dem der Benutzer etwas anderes meint.
- **Der Löschlauf sammelt rückwärts.** Backspace nimmt die Bytes vor dem Cursor, also gehört das, was der zweite Druck nimmt, **vor** das, was der erste genommen hat. Ein Anhängen hätte den Text beim Rückgängigmachen von innen nach außen gedreht — richtig lang, falsch sortiert, und nur mit drei Tastendrücken hintereinander zu sehen.
- **Alt musste den Pfeiltasten weggenommen werden.** Der Cursor-Zweig steht vor dem Zeilen-Zweig, also hätte ein Alt+Hoch erst den Cursor bewegt und dann die Zeile unter ihm weggeschoben. Festgehalten wird das nicht über den Text — der stimmt fast — sondern über den **Cursor-Offset danach**.
- **Der Speicherpunkt ist eine Cursorposition im Stapel, keine Fahne.** `fuiEditorClearModified` merkt sich, an welcher Stelle der Historie gespeichert wurde; Undo und Redo rechnen `isModified` daraus aus. Der Fall, an dem eine reine Fahne stirbt: speichern, einen Schritt zurück, etwas anderes schreiben — die Historie ist wieder gleich lang, aber es sind nicht dieselben Schritte. Der Speicherpunkt wird deshalb **ungültig**, sobald der Zweig, auf dem er lag, weggeworfen wird.
- **Das Budget wirft nur ganze Schritte weg.** Ein halb weggeworfener Schritt wäre schlimmer als gar keine Historie: ein Ctrl+Z legte einen Teil einer Operation zurück und ließe den Rest stehen. Die Prüfung dafür brauchte einen zweiten Anlauf — mit einem Datensatz je Schritt ist „ganz" von „halb" nicht zu unterscheiden, also schreibt sie jetzt **zwei je Schritt**.
- **Auch ein *zusammengefasster* Lauf muss am Budget gemessen werden.** Er legt keinen Datensatz an, wächst aber die Arena — ein Budget, das nur beim Anlegen hinschaut, schaut bei einem langen Tipplauf nie wieder hin. Das ist die eine Prüfung, die nicht über das Verhalten geht, sondern über `undo.arenaLength` selbst: der Schrittzähler bleibt so oder so bei eins, und genau deshalb kann er es nicht sehen.
- **Zwei Sicherheitsnetze wurden wieder ausgebaut, weil keine Prüfung sie rot bekommen konnte.** `fuiEditorBeginUndoGroup` und `fuiEditorEndUndoGroup` setzten beide `mayCoalesce = false`. Beides ist überflüssig: `fuiEditor__UndoTryCoalesce` verweigert ohnehin, solange eine Gruppe offen ist, und `fuiEditor__RecordEdit` schaltet den Lauf für jeden Datensatz in einer Gruppe selbst ab. Toter Code, den keine Prüfung sehen kann, ist schlechter als kein Code. Was stehen bleibt, ist die Sperre im Budget gegen das Wegwerfen eines Schritts, den Redo noch braucht — mit einem Kommentar, dass heute kein Aufrufer sie erreicht.
- **Und wieder wurde jede neue Absicherung absichtlich kaputtgemacht und die Suite dabei rot gesehen** — die Reihenfolge im Löschlauf (2 Fehler), der Cursorsprung als Laufende (1), Tippen über eine Auswahl als ein Schritt (5), Tab in den Editor hinein (4), die aufgebrauchte Tab-Taste (1), das Zeilenende beim Verschieben am Dateiende (8), der Speicherpunkt (2) und der weggeworfene Zweig (1), ganze Schritte im Budget (5), das Budget beim Zusammenfassen (1), die Stelle, an die angehängt wird (3), die Größe, ab der nicht mehr zusammengefasst wird (1), das Wegwerfen der Redo-Schritte (1), die rohe Kopie (9), Alt gegen die Pfeiltasten (6), Ctrl+Shift+D (4), Ctrl+Shift+Z (2), Ctrl+Y (2), Shift+Tab (2), die markierte Kopie (3), der mitgeführte Cursor beim Verschieben (20), übersprungene Leerzeilen (4), ein Einrückungsschritt statt aller (3), die Auswahl, die am Umbruch aufhört (2), die weggeworfene Historie beim Laden (16), das Aufzeichnen vor dem Löschen (21), das Aufzeichnen nach den Reservierungen (95) und das Duplizieren mit dem Cursor (2). **Zwei Prüfungen waren dabei blind** und wurden geschärft, eine dritte hat einen Aufhänger im Test selbst gefunden.

Und was bei Iteration 6 dazukam:

- **Die Trefferzahl entscheidet, was „der nächste Treffer" heißt.** Der Plan sagt, die Zahl in der Leiste muss mit `grep -o | wc -l` übereinstimmen — und `grep -o` zählt **überlappungsfrei**. Damit war auch entschieden, wo `fuiEditorFindNext` weitersucht: hinter dem **Ende** der Auswahl, nicht ein Byte hinter ihrem Anfang. Sonst fände „aa" in „aaaa" dreimal etwas, während daneben „2" stünde.
- **Die Zahl und der Index sind derselbe Lauf.** Wie viele es sind und der wievielte der aktuelle ist, fallen beide beim selben Durchgang an. Gehalten wird das gegen Dokumentversion, Suchtext, Flags **und beide Enden der Auswahl**. Das letzte fehlte zuerst: ein `fuiEditorFindNext`, das genau dort einen Treffer findet, wo der Cursor schon stand, bewegt nur das **hintere** Ende — der gemerkte Anfang blieb gleich, und die Leiste zeigte weiter „kein Treffer". Gefunden von der Prüfung, nicht beim Lesen.
- **Die Leiste schwebt, und was den Cursor sichtbar macht, weiß das.** Eine Leiste, die sich Platz nimmt, schiebt beim Ctrl+F jede Zeile nach unten und beim Escape wieder hoch. Also schwebt sie — und `fuiEditor__EnsureCaretVisible` bekommt eine **Oberkante**, die nicht die des Rahmens ist. Ohne die läge der Cursor genau dann unter der Leiste, wenn man sie benutzt.
- **Ein Sprung wird zentriert, ein Schritt nur nachgeführt.** Beides über dieselbe Funktion wäre falsch: eine Zeile, die nach einem Sprung bündig an der Kante klebt, an der sie angekommen ist, hat nichts um sich herum zu lesen — und ein F3 innerhalb des Sichtfensters darf die Ansicht nicht umherwerfen. Also: war der Cursor schon zu sehen, wird nur nachgeführt, sonst in die Mitte.
- **Alle Treffer zu markieren heißt: je sichtbarer Zeile suchen.** Eine Liste aller Treffer wäre ein Eintrag je Fund im ganzen Dokument — genau die Rechnung je Dokumentzeile, die dieses Widget nicht macht. Gesucht wird deshalb in jeder sichtbaren Zeile neu, und das kostet die Bytes dieser Zeile.
- **Das Undo-Budget konnte den Schritt wegwerfen, den es gerade schrieb.** Es wirft ganze Schritte am ältesten Ende weg und nie einen, den Redo noch braucht — aber nichts hielt es davon ab, die **offene Gruppe** selbst anzuknabbern. „Alle ersetzen" ist die erste Operation, die genug Datensätze in einer Gruppe schreibt, um das Budget mitten im Lauf zu erreichen. Eine offene Gruppe ist jetzt tabu.
- **`didChange` wird am Ende des Builds gelesen, nicht in der Mitte.** Die Leiste wird als **letztes** gebaut, damit sie den Klick vom Text darunter wegnimmt — und ihre Ersetzen-Knöpfe schreiben. Früher gelesen war eine Ersetzung aus der Leiste eine Änderung, von der niemand erfuhr: der nächste Build vergleicht gegen die neue Version und meldet sie auch nicht mehr.
- **Breiten werden gemessen, nicht gesetzt.** Der erste Entwurf hatte Pixelbreiten für Beschriftungen, Knöpfe und Kästchen. Im Demo — Monospace-Schnitt — war „Match case" abgeschnitten, „1161 found" auch, und „Close" klebte am Nachbarn. Der Schnitt gehört dem Aufrufer, also darf keine Breite geraten werden: jedes Stück der Leiste misst seine eigene Beschriftung. Gesehen wurde das an einem Screenshot, nicht an einer Prüfung.
- **`final_ui.h` brauchte nichts.** Erwartet war, dass ein Ctrl+F den Inhalt des Suchfelds markieren will und dafür ein öffentliches „Feld komplett auswählen" fehlt. Es fehlt wirklich — nur ist die Folge klein genug, um sie hinzunehmen: der Cursor landet **hinter** dem übernommenen Text statt darauf. Notiert für Iteration 8, nicht dafür eine API aufgemacht.
- **Ein Text ohne Null wird auch ohne Null gelesen.** `fuiEditorSetSearchText` nimmt eine Länge, und der Suchtext aus einer Auswahl kommt genau so an: N Bytes, keine terminierende Null. Das Zurückziehen auf eine Zeichengrenze schaute aber auf das Byte **hinter** dem Behaltenen — und wenn nichts abgeschnitten wurde, liegt das schon hinter dem, was der Aufrufer hergegeben hat. Zurückgezogen wird jetzt nur noch, wenn wirklich gekürzt wurde. Gefunden beim Nachlesen des eigenen Codes, nicht von einer Prüfung; die Prüfung dazu legt jetzt ein Continuation-Byte hinter den Text und wird ohne die Absicherung rot.
- **Suchen und Ersetzen mussten einzeln abschaltbar werden.** Nachgefragt, nicht geplant. Der Fall ist ein **Nur-Lese-Diff-Dialog**: der will durchsucht werden und hat nichts anzubieten, womit man ihn ändert. Nur-Lesen allein reichte nicht — die Ersetzen-Zeile wurde weiter gezeichnet, nur mit ausgegrauten Knöpfen, und eine Zeile, die man nie benutzen kann, ist Rauschen statt Information. Es gibt jetzt drei Schalter (`canFind`, `canReplace`, `canGoToLine`), und Nur-Lesen nimmt die Ersetzen-Zeile **ganz** weg, egal was der Schalter sagt. Was sie zumachen, ist der Weg des **Benutzers**; `fuiEditorFind`, `fuiEditorFindNext`, `fuiEditorReplaceAll` und `fuiEditorGoToLine` bleiben dahinter offen — aus demselben Grund, aus dem `fuiEditorInsert` in einer Nur-Lese-Ansicht offen bleibt: wer die Leiste abschaltet, tut das, um seine **eigene** davorzusetzen, nicht um die Suche zu verlieren.
- **Eine Leiste, die nicht mehr erlaubt ist, geht zu.** Sonst stünde sie bis zum nächsten Escape und beantwortete Tasten, deren Wirkung niemand mehr sieht — und die Tastatur läge auf einem Feld, das gar nicht mehr gezeichnet wird. Sie wird geschlossen und der Fokus geht zurück an den Editor.
- **Zwei Prüfungen waren blind und wurden geschärft.** Die überlappungsfreie Zählung hing an einem einzigen „aaaa"; ein zweiter Vergleich über zwei Leerzeichen in `final_ui.h` prüft dasselbe jetzt tausendfach. Und der Klick auf die Leiste traf einen Punkt, den der Editor ohnehin auf den Offset abgebildet hätte, auf dem der Cursor schon stand — er beweist erst etwas, seit er woanders hinzielt.
- **Und wieder wurde jede neue Absicherung absichtlich kaputtgemacht und die Suite dabei rot gesehen** — überlappende Treffer gezählt (2 Fehler in beiden Zählschleifen), der Vergleich an der Lücke vorbei (26), ganzes Wort ohne den Blick nach vorne (6), nie gefaltete Groß-/Kleinschreibung (6), eine Suche ohne Umlauf (8), rückwärts als erster statt als letzter Treffer (6), „Alle ersetzen" als viele Schritte (13), Ersetzen ohne die Prüfung auf einen Treffer (8), die Rückmeldung vor der Leiste gelesen (1), die Leiste, die den Klick nicht nimmt (1), Escape ohne Rückgabe der Tastatur (2), ein nicht aufgebrauchtes Enter (11), Ctrl+F ohne Übernahme der Auswahl (1), das vergessene hintere Auswahlende im gemerkten Stand (1), der Cursor unter der Leiste (1), ein Sprung, der nur nachführt statt zu zentrieren (2), das Zurückziehen auf eine Zeichengrenze ohne Kürzung (1), sowie die drei Schalter einzeln (6, 9, 2), Nur-Lesen gegen die Ersetzen-Zeile (7), die Leiste, die nicht zugeht (4), und das Öffnen mit einer Zeile, die es nicht geben darf (1).

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
| `fuiRegisterFocusable` ✅ | `fui__RegisterFocusable` hängt ein Widget in die Tab-Kette; ohne öffentliche Fassung kann kein fremdes Widget daran teilnehmen | Iteration 2, drin seit `final_ui.h` v0.9.7 |
| `fuiGetFrameTime` ✅ | War nicht geplant. Ein Add-on bekommt nur den Kontext, und ohne Zeitquelle kann es nichts takten — Blinken, Doppelklick, Auto-Scrollen | Iteration 2, drin seit `final_ui.h` v0.9.7 |
| `fuiTheme.scrollTrackColor` ✅ | War nicht geplant. Die Schiene wurde in `widgetTrackColor` gezeichnet — derselben Farbe, mit der sich ein scrollender Container selbst füllt. Die Rinne verschwand also im Feld daneben, und der Daumen schien frei im Inhalt zu schweben | Iteration 3, drin seit `final_ui.h` v0.9.7 |
| `fuiMouseButtonWentDown`, `fuiIsMouseButtonDown` ✅ | `fuiInteract` antwortet für die linke Taste und für keine andere. Ein Widget, das mit der mittleren etwas meint — ein Einfügen —, konnte gar nicht fragen | Iteration 4, drin seit `final_ui.h` v0.9.7 |
| `fuiConsumeKey` ✅ | Braucht ein Widget, das eine Taste selbst beantwortet hat. Das mehrzeilige Textfeld macht das seit jeher über `context->keys[...]` direkt; öffentlich gab es das nicht, und ohne es committet der Dialog auf demselben Enter, das gerade eine Zeile umgebrochen hat | Iteration 4, drin seit `final_ui.h` v0.9.7 |
| *(nichts)* | Iteration 5 hat die Tab-Taste beansprucht und dafür **keinen** Zusatz gebraucht. Erwartet war eine öffentliche Fassung von `tabWasConsumedThisFrame`; sie ist überflüssig, weil sowohl `fui__RegisterFocusable` als auch der Umlauf am Frameende über `fuiKeyWentDown` fragen — und genau das nullt `fuiConsumeKey` bereits | Iteration 5 |
| *(nichts)* | Iteration 6 hat eine ganze Suchleiste gebaut und dafür **keinen** Zusatz gebraucht: `fuiTextInput`, `fuiButton`, `fuiCheckbox`, `fuiLabel`, `fuiInteract`, `fuiBlockMouse` und `fuiMeasureText` reichen. Was wirklich fehlt, ist ein öffentliches „markiere den Inhalt dieses Feldes" — die Folge ist, dass Ctrl+F den Cursor **hinter** den übernommenen Suchtext setzt statt darauf. Klein genug, um auf Iteration 8 zu warten | Iteration 6 |

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

// Einfärben
void fuiEditorSetLexer(fuiEditor *editor, const fuiEditorLexer *lexer);                     // null nimmt ihn weg
void fuiEditorInvalidateStyles(fuiEditor *editor, const int32_t documentLine);
void fuiEditorSetDecorations(fuiEditor *editor, const fuiEditorDecorations *decorations);   // null nimmt sie weg

// Schreiben (Iteration 4). Alles davon geht durch fuiEditorInsert/fuiEditorErase und durch nichts sonst,
// und alles davon ist von toggles.isReadOnly gesperrt - die beiden Primitive selbst nicht
bool fuiEditorInsertAtCaret(fuiEditor *editor, const char *text, const int32_t textLength);
bool fuiEditorInsertLineBreak(fuiEditor *editor);
bool fuiEditorDeleteSelection(fuiEditor *editor);
bool fuiEditorDeleteBackward(fuiEditor *editor);
bool fuiEditorDeleteForward(fuiEditor *editor);
bool fuiEditorDeleteLine(fuiEditor *editor, const int32_t documentLine);

// Zustand des Schreibens
bool fuiEditorIsReadOnly(const fuiEditor *editor);
bool fuiEditorIsModified(const fuiEditor *editor);
void fuiEditorClearModified(fuiEditor *editor);          // was Speichern damit macht, ist Sache des Aufrufers
bool fuiEditorIsOverwriting(const fuiEditor *editor);
void fuiEditorSetOverwriting(fuiEditor *editor, const bool isOverwriting);
```

Zwei Konventionen, die überall gelten:

- **Kopieren sagt immer die volle Länge**, auch wenn es nicht gepasst hat. Also einmal mit `null` fragen, allozieren, nochmal fragen.
- **Ein `textLength` von 0 heißt „bis zur terminierenden Null"**, so wie `fuiMeasureText` es hält.

### 3.2 Was noch kommt

Der Zeilenumbruch (Iteration 7). `fuiEditorConfig` hat seit Iteration 4 auch `callbacks` und seit Iteration 5 `limits`; `shortcuts` fehlt noch und kommt in Iteration 8.

Der Encoding-Teil von Iteration 7 hat dazugelegt: `fuiEditorEncodingUtf16Le`, `fuiEditorEncodingUtf16Be`, `fuiEditorEncodingUtf7`, `fuiEditorEncodingLatin1`, `fuiEditorEncodingCp1252`, `fuiEditorDetectEncoding`, `fuiEditorSaveToMemory`, `fuiEditorGetEncoding`, `fuiEditorSetEncoding`, `fuiEditorHasByteOrderMark` und `fuiEditorSetByteOrderMark` — und `fuiEditorEncoding.getBomLength` ist `getBomBytes` gewichen.

Iteration 6 hat dazugelegt: `fuiEditorFind`, `fuiEditorCountMatches`, `fuiEditorSetSearchText`, `fuiEditorGetSearchText`, `fuiEditorSetReplaceText`, `fuiEditorGetReplaceText`, `fuiEditorSetFindFlags`, `fuiEditorGetFindFlags`, `fuiEditorFindNext`, `fuiEditorFindPrevious`, `fuiEditorGetMatchCount`, `fuiEditorGetCurrentMatchIndex`, `fuiEditorReplaceCurrent`, `fuiEditorReplaceAll`, `fuiEditorOpenFind`, `fuiEditorOpenGoToLine`, `fuiEditorCloseFind`, `fuiEditorIsFindOpen` und `fuiEditorGoToLine` — dazu `fuiEditorFindFlags`, `fuiEditorMatch`, `colors.findHighlightBackground` und `toggles.canFind` / `toggles.canReplace` / `toggles.canGoToLine`.

Iteration 5 hat dazugelegt: `fuiEditorUndo`, `fuiEditorRedo`, `fuiEditorCanUndo`, `fuiEditorCanRedo`, `fuiEditorClearUndo`, `fuiEditorBeginUndoGroup`, `fuiEditorEndUndoGroup`, `fuiEditorBreakUndoRun`, `fuiEditorGetUndoStepCount`, `fuiEditorGetRedoStepCount`, `fuiEditorIndentSelection`, `fuiEditorUnindentSelection`, `fuiEditorDuplicate`, `fuiEditorMoveLinesUp`, `fuiEditorMoveLinesDown` — dazu `toggles.autoIndent`, `toggles.usesSpacesForIndent` und `limits.undoMemoryBytes`.

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

- **Zustandsarray** (Iteration 3, umgesetzt — ohne das geplante Style-Array): ein `int32_t` Parser-Zustand je Zeile, in denselben Slots wie die Zeilenanfänge, plus ein Wasserstand `styledUpToLine`. Eine Änderung in Zeile L setzt `styledUpToLine = min(styledUpToLine, L+1)`. Die Style-Bytes werden für die sichtbaren Zeilen in einen Scratch gelext statt fürs ganze Dokument gespeichert — die Inkrementalität hängt an den Zuständen, nicht an den Styles.

  Der Fall, an dem so etwas sonst stirbt — Cursor auf Zeile 500 000, Änderung in Zeile 3 — wird über **Zustandskonvergenz** abgefangen: das Nachfärben bricht ab, sobald der neu berechnete Ausgangszustand einer Zeile dem gespeicherten entspricht und man hinter allem ist, was neu dazugekommen ist. Alles danach war schon richtig. `lexConvergenceFloor` hält fest, bis zu welchem Zeilenindex ein Slot noch nie von einem Lexer beschrieben wurde — auf einem solchen darf nicht konvergiert werden, denn ein zufällig passender Müllwert sähe genauso aus wie eine echte Übereinstimmung.

- **Tabulatoren** (Iteration 1): `fuiDrawText` kennt kein `\t`. Der Editor zerlegt jede Zeile an den Tabulatoren und setzt x auf den nächsten Tabstopp. Zusammen mit den Style-Läufen aus Iteration 3 sind die Segmente der Schnitt aus beidem.

- **Monospace-Schnellweg** (Iteration 1): Beim Setzen der Schrift wird `"W"` gegen `"i"` gemessen. Sind sie gleich breit, wird Spalte ↔ x eine Multiplikation statt einer Messschleife. Das umgeht nebenbei das O(n²), das `fui__ColumnFromCursorX` (`final_ui.h:9119`) bei langen Codezeilen hat.

- **Der Undo-Stapel** (Iteration 5, umgesetzt): ein Array von Datensätzen plus **eine** Arena, in die in derselben Reihenfolge angehängt wird. Ein Datensatz ist ein Aufruf von `fuiEditorInsert` oder `fuiEditorErase` und trägt beide Hälften — die weggegangenen Bytes zuerst, die dazugekommenen dahinter —, dazu Cursor und Anker davor und danach und eine Gruppen-Nummer.

  ```c
  // Datensätze [0, undoCursor) stehen ANGEWANDT, [undoCursor, recordCount) sind zurückgenommen
  // Arena-Spanne eines Datensatzes: [arenaStart, arenaStart + removedLength + insertedLength)
  ```

  Weil die Arena streng in Datensatzreihenfolge wächst, ist das Wegwerfen der zurückgenommenen Schritte ein Kürzen auf `records[undoCursor].arenaStart`, und das Wegwerfen des ältesten Schritts ein `memmove` von vorne. Ein **Schritt** ist eine Gruppe, nicht ein Datensatz: Tippen sammelt sich durch Zusammenfassen in *einen* Datensatz, und was mehr als einmal schreibt, klammert sich über `fuiEditorBeginUndoGroup`.

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
- `fuiRegisterFocusable` und `fuiGetFrameTime` in `final_ui.h` (v0.9.7), Editor in der Tab-Kette.
- `fuiEditorConfig`: `colors.selectionBackground`, `colors.caret`, `metrics.caretWidth`, `toggles.isInteractive`.

**Abnahme:** *Erfüllt.* Als Prüfung im `--selftest` automatisiert (`[copy against file]`): `final_ui.h` wird geladen, komplett markiert, herauskopiert und byteweise gegen die Datei gehalten — plus eine Auswahl, die nicht bei null anfängt, damit die Offsets und nicht nur die Länge geprüft werden. `[wheel against caret]` fährt den Fallstrick ab: mit dem Rad wegscrollen, drei Frames nichts tun, der Offset muss stehen bleiben; dann eine Pfeiltaste, und er muss zurückkommen. `[keyboard]` drückt Tasten kopflos, `[line geometry]` prüft, dass Offset↔Position über Tabulatoren hinweg zueinander invers sind.

**Damit ist die Kerniteration Read-Only fertig.**

**Was noch aussteht:** Was `fuiSetClipboardText` mit dem Text macht, ist Sache der Plattform — und FPLs X11-Backend macht bei Überlänge gar nichts, hinterlässt aber eine leere Zwischenablage. Der Editor selbst gibt die ganze Auswahl heraus; `fuiEditorCopySelection` ist der Weg, sie vollständig zu bekommen, und die Größengrenze gehört in den Hook. Siehe Abschnitt 8 und 9.

### Iteration 3 — Whitespace und Einfärben ✅

- Leerzeichen als Punkt, Tabulator als Pfeil über die volle Tabstoppbreite, Zeilenende als `LF`/`CRLF`. Abschaltbar über `toggles.showWhitespace` und `toggles.showLineEndings`.
- Zustandsarray je Zeile, `fuiEditorStyleDef`-Tabelle, Lexer-Callback, `styledUpToLine`, Konvergenzabbruch, `lexConvergenceFloor`.
- Dekorationsschicht: Zeilenhintergrund und Randspaltenmarker (`fuiEditorLineDecoration`), plus explizite Bereichsliste für Teilzeilen (`fuiEditorRangeDecoration`). Beide Arrays gehören dem Aufrufer, sind sortiert und werden per Binärsuche aufs sichtbare Fenster eingegrenzt.
- Zeichnen in Style-Läufen, an Tabulatoren und an Leerzeichenläufen geschnitten, als Präfix gemessen.
- Demo: kleiner C-Lexer (Kommentare über Zeilengrenzen, Strings, Zahlen, Schlüsselwörter, Typen, Präprozessor) und eine Ansicht der geänderten Zeilen gegen die Datei, aus der geladen wurde.

**Abnahme:** *Erfüllt.* Der C-Lexer färbt `final_ui.h`, und der Blockkommentar des Dateikopfs trägt korrekt über achtzig Zeilen. Statt „unter einem Frame" wird die härtere Zahl geprüft: `[incremental colouring]` **zählt die Lexer-Aufrufe**. Ein volles Dokument von 2000 Zeilen kostet 2000 Aufrufe, ein zweites Nachfragen null, und eine Änderung in Zeile 3 danach **höchstens zwei** — und die Gegenprobe, dass eine Änderung, die den Zustand wirklich verändert (ein geöffneter Blockkommentar), eben *nicht* früh abbricht.

Nachgeprüft wurde außerdem von Hand, was keine kopflose Prüfung zeigt: beide Scrollbalken sind sichtbar, ein Dokument mit gemischten Zeilenenden zeigt je Zeile `LF` beziehungsweise `CRLF` und in der Statusleiste `Mixed`, und die Marken stehen bündig am letzten Zeichen.

**Was noch aussteht:** Eine echte Diff-Ansicht braucht zwei Fassungen zum Vergleichen. Das Demo vergleicht positionsweise gegen die geladene Datei, was genau das ist, was es sagt — welche Zeilen sich *verschoben* haben, findet ein Diff heraus, und das ist Sache des Aufrufers. Der Editor nimmt nur die Antwort entgegen.

### Iteration 4 — Bearbeitungsmodus ✅

- Tippen, Enter, Backspace und Entf **mit Berücksichtigung der Auswahl**. Alles, was ein Frame an Codepoints geliefert hat, wird zu **einem** Insert gesammelt.
- Einfg schaltet Einfügen/Überschreiben um, sichtbar am Cursor — Strich gegen Kasten, und der Kasten als **Umriss**, damit das Zeichen darunter lesbar bleibt. Überschreiben hält am Zeilenende an.
- Ctrl+V, Shift+Einfg und die **mittlere Maustaste**, die dort einfügt, wo geklickt wurde. Ctrl+X und Shift+Entf schneiden die Auswahl aus, sonst die ganze Zeile samt Ende — und verweigern, wenn die Zwischenablage den Text nicht genommen hat. Ctrl+D löscht die Zeile.
- Geändert-Flag, `onChange` mit `fuiEditorChange`, `isReadOnly` sperrt jeden schreibenden Zweig — `fuiEditorInsert`/`fuiEditorErase` bleiben offen, sonst wäre eine Nur-Lese-Ansicht eine Ansicht auf nichts.
- Cursor, Auswahlanker und Drag-Anker werden von der Änderung selbst nachgeführt, in `fuiEditorInsert` und `fuiEditorErase`.
- `fuiIsMouseButtonDown`, `fuiMouseButtonWentDown` und `fuiConsumeKey` in `final_ui.h` (v0.9.7, Version unverändert).

**Abnahme:** *Erfüllt.* Im Demo `final_ui.h` geladen, eine Zeile beschrieben, eine gelöscht, „Save & verify" — 658 309 Bytes geschrieben und byteweise identisch zurückgelesen; gegengeprüft gegen eine unabhängig aus der Quelldatei gerechnete Erwartung. Kopflos automatisiert steht dafür `[edits against a plain buffer]`: 400 gemischte Änderungen an pseudozufälligen Stellen, parallel auf einem flachen `memmove`-Puffer mitgeführt und am Ende byteweise verglichen, plus der Zeilenindex gegen einen rohen Scan. Dazu `[typing]`, `[enter, backspace and delete]`, `[overwrite mode]`, `[cut, paste and the line commands]`, `[middle button paste]`, `[read only]`, `[an edit moves the caret]` und `[the change callback]`.

**Was danach noch ausstand:** **Tab rückte nicht ein** — die Taste gehörte der Fokuskette — und es gab **kein Undo**. Beides in Iteration 5 nachgeholt.

### Iteration 5 — Undo/Redo und Blockoperationen ✅

- Undo-Stapel mit Zusammenfassen: eine getippte Wortfolge ist **ein** Schritt, und ein Löschlauf genauso. Ctrl+Z, Ctrl+Y, Ctrl+Shift+Z, Cursor und Auswahl inklusive. Aufgezeichnet wird in `fuiEditorInsert` und `fuiEditorErase` selbst — eine Löschung **bevor** ihre Bytes weg sind, eine Einfügung erst, wenn beide Reservierungen durch sind und nichts mehr fehlschlagen kann.
- `fuiEditorBeginUndoGroup` / `fuiEditorEndUndoGroup` machen aus beliebig vielen Änderungen einen Schritt. Alles, was mehr als einmal schreibt, benutzt sie: Tippen über eine Auswahl, Zeile löschen, Einrücken, Duplizieren, Zeilen verschieben — und in Iteration 6 „Alle ersetzen".
- Ein **Budget** (`limits.undoMemoryBytes`, vier Megabyte per Default) wirft am ältesten Ende ganze Schritte weg, nie halbe und nie einen, den Redo noch braucht.
- `fuiEditorClearModified` merkt sich die **Stelle** in der Historie, an der gespeichert wurde; bis dorthin zurück ist wieder „ungeändert".
- Tab/Shift+Tab rücken eine Markierung ein und aus, Ctrl+Shift+D dupliziert, Alt+Hoch/Runter verschiebt, Enter übernimmt die Einrückung. Die Tab-Taste gehört dem Editor nur, wenn er die Tastatur **schon vorher** hatte.
- **Keine Zusätze in `final_ui.h`.**

**Abnahme:** *Erfüllt.* `[two hundred steps, back and forward again]` fährt 200 gemischte Schritte über `final_ui.h`, nimmt sie einzeln zurück und vergleicht byteweise mit der geladenen Datei; danach 200-mal vorwärts und byteweise mit dem bearbeiteten Stand. Die Schrittzahl wird in beide Richtungen mitgezählt und muss stimmen, das Geändert-Flag am Ende der Rückwärtsstrecke aus sein, und der Zeilenindex am Ende gegen einen rohen Scan halten. Dazu `[undo and redo]`, `[a run of typing is one step]`, `[tab against the focus chain]`, `[blocks of lines]`, `[the keys they are on]` und `[the undo budget]`.

### Iteration 6 — Suchen und Ersetzen ✅

- Overlay im Editor (nicht modal), Ctrl+F suchen, Ctrl+H **und** Ctrl+R mit ausklappbarer Ersetzen-Zeile, Escape schließt und gibt die Tastatur zurück. Die Felder sind `fuiTextInput`. Es **schwebt** über dem Text, und alles, was den Cursor sichtbar macht, kennt seine Höhe.
- F3 / Shift+F3 auch bei geschlossener Leiste, alle Treffer markiert, Groß-/Kleinschreibung, ganzes Wort, „n von m", „Alle ersetzen" als **ein** Undo-Schritt.
- Gehe-zu-Zeile über Ctrl+G, und ein Sprung landet in der **Mitte** der Ansicht statt an ihrer Kante.
- Drei Schalter davor: `toggles.canFind`, `toggles.canReplace`, `toggles.canGoToLine`. Nur-Lesen nimmt die Ersetzen-Zeile zusätzlich ganz weg. Die programmatische API bleibt dahinter offen.

**Abnahme:** *Erfüllt.* `[finding in a real file]` sucht in `final_ui.h` nach `fui__` und hält die Zahl gegen einen flachen Lauf über dieselben Bytes — 888 mit und 1161 ohne Beachtung der Groß-/Kleinschreibung, genau die Zahlen, die `grep -o | wc -l` und `grep -oi | wc -l` liefern. Zusätzlich mit der Lücke mitten in der Datei statt an ihrem Ende, und mit zwei Leerzeichen als Nadel, die sich selbst überlappt. `[replace all over a file]` ersetzt alle 888 auf einen Schlag, prüft Länge und Trefferzahl, nimmt sie mit **einem** `fuiEditorUndo` zurück und vergleicht byteweise mit der geladenen Datei. Dazu `[finding]`, `[find next and previous]`, `[replacing]`, `[the find bar's keys]`, `[the find bar under the mouse]`, `[the view follows a jump]` und `[go to line]`.

### Iteration 7 — Encodings und Zeilenumbruch

**Der Encoding-Teil ist umgesetzt; der Zeilenumbruch steht noch aus.**

- Backends: UTF-16LE, UTF-16BE, UTF-7, Latin-1 und CP1252 stehen neben UTF-8 und ASCII. ✅
- BOM-Erkennung und Zeilenende-Normalisierung. ✅
- `fuiEditorSaveToMemory` über die Vtable, mit dem gemerkten Ursprungs-Encoding. ✅
- Dazu ungeplant: `fuiEditorDetectEncoding`, `fuiEditorGetEncoding`/`SetEncoding`, `fuiEditorHasByteOrderMark`/`SetByteOrderMark`.
- Optionaler Zeilenumbruch: zweiter Index Bildschirmzeile ↔ Dokumentzeile, bei Breitenänderung neu gebaut. Zeilennummer nur an der ersten Bildschirmzeile einer Dokumentzeile. — **offen**

**Abnahme des Encoding-Teils:** *Erfüllt.* `[a file through utf-16 and back]` schreibt `final_ui.h` mit dem eigenen Writer nach UTF-16LE mit BOM, lädt es, hält das Dokument byteweise gegen die Datei, speichert und vergleicht byteweise mit dem, was hineinging — dann eine Zeile eingefügt, und **nur dort** darf sich etwas unterscheiden, vorne und hinten je byteweise geprüft. Weil `final_ui.h` reines ASCII ist und damit weder ein Mehrbyte-Zeichen noch ein Surrogatpaar enthält, steht daneben `[every encoding, out and back]`: ein gebauter Text aus ASCII, Latin-Buchstaben, Dreibyte-Zeichen und einem Zeichen aus der zweiten Ebene, 500-mal wiederholt, **durch jedes Encoding hinaus und wieder herein** — und in beide Richtungen verglichen, denn ein Konverter, der in beide Richtungen denselben Fehler macht, ginge über einen reinen Rundlauf glatt durch. Dazu `[utf-16]`, `[utf-7]`, `[latin-1 and windows-1252]`, `[detecting an encoding]` und `[line endings through a load and a save]`.

**Abnahme des Umbruchs:** Umbruch an/aus, ohne dass Cursor oder Auswahl springen. — offen

**Was beim Encoding-Teil anders lief als geplant:**

- **Der BOM wird als Codepoint abgeschnitten, nicht als Bytes.** Geplant war `getBomLength` je Encoding, also ein Byte-Muster je Encoding vor der Konvertierung. Jede Marke, die es gibt, ist aber **dieselbe** — das Zero Width No-Break Space, nur im Alphabet des jeweiligen Encodings buchstabiert. Nach der Konvertierung gibt es also genau eine Sache zu suchen und genau eine Stelle, an der gesucht wird. Bei UTF-7 ist das nicht nur bequemer, sondern der einzige Weg: `+/v8-` ist nur *eine* der erlaubten Schreibweisen, und ein `+/v8`, das direkt in weiteren Base64 übergeht, hat gar keine feste Bytelänge. In der Vtable steht deshalb `getBomBytes` statt `getBomLength` — nur zum Schreiben, denn zum Lesen braucht es sie nicht mehr.
- **Nur der einzelne Carriage Return wird normalisiert, CRLF nicht.** Das Dokumentmodell versteht CR+LF (Abschnitt 4.3), und die Zeilenende-Anzeige je Zeile aus Iteration 3 lebt davon. Alles auf LF zu normalisieren, wie es Scintilla tut, hätte diese Anzeige zu einer Anzeige gemacht, die immer dasselbe sagt. Normalisiert wird deshalb genau der Fall, den das Modell **nicht** darstellen kann: ein Carriage Return ohne Line Feed dahinter.
- **`fuiEditorEol_Mixed` schreibt, was dasteht — jede andere Einstellung vereinheitlicht.** Das war der Punkt, an dem „Mixed" von einer Auskunft zu einer Bedeutung wurde: Die Statusleiste sagt seit Iteration 1 „Mixed", und der Kommentar daneben sagt, das sei die einzige Art, dem Aufrufer mitzuteilen, dass Speichern die Zeilen vereinheitlicht. Genau das tut es jetzt, und `fuiEditorSetEol` ist damit nebenbei „Zeilenenden umwandeln".
- **Speichern kopiert das Dokument nur, wenn es muss.** Der Zähllauf über die Zeilenenden meldet mit, ob überhaupt etwas anders geschrieben würde; ist es das nicht — der Normalfall — geht der Text direkt aus dem Puffer in den Konverter, ohne 650 KB Zwischenkopie.
- **Zwei Bitmasken sind wieder ausgebaut worden.** Der UTF-7-Coder hielt seinen Bit-Puffer nach jeder Entnahme kanonisch. Keine Prüfung konnte das rot bekommen — und zwar zu Recht: geschuldet sind immer die **unteren** Bits, und was oben aus einem `uint32` herausläuft, trägt nichts weg. Statt der Masken steht jetzt ein Kommentar an der Entnahme, der genau das sagt.
- **Die schärfste Prüfung war wieder eine zweite Quelle.** `final_ui.h` ist reines ASCII — durch UTF-16 gedreht ist daran kein einziges Surrogatpaar und kein einziges Mehrbyte-Zeichen beteiligt. Der Rundlauf über einen eigens gebauten Text hat deshalb Fälle gefunden, die die echte Datei niemals berührt.
- **Ein UTF-7-Lauf muss mit einem Strich geschlossen werden, wenn ein Buchstabe folgt.** Der erste Rundlauf-Text hatte hinter jedem Sonderzeichen ein Leerzeichen — und ein Leerzeichen beendet einen Base64-Lauf ohnehin. Erst mit einem Buchstaben direkt dahinter (`äx`) wird die Prüfung scharf: ohne den Strich zöge der Lauf den Buchstaben mit hinein. Absichtlich kaputtgemacht: vorher 2 Fehler, nachher 6.
- **Und wieder wurde jede neue Absicherung absichtlich kaputtgemacht und die Suite dabei rot gesehen** — der fehlende Strich am Laufende (6 Fehler), das Plus als direktes Zeichen (2), die vertauschten Surrogathälften (3), die ignorierte Bytereihenfolge (4), der nicht abgeschnittene BOM (14), der nicht geschriebene BOM (11), die fehlende CR-Normalisierung (5), das Zeilenende nach statt vor der Normalisierung erkannt (2), der übergangene CP1252-Block (4), die Surrogat-Paarung in UTF-16 (6) und das „unverändert", das immer ja sagt (3).

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

Das ist der Modus, gegen den entwickelt wird, denn ein Gap-Buffer ist genau die Art Sache, die auf dem Bildschirm richtig aussieht und über der Lücke falsch ist — und ein Zeilenindex genau die Art Sache, die irgendwo in der Mitte einer Datei um eins danebenliegt, zu der niemand gescrollt hat. Neunundfünfzig Gruppen: leeres Dokument, Zeilenindex, Einfügen, Löschen, Lückenbewegung, Wachstum, Zeilenenden, UTF-8, Encodings, Ansichtshelfer, zusammenhängende Läufe, Cursorzeile, Dokument gegen Datei, Widget-Layout, leeres Widget, Scrollbalken nicht übermalt, Zeilengeometrie, Wörter, Auswahl, Tastatur, Rad gegen Cursor, Kopieren gegen Datei, Zustände folgen ihren Zeilen, inkrementelles Einfärben, Dekorations-Nachschlag, Zeilenenden je Zeile, Tippen, Enter/Backspace/Entf, Überschreiben, Ausschneiden/Einfügen/Zeilen, mittlere Maustaste, Nur-Lesen, Cursor folgt der Änderung, Änderungs-Callback, Änderungen gegen einen flachen Puffer, Undo/Redo, Tipplauf als ein Schritt, Tab gegen die Fokuskette, Zeilenblöcke, die Tasten dazu, das Undo-Budget, 200 Schritte hin und zurück, Suchen, Suchen in einer echten Datei, Vor und Zurück, Ersetzen, Alles ersetzen über eine Datei, die Tasten der Suchleiste, die Suchleiste unter der Maus, die Ansicht nach einem Sprung, das Abschalten der Leiste, Gehe-zu-Zeile, UTF-16, UTF-7, Latin-1 und CP1252, das Erkennen eines Encodings, Zeilenenden über Laden und Speichern, jedes Encoding hinaus und wieder herein, und eine echte Datei durch UTF-16 und zurück.

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
- **FPLs Zwischenablage.** `fplSetClipboardText` kopiert unter X11 in einen festen Puffer von 2048 Bytes; `fplGetClipboardText` liest aus demselben. Es schneidet dabei nicht ab, sondern kopiert bei Überlänge **gar nichts** (`fplCopyString` liefert null und schreibt nicht), übernimmt den Selection-Owner aber trotzdem — die Zwischenablage ist danach leer, das Vorherige ist weg, und der Rückgabewert sagt „erfolgreich". Nachgeprüft: eine große Auswahl in Kate einzufügen liefert nichts, kurze funktionieren. Für einen Editor ist das zu klein — eine markierte Datei sind schnell hunderte Kilobyte. Zu lösen ist das in `final_platform_layer.h` mit dynamischem Speicher für den Zwischenablagepuffer, nicht hier. Bewusst ein eigenes Thema: es ist eine Änderung an einer plattformübergreifenden Datei, die nichts mit dem Editor zu tun hat, und beim Setzen großer Selektionen kommt über X11 obendrein das INCR-Protokoll ins Spiel.
- **X11 PRIMARY selection.** Die mittlere Maustaste fügt aus der normalen Zwischenablage ein, weil FPL nur `CLIPBOARD` kennt und nicht `PRIMARY`. Unter Linux ist das nicht ganz die gewohnte Geste.

---

## 9. Risiken

| Risiko | Gegenmaßnahme |
|---|---|
| ~~FiraCode bläht `final_fonts.h` auf~~ ✅ | Beide Schnitte sind drin. `final_fonts.h` ist von 2,39 MB auf 3,22 MB gewachsen, also 830 KB für Bitstream Vera Sans Mono (49 KB Fontdaten) und FiraCode (290 KB). Das war tragbar, die befürchteten ~2 MB allein für FiraCode sind es nicht geworden |
| **Bestätigt, und schlimmer als eine Kürzung:** `fplSetClipboardText` kopiert unter X11 über `fplCopyString` in `clipboardOut[FPL_MAX_BUFFER_LENGTH]`. `fplCopyString` schreibt bei zu kleinem Ziel **gar nichts** und liefert null zurück — der Selection-Owner wird trotzdem übernommen und liefert dann null Bytes aus. Die Zwischenablage ist danach also **leer**, samt dem, was vorher darin stand, und der Aufruf meldet Erfolg, weil die Übernahme geklappt hat. Unter Windows gibt es die Grenze nicht | Der Editor macht seinen Teil vollständig: `fuiEditorCopySelection` gibt die ganze Auswahl heraus, und Ctrl+C alloziert genau dafür. Die Größengrenze gehört in den Plattform-Hook, und genau dort setzt das Demo sie: `DemoSetClipboardText` verweigert, was nicht hineinpasst, und lässt die vorhandene Zwischenablage in Ruhe. **Eigenes Thema: FPL braucht dort dynamisches Speichermanagement**, siehe Abschnitt 8 |
| ~~Nachfärben nach einer Änderung weit über dem Sichtfenster~~ ✅ | Zustandskonvergenz-Abbruch, in Iteration 3 mit genau diesem Fall abgenommen — und über die **Anzahl der Lexer-Aufrufe** geprüft, nicht über das Ergebnis. Genau das hat den Fehler in der Schwelle gefunden |
| Viele Style-Läufe je Zeile treiben die Draw-Commands hoch | `fuiSetDrawBatching`, Läufe gleicher Farbe zusammenfassen, in Iteration 8 messen |
| Rückwärtsscrollen mit Umbruch ist beim alten Textfeld O(Dokument) (`final_ui.h:9258`) | Der zweite Index wird einmal je Breite gebaut und gehalten, nicht je Frame |
| Die breiteste Zeile ist die breiteste *gesehene* — der waagerechte Bereich wächst also beim Durchscrollen | Bewusst so, und dokumentiert. Scintilla verhält sich genauso. Eine Änderung setzt ihn zurück |
| ~~Ein Undo-Stapel über einem 650-KB-Dokument wächst mit jedem Tastendruck~~ ✅ | `limits.undoMemoryBytes`, per Default vier Megabyte. Darüber fallen am ältesten Ende **ganze Schritte** weg. Geprüft mit einem Budget von 512 Byte und Schritten aus je zwei Datensätzen — und einmal über `undo.arenaLength` selbst, weil ein zusammengefasster Tipplauf am Schrittzähler nicht zu sehen ist |
| ~~Der Cursor der Suchfelder und der des Editors stören sich~~ ✅ | Der Editor hält seinen eigenen, `fuiTextInput` seinen auf dem Kontext, und sie kommen sich nicht ins Gehege: solange ein Feld die Tastatur hat, läuft `fuiEditor__HandleKeyboard` gar nicht erst. Geprüft in `[the find bar's keys]` — Pfeiltaste, Ende und Backspace im Suchfeld lassen Cursor **und** Dokument des Editors unangetastet |
| Dokumente über 2 GB | `int32_t` durchgängig. Bewusst: die Grenze ist dokumentiert und für einen Texteditor keine |
