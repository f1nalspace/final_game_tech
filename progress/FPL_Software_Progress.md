# FPL_Software — Progress Tracker

Tracks implementation of [FPL_Software_Plan.md](./FPL_Software_Plan.md).

Status legend: ☐ todo · ◐ in progress · ☑ done · ✗ blocked

Last updated: 2026-06-02

---

## Phase 1 — Normal Pixel Rendering (top-down)

| # | Task | Status | Notes |
|---|------|--------|-------|
| 1.1 | Filled quad with clipping (`BackbufferFillQuad`) | ☑ | Inclusive both edges. |
| 1.2 | H/V line, 1px, clipping (`BackbufferDrawHLine`/`VLine`) | ☑ | Span core `BackbufferHSpan`/`VSpan`. |
| 1.3 | Filled circle, Bresenham, clipping (`BackbufferFillCircle`) | ☑ | Midpoint + clipped spans. |

## Phase 2 — Orthographic Pixel Pipeline (OpenGL coordinates)

| # | Task | Status | Notes |
|---|------|--------|-------|
| 2.1 | Pipeline struct + ortho/model setup + project | ☑ | OpenGL y-up; custom `GfxTransform` (see bug note). |
| 2.2 | Quad (float) filled/stroked + thickness + real clip | ☑ | `PipelineDrawQuad`, scanline convex fill. |
| 2.3 | Line (float) + thickness + real clip | ☑ | `PipelineDrawLine`, thick segment quad. |
| 2.4 | Circle (float) filled/stroked + thickness + real clip | ☑ | `PipelineDrawCircle`, screen-space spans. |
| 2.5 | `Texture2D` + load via stb_image | ☑ | `TextureLoadFromFile` (v-flip for GL), FPL memory. |
| 2.6 | Textured quad (float) + 4 UVs + real clip | ☑ | `PipelineDrawTexturedQuad`, 2 tris, affine UV. |
| 2.7 | Sub-pixel rendering + bilinear filtering | ☑ | Pixel-center sampling + `GfxSampleBilinear`. |

## Phase 3 — Scenario System

| # | Task | Status | Notes |
|---|------|--------|-------|
| 3.1 | Scenario scaffolding (struct/union, F1 switch) | ☑ | Tagged union + dispatch; F2 toggles filter. |
| 3.2 | Scenario: **Planes** (current impl) | ☑ | Phase 1 baseline / regression. |
| 3.3 | Scenario: **Pixel Primitives** | ☑ | Phase 1.1–1.3 clip showcase. |
| 3.4 | Scenario: **Ortho Pipeline** | ☑ | Phase 2.1–2.2, custom world units. |
| 3.5 | Scenario: **Vector Shapes** | ☑ | Phase 2.2–2.4 stroke/fill+thickness. |
| 3.6 | Scenario: **Textured Quad** | ☑ | Phase 2.5–2.6 UV control. |
| 3.7 | Scenario: **Sub-Pixel Motion** | ☑ | Phase 2.7 nearest vs bilinear. |

## Build / Verification

| Item | Status | Notes |
|------|--------|-------|
| Compiles (gcc -std=c99, -Wall) | ☑ | No warnings from demo/final_graphics; only pre-existing FPL lib warnings. |
| Runs (X11) | ☑ | 4s smoke run, no crash. |
| Offscreen rasterizer test | ☑ | All primitives + clipping at every edge + textured nearest/bilinear, no crash. |

---

## Notes / Deviations

- **`V4fMultM4f` bug (final_math.h):** transposed convention vs `M4fOrthoRH`/
  `M4fTranslation` (translation stored in the bottom row). `V2iProject`/
  `V2fUnproject` inherit it. The pipeline uses its own `GfxTransform` instead.
  Not patched in final_math.h to avoid breaking callers — flagged for the author.
- **OpenGL coordinates:** pipeline is GL-style (NDC y-up → visual up, glOrtho,
  texture v-up via `stbi_set_flip_vertically_on_load`). Phase-1 pixel layer
  stays top-down per spec.
- **No libc:** uses `final_math.h` scalars (added `F32Floor`/`F32Ceil`) and FPL
  memory (`fplMemoryAllocate`/`fplMemoryFree`) instead of math.h/stdlib.

## Changelog

- 2026-06-02 — Plan + tracker created.
- 2026-06-02 — Implemented all phases (final_graphics.h primitives/pipeline/
  texture + scenario-based demo). Added `F32Floor`/`F32Ceil` to final_math.h.
  Compiles + runs; offscreen test passes. Flagged `V4fMultM4f` bug.
