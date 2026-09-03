Erstelle bitte einen Plan für folgendes Feature in final_ui.h.

# Editor-Widget

Wir haben aktuell nur ein einfaches TextEditor Widget welches größeren Text darstellen kann.
Dieser hat allerdings kaum Editierfunktionen und unterstützt keinerlei Farben.
Zum Code anzeigen oder Diff's die Farben, bzw. Syntax Highlighting benötigen ist das nicht benutzbar.

Deshalb soll jetzt ein neues richtiges EditorWidget erstellt werden, mit allen Features die man in einem reinen Text -und Code-Editor erwartet.
Das ganze soll iterativ erstellt werden, die Reihenfolge muss noch abgestimmt werden!

Kerniterationen sind: Read-Only und dann Bearbeitungsmodus.

Man darf sich hier gern an Scintilla oder so orientieren.

## Kernfunktionen

- Support für mehrere Encodings (UTF-7, UTF-8, UTF-16, ASCII, etc.) -> Hier brauchen wir eine Abstraction für ein Encoding Backend
- Zeilennummern mit einem sichtbaren Rand und Padding (Nummern sind rechts ausgerichtet, nicht gepadded)
- Highlight von aktueller Zeile mit Hintergrundfarbe, inkl. der Zeilennummer
- Cursor-Informationen in einer Statusbar unter dem Editor selbst
- Vertical und horizontal scrollbar (abschaltbar)
- ReadOnly support (nur anzeigen oder bearbeiten)
- Rückwärts löschen mit Berücksichtigen von Selected-Text (shortcut Backspace)
- Vorwärts löschen mit Berücksichtigen von Selected-Text (shortcut Entf)
- Wechseln vom Modus über die Insert-Taste (einfügen oder überschreiben, erkennbar am Cursor - strich oder kasten)
- Neue leere Zeilen einfügen (shortcut Enter)
- Löschen von Zeilen (shortcut Ctrl+D)
- Ausschneiden von Zeilen (shortcut Ctrl+X)
- Text Selektieren mit der Maus, über mehrere Zeilen hinweg
- Copy/Paste To Clipboard (shortcuts wie in Linux und Windows, Ctrl+C + Ctrl+V und zusätzlich mittlere Maustaste für einfügen)
- Whitespaces anzeigen (leerzeichen als '.', tabs als Pfeil mit z.b. 4 charaktern Länge, zeilenumbrüche wie CR, LF, CRLF)

## Zusätzliche Kernfunktionen

HIER einfügen

## Einfärben

- Custom Foreground/Background Color-Support für Textbereiche und ganze Linien
- Hier brauchen wir einen oder mehrere function callbacks, damit der User die Farben für bestimmte Textbereiche ändern oder zurücksetzen kann
- Muss intelligent gebaut sein damit nur sichtbare Bereiche eingefärbt werden müssen - allerdings muss das auch zusammenpassen mit parsen von Code oder einem speziellen Format

### Anwendungsgebiete

- Diff Einfärbung (ganze Zeilen, einzelne Bereiche, einzelne Buchstaben, gemischt)
- Syntax-Highlighting für Code (C / C++, C#, asm, HTML, CSS, JS, Delphi, Python, usw.).

## Find & Replace

- Find & Replace Dialog (replace ein/ausklappbar) zum Suchen und markieren eines Textbereichs
- Standard Shortcuts (Ctrl+F -> nur Suchen, Ctrl+R -> suchen und ersetzen, )

## Einstellbarkeit über großes Config-Struct

Alles muss Einstellbar sein über ein großes Config-Struct, wie Abstände, Farben, Schriftart, Toggles, Enums, Callbacks, etc.
Ebenfalls sollen Shortcuts änderbar sein, außer jetzt Enter, Backspace, Entf, Clipboard copy/paste - die sind ja Standard.
Der User ist nicht gezwungen ein Config-Struct anzugeben, NULL soll erlaubt sein und dann gibts halt die Standard-Einstellungen und Standard-Farben.

