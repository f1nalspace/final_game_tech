# FPL_Software — Progress Tracker

Tracks implementation of [FPL_Software_Plan.md](./FPL_Software_Plan.md).

Status legend: ☐ todo · ◐ in progress · ☑ done · ✗ blocked

Last updated: 2026-06-02

---

## Phase 1 — Normal Pixel Rendering (top-down)

| # | Task | Status | Notes |
|---|------|--------|-------|
| 1.1 | Filled quad with clipping (`BackbufferFillQuad`) | ☐ | |
| 1.2 | H/V line, 1px, clipping (`BackbufferDrawHLine`/`VLine`) | ☐ | |
| 1.3 | Filled circle, Bresenham, clipping (`BackbufferFillCircle`) | ☐ | |

## Phase 2 — Orthographic Pixel Pipeline

| # | Task | Status | Notes |
|---|------|--------|-------|
| 2.1 | Pipeline struct + ortho/model setup + project | ☐ | |
| 2.2 | Quad (float) filled/stroked + thickness + real clip | ☐ | |
| 2.3 | Line (float) + thickness + real clip | ☐ | |
| 2.4 | Circle (float) filled/stroked + thickness + real clip | ☐ | |
| 2.5 | `Texture2D` + load via stb_image | ☐ | |
| 2.6 | Textured quad (float) + 4 UVs + real clip | ☐ | |
| 2.7 | Sub-pixel rendering + bilinear filtering | ☐ | |

## Phase 3 — Scenario System

| # | Task | Status | Notes |
|---|------|--------|-------|
| 3.1 | Scenario scaffolding (struct/union, F1 switch) | ☐ | Build BEFORE scenarios |
| 3.2 | Scenario: **Planes** (current impl) | ☐ | Phase 1 baseline / regression |
| 3.3 | Scenario: **Pixel Primitives** | ☐ | Phase 1.1–1.3 clip showcase |
| 3.4 | Scenario: **Ortho Pipeline** | ☐ | Phase 2.1–2.2 |
| 3.5 | Scenario: **Vector Shapes** | ☐ | Phase 2.2–2.4 stroke/fill+thickness |
| 3.6 | Scenario: **Textured Quad** | ☐ | Phase 2.5–2.6 UV control |
| 3.7 | Scenario: **Sub-Pixel Motion** | ☐ | Phase 2.7 bilinear smooth |

---

## Changelog

- 2026-06-02 — Plan + tracker created. No code yet.
