# final_math.h — Doxygen Documentation Pass

Goal: add Doxygen-style comments to **all** types, constants and functions in
`demos/additions/final_math.h`, using the `@` command identifier, matching the
convention already used in `final_platform_layer.h` (e.g. `@brief`, `@param[in]`,
`@return`).

Not started yet — deferred to a fresh session (full pass is too large for a
near-exhausted budget; a half-done pass would leave the file inconsistent).

## Style

- Block comment above each item:
  ```c
  /**
  * @brief One-line summary.
  * @param[in] x Description.
  * @return Description.
  */
  ```
- Types: `@struct` / `@union` + `@brief`; document notable members inline with
  `//!< ...` where helpful.
- Constants: `@brief`.
- Keep it terse and accurate; no behavior changes — comments only.

## Important context (already established this branch)

- Matrices are **column-major** (`r[i]` = column i, i.e. `r[col][row]`),
  GL-correct. Transform is `M * v` (see `V4fMultM4f` / `V3fMultM3f` /
  `V2fMultMat2`). Compose model as `mvp = M4fMult(proj, M4fMult(T, S))`.
- Constants are self-defined (no math.h/float.h constants). math.h is included
  only for functions.
- See memory: `project_v4fmultm4f_transpose_bug.md` for the convention + recent
  fixes. Test suite is `apps/mathtest/` (225 checks) — keep it green.

## Order (one commit per section, so progress is safe if interrupted)

1. Header types + constants (Ratio64, Vec2i/2u/2f, Rect2f, Vec3f, Vec4f,
   Mat2f, Mat4f, Quaternion, Pixel, Viewport4i/4f) + the `const float F32*`.
2. Scalar functions (F32*).
3. Vec2i / Vec2u functions.
4. Vec2f functions.
5. Vec3f functions.
6. Vec4f functions.
7. Mat2f functions.
8. Mat4f functions (incl. Ortho/Perspective/LookAt/Translation/Scale/Rotation/
   Mult/Transpose/Inverse).
9. Quaternion functions.
10. Color / Pixel functions (Pixel pack/unpack, sRGB<->linear, RGBA/BGRA).
11. Viewport / Ratio functions + project/unproject (V2iProject/V2fUnproject).
12. Mat3f section (at end of file) — types + functions.

## Verify after the pass

- `make -C apps mathtest` still builds and prints `225/225 passed`.
- A C++ TU including final_math.h still compiles (overloads unaffected).
- No accidental code edits (diff should be comments only).
