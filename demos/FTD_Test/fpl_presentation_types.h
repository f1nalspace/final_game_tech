/*
-------------------------------------------------------------------------------
Name:
	FPL Presentation Types

Description:
	Host-side C types and FTD schema descriptors for the presentation file
	fpl_presentation.ftd. Used by the FTD_Test demo to verify that a full,
	realistic FTD document can be parsed against a complete schema.

	This header is intentionally self-contained: it declares both the C
	structs and the matching ftdType / ftdField / ftdEnumValue tables, plus
	a single setup function (FplPresentationTypes_Register) that registers
	everything (types, enums, aliases for value enum names, the RGBA / RGBA24
	helpers, and the host-side resource globals).

Author:
	Torsten Spaete
-------------------------------------------------------------------------------
*/

#ifndef FPL_PRESENTATION_TYPES_H
#define FPL_PRESENTATION_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <final_ftd.h>

// =============================================================================
// Math
// =============================================================================
typedef struct Vec2f { float x, y; } Vec2f;
typedef struct Vec3f { float x, y, z; } Vec3f;
typedef struct Vec4f { float x, y, z, w; } Vec4f;

// =============================================================================
// Enums
// =============================================================================
typedef enum HorizontalAlignment {
	HorizontalAlignment_Left   = 0,
	HorizontalAlignment_Center = 1,
	HorizontalAlignment_Right  = 2,
} HorizontalAlignment;

typedef enum VerticalAlignment {
	VerticalAlignment_Top    = 0,
	VerticalAlignment_Middle = 1,
	VerticalAlignment_Bottom = 2,
} VerticalAlignment;

typedef enum TextAlign {
	TextAlign_Left   = 0,
	TextAlign_Center = 1,
	TextAlign_Right  = 2,
} TextAlign;

typedef enum BackgroundKind {
	BackgroundKind_Solid                  = 0,
	BackgroundKind_HalfGradientHorizontal = 1,
	BackgroundKind_HalfGradientVertical   = 2,
} BackgroundKind;

typedef enum PresentationBlockType {
	PresentationBlockType_Text  = 0,
	PresentationBlockType_Image = 1,
} PresentationBlockType;

// =============================================================================
// Host-registered resources (referenced by FontResources.Foo etc.)
// =============================================================================
typedef struct FontResource {
	const char *name;
} FontResource;

typedef struct ImageResource {
	const char *name;
} ImageResource;

typedef struct SoundResource {
	const char *name;
} SoundResource;

// =============================================================================
// Composite structs
// =============================================================================
typedef struct Rotation {
	float angleDegrees;
	Vec3f axis;
} Rotation;

typedef struct ContentAlignment {
	HorizontalAlignment h;
	VerticalAlignment   v;
} ContentAlignment;

typedef struct BackgroundStyle {
	BackgroundKind kind;
	Vec4f          primaryColor;
	Vec4f          secondaryColor;
} BackgroundStyle;

typedef struct TextStyle {
	Vec4f foregroundColor;
	Vec4f shadowColor;
	Vec2f shadowOffset;
	bool  drawShadow;
} TextStyle;

typedef struct PresentationTextBlock {
	const char *text;
	TextAlign   textAlign;
	float       fontSize;
	Vec4f       color;
} PresentationTextBlock;

typedef struct PresentationImageBlock {
	const ImageResource *imageResource;
	Vec2f                size;
	bool                 keepAspect;
	Vec4f                tintColor;
} PresentationImageBlock;

typedef struct PresentationBlock {
	PresentationBlockType type;
	Vec2f                 pos;
	Vec2f                 size;
	ContentAlignment      contentAlignment;
	union {
		PresentationTextBlock  text;
		PresentationImageBlock image;
	};
} PresentationBlock;

typedef struct SoundInstance {
	const SoundResource *name;
	float                startTime;
} SoundInstance;

typedef struct PresentationSlide {
	const char           *title;
	float                 duration;
	Rotation              rotation;
	const BackgroundStyle *background;
	const char           *talk;
	ftdArrayHandle        blocks; // PresentationBlock[]
	ftdArrayHandle        sounds; // SoundInstance[]
} PresentationSlide;

typedef struct PresentationSlideRef {
	const PresentationSlide *slide;
} PresentationSlideRef;

typedef struct FontDef {
	const FontResource *name;
	float               size;
	float               lineScale;
	const TextStyle    *style;
} FontDef;

typedef struct PresentationHeader {
	FontDef     font;
	float       height;
	const char *leftText;
	const char *centerText;
	const char *rightText;
	Vec2f       padding;
} PresentationHeader;

typedef struct PresentationFooter {
	FontDef     font;
	float       height;
	const char *leftText;
	const char *centerText;
	const char *rightText;
	Vec2f       padding;
} PresentationFooter;

typedef struct PresentationRoot {
	Vec2f                     slideSize;
	float                     padding;
	ftdArrayHandle            slides; // PresentationSlideRef[]
	const PresentationHeader *header;
	const PresentationFooter *footer;
	FontDef                   titleFont;
	FontDef                   normalFont;
	FontDef                   consoleFont;
} PresentationRoot;

// =============================================================================
// Field tables and types
// =============================================================================
static const ftdField Vec2f_fields[] = {
	{ "x", offsetof(Vec2f, x), ftdFieldKind_F32, NULL, NULL, 0, 0 },
	{ "y", offsetof(Vec2f, y), ftdFieldKind_F32, NULL, NULL, 0, 0 },
};
static const ftdType Vec2f_type = {
	"Vec2f", sizeof(Vec2f), _Alignof(Vec2f), Vec2f_fields, 2, NULL, 0
};

static const ftdField Vec3f_fields[] = {
	{ "x", offsetof(Vec3f, x), ftdFieldKind_F32, NULL, NULL, 0, 0 },
	{ "y", offsetof(Vec3f, y), ftdFieldKind_F32, NULL, NULL, 0, 0 },
	{ "z", offsetof(Vec3f, z), ftdFieldKind_F32, NULL, NULL, 0, 0 },
};
static const ftdType Vec3f_type = {
	"Vec3f", sizeof(Vec3f), _Alignof(Vec3f), Vec3f_fields, 3, NULL, 0
};

static const ftdField Vec4f_fields[] = {
	{ "x", offsetof(Vec4f, x), ftdFieldKind_F32, NULL, NULL, 0, 0 },
	{ "y", offsetof(Vec4f, y), ftdFieldKind_F32, NULL, NULL, 0, 0 },
	{ "z", offsetof(Vec4f, z), ftdFieldKind_F32, NULL, NULL, 0, 0 },
	{ "w", offsetof(Vec4f, w), ftdFieldKind_F32, NULL, NULL, 0, 0 },
};
static const ftdType Vec4f_type = {
	"Vec4f", sizeof(Vec4f), _Alignof(Vec4f), Vec4f_fields, 4, NULL, 0
};

static const ftdEnumValue HorizontalAlignment_values[] = {
	{ "Left", 0 }, { "Center", 1 }, { "Right", 2 },
};
static const ftdType HorizontalAlignment_type = {
	"HorizontalAlignment", sizeof(int32_t), 4, NULL, 0,
	HorizontalAlignment_values, 3
};

static const ftdEnumValue VerticalAlignment_values[] = {
	{ "Top", 0 }, { "Middle", 1 }, { "Bottom", 2 },
};
static const ftdType VerticalAlignment_type = {
	"VerticalAlignment", sizeof(int32_t), 4, NULL, 0,
	VerticalAlignment_values, 3
};

static const ftdEnumValue TextAlign_values[] = {
	{ "Left", 0 }, { "Center", 1 }, { "Right", 2 },
};
static const ftdType TextAlign_type = {
	"TextAlign", sizeof(int32_t), 4, NULL, 0, TextAlign_values, 3
};

static const ftdEnumValue BackgroundKind_values[] = {
	{ "Solid", 0 }, { "HalfGradientHorizontal", 1 }, { "HalfGradientVertical", 2 },
};
static const ftdType BackgroundKind_type = {
	"BackgroundKind", sizeof(int32_t), 4, NULL, 0, BackgroundKind_values, 3
};

static const ftdEnumValue PresentationBlockType_values[] = {
	{ "Text", 0 }, { "Image", 1 },
};
static const ftdType PresentationBlockType_type = {
	"BlockType", sizeof(int32_t), 4, NULL, 0, PresentationBlockType_values, 2
};

// Resource structs (only `name` is exposed to FTD; host pre-registers globals)
static const ftdField FontResource_fields[] = {
	{ "name", offsetof(FontResource, name), ftdFieldKind_String, NULL, NULL, 0, 0 },
};
static const ftdType FontResource_type = {
	"FontResource", sizeof(FontResource), _Alignof(FontResource),
	FontResource_fields, 1, NULL, 0
};

static const ftdField ImageResource_fields[] = {
	{ "name", offsetof(ImageResource, name), ftdFieldKind_String, NULL, NULL, 0, 0 },
};
static const ftdType ImageResource_type = {
	"ImageResource", sizeof(ImageResource), _Alignof(ImageResource),
	ImageResource_fields, 1, NULL, 0
};

static const ftdField SoundResource_fields[] = {
	{ "name", offsetof(SoundResource, name), ftdFieldKind_String, NULL, NULL, 0, 0 },
};
static const ftdType SoundResource_type = {
	"SoundResource", sizeof(SoundResource), _Alignof(SoundResource),
	SoundResource_fields, 1, NULL, 0
};

static const ftdField Rotation_fields[] = {
	{ "angleDegrees", offsetof(Rotation, angleDegrees), ftdFieldKind_F32,    NULL,        NULL, 0, 0 },
	{ "axis",         offsetof(Rotation, axis),         ftdFieldKind_Struct, &Vec3f_type, NULL, 0, 0 },
};
static const ftdType Rotation_type = {
	"Rotation", sizeof(Rotation), _Alignof(Rotation),
	Rotation_fields, 2, NULL, 0
};

static const ftdField ContentAlignment_fields[] = {
	{ "h", offsetof(ContentAlignment, h), ftdFieldKind_Enum, &HorizontalAlignment_type, NULL, 0, 0 },
	{ "v", offsetof(ContentAlignment, v), ftdFieldKind_Enum, &VerticalAlignment_type,   NULL, 0, 0 },
};
static const ftdType ContentAlignment_type = {
	"ContentAlignment", sizeof(ContentAlignment), _Alignof(ContentAlignment),
	ContentAlignment_fields, 2, NULL, 0
};

static const ftdField BackgroundStyle_fields[] = {
	{ "kind",           offsetof(BackgroundStyle, kind),           ftdFieldKind_Enum,   &BackgroundKind_type, NULL, 0, 0 },
	{ "primaryColor",   offsetof(BackgroundStyle, primaryColor),   ftdFieldKind_Struct, &Vec4f_type,          NULL, 0, 0 },
	{ "secondaryColor", offsetof(BackgroundStyle, secondaryColor), ftdFieldKind_Struct, &Vec4f_type,          NULL, 0, 0 },
};
static const ftdType BackgroundStyle_type = {
	"BackgroundStyle", sizeof(BackgroundStyle), _Alignof(BackgroundStyle),
	BackgroundStyle_fields, 3, NULL, 0
};

static const ftdField TextStyle_fields[] = {
	{ "foregroundColor", offsetof(TextStyle, foregroundColor), ftdFieldKind_Struct, &Vec4f_type, NULL, 0, 0 },
	{ "shadowColor",     offsetof(TextStyle, shadowColor),     ftdFieldKind_Struct, &Vec4f_type, NULL, 0, 0 },
	{ "shadowOffset",    offsetof(TextStyle, shadowOffset),    ftdFieldKind_Struct, &Vec2f_type, NULL, 0, 0 },
	{ "drawShadow",      offsetof(TextStyle, drawShadow),      ftdFieldKind_Bool,   NULL,        NULL, 0, 0 },
};
static const ftdType TextStyle_type = {
	"TextStyle", sizeof(TextStyle), _Alignof(TextStyle),
	TextStyle_fields, 4, NULL, 0
};

static const ftdField PresentationTextBlock_fields[] = {
	{ "text",      offsetof(PresentationTextBlock, text),      ftdFieldKind_String, NULL,            NULL, 0, 0 },
	{ "textAlign", offsetof(PresentationTextBlock, textAlign), ftdFieldKind_Enum,   &TextAlign_type, NULL, 0, 0 },
	{ "fontSize",  offsetof(PresentationTextBlock, fontSize),  ftdFieldKind_F32,    NULL,            NULL, 0, 0 },
	{ "color",     offsetof(PresentationTextBlock, color),     ftdFieldKind_Struct, &Vec4f_type,     NULL, 0, 0 },
};
static const ftdType PresentationTextBlock_type = {
	"TextBlock", sizeof(PresentationTextBlock), _Alignof(PresentationTextBlock),
	PresentationTextBlock_fields, 4, NULL, 0
};

static const ftdField PresentationImageBlock_fields[] = {
	{ "imageResource", offsetof(PresentationImageBlock, imageResource), ftdFieldKind_Ref,    &ImageResource_type, NULL, 0, 0 },
	{ "size",          offsetof(PresentationImageBlock, size),          ftdFieldKind_Struct, &Vec2f_type,         NULL, 0, 0 },
	{ "keepAspect",    offsetof(PresentationImageBlock, keepAspect),    ftdFieldKind_Bool,   NULL,                NULL, 0, 0 },
	{ "tintColor",     offsetof(PresentationImageBlock, tintColor),     ftdFieldKind_Struct, &Vec4f_type,         NULL, 0, 0 },
};
static const ftdType PresentationImageBlock_type = {
	"ImageBlock", sizeof(PresentationImageBlock), _Alignof(PresentationImageBlock),
	PresentationImageBlock_fields, 4, NULL, 0
};

static const ftdField PresentationBlock_fields[] = {
	{ "type",             offsetof(PresentationBlock, type),             ftdFieldKind_Enum,   &PresentationBlockType_type,  NULL,   0,                              0 },
	{ "pos",              offsetof(PresentationBlock, pos),              ftdFieldKind_Struct, &Vec2f_type,                  NULL,   0,                              0 },
	{ "size",             offsetof(PresentationBlock, size),             ftdFieldKind_Struct, &Vec2f_type,                  NULL,   0,                              0 },
	{ "contentAlignment", offsetof(PresentationBlock, contentAlignment), ftdFieldKind_Struct, &ContentAlignment_type,       NULL,   0,                              0 },
	{ "text",             offsetof(PresentationBlock, text),             ftdFieldKind_Union,  &PresentationTextBlock_type,  "type", PresentationBlockType_Text,     0 },
	{ "image",            offsetof(PresentationBlock, image),            ftdFieldKind_Union,  &PresentationImageBlock_type, "type", PresentationBlockType_Image,    0 },
};
static const ftdType PresentationBlock_type = {
	"Block", sizeof(PresentationBlock), _Alignof(PresentationBlock),
	PresentationBlock_fields, 6, NULL, 0
};

static const ftdField SoundInstance_fields[] = {
	{ "name",      offsetof(SoundInstance, name),      ftdFieldKind_Ref, &SoundResource_type, NULL, 0, 0 },
	{ "startTime", offsetof(SoundInstance, startTime), ftdFieldKind_F32, NULL,                NULL, 0, 0 },
};
static const ftdType SoundInstance_type = {
	"SoundInstance", sizeof(SoundInstance), _Alignof(SoundInstance),
	SoundInstance_fields, 2, NULL, 0
};

static const ftdField PresentationSlide_fields[] = {
	{ "title",      offsetof(PresentationSlide, title),      ftdFieldKind_String, NULL,                       NULL, 0, 0 },
	{ "duration",   offsetof(PresentationSlide, duration),   ftdFieldKind_F32,    NULL,                       NULL, 0, 0 },
	{ "rotation",   offsetof(PresentationSlide, rotation),   ftdFieldKind_Struct, &Rotation_type,             NULL, 0, 0 },
	{ "background", offsetof(PresentationSlide, background), ftdFieldKind_Ref,    &BackgroundStyle_type,      NULL, 0, 0 },
	{ "talk",       offsetof(PresentationSlide, talk),       ftdFieldKind_String, NULL,                       NULL, 0, 0 },
	{ "blocks",     offsetof(PresentationSlide, blocks),     ftdFieldKind_Array,  &PresentationBlock_type,    NULL, 0, 0 },
	{ "sounds",     offsetof(PresentationSlide, sounds),     ftdFieldKind_Array,  &SoundInstance_type,        NULL, 0, 0 },
};
static const ftdType PresentationSlide_type = {
	"Slide", sizeof(PresentationSlide), _Alignof(PresentationSlide),
	PresentationSlide_fields, 7, NULL, 0
};

// Empty-field 8-byte type used as the element type of `Presentation.slides`,
// so the array stride matches a pointer.
static const ftdType PresentationSlideRef_type = {
	"SlideRef", sizeof(PresentationSlideRef), _Alignof(PresentationSlideRef),
	NULL, 0, NULL, 0
};

static const ftdField FontDef_fields[] = {
	{ "name",      offsetof(FontDef, name),      ftdFieldKind_Ref, &FontResource_type, NULL, 0, 0 },
	{ "size",      offsetof(FontDef, size),      ftdFieldKind_F32, NULL,               NULL, 0, 0 },
	{ "lineScale", offsetof(FontDef, lineScale), ftdFieldKind_F32, NULL,               NULL, 0, 0 },
	{ "style",     offsetof(FontDef, style),     ftdFieldKind_Ref, &TextStyle_type,    NULL, 0, 0 },
};
static const ftdType FontDef_type = {
	"FontDef", sizeof(FontDef), _Alignof(FontDef),
	FontDef_fields, 4, NULL, 0
};

static const ftdField PresentationHeader_fields[] = {
	{ "font",       offsetof(PresentationHeader, font),       ftdFieldKind_Struct, &FontDef_type, NULL, 0, 0 },
	{ "height",     offsetof(PresentationHeader, height),     ftdFieldKind_F32,    NULL,          NULL, 0, 0 },
	{ "leftText",   offsetof(PresentationHeader, leftText),   ftdFieldKind_String, NULL,          NULL, 0, 0 },
	{ "centerText", offsetof(PresentationHeader, centerText), ftdFieldKind_String, NULL,          NULL, 0, 0 },
	{ "rightText",  offsetof(PresentationHeader, rightText),  ftdFieldKind_String, NULL,          NULL, 0, 0 },
	{ "padding",    offsetof(PresentationHeader, padding),    ftdFieldKind_Struct, &Vec2f_type,   NULL, 0, 0 },
};
static const ftdType PresentationHeader_type = {
	"HeaderDefinition", sizeof(PresentationHeader), _Alignof(PresentationHeader),
	PresentationHeader_fields, 6, NULL, 0
};

static const ftdField PresentationFooter_fields[] = {
	{ "font",       offsetof(PresentationFooter, font),       ftdFieldKind_Struct, &FontDef_type, NULL, 0, 0 },
	{ "height",     offsetof(PresentationFooter, height),     ftdFieldKind_F32,    NULL,          NULL, 0, 0 },
	{ "leftText",   offsetof(PresentationFooter, leftText),   ftdFieldKind_String, NULL,          NULL, 0, 0 },
	{ "centerText", offsetof(PresentationFooter, centerText), ftdFieldKind_String, NULL,          NULL, 0, 0 },
	{ "rightText",  offsetof(PresentationFooter, rightText),  ftdFieldKind_String, NULL,          NULL, 0, 0 },
	{ "padding",    offsetof(PresentationFooter, padding),    ftdFieldKind_Struct, &Vec2f_type,   NULL, 0, 0 },
};
static const ftdType PresentationFooter_type = {
	"FooterDefinition", sizeof(PresentationFooter), _Alignof(PresentationFooter),
	PresentationFooter_fields, 6, NULL, 0
};

static const ftdField PresentationRoot_fields[] = {
	{ "slideSize",   offsetof(PresentationRoot, slideSize),   ftdFieldKind_Struct, &Vec2f_type,                NULL, 0, 0 },
	{ "padding",     offsetof(PresentationRoot, padding),     ftdFieldKind_F32,    NULL,                       NULL, 0, 0 },
	{ "slides",      offsetof(PresentationRoot, slides),      ftdFieldKind_Array,  &PresentationSlideRef_type, NULL, 0, 0 },
	{ "header",      offsetof(PresentationRoot, header),      ftdFieldKind_Ref,    &PresentationHeader_type,   NULL, 0, 0 },
	{ "footer",      offsetof(PresentationRoot, footer),      ftdFieldKind_Ref,    &PresentationFooter_type,   NULL, 0, 0 },
	{ "titleFont",   offsetof(PresentationRoot, titleFont),   ftdFieldKind_Struct, &FontDef_type,              NULL, 0, 0 },
	{ "normalFont",  offsetof(PresentationRoot, normalFont),  ftdFieldKind_Struct, &FontDef_type,              NULL, 0, 0 },
	{ "consoleFont", offsetof(PresentationRoot, consoleFont), ftdFieldKind_Struct, &FontDef_type,              NULL, 0, 0 },
};
static const ftdType PresentationRoot_type = {
	"Presentation", sizeof(PresentationRoot), _Alignof(PresentationRoot),
	PresentationRoot_fields, 8, NULL, 0
};

// =============================================================================
// Helpers: RGBA(r,g,b,a) and RGBA24(hex, alpha) -> Vec4f (linearized 0..1)
// =============================================================================
static uint64_t FplPresentation__ValueToUInt(const ftdValue *v) {
	switch (v->kind) {
		case ftdValueKind_UInt:  return v->as.u;
		case ftdValueKind_Int:   return (uint64_t)v->as.i;
		case ftdValueKind_Float: return (uint64_t)v->as.f;
		default:                 return 0;
	}
}

static bool FplPresentation_RGBA24(ftdContext *ctx, const ftdValue *args, uint32_t argCount, void *outValue, void *userData) {
	(void)ctx; (void)userData;
	Vec4f *o = (Vec4f *)outValue;
	uint64_t hex = (argCount >= 1) ? FplPresentation__ValueToUInt(&args[0]) : 0;
	uint64_t a8  = (argCount >= 2) ? FplPresentation__ValueToUInt(&args[1]) : 255;
	o->x = (float)((hex >> 16) & 0xFF) / 255.0f;
	o->y = (float)((hex >>  8) & 0xFF) / 255.0f;
	o->z = (float)((hex      ) & 0xFF) / 255.0f;
	o->w = (float)(a8 & 0xFF) / 255.0f;
	return true;
}

static bool FplPresentation_RGBA(ftdContext *ctx, const ftdValue *args, uint32_t argCount, void *outValue, void *userData) {
	(void)ctx; (void)userData;
	Vec4f *o = (Vec4f *)outValue;
	uint64_t r = (argCount >= 1) ? FplPresentation__ValueToUInt(&args[0]) : 0;
	uint64_t g = (argCount >= 2) ? FplPresentation__ValueToUInt(&args[1]) : 0;
	uint64_t b = (argCount >= 3) ? FplPresentation__ValueToUInt(&args[2]) : 0;
	uint64_t a = (argCount >= 4) ? FplPresentation__ValueToUInt(&args[3]) : 255;
	o->x = (float)(r & 0xFF) / 255.0f;
	o->y = (float)(g & 0xFF) / 255.0f;
	o->z = (float)(b & 0xFF) / 255.0f;
	o->w = (float)(a & 0xFF) / 255.0f;
	return true;
}

// =============================================================================
// Resource tables (referenced by the .ftd via FontResources.X, etc.)
// =============================================================================
typedef struct FplPresentation_NamedFont {
	const char         *dottedName; // e.g., "FontResources.Arimo"
	const FontResource *resource;
} FplPresentation_NamedFont;

typedef struct FplPresentation_NamedImage {
	const char          *dottedName;
	const ImageResource *resource;
} FplPresentation_NamedImage;

typedef struct FplPresentation_NamedSound {
	const char          *dottedName;
	const SoundResource *resource;
} FplPresentation_NamedSound;

#define FPL_PRES_FONT(LocalName)  static const FontResource  FontRes_##LocalName  = { #LocalName }
#define FPL_PRES_IMAGE(LocalName) static const ImageResource ImageRes_##LocalName = { #LocalName }
#define FPL_PRES_SOUND(LocalName) static const SoundResource SoundRes_##LocalName = { #LocalName }

FPL_PRES_FONT(Arimo);
FPL_PRES_FONT(BitStreamVerySans);

FPL_PRES_IMAGE(Card_CPU);
FPL_PRES_IMAGE(Card_Memory);
FPL_PRES_IMAGE(Card_C_Language);
FPL_PRES_IMAGE(Card_Audio);
FPL_PRES_IMAGE(Card_Video);
FPL_PRES_IMAGE(Card_WindowManagement);
FPL_PRES_IMAGE(Card_Performance);
FPL_PRES_IMAGE(Card_NoDependencies);
FPL_PRES_IMAGE(Card_KeyboardMouse);
FPL_PRES_IMAGE(Card_Gamepad);
FPL_PRES_IMAGE(MagicHat);
FPL_PRES_IMAGE(Arigatou);
FPL_PRES_IMAGE(MinimumSource);
FPL_PRES_IMAGE(DataVisualization);
FPL_PRES_IMAGE(GameDev);
FPL_PRES_IMAGE(MultimediaDev);
FPL_PRES_IMAGE(SimulationDev);
FPL_PRES_IMAGE(Code);
FPL_PRES_IMAGE(Vendor_FreeBSD);
FPL_PRES_IMAGE(Vendor_Linux);
FPL_PRES_IMAGE(Vendor_Windows);
FPL_PRES_IMAGE(Vendor_Raspberry);
FPL_PRES_IMAGE(Vendor_OpenGL);
FPL_PRES_IMAGE(Vendor_Vulkan);
FPL_PRES_IMAGE(Vendor_DirectX);
FPL_PRES_IMAGE(Vendor_Alsa);
FPL_PRES_IMAGE(Vendor_XLib);
FPL_PRES_IMAGE(Demo_ImGUI);
FPL_PRES_IMAGE(Demo_FFMPEG);
FPL_PRES_IMAGE(Demo_NBodySimulation);
FPL_PRES_IMAGE(Demo_Audio);
FPL_PRES_IMAGE(Demo_Input);
FPL_PRES_IMAGE(Demo_Crackout);
FPL_PRES_IMAGE(Demo_OpenGL);
FPL_PRES_IMAGE(Demo_Raytracer);
FPL_PRES_IMAGE(Feature_SingleHeaderFile);
FPL_PRES_IMAGE(Feature_RuntimeLinking);
FPL_PRES_IMAGE(Feature_PreProcessor);
FPL_PRES_IMAGE(Feature_NoCodeDuplication);

FPL_PRES_SOUND(Intro1);
FPL_PRES_SOUND(Intro2);
FPL_PRES_SOUND(WhoAmi1);
FPL_PRES_SOUND(What1);
FPL_PRES_SOUND(What2);
FPL_PRES_SOUND(Motivation);

// =============================================================================
// Registration entry point
// =============================================================================
static void FplPresentationTypes_Register(ftdContext *ctx) {
	// Structs
	ftdRegisterStruct(ctx, &Vec2f_type);
	ftdRegisterStruct(ctx, &Vec3f_type);
	ftdRegisterStruct(ctx, &Vec4f_type);
	ftdRegisterStruct(ctx, &FontResource_type);
	ftdRegisterStruct(ctx, &ImageResource_type);
	ftdRegisterStruct(ctx, &SoundResource_type);
	ftdRegisterStruct(ctx, &Rotation_type);
	ftdRegisterStruct(ctx, &ContentAlignment_type);
	ftdRegisterStruct(ctx, &BackgroundStyle_type);
	ftdRegisterStruct(ctx, &TextStyle_type);
	ftdRegisterStruct(ctx, &PresentationTextBlock_type);
	ftdRegisterStruct(ctx, &PresentationImageBlock_type);
	ftdRegisterStruct(ctx, &PresentationBlock_type);
	ftdRegisterStruct(ctx, &SoundInstance_type);
	ftdRegisterStruct(ctx, &PresentationSlide_type);
	ftdRegisterStruct(ctx, &PresentationSlideRef_type);
	ftdRegisterStruct(ctx, &FontDef_type);
	ftdRegisterStruct(ctx, &PresentationHeader_type);
	ftdRegisterStruct(ctx, &PresentationFooter_type);
	ftdRegisterStruct(ctx, &PresentationRoot_type);

	// Enums
	ftdRegisterEnum(ctx, &HorizontalAlignment_type);
	ftdRegisterEnum(ctx, &VerticalAlignment_type);
	ftdRegisterEnum(ctx, &TextAlign_type);
	ftdRegisterEnum(ctx, &BackgroundKind_type);
	ftdRegisterEnum(ctx, &PresentationBlockType_type);

	// Helpers
	ftdRegisterHelper(ctx, "RGBA24", &Vec4f_type, FplPresentation_RGBA24, NULL);
	ftdRegisterHelper(ctx, "RGBA",   &Vec4f_type, FplPresentation_RGBA,   NULL);

	// Resource globals (referenced as FontResources.X, ImageResources.X, SoundResources.X)
	#define REG_FONT(LocalName)  ftdRegisterGlobal(ctx, "FontResources."  #LocalName, &FontResource_type,  &FontRes_##LocalName)
	#define REG_IMAGE(LocalName) ftdRegisterGlobal(ctx, "ImageResources." #LocalName, &ImageResource_type, &ImageRes_##LocalName)
	#define REG_SOUND(LocalName) ftdRegisterGlobal(ctx, "SoundResources." #LocalName, &SoundResource_type, &SoundRes_##LocalName)

	REG_FONT(Arimo);
	REG_FONT(BitStreamVerySans);

	REG_IMAGE(Card_CPU);
	REG_IMAGE(Card_Memory);
	REG_IMAGE(Card_C_Language);
	REG_IMAGE(Card_Audio);
	REG_IMAGE(Card_Video);
	REG_IMAGE(Card_WindowManagement);
	REG_IMAGE(Card_Performance);
	REG_IMAGE(Card_NoDependencies);
	REG_IMAGE(Card_KeyboardMouse);
	REG_IMAGE(Card_Gamepad);
	REG_IMAGE(MagicHat);
	REG_IMAGE(Arigatou);
	REG_IMAGE(MinimumSource);
	REG_IMAGE(DataVisualization);
	REG_IMAGE(GameDev);
	REG_IMAGE(MultimediaDev);
	REG_IMAGE(SimulationDev);
	REG_IMAGE(Code);
	REG_IMAGE(Vendor_FreeBSD);
	REG_IMAGE(Vendor_Linux);
	REG_IMAGE(Vendor_Windows);
	REG_IMAGE(Vendor_Raspberry);
	REG_IMAGE(Vendor_OpenGL);
	REG_IMAGE(Vendor_Vulkan);
	REG_IMAGE(Vendor_DirectX);
	REG_IMAGE(Vendor_Alsa);
	REG_IMAGE(Vendor_XLib);
	REG_IMAGE(Demo_ImGUI);
	REG_IMAGE(Demo_FFMPEG);
	REG_IMAGE(Demo_NBodySimulation);
	REG_IMAGE(Demo_Audio);
	REG_IMAGE(Demo_Input);
	REG_IMAGE(Demo_Crackout);
	REG_IMAGE(Demo_OpenGL);
	REG_IMAGE(Demo_Raytracer);
	REG_IMAGE(Feature_SingleHeaderFile);
	REG_IMAGE(Feature_RuntimeLinking);
	REG_IMAGE(Feature_PreProcessor);
	REG_IMAGE(Feature_NoCodeDuplication);

	REG_SOUND(Intro1);
	REG_SOUND(Intro2);
	REG_SOUND(WhoAmi1);
	REG_SOUND(What1);
	REG_SOUND(What2);
	REG_SOUND(Motivation);

	#undef REG_FONT
	#undef REG_IMAGE
	#undef REG_SOUND
}

#endif // FPL_PRESENTATION_TYPES_H
