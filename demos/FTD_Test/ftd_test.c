/*
-------------------------------------------------------------------------------
Name:
	FTD | Test

Description:
	This demo shows how to use the "Final Text Data" (FTD) library and acts as
	a self-contained correctness test. It exercises every public API: arenas
	(implicit), tokenizer, parser, type/enum/alias/global/helper registration,
	forward references, unions with discriminators, arrays (default and custom
	slot bindings), diagnostics, and the hot-reload pattern.

Requirements:
	- C99

Author:
	Torsten Spaete

Changelog:
	## 2026-05-23
	- Initial version
-------------------------------------------------------------------------------
*/

#define FTD_IMPLEMENTATION
#include <final_ftd.h>

#include "fpl_presentation_types.h"
#include "fpl_presentation_ftd_data.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define ftdAlwaysAssert(exp) if(!(exp)) {*(int *)0 = 0;}

#define ftdAlmostEqual(a, b) (fabs((double)(a) - (double)(b)) < 1e-5)

// -----------------------------------------------------------------------------
// Shared host types
// -----------------------------------------------------------------------------
typedef struct V2f { float x, y; } V2f;
typedef struct V4f { float r, g, b, a; } V4f;

typedef enum Color {
	Color_Red   = 0,
	Color_Green = 1,
	Color_Blue  = 2,
} Color;

typedef enum BlockType {
	BlockType_Text  = 0,
	BlockType_Image = 1,
} BlockType;

typedef struct TextBlock {
	const char *content;
	float       fontSize;
} TextBlock;

typedef struct ImageBlock {
	const char *imageName;
	V2f         size;
} ImageBlock;

typedef struct Block {
	V2f       pos;
	BlockType type;
	union {
		TextBlock  text;
		ImageBlock image;
	};
} Block;

typedef struct Slide {
	const char     *title;
	V4f             background;
	ftdArrayHandle  blocks;
} Slide;

typedef struct CustomArraySlideBlocks {
	Block    *items;
	uint32_t  itemCount;
} CustomArraySlideBlocks;

typedef struct CustomSlide {
	const char            *title;
	CustomArraySlideBlocks blocks;
} CustomSlide;

typedef struct Resource {
	const char *name;
	int32_t     id;
} Resource;

typedef struct Linker {
	const Resource *primary;
	const Resource *secondary;
} Linker;

typedef struct AllScalars {
	bool        b;
	int8_t      i8;
	int16_t     i16;
	int32_t     i32;
	int64_t     i64;
	uint8_t     u8;
	uint16_t    u16;
	uint32_t    u32;
	uint64_t    u64;
	float       f32;
	double      f64;
	const char *str;
} AllScalars;

// -----------------------------------------------------------------------------
// Field tables
// -----------------------------------------------------------------------------
static const ftdField V2f_fields[] = {
	{ "x", offsetof(V2f, x), ftdFieldKind_F32, NULL, NULL, 0, 0 },
	{ "y", offsetof(V2f, y), ftdFieldKind_F32, NULL, NULL, 0, 0 },
};
static const ftdType V2f_type = {
	"V2f", sizeof(V2f), _Alignof(V2f),
	V2f_fields, 2,
	NULL, 0
};

static const ftdField V4f_fields[] = {
	{ "r", offsetof(V4f, r), ftdFieldKind_F32, NULL, NULL, 0, 0 },
	{ "g", offsetof(V4f, g), ftdFieldKind_F32, NULL, NULL, 0, 0 },
	{ "b", offsetof(V4f, b), ftdFieldKind_F32, NULL, NULL, 0, 0 },
	{ "a", offsetof(V4f, a), ftdFieldKind_F32, NULL, NULL, 0, 0 },
};
static const ftdType V4f_type = {
	"V4f", sizeof(V4f), _Alignof(V4f),
	V4f_fields, 4,
	NULL, 0
};

static const ftdEnumValue Color_values[] = {
	{ "Red", 0 }, { "Green", 1 }, { "Blue", 2 },
};
static const ftdType Color_type = {
	"Color", sizeof(int), _Alignof(int),
	NULL, 0,
	Color_values, 3
};

static const ftdEnumValue BlockType_values[] = {
	{ "Text", 0 }, { "Image", 1 },
};
static const ftdType BlockType_type = {
	"BlockType", sizeof(int), _Alignof(int),
	NULL, 0,
	BlockType_values, 2
};

static const ftdField TextBlock_fields[] = {
	{ "content",  offsetof(TextBlock, content),  ftdFieldKind_String, NULL, NULL, 0, 0 },
	{ "fontSize", offsetof(TextBlock, fontSize), ftdFieldKind_F32,    NULL, NULL, 0, 0 },
};
static const ftdType TextBlock_type = {
	"TextBlock", sizeof(TextBlock), _Alignof(TextBlock),
	TextBlock_fields, 2,
	NULL, 0
};

static const ftdField ImageBlock_fields[] = {
	{ "imageName", offsetof(ImageBlock, imageName), ftdFieldKind_String, NULL,      NULL, 0, 0 },
	{ "size",      offsetof(ImageBlock, size),      ftdFieldKind_Struct, &V2f_type, NULL, 0, 0 },
};
static const ftdType ImageBlock_type = {
	"ImageBlock", sizeof(ImageBlock), _Alignof(ImageBlock),
	ImageBlock_fields, 2,
	NULL, 0
};

static const ftdField Block_fields[] = {
	{ "pos",   offsetof(Block, pos),   ftdFieldKind_Struct, &V2f_type,        NULL,   0,               0 },
	{ "type",  offsetof(Block, type),  ftdFieldKind_Enum,   &BlockType_type,  NULL,   0,               0 },
	{ "text",  offsetof(Block, text),  ftdFieldKind_Union,  &TextBlock_type,  "type", BlockType_Text,  0 },
	{ "image", offsetof(Block, image), ftdFieldKind_Union,  &ImageBlock_type, "type", BlockType_Image, 0 },
};
static const ftdType Block_type = {
	"Block", sizeof(Block), _Alignof(Block),
	Block_fields, 4,
	NULL, 0
};

static const ftdField Slide_fields[] = {
	{ "title",      offsetof(Slide, title),      ftdFieldKind_String, NULL,        NULL, 0, 0 },
	{ "background", offsetof(Slide, background), ftdFieldKind_Struct, &V4f_type,   NULL, 0, 0 },
	{ "blocks",     offsetof(Slide, blocks),     ftdFieldKind_Array,  &Block_type, NULL, 0, 0 },
};
static const ftdType Slide_type = {
	"Slide", sizeof(Slide), _Alignof(Slide),
	Slide_fields, 3,
	NULL, 0
};

static const ftdField CustomSlide_fields[] = {
	{ "title",  offsetof(CustomSlide, title),  ftdFieldKind_String, NULL,        NULL, 0, 0 },
	{ "blocks", offsetof(CustomSlide, blocks), ftdFieldKind_Array,  &Block_type, NULL, 0, 0 },
};
static const ftdType CustomSlide_type = {
	"CustomSlide", sizeof(CustomSlide), _Alignof(CustomSlide),
	CustomSlide_fields, 2,
	NULL, 0
};

static const ftdField Resource_fields[] = {
	{ "name", offsetof(Resource, name), ftdFieldKind_String, NULL, NULL, 0, 0 },
	{ "id",   offsetof(Resource, id),   ftdFieldKind_S32,    NULL, NULL, 0, 0 },
};
static const ftdType Resource_type = {
	"Resource", sizeof(Resource), _Alignof(Resource),
	Resource_fields, 2,
	NULL, 0
};

static const ftdField Linker_fields[] = {
	{ "primary",   offsetof(Linker, primary),   ftdFieldKind_Ref, &Resource_type, NULL, 0, 0 },
	{ "secondary", offsetof(Linker, secondary), ftdFieldKind_Ref, &Resource_type, NULL, 0, 0 },
};
static const ftdType Linker_type = {
	"Linker", sizeof(Linker), _Alignof(Linker),
	Linker_fields, 2,
	NULL, 0
};

static const ftdField AllScalars_fields[] = {
	{ "b",   offsetof(AllScalars, b),   ftdFieldKind_Bool,   NULL, NULL, 0, 0 },
	{ "i8",  offsetof(AllScalars, i8),  ftdFieldKind_S8,     NULL, NULL, 0, 0 },
	{ "i16", offsetof(AllScalars, i16), ftdFieldKind_S16,    NULL, NULL, 0, 0 },
	{ "i32", offsetof(AllScalars, i32), ftdFieldKind_S32,    NULL, NULL, 0, 0 },
	{ "i64", offsetof(AllScalars, i64), ftdFieldKind_S64,    NULL, NULL, 0, 0 },
	{ "u8",  offsetof(AllScalars, u8),  ftdFieldKind_U8,     NULL, NULL, 0, 0 },
	{ "u16", offsetof(AllScalars, u16), ftdFieldKind_U16,    NULL, NULL, 0, 0 },
	{ "u32", offsetof(AllScalars, u32), ftdFieldKind_U32,    NULL, NULL, 0, 0 },
	{ "u64", offsetof(AllScalars, u64), ftdFieldKind_U64,    NULL, NULL, 0, 0 },
	{ "f32", offsetof(AllScalars, f32), ftdFieldKind_F32,    NULL, NULL, 0, 0 },
	{ "f64", offsetof(AllScalars, f64), ftdFieldKind_F64,    NULL, NULL, 0, 0 },
	{ "str", offsetof(AllScalars, str), ftdFieldKind_String, NULL, NULL, 0, 0 },
};
static const ftdType AllScalars_type = {
	"AllScalars", sizeof(AllScalars), _Alignof(AllScalars),
	AllScalars_fields, 12,
	NULL, 0
};

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------
static ftdContext *MakeCommonContext(void) {
	ftdContext *ctx = ftdCreate(NULL);
	ftdAlwaysAssert(ctx != NULL);
	ftdRegisterStruct(ctx, &V2f_type);
	ftdRegisterStruct(ctx, &V4f_type);
	ftdRegisterEnum  (ctx, &Color_type);
	ftdRegisterEnum  (ctx, &BlockType_type);
	ftdRegisterStruct(ctx, &TextBlock_type);
	ftdRegisterStruct(ctx, &ImageBlock_type);
	ftdRegisterStruct(ctx, &Block_type);
	ftdRegisterStruct(ctx, &Slide_type);
	ftdRegisterStruct(ctx, &CustomSlide_type);
	ftdRegisterStruct(ctx, &Resource_type);
	ftdRegisterStruct(ctx, &Linker_type);
	ftdRegisterStruct(ctx, &AllScalars_type);
	return ctx;
}

static ftdResult ParseLiteral(ftdContext *ctx, const char *src) {
	return ftdParseString(ctx, src, strlen(src), "<test>");
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------
static void TestContextLifecycle(void) {
	ftdContext *ctx = ftdCreate(NULL);
	ftdAlwaysAssert(ctx != NULL);
	ftdDestroy(ctx);
	ftdDestroy(NULL); // must be safe
}

static void TestEmptySource(void) {
	ftdContext *ctx = ftdCreate(NULL);
	ftdResult r = ParseLiteral(ctx, "");
	ftdAlwaysAssert(r.ok);
	ftdAlwaysAssert(r.errorCount == 0);
	ftdAlwaysAssert(r.warningCount == 0);
	ftdDestroy(ctx);
}

static void TestComments(void) {
	ftdContext *ctx = ftdCreate(NULL);
	const char *src =
		"// line comment\n"
		"# hash comment\n"
		"/* block\n"
		"   comment */\n"
		"float X = 1.0 // trailing\n"
		"float Y = 2.0 # trailing\n"
		"float Z = 3.0 /* inline */\n";
	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r.ok);

	const float *x = (const float *)ftdLookup(ctx, "X", NULL);
	const float *y = (const float *)ftdLookup(ctx, "Y", NULL);
	const float *z = (const float *)ftdLookup(ctx, "Z", NULL);
	ftdAlwaysAssert(x && ftdAlmostEqual(*x, 1.0));
	ftdAlwaysAssert(y && ftdAlmostEqual(*y, 2.0));
	ftdAlwaysAssert(z && ftdAlmostEqual(*z, 3.0));
	ftdDestroy(ctx);
}

static void TestNumberLiterals(void) {
	ftdContext *ctx = ftdCreate(NULL);
	const char *src =
		"int  A = 42\n"
		"int  B = -17\n"
		"u32  C = 0xFF\n"
		"u32  D = 0b1010\n"
		"u32  E = 0o17\n"
		"f32  F = 1e3\n"
		"f64  G = -3.14\n"
		"f32  H = 0.5f\n"
		"int  I = 'A'\n";
	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r.ok);

	const int32_t  *a = (const int32_t  *)ftdLookup(ctx, "A", NULL);
	const int32_t  *b = (const int32_t  *)ftdLookup(ctx, "B", NULL);
	const uint32_t *c = (const uint32_t *)ftdLookup(ctx, "C", NULL);
	const uint32_t *d = (const uint32_t *)ftdLookup(ctx, "D", NULL);
	const uint32_t *e = (const uint32_t *)ftdLookup(ctx, "E", NULL);
	const float    *f = (const float    *)ftdLookup(ctx, "F", NULL);
	const double   *g = (const double   *)ftdLookup(ctx, "G", NULL);
	const float    *h = (const float    *)ftdLookup(ctx, "H", NULL);
	const int32_t  *i = (const int32_t  *)ftdLookup(ctx, "I", NULL);
	ftdAlwaysAssert(a && *a == 42);
	ftdAlwaysAssert(b && *b == -17);
	ftdAlwaysAssert(c && *c == 0xFFu);
	ftdAlwaysAssert(d && *d == 10u);
	ftdAlwaysAssert(e && *e == 15u);
	ftdAlwaysAssert(f && ftdAlmostEqual(*f, 1000.0));
	ftdAlwaysAssert(g && ftdAlmostEqual(*g, -3.14));
	ftdAlwaysAssert(h && ftdAlmostEqual(*h, 0.5));
	ftdAlwaysAssert(i && *i == 'A');

	ftdDestroy(ctx);
}

static void TestStringEscapesAndConcatenation(void) {
	ftdContext *ctx = ftdCreate(NULL);
	const char *src =
		"string A = \"hello\\nworld\"\n"
		"string B = \"foo\" \"bar\"\n"
		"string C = \"line1\"\n"
		"           \"line2\"\n"
		"string D = \"tab\\there\\x41\"\n";
	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r.ok);

	const char *a = *(const char *const *)ftdLookup(ctx, "A", NULL);
	const char *b = *(const char *const *)ftdLookup(ctx, "B", NULL);
	const char *c = *(const char *const *)ftdLookup(ctx, "C", NULL);
	const char *d = *(const char *const *)ftdLookup(ctx, "D", NULL);
	ftdAlwaysAssert(a && strcmp(a, "hello\nworld") == 0);
	ftdAlwaysAssert(b && strcmp(b, "foobar") == 0);
	ftdAlwaysAssert(c && strcmp(c, "line1line2") == 0);
	ftdAlwaysAssert(d && strcmp(d, "tab\thereA") == 0);

	ftdDestroy(ctx);
}

static void TestAllScalarsInStruct(void) {
	ftdContext *ctx = MakeCommonContext();
	const char *src =
		"AllScalars S {\n"
		"    b   = true\n"
		"    i8  = -8\n"
		"    i16 = -1600\n"
		"    i32 = -32000\n"
		"    i64 = -640000000000\n"
		"    u8  = 200\n"
		"    u16 = 60000\n"
		"    u32 = 0xDEADBEEF\n"
		"    u64 = 0xCAFEBABEDEADBEEF\n"
		"    f32 = 1.5\n"
		"    f64 = 2.5\n"
		"    str = \"hi\"\n"
		"}\n";
	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r.ok);

	const AllScalars *s = (const AllScalars *)ftdLookup(ctx, "S", NULL);
	ftdAlwaysAssert(s != NULL);
	ftdAlwaysAssert(s->b == true);
	ftdAlwaysAssert(s->i8  == -8);
	ftdAlwaysAssert(s->i16 == -1600);
	ftdAlwaysAssert(s->i32 == -32000);
	ftdAlwaysAssert(s->i64 == -640000000000LL);
	ftdAlwaysAssert(s->u8  == 200);
	ftdAlwaysAssert(s->u16 == 60000);
	ftdAlwaysAssert(s->u32 == 0xDEADBEEFu);
	ftdAlwaysAssert(s->u64 == 0xCAFEBABEDEADBEEFull);
	ftdAlwaysAssert(ftdAlmostEqual(s->f32, 1.5));
	ftdAlwaysAssert(ftdAlmostEqual(s->f64, 2.5));
	ftdAlwaysAssert(s->str && strcmp(s->str, "hi") == 0);

	ftdDestroy(ctx);
}

static void TestCoercion(void) {
	ftdContext *ctx = MakeCommonContext();
	const char *src =
		"AllScalars S {\n"
		"    i32 = 3.9\n"   // float -> int (truncates)
		"    f32 = 42\n"    // int -> float
		"    b   = 1\n"     // int -> bool
		"}\n";
	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r.ok);
	const AllScalars *s = (const AllScalars *)ftdLookup(ctx, "S", NULL);
	ftdAlwaysAssert(s != NULL);
	ftdAlwaysAssert(s->i32 == 3);
	ftdAlwaysAssert(ftdAlmostEqual(s->f32, 42.0));
	ftdAlwaysAssert(s->b == true);
	ftdDestroy(ctx);
}

static void TestAlias(void) {
	ftdContext *ctx = MakeCommonContext();
	const char *src =
		"alias Vec2  = V2f\n"
		"alias Pos   = Vec2\n"   // chained alias
		"alias Red0  = Color.Red\n"
		"V2f A = Vec2(1, 2)\n"
		"V2f B = Pos(3, 4)\n";
	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r.ok);
	const V2f *a = (const V2f *)ftdLookup(ctx, "A", NULL);
	const V2f *b = (const V2f *)ftdLookup(ctx, "B", NULL);
	ftdAlwaysAssert(a && ftdAlmostEqual(a->x, 1.0) && ftdAlmostEqual(a->y, 2.0));
	ftdAlwaysAssert(b && ftdAlmostEqual(b->x, 3.0) && ftdAlmostEqual(b->y, 4.0));
	ftdDestroy(ctx);
}

static void TestRegisterAliasFromHost(void) {
	ftdContext *ctx = MakeCommonContext();
	ftdRegisterAlias(ctx, "Vector2", "V2f");
	const char *src = "Vector2 P = Vector2(7, 8)\n";
	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r.ok);
	const V2f *p = (const V2f *)ftdLookup(ctx, "P", NULL);
	ftdAlwaysAssert(p && ftdAlmostEqual(p->x, 7.0) && ftdAlmostEqual(p->y, 8.0));
	ftdDestroy(ctx);
}

static void TestNestedStructAndDottedPath(void) {
	ftdContext *ctx = MakeCommonContext();
	const char *src =
		"Block B {\n"
		"    pos.x = 11\n"
		"    pos.y = 22\n"
		"    type  = Text\n"
		"    text {\n"
		"        content = \"in nested\"\n"
		"        fontSize = 30\n"
		"    }\n"
		"}\n";
	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r.ok);
	const Block *b = (const Block *)ftdLookup(ctx, "B", NULL);
	ftdAlwaysAssert(b != NULL);
	ftdAlwaysAssert(ftdAlmostEqual(b->pos.x, 11.0));
	ftdAlwaysAssert(ftdAlmostEqual(b->pos.y, 22.0));
	ftdAlwaysAssert(b->type == BlockType_Text);
	ftdAlwaysAssert(strcmp(b->text.content, "in nested") == 0);
	ftdAlwaysAssert(ftdAlmostEqual(b->text.fontSize, 30.0));
	ftdDestroy(ctx);
}

static void TestPositionalAndInlineForms(void) {
	ftdContext *ctx = MakeCommonContext();
	const char *src =
		"V2f A = V2f(5, 6)\n"
		"V2f B = V2f{x = 9, y = 10}\n";
	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r.ok);
	const V2f *a = (const V2f *)ftdLookup(ctx, "A", NULL);
	const V2f *b = (const V2f *)ftdLookup(ctx, "B", NULL);
	ftdAlwaysAssert(a && ftdAlmostEqual(a->x, 5.0) && ftdAlmostEqual(a->y, 6.0));
	ftdAlwaysAssert(b && ftdAlmostEqual(b->x, 9.0) && ftdAlmostEqual(b->y, 10.0));
	ftdDestroy(ctx);
}

static void TestPositionalMissingArgsZeroInit(void) {
	ftdContext *ctx = MakeCommonContext();
	// Only x given; y must be zero-initialized.
	const char *src = "V2f P = V2f(42)\n";
	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r.ok);
	const V2f *p = (const V2f *)ftdLookup(ctx, "P", NULL);
	ftdAlwaysAssert(p && ftdAlmostEqual(p->x, 42.0) && ftdAlmostEqual(p->y, 0.0));
	ftdDestroy(ctx);
}

static void TestEnumShortAndQualified(void) {
	ftdContext *ctx = MakeCommonContext();
	const char *src =
		"Block A { type = Text }\n"
		"Block B { type = BlockType.Image }\n";
	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r.ok);
	const Block *a = (const Block *)ftdLookup(ctx, "A", NULL);
	const Block *b = (const Block *)ftdLookup(ctx, "B", NULL);
	ftdAlwaysAssert(a && a->type == BlockType_Text);
	ftdAlwaysAssert(b && b->type == BlockType_Image);
	ftdDestroy(ctx);
}

static void TestArrayDefaultHandle(void) {
	ftdContext *ctx = MakeCommonContext();
	const char *src =
		"Slide S {\n"
		"    title = \"with blocks\"\n"
		"    blocks = [\n"
		"        Block { pos = V2f(0, 0)  type = Text   text  { content = \"a\"     fontSize = 12 } }\n"
		"        Block { pos = V2f(1, 1)  type = Image  image { imageName = \"x.png\" size = V2f(8, 8) } }\n"
		"        Block { pos = V2f(2, 2)  type = Text   text  { content = \"c\"     fontSize = 14 } },\n"
		"    ]\n"
		"}\n";
	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r.ok);
	const Slide *s = (const Slide *)ftdLookup(ctx, "S", NULL);
	ftdAlwaysAssert(s != NULL);
	ftdAlwaysAssert(s->blocks.count == 3);
	const Block *items = (const Block *)s->blocks.data;
	ftdAlwaysAssert(items[0].type == BlockType_Text);
	ftdAlwaysAssert(strcmp(items[0].text.content, "a") == 0);
	ftdAlwaysAssert(items[1].type == BlockType_Image);
	ftdAlwaysAssert(strcmp(items[1].image.imageName, "x.png") == 0);
	ftdAlwaysAssert(items[2].type == BlockType_Text);
	ftdAlwaysAssert(strcmp(items[2].text.content, "c") == 0);
	ftdDestroy(ctx);
}

static void TestArrayEmpty(void) {
	ftdContext *ctx = MakeCommonContext();
	const char *src = "Slide S { title = \"t\"  blocks = [] }\n";
	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r.ok);
	const Slide *s = (const Slide *)ftdLookup(ctx, "S", NULL);
	ftdAlwaysAssert(s && s->blocks.count == 0);
	ftdDestroy(ctx);
}

static void TestArrayCustomSlot(void) {
	ftdContext *ctx = MakeCommonContext();
	ftdSetArraySlot(ctx, &CustomSlide_type, "blocks",
		offsetof(CustomSlide, blocks.items),
		offsetof(CustomSlide, blocks.itemCount),
		4);
	const char *src =
		"CustomSlide S {\n"
		"    title = \"custom\"\n"
		"    blocks = [\n"
		"        Block { type = Text  text { content = \"q\"  fontSize = 1 } }\n"
		"        Block { type = Text  text { content = \"r\"  fontSize = 2 } }\n"
		"    ]\n"
		"}\n";
	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r.ok);
	const CustomSlide *s = (const CustomSlide *)ftdLookup(ctx, "S", NULL);
	ftdAlwaysAssert(s != NULL);
	ftdAlwaysAssert(s->blocks.itemCount == 2);
	ftdAlwaysAssert(s->blocks.items != NULL);
	ftdAlwaysAssert(strcmp(s->blocks.items[0].text.content, "q") == 0);
	ftdAlwaysAssert(strcmp(s->blocks.items[1].text.content, "r") == 0);
	ftdDestroy(ctx);
}

static void TestUnionMatchAndMismatch(void) {
	ftdContext *ctx = MakeCommonContext();

	// matched: type=Text + text { ... }  ✓
	{
		const char *src =
			"Block B {\n"
			"    type = Text\n"
			"    text { content = \"ok\"  fontSize = 10 }\n"
			"}\n";
		ftdResult r = ParseLiteral(ctx, src);
		ftdAlwaysAssert(r.ok);
		const Block *b = (const Block *)ftdLookup(ctx, "B", NULL);
		ftdAlwaysAssert(b && strcmp(b->text.content, "ok") == 0);
	}

	// mismatched: type=Text but assigned image variant -> warning, skipped
	{
		ftdResetParse(ctx);
		const char *src =
			"Block B {\n"
			"    type = Text\n"
			"    image { imageName = \"oops.png\"  size = V2f(1, 1) }\n"
			"}\n";
		ftdResult r = ParseLiteral(ctx, src);
		ftdAlwaysAssert(r.ok);                  // warnings only, not errors
		const Block *b = (const Block *)ftdLookup(ctx, "B", NULL);
		ftdAlwaysAssert(b != NULL);
		ftdAlwaysAssert(b->type == BlockType_Text);
		// image union memory should NOT have been written (still zero)
		ftdAlwaysAssert(b->image.imageName == NULL);
	}

	ftdDestroy(ctx);
}

static void TestRefAndForwardReference(void) {
	ftdContext *ctx = MakeCommonContext();

	// Backward reference: target defined first
	{
		const char *src =
			"Resource A { name = \"first\"  id = 1 }\n"
			"Linker L { primary = A }\n";
		ftdResult r = ParseLiteral(ctx, src);
		ftdAlwaysAssert(r.ok);
		const Linker *l = (const Linker *)ftdLookup(ctx, "L", NULL);
		ftdAlwaysAssert(l != NULL);
		ftdAlwaysAssert(l->primary != NULL);
		ftdAlwaysAssert(strcmp(l->primary->name, "first") == 0);
		ftdAlwaysAssert(l->primary->id == 1);
	}

	// Forward reference: target defined later (requires fixup)
	{
		ftdResetParse(ctx);
		const char *src =
			"Linker L { primary = Later  secondary = AlsoLater }\n"
			"Resource Later     { name = \"deferred\"  id = 99 }\n"
			"Resource AlsoLater { name = \"deferred2\" id = 100 }\n";
		ftdResult r = ParseLiteral(ctx, src);
		ftdAlwaysAssert(r.ok);
		const Linker *l = (const Linker *)ftdLookup(ctx, "L", NULL);
		ftdAlwaysAssert(l != NULL);
		ftdAlwaysAssert(l->primary != NULL && strcmp(l->primary->name, "deferred")  == 0);
		ftdAlwaysAssert(l->secondary != NULL && strcmp(l->secondary->name, "deferred2") == 0);
	}

	ftdDestroy(ctx);
}

static void TestUnresolvedReferenceWarning(void) {
	ftdContext *ctx = MakeCommonContext();
	const char *src = "Linker L { primary = NonExistent }\n";
	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r.ok); // unresolved fixup is a warning, not an error
	ftdAlwaysAssert(r.warningCount >= 1);
	const Linker *l = (const Linker *)ftdLookup(ctx, "L", NULL);
	ftdAlwaysAssert(l && l->primary == NULL);
	ftdDestroy(ctx);
}

static Resource s_globalRes = { "globalResource", 1234 };

static void TestGlobalRegistration(void) {
	ftdContext *ctx = MakeCommonContext();
	ftdRegisterGlobal(ctx, "ResourceTable.Mine", &Resource_type, &s_globalRes);
	const char *src = "Linker L { primary = ResourceTable.Mine }\n";
	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r.ok);
	const Linker *l = (const Linker *)ftdLookup(ctx, "L", NULL);
	ftdAlwaysAssert(l != NULL);
	ftdAlwaysAssert(l->primary == &s_globalRes);
	ftdAlwaysAssert(strcmp(l->primary->name, "globalResource") == 0);

	// alias for a dotted-path global
	ftdResetParse(ctx);
	{
		const char *src2 =
			"alias MyRes = ResourceTable.Mine\n"
			"Linker L { primary = MyRes }\n";
		ftdResult r2 = ParseLiteral(ctx, src2);
		ftdAlwaysAssert(r2.ok);
		const Linker *l2 = (const Linker *)ftdLookup(ctx, "L", NULL);
		ftdAlwaysAssert(l2 && l2->primary == &s_globalRes);
	}

	ftdDestroy(ctx);
}

// -----------------------------------------------------------------------------
// Namespaces: the untyped `Name { ... }` block. Members are not parsed into a
// C struct - they live only in the parser's symbol table and are reached via
// dotted paths. A member RHS that is a bare qname becomes an alias; anything
// else (literal, constructor, ref) becomes a parse-time const.
// -----------------------------------------------------------------------------

// Scalar / struct / typed-ref members. Verifies that:
//   * a float member parsed as a scalar is fetchable through ftdLookup;
//   * a struct member produced by a constructor (V4f(...)) survives lookup
//     and can be assigned to a struct-typed field elsewhere;
//   * a Ref member that points at a host-registered Global is honored when
//     the field that consumes it is a Ref of the matching type.
static Resource s_nsGlobalRes = { "nsGlobalRes", 7777 };

static void TestNamespaceBasicMembers(void) {
	ftdContext *ctx = MakeCommonContext();
	ftdRegisterGlobal(ctx, "BuiltinRes.Mine", &Resource_type, &s_nsGlobalRes);

	const char *src =
		"MyDefaults {\n"
		"    headerSize = 24.0\n"
		"    accentColor = V4f(0.1, 0.2, 0.3, 1.0)\n"
		"    mainResource = BuiltinRes.Mine\n"
		"}\n"
		"AllScalars S { f32 = MyDefaults.headerSize }\n"
		"Linker      L { primary = MyDefaults.mainResource }\n";

	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r.ok);
	ftdAlwaysAssert(r.errorCount == 0);

	// Direct dotted lookup of the scalar member.
	const double *headerSize = (const double *)ftdLookup(ctx, "MyDefaults.headerSize", NULL);
	ftdAlwaysAssert(headerSize != NULL);
	ftdAlwaysAssert(ftdAlmostEqual(*headerSize, 24.0));

	// Direct dotted lookup of the struct-valued member.
	const ftdType *colorType = NULL;
	const V4f *accent = (const V4f *)ftdLookup(ctx, "MyDefaults.accentColor", &colorType);
	ftdAlwaysAssert(accent != NULL);
	ftdAlwaysAssert(colorType == &V4f_type);
	ftdAlwaysAssert(ftdAlmostEqual(accent->a, 1.0));

	// Direct dotted lookup of the global-ref member should fold through the
	// alias and yield the original Global pointer.
	const Resource *res = (const Resource *)ftdLookup(ctx, "MyDefaults.mainResource", NULL);
	ftdAlwaysAssert(res == &s_nsGlobalRes);

	// Scalar member feeding a struct field at parse time.
	const AllScalars *s = (const AllScalars *)ftdLookup(ctx, "S", NULL);
	ftdAlwaysAssert(s != NULL);
	ftdAlwaysAssert(ftdAlmostEqual(s->f32, 24.0));

	// Ref-of-Resource field receiving a namespace member.
	const Linker *l = (const Linker *)ftdLookup(ctx, "L", NULL);
	ftdAlwaysAssert(l != NULL);
	ftdAlwaysAssert(l->primary == &s_nsGlobalRes);

	ftdDestroy(ctx);
}

// Partial nesting: a member RHS that is itself a namespace name becomes an
// alias to that namespace. Dotted access then transparently folds through
// the layer, so `Outer.sub.member` resolves to `Inner.member`.
static Resource s_nsNestedA = { "nestedA", 1 };
static Resource s_nsNestedB = { "nestedB", 2 };

static void TestNamespaceNested(void) {
	ftdContext *ctx = MakeCommonContext();
	ftdRegisterGlobal(ctx, "Hosted.A", &Resource_type, &s_nsNestedA);
	ftdRegisterGlobal(ctx, "Hosted.B", &Resource_type, &s_nsNestedB);

	const char *src =
		"Inner {\n"
		"    a = Hosted.A\n"
		"    b = Hosted.B\n"
		"}\n"
		"Outer {\n"
		"    sub = Inner\n"
		"    direct = Hosted.A\n"
		"}\n"
		"Linker L1 { primary = Outer.sub.a   secondary = Outer.sub.b }\n"
		"Linker L2 { primary = Outer.direct }\n";

	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r.ok);
	ftdAlwaysAssert(r.errorCount == 0);

	// Nested dotted path folds Outer.sub -> Inner, then resolves .a / .b.
	const Linker *l1 = (const Linker *)ftdLookup(ctx, "L1", NULL);
	ftdAlwaysAssert(l1 != NULL);
	ftdAlwaysAssert(l1->primary   == &s_nsNestedA);
	ftdAlwaysAssert(l1->secondary == &s_nsNestedB);

	// Direct (non-nested) member alongside the nesting still works.
	const Linker *l2 = (const Linker *)ftdLookup(ctx, "L2", NULL);
	ftdAlwaysAssert(l2 != NULL);
	ftdAlwaysAssert(l2->primary == &s_nsNestedA);

	// And the full nested dotted path is reachable from ftdLookup itself.
	const Resource *deep = (const Resource *)ftdLookup(ctx, "Outer.sub.a", NULL);
	ftdAlwaysAssert(deep == &s_nsNestedA);

	ftdDestroy(ctx);
}

// Namespaces share names with file-local constants and named instances, all
// living in the parse-arena symbol table. ftdResetParse must drop them
// alongside everything else.
static void TestNamespaceClearedOnReset(void) {
	ftdContext *ctx = MakeCommonContext();
	ftdRegisterGlobal(ctx, "Builtin.X", &Resource_type, &s_nsGlobalRes);

	const char *src =
		"Cache { x = Builtin.X }\n"
		"Linker L { primary = Cache.x }\n";
	ftdResult r1 = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r1.ok);
	ftdAlwaysAssert(ftdLookup(ctx, "Cache.x", NULL) == &s_nsGlobalRes);

	// A fresh parse with no namespace should not see the previous one.
	const char *src2 = "Linker L { primary = Cache.x }\n";
	ftdResult r2 = ParseLiteral(ctx, src2);
	ftdAlwaysAssert(r2.ok); // unresolved fixup -> warning, not error
	ftdAlwaysAssert(r2.warningCount >= 1);
	const Linker *l = (const Linker *)ftdLookup(ctx, "L", NULL);
	ftdAlwaysAssert(l != NULL);
	ftdAlwaysAssert(l->primary == NULL);

	ftdDestroy(ctx);
}

// Namespaces are NOT parsed into a C struct: there is no host type for them
// and asking ftdLookup for the namespace name itself must return NULL (it
// is not a value). Members fetched via the dotted path are still resolvable.
static void TestNamespaceIsNotAValue(void) {
	ftdContext *ctx = MakeCommonContext();
	const char *src =
		"Pal {\n"
		"    red = V4f(1, 0, 0, 1)\n"
		"}\n";
	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r.ok);

	const ftdType *outType = (const ftdType *)0x1; // sentinel
	const void *p = ftdLookup(ctx, "Pal", &outType);
	ftdAlwaysAssert(p == NULL);
	ftdAlwaysAssert(outType == NULL);

	const V4f *red = (const V4f *)ftdLookup(ctx, "Pal.red", NULL);
	ftdAlwaysAssert(red != NULL && ftdAlmostEqual(red->r, 1.0));

	ftdDestroy(ctx);
}

// Diagnostics for malformed namespaces: dotted namespace name is rejected,
// missing '=' after member name is an error, but the parser still recovers.
static void TestNamespaceErrors(void) {
	ftdContext *ctx = MakeCommonContext();

	// Dotted namespace name is rejected.
	{
		const char *src =
			"Foo.Bar {\n"
			"    x = 1.0\n"
			"}\n";
		ftdResult r = ParseLiteral(ctx, src);
		ftdAlwaysAssert(!r.ok);
		ftdAlwaysAssert(r.errorCount >= 1);
		// member must NOT have been registered under either flat name.
		ftdAlwaysAssert(ftdLookup(ctx, "Foo.Bar.x", NULL) == NULL);
		ftdAlwaysAssert(ftdLookup(ctx, "Bar.x",     NULL) == NULL);
	}

	// Missing '=' after member is an error, parser must recover and continue.
	ftdResetParse(ctx);
	{
		const char *src =
			"NS {\n"
			"    bad\n"
			"    good = 7\n"
			"}\n";
		ftdResult r = ParseLiteral(ctx, src);
		ftdAlwaysAssert(r.errorCount >= 1);
		const int64_t *good = (const int64_t *)ftdLookup(ctx, "NS.good", NULL);
		ftdAlwaysAssert(good != NULL);
		ftdAlwaysAssert(*good == 7);
	}

	ftdDestroy(ctx);
}

static uint64_t ValueToUInt(const ftdValue *v) {
	switch (v->kind) {
		case ftdValueKind_UInt:  return v->as.u;
		case ftdValueKind_Int:   return (uint64_t)v->as.i;
		case ftdValueKind_Float: return (uint64_t)v->as.f;
		default:                 return 0;
	}
}

static bool RGBA24Helper(ftdContext *ctx, const ftdValue *args, uint32_t argCount, void *outValue, void *userData) {
	(void)ctx; (void)userData;
	V4f *o = (V4f *)outValue;
	uint64_t hex = (argCount >= 1) ? ValueToUInt(&args[0]) : 0;
	uint64_t a8  = (argCount >= 2) ? ValueToUInt(&args[1]) : 255;
	o->r = (float)((hex >> 16) & 0xFF) / 255.0f;
	o->g = (float)((hex >>  8) & 0xFF) / 255.0f;
	o->b = (float)((hex      ) & 0xFF) / 255.0f;
	o->a = (float)(a8 & 0xFF) / 255.0f;
	return true;
}

static bool RGBAHelper(ftdContext *ctx, const ftdValue *args, uint32_t argCount, void *outValue, void *userData) {
	(void)ctx; (void)userData;
	V4f *o = (V4f *)outValue;
	uint64_t r = (argCount >= 1) ? ValueToUInt(&args[0]) : 0;
	uint64_t g = (argCount >= 2) ? ValueToUInt(&args[1]) : 0;
	uint64_t b = (argCount >= 3) ? ValueToUInt(&args[2]) : 0;
	uint64_t a = (argCount >= 4) ? ValueToUInt(&args[3]) : 255;
	o->r = (float)(r & 0xFF) / 255.0f;
	o->g = (float)(g & 0xFF) / 255.0f;
	o->b = (float)(b & 0xFF) / 255.0f;
	o->a = (float)(a & 0xFF) / 255.0f;
	return true;
}

static void TestUserRegisteredColorHelpers(void) {
	ftdContext *ctx = MakeCommonContext();
	ftdRegisterHelper(ctx, "RGBA24", &V4f_type, RGBA24Helper, NULL);
	ftdRegisterHelper(ctx, "RGBA",   &V4f_type, RGBAHelper,   NULL);
	const char *src =
		"V4f A = RGBA24(0xFF8000, 200)\n"
		"V4f B = RGBA(255, 0, 0, 255)\n";
	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r.ok);
	const V4f *a = (const V4f *)ftdLookup(ctx, "A", NULL);
	const V4f *b = (const V4f *)ftdLookup(ctx, "B", NULL);
	ftdAlwaysAssert(a != NULL);
	ftdAlwaysAssert(ftdAlmostEqual(a->r, 1.0));
	ftdAlwaysAssert(ftdAlmostEqual(a->g, 128.0 / 255.0));
	ftdAlwaysAssert(ftdAlmostEqual(a->b, 0.0));
	ftdAlwaysAssert(ftdAlmostEqual(a->a, 200.0 / 255.0));
	ftdAlwaysAssert(b != NULL);
	ftdAlwaysAssert(ftdAlmostEqual(b->r, 1.0));
	ftdAlwaysAssert(ftdAlmostEqual(b->g, 0.0));
	ftdAlwaysAssert(ftdAlmostEqual(b->b, 0.0));
	ftdAlwaysAssert(ftdAlmostEqual(b->a, 1.0));
	ftdDestroy(ctx);
}

// Custom helper: Double(x) -> V2f { x*2, x*2 }
static bool DoubleHelper(ftdContext *ctx, const ftdValue *args, uint32_t argCount, void *outValue, void *userData) {
	(void)ctx; (void)userData;
	float val = 0.0f;
	if (argCount >= 1) {
		if (args[0].kind == ftdValueKind_Float) {
			val = (float)args[0].as.f;
		} else if (args[0].kind == ftdValueKind_Int) {
			val = (float)args[0].as.i;
		}
	}
	V2f *out = (V2f *)outValue;
	out->x = val * 2.0f;
	out->y = val * 2.0f;
	return true;
}

static void TestCustomHelper(void) {
	ftdContext *ctx = MakeCommonContext();
	ftdRegisterHelper(ctx, "Double", &V2f_type, DoubleHelper, NULL);
	const char *src = "V2f P = Double(3)\n";
	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r.ok);
	const V2f *p = (const V2f *)ftdLookup(ctx, "P", NULL);
	ftdAlwaysAssert(p && ftdAlmostEqual(p->x, 6.0) && ftdAlmostEqual(p->y, 6.0));
	ftdDestroy(ctx);
}

static void TestConstantInterning(void) {
	ftdContext *ctx = MakeCommonContext();
	const char *src =
		"float TitleSize = 64.0\n"
		"V2f   Origin    = V2f(0, 0)\n"
		"AllScalars S { f32 = TitleSize  i32 = 7 }\n";
	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r.ok);
	const AllScalars *s = (const AllScalars *)ftdLookup(ctx, "S", NULL);
	ftdAlwaysAssert(s && ftdAlmostEqual(s->f32, 64.0) && s->i32 == 7);
	const V2f *o = (const V2f *)ftdLookup(ctx, "Origin", NULL);
	ftdAlwaysAssert(o && ftdAlmostEqual(o->x, 0.0) && ftdAlmostEqual(o->y, 0.0));
	ftdDestroy(ctx);
}

static void TestUnknownFieldWarningRecovery(void) {
	ftdContext *ctx = MakeCommonContext();
	const char *src =
		"Block B {\n"
		"    type = Text\n"
		"    bogusField = 42\n"
		"    text { content = \"after\"  fontSize = 8 }\n"
		"}\n";
	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r.ok);
	ftdAlwaysAssert(r.warningCount >= 1);
	const Block *b = (const Block *)ftdLookup(ctx, "B", NULL);
	ftdAlwaysAssert(b != NULL);
	ftdAlwaysAssert(strcmp(b->text.content, "after") == 0);
	ftdAlwaysAssert(ftdAlmostEqual(b->text.fontSize, 8.0));
	ftdDestroy(ctx);
}

static void TestUnknownTypeError(void) {
	ftdContext *ctx = MakeCommonContext();
	const char *src =
		"NoSuchType Bad = 1\n"
		"Block Good { type = Text  text { content = \"good\"  fontSize = 1 } }\n";
	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r.ok == false);
	ftdAlwaysAssert(r.errorCount >= 1);
	// Recovery should still pick up Good
	const Block *g = (const Block *)ftdLookup(ctx, "Good", NULL);
	ftdAlwaysAssert(g && strcmp(g->text.content, "good") == 0);
	ftdDestroy(ctx);
}

static void TestDiagnosticsHaveSpans(void) {
	ftdContext *ctx = MakeCommonContext();
	const char *src = "NoSuchType X = 1\n";
	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(!r.ok);
	ftdAlwaysAssert(r.diagnosticCount >= 1);
	for (uint32_t i = 0; i < r.diagnosticCount; ++i) {
		const ftdDiagnostic *d = &r.diagnostics[i];
		ftdAlwaysAssert(d->message != NULL);
		ftdAlwaysAssert(d->span.line >= 1);
	}
	ftdDestroy(ctx);
}

static void TestResetParseHotReload(void) {
	ftdContext *ctx = MakeCommonContext();

	// First parse
	{
		const char *src = "V2f P = V2f(1, 2)\n";
		ftdResult r = ParseLiteral(ctx, src);
		ftdAlwaysAssert(r.ok);
		const V2f *p = (const V2f *)ftdLookup(ctx, "P", NULL);
		ftdAlwaysAssert(p && ftdAlmostEqual(p->x, 1.0));
	}

	// Second parse via ftdParseString — must reset internally
	{
		const char *src = "V2f P = V2f(7, 8)\n";
		ftdResult r = ParseLiteral(ctx, src);
		ftdAlwaysAssert(r.ok);
		const V2f *p = (const V2f *)ftdLookup(ctx, "P", NULL);
		ftdAlwaysAssert(p && ftdAlmostEqual(p->x, 7.0) && ftdAlmostEqual(p->y, 8.0));
	}

	// Explicit reset then parse with no symbols — old P must be gone
	ftdResetParse(ctx);
	{
		const V2f *p = (const V2f *)ftdLookup(ctx, "P", NULL);
		ftdAlwaysAssert(p == NULL);
	}

	// Schema is preserved after reset
	{
		const char *src = "V2f Q = V2f(11, 22)\n";
		ftdResult r = ParseLiteral(ctx, src);
		ftdAlwaysAssert(r.ok);
		const V2f *q = (const V2f *)ftdLookup(ctx, "Q", NULL);
		ftdAlwaysAssert(q && ftdAlmostEqual(q->x, 11.0));
	}

	ftdDestroy(ctx);
}

static void TestLookupNotFound(void) {
	ftdContext *ctx = MakeCommonContext();
	const ftdType *outType = (const ftdType *)0xDEADBEEF;
	const void *p = ftdLookup(ctx, "DoesNotExist", &outType);
	ftdAlwaysAssert(p == NULL);
	ftdAlwaysAssert(outType == NULL);
	ftdDestroy(ctx);
}

static void TestLookupReturnsType(void) {
	ftdContext *ctx = MakeCommonContext();
	const char *src = "V2f P = V2f(1, 2)\n";
	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r.ok);
	const ftdType *outType = NULL;
	const void *p = ftdLookup(ctx, "P", &outType);
	ftdAlwaysAssert(p != NULL);
	ftdAlwaysAssert(outType == &V2f_type);
	ftdDestroy(ctx);
}

// Custom allocator that tracks bytes
typedef struct CountingAllocator {
	size_t   totalAllocated;
	uint32_t allocCount;
	uint32_t freeCount;
} CountingAllocator;

static void *CountingAlloc(size_t size, size_t align, void *userData) {
	(void)align;
	CountingAllocator *ca = (CountingAllocator *)userData;
	ca->totalAllocated += size;
	ca->allocCount++;
	void *p = calloc(1, size);
	return p;
}
static void CountingFree(void *ptr, void *userData) {
	CountingAllocator *ca = (CountingAllocator *)userData;
	ca->freeCount++;
	free(ptr);
}

static void TestCustomAllocator(void) {
	CountingAllocator counter = { 0, 0, 0 };
	ftdAllocator al = { CountingAlloc, CountingFree, &counter };
	ftdContext *ctx = ftdCreate(&al);
	ftdAlwaysAssert(ctx != NULL);
	ftdAlwaysAssert(counter.allocCount >= 1);

	ftdRegisterStruct(ctx, &V2f_type);
	const char *src = "V2f P = V2f(1, 2)\n";
	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r.ok);

	ftdDestroy(ctx);
	// Every allocation must have been freed
	ftdAlwaysAssert(counter.allocCount == counter.freeCount);
}

static void TestNullAndBoolLiteral(void) {
	ftdContext *ctx = MakeCommonContext();
	const char *src =
		"AllScalars S {\n"
		"    b   = false\n"
		"    str = null\n"
		"}\n";
	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r.ok);
	const AllScalars *s = (const AllScalars *)ftdLookup(ctx, "S", NULL);
	ftdAlwaysAssert(s != NULL);
	ftdAlwaysAssert(s->b == false);
	ftdAlwaysAssert(s->str == NULL);
	ftdDestroy(ctx);
}

static void TestRepeatedFieldLastWins(void) {
	ftdContext *ctx = MakeCommonContext();
	const char *src =
		"V2f P {\n"
		"    x = 1\n"
		"    x = 2\n"
		"    x = 3\n"
		"    y = 99\n"
		"}\n";
	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r.ok);
	const V2f *p = (const V2f *)ftdLookup(ctx, "P", NULL);
	ftdAlwaysAssert(p && ftdAlmostEqual(p->x, 3.0) && ftdAlmostEqual(p->y, 99.0));
	ftdDestroy(ctx);
}

static void TestCommaAndNewlineSeparators(void) {
	ftdContext *ctx = MakeCommonContext();
	// Mix commas and newlines in array
	const char *src =
		"Slide S {\n"
		"    title = \"sep\"\n"
		"    blocks = [\n"
		"        Block { type = Text  text { content = \"a\"  fontSize = 1 } },\n"
		"        Block { type = Text  text { content = \"b\"  fontSize = 2 } }\n"
		"        Block { type = Text  text { content = \"c\"  fontSize = 3 } },\n"
		"    ]\n"
		"}\n";
	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r.ok);
	const Slide *s = (const Slide *)ftdLookup(ctx, "S", NULL);
	ftdAlwaysAssert(s && s->blocks.count == 3);
	ftdDestroy(ctx);
}

// Decomposed array via the new ftdFieldKind_ArrayData/Count/Capacity field
// kinds. The host type stores items as a plain pointer + count (+ optional
// capacity); the .ftd source still writes `blocks = [ ... ]` as usual.
typedef struct BundleHost {
	const char *title;
	Block      *items;     // blocks.data
	uint32_t    itemCount; // blocks.count
	uint32_t    itemCap;   // blocks.capacity
} BundleHost;

static const ftdField BundleHost_fields[] = {
	{ "title",           offsetof(BundleHost, title),     ftdFieldKind_String,        NULL,        NULL, 0, 0 },
	{ "blocks.data",     offsetof(BundleHost, items),     ftdFieldKind_ArrayData,     &Block_type, NULL, 0, 0 },
	{ "blocks.count",    offsetof(BundleHost, itemCount), ftdFieldKind_ArrayCount,    NULL,        NULL, 0, 0 },
	{ "blocks.capacity", offsetof(BundleHost, itemCap),   ftdFieldKind_ArrayCapacity, NULL,        NULL, 0, 0 },
};
static const ftdType BundleHost_type = {
	"BundleHost", sizeof(BundleHost), _Alignof(BundleHost),
	BundleHost_fields, 4,
	NULL, 0
};

// Same as BundleHost but only the .data + .count fields are declared —
// .capacity is omitted to verify it stays optional.
typedef struct BundleHostNoCap {
	Block    *items;
	uint32_t  itemCount;
} BundleHostNoCap;

static const ftdField BundleHostNoCap_fields[] = {
	{ "blocks.data",  offsetof(BundleHostNoCap, items),     ftdFieldKind_ArrayData,  &Block_type, NULL, 0, 0 },
	{ "blocks.count", offsetof(BundleHostNoCap, itemCount), ftdFieldKind_ArrayCount, NULL,        NULL, 0, 0 },
};
static const ftdType BundleHostNoCap_type = {
	"BundleHostNoCap", sizeof(BundleHostNoCap), _Alignof(BundleHostNoCap),
	BundleHostNoCap_fields, 2,
	NULL, 0
};

static void TestArrayBundleFields(void) {
	ftdContext *ctx = MakeCommonContext();
	ftdRegisterStruct(ctx, &BundleHost_type);

	const char *src =
		"BundleHost B {\n"
		"    title = \"bundle\"\n"
		"    blocks = [\n"
		"        Block { type = Text   text  { content = \"a\" fontSize = 1 } }\n"
		"        Block { type = Image  image { imageName = \"x.png\" size = V2f(8, 8) } }\n"
		"        Block { type = Text   text  { content = \"c\" fontSize = 3 } }\n"
		"    ]\n"
		"}\n";
	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r.ok);

	const BundleHost *b = (const BundleHost *)ftdLookup(ctx, "B", NULL);
	ftdAlwaysAssert(b != NULL);
	ftdAlwaysAssert(strcmp(b->title, "bundle") == 0);
	ftdAlwaysAssert(b->itemCount == 3);
	ftdAlwaysAssert(b->itemCap   >= 3);
	ftdAlwaysAssert(b->items     != NULL);
	ftdAlwaysAssert(b->items[0].type == BlockType_Text);
	ftdAlwaysAssert(strcmp(b->items[0].text.content, "a") == 0);
	ftdAlwaysAssert(b->items[1].type == BlockType_Image);
	ftdAlwaysAssert(strcmp(b->items[1].image.imageName, "x.png") == 0);
	ftdAlwaysAssert(b->items[2].type == BlockType_Text);
	ftdAlwaysAssert(strcmp(b->items[2].text.content, "c") == 0);

	// Sentinel: arrays always own one trailing zero-initialized element past
	// the last real entry, so itemCap > itemCount and items[itemCount] is
	// fully zero. The empty array case is the simplest: count=0, but data
	// is still a valid one-slot zero buffer.
	ftdAlwaysAssert(b->itemCap > b->itemCount);
	{
		const Block *sentinel = &b->items[b->itemCount];
		ftdAlwaysAssert(sentinel->type == 0);
		ftdAlwaysAssert(sentinel->text.content == NULL);
	}

	ftdResetParse(ctx);
	{
		const char *src2 = "BundleHost B { blocks = [] }\n";
		ftdResult r2 = ParseLiteral(ctx, src2);
		ftdAlwaysAssert(r2.ok);
		const BundleHost *b2 = (const BundleHost *)ftdLookup(ctx, "B", NULL);
		ftdAlwaysAssert(b2 != NULL);
		ftdAlwaysAssert(b2->itemCount == 0);
		// Sentinel-only: data is allocated and zero so callers can do
		// `for (Block *e = b2->items; e->type != 0 || e->text.content != NULL; ++e)`
		// without checking itemCount.
		ftdAlwaysAssert(b2->items != NULL);
		ftdAlwaysAssert(b2->itemCap >= 1);
		ftdAlwaysAssert(b2->items[0].type == 0);
		ftdAlwaysAssert(b2->items[0].text.content == NULL);
	}

	ftdDestroy(ctx);
}

static void TestArrayBundleOptionalCapacity(void) {
	ftdContext *ctx = MakeCommonContext();
	ftdRegisterStruct(ctx, &BundleHostNoCap_type);

	const char *src =
		"BundleHostNoCap B {\n"
		"    blocks = [\n"
		"        Block { type = Text  text { content = \"only\" fontSize = 9 } }\n"
		"    ]\n"
		"}\n";
	ftdResult r = ParseLiteral(ctx, src);
	ftdAlwaysAssert(r.ok);

	const BundleHostNoCap *b = (const BundleHostNoCap *)ftdLookup(ctx, "B", NULL);
	ftdAlwaysAssert(b != NULL);
	ftdAlwaysAssert(b->itemCount == 1);
	ftdAlwaysAssert(b->items != NULL);
	ftdAlwaysAssert(strcmp(b->items[0].text.content, "only") == 0);

	ftdDestroy(ctx);
}

#if !defined(FTD_NO_STDIO)
static void TestParseFile(void) {
	const char *path = "ftd_test_tmp.ftd";
	FILE *fp = fopen(path, "wb");
	ftdAlwaysAssert(fp != NULL);
	const char *src =
		"V2f From_File = V2f(123, 456)\n";
	fwrite(src, 1, strlen(src), fp);
	fclose(fp);

	ftdContext *ctx = MakeCommonContext();
	ftdResult r = ftdParseFile(ctx, path);
	ftdAlwaysAssert(r.ok);
	const V2f *p = (const V2f *)ftdLookup(ctx, "From_File", NULL);
	ftdAlwaysAssert(p && ftdAlmostEqual(p->x, 123.0) && ftdAlmostEqual(p->y, 456.0));
	ftdDestroy(ctx);

	remove(path);
}

static void TestParseMissingFile(void) {
	ftdContext *ctx = MakeCommonContext();
	ftdResult r = ftdParseFile(ctx, "no_such_file_exists.ftd");
	ftdAlwaysAssert(!r.ok);
	ftdAlwaysAssert(r.errorCount >= 1);
	ftdDestroy(ctx);
}
#endif

// Parse an inline source against the full presentation schema while using
// *no* aliases (neither file-local nor host-registered). The schema types
// (Vec2f / Vec4f / Block / Slide / ...) and the RGBA helper are addressed
// directly by their registered names, the enum values by their fully
// qualified path, and the host-side resources by their dotted-path globals.
// This isolates "does the parser handle the full type system end-to-end"
// from "does alias resolution work".
static void TestParsePresentationFullNoAliases(void) {
	ftdContext *ctx = ftdCreate(NULL);
	ftdAlwaysAssert(ctx != NULL);
	FplPresentationTypes_Register(ctx);

	const char *src =
		"BackgroundStyle BG {\n"
		"    kind           = BackgroundKind.HalfGradientHorizontal\n"
		"    primaryColor   = RGBA24(0x000000, 255)\n"
		"    secondaryColor = RGBA(15, 13, 80, 255)\n"
		"}\n"
		"\n"
		"TextStyle TS {\n"
		"    foregroundColor = Vec4f(1, 1, 1, 1)\n"
		"    shadowColor     = Vec4f(0, 0, 0, 1)\n"
		"    shadowOffset    = Vec2f(2, 1)\n"
		"    drawShadow      = true\n"
		"}\n"
		"\n"
		"Slide OnlySlide {\n"
		"    title      = \"NoAliases\"\n"
		"    duration   = 7.5\n"
		"    rotation   = Rotation{ angleDegrees = 30.0, axis = Vec3f(0, 1, 0) }\n"
		"    background = BG\n"
		"    talk       = \"single line\"\n"
		"    blocks = [\n"
		"        Block {\n"
		"            type = BlockType.Text\n"
		"            pos  = Vec2f(0.1, 0.2)\n"
		"            size = Vec2f(0.5, 0.5)\n"
		"            contentAlignment = ContentAlignment{ h = HorizontalAlignment.Center, v = VerticalAlignment.Middle }\n"
		"            text { text = \"hi\"  textAlign = TextAlign.Center  fontSize = 24.0  color = Vec4f(1,1,1,1) }\n"
		"        }\n"
		"        Block {\n"
		"            type = BlockType.Image\n"
		"            pos  = Vec2f(0.0, 0.0)\n"
		"            size = Vec2f(1.0, 1.0)\n"
		"            contentAlignment = ContentAlignment{ h = HorizontalAlignment.Left, v = VerticalAlignment.Top }\n"
		"            image { imageResource = ImageResources.MagicHat  size = Vec2f(0.8, 0.8)  keepAspect = true  tintColor = Vec4f(1,1,1,1) }\n"
		"        }\n"
		"    ]\n"
		"    sounds = [\n"
		"        SoundInstance{ name = SoundResources.Intro1  startTime = 1.0 }\n"
		"    ]\n"
		"}\n"
		"\n"
		"HeaderDefinition H {\n"
		"    font { name = FontResources.Arimo  size = 24.0  lineScale = 1.15  style = TS }\n"
		"    height = 32.0  leftText = \"L\"  centerText = \"\"  rightText = \"\"  padding = Vec2f(2, 2)\n"
		"}\n"
		"FooterDefinition F {\n"
		"    font { name = FontResources.Arimo  size = 24.0  lineScale = 1.15  style = TS }\n"
		"    height = 32.0  leftText = \"l\"  centerText = \"c\"  rightText = \"r\"  padding = Vec2f(2, 3)\n"
		"}\n"
		"\n"
		"Presentation P {\n"
		"    slideSize = Vec2f(1280, 720)\n"
		"    padding   = 20.0\n"
		"    slides    = [ OnlySlide ]\n"
		"    header    = H\n"
		"    footer    = F\n"
		"    titleFont   { name = FontResources.Arimo              size = 64.0  lineScale = 1.15  style = TS }\n"
		"    normalFont  { name = FontResources.Arimo              size = 42.0  lineScale = 1.15  style = TS }\n"
		"    consoleFont { name = FontResources.BitStreamVerySans  size = 36.0  lineScale = 1.15  style = TS }\n"
		"}\n";

	ftdResult r = ftdParseString(ctx, src, strlen(src), "<noaliases>");
	if (!r.ok) {
		for (uint32_t i = 0; i < r.diagnosticCount && i < 20; ++i) {
			const ftdDiagnostic *d = &r.diagnostics[i];
			fprintf(stderr, "  diag[%u] %s:%u:%u: sev=%d %s\n",
				i, d->span.file ? d->span.file : "?",
				d->span.line, d->span.column,
				(int)d->severity, d->message);
		}
	}
	ftdAlwaysAssert(r.ok);
	ftdAlwaysAssert(r.errorCount == 0);

	const PresentationRoot *p = (const PresentationRoot *)ftdLookup(ctx, "P", NULL);
	ftdAlwaysAssert(p != NULL);
	ftdAlwaysAssert(ftdAlmostEqual(p->slideSize.x, 1280.0));
	ftdAlwaysAssert(p->slides.count == 1);
	ftdAlwaysAssert(p->header != NULL && strcmp(p->header->leftText, "L") == 0);
	ftdAlwaysAssert(p->footer != NULL && strcmp(p->footer->centerText, "c") == 0);

	const PresentationSlide *s = (const PresentationSlide *)ftdLookup(ctx, "OnlySlide", NULL);
	ftdAlwaysAssert(s != NULL);
	ftdAlwaysAssert(strcmp(s->title, "NoAliases") == 0);
	ftdAlwaysAssert(ftdAlmostEqual(s->duration, 7.5));
	ftdAlwaysAssert(s->background != NULL && s->background->kind == BackgroundKind_HalfGradientHorizontal);
	ftdAlwaysAssert(s->blocks.count == 2);
	ftdAlwaysAssert(s->sounds.count == 1);

	const PresentationBlock *blocks = (const PresentationBlock *)s->blocks.data;
	ftdAlwaysAssert(blocks[0].type == PresentationBlockType_Text);
	ftdAlwaysAssert(strcmp(blocks[0].text.text, "hi") == 0);
	ftdAlwaysAssert(blocks[1].type == PresentationBlockType_Image);
	ftdAlwaysAssert(blocks[1].image.imageResource == &ImageRes_MagicHat);
	ftdAlwaysAssert(blocks[1].image.keepAspect == true);

	const SoundInstance *sounds = (const SoundInstance *)s->sounds.data;
	ftdAlwaysAssert(sounds[0].name == &SoundRes_Intro1);
	ftdAlwaysAssert(ftdAlmostEqual(sounds[0].startTime, 1.0));

	const PresentationSlideRef *refs = (const PresentationSlideRef *)p->slides.data;
	ftdAlwaysAssert(refs[0].slide == s);

	ftdDestroy(ctx);
}

// Vice-versa of TestParsePresentationFullNoAliases: register *only* aliases
// (no struct/enum types, no helpers, no globals). The aliases point at
// names that resolve to nothing, so every typed declaration should fail
// with an "unknown type" error. What we care about: the parser still
// terminates cleanly and reports those errors instead of looping or
// crashing on stale alias chains.
static void TestParsePresentationAliasesNoTypes(void) {
	ftdContext *ctx = ftdCreate(NULL);
	ftdAlwaysAssert(ctx != NULL);

	// Aliases identical to the ones in fpl_presentation.ftd, plus a couple of
	// enum-value aliases. None of the targets are registered.
	ftdRegisterAlias(ctx, "V2f",    "Vec2f");
	ftdRegisterAlias(ctx, "V3f",    "Vec3f");
	ftdRegisterAlias(ctx, "V4f",    "Vec4f");
	ftdRegisterAlias(ctx, "Left",   "HorizontalAlignment.Left");
	ftdRegisterAlias(ctx, "Center", "HorizontalAlignment.Center");
	ftdRegisterAlias(ctx, "Top",    "VerticalAlignment.Top");
	ftdRegisterAlias(ctx, "Arimo",  "FontResources.Arimo");

	const char *src =
		"V2f A = V2f(1, 2)\n"
		"V4f W = V4f(1, 1, 1, 1)\n"
		"Slide S {\n"
		"    title    = \"x\"\n"
		"    rotation = { angleDegrees = 0.0, axis = V3f(1, 1, 1) }\n"
		"}\n"
		"BackgroundStyle BG { kind = HalfGradientHorizontal }\n";

	ftdResult r = ftdParseString(ctx, src, strlen(src), "<aliases-only>");
	// Parser must return; errors are expected (unknown target types).
	ftdAlwaysAssert(r.diagnosticCount == r.errorCount + r.warningCount);
	ftdAlwaysAssert(r.errorCount > 0);
	// All names declared in the source must be missing from lookup.
	ftdAlwaysAssert(ftdLookup(ctx, "A",  NULL) == NULL);
	ftdAlwaysAssert(ftdLookup(ctx, "W",  NULL) == NULL);
	ftdAlwaysAssert(ftdLookup(ctx, "S",  NULL) == NULL);
	ftdAlwaysAssert(ftdLookup(ctx, "BG", NULL) == NULL);

	ftdDestroy(ctx);
}

// -----------------------------------------------------------------------------
// Presentation: the source is embedded via fpl_presentation_ftd_data.h so the
// test does not depend on the current working directory or filesystem layout.
// These tests do not need ftdParseFile, so they run even with FTD_NO_STDIO.
// -----------------------------------------------------------------------------

// Parse the full presentation source with NO host types registered at all.
// This is purely a robustness check: the parser must not hang or crash on
// unknown types, unknown helpers, or stray closing delimiters left behind
// while it recovers. We expect lots of errors here — that is fine.
static void TestParsePresentationNoTypesRegistered(void) {
	ftdContext *ctx = ftdCreate(NULL);
	ftdAlwaysAssert(ctx != NULL);
	ftdResult r = ftdParseString(ctx,
		fpl_presentation_ftd, sizeof(fpl_presentation_ftd) - 1,
		"fpl_presentation.ftd");
	// Lots of errors are OK; what we *require* is that the parser returned
	// and that the diagnostic count is finite. A failure here used to manifest
	// as an infinite loop / hang.
	ftdAlwaysAssert(r.diagnosticCount == r.errorCount + r.warningCount);
	ftdAlwaysAssert(r.errorCount > 0); // unknown types must be reported
	ftdDestroy(ctx);
}

// Parse the full presentation source with the schema from fpl_presentation_types.h
// fully registered. This verifies that a real-world FTD document round-trips
// cleanly: types, enums, aliases, helpers, host globals, refs, unions and
// arrays all working together.
static void TestParsePresentationFull(void) {
	ftdContext *ctx = ftdCreate(NULL);
	ftdAlwaysAssert(ctx != NULL);
	FplPresentationTypes_Register(ctx);

	ftdResult r = ftdParseString(ctx,
		fpl_presentation_ftd, sizeof(fpl_presentation_ftd) - 1,
		"fpl_presentation.ftd");
	if (!r.ok) {
		for (uint32_t i = 0; i < r.diagnosticCount && i < 20; ++i) {
			const ftdDiagnostic *d = &r.diagnostics[i];
			fprintf(stderr, "  diag[%u] %s:%u:%u: sev=%d %s\n",
				i, d->span.file ? d->span.file : "?",
				d->span.line, d->span.column,
				(int)d->severity, d->message);
		}
	}
	ftdAlwaysAssert(r.ok);
	ftdAlwaysAssert(r.errorCount == 0);

	// Root presentation object
	const ftdType *rootType = NULL;
	const PresentationRoot *root = (const PresentationRoot *)ftdLookup(ctx, "FPLPresentation", &rootType);
	ftdAlwaysAssert(root != NULL);
	ftdAlwaysAssert(rootType == &PresentationRoot_type);
	ftdAlwaysAssert(ftdAlmostEqual(root->slideSize.x, 1280.0));
	ftdAlwaysAssert(ftdAlmostEqual(root->slideSize.y, 720.0));
	ftdAlwaysAssert(ftdAlmostEqual(root->padding,     20.0));
	ftdAlwaysAssert(root->slides.count == 13);
	ftdAlwaysAssert(root->slides.data  != NULL);
	ftdAlwaysAssert(root->header != NULL);
	ftdAlwaysAssert(root->footer != NULL);

	// Header / Footer
	ftdAlwaysAssert(strcmp(root->header->leftText, "Final-Platform-Layer") == 0);
	ftdAlwaysAssert(root->footer->leftText != NULL);
	ftdAlwaysAssert(strstr(root->footer->centerText, "Torsten Spaete") != NULL);

	// Top-level constants
	const float *titleSize = (const float *)ftdLookup(ctx, "TitleFontSize", NULL);
	ftdAlwaysAssert(titleSize != NULL);
	ftdAlwaysAssert(ftdAlmostEqual(*titleSize, 64.0));

	const Vec4f *white = (const Vec4f *)ftdLookup(ctx, "White", NULL);
	ftdAlwaysAssert(white != NULL);
	ftdAlwaysAssert(ftdAlmostEqual(white->x, 1.0));
	ftdAlwaysAssert(ftdAlmostEqual(white->w, 1.0));

	// Named slide instance
	const PresentationSlide *intro = (const PresentationSlide *)ftdLookup(ctx, "Intro", NULL);
	ftdAlwaysAssert(intro != NULL);
	ftdAlwaysAssert(intro->title != NULL && strcmp(intro->title, "Introduction") == 0);
	ftdAlwaysAssert(ftdAlmostEqual(intro->duration, 18.5));
	ftdAlwaysAssert(intro->background != NULL);
	ftdAlwaysAssert(intro->background->kind == BackgroundKind_HalfGradientHorizontal);
	ftdAlwaysAssert(intro->blocks.count == 1);
	ftdAlwaysAssert(intro->sounds.count == 2);

	// Resolve the first slide ref in the root's `slides` array.
	const PresentationSlideRef *slideRefs = (const PresentationSlideRef *)root->slides.data;
	ftdAlwaysAssert(slideRefs[0].slide == intro);

	// Spot-check a couple of the later slides
	const PresentationSlide *features = (const PresentationSlide *)ftdLookup(ctx, "Features", NULL);
	ftdAlwaysAssert(features != NULL);
	ftdAlwaysAssert(features->blocks.count == 20);

	const PresentationSlide *thanks = (const PresentationSlide *)ftdLookup(ctx, "Thanks", NULL);
	ftdAlwaysAssert(thanks != NULL);
	ftdAlwaysAssert(strcmp(thanks->title, "Thanks!") == 0);

	ftdDestroy(ctx);
}

// -----------------------------------------------------------------------------
// Stress: many top-level statements
// -----------------------------------------------------------------------------
static void TestStressMany(void) {
	ftdContext *ctx = MakeCommonContext();
	// Build a large source with many V2f declarations.
	size_t cap = 65536;
	char *buf = (char *)malloc(cap);
	ftdAlwaysAssert(buf != NULL);
	size_t len = 0;
	int n = 256;
	for (int i = 0; i < n; ++i) {
		char line[64];
		int w = snprintf(line, sizeof(line), "V2f P%d = V2f(%d, %d)\n", i, i, i * 2);
		if (len + (size_t)w >= cap) {
			cap *= 2;
			buf = (char *)realloc(buf, cap);
			ftdAlwaysAssert(buf != NULL);
		}
		memcpy(buf + len, line, (size_t)w);
		len += (size_t)w;
	}
	buf[len] = '\0';

	ftdResult r = ftdParseString(ctx, buf, len, "stress");
	ftdAlwaysAssert(r.ok);

	for (int i = 0; i < n; ++i) {
		char key[32];
		snprintf(key, sizeof(key), "P%d", i);
		const V2f *p = (const V2f *)ftdLookup(ctx, key, NULL);
		ftdAlwaysAssert(p != NULL);
		ftdAlwaysAssert(ftdAlmostEqual(p->x, (double)i));
		ftdAlwaysAssert(ftdAlmostEqual(p->y, (double)(i * 2)));
	}

	free(buf);
	ftdDestroy(ctx);
}

int main(int argc, char **args) {
	(void)argc; (void)args;

	TestContextLifecycle();
	TestEmptySource();
	TestComments();
	TestNumberLiterals();
	TestStringEscapesAndConcatenation();
	TestAllScalarsInStruct();
	TestCoercion();
	TestAlias();
	TestRegisterAliasFromHost();
	TestNestedStructAndDottedPath();
	TestPositionalAndInlineForms();
	TestPositionalMissingArgsZeroInit();
	TestEnumShortAndQualified();
	TestArrayDefaultHandle();
	TestArrayEmpty();
	TestArrayCustomSlot();
	TestUnionMatchAndMismatch();
	TestRefAndForwardReference();
	TestUnresolvedReferenceWarning();
	TestGlobalRegistration();
	TestNamespaceBasicMembers();
	TestNamespaceNested();
	TestNamespaceClearedOnReset();
	TestNamespaceIsNotAValue();
	TestNamespaceErrors();
	TestUserRegisteredColorHelpers();
	TestCustomHelper();
	TestConstantInterning();
	TestUnknownFieldWarningRecovery();
	TestUnknownTypeError();
	TestDiagnosticsHaveSpans();
	TestResetParseHotReload();
	TestLookupNotFound();
	TestLookupReturnsType();
	TestCustomAllocator();
	TestNullAndBoolLiteral();
	TestRepeatedFieldLastWins();
	TestCommaAndNewlineSeparators();
	TestArrayBundleFields();
	TestArrayBundleOptionalCapacity();
#if !defined(FTD_NO_STDIO)
	TestParseFile();
	TestParseMissingFile();
#endif
	TestParsePresentationNoTypesRegistered();
	TestParsePresentationFull();
	TestParsePresentationFullNoAliases();
	TestParsePresentationAliasesNoTypes();
	TestStressMany();

	printf("All FTD tests passed.\n");
	return 0;
}
