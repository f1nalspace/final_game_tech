/***
final_ftd.h

-------------------------------------------------------------------------------
	About
-------------------------------------------------------------------------------

Final Text Data (FTD) - An open source single header file parser for a small,
line-oriented, UTF-8 typed-data text format that maps 1:1 onto C99 types
(struct / enum / union / array / scalars / strings / refs).

The library is application-agnostic - the same header parses presentation
files, level files, config files, or anything else, by letting the host
register its own struct schemas, enums, host-registered identifiers, and
helper functions.

- Header name: `final_ftd.h`
- Style: single-header in the FPL tradition (declarations always visible;
  implementation gated by `#define FTD_IMPLEMENTATION` in exactly one TU).
- Memory: arena-based, with a pluggable backing allocator.
- Thread-safety: a context is NOT thread-safe.

-------------------------------------------------------------------------------
	Getting started
-------------------------------------------------------------------------------

	#define FTD_IMPLEMENTATION
	#include "final_ftd.h"

	ftdContext *ctx = ftdCreate(NULL);
	ftdRegisterStruct(ctx, &MyVec2_type);
	ftdResult res = ftdParseFile(ctx, "config.ftd");
	if (res.ok) {
		const MyConfig *cfg = ftdLookup(ctx, "MyAppConfig", NULL);
	}
	ftdDestroy(ctx);

-------------------------------------------------------------------------------
	Switches
-------------------------------------------------------------------------------

| Macro                       | Default | Effect
| FTD_NO_STDIO                | off     | Compiles out ftdParseFile().
| FTD_NO_DEFAULT_ALLOCATOR    | off     | Removes the malloc/free default.
| FTD_ASSERT(expr)            | assert  | Override for internal sanity checks.
| FTD_ARENA_CHUNK_SIZE        | 65536   | Default size of each arena chunk.
| FTD_API                     | extern  | Linkage qualifier on public functions.

-------------------------------------------------------------------------------
	FTD file format specification
-------------------------------------------------------------------------------

	File extension : .ftd
	Encoding       : UTF-8 (BOM optional and ignored)
	Terminator     : newline (\n or \r\n). No semicolons.

	== Lexical ==

	Whitespace: spaces, tabs, \r, \n separate tokens. Newlines end top-level
	statements and act as separators inside { } [ ] ( ) (commas also work).

	Comments:
	    Line comment   : "//" to end of line
	    Line comment   : "#"  to end of line
	    Block comment  : standard C-style multi-line, may span lines

	Identifiers : [A-Za-z_][A-Za-z0-9_]*  (case-sensitive)
	Dotted paths: A.B.C  (qualified names, nested field paths)

	Literals:
	    bool     : true, false
	    null     : null   (becomes zero-initialized field)
	    integer  : 0, 42, -17, 0xFF, 0b1010, 0o77
	               optional suffix: u, i, u8, i32, s64, z (size_t), etc.
	    float    : 1.0, -3.14, 1e9, 0.5f
	               suffix f = f32, d = f64 (default)
	    string   : "hello\nworld"
	               escapes: \n \r \t \" \\ \0 \xHH
	               adjacent literals are concatenated like in C:
	                   "foo" "bar"  becomes  "foobar"
	    char     : 'A', '\n'   (becomes an integer code point)

	Reserved words: alias, true, false, null

	== Top-level statements ==

	Three forms (separated by newlines):

	  1. Alias - rename a type, enum value, or host identifier.
	         alias V2f = Vec2f
	         alias Left = HorizontalAlignment.Left
	         alias <Type> NewName = qname     // optional type prefix is doc only

	  2. File-local constant - bind Name to an evaluated value.
	         float TitleSize = 64.0
	         V4f   LinkGreen = V4f(0.0, 0.8, 0.2, 1.0)

	  3. Named instance - top-level struct value referenceable by name.
	         ImageResource Card_CPU {
	             name = "card_cpu.png"
	             type = File
	             file { relativeFilePath = "card_cpu.png" }
	         }

	== Inline value forms (anywhere a value is expected) ==

	    literal                            // see literals above
	    Name                               // alias / const / instance / global
	    TypeName { field = value ... }     // named-field struct
	    TypeName ( v1, v2, ... )           // positional, declaration order
	    { field = value ... }              // bare block (type inferred from
	                                       // surrounding field's declared type)
	    [ v1, v2, ... ]                    // array literal
	    HelperName ( v1, v2, ... )         // user-registered helper call

	== Field assignment inside { ... } blocks ==

	    field = value             // scalar / inline value / reference
	    field { ... }             // nested struct (same as field = { ... })
	    field [ ... ]             // array          (same as field = [ ... ])
	    field.sub = value         // dotted path; auto-creates intermediates

	Field order does not matter. Fields may appear multiple times - the last
	assignment wins. All fields are optional; omitted fields are zero-init.

	== Arrays ==

	    field = [ a, b, c ]
	    field = [
	        a
	        b
	        c
	    ]
	    field = []                // empty

	No size declared. Separators: comma, newline, or both. Trailing separators
	allowed. Nested arrays are fine: [[1,2],[3,4]].

	== Enums ==

	Either fully qualified or short:
	    type = BlockType.Image
	    type = Image              // ok if unambiguous in this field's type

	== Unions ==

	A union uses the enclosing struct's discriminator field to pick the active
	variant; the variant is written as a nested block under its member name:

	    SoundResource Intro1 {
	        type = File                   // discriminator
	        file { relativeFilePath = "intro.mp3" }
	    }

	Variant blocks whose name does not match the discriminator are ignored
	(with a warning). If a union has no discriminator the first variant set
	wins.

	== References ==

	A pointer-typed field accepts a named instance, global, or alias - no '&'
	and no '*':

	    imageResource = Card_CPU            // backward reference
	    imageResource = Card_CPU_LaterDef   // forward reference (resolved
	                                        // after the whole file is parsed)

	Unresolved names at end-of-file become warnings, not errors.

	== Lookup rules for an identifier in value position ==

	    1. enum value of the field's declared type (short form)
	    2. alias - fold and retry
	    3. host-registered global identifier (dotted path)
	    4. file-local constant or named instance
	    5. enum value of any in-scope enum
	    6. otherwise -> error: undefined name

	== Built-in scalar type names ==

	    bool                           : bool
	    s8, int8                       : int8_t
	    s16, int16                     : int16_t
	    s32, int, int32                : int32_t
	    s64, int64                     : int64_t
	    u8, uint8                      : uint8_t
	    u16, uint16                    : uint16_t
	    u32, uint, uint32              : uint32_t
	    u64, uint64, size, size_t      : uint64_t
	    f32, float                     : float
	    f64, double                    : double
	    string, cstr                   : const char *

	Other type names (Vec2f, BlockDefinition, ...) come from the host, which
	registers them with the parser before loading the file.

	== Robustness ==

	- Unknown field name -> ignored + warning.
	- Unknown type name  -> error, other statements still parse.
	- Wrong scalar type  -> coerced if possible (int<->float etc), otherwise
	                        warning + zero.
	- Missing field      -> zero-initialized; no error.
	- Trailing commas / extra newlines / extra whitespace always OK.

	Renaming, adding, or removing C fields does not break old .ftd files: the
	file simply describes a subset.

	== Minimal example ==

	    alias V2f = Vec2f
	    alias V4f = Vec4f

	    float TitleSize = 64.0
	    V4f   White     = V4f(1, 1, 1, 1)

	    Slide Intro {
	        title    = "Hello FPL"
	        duration = 5.0
	        blocks   = [
	            Block {
	                type = Image
	                pos  = V2f(0, 0)
	                size = V2f(1, 1)
	                image {
	                    imageResource = ImageResources.FPLLogo
	                    tintColor     = White
	                }
	            }
	        ]
	    }

	What is NOT in the format: arithmetic / expressions (a + b, 1 << 3),
	#include / file composition, variables / templates / for-each loops,
	reflection or runtime field introspection, serialization (writing .ftd).

-------------------------------------------------------------------------------
	License
-------------------------------------------------------------------------------

MIT License

Copyright (c) 2026 Torsten Spaete

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
***/

#ifndef FINAL_FTD_H
#define FINAL_FTD_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef FTD_API
#	define FTD_API extern
#endif

#ifndef FTD_ARENA_CHUNK_SIZE
#	define FTD_ARENA_CHUNK_SIZE 65536
#endif

#ifndef FTD_MAX_INCLUDE_DEPTH
#	define FTD_MAX_INCLUDE_DEPTH 8
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct ftdContext ftdContext;
typedef struct ftdType ftdType;
typedef struct ftdField ftdField;
typedef struct ftdValue ftdValue;

// Allocator hook
typedef void *(*ftdAllocFn)(size_t size, size_t align, void *userData);
typedef void  (*ftdFreeFn) (void *ptr,   void *userData);

typedef struct ftdAllocator {
	ftdAllocFn alloc;
	ftdFreeFn  free;
	void      *userData;
} ftdAllocator;

// Field kinds
typedef enum ftdFieldKind {
	ftdFieldKind_None = 0,
	ftdFieldKind_Bool,
	ftdFieldKind_S8, ftdFieldKind_S16, ftdFieldKind_S32, ftdFieldKind_S64,
	ftdFieldKind_U8, ftdFieldKind_U16, ftdFieldKind_U32, ftdFieldKind_U64,
	ftdFieldKind_F32, ftdFieldKind_F64,
	ftdFieldKind_String,
	ftdFieldKind_Enum,
	ftdFieldKind_Struct,
	ftdFieldKind_Array,
	ftdFieldKind_Ref,
	ftdFieldKind_Union,

	// Array decomposed into separate host-struct fields. Use these instead of
	// the single ftdArrayHandle-backed ftdFieldKind_Array when you prefer
	// ergonomic native fields like:
	//     T        *items;
	//     uint32_t  itemCount;
	//     uint32_t  itemCapacity;
	// Field names share a prefix and end with ".data" / ".count" / ".capacity"
	// — for example "blocks.data", "blocks.count", "blocks.capacity". The .ftd
	// source still uses the bare prefix: `blocks = [ ... ]`.
	// .data is required and carries the element type in `subtype`. The other
	// two are optional. Count and capacity are written as uint32_t.
	ftdFieldKind_ArrayData,
	ftdFieldKind_ArrayCount,
	ftdFieldKind_ArrayCapacity,
} ftdFieldKind;

typedef enum ftdFieldFlags {
	ftdFieldFlag_None     = 0,
	ftdFieldFlag_Required = 1 << 0,
	ftdFieldFlag_Hidden   = 1 << 1,
} ftdFieldFlags;

struct ftdField {
	const char     *name;
	uint32_t        offset;
	ftdFieldKind    kind;
	const ftdType  *subtype;
	const char     *discriminator;
	uint32_t        unionTag;
	uint32_t        flags;
};

typedef struct ftdEnumValue {
	const char *name;
	int32_t     value;
} ftdEnumValue;

struct ftdType {
	const char         *name;
	size_t              size;
	size_t              align;
	const ftdField     *fields;
	uint32_t            fieldCount;
	const ftdEnumValue *enumValues;
	uint32_t            enumValueCount;
};

// Default array storage
typedef struct ftdArrayHandle {
	void    *data;
	uint32_t count;
	uint32_t capacity;
} ftdArrayHandle;

// Value scratch type used by helpers and internally
typedef enum ftdValueKind {
	ftdValueKind_None = 0,
	ftdValueKind_Bool,
	ftdValueKind_Int,
	ftdValueKind_UInt,
	ftdValueKind_Float,
	ftdValueKind_String,
	ftdValueKind_Enum,
	ftdValueKind_Ref,
	ftdValueKind_Struct,
	ftdValueKind_Array,
} ftdValueKind;

typedef struct ftdSourceSpan {
	const char *file;
	uint32_t    line;
	uint32_t    column;
	uint32_t    length;
} ftdSourceSpan;

struct ftdValue {
	ftdValueKind         kind;
	const ftdType       *type;
	ftdSourceSpan        span;
	union {
		bool                b;
		int64_t             i;
		uint64_t            u;
		double              f;
		const char         *str;
		void               *ptr;
		ftdArrayHandle      arr;
	} as;
};

// Helper signature
typedef bool (*ftdHelperFn)(ftdContext *ctx,
                            const ftdValue *args,
                            uint32_t        argCount,
                            void           *outValue,
                            void           *userData);

// Diagnostics
typedef enum ftdSeverity {
	ftdSeverity_Info    = 0,
	ftdSeverity_Warning = 1,
	ftdSeverity_Error   = 2,
} ftdSeverity;

typedef struct ftdDiagnostic {
	ftdSeverity   severity;
	ftdSourceSpan span;
	const char   *message;
} ftdDiagnostic;

typedef struct ftdResult {
	bool                ok;
	uint32_t            errorCount;
	uint32_t            warningCount;
	uint32_t            diagnosticCount;
	const ftdDiagnostic *diagnostics;
} ftdResult;

// Context lifecycle
FTD_API ftdContext *ftdCreate (const ftdAllocator *allocator);
FTD_API void        ftdDestroy(ftdContext *ctx);
FTD_API void        ftdResetParse(ftdContext *ctx);

// Registration
FTD_API void ftdRegisterStruct(ftdContext *ctx, const ftdType *type);
FTD_API void ftdRegisterEnum  (ftdContext *ctx, const ftdType *type);
FTD_API void ftdRegisterAlias (ftdContext *ctx, const char *alias, const char *target);
FTD_API void ftdRegisterGlobal(ftdContext *ctx, const char *dottedName, const ftdType *type, const void *value);
FTD_API void ftdRegisterHelper(ftdContext *ctx, const char *name, const ftdType *returnType, ftdHelperFn fn, void *userData);

// Array slot binding (custom T*+count storage)
FTD_API void ftdSetArraySlot(ftdContext *ctx,
                             const ftdType *ownerType,
                             const char    *fieldName,
                             uint32_t       dataOffset,
                             uint32_t       countOffset,
                             uint32_t       countSize);

// Parsing
FTD_API ftdResult ftdParseString(ftdContext *ctx,
                                 const char *source,
                                 size_t      length,
                                 const char *displayName);

#if !defined(FTD_NO_STDIO)
FTD_API ftdResult ftdParseFile(ftdContext *ctx, const char *filePath);
#endif

// Lookup a top-level identifier (alias / const / named instance / global).
FTD_API const void *ftdLookup(ftdContext *ctx, const char *dottedName, const ftdType **outType);

#ifdef __cplusplus
}
#endif

#endif // FINAL_FTD_H

// =============================================================================
// IMPLEMENTATION
// =============================================================================

#if defined(FTD_IMPLEMENTATION) && !defined(FINAL_FTD_IMPLEMENTED)
#define FINAL_FTD_IMPLEMENTED

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#if !defined(FTD_NO_STDIO)
#	include <stdio.h>
#endif

#ifndef FTD_ASSERT
#	include <assert.h>
#	define FTD_ASSERT(expr) assert(expr)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define FTD__UNUSED(x) ((void)(x))

// -----------------------------------------------------------------------------
// Default allocator
// -----------------------------------------------------------------------------
#if !defined(FTD_NO_DEFAULT_ALLOCATOR)
static void *ftd__defaultAlloc(size_t size, size_t align, void *userData) {
	FTD__UNUSED(userData);
	FTD__UNUSED(align);
	void *p = calloc(1, size);
	return p;
}
static void ftd__defaultFree(void *ptr, void *userData) {
	FTD__UNUSED(userData);
	free(ptr);
}
static const ftdAllocator ftd__defaultAllocator = {
	ftd__defaultAlloc, ftd__defaultFree, NULL
};
#endif

// -----------------------------------------------------------------------------
// Arena allocator
// -----------------------------------------------------------------------------
typedef struct ftd__ArenaChunk {
	struct ftd__ArenaChunk *next;
	size_t  capacity;
	size_t  used;
	// payload follows
} ftd__ArenaChunk;

typedef struct ftd__Arena {
	ftd__ArenaChunk    *first;
	ftd__ArenaChunk    *current;
	size_t              defaultChunkSize;
	const ftdAllocator *allocator;
	bool                outOfMemory;
} ftd__Arena;

static size_t ftd__alignUp(size_t v, size_t align) {
	if (align < 1) {
		align = 1;
	}
	return (v + (align - 1)) & ~(align - 1);
}

static ftd__ArenaChunk *ftd__arenaAllocChunk(ftd__Arena *a, size_t neededBytes) {
	size_t cap = a->defaultChunkSize;
	if (neededBytes > cap) {
		cap = neededBytes;
	}
	size_t total = sizeof(ftd__ArenaChunk) + cap;
	void *raw = a->allocator->alloc(total, 16, a->allocator->userData);
	if (raw == NULL) {
		a->outOfMemory = true;
		return NULL;
	}
	ftd__ArenaChunk *c = (ftd__ArenaChunk *)raw;
	c->next = NULL;
	c->capacity = cap;
	c->used = 0;
	return c;
}

static void ftd__arenaInit(ftd__Arena *a, const ftdAllocator *al, size_t chunkSize) {
	a->first = NULL;
	a->current = NULL;
	a->defaultChunkSize = chunkSize ? chunkSize : FTD_ARENA_CHUNK_SIZE;
	a->allocator = al;
	a->outOfMemory = false;
}

static void *ftd__arenaAlloc(ftd__Arena *a, size_t size, size_t align) {
	if (size == 0) {
		size = 1;
	}
	if (align == 0) {
		align = 1;
	}
	if (a->current == NULL) {
		a->first = ftd__arenaAllocChunk(a, size + align);
		if (a->first == NULL) {
			return NULL;
		}
		a->current = a->first;
	}
	for (;;) {
		ftd__ArenaChunk *c = a->current;
		uint8_t *base = (uint8_t *)c + sizeof(ftd__ArenaChunk);
		size_t off = ftd__alignUp(c->used, align);
		if (off + size <= c->capacity) {
			c->used = off + size;
			void *p = base + off;
			memset(p, 0, size);
			return p;
		}
		ftd__ArenaChunk *nc = ftd__arenaAllocChunk(a, size + align);
		if (nc == NULL) {
			return NULL;
		}
		c->next = nc;
		a->current = nc;
	}
}

static char *ftd__arenaStrDupN(ftd__Arena *a, const char *src, size_t length) {
	char *dst = (char *)ftd__arenaAlloc(a, length + 1, 1);
	if (dst == NULL) {
		return NULL;
	}
	if (length > 0) {
		memcpy(dst, src, length);
	}
	dst[length] = '\0';
	return dst;
}

static char *ftd__arenaStrDup(ftd__Arena *a, const char *src) {
	if (src == NULL) {
		return NULL;
	}
	return ftd__arenaStrDupN(a, src, strlen(src));
}

static void ftd__arenaResetKeepFirst(ftd__Arena *a) {
	if (a->first == NULL) {
		return;
	}
	ftd__ArenaChunk *c = a->first->next;
	while (c != NULL) {
		ftd__ArenaChunk *next = c->next;
		if (a->allocator->free) {
			a->allocator->free(c, a->allocator->userData);
		}
		c = next;
	}
	a->first->next = NULL;
	a->first->used = 0;
	a->current = a->first;
	a->outOfMemory = false;
}

static void ftd__arenaFree(ftd__Arena *a) {
	ftd__ArenaChunk *c = a->first;
	while (c != NULL) {
		ftd__ArenaChunk *next = c->next;
		if (a->allocator->free) {
			a->allocator->free(c, a->allocator->userData);
		}
		c = next;
	}
	a->first = NULL;
	a->current = NULL;
	a->outOfMemory = false;
}

// -----------------------------------------------------------------------------
// String hashing (FNV-1a 32-bit)
// -----------------------------------------------------------------------------
static uint32_t ftd__fnv1a(const char *data, size_t len) {
	uint32_t h = 2166136261u;
	for (size_t i = 0; i < len; ++i) {
		h ^= (uint8_t)data[i];
		h *= 16777619u;
	}
	return h;
}
static uint32_t ftd__fnv1aStr(const char *s) {
	return ftd__fnv1a(s, strlen(s));
}

// -----------------------------------------------------------------------------
// Symbol kinds & registry
// -----------------------------------------------------------------------------
typedef enum ftd__SymKind {
	ftd__Sym_None = 0,
	ftd__Sym_Type,
	ftd__Sym_Enum,
	ftd__Sym_Alias,
	ftd__Sym_Global,
	ftd__Sym_Const,
	ftd__Sym_Instance,
	ftd__Sym_Helper,
	ftd__Sym_Builtin,
} ftd__SymKind;

typedef struct ftd__Symbol {
	const char    *name;
	uint32_t       hash;
	ftd__SymKind   kind;
	const ftdType *type;
	ftdFieldKind   scalarKind; // for Const/Builtin of scalar types
	union {
		const char *aliasTarget;
		void       *ptr;
		const void *constPtr;
		struct {
			ftdHelperFn fn;
			void       *userData;
			const ftdType *returnType;
		} helper;
	} as;
} ftd__Symbol;

typedef struct ftd__SymTable {
	ftd__Symbol *entries;
	uint32_t     mask;
	uint32_t     count;
	uint32_t     capacity;
	ftd__Arena  *arena;
} ftd__SymTable;

static void ftd__symTableInit(ftd__SymTable *t, ftd__Arena *arena) {
	t->entries = NULL;
	t->mask = 0;
	t->count = 0;
	t->capacity = 0;
	t->arena = arena;
}

static void ftd__symTableGrow(ftd__SymTable *t, uint32_t newCap);

static ftd__Symbol *ftd__symTableSlot(ftd__SymTable *t, const char *name, uint32_t hash, bool *outExists) {
	if (t->capacity == 0) {
		ftd__symTableGrow(t, 64);
		if (t->capacity == 0) {
			if (outExists) {
				*outExists = false;
			}
			return NULL;
		}
	}
	uint32_t idx = hash & t->mask;
	for (;;) {
		ftd__Symbol *s = &t->entries[idx];
		if (s->kind == ftd__Sym_None) {
			if (outExists) {
				*outExists = false;
			}
			return s;
		}
		if (s->hash == hash && strcmp(s->name, name) == 0) {
			if (outExists) {
				*outExists = true;
			}
			return s;
		}
		idx = (idx + 1) & t->mask;
	}
}

static void ftd__symTableGrow(ftd__SymTable *t, uint32_t newCap) {
	ftd__Symbol *oldEntries = t->entries;
	uint32_t oldCap = t->capacity;
	// power of 2
	uint32_t cap = 16;
	while (cap < newCap) {
		cap <<= 1;
	}
	t->entries = (ftd__Symbol *)ftd__arenaAlloc(t->arena, sizeof(ftd__Symbol) * cap, 16);
	if (t->entries == NULL) {
		t->entries = oldEntries;
		return;
	}
	t->capacity = cap;
	t->mask = cap - 1;
	t->count = 0;
	for (uint32_t i = 0; i < oldCap; ++i) {
		if (oldEntries[i].kind != ftd__Sym_None) {
			ftd__Symbol src = oldEntries[i];
			bool exists = false;
			ftd__Symbol *slot = ftd__symTableSlot(t, src.name, src.hash, &exists);
			if (slot != NULL && !exists) {
				*slot = src;
				t->count++;
			}
		}
	}
}

static ftd__Symbol *ftd__symTableInsert(ftd__SymTable *t, const char *name, uint32_t hash) {
	if (t->capacity == 0 || t->count * 2 >= t->capacity) {
		ftd__symTableGrow(t, t->capacity ? t->capacity * 2 : 64);
	}
	bool exists = false;
	ftd__Symbol *s = ftd__symTableSlot(t, name, hash, &exists);
	if (s == NULL) {
		return NULL;
	}
	if (!exists) {
		s->name = name;
		s->hash = hash;
		t->count++;
	}
	return s;
}

static const ftd__Symbol *ftd__symTableLookup(const ftd__SymTable *t, const char *name) {
	if (t->capacity == 0) {
		return NULL;
	}
	uint32_t hash = ftd__fnv1aStr(name);
	uint32_t idx = hash & t->mask;
	for (;;) {
		const ftd__Symbol *s = &t->entries[idx];
		if (s->kind == ftd__Sym_None) {
			return NULL;
		}
		if (s->hash == hash && strcmp(s->name, name) == 0) {
			return s;
		}
		idx = (idx + 1) & t->mask;
	}
}

// -----------------------------------------------------------------------------
// Array-slot binding
// -----------------------------------------------------------------------------
typedef struct ftd__ArraySlot {
	const ftdType *ownerType;
	const char    *fieldName;
	uint32_t       dataOffset;
	uint32_t       countOffset;
	uint32_t       countSize;
	struct ftd__ArraySlot *next;
} ftd__ArraySlot;

// -----------------------------------------------------------------------------
// Diagnostics (collected during parse)
// -----------------------------------------------------------------------------
typedef struct ftd__DiagNode {
	ftdDiagnostic   d;
	struct ftd__DiagNode *next;
} ftd__DiagNode;

// -----------------------------------------------------------------------------
// Context
// -----------------------------------------------------------------------------
struct ftdContext {
	ftdAllocator        allocator;
	ftd__Arena          schemaArena;
	ftd__Arena          parseArena;

	ftd__SymTable       schemaSyms;   // Type/Enum/Alias/Global/Helper/Builtin
	ftd__SymTable       parseSyms;    // Const/Instance/Alias (parse-time aliases)
	ftd__ArraySlot     *arraySlots;

	// Diagnostics list (parse-arena)
	ftd__DiagNode      *diagHead;
	ftd__DiagNode      *diagTail;
	uint32_t            errorCount;
	uint32_t            warningCount;
	uint32_t            diagCount;

	// Compiled diagnostics array (snapshot)
	ftdDiagnostic      *diagArray;

	ftdResult           lastResult;

	// Source name (arena-owned, shared across spans of a parse)
	const char         *currentSourceName;
};

// -----------------------------------------------------------------------------
// Built-in scalar names (registered during ftdCreate)
// -----------------------------------------------------------------------------
typedef struct ftd__BuiltinEntry {
	const char  *name;
	ftdFieldKind kind;
	size_t       size;
} ftd__BuiltinEntry;

static const ftd__BuiltinEntry ftd__builtinTable[] = {
	{ "bool",    ftdFieldKind_Bool,   sizeof(bool) },
	{ "s8",      ftdFieldKind_S8,     1 },
	{ "int8",    ftdFieldKind_S8,     1 },
	{ "s16",     ftdFieldKind_S16,    2 },
	{ "int16",   ftdFieldKind_S16,    2 },
	{ "s32",     ftdFieldKind_S32,    4 },
	{ "int",     ftdFieldKind_S32,    4 },
	{ "int32",   ftdFieldKind_S32,    4 },
	{ "s64",     ftdFieldKind_S64,    8 },
	{ "int64",   ftdFieldKind_S64,    8 },
	{ "u8",      ftdFieldKind_U8,     1 },
	{ "uint8",   ftdFieldKind_U8,     1 },
	{ "u16",     ftdFieldKind_U16,    2 },
	{ "uint16",  ftdFieldKind_U16,    2 },
	{ "u32",     ftdFieldKind_U32,    4 },
	{ "uint",    ftdFieldKind_U32,    4 },
	{ "uint32",  ftdFieldKind_U32,    4 },
	{ "u64",     ftdFieldKind_U64,    8 },
	{ "uint64",  ftdFieldKind_U64,    8 },
	{ "size",    ftdFieldKind_U64,    8 },
	{ "size_t",  ftdFieldKind_U64,    8 },
	{ "f32",     ftdFieldKind_F32,    4 },
	{ "float",   ftdFieldKind_F32,    4 },
	{ "f64",     ftdFieldKind_F64,    8 },
	{ "double",  ftdFieldKind_F64,    8 },
	{ "string",  ftdFieldKind_String, sizeof(const char *) },
	{ "cstr",    ftdFieldKind_String, sizeof(const char *) },
};

static void ftd__registerBuiltins(ftdContext *ctx) {
	size_t n = sizeof(ftd__builtinTable) / sizeof(ftd__builtinTable[0]);
	for (size_t i = 0; i < n; ++i) {
		const char *name = ftd__builtinTable[i].name;
		uint32_t h = ftd__fnv1aStr(name);
		ftd__Symbol *s = ftd__symTableInsert(&ctx->schemaSyms, name, h);
		if (s == NULL) {
			continue;
		}
		s->kind = ftd__Sym_Builtin;
		s->scalarKind = ftd__builtinTable[i].kind;
		s->type = NULL;
	}
}

// -----------------------------------------------------------------------------
// Public lifecycle
// -----------------------------------------------------------------------------
FTD_API ftdContext *ftdCreate(const ftdAllocator *allocator) {
	const ftdAllocator *al = allocator;
#if !defined(FTD_NO_DEFAULT_ALLOCATOR)
	if (al == NULL) {
		al = &ftd__defaultAllocator;
	}
#endif
	if (al == NULL || al->alloc == NULL) {
		return NULL;
	}
	void *raw = al->alloc(sizeof(ftdContext), 16, al->userData);
	if (raw == NULL) {
		return NULL;
	}
	ftdContext *ctx = (ftdContext *)raw;
	memset(ctx, 0, sizeof(*ctx));
	ctx->allocator = *al;

	ftd__arenaInit(&ctx->schemaArena, &ctx->allocator, FTD_ARENA_CHUNK_SIZE);
	ftd__arenaInit(&ctx->parseArena,  &ctx->allocator, FTD_ARENA_CHUNK_SIZE);

	ftd__symTableInit(&ctx->schemaSyms, &ctx->schemaArena);
	ftd__symTableInit(&ctx->parseSyms,  &ctx->parseArena);

	ftd__registerBuiltins(ctx);
	return ctx;
}

FTD_API void ftdDestroy(ftdContext *ctx) {
	if (ctx == NULL) {
		return;
	}
	ftdAllocator al = ctx->allocator;
	ftd__arenaFree(&ctx->schemaArena);
	ftd__arenaFree(&ctx->parseArena);
	if (al.free) {
		al.free(ctx, al.userData);
	}
}

FTD_API void ftdResetParse(ftdContext *ctx) {
	if (ctx == NULL) {
		return;
	}
	ftd__arenaResetKeepFirst(&ctx->parseArena);
	ftd__symTableInit(&ctx->parseSyms, &ctx->parseArena);
	ctx->diagHead = NULL;
	ctx->diagTail = NULL;
	ctx->errorCount = 0;
	ctx->warningCount = 0;
	ctx->diagCount = 0;
	ctx->diagArray = NULL;
	ctx->currentSourceName = NULL;
	memset(&ctx->lastResult, 0, sizeof(ctx->lastResult));
}

// -----------------------------------------------------------------------------
// Registration
// -----------------------------------------------------------------------------
FTD_API void ftdRegisterStruct(ftdContext *ctx, const ftdType *type) {
	if (ctx == NULL || type == NULL || type->name == NULL) {
		return;
	}
	const char *name = ftd__arenaStrDup(&ctx->schemaArena, type->name);
	uint32_t h = ftd__fnv1aStr(name);
	ftd__Symbol *s = ftd__symTableInsert(&ctx->schemaSyms, name, h);
	if (s == NULL) {
		return;
	}
	s->kind = ftd__Sym_Type;
	s->type = type;
}

FTD_API void ftdRegisterEnum(ftdContext *ctx, const ftdType *type) {
	if (ctx == NULL || type == NULL || type->name == NULL) {
		return;
	}
	const char *name = ftd__arenaStrDup(&ctx->schemaArena, type->name);
	uint32_t h = ftd__fnv1aStr(name);
	ftd__Symbol *s = ftd__symTableInsert(&ctx->schemaSyms, name, h);
	if (s == NULL) {
		return;
	}
	s->kind = ftd__Sym_Enum;
	s->type = type;

	// Also register `EnumName.Value` fully-qualified for each enum value
	for (uint32_t i = 0; i < type->enumValueCount; ++i) {
		const ftdEnumValue *ev = &type->enumValues[i];
		size_t total = strlen(type->name) + 1 + strlen(ev->name) + 1;
		char *qn = (char *)ftd__arenaAlloc(&ctx->schemaArena, total, 1);
		if (qn == NULL) {
			continue;
		}
		// build "EnumName.Value"
		strcpy(qn, type->name);
		strcat(qn, ".");
		strcat(qn, ev->name);
		uint32_t qh = ftd__fnv1aStr(qn);
		ftd__Symbol *qs = ftd__symTableInsert(&ctx->schemaSyms, qn, qh);
		if (qs != NULL) {
			qs->kind = ftd__Sym_Const;
			qs->type = type;
			qs->scalarKind = ftdFieldKind_Enum;
			// Store enum integer value as a pointer to int32 in schema arena
			int32_t *iv = (int32_t *)ftd__arenaAlloc(&ctx->schemaArena, sizeof(int32_t), 4);
			if (iv != NULL) {
				*iv = ev->value;
				qs->as.constPtr = iv;
			}
		}
	}
}

FTD_API void ftdRegisterAlias(ftdContext *ctx, const char *alias, const char *target) {
	if (ctx == NULL || alias == NULL || target == NULL) {
		return;
	}
	const char *name = ftd__arenaStrDup(&ctx->schemaArena, alias);
	const char *tgt  = ftd__arenaStrDup(&ctx->schemaArena, target);
	uint32_t h = ftd__fnv1aStr(name);
	ftd__Symbol *s = ftd__symTableInsert(&ctx->schemaSyms, name, h);
	if (s == NULL) {
		return;
	}
	s->kind = ftd__Sym_Alias;
	s->as.aliasTarget = tgt;
}

FTD_API void ftdRegisterGlobal(ftdContext *ctx, const char *dottedName, const ftdType *type, const void *value) {
	if (ctx == NULL || dottedName == NULL) {
		return;
	}
	const char *name = ftd__arenaStrDup(&ctx->schemaArena, dottedName);
	uint32_t h = ftd__fnv1aStr(name);
	ftd__Symbol *s = ftd__symTableInsert(&ctx->schemaSyms, name, h);
	if (s == NULL) {
		return;
	}
	s->kind = ftd__Sym_Global;
	s->type = type;
	s->as.constPtr = value;
}

FTD_API void ftdRegisterHelper(ftdContext *ctx, const char *name, const ftdType *returnType, ftdHelperFn fn, void *userData) {
	if (ctx == NULL || name == NULL || fn == NULL) {
		return;
	}
	const char *nm = ftd__arenaStrDup(&ctx->schemaArena, name);
	uint32_t h = ftd__fnv1aStr(nm);
	ftd__Symbol *s = ftd__symTableInsert(&ctx->schemaSyms, nm, h);
	if (s == NULL) {
		return;
	}
	s->kind = ftd__Sym_Helper;
	s->type = returnType;
	s->as.helper.fn = fn;
	s->as.helper.userData = userData;
	s->as.helper.returnType = returnType;
}

FTD_API void ftdSetArraySlot(ftdContext *ctx,
                             const ftdType *ownerType,
                             const char    *fieldName,
                             uint32_t       dataOffset,
                             uint32_t       countOffset,
                             uint32_t       countSize) {
	if (ctx == NULL || ownerType == NULL || fieldName == NULL) {
		return;
	}
	ftd__ArraySlot *sl = (ftd__ArraySlot *)ftd__arenaAlloc(&ctx->schemaArena, sizeof(ftd__ArraySlot), 8);
	if (sl == NULL) {
		return;
	}
	sl->ownerType = ownerType;
	sl->fieldName = ftd__arenaStrDup(&ctx->schemaArena, fieldName);
	sl->dataOffset = dataOffset;
	sl->countOffset = countOffset;
	sl->countSize = countSize;
	sl->next = ctx->arraySlots;
	ctx->arraySlots = sl;
}

static const ftd__ArraySlot *ftd__findArraySlot(const ftdContext *ctx, const ftdType *owner, const char *fieldName) {
	const ftd__ArraySlot *cur = ctx->arraySlots;
	while (cur != NULL) {
		if (cur->ownerType == owner && strcmp(cur->fieldName, fieldName) == 0) {
			return cur;
		}
		cur = cur->next;
	}
	return NULL;
}

// -----------------------------------------------------------------------------
// Tokenizer
// -----------------------------------------------------------------------------
typedef enum ftd__TokKind {
	ftd__Tok_EOF = 0,
	ftd__Tok_Newline,
	ftd__Tok_Ident,
	ftd__Tok_Int,
	ftd__Tok_Float,
	ftd__Tok_String,
	ftd__Tok_Char,
	ftd__Tok_Bool,
	ftd__Tok_Null,
	ftd__Tok_LBrace, ftd__Tok_RBrace,
	ftd__Tok_LBracket, ftd__Tok_RBracket,
	ftd__Tok_LParen, ftd__Tok_RParen,
	ftd__Tok_Eq,
	ftd__Tok_Comma,
	ftd__Tok_Dot,
	ftd__Tok_KwAlias,
	ftd__Tok_Error,
} ftd__TokKind;

typedef struct ftd__Token {
	ftd__TokKind   kind;
	ftdSourceSpan  span;
	// payload (kind-dependent)
	bool           b;
	int64_t        i;
	double         f;
	const char    *str;       // for Ident/String (NUL-terminated, arena-owned for String)
	size_t         strLen;
} ftd__Token;

typedef struct ftd__Lexer {
	const char *src;
	const char *cur;     // src + pos; mirrors `pos` for easier debugging (always in sync)
	size_t      length;
	size_t      pos;
	uint32_t    line;
	uint32_t    col;
	ftdContext *ctx;
} ftd__Lexer;

// -----------------------------------------------------------------------------
// Diagnostic emission
// -----------------------------------------------------------------------------
static const char *ftd__formatMsg(ftdContext *ctx, const char *fmt, va_list ap) {
	va_list aq;
	va_copy(aq, ap);
	int needed = vsnprintf(NULL, 0, fmt, aq);
	va_end(aq);
	if (needed < 0) {
		return ftd__arenaStrDup(&ctx->parseArena, "(format error)");
	}
	char *buf = (char *)ftd__arenaAlloc(&ctx->parseArena, (size_t)needed + 1, 1);
	if (buf == NULL) {
		return "";
	}
	vsnprintf(buf, (size_t)needed + 1, fmt, ap);
	return buf;
}

static void ftd__emit(ftdContext *ctx, ftdSeverity sev, ftdSourceSpan span, const char *fmt, ...) {
	ftd__DiagNode *n = (ftd__DiagNode *)ftd__arenaAlloc(&ctx->parseArena, sizeof(ftd__DiagNode), 8);
	if (n == NULL) {
		return;
	}
	va_list ap;
	va_start(ap, fmt);
	const char *msg = ftd__formatMsg(ctx, fmt, ap);
	va_end(ap);
	n->d.severity = sev;
	n->d.span = span;
	n->d.message = msg;
	n->next = NULL;
	if (ctx->diagHead == NULL) {
		ctx->diagHead = n;
	} else {
		ctx->diagTail->next = n;
	}
	ctx->diagTail = n;
	ctx->diagCount++;
	if (sev == ftdSeverity_Error) {
		ctx->errorCount++;
	} else if (sev == ftdSeverity_Warning) {
		ctx->warningCount++;
	}
}

// -----------------------------------------------------------------------------
// Lexer functions
// -----------------------------------------------------------------------------
static void ftd__lexerInit(ftd__Lexer *lex, ftdContext *ctx, const char *src, size_t len) {
	lex->src = src;
	lex->length = len;
	lex->pos = 0;
	lex->line = 1;
	lex->col = 1;
	lex->ctx = ctx;
	// Skip BOM
	if (len >= 3 && (uint8_t)src[0] == 0xEF && (uint8_t)src[1] == 0xBB && (uint8_t)src[2] == 0xBF) {
		lex->pos = 3;
	}
	lex->cur = src + lex->pos;
}

static int ftd__peekC(ftd__Lexer *lex, size_t off) {
	if (lex->pos + off >= lex->length) {
		return -1;
	}
	return (uint8_t)lex->src[lex->pos + off];
}

static int ftd__nextC(ftd__Lexer *lex) {
	if (lex->pos >= lex->length) {
		lex->cur = lex->src + lex->length;
		return -1;
	}
	int c = (uint8_t)lex->src[lex->pos++];
	lex->cur = lex->src + lex->pos;
	if (c == '\n') {
		lex->line++;
		lex->col = 1;
	} else {
		lex->col++;
	}
	return c;
}

static ftdSourceSpan ftd__makeSpan(ftd__Lexer *lex, size_t startPos, uint32_t startLine, uint32_t startCol) {
	ftdSourceSpan sp = { 0 };
	sp.file = lex->ctx->currentSourceName;
	sp.line = startLine;
	sp.column = startCol;
	sp.length = (uint32_t)(lex->pos - startPos);
	return sp;
}

static bool ftd__isIdentStart(int c) {
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
}
static bool ftd__isIdentCont(int c) {
	return ftd__isIdentStart(c) || (c >= '0' && c <= '9');
}

static bool ftd__skipTrivia(ftd__Lexer *lex, bool *sawNewline) {
	*sawNewline = false;
	for (;;) {
		int c = ftd__peekC(lex, 0);
		if (c < 0) {
			return false;
		}
		if (c == ' ' || c == '\t' || c == '\r') {
			ftd__nextC(lex);
			continue;
		}
		if (c == '\n') {
			*sawNewline = true;
			ftd__nextC(lex);
			continue;
		}
		if (c == '/' && ftd__peekC(lex, 1) == '/') {
			while (lex->pos < lex->length && lex->src[lex->pos] != '\n') {
				ftd__nextC(lex);
			}
			continue;
		}
		if (c == '#') {
			while (lex->pos < lex->length && lex->src[lex->pos] != '\n') {
				ftd__nextC(lex);
			}
			continue;
		}
		if (c == '/' && ftd__peekC(lex, 1) == '*') {
			ftd__nextC(lex); ftd__nextC(lex);
			while (lex->pos < lex->length) {
				if (lex->src[lex->pos] == '*' && (lex->pos + 1 < lex->length) && lex->src[lex->pos + 1] == '/') {
					ftd__nextC(lex);
					ftd__nextC(lex);
					break;
				}
				ftd__nextC(lex);
			}
			continue;
		}
		return true;
	}
}

static int ftd__hexDigit(int c) {
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -1;
}

static void ftd__readNumber(ftd__Lexer *lex, ftd__Token *tok, bool negative) {
	size_t startPos = lex->pos;
	uint32_t startLine = lex->line;
	uint32_t startCol = lex->col;
	bool isFloat = false;
	int base = 10;

	if (negative) {
		// minus was consumed separately
		startPos -= 1;
		startCol -= 1;
	}

	// detect base prefix
	if (lex->src[lex->pos] == '0' && (lex->pos + 1) < lex->length) {
		int n = (uint8_t)lex->src[lex->pos + 1];
		if (n == 'x' || n == 'X') {
			base = 16;
			ftd__nextC(lex); ftd__nextC(lex);
		} else if (n == 'b' || n == 'B') {
			base = 2;
			ftd__nextC(lex); ftd__nextC(lex);
		} else if (n == 'o' || n == 'O') {
			base = 8;
			ftd__nextC(lex); ftd__nextC(lex);
		}
	}

	uint64_t intVal = 0;
	while (lex->pos < lex->length) {
		int c = (uint8_t)lex->src[lex->pos];
		if (base == 10 && c >= '0' && c <= '9') {
			intVal = intVal * 10 + (uint64_t)(c - '0');
			ftd__nextC(lex);
		} else if (base == 16) {
			int d = ftd__hexDigit(c);
			if (d < 0) break;
			intVal = intVal * 16 + (uint64_t)d;
			ftd__nextC(lex);
		} else if (base == 2 && (c == '0' || c == '1')) {
			intVal = intVal * 2 + (uint64_t)(c - '0');
			ftd__nextC(lex);
		} else if (base == 8 && c >= '0' && c <= '7') {
			intVal = intVal * 8 + (uint64_t)(c - '0');
			ftd__nextC(lex);
		} else {
			break;
		}
	}

	// fractional / exponent (only for base 10)
	if (base == 10) {
		if (lex->pos < lex->length && lex->src[lex->pos] == '.' &&
		    (lex->pos + 1 < lex->length) && (uint8_t)lex->src[lex->pos + 1] >= '0' && (uint8_t)lex->src[lex->pos + 1] <= '9') {
			isFloat = true;
			ftd__nextC(lex);
			while (lex->pos < lex->length && (uint8_t)lex->src[lex->pos] >= '0' && (uint8_t)lex->src[lex->pos] <= '9') {
				ftd__nextC(lex);
			}
		}
		if (lex->pos < lex->length && (lex->src[lex->pos] == 'e' || lex->src[lex->pos] == 'E')) {
			isFloat = true;
			ftd__nextC(lex);
			if (lex->pos < lex->length && (lex->src[lex->pos] == '+' || lex->src[lex->pos] == '-')) {
				ftd__nextC(lex);
			}
			while (lex->pos < lex->length && (uint8_t)lex->src[lex->pos] >= '0' && (uint8_t)lex->src[lex->pos] <= '9') {
				ftd__nextC(lex);
			}
		}
	}

	// suffix
	size_t suffixStart = lex->pos;
	while (lex->pos < lex->length && ftd__isIdentCont((uint8_t)lex->src[lex->pos])) {
		ftd__nextC(lex);
	}
	size_t suffixLen = lex->pos - suffixStart;
	// 'f' or 'd' suffix forces float
	if (suffixLen >= 1) {
		char first = lex->src[suffixStart];
		if (first == 'f' || first == 'd' || first == 'F' || first == 'D') {
			if (suffixLen == 1) {
				isFloat = true;
			}
		}
	}

	tok->span = ftd__makeSpan(lex, startPos, startLine, startCol);
	if (isFloat) {
		// parse via strtod over the original lexeme (minus suffix);
		// the lexeme already includes the sign when negative=true
		size_t numStart = startPos;
		size_t numEnd   = suffixStart;
		char small[64];
		size_t n = numEnd - numStart;
		if (n >= sizeof(small)) {
			n = sizeof(small) - 1;
		}
		memcpy(small, lex->src + numStart, n);
		small[n] = '\0';
		double d = strtod(small, NULL);
		tok->kind = ftd__Tok_Float;
		tok->f = d;
	} else {
		tok->kind = ftd__Tok_Int;
		tok->i = negative ? -(int64_t)intVal : (int64_t)intVal;
	}
}

static char *ftd__readString(ftd__Lexer *lex, size_t *outLen) {
	// Already at first character after opening quote (we'll handle here)
	// Builds an arena-owned string. Handles C-style adjacency.
	ftd__Arena *arena = &lex->ctx->parseArena;
	// Use simple growth strategy via temporary expansion.
	char *buf = NULL;
	size_t cap = 0;
	size_t len = 0;
	for (;;) {
		// expect opening quote
		if (lex->pos >= lex->length || lex->src[lex->pos] != '"') {
			break;
		}
		ftd__nextC(lex); // consume opening "
		while (lex->pos < lex->length) {
			int c = (uint8_t)lex->src[lex->pos];
			if (c == '"') {
				ftd__nextC(lex);
				break;
			}
			if (c == '\\' && lex->pos + 1 < lex->length) {
				int n = (uint8_t)lex->src[lex->pos + 1];
				int out = -1;
				size_t consumed = 2;
				switch (n) {
					case 'n':  out = '\n'; break;
					case 'r':  out = '\r'; break;
					case 't':  out = '\t'; break;
					case '\\': out = '\\'; break;
					case '"':  out = '"';  break;
					case '\'': out = '\''; break;
					case '0':  out = 0;    break;
					case 'x': {
						if (lex->pos + 3 < lex->length) {
							int h1 = ftd__hexDigit((uint8_t)lex->src[lex->pos + 2]);
							int h2 = ftd__hexDigit((uint8_t)lex->src[lex->pos + 3]);
							if (h1 >= 0 && h2 >= 0) {
								out = (h1 << 4) | h2;
								consumed = 4;
							}
						}
					} break;
					default: out = n; break;
				}
				if (out >= 0) {
					if (len + 1 >= cap) {
						size_t ncap = cap ? cap * 2 : 32;
						char *nb = (char *)ftd__arenaAlloc(arena, ncap, 1);
						if (nb == NULL) {
							break;
						}
						if (buf) {
							memcpy(nb, buf, len);
						}
						buf = nb;
						cap = ncap;
					}
					buf[len++] = (char)out;
				}
				for (size_t k = 0; k < consumed; ++k) {
					ftd__nextC(lex);
				}
				continue;
			}
			if (len + 1 >= cap) {
				size_t ncap = cap ? cap * 2 : 32;
				char *nb = (char *)ftd__arenaAlloc(arena, ncap, 1);
				if (nb == NULL) {
					break;
				}
				if (buf) {
					memcpy(nb, buf, len);
				}
				buf = nb;
				cap = ncap;
			}
			buf[len++] = (char)c;
			ftd__nextC(lex);
		}
		// Adjacent string?
		size_t save = lex->pos;
		uint32_t saveLine = lex->line;
		uint32_t saveCol  = lex->col;
		bool sawNL = false;
		ftd__skipTrivia(lex, &sawNL);
		if (lex->pos < lex->length && lex->src[lex->pos] == '"') {
			// continue concatenation
			continue;
		}
		// restore
		lex->pos = save;
		lex->cur = lex->src + save;
		lex->line = saveLine;
		lex->col = saveCol;
		break;
	}
	if (buf == NULL) {
		buf = (char *)ftd__arenaAlloc(arena, 1, 1);
		if (buf) {
			buf[0] = '\0';
		}
		if (outLen) {
			*outLen = 0;
		}
		return buf;
	}
	// Ensure NUL-terminated
	if (len >= cap) {
		// grow once more
		size_t ncap = cap + 1;
		char *nb = (char *)ftd__arenaAlloc(arena, ncap, 1);
		if (nb) {
			memcpy(nb, buf, len);
			buf = nb;
			cap = ncap;
		}
	}
	buf[len] = '\0';
	if (outLen) {
		*outLen = len;
	}
	return buf;
}

static void ftd__lexNext(ftd__Lexer *lex, ftd__Token *tok) {
	memset(tok, 0, sizeof(*tok));
	bool sawNL = false;
	bool hasMore = ftd__skipTrivia(lex, &sawNL);
	if (sawNL) {
		tok->kind = ftd__Tok_Newline;
		tok->span.file = lex->ctx->currentSourceName;
		tok->span.line = lex->line;
		tok->span.column = lex->col;
		tok->span.length = 1;
		return;
	}
	if (!hasMore) {
		tok->kind = ftd__Tok_EOF;
		tok->span.file = lex->ctx->currentSourceName;
		tok->span.line = lex->line;
		tok->span.column = lex->col;
		return;
	}

	size_t startPos = lex->pos;
	uint32_t startLine = lex->line;
	uint32_t startCol  = lex->col;

	int c = (uint8_t)lex->src[lex->pos];

	if (c == '{') { ftd__nextC(lex); tok->kind = ftd__Tok_LBrace;    tok->span = ftd__makeSpan(lex, startPos, startLine, startCol); return; }
	if (c == '}') { ftd__nextC(lex); tok->kind = ftd__Tok_RBrace;    tok->span = ftd__makeSpan(lex, startPos, startLine, startCol); return; }
	if (c == '[') { ftd__nextC(lex); tok->kind = ftd__Tok_LBracket;  tok->span = ftd__makeSpan(lex, startPos, startLine, startCol); return; }
	if (c == ']') { ftd__nextC(lex); tok->kind = ftd__Tok_RBracket;  tok->span = ftd__makeSpan(lex, startPos, startLine, startCol); return; }
	if (c == '(') { ftd__nextC(lex); tok->kind = ftd__Tok_LParen;    tok->span = ftd__makeSpan(lex, startPos, startLine, startCol); return; }
	if (c == ')') { ftd__nextC(lex); tok->kind = ftd__Tok_RParen;    tok->span = ftd__makeSpan(lex, startPos, startLine, startCol); return; }
	if (c == '=') { ftd__nextC(lex); tok->kind = ftd__Tok_Eq;        tok->span = ftd__makeSpan(lex, startPos, startLine, startCol); return; }
	if (c == ',') { ftd__nextC(lex); tok->kind = ftd__Tok_Comma;     tok->span = ftd__makeSpan(lex, startPos, startLine, startCol); return; }
	if (c == '.') { ftd__nextC(lex); tok->kind = ftd__Tok_Dot;       tok->span = ftd__makeSpan(lex, startPos, startLine, startCol); return; }

	if (c == '"') {
		size_t slen = 0;
		char *s = ftd__readString(lex, &slen);
		tok->kind = ftd__Tok_String;
		tok->str = s;
		tok->strLen = slen;
		tok->span = ftd__makeSpan(lex, startPos, startLine, startCol);
		return;
	}

	if (c == '\'') {
		// char literal -> integer
		ftd__nextC(lex);
		int val = 0;
		if (lex->pos < lex->length) {
			int ch = (uint8_t)lex->src[lex->pos];
			if (ch == '\\' && lex->pos + 1 < lex->length) {
				int n = (uint8_t)lex->src[lex->pos + 1];
				switch (n) {
					case 'n':  val = '\n'; break;
					case 'r':  val = '\r'; break;
					case 't':  val = '\t'; break;
					case '\\': val = '\\'; break;
					case '\'': val = '\''; break;
					case '"':  val = '"';  break;
					case '0':  val = 0;    break;
					default:   val = n;    break;
				}
				ftd__nextC(lex);
				ftd__nextC(lex);
			} else {
				val = ch;
				ftd__nextC(lex);
			}
		}
		if (lex->pos < lex->length && lex->src[lex->pos] == '\'') {
			ftd__nextC(lex);
		}
		tok->kind = ftd__Tok_Int;
		tok->i = val;
		tok->span = ftd__makeSpan(lex, startPos, startLine, startCol);
		return;
	}

	if (c == '-' || c == '+') {
		// signed number
		int nxt = ftd__peekC(lex, 1);
		if (nxt >= '0' && nxt <= '9') {
			bool neg = (c == '-');
			ftd__nextC(lex); // consume sign
			ftd__readNumber(lex, tok, neg);
			return;
		}
	}

	if (c >= '0' && c <= '9') {
		ftd__readNumber(lex, tok, false);
		return;
	}

	if (ftd__isIdentStart(c)) {
		size_t idStart = lex->pos;
		while (lex->pos < lex->length && ftd__isIdentCont((uint8_t)lex->src[lex->pos])) {
			ftd__nextC(lex);
		}
		size_t idLen = lex->pos - idStart;
		const char *ptr = lex->src + idStart;

		// Reserved words
		if (idLen == 4 && memcmp(ptr, "true", 4) == 0) {
			tok->kind = ftd__Tok_Bool; tok->b = true;
		} else if (idLen == 5 && memcmp(ptr, "false", 5) == 0) {
			tok->kind = ftd__Tok_Bool; tok->b = false;
		} else if (idLen == 4 && memcmp(ptr, "null", 4) == 0) {
			tok->kind = ftd__Tok_Null;
		} else if (idLen == 5 && memcmp(ptr, "alias", 5) == 0) {
			tok->kind = ftd__Tok_KwAlias;
		} else {
			tok->kind = ftd__Tok_Ident;
			tok->str = ftd__arenaStrDupN(&lex->ctx->parseArena, ptr, idLen);
			tok->strLen = idLen;
		}
		tok->span = ftd__makeSpan(lex, startPos, startLine, startCol);
		return;
	}

	// Unknown character
	ftd__nextC(lex);
	tok->kind = ftd__Tok_Error;
	tok->span = ftd__makeSpan(lex, startPos, startLine, startCol);
}

// -----------------------------------------------------------------------------
// Parser
// -----------------------------------------------------------------------------
typedef struct ftd__Fixup {
	void        **slot;        // *slot = (void*)resolved
	const char   *name;        // identifier to resolve
	ftdSourceSpan span;
	const ftdType *expectedType;
	struct ftd__Fixup *next;
} ftd__Fixup;

typedef struct ftd__Parser {
	ftdContext  *ctx;
	ftd__Lexer   lex;
	ftd__Token   cur;
	ftd__Token   peek;
	bool         hasPeek;
	ftd__Fixup  *fixupHead;
} ftd__Parser;

static void ftd__parserAdvance(ftd__Parser *p) {
	if (p->hasPeek) {
		p->cur = p->peek;
		p->hasPeek = false;
	} else {
		ftd__lexNext(&p->lex, &p->cur);
	}
}

static const ftd__Token *ftd__parserPeek(ftd__Parser *p) {
	if (!p->hasPeek) {
		ftd__lexNext(&p->lex, &p->peek);
		p->hasPeek = true;
	}
	return &p->peek;
}

static void ftd__skipNewlines(ftd__Parser *p) {
	while (p->cur.kind == ftd__Tok_Newline) {
		ftd__parserAdvance(p);
	}
}

static bool ftd__match(ftd__Parser *p, ftd__TokKind k) {
	if (p->cur.kind == k) {
		ftd__parserAdvance(p);
		return true;
	}
	return false;
}

// Resolve an alias chain to a final symbol.
static const ftd__Symbol *ftd__resolveAlias(ftdContext *ctx, const char *name, int depth) {
	if (depth > 16) {
		return NULL;
	}
	const ftd__Symbol *s = ftd__symTableLookup(&ctx->parseSyms, name);
	if (s == NULL) {
		s = ftd__symTableLookup(&ctx->schemaSyms, name);
	}
	if (s == NULL) {
		return NULL;
	}
	if (s->kind == ftd__Sym_Alias) {
		return ftd__resolveAlias(ctx, s->as.aliasTarget, depth + 1);
	}
	return s;
}

// Lookup an enum value name (short or qualified) given a known enum type.
static bool ftd__lookupEnumValue(const ftdType *enumType, const char *name, int32_t *outValue) {
	if (enumType == NULL || enumType->enumValues == NULL) {
		return false;
	}
	for (uint32_t i = 0; i < enumType->enumValueCount; ++i) {
		if (strcmp(enumType->enumValues[i].name, name) == 0) {
			*outValue = enumType->enumValues[i].value;
			return true;
		}
	}
	return false;
}

// Find a field in a struct's field table by name. Returns NULL if not found.
static const ftdField *ftd__findField(const ftdType *t, const char *name) {
	if (t == NULL || t->fields == NULL) {
		return NULL;
	}
	for (uint32_t i = 0; i < t->fieldCount; ++i) {
		if (strcmp(t->fields[i].name, name) == 0) {
			return &t->fields[i];
		}
	}
	return NULL;
}

// -----------------------------------------------------------------------------
// Coercion writes
// -----------------------------------------------------------------------------
static size_t ftd__scalarSize(ftdFieldKind k) {
	switch (k) {
		case ftdFieldKind_Bool: return sizeof(bool);
		case ftdFieldKind_S8:   return 1;
		case ftdFieldKind_S16:  return 2;
		case ftdFieldKind_S32:  return 4;
		case ftdFieldKind_S64:  return 8;
		case ftdFieldKind_U8:   return 1;
		case ftdFieldKind_U16:  return 2;
		case ftdFieldKind_U32:  return 4;
		case ftdFieldKind_U64:  return 8;
		case ftdFieldKind_F32:  return 4;
		case ftdFieldKind_F64:  return 8;
		case ftdFieldKind_String: return sizeof(const char *);
		case ftdFieldKind_Enum:   return 4;
		default: return 0;
	}
}

static void ftd__writeScalar(void *out, ftdFieldKind kind, const ftdValue *v) {
	switch (kind) {
		case ftdFieldKind_Bool: {
			bool bv = false;
			if (v->kind == ftdValueKind_Bool) bv = v->as.b;
			else if (v->kind == ftdValueKind_Int) bv = v->as.i != 0;
			else if (v->kind == ftdValueKind_UInt) bv = v->as.u != 0;
			else if (v->kind == ftdValueKind_Float) bv = v->as.f != 0.0;
			*(bool *)out = bv;
		} break;
		case ftdFieldKind_S8:  { int8_t  x = (int8_t)(v->kind == ftdValueKind_Float ? (int64_t)v->as.f : v->as.i); *(int8_t *)out = x; } break;
		case ftdFieldKind_S16: { int16_t x = (int16_t)(v->kind == ftdValueKind_Float ? (int64_t)v->as.f : v->as.i); *(int16_t *)out = x; } break;
		case ftdFieldKind_S32: {
			int32_t x;
			if (v->kind == ftdValueKind_Float) x = (int32_t)v->as.f;
			else if (v->kind == ftdValueKind_UInt) x = (int32_t)v->as.u;
			else x = (int32_t)v->as.i;
			*(int32_t *)out = x;
		} break;
		case ftdFieldKind_S64: {
			int64_t x;
			if (v->kind == ftdValueKind_Float) x = (int64_t)v->as.f;
			else if (v->kind == ftdValueKind_UInt) x = (int64_t)v->as.u;
			else x = v->as.i;
			*(int64_t *)out = x;
		} break;
		case ftdFieldKind_U8:  { uint8_t  x = (uint8_t)(v->kind == ftdValueKind_Float ? (uint64_t)v->as.f : (v->kind == ftdValueKind_Int ? (uint64_t)v->as.i : v->as.u)); *(uint8_t *)out = x; } break;
		case ftdFieldKind_U16: { uint16_t x = (uint16_t)(v->kind == ftdValueKind_Float ? (uint64_t)v->as.f : (v->kind == ftdValueKind_Int ? (uint64_t)v->as.i : v->as.u)); *(uint16_t *)out = x; } break;
		case ftdFieldKind_U32: { uint32_t x = (uint32_t)(v->kind == ftdValueKind_Float ? (uint64_t)v->as.f : (v->kind == ftdValueKind_Int ? (uint64_t)v->as.i : v->as.u)); *(uint32_t *)out = x; } break;
		case ftdFieldKind_U64: { uint64_t x = (v->kind == ftdValueKind_Float ? (uint64_t)v->as.f : (v->kind == ftdValueKind_Int ? (uint64_t)v->as.i : v->as.u)); *(uint64_t *)out = x; } break;
		case ftdFieldKind_F32: {
			float x;
			if (v->kind == ftdValueKind_Float) x = (float)v->as.f;
			else if (v->kind == ftdValueKind_UInt) x = (float)v->as.u;
			else x = (float)v->as.i;
			*(float *)out = x;
		} break;
		case ftdFieldKind_F64: {
			double x;
			if (v->kind == ftdValueKind_Float) x = v->as.f;
			else if (v->kind == ftdValueKind_UInt) x = (double)v->as.u;
			else x = (double)v->as.i;
			*(double *)out = x;
		} break;
		case ftdFieldKind_String: {
			const char *s = (v->kind == ftdValueKind_String) ? v->as.str : NULL;
			*(const char **)out = s;
		} break;
		case ftdFieldKind_Enum: {
			int32_t x = (int32_t)v->as.i;
			*(int32_t *)out = x;
		} break;
		default: break;
	}
}

// -----------------------------------------------------------------------------
// Parsing forward declarations
// -----------------------------------------------------------------------------
typedef struct ftd__ArrayBundle {
	const ftdField *data;     // required (ftdFieldKind_ArrayData, subtype = elemType)
	const ftdField *count;    // optional (ftdFieldKind_ArrayCount)
	const ftdField *capacity; // optional (ftdFieldKind_ArrayCapacity)
	const ftdType  *elemType;
} ftd__ArrayBundle;

static bool ftd__parseValue(ftd__Parser *p, const ftdField *fieldHint, const ftdType *typeHint, ftdValue *out);
static bool ftd__parseBlockInto(ftd__Parser *p, const ftdType *type, void *outStruct);
static bool ftd__parseArrayInto(ftd__Parser *p, const ftdField *arrayField, void *ownerStruct);
static bool ftd__parseArrayBundle(ftd__Parser *p, const ftd__ArrayBundle *bundle, void *ownerStruct);
static bool ftd__findArrayBundle(const ftdType *t, const char *name, ftd__ArrayBundle *out);
static void ftd__skipToNewlineOrEOF(ftd__Parser *p);
static void ftd__skipBalanced(ftd__Parser *p);

// Read a dotted qualified name (path). Returns arena-owned NUL-terminated string.
static const char *ftd__parseQName(ftd__Parser *p) {
	if (p->cur.kind != ftd__Tok_Ident) {
		return NULL;
	}
	// Concatenate "Ident('.' Ident)*"
	char *buf = NULL;
	size_t cap = 0;
	size_t len = 0;

	#define APP(src, slen) do { \
		size_t nlen = len + (slen); \
		if (nlen + 1 > cap) { \
			size_t ncap = cap ? cap : 32; \
			while (ncap < nlen + 1) ncap *= 2; \
			char *nb = (char *)ftd__arenaAlloc(&p->ctx->parseArena, ncap, 1); \
			if (nb == NULL) { return NULL; } \
			if (buf) memcpy(nb, buf, len); \
			buf = nb; cap = ncap; \
		} \
		memcpy(buf + len, (src), (slen)); \
		len = nlen; \
		buf[len] = '\0'; \
	} while (0)

	APP(p->cur.str, p->cur.strLen);
	ftd__parserAdvance(p);
	for (;;) {
		if (p->cur.kind != ftd__Tok_Dot) {
			break;
		}
		const ftd__Token *pk = ftd__parserPeek(p);
		if (pk->kind != ftd__Tok_Ident) {
			break;
		}
		ftd__parserAdvance(p); // .
		APP(".", 1);
		APP(p->cur.str, p->cur.strLen);
		ftd__parserAdvance(p);
	}
	#undef APP
	return buf;
}

// Skip a value form (used after unknown field) — balanced.
static void ftd__skipBalanced(ftd__Parser *p) {
	int braceDepth = 0;
	int parenDepth = 0;
	int bracketDepth = 0;
	bool started = false;
	for (;;) {
		ftd__TokKind k = p->cur.kind;
		if (k == ftd__Tok_EOF) {
			return;
		}
		if (k == ftd__Tok_LBrace)   { braceDepth++; started = true; }
		else if (k == ftd__Tok_RBrace) { braceDepth--; }
		else if (k == ftd__Tok_LBracket) { bracketDepth++; started = true; }
		else if (k == ftd__Tok_RBracket) { bracketDepth--; }
		else if (k == ftd__Tok_LParen) { parenDepth++; started = true; }
		else if (k == ftd__Tok_RParen) { parenDepth--; }
		else if (k == ftd__Tok_Newline) {
			if (started == false && braceDepth == 0 && parenDepth == 0 && bracketDepth == 0) {
				return;
			}
		}
		ftd__parserAdvance(p);
		if (started && braceDepth == 0 && parenDepth == 0 && bracketDepth == 0) {
			return;
		}
		if (!started && braceDepth == 0 && parenDepth == 0 && bracketDepth == 0 && k == ftd__Tok_Newline) {
			return;
		}
	}
}

static void ftd__skipToNewlineOrEOF(ftd__Parser *p) {
	while (p->cur.kind != ftd__Tok_Newline && p->cur.kind != ftd__Tok_EOF) {
		// also bail on a closing delimiter
		if (p->cur.kind == ftd__Tok_RBrace || p->cur.kind == ftd__Tok_RBracket || p->cur.kind == ftd__Tok_RParen) {
			return;
		}
		ftd__parserAdvance(p);
	}
}

// Schedule a fixup for an unresolved name.
static void ftd__scheduleFixup(ftd__Parser *p, void **slot, const char *name, ftdSourceSpan span, const ftdType *expected) {
	ftd__Fixup *fx = (ftd__Fixup *)ftd__arenaAlloc(&p->ctx->parseArena, sizeof(ftd__Fixup), 8);
	if (fx == NULL) {
		return;
	}
	fx->slot = slot;
	fx->name = ftd__arenaStrDup(&p->ctx->parseArena, name);
	fx->span = span;
	fx->expectedType = expected;
	fx->next = p->fixupHead;
	p->fixupHead = fx;
}

// -----------------------------------------------------------------------------
// Positional value into struct (TypeName(arg, arg, ...))
// -----------------------------------------------------------------------------
static bool ftd__parsePositionalInto(ftd__Parser *p, const ftdType *type, void *outStruct) {
	if (!ftd__match(p, ftd__Tok_LParen)) {
		return false;
	}
	uint32_t argIdx = 0;
	for (;;) {
		ftd__skipNewlines(p);
		if (p->cur.kind == ftd__Tok_RParen) {
			ftd__parserAdvance(p);
			break;
		}
		if (argIdx >= type->fieldCount) {
			// Extra args ignored
			ftdValue dummy;
			ftd__parseValue(p, NULL, NULL, &dummy);
		} else {
			const ftdField *f = &type->fields[argIdx];
			ftdValue val;
			if (ftd__parseValue(p, f, f->subtype, &val)) {
				// write into struct based on field kind
				void *slot = (uint8_t *)outStruct + f->offset;
				if (f->kind >= ftdFieldKind_Bool && f->kind <= ftdFieldKind_Enum) {
					ftd__writeScalar(slot, f->kind, &val);
				} else if (f->kind == ftdFieldKind_Struct && val.kind == ftdValueKind_Struct && val.as.ptr != NULL) {
					if (f->subtype != NULL && f->subtype->size > 0) {
						memcpy(slot, val.as.ptr, f->subtype->size);
					}
				} else if (f->kind == ftdFieldKind_Ref) {
					*(void **)slot = val.as.ptr;
				} else if (f->kind == ftdFieldKind_String && val.kind == ftdValueKind_String) {
					*(const char **)slot = val.as.str;
				}
			}
		}
		argIdx++;
		ftd__skipNewlines(p);
		if (ftd__match(p, ftd__Tok_Comma)) {
			continue;
		}
		if (p->cur.kind == ftd__Tok_RParen) {
			ftd__parserAdvance(p);
			break;
		}
		if (p->cur.kind == ftd__Tok_Newline) {
			continue;
		}
		break;
	}
	return true;
}

// -----------------------------------------------------------------------------
// Helper call: HelperName(args...)
// -----------------------------------------------------------------------------
static bool ftd__callHelper(ftd__Parser *p, const ftd__Symbol *helperSym, ftdValue *outVal) {
	if (!ftd__match(p, ftd__Tok_LParen)) {
		return false;
	}
	ftdValue args[16];
	uint32_t argCount = 0;
	for (;;) {
		ftd__skipNewlines(p);
		if (p->cur.kind == ftd__Tok_RParen) {
			ftd__parserAdvance(p);
			break;
		}
		if (argCount < 16) {
			ftd__parseValue(p, NULL, NULL, &args[argCount++]);
		} else {
			ftdValue dummy;
			ftd__parseValue(p, NULL, NULL, &dummy);
		}
		ftd__skipNewlines(p);
		if (ftd__match(p, ftd__Tok_Comma)) {
			continue;
		}
		if (p->cur.kind == ftd__Tok_RParen) {
			ftd__parserAdvance(p);
			break;
		}
		if (p->cur.kind == ftd__Tok_Newline) {
			continue;
		}
		break;
	}
	const ftdType *ret = helperSym->as.helper.returnType;
	if (ret == NULL || ret->size == 0) {
		outVal->kind = ftdValueKind_None;
		return true;
	}
	void *mem = ftd__arenaAlloc(&p->ctx->parseArena, ret->size, ret->align ? ret->align : 8);
	if (mem == NULL) {
		return false;
	}
	helperSym->as.helper.fn(p->ctx, args, argCount, mem, helperSym->as.helper.userData);
	outVal->kind = ftdValueKind_Struct;
	outVal->type = ret;
	outVal->as.ptr = mem;
	return true;
}

// -----------------------------------------------------------------------------
// Parse a single value
// -----------------------------------------------------------------------------
static bool ftd__parseValue(ftd__Parser *p, const ftdField *fieldHint, const ftdType *typeHint, ftdValue *out) {
	memset(out, 0, sizeof(*out));
	out->span = p->cur.span;

	ftd__TokKind k = p->cur.kind;

	if (k == ftd__Tok_Bool) {
		out->kind = ftdValueKind_Bool;
		out->as.b = p->cur.b;
		ftd__parserAdvance(p);
		return true;
	}
	if (k == ftd__Tok_Null) {
		out->kind = ftdValueKind_None;
		ftd__parserAdvance(p);
		return true;
	}
	if (k == ftd__Tok_Int) {
		out->kind = ftdValueKind_Int;
		out->as.i = p->cur.i;
		ftd__parserAdvance(p);
		return true;
	}
	if (k == ftd__Tok_Float) {
		out->kind = ftdValueKind_Float;
		out->as.f = p->cur.f;
		ftd__parserAdvance(p);
		return true;
	}
	if (k == ftd__Tok_String) {
		out->kind = ftdValueKind_String;
		out->as.str = p->cur.str;
		ftd__parserAdvance(p);
		return true;
	}
	if (k == ftd__Tok_LBrace) {
		// Anonymous struct literal — use typeHint
		if (typeHint == NULL || typeHint->size == 0) {
			// can't construct - skip
			ftd__skipBalanced(p);
			out->kind = ftdValueKind_None;
			return true;
		}
		void *mem = ftd__arenaAlloc(&p->ctx->parseArena, typeHint->size, typeHint->align ? typeHint->align : 8);
		if (mem == NULL) {
			return false;
		}
		ftd__parseBlockInto(p, typeHint, mem);
		out->kind = ftdValueKind_Struct;
		out->type = typeHint;
		out->as.ptr = mem;
		return true;
	}
	if (k == ftd__Tok_LBracket) {
		// Anonymous array literal — needs element type from hint
		ftd__skipBalanced(p);
		out->kind = ftdValueKind_None;
		return true;
	}
	if (k == ftd__Tok_Ident) {
		// Could be: enum value (short), alias, type-call (Type(args)/Type{...}), const/instance/global/helper
		const char *name = ftd__parseQName(p);
		if (name == NULL) {
			out->kind = ftdValueKind_None;
			return false;
		}

		// 1. Enum short form against fieldHint's enum type
		if (fieldHint != NULL && fieldHint->kind == ftdFieldKind_Enum && fieldHint->subtype != NULL) {
			int32_t ev;
			if (ftd__lookupEnumValue(fieldHint->subtype, name, &ev)) {
				out->kind = ftdValueKind_Enum;
				out->type = fieldHint->subtype;
				out->as.i = ev;
				return true;
			}
		}

		// Resolve via alias chain
		const ftd__Symbol *sym = ftd__resolveAlias(p->ctx, name, 0);

		if (sym != NULL) {
			// Helper call?
			if (sym->kind == ftd__Sym_Helper && p->cur.kind == ftd__Tok_LParen) {
				return ftd__callHelper(p, sym, out);
			}
			// Type call?
			if (sym->kind == ftd__Sym_Type && (p->cur.kind == ftd__Tok_LParen || p->cur.kind == ftd__Tok_LBrace)) {
				const ftdType *t = sym->type;
				void *mem = ftd__arenaAlloc(&p->ctx->parseArena, t->size, t->align ? t->align : 8);
				if (mem == NULL) {
					return false;
				}
				if (p->cur.kind == ftd__Tok_LParen) {
					ftd__parsePositionalInto(p, t, mem);
				} else {
					ftd__parseBlockInto(p, t, mem);
				}
				out->kind = ftdValueKind_Struct;
				out->type = t;
				out->as.ptr = mem;
				return true;
			}
			// Const/Global/Instance: dereference or pass as ref
			if (sym->kind == ftd__Sym_Const || sym->kind == ftd__Sym_Global || sym->kind == ftd__Sym_Instance) {
				if (sym->scalarKind != ftdFieldKind_None && sym->scalarKind != ftdFieldKind_Enum) {
					// scalar constant — extract value
					const void *cp = sym->as.constPtr;
					switch (sym->scalarKind) {
						case ftdFieldKind_Bool: out->kind = ftdValueKind_Bool;  out->as.b = *(const bool *)cp; break;
						case ftdFieldKind_S8:   out->kind = ftdValueKind_Int;   out->as.i = *(const int8_t *)cp; break;
						case ftdFieldKind_S16:  out->kind = ftdValueKind_Int;   out->as.i = *(const int16_t *)cp; break;
						case ftdFieldKind_S32:  out->kind = ftdValueKind_Int;   out->as.i = *(const int32_t *)cp; break;
						case ftdFieldKind_S64:  out->kind = ftdValueKind_Int;   out->as.i = *(const int64_t *)cp; break;
						case ftdFieldKind_U8:   out->kind = ftdValueKind_UInt;  out->as.u = *(const uint8_t *)cp; break;
						case ftdFieldKind_U16:  out->kind = ftdValueKind_UInt;  out->as.u = *(const uint16_t *)cp; break;
						case ftdFieldKind_U32:  out->kind = ftdValueKind_UInt;  out->as.u = *(const uint32_t *)cp; break;
						case ftdFieldKind_U64:  out->kind = ftdValueKind_UInt;  out->as.u = *(const uint64_t *)cp; break;
						case ftdFieldKind_F32:  out->kind = ftdValueKind_Float; out->as.f = *(const float *)cp; break;
						case ftdFieldKind_F64:  out->kind = ftdValueKind_Float; out->as.f = *(const double *)cp; break;
						case ftdFieldKind_String: out->kind = ftdValueKind_String; out->as.str = *(const char *const *)cp; break;
						default: break;
					}
					return true;
				}
				if (sym->scalarKind == ftdFieldKind_Enum) {
					out->kind = ftdValueKind_Enum;
					out->type = sym->type;
					out->as.i = *(const int32_t *)sym->as.constPtr;
					return true;
				}
				// struct const/global/instance — pass as ref or struct
				if (fieldHint != NULL && fieldHint->kind == ftdFieldKind_Struct && fieldHint->subtype != NULL && sym->type == fieldHint->subtype) {
					out->kind = ftdValueKind_Struct;
					out->type = sym->type;
					out->as.ptr = (void *)sym->as.constPtr;
					return true;
				}
				out->kind = ftdValueKind_Ref;
				out->type = sym->type;
				out->as.ptr = (void *)sym->as.constPtr;
				return true;
			}
			if (sym->kind == ftd__Sym_Enum) {
				// "EnumName.Value" must be the resolved form; bare "EnumName" alone is not a value.
				out->kind = ftdValueKind_None;
				return true;
			}
		}

		// 5. Enum short form against any in-scope enum (search all enums for the name)
		// (we only do this if not yet matched)
		// — best-effort: skip unless fieldHint already gave a type
		// fallthrough: deferred ref (fixup) if Ref field

		if (fieldHint != NULL && fieldHint->kind == ftdFieldKind_Ref) {
			out->kind = ftdValueKind_Ref;
			out->type = fieldHint->subtype;
			out->as.ptr = NULL;
			// Caller schedules fixup using the name (stored in out via span only)
			// We use the span trick: copy the name into out->as.str position via a side channel.
			// Cleaner: caller checks if Ref returned NULL and schedules fixup with the source name.
			// We need to pass back the name — encode by stashing in a static spot is not safe.
			// Instead, we let the caller schedule the fixup; provide name via a wrapper.
			// Workaround: re-encode the name in span — we cannot. Use scheduleFixup at this site:
			// (no slot available yet)
			// Mark with kind=Ref and ptr=NULL; the field assigner will detect and schedule.
			// We need to relay the name; stash it into the value's `str` shadow.
			out->as.ptr = NULL;
			// We can't store both ptr and str via union; trick: allocate a small fixup-stub
			// and put its address in ptr (caller knows to read).
			char **stub = (char **)ftd__arenaAlloc(&p->ctx->parseArena, sizeof(char *), 8);
			if (stub != NULL) {
				*stub = (char *)name;
				out->as.ptr = stub; // out marker: ptr is a (char**) when value is unresolved-ref
				out->type = (const ftdType *)(uintptr_t)1; // sentinel: unresolved
				out->kind = ftdValueKind_Ref;
				return true;
			}
			return true;
		}

		// 5. Enum short-form against fieldHint? already tried. Try any in-scope enum (rare).
		// 6. Otherwise — error
		ftd__emit(p->ctx, ftdSeverity_Error, out->span, "undefined name '%s'", name);
		out->kind = ftdValueKind_None;
		return true;
	}

	// Unknown
	ftd__emit(p->ctx, ftdSeverity_Error, p->cur.span, "expected value");
	ftd__parserAdvance(p);
	out->kind = ftdValueKind_None;
	return false;
}

// -----------------------------------------------------------------------------
// Field assignment (path = value, path { ... }, path [ ... ])
// -----------------------------------------------------------------------------
static const ftdField *ftd__resolvePath(ftd__Parser *p, const ftdType *baseType, void *baseStruct,
                                        void **outSlotStruct, const ftdType **outSlotType) {
	// p->cur is the first Ident of the path
	const ftdField *field = NULL;
	const ftdType *curType = baseType;
	void *curStruct = baseStruct;

	for (;;) {
		if (p->cur.kind != ftd__Tok_Ident) {
			return NULL;
		}
		const char *segName = p->cur.str;
		field = ftd__findField(curType, segName);
		if (field == NULL) {
			// unknown field on parent path — warn and bail
			ftd__emit(p->ctx, ftdSeverity_Warning, p->cur.span,
				"unknown field '%s' on type '%s' - ignored",
				segName, curType ? curType->name : "?");
			ftd__parserAdvance(p);
			// fast-forward dotted suffix
			while (p->cur.kind == ftd__Tok_Dot) {
				ftd__parserAdvance(p);
				if (p->cur.kind == ftd__Tok_Ident) {
					ftd__parserAdvance(p);
				}
			}
			return NULL;
		}
		ftd__parserAdvance(p); // consume segment

		// Look ahead: if next is Dot+Ident, recurse into nested struct
		if (p->cur.kind == ftd__Tok_Dot) {
			const ftd__Token *pk = ftd__parserPeek(p);
			if (pk->kind == ftd__Tok_Ident) {
				ftd__parserAdvance(p); // dot
				// auto-descend: field must be a nested struct
				if (field->kind == ftdFieldKind_Struct && field->subtype != NULL) {
					curStruct = (uint8_t *)curStruct + field->offset;
					curType = field->subtype;
					continue;
				} else {
					ftd__emit(p->ctx, ftdSeverity_Warning, p->cur.span,
						"dotted path into non-struct field '%s'", segName);
					return NULL;
				}
			}
		}
		break;
	}

	*outSlotStruct = (uint8_t *)curStruct + field->offset;
	*outSlotType = curType;
	return field;
}

static void ftd__writeFieldValue(ftd__Parser *p, const ftdField *field, void *fieldSlot, const ftdValue *v, const ftdType *ownerType, void *ownerStruct) {
	if (field == NULL) {
		return;
	}
	if (field->flags & ftdFieldFlag_Hidden) {
		return;
	}
	if (field->kind >= ftdFieldKind_Bool && field->kind <= ftdFieldKind_Enum) {
		ftd__writeScalar(fieldSlot, field->kind, v);
		return;
	}
	if (field->kind == ftdFieldKind_Struct) {
		if (v->kind == ftdValueKind_Struct && v->as.ptr != NULL && field->subtype != NULL) {
			memcpy(fieldSlot, v->as.ptr, field->subtype->size);
		}
		return;
	}
	if (field->kind == ftdFieldKind_Ref) {
		if (v->kind == ftdValueKind_Ref) {
			if (v->type == (const ftdType *)(uintptr_t)1) {
				// unresolved -> schedule fixup
				char **stub = (char **)v->as.ptr;
				if (stub != NULL) {
					ftd__scheduleFixup(p, (void **)fieldSlot, *stub, v->span, field->subtype);
				}
			} else {
				*(void **)fieldSlot = v->as.ptr;
			}
		} else if (v->kind == ftdValueKind_Struct) {
			*(void **)fieldSlot = v->as.ptr;
		} else if (v->kind == ftdValueKind_None) {
			*(void **)fieldSlot = NULL;
		}
		return;
	}
	if (field->kind == ftdFieldKind_Union) {
		// Determine discriminator value, if any
		if (field->discriminator != NULL && ownerType != NULL) {
			const ftdField *disc = ftd__findField(ownerType, field->discriminator);
			if (disc != NULL && disc->kind == ftdFieldKind_Enum) {
				int32_t discVal = *(const int32_t *)((uint8_t *)ownerStruct + disc->offset);
				if ((uint32_t)discVal != field->unionTag) {
					ftd__emit(p->ctx, ftdSeverity_Warning, v->span,
						"union variant set while discriminator selects a different variant");
					return;
				}
			}
		}
		if (v->kind == ftdValueKind_Struct && v->as.ptr != NULL && field->subtype != NULL) {
			memcpy(fieldSlot, v->as.ptr, field->subtype->size);
		}
		return;
	}
}

// -----------------------------------------------------------------------------
// Block parsing
// -----------------------------------------------------------------------------
static bool ftd__parseBlockInto(ftd__Parser *p, const ftdType *type, void *outStruct) {
	if (!ftd__match(p, ftd__Tok_LBrace)) {
		ftd__emit(p->ctx, ftdSeverity_Error, p->cur.span, "expected '{' for type '%s'", type ? type->name : "?");
		return false;
	}
	for (;;) {
		ftd__skipNewlines(p);
		if (p->cur.kind == ftd__Tok_RBrace) {
			ftd__parserAdvance(p);
			break;
		}
		if (p->cur.kind == ftd__Tok_EOF) {
			ftd__emit(p->ctx, ftdSeverity_Error, p->cur.span, "unexpected EOF inside block");
			break;
		}
		if (p->cur.kind != ftd__Tok_Ident) {
			ftd__emit(p->ctx, ftdSeverity_Error, p->cur.span, "expected field name");
			// Force forward progress: skipToNewlineOrEOF stops on closing
			// delimiters, so a stray ')'/'}'/']' inside the block could loop
			// forever. Step past it.
			ftd__parserAdvance(p);
			ftd__skipToNewlineOrEOF(p);
			continue;
		}

		// Detect array-bundle reference (name where name.data exists in the
		// type's field table). Only triggers when the next token is `=` or `[`
		// so a regular field path like `foo.x = ...` is unaffected.
		{
			const ftd__Token *pk = ftd__parserPeek(p);
			if (pk->kind == ftd__Tok_Eq || pk->kind == ftd__Tok_LBracket) {
				ftd__ArrayBundle bundle;
				if (ftd__findArrayBundle(type, p->cur.str, &bundle)) {
					ftdSourceSpan nameSpan = p->cur.span;
					ftd__parserAdvance(p); // consume name
					if (p->cur.kind == ftd__Tok_Eq) {
						ftd__parserAdvance(p);
					}
					if (p->cur.kind == ftd__Tok_LBracket) {
						ftd__parseArrayBundle(p, &bundle, outStruct);
					} else {
						ftd__emit(p->ctx, ftdSeverity_Warning, nameSpan,
							"expected '[' for array '%.*s'",
							(int)(strchr(bundle.data->name, '.') - bundle.data->name),
							bundle.data->name);
						ftd__skipToNewlineOrEOF(p);
					}
					if (p->cur.kind == ftd__Tok_Comma) {
						ftd__parserAdvance(p);
					}
					ftd__skipNewlines(p);
					continue;
				}
			}
		}

		// resolve path
		void *slot = NULL;
		const ftdType *slotType = NULL;
		const ftdField *field = ftd__resolvePath(p, type, outStruct, &slot, &slotType);
		if (field == NULL) {
			// unknown field — already warned; skip its value
			if (p->cur.kind == ftd__Tok_Eq) {
				ftd__parserAdvance(p);
				ftdValue dummy;
				ftd__parseValue(p, NULL, NULL, &dummy);
			} else if (p->cur.kind == ftd__Tok_LBrace || p->cur.kind == ftd__Tok_LBracket || p->cur.kind == ftd__Tok_LParen) {
				ftd__skipBalanced(p);
			} else {
				ftd__skipToNewlineOrEOF(p);
			}
			continue;
		}

		// dispatch
		if (p->cur.kind == ftd__Tok_Eq) {
			ftd__parserAdvance(p);
			if (field->kind == ftdFieldKind_Array && p->cur.kind == ftd__Tok_LBracket) {
				ftd__parseArrayInto(p, field, outStruct);
			} else {
				ftdValue val;
				ftd__parseValue(p, field, field->subtype, &val);
				ftd__writeFieldValue(p, field, slot, &val, type, outStruct);
			}
		} else if (p->cur.kind == ftd__Tok_LBrace) {
			// nested struct / union variant
			if (field->kind == ftdFieldKind_Struct && field->subtype != NULL) {
				ftd__parseBlockInto(p, field->subtype, slot);
			} else if (field->kind == ftdFieldKind_Union && field->subtype != NULL) {
				// Check discriminator first
				bool active = true;
				if (field->discriminator != NULL) {
					const ftdField *disc = ftd__findField(type, field->discriminator);
					if (disc != NULL && disc->kind == ftdFieldKind_Enum) {
						int32_t discVal = *(const int32_t *)((uint8_t *)outStruct + disc->offset);
						if ((uint32_t)discVal != field->unionTag) {
							active = false;
						}
					}
				}
				if (active) {
					ftd__parseBlockInto(p, field->subtype, slot);
				} else {
					// Skip the block silently — variant doesn't match
					ftd__skipBalanced(p);
				}
			} else {
				ftd__emit(p->ctx, ftdSeverity_Warning, p->cur.span,
					"unexpected '{' for field '%s'", field->name);
				ftd__skipBalanced(p);
			}
		} else if (p->cur.kind == ftd__Tok_LBracket) {
			if (field->kind == ftdFieldKind_Array) {
				ftd__parseArrayInto(p, field, outStruct);
			} else {
				ftd__emit(p->ctx, ftdSeverity_Warning, p->cur.span,
					"array literal for non-array field '%s'", field->name);
				ftd__skipBalanced(p);
			}
		} else {
			ftd__emit(p->ctx, ftdSeverity_Error, p->cur.span,
				"expected '=', '{' or '[' after field '%s'", field->name);
			ftd__skipToNewlineOrEOF(p);
		}

		// statement terminator
		if (p->cur.kind == ftd__Tok_Comma) {
			ftd__parserAdvance(p);
		}
		ftd__skipNewlines(p);
	}
	return true;
}

// -----------------------------------------------------------------------------
// Array parsing
// -----------------------------------------------------------------------------
static void ftd__arrayWriteResult(ftd__Parser *p, const ftdField *arrayField, void *ownerStruct,
                                  void *data, uint32_t count, uint32_t capacity) {
	const ftd__ArraySlot *slot = ftd__findArraySlot(p->ctx, NULL, arrayField->name);
	// We need owner type to lookup slot. The caller knows.
	// (See parseArrayInto below — we pass owner via field offset bookkeeping.)
	if (slot != NULL) {
		uint8_t *base = (uint8_t *)ownerStruct;
		*(void **)(base + slot->dataOffset) = data;
		if (slot->countSize == 8) {
			*(uint64_t *)(base + slot->countOffset) = count;
		} else {
			*(uint32_t *)(base + slot->countOffset) = count;
		}
	} else {
		ftdArrayHandle *h = (ftdArrayHandle *)((uint8_t *)ownerStruct + arrayField->offset);
		h->data = data;
		h->count = count;
		h->capacity = capacity;
	}
}

// -----------------------------------------------------------------------------
// Array bundle: an array split into three host-struct fields
// ("X.data" / "X.count" / "X.capacity") instead of one ftdArrayHandle.
// .data is required; .count and .capacity are optional. Element type comes
// from the .data field's `subtype`.
// -----------------------------------------------------------------------------
static bool ftd__findArrayBundle(const ftdType *t, const char *name, ftd__ArrayBundle *out) {
	memset(out, 0, sizeof(*out));
	if (t == NULL || t->fields == NULL || name == NULL) {
		return false;
	}
	size_t nameLen = strlen(name);
	for (uint32_t i = 0; i < t->fieldCount; ++i) {
		const ftdField *f = &t->fields[i];
		if (f->name == NULL) {
			continue;
		}
		if (strncmp(f->name, name, nameLen) != 0) {
			continue;
		}
		if (f->name[nameLen] != '.') {
			continue;
		}
		const char *sub = f->name + nameLen + 1;
		if (f->kind == ftdFieldKind_ArrayData && strcmp(sub, "data") == 0) {
			out->data = f;
			out->elemType = f->subtype;
		} else if (f->kind == ftdFieldKind_ArrayCount && strcmp(sub, "count") == 0) {
			out->count = f;
		} else if (f->kind == ftdFieldKind_ArrayCapacity && strcmp(sub, "capacity") == 0) {
			out->capacity = f;
		}
	}
	return out->data != NULL;
}

static void ftd__bundleWriteResult(const ftd__ArrayBundle *bundle, void *ownerStruct,
                                   void *data, uint32_t count, uint32_t capacity) {
	uint8_t *base = (uint8_t *)ownerStruct;
	if (bundle->data != NULL) {
		*(void **)(base + bundle->data->offset) = data;
	}
	if (bundle->count != NULL) {
		*(uint32_t *)(base + bundle->count->offset) = count;
	}
	if (bundle->capacity != NULL) {
		*(uint32_t *)(base + bundle->capacity->offset) = capacity;
	}
}

// Read array elements into a fresh, contiguous, arena-owned buffer, then hand
// the buffer + count + capacity to the bundle. Mirrors ftd__parseArrayInto
// element handling so refs/structs/fixups all behave identically.
static bool ftd__parseArrayBundle(ftd__Parser *p, const ftd__ArrayBundle *bundle, void *ownerStruct) {
	if (!ftd__match(p, ftd__Tok_LBracket)) {
		return false;
	}
	const ftdType *elemType = bundle->elemType;
	if (elemType == NULL) {
		ftd__skipBalanced(p);
		ftd__bundleWriteResult(bundle, ownerStruct, NULL, 0, 0);
		return true;
	}
	size_t elemSize = elemType->size > 0 ? elemType->size : sizeof(void *);
	size_t elemAlign = elemType->align > 0 ? elemType->align : 8;

	uint8_t *data = NULL;
	uint32_t count = 0;
	uint32_t capacity = 0;

	for (;;) {
		ftd__skipNewlines(p);
		if (p->cur.kind == ftd__Tok_RBracket) {
			ftd__parserAdvance(p);
			break;
		}
		if (p->cur.kind == ftd__Tok_EOF) {
			break;
		}

		if (count >= capacity) {
			uint32_t newCap = capacity == 0 ? 4 : capacity * 2;
			uint8_t *newData = (uint8_t *)ftd__arenaAlloc(&p->ctx->parseArena, newCap * elemSize, elemAlign);
			if (newData == NULL) {
				break;
			}
			if (data != NULL && count > 0) {
				memcpy(newData, data, count * elemSize);
			}
			data = newData;
			capacity = newCap;
		}

		void *elemSlot = data + count * elemSize;
		ftdValue val;
		ftd__parseValue(p, NULL, elemType, &val);

		if (val.kind == ftdValueKind_Struct && val.as.ptr != NULL && elemType->size > 0) {
			memcpy(elemSlot, val.as.ptr, elemType->size);
		} else if (val.kind == ftdValueKind_Ref) {
			if (val.type == (const ftdType *)(uintptr_t)1) {
				char **stub = (char **)val.as.ptr;
				if (stub != NULL) {
					ftd__scheduleFixup(p, (void **)elemSlot, *stub, val.span, elemType);
				}
			} else {
				*(void **)elemSlot = val.as.ptr;
			}
		}
		count++;

		ftd__skipNewlines(p);
		if (ftd__match(p, ftd__Tok_Comma)) {
			continue;
		}
		if (p->cur.kind == ftd__Tok_RBracket) {
			ftd__parserAdvance(p);
			break;
		}
		if (p->cur.kind == ftd__Tok_Newline) {
			continue;
		}
	}

	ftd__bundleWriteResult(bundle, ownerStruct, data, count, capacity);
	return true;
}

static bool ftd__parseArrayInto(ftd__Parser *p, const ftdField *arrayField, void *ownerStruct) {
	if (!ftd__match(p, ftd__Tok_LBracket)) {
		return false;
	}
	const ftdType *elemType = arrayField->subtype;
	if (elemType == NULL) {
		// Skip — can't construct elements
		ftd__skipBalanced(p);
		return true;
	}
	size_t elemSize = elemType->size > 0 ? elemType->size : sizeof(void *);
	size_t elemAlign = elemType->align > 0 ? elemType->align : 8;

	uint8_t *data = NULL;
	uint32_t count = 0;
	uint32_t capacity = 0;

	for (;;) {
		ftd__skipNewlines(p);
		if (p->cur.kind == ftd__Tok_RBracket) {
			ftd__parserAdvance(p);
			break;
		}
		if (p->cur.kind == ftd__Tok_EOF) {
			break;
		}

		// Grow
		if (count >= capacity) {
			uint32_t newCap = capacity == 0 ? 4 : capacity * 2;
			uint8_t *newData = (uint8_t *)ftd__arenaAlloc(&p->ctx->parseArena, newCap * elemSize, elemAlign);
			if (newData == NULL) {
				break;
			}
			if (data != NULL && count > 0) {
				memcpy(newData, data, count * elemSize);
			}
			data = newData;
			capacity = newCap;
		}

		void *elemSlot = data + count * elemSize;
		ftdValue val;
		ftd__parseValue(p, NULL, elemType, &val);

		// Write into element slot based on element type
		if (val.kind == ftdValueKind_Struct && val.as.ptr != NULL && elemType->size > 0) {
			memcpy(elemSlot, val.as.ptr, elemType->size);
		} else if (val.kind == ftdValueKind_Ref) {
			if (val.type == (const ftdType *)(uintptr_t)1) {
				char **stub = (char **)val.as.ptr;
				if (stub != NULL) {
					ftd__scheduleFixup(p, (void **)elemSlot, *stub, val.span, elemType);
				}
			} else {
				*(void **)elemSlot = val.as.ptr;
			}
		}
		count++;

		ftd__skipNewlines(p);
		if (ftd__match(p, ftd__Tok_Comma)) {
			continue;
		}
		if (p->cur.kind == ftd__Tok_RBracket) {
			ftd__parserAdvance(p);
			break;
		}
		if (p->cur.kind == ftd__Tok_Newline) {
			continue;
		}
	}

	ftd__arrayWriteResult(p, arrayField, ownerStruct, data, count, capacity);
	return true;
}

// -----------------------------------------------------------------------------
// Top-level statement parsing
// -----------------------------------------------------------------------------
static void ftd__parseAlias(ftd__Parser *p) {
	// 'alias' already consumed
	// alias [Type] Name = qname
	// Skip optional type prefix (token Ident not followed by '=')
	if (p->cur.kind != ftd__Tok_Ident) {
		ftd__emit(p->ctx, ftdSeverity_Error, p->cur.span, "expected identifier after 'alias'");
		ftd__skipToNewlineOrEOF(p);
		return;
	}

	const char *first = ftd__parseQName(p);
	const char *aliasName = first;
	const char *target = NULL;

	if (p->cur.kind == ftd__Tok_Ident) {
		// We had "alias Type Name = ..." form. The first qname was the type prefix; current is the actual name.
		aliasName = ftd__parseQName(p);
	}

	if (!ftd__match(p, ftd__Tok_Eq)) {
		ftd__emit(p->ctx, ftdSeverity_Error, p->cur.span, "expected '=' in alias declaration");
		ftd__skipToNewlineOrEOF(p);
		return;
	}
	if (p->cur.kind != ftd__Tok_Ident) {
		ftd__emit(p->ctx, ftdSeverity_Error, p->cur.span, "expected target name");
		ftd__skipToNewlineOrEOF(p);
		return;
	}
	target = ftd__parseQName(p);

	// Register in parse-time symbols (so resetParse drops them)
	const char *nm = ftd__arenaStrDup(&p->ctx->parseArena, aliasName);
	const char *tg = ftd__arenaStrDup(&p->ctx->parseArena, target);
	uint32_t h = ftd__fnv1aStr(nm);
	ftd__Symbol *s = ftd__symTableInsert(&p->ctx->parseSyms, nm, h);
	if (s != NULL) {
		s->kind = ftd__Sym_Alias;
		s->as.aliasTarget = tg;
	}
	(void)first;
}

static void ftd__parseTypedDecl(ftd__Parser *p) {
	if (p->cur.kind != ftd__Tok_Ident) {
		ftd__skipToNewlineOrEOF(p);
		return;
	}
	ftdSourceSpan typeSpan = p->cur.span;
	const char *typeName = ftd__parseQName(p);
	const ftd__Symbol *typeSym = ftd__resolveAlias(p->ctx, typeName, 0);

	if (typeSym == NULL) {
		ftd__emit(p->ctx, ftdSeverity_Error, typeSpan, "unknown type '%s'", typeName);
		ftd__skipToNewlineOrEOF(p);
		return;
	}

	if (p->cur.kind != ftd__Tok_Ident) {
		ftd__emit(p->ctx, ftdSeverity_Error, p->cur.span, "expected name after type '%s'", typeName);
		ftd__skipToNewlineOrEOF(p);
		return;
	}

	const char *declName = ftd__parseQName(p);
	const char *internName = ftd__arenaStrDup(&p->ctx->parseArena, declName);
	uint32_t declHash = ftd__fnv1aStr(internName);

	if (ftd__match(p, ftd__Tok_Eq)) {
		// File-local constant
		ftdValue val;
		ftd__parseValue(p, NULL, typeSym->type, &val);

		if (typeSym->kind == ftd__Sym_Builtin) {
			// scalar const
			ftdFieldKind sk = typeSym->scalarKind;
			size_t sz = ftd__scalarSize(sk);
			void *mem = ftd__arenaAlloc(&p->ctx->parseArena, sz ? sz : sizeof(int), 8);
			if (mem != NULL) {
				ftd__writeScalar(mem, sk, &val);
				ftd__Symbol *s = ftd__symTableInsert(&p->ctx->parseSyms, internName, declHash);
				if (s != NULL) {
					s->kind = ftd__Sym_Const;
					s->scalarKind = sk;
					s->as.constPtr = mem;
				}
			}
		} else if (typeSym->kind == ftd__Sym_Type && typeSym->type != NULL) {
			// struct const
			const ftdType *t = typeSym->type;
			void *mem = ftd__arenaAlloc(&p->ctx->parseArena, t->size, t->align ? t->align : 8);
			if (mem != NULL) {
				if (val.kind == ftdValueKind_Struct && val.as.ptr != NULL) {
					memcpy(mem, val.as.ptr, t->size);
				}
				ftd__Symbol *s = ftd__symTableInsert(&p->ctx->parseSyms, internName, declHash);
				if (s != NULL) {
					s->kind = ftd__Sym_Const;
					s->type = t;
					s->as.constPtr = mem;
				}
			}
		} else if (typeSym->kind == ftd__Sym_Enum) {
			void *mem = ftd__arenaAlloc(&p->ctx->parseArena, sizeof(int32_t), 4);
			if (mem != NULL) {
				if (val.kind == ftdValueKind_Enum || val.kind == ftdValueKind_Int) {
					*(int32_t *)mem = (int32_t)val.as.i;
				}
				ftd__Symbol *s = ftd__symTableInsert(&p->ctx->parseSyms, internName, declHash);
				if (s != NULL) {
					s->kind = ftd__Sym_Const;
					s->type = typeSym->type;
					s->scalarKind = ftdFieldKind_Enum;
					s->as.constPtr = mem;
				}
			}
		}
	} else if (p->cur.kind == ftd__Tok_LBrace) {
		// Named instance
		if (typeSym->kind != ftd__Sym_Type || typeSym->type == NULL) {
			ftd__emit(p->ctx, ftdSeverity_Error, typeSpan,
				"named instance declared with non-struct type '%s'", typeName);
			ftd__skipBalanced(p);
			return;
		}
		const ftdType *t = typeSym->type;
		void *mem = ftd__arenaAlloc(&p->ctx->parseArena, t->size, t->align ? t->align : 8);
		if (mem == NULL) {
			ftd__skipBalanced(p);
			return;
		}
		ftd__parseBlockInto(p, t, mem);
		ftd__Symbol *s = ftd__symTableInsert(&p->ctx->parseSyms, internName, declHash);
		if (s != NULL) {
			s->kind = ftd__Sym_Instance;
			s->type = t;
			s->as.constPtr = mem;
		}
	} else {
		ftd__emit(p->ctx, ftdSeverity_Error, p->cur.span,
			"expected '=' or '{' after '%s %s'", typeName, declName);
		ftd__skipToNewlineOrEOF(p);
	}
}

// -----------------------------------------------------------------------------
// Fixup resolution pass
// -----------------------------------------------------------------------------
static void ftd__resolveFixups(ftd__Parser *p) {
	ftd__Fixup *fx = p->fixupHead;
	while (fx != NULL) {
		const ftd__Symbol *sym = ftd__resolveAlias(p->ctx, fx->name, 0);
		if (sym != NULL && (sym->kind == ftd__Sym_Instance || sym->kind == ftd__Sym_Const || sym->kind == ftd__Sym_Global)) {
			if (fx->slot != NULL) {
				*fx->slot = (void *)sym->as.constPtr;
			}
		} else {
			ftd__emit(p->ctx, ftdSeverity_Warning, fx->span,
				"unresolved fixup: '%s' (file may be out of date)", fx->name);
		}
		fx = fx->next;
	}
}

// -----------------------------------------------------------------------------
// Diagnostics snapshot
// -----------------------------------------------------------------------------
static void ftd__finalizeResult(ftdContext *ctx) {
	ftdDiagnostic *arr = NULL;
	if (ctx->diagCount > 0) {
		arr = (ftdDiagnostic *)ftd__arenaAlloc(&ctx->parseArena, sizeof(ftdDiagnostic) * ctx->diagCount, 8);
	}
	if (arr != NULL) {
		uint32_t i = 0;
		ftd__DiagNode *n = ctx->diagHead;
		while (n != NULL) {
			arr[i++] = n->d;
			n = n->next;
		}
	}
	ctx->diagArray = arr;
	ctx->lastResult.ok = (ctx->errorCount == 0);
	ctx->lastResult.errorCount = ctx->errorCount;
	ctx->lastResult.warningCount = ctx->warningCount;
	ctx->lastResult.diagnosticCount = ctx->diagCount;
	ctx->lastResult.diagnostics = arr;
}

// -----------------------------------------------------------------------------
// Public parsing entry points
// -----------------------------------------------------------------------------
FTD_API ftdResult ftdParseString(ftdContext *ctx,
                                 const char *source,
                                 size_t      length,
                                 const char *displayName) {
	ftdResult empty;
	memset(&empty, 0, sizeof(empty));
	if (ctx == NULL || source == NULL) {
		return empty;
	}
	ftdResetParse(ctx);

	if (displayName != NULL) {
		ctx->currentSourceName = ftd__arenaStrDup(&ctx->parseArena, displayName);
	} else {
		ctx->currentSourceName = NULL;
	}

	// Copy source into parse arena so spans remain valid
	char *copy = (char *)ftd__arenaAlloc(&ctx->parseArena, length + 1, 1);
	if (copy == NULL) {
		ftd__finalizeResult(ctx);
		return ctx->lastResult;
	}
	memcpy(copy, source, length);
	copy[length] = '\0';

	ftd__Parser p;
	memset(&p, 0, sizeof(p));
	p.ctx = ctx;
	ftd__lexerInit(&p.lex, ctx, copy, length);
	ftd__parserAdvance(&p);

	for (;;) {
		ftd__skipNewlines(&p);
		if (p.cur.kind == ftd__Tok_EOF) {
			break;
		}
		if (p.cur.kind == ftd__Tok_KwAlias) {
			ftd__parserAdvance(&p);
			ftd__parseAlias(&p);
			continue;
		}
		if (p.cur.kind == ftd__Tok_Ident) {
			ftd__parseTypedDecl(&p);
			continue;
		}
		ftd__emit(ctx, ftdSeverity_Error, p.cur.span, "unexpected token at top level");
		// Always advance at least once so we make forward progress, even when
		// the offending token is a stray closing delimiter that
		// ftd__skipToNewlineOrEOF refuses to step over.
		ftd__parserAdvance(&p);
		ftd__skipToNewlineOrEOF(&p);
	}

	ftd__resolveFixups(&p);
	ftd__finalizeResult(ctx);
	return ctx->lastResult;
}

#if !defined(FTD_NO_STDIO)
FTD_API ftdResult ftdParseFile(ftdContext *ctx, const char *filePath) {
	ftdResult empty;
	memset(&empty, 0, sizeof(empty));
	if (ctx == NULL || filePath == NULL) {
		return empty;
	}
	FILE *fp = fopen(filePath, "rb");
	if (fp == NULL) {
		ftdResetParse(ctx);
		ftdSourceSpan sp = { 0 };
		sp.file = filePath;
		ftd__emit(ctx, ftdSeverity_Error, sp, "could not open file '%s'", filePath);
		ftd__finalizeResult(ctx);
		return ctx->lastResult;
	}
	fseek(fp, 0, SEEK_END);
	long sz = ftell(fp);
	if (sz < 0) {
		fclose(fp);
		ftdResetParse(ctx);
		ftdSourceSpan sp = { 0 };
		sp.file = filePath;
		ftd__emit(ctx, ftdSeverity_Error, sp, "could not read file size '%s'", filePath);
		ftd__finalizeResult(ctx);
		return ctx->lastResult;
	}
	fseek(fp, 0, SEEK_SET);
	char *buf = (char *)malloc((size_t)sz + 1);
	if (buf == NULL) {
		fclose(fp);
		ftdResetParse(ctx);
		ftdSourceSpan sp = { 0 };
		sp.file = filePath;
		ftd__emit(ctx, ftdSeverity_Error, sp, "out of memory reading '%s'", filePath);
		ftd__finalizeResult(ctx);
		return ctx->lastResult;
	}
	size_t got = fread(buf, 1, (size_t)sz, fp);
	fclose(fp);
	buf[got] = '\0';
	ftdResult r = ftdParseString(ctx, buf, got, filePath);
	free(buf);
	return r;
}
#endif

// -----------------------------------------------------------------------------
// Lookup
// -----------------------------------------------------------------------------
FTD_API const void *ftdLookup(ftdContext *ctx, const char *dottedName, const ftdType **outType) {
	if (outType) {
		*outType = NULL;
	}
	if (ctx == NULL || dottedName == NULL) {
		return NULL;
	}
	const ftd__Symbol *s = ftd__resolveAlias(ctx, dottedName, 0);
	if (s == NULL) {
		return NULL;
	}
	if (s->kind == ftd__Sym_Const || s->kind == ftd__Sym_Instance || s->kind == ftd__Sym_Global) {
		if (outType) {
			*outType = s->type;
		}
		return s->as.constPtr;
	}
	return NULL;
}

#ifdef __cplusplus
}
#endif

#endif // FTD_IMPLEMENTATION
