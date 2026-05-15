# FPL 1.0.0 Presentation — Slide Deck Guideline

This folder now contains three deliverables:

| File                              | Purpose                                                |
|-----------------------------------|--------------------------------------------------------|
| `FPL_1.0.0_Presentation.md`       | Narrative script with timestamps + B-roll suggestions  |
| `FPL_1.0.0_Presentation.pptx`     | PowerPoint deck (20 slides, 16:9, dark theme)          |
| `FPL_1.0.0_Presentation.odp`      | OpenOffice / LibreOffice Impress version of the same   |

Both decks were generated programmatically from the same source. Open either one in PowerPoint, Keynote, Google Slides, or Impress.

## Slide Map (20 slides, 16:9)

| # | Title                                              | Type             |
|---|----------------------------------------------------|------------------|
|  1| Title — Final Platform Layer 1.0.0                 | Title slide      |
|  2| What is FPL?                                       | Bullets          |
|  3| The Problem It Solves                              | Comparison table |
|  4| Core Feature Set                                   | 4-column grid    |
|  5| Hello World                                        | Code             |
|  6| Video Backends: Context vs. API                    | Comparison table |
|  7| Software Backend                                   | Code             |
|  8| OpenGL Backend (needs loader)                      | Code             |
|  9| Vulkan Backend (needs loader)                      | Code             |
| 10| Advantages                                         | Bullets (10)     |
| 11| Disadvantages & Honest Trade-offs                  | Bullets          |
| 12| Supported Matrix                                   | 4-column grid    |
| 13| Use Cases — Games & Simulation                     | Demo bullets     |
| 14| Use Cases — Multimedia & Examples                  | Demo bullets     |
| 15| What's New in 1.0.0                                | 2-column changelog |
| 16| The Wider Family of Libraries                      | Table            |
| 17| Getting Started                                    | Steps + code     |
| 18| Who Is It For?                                     | Bullets          |
| 19| Roadmap / Future Work                              | Bullets          |
| 20| CTA / Outro                                        | Outro            |

Every slide has the narration text from the markdown attached as **speaker notes** (visible in Notes view or via `View → Notes Page` in PowerPoint / `Insert → Header & Footer → Notes` in Impress).

## Design Choices

- **16:9 widescreen** (13.333 × 7.5 in) — matches YouTube 1080p and 1440p.
- **Dark theme** with a single accent blue (`#4FC3F7`) and amber highlight (`#FFC107`) — re-theme via PowerPoint **Design → Variants** or Impress **Slide → Slide Properties → Background**.
- **Consolas** monospace for all code blocks; system sans for body text.
- **Speaker notes** carry the full narration from `FPL_1.0.0_Presentation.md`.
- **Left accent bar** on every content slide for visual rhythm.
- **Footer** with project URL + page count on slides 2–19.

## Customizing

### Replace the theme
1. Open the PPTX/ODP.
2. PowerPoint: **Design** tab → pick a theme.
   Impress: **Slide → Slide Properties → Background**.
3. Backgrounds, accent colors, and fonts will apply to all slides because every shape uses simple solid fills, not theme placeholders.

### Add screenshots
The repository already has high-quality screenshots in `screenshots/` — drop them onto slides 13, 14, and 16:
- `FPL_AudioPlayer.png`, `FPL_FFMpeg.png`, `FPL_ImageViewer.png`
- `FPL_Emulator.png`, `FPL_NBodySimulation.png`, `FPL_Raytracer.png`
- `FPL_Crackout.png`, `FPL_Towadev.png`
- `FPL_OpenGL.png`, `FPL_Software.png`, `FPL_ImGui.png`, `FTT_TileTracingDemo.png`

### Add a logo
The FPL logo lives at `assets/final_game_tech.jpg` (repo) or the website's `res/fpl_logo_256x256.png` — drop into the title slide and outro.

## Regenerating from Source

The deck is built from `/tmp/build_fpl_pptx.py` using **python-pptx**. To rebuild after edits:

```bash
python3 -m venv /tmp/pptxvenv
/tmp/pptxvenv/bin/pip install python-pptx
/tmp/pptxvenv/bin/python3 /tmp/build_fpl_pptx.py
soffice --headless --convert-to odp \
        /home/final/_projects/fpl/final_game_tech/progress/FPL_1.0.0_Presentation.pptx \
        --outdir /home/final/_projects/fpl/final_game_tech/progress/
```

Slide content lives inline in the Python script — every `slide(builder, notes=...)` call corresponds to one slide and one speaker-note block.

## Suggested Video Cadence

If you record narration from the speaker notes, total runtime is ~11 minutes:

| Slides | Duration |
|--------|----------|
| 1      | 20 s   (hook)            |
| 2–4    | 40 s each                |
| 5      | 30 s                     |
| 6–9    | 30–60 s each             |
| 10–11  | 50 s each                |
| 12     | 40 s                     |
| 13–14  | 60 s each (demo carousel)|
| 15     | 60 s                     |
| 16–17  | 30 s each                |
| 18–19  | 30 s each                |
| 20     | 30 s   (outro)           |

For a shorter cut (5 min), keep: 1 → 2 → 3 → 5 → 6 → 10 → 11 → 13/14 montage → 20.
For a long-form deep-dive (20 min+), add live coding for the OpenGL and Vulkan slides.

## Notes on the ODP Conversion

LibreOffice produced the ODP via the `impress8` filter. Most layouts survive perfectly, but a few quirks to expect:

- Font fallback: if Consolas is not installed on the rendering machine, Impress substitutes a similar mono font — readability is preserved.
- Code-block background rectangles render as filled shapes, not text-box backgrounds — they are fully editable in Impress.
- Speaker notes round-trip cleanly.

If you prefer to start from the ODP rather than the PPTX, open it in Impress and use **File → Export As → PowerPoint** to round-trip back. There is no information loss in either direction for this deck.
