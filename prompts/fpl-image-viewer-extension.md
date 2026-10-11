Ich möchte den FPL_ImageViewer als eigenständige App umbauen, dazu fehlt allerdings noch einiges damit es ein brauchbarer und schneller image-viewer wird.
Zum einen soll es final_ui.h nutzen, damit man auch mit der Maus darin arbeiten kann - auch wenn es Primär über Tasten gesteuert wird.
Wichtiger ist allerdings erstmal die technische Basis, hier fehlt noch einiges.

Erstelle mit einen Markdown-Plan wie üblich für folgende Anpassungen in "plans/".

# Technik

## Upscale / Downscale
Es gibt weder ein ordentliches upscale noch ein downscale, wir haben zwar diverse Shader/Filter aber diese habe ich nur eingebaut um das upscale/downscale zu kompensieren.

### Downscale
Wir brauchen daher zu erst mal ein richtiges downscale, also große Bilder kleiner Rechnen damit diese auch gut auf dem jeweiligen Fenster oder im Vollbild ordentlich dargestellt werden.
Wenn man das über Shader/Filter abbilden kann, dann könnte man auch anbieten mehrere Filter anzubieten und entsprechend diese umschalten zu können.
Evtl. muss man ein 2-4 LOD-Bilder erzeugen, damit das richtig klappt und gut aussieht - das muss dann als zusätzlichen Schritt in den Bild-Lade Threads passieren - hier muss auch die Fortschrittsanzeige angepasst werden, damit es stimmt.

###Upscale
Für upscale könnte man die jetztigen Shader/Filter nutzen und den besten dafür nehmen - der aber bei Bedarf umgeschaltet werden kann.

## OpenGL 3.3 Backend
Der ImageViewer nutzt aktuell OpenGL 3.3, es gibt allerdings nur ein fui_backend_gl3.h.
Wir brauchen daher ab dem Punkt wo wir final_ui.h benutzen, ein neues "fui_backend_gl3.h" mit voller OpenGL 3.3 Unterstützung.

## OpenGL 4.x Backend
Ist OpenGL 3.3 nicht ausreichend, können wir später auch ein OpenGL 4.x Backend bauen, um z.b. Compute-Shader zu nutzen.

# Steuerung

## Pan & Zoom
Es gibt aktuell weder die Möglichkeit in einem Bild zu zoomen noch sich darin zu bewegen.
Bitte entsprechend Industrie-Standard Pan & Zoom einbauen, welches aber nicht kollidieren darf mit den Next-Image/Prev-Image Aktionen.

Auch während man gezoomed hat, soll man Bilder wechseln können. Aber logischerweise bei einem wechsel verliert man den Zoom/Pan erstmal.
Das soll aber einstellbar sein, so dass es auch für alle Bilder beibehalten werden kann -> wichtig um Bilder mit gleicher größe zu vergleichen oder gleiche Bilder mit unterschiedlichen größen.

# UI
final_ui soll fest integriert werden, aber bis auf Text-Rendering noch nicht benutzt werden.
Oben links soll der Dateiname (ohne Pfad oder mit relativem Pfad -> einstellbar) angezeigt werden und in der gleichen Zeile rechts ausgerichtet die originale Bildgröße und Bits pro Pixel angezeigt werden.

# Test Bilder
Erstelle dir bitte auch unterschiedliche Bilder selbst um die entsprechenden Down/Up Scale Filter richtig zu testen und zu validieren.

# Reale Bilder
Im Ordner "/home/final/Bilder/202308" liegen zahlreiche Fotos die man zum testen verwenden kann.
Auch im Ordner "/home/final/Bilder/Bildschirmfotos/" liegen Bilder in unterschiedlichen Größen.

# Optimierungen
Du darfst SIMD verwenden bis AVX-512! Aber es muss auch mit älteren SSE Versionen (bis SSE 2) funktionieren.
Ist kein SSE 2 vorhanden, dann bricht es auf Skalar runter und ist dann halt schlichtweg langsam.
Nutze FPL für CPU feature Erkennung! Baue eine Architekur um später auch ARM zu unterstützen.

