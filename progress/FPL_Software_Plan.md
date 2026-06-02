# FPL_Software — Extension Plan

Plan for extending `demos/FPL_Software` from a single hardcoded scene into a
multi-scenario software renderer with a pixel-pipeline (orthographic + model
matrix) and textured/sub-pixel drawing.

## Current State (baseline)

- `demos/FPL_Software/fpl_software.c`
  - Single hardcoded scene: noise background, 4 border lines (the "planes"),
    one white quad bouncing inside, integrated + collided every frame.
  - Draws via `final_graphics.h`.
- `demos/additions/final_graphics.h`
  - `BackbufferDrawLine` (Bresenham, **no clipping** — only a global index
    bounds check, so lines can wrap across rows).
  - `BackbufferDrawRect` (axis-aligned fill, clamped, integer-snapped,
    exclusive max edge).
- `demos/additions/final_math.h` provides everything needed for the pipeline:
  - `Vec2f`, `Vec3f`, `Vec4f`, `Mat4f`.
  - `M4fOrthoRH` / `M4fOrthoLH`, `M4fMult`, `V4fMultM4f`,
    `M4fTranslationV2`, `M4fScaleV2`, `M4fIdentity`.
- `stb_image.h` available at `demos/dependencies/stb/stb_image.h`.

## Design Principles

- Keep the **header-only** style. All new drawing primitives go into
  `final_graphics.h` behind the existing `FINAL_GRAPHICS_IMPLEMENTATION` macro.
- Stay **C99** (the demo forces C). No C++-only math overloads.
- Follow repo conventions from memory: single-line comments, brace **all**
  conditional bodies even single-statement, short commits with no
  `Co-Authored-By` trailer.
- Two clearly separated layers:
  1. **Pixel layer** — integer, top-down, y-down. Raw backbuffer ops.
  2. **Pipeline layer** — float world coords through ortho-proj + model
     matrix, mapped to screen, with real (analytic) clipping.

---

## Phase 1 — Normal Pixel Rendering (top-down pixel coordinates)

Integer pixel space, origin top-left, y increasing downward. All functions
clip against the backbuffer rectangle (no wrap, no overdraw outside bounds).

### 1.1 Filled quad with clipping
`BackbufferFillQuad(bb, x0, y0, x1, y1, color)`
- Normalize min/max, clamp to `[0,w)`/`[0,h)`.
- Fix the existing exclusive-edge quirk: decide and document
  inclusive-min / exclusive-max consistently.
- Row-major fill loop.

### 1.2 Horizontal / vertical line, 1px thick, clipped
`BackbufferDrawHLine(bb, x0, x1, y, color)`
`BackbufferDrawVLine(bb, x, y0, y1, color)`
- Reject if the constant axis is out of range.
- Clamp the spanning axis to bounds, then write the run.

### 1.3 Filled circle (Bresenham/midpoint) with clipping
`BackbufferFillCircle(bb, cx, cy, radius, color)`
- Midpoint circle to find span per scanline; fill each scanline as a clipped
  horizontal run (reuses 1.2 clamp logic).
- Per-row clip; reject fully off-screen radius early.

**Deliverable:** primitives compile and are exercised by a debug scenario.

---

## Phase 2 — Orthographic Pixel Pipeline (any coordinate system)

A small pipeline that lets a scenario pick its own world coordinate system via
an orthographic projection, plus a per-object model matrix (scale + position).
All math via `final_math.h`.

### 2.1 Pipeline struct + setup
```c
typedef struct PixelPipeline {
    Mat4f proj;        // M4fOrthoRH(left,right,bottom,top,near,far)
    Mat4f model;       // scale * translation (per object)
    Mat4f mvp;         // proj * model, recomputed on change
    int   viewportW;   // backbuffer width
    int   viewportH;   // backbuffer height
} PixelPipeline;
```
- `PipelineInitOrtho(pl, bb, left, right, bottom, top)` — build proj, store viewport.
- `PipelineSetModel(pl, posV2, scaleV2)` — `model = T * S`; recompute `mvp`.
- `PipelineProject(pl, worldPt) -> Vec2f screenPx` — `V4fMultM4f` then NDC→pixel
  (`x = (ndc.x*0.5+0.5)*w`, `y = (1 - (ndc.y*0.5+0.5))*h` for y-down screen).
- Keep transforms 2D (z=0, w=1); ortho near/far trivial.

### 2.2 Arbitrary quad (float), filled or stroked, variable thickness, real clipping
`PipelineDrawQuad(pl, bb, p0,p1,p2,p3, color, filled, thickness)`
- 4 corners in world space → projected to screen floats.
- **Filled:** scanline polygon fill of the projected quad (handles rotation /
  non-axis-aligned), clipped per scanline to backbuffer.
- **Stroked:** draw the 4 edges as thick lines (see 2.3) with `thickness`.
- "Real clipping" = analytic clip of the projected polygon to the viewport
  rectangle (Sutherland–Hodgman), not just per-pixel rejection.

### 2.3 Arbitrary line (float), variable thickness, real clipping
`PipelineDrawLine(pl, bb, a, b, color, thickness)`
- Project endpoints; build a thick segment as a quad (expand by half-thickness
  along the segment normal).
- Liang–Barsky / Cohen–Sutherland clip the segment to the viewport before
  rasterizing; fall back to scanline fill of the thickness quad.

### 2.4 Arbitrary circle (float), filled or stroked, variable thickness, real clipping
`PipelineDrawCircle(pl, bb, center, radius, color, filled, thickness)`
- Project center + radius (radius scaled by model scale).
- **Filled:** scanline fill in screen space (analytic span per row), clipped.
- **Stroked:** fill ring between `radius - t/2` and `radius + t/2` per scanline.

### 2.5 2D texture loaded via stb_image
```c
typedef struct Texture2D {
    uint32_t *pixels;  // RGBA8, premultiplied or straight (document)
    int width, height;
} Texture2D;
```
- `TextureLoadFromFile(tex, path)` — `stbi_load(..., 4)`, convert to backbuffer
  pixel layout (match `fplColor32` channel order), store.
- `TextureRelease(tex)` — `stbi_image_free`.
- stb included once in the demo TU with `STB_IMAGE_IMPLEMENTATION`.

### 2.6 Textured quad (float) with 4 UVs, real clipping
`PipelineDrawTexturedQuad(pl, bb, p0..p3, uv0..uv3, tex)`
- Project the 4 world corners to screen.
- Scanline-fill with **perspective-irrelevant** (affine) UV interpolation across
  the quad (split into 2 triangles, barycentric/edge interpolation of UV).
- Sutherland–Hodgman clip of the projected polygon (carry interpolated UVs at
  new clip vertices) → real clipping.
- Nearest-texel sample first (correctness pass).

### 2.7 Sub-pixel rendering via bilinear filtering
- Switch rasterizers from integer-snapped to sub-pixel:
  - Use floating screen coords for edges (top-left fill rule, fractional
    coverage on edges optional).
  - Texture sampling: bilinear filter (`sample4 + lerp` on fractional UV*size).
- Goal: smooth animation when objects move by fractional pixels.
- Add a global toggle (e.g. key) to compare nearest vs bilinear for validation.

**Deliverable:** pipeline + textured/sub-pixel drawing usable by any scenario.

---

## Phase 3 — Scenario System

Before any scenario is implemented, build the scaffolding so scenarios are
swappable and self-contained.

### 3.1 Architecture
- One `struct` per scenario holding its own state.
- A global tagged union/struct holding **all** scenarios:
```c
typedef enum ScenarioType {
    ScenarioType_Planes = 0,      // Phase 1 baseline
    ScenarioType_PixelPrimitives, // Phase 1.1-1.3
    ScenarioType_OrthoPipeline,   // Phase 2.1-2.2
    ScenarioType_VectorShapes,    // Phase 2.2-2.4
    ScenarioType_TexturedQuad,    // Phase 2.5-2.6
    ScenarioType_SubPixelMotion,  // Phase 2.7
    ScenarioType_Count
} ScenarioType;

typedef struct PlanesScenario        { /* rect pos/vel/radius, world bounds */ } PlanesScenario;
typedef struct PixelPrimitivesScenario { /* anim phase for clip showcase */ }    PixelPrimitivesScenario;
typedef struct OrthoPipelineScenario { /* world bounds, model pos/scale */ }      OrthoPipelineScenario;
typedef struct VectorShapesScenario  { /* thickness, fill/stroke toggles */ }     VectorShapesScenario;
typedef struct TexturedQuadScenario  { /* Texture2D, uv animation phase */ }      TexturedQuadScenario;
typedef struct SubPixelMotionScenario{ /* fractional pos, filter toggle */ }      SubPixelMotionScenario;

typedef struct AppScenarios {
    ScenarioType current;
    union {
        PlanesScenario          planes;
        PixelPrimitivesScenario pixelPrimitives;
        OrthoPipelineScenario   orthoPipeline;
        VectorShapesScenario    vectorShapes;
        TexturedQuadScenario    texturedQuad;
        SubPixelMotionScenario  subPixelMotion;
    } data;
} AppScenarios;
```
- Each scenario exposes a uniform interface (function pointers or a switch):
  - `Init(scn, bb)`
  - `Update(scn, dt, input)`
  - `Render(scn, bb)`
- **F1 key** cycles `current` (`(current + 1) % ScenarioType_Count`),
  re-initializing the newly selected scenario.
- Optional on-screen label of the active scenario.

### 3.2 Scenario catalogue

One scenario per phase area, each built to exercise/validate the primitives of
that phase. Enum order = F1 cycle order.

| Enum | Name | Exercises | Description |
|------|------|-----------|-------------|
| `ScenarioType_Planes` | **Planes** | Phase 1 (baseline) | Current behavior: noise background, 4 border lines as planes, one quad bouncing + colliding against the planes. |
| `ScenarioType_PixelPrimitives` | **Pixel Primitives** | Phase 1.1–1.3 | Static + animated filled quads, H/V lines, filled circles. Objects deliberately pushed partway off every edge to prove clipping. |
| `ScenarioType_OrthoPipeline` | **Ortho Pipeline** | Phase 2.1–2.2 | Custom world coordinate system via ortho projection; a quad positioned/scaled through the model matrix, moving in world units. |
| `ScenarioType_VectorShapes` | **Vector Shapes** | Phase 2.2–2.4 | Float quads, lines, circles — filled vs stroked with variable thickness; some clipped against the viewport to prove analytic clipping. |
| `ScenarioType_TexturedQuad` | **Textured Quad** | Phase 2.5–2.6 | stb-loaded texture drawn on a moving/scaling quad; animate the 4 UVs to show mapping control; partly off-screen to prove clipping. |
| `ScenarioType_SubPixelMotion` | **Sub-Pixel Motion** | Phase 2.7 | Smooth fractional-pixel motion with bilinear filtering; toggle nearest vs bilinear to compare jitter/smoothness. |

#### Scenario 1 — Planes (regression baseline)
- Reproduce existing behavior exactly first (1:1 with `fpl_software.c` today).
- Then reimplement on top of the new primitives to validate them.
- This is the regression baseline for all later work.

#### Implementation order
- Build the scaffolding (3.1) before any scenario.
- Implement each scenario as its phase completes (Planes → after Phase 1;
  Textured/Sub-Pixel → after the matching Phase 2 tasks).
- Each scenario added must be verified before starting the next.

---

## Build / Integration Notes

- New primitives in `demos/additions/final_graphics.h` (decl + impl behind macro).
- Texture/pipeline types may live in `final_graphics.h` or a new
  `final_pixelpipeline.h` addition — decide at Phase 2 start (prefer extending
  `final_graphics.h` to avoid a new include unless it grows large).
- Update `CMakeLists.txt` / `Makefile` / vcxproj only if new files or the stb
  include path are needed (stb path already exists under `demos/dependencies`).
- Verify compile on Linux (primary env) after each phase.

## Verification Strategy

- After each phase, build and run the demo; visually confirm primitives clip
  correctly at all 4 edges (push objects partially off-screen).
- Scenario 1 must look/behave identical to the original before moving on.
- For sub-pixel: confirm smooth motion at fractional velocities (no jitter).
