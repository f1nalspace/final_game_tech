# Plan: Port `final_tiletrace.hpp` (C++11) → `final_tiletrace.h` (pure C17)

Goal: a standalone single-file header that produces the *exact same* chain-segment
output as the existing C++ tracer, but written in pure C17, using an arena-based
growing memory scheme with a pluggable allocator, no STL, no other Final libraries,
fully public (non-opaque) data, and an iterative step-API the demo can visualize.

Source of truth for behavior: `final_tiletrace.hpp` (961 lines).
Style/structure reference: `final_dynamic_opengl.h` (use `@` for doxygen, not `\`).

---

## 0. Hard constraints (from the request)

- Arena memory scheme that can **grow** (algorithm needs dynamic arrays / lists).
- Caller may pass a **custom allocator**; default allocator is `malloc`/`free`.
- Algorithm must behave **exactly** as before, but be **as efficient as possible**.
- Prefix everything with `ftt` (types `fttChainSegment`, defines `FTT_IMPLEMENTATION`, …).
- **No** Final libraries — fully standalone (only C stdlib).
- Custom math structs **only**. No `final_math.h`, no use-final-math switch
  (user decision 2026-06-04). Library defines its own `fttVec2i`/`fttVec2u`.
- **No opaque structs** — every field accessible; demo reads internals to draw state.
- Same code style/structure as `final_dynamic_opengl.h`, `@` doxygen commands.

---

## 1. File skeleton (mirror `final_dynamic_opengl.h`)

```
/***
final_tiletrace.h
  About / Getting started / Usage / Preprocessor overrides / License
***/

/*!
  @file final_tiletrace.h
  @version v2.0.0
  @author Torsten Spaete
  @brief Final TileTrace (FTT) - C17 single file header contour tile tracing library.
*/

// > Changelog  (v2.0.0: full C17 rewrite, arena allocator, pluggable allocator)

#ifndef FTT_INCLUDE_H
#define FTT_INCLUDE_H
   // --- includes, config macros, api macros, bool/null ---
   // --- math types (custom or final_math) ---
   // --- allocator + arena types ---
   // --- dynamic array types ---
   // --- enums (direction, step) ---
   // --- core structs (tile, edge, tile verts/indices/edges, chain segment) ---
   // --- main tracer struct (fttTileTracer) ---
   // --- public function prototypes + inline accessors ---
#endif // FTT_INCLUDE_H

#if defined(FTT_IMPLEMENTATION) && !defined(FTT_IMPLEMENTED)
#  define FTT_IMPLEMENTED
   // --- allocator/arena impl ---
   // --- dynamic array impl ---
   // --- internal helpers (ftt__ prefix) ---
   // --- public api impl ---
#endif
```

Config / boilerplate macros to add (match FGL conventions):
- `FTT_INCLUDE_H` include guard, `FTT_IMPLEMENTATION` / `FTT_IMPLEMENTED`.
- `FTT_API_AS_PRIVATE` → `ftt_api` = `static` or `extern` (keep existing knob).
- `ftt_null` (use `NULL` in C; keep macro for parity).
- `ftt_inline` (`static inline`).
- `#include <stdint.h> <stddef.h> <stdbool.h> <string.h> <assert.h>` (and `<stdlib.h>` in impl for malloc/free).
- Internal-only symbols use `ftt__` double-underscore prefix (FGL convention).

---

## 2. Math types (custom only)

Custom, standalone — no `final_math.h`, no switch macro:

```c
typedef union fttVec2i {
    struct { int32_t x, y; };
    struct { int32_t w, h; };
    int32_t e[2];
} fttVec2i;

typedef union fttVec2u {
    struct { uint32_t x, y; };
    struct { uint32_t w, h; };
    uint32_t e[2];
} fttVec2u;
```

Note: anonymous structs inside a union are C11/C17 standard — fine for C17.

Math helpers (ftt-defined, operate on `fttVec2i`, single-line comments):
- `fttV2i(x,y)`, `fttV2iIsEqual(a,b)`, `fttV2iSub(a,b)`, `fttV2iDot(a,b)`.
- Static direction table `ftt__directions[4] = {(0,-1),(1,0),(0,1),(-1,0)}` (Up,Right,Down,Left).

---

## 3. Allocator + Arena (the core new piece)

### 3.1 Allocator interface (pluggable, default malloc/free)

```c
typedef struct fttAllocator {
    void *(*allocate)(void *userData, size_t size);   // returns block >= size
    void  (*release)(void *userData, void *ptr);      // frees block from allocate
    void  *userData;
} fttAllocator;

ftt_api fttAllocator fttDefaultAllocator(void); // malloc/free wrappers, userData=NULL
```

Passing `NULL` allocator to init → use `fttDefaultAllocator()`.
Allocator hands out **large blocks**; the arena sub-allocates from them.

### 3.2 Growing arena (linked list of blocks)

```c
typedef struct fttMemoryBlock {
    uint8_t *base;
    size_t   size;
    size_t   used;
    struct fttMemoryBlock *next;
} fttMemoryBlock;

typedef struct fttArena {
    fttMemoryBlock *first;
    fttMemoryBlock *current;
    fttAllocator    allocator;
    size_t          minBlockSize;   // default e.g. 64 KB
} fttArena;
```

- `ftt__ArenaInit(arena, allocator, minBlockSize)`.
- `ftt__ArenaPush(arena, size, align)` → bump in `current`; if it doesn't fit,
  request a new block of `max(minBlockSize, size+align)` from the allocator,
  chain it, then bump. This is the "grows" requirement.
- `ftt__ArenaFree(arena)` → walk block list, `allocator.release` each block, reset.
- Alignment helper (default `_Alignof(max_align_t)` or 16) so `fttVec2i`/structs are aligned.

Rationale: arena = one owner for all dynamic memory; teardown is O(blocks);
no per-element frees; cache-friendly contiguous storage.

### 3.3 Growable dynamic array on top of the arena

The algorithm needs `push_back`, indexed access, ordered `erase` (one spot),
and `pop_back` (stack). Implement a typed growable array.

```c
typedef struct fttArray {       // generic header (kept public/non-opaque)
    void   *items;
    size_t  count;
    size_t  capacity;
    size_t  itemSize;
    fttArena *arena;
} fttArray;
```

Operations (`ftt__` helpers, used through typed wrapper macros):
- `ftt__ArrayReserve(arr, n)`: if `n > capacity`, allocate new region from arena,
  `memcpy` old → new, repoint. Grow policy: `max(n, capacity*2, 8)`.
- `ftt__ArrayPush(arr, &value)`: reserve `count+1`, copy, `count++`, return index.
- `ftt__ArrayRemoveOrdered(arr, index)`: `memmove` tail left by one, `count--`
  (matches `std::vector::erase` order-preserving semantics in `RemoveOverlapEdges`).
- `ftt__ArrayPop(arr)`: `count--` (openList LIFO `pop_back`).
- `ftt__ArrayClear(arr)`: `count = 0` (capacity/memory retained).
- Indexed access via `((T*)arr.items)[i]`.

Typed convenience (either thin macros `FTT_ARRAY_AT(arr,T,i)` or per-type structs).
Recommend **typed view structs** for readability + non-opaque inspection in the demo:
`fttTileArray`, `fttTilePtrArray`, `fttVec2iArray`, `fttEdgeArray`, `fttChainSegmentArray`
— each is just `{ T *items; size_t count, capacity; fttArena *arena; }`.

### 3.4 Efficiency: pre-reserve from known bounds (no growth in practice)

Tile count `N = w*h` gives tight upper bounds, so reserve up front and the arrays
**never realloc** during a run (single allocation each, zero copy/waste):
- `tiles`            : exactly `N`.
- `openList`         : ≤ `N`.
- `mainVertices`     : ≤ `4*N` (dedup keeps it smaller).
- `mainEdges`        : ≤ `4*N`.
- `chainSegments`    : small; reserve modest, allow grow.
- per-segment verts  : ≤ current `mainEdges.count`; reserve lazily on segment creation.

Growable path remains as a safe fallback; with reservations it stays a no-op.

---

## 4. Core data structs (C ports, all public)

Direct field-for-field ports (single-line comments, ftt prefix):

```c
typedef enum fttDirection {            // table order Up,Right,Down,Left
    FTT_DIRECTION_UP = 0, FTT_DIRECTION_RIGHT, FTT_DIRECTION_DOWN, FTT_DIRECTION_LEFT,
    FTT_DIRECTION_COUNT
} fttDirection;

typedef enum fttStep {
    FTT_STEP_NONE = 0,
    FTT_STEP_FIND_START,
    FTT_STEP_GET_NEXT_OPEN_TILE,
    FTT_STEP_FIND_NEXT_TILE,
    FTT_STEP_ROTATE_FORWARD,
    FTT_STEP_TRAVERSE_FIND_STARTING_EDGE,
    FTT_STEP_TRAVERSE_NEXT_EDGE,
    FTT_STEP_DONE,
} fttStep;

typedef struct fttTile {
    int32_t  x, y;
    uint32_t traceDirection;   // 0..3 index into ftt__directions
    int32_t  isSolid;          // >0 solid, 0 empty, -1 visited/removed
} fttTile;

typedef struct fttEdge {
    int32_t   index;           // 0..3 local edge (left,top,right,bottom)
    int32_t   vertIndex0;      // index into mainVertices
    int32_t   vertIndex1;
    fttVec2i  tilePosition;
    bool      isInvalid;       // consumed in phase 2
} fttEdge;

typedef struct fttTileVertices { fttVec2i verts[4]; } fttTileVertices;
typedef struct fttTileIndices  { int32_t indices[4]; } fttTileIndices;
typedef struct fttTileEdges    { fttEdge edges[4]; uint32_t count; } fttTileEdges;

typedef struct fttChainSegment { fttVec2iArray vertices; } fttChainSegment;
```

### 4.1 Pointer-stability refactor (important correctness detail)

The C++ code holds raw pointers into vectors across steps. In C with arena arrays,
prefer **indices** wherever the backing array could (in principle) realloc:

- `startTile` / `curTile` / `nextTile`: the `tiles` array is allocated **once** at a
  fixed size → `fttTile*` pointers stay valid. Keep as pointers (or store indices).
- `openList`: stores `fttTile*` into the stable `tiles` array → safe as pointers.
- `startEdge` / `lastEdge`: in the C++ code these point into `mainEdges`. Phase 1
  finishes (no more growth) before Phase 2 traverses, so they are stable there too —
  but to be realloc-safe and clear, store them as **`int32_t` indices** into `mainEdges`
  (`startEdgeIndex`, `lastEdgeIndex`, `-1` = none).
- `curChainSegment`: C++ takes `&chainSegments[last]` right after `push_back` — fragile.
  Store **`int32_t curChainSegmentIndex`** into `chainSegments` instead.

This is the single behavioral-safety change; logic/output is identical.

### 4.2 Main tracer struct

```c
typedef struct fttTileTracer {
    fttAllocator        allocator;     // resolved (default if caller passed none)
    fttArena            arena;         // owns all dynamic memory below

    fttVec2u            tileCount;
    fttTileArray        tiles;         // row-major, modified in place
    fttStep             curStep;

    fttTile            *startTile;
    fttTile            *curTile;
    fttTile            *nextTile;

    int32_t             startEdgeIndex;        // into mainEdges, -1 = none
    int32_t             lastEdgeIndex;
    int32_t             curChainSegmentIndex;  // into chainSegments, -1 = none

    fttTilePtrArray     openList;      // LIFO stack of fttTile*
    fttVec2iArray       mainVertices;  // dedup vertex pool
    fttEdgeArray        mainEdges;     // boundary edges
    fttChainSegmentArray chainSegments;// output
} fttTileTracer;
```

---

## 5. Public API (C functions replace the C++ class)

```c
ftt_api void  fttInitTileTracer(fttTileTracer *tracer, fttVec2u tileCount,
                                const uint8_t *mapTiles, const fttAllocator *allocator /*nullable*/);
ftt_api bool  fttNextTileTraceStep(fttTileTracer *tracer);
ftt_api void  fttRunTileTracer(fttTileTracer *tracer);   // loop NextStep until false
ftt_api void  fttFreeTileTracer(fttTileTracer *tracer);  // frees arena via allocator.release
```

Inline accessors mirroring the old C++ getters (non-opaque, but convenient & const-safe):
`fttGetChainSegmentCount`, `fttGetChainSegment`, `fttGetVertexCount`, `fttGetVertex`,
`fttGetEdgeCount`, `fttGetEdge`, `fttGetTile(x,y)`, `fttGetOpenTileCount`,
`fttGetOpenTile`, `fttGetStartTile`, `fttGetCurrentTile`.
(Thin `static inline`; demo can also touch fields directly since nothing is opaque.)

`fttInitTileTracer` responsibilities:
1. Resolve allocator (`*allocator` or `fttDefaultAllocator()`), store it.
2. `ftt__ArenaInit`.
3. Reserve arrays per §3.4 bounds.
4. Fill `tiles` from `mapTiles` (`MakeTile(x,y,isSolid)` per cell).
5. Reset state: `curStep = FTT_STEP_FIND_START`, indices = -1, pointers = NULL,
   clear openList/mainVertices/mainEdges/chainSegments.

(No constructor/`Run()`/`Next()` C++ wrappers; that surface is gone in pure C.)

---

## 6. Internal helper mapping (C++ `internals::` → C `ftt__`)

One-to-one ports, semantics unchanged:

| C++ (`internals::`)        | C (`ftt__`)                  | Notes |
|----------------------------|------------------------------|-------|
| `ArrayCount`               | drop (`FTT_ARRAY_COUNT` macro)| fixed C arrays |
| `ArrayRemoveAndKeepOrder`  | folded into `ftt__ArrayRemoveOrdered` | |
| `V2i/IsEqual/Subtract/Dot` | `ftt__V2i/IsEqual/Sub/Dot`   | §2 |
| `MakeTile/MakeTileEdge`    | `ftt__MakeTile/MakeTileEdge` | |
| `ComputeTileIndex`         | `ftt__ComputeTileIndex`      | `y*w+x` |
| `IsTileSolid/GetTile/SetTileSolid/RemoveTile` | same names `ftt__…` | operate on `fttTileArray` |
| `GetFirstSolidTile`        | `ftt__GetFirstSolidTile`     | row-major scan |
| `CreateTileVertices`       | `ftt__CreateTileVertices`    | corner layout v0..v3 |
| `PushTileVertices`         | `ftt__PushTileVertices`      | linear dedup into mainVertices |
| `CreateTileEdges`          | `ftt__CreateTileEdges`       | 4 CW edges |
| `RemoveOverlapEdges`       | `ftt__RemoveOverlapEdges`    | antiparallel cancel, ordered erase |
| `IsTileSharesCommonEdges`  | `ftt__IsTileSharesCommonEdges`| |
| `ClearLineSegmentPoints`   | `ftt__ClearLineSegmentPoints`| collinear `dot>0` removal |
| `OptimizeChainSegment`     | `ftt__OptimizeChainSegment`  | last 3 verts |
| `FinalizeChainSegment`     | `ftt__FinalizeChainSegment`  | wrap-around seam (two checks) |
| `AddChainSegmentVertex`/`RemoveSegmentVertex` | `ftt__…` | array push/remove |
| `ProcessTraverseNextEdge`  | `ftt__ProcessTraverseNextEdge`| uses `lastEdgeIndex`/`startEdgeIndex` |
| `ProcessTraverseFindStartingEdge` | `ftt__ProcessTraverseFindStartingEdge` | creates segment, sets `curChainSegmentIndex` |
| `GetNextOpenTile`          | `ftt__GetNextOpenTile`       | peek last / back to FindStart |
| `RotateForward`            | `ftt__RotateForward`         | advance dir or pop |
| `AddTile`                  | `ftt__AddTile`               | push openList, remove tile, push verts/edges, cancel overlaps |

`fttNextTileTraceStep`: same `switch(curStep)` state machine as C++ lines 862–937,
1:1, just using indices for edge/segment references and `ftt__` calls.

Edge/segment pointer dereferences in the original become index lookups:
e.g. `traceState->lastEdge->vertIndex1` → `tracer->mainEdges.items[tracer->lastEdgeIndex].vertIndex1`.
"`lastEdge = NULL`" → "`lastEdgeIndex = -1`".

---

## 7. Behavior-parity checklist (must match C++ exactly)

- Row-major start scan; first `isSolid>0` tile.
- Vertex layout v0=(x,y+1) v1=(x,y) v2=(x+1,y) v3=(x+1,y+1); edges CW.
- Linear vertex dedup (exact equality) — keep linear scan to preserve index order
  (output indices must match; do NOT swap in a hash map that changes insertion order).
- Antiparallel edge cancellation removes **both** input + matching main edge; ordered erase.
- Neighbor accepted only if solid AND shares a common (antiparallel) edge.
- `traceDirection` rotate Up→Right→Down→Left, then pop.
- Phase 2: starting edge = first `!isInvalid`; closed-loop vs open continuation vs dead-end
  exactly as lines 712–787; closed loop appends copy of first vertex.
- Collinear optimization `dot(d1,d2)>0` removes middle; finalize does both seam checks.

Self-check: identical chain segment count, vertex counts, and vertex coordinates
for the demo's tile map.

---

## 8. Demo update (`demos/FTT_TileTracingDemo`)

The demo is **fully switched to C17** — no C++ remnants, compiled as a pure `.c` TU.
The C++ `.hpp` and its class API are marked **obsolete** (kept temporarily for
reference, removed in a later step).

- Port `ftt_tiletracingdemo.cpp` to use the C API (`#include "final_tiletrace.h"`):
  `fttTileTracer` value, `fttInitTileTracer(&t, tileCount, map, NULL)`,
  `fttNextTileTraceStep(&t)`, accessors `fttGetTile`, `fttGetVertex`, `fttGetEdge`,
  `fttGetChainSegment(...).vertices.items[i]`, `fttFreeTileTracer(&t)`.
- C-port the file to `ftt_tiletracingdemo.c` (drop C++ `{}`-init / refs); update
  `.vcxproj` / `.filters` / `.sln` references to the new file.
- Keep the rest of the visualization untouched — it only reads tracer state, all public.
- This is the manual verification harness: step through and confirm visuals match the
  old C++ run before deleting the `.hpp`.
- Mark `final_tiletrace.hpp` obsolete now (header banner note); schedule its removal
  once the C demo is confirmed working.

---

## 9. Build / test verification

1. C17 compile check (standalone, default allocator): a tiny TU with
   `#define FTT_IMPLEMENTATION` + `#include "final_tiletrace.h"`, run on the demo map,
   print segment/vertex/edge counts.
2. Diff output (counts + vertices) against the C++ tracer → must be identical.
3. Compilers: gcc/clang `-std=c17 -Wall -Wextra`; also MSVC `/std:c17` if available.
4. Free path: run under a leak checker (or wrap default allocator with counters) to
   confirm `fttFreeTileTracer` releases every block.

---

## 10. Step-by-step execution order

1. Header skeleton + macros/includes/api knobs (§1).
2. Custom math types + helpers (§2).
3. Allocator + default malloc/free (§3.1).
4. Growing arena (§3.2).
5. Generic + typed dynamic arrays (§3.3).
6. Core structs + enums (§4), main `fttTileTracer` (§4.2) with index-based refs (§4.1).
7. Public prototypes + inline accessors (§5).
8. Implementation block: arena/array impl, `ftt__` helpers (§6), state machine,
   init/next/run/free.
9. Reserve-from-bounds tuning (§3.4).
10. Port/verify demo (§8), run parity + build checks (§7, §9).
11. Doc blocks (About/Usage/Preprocessor overrides/Changelog) + license header,
    `@` doxygen, single-line comments, braces on all conditionals (repo style rules).

## 11. Decisions (locked)

- Custom math only; no `final_math.h` (2026-06-04).
- Demo fully switched to C17 (pure `.c`); C++ `.hpp` marked obsolete, removed later (2026-06-04).
- Default arena `minBlockSize` = 64 KB (2026-06-04).
