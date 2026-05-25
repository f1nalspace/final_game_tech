#pragma once

#include <final_ftd.h>

#include "types.h"
#include "presentation.h"
#include "ftd_helper.h"
#include "builtin.h"

namespace Vec2fStructDef {
    static constexpr ftdField Fields[] = {
        FTD_FIELD(Vec2f, x),
        FTD_FIELD(Vec2f, y),
    };
    static constexpr ftdType Type = FTD_TYPE_STRUCT(Vec2f, Fields);
}
FTD_BIND_TYPE(Vec2f, Vec2fStructDef::Type)

namespace Vec3fStructDef {
    static constexpr ftdField Fields[] = {
        FTD_FIELD(Vec3f, x),
        FTD_FIELD(Vec3f, y),
        FTD_FIELD(Vec3f, z),
    };
    static constexpr ftdType Type = FTD_TYPE_STRUCT(Vec3f, Fields);
}
FTD_BIND_TYPE(Vec3f, Vec3fStructDef::Type)

namespace Vec4fStructDef {
    static constexpr ftdField Fields[] = {
        FTD_FIELD(Vec4f, x),
        FTD_FIELD(Vec4f, y),
        FTD_FIELD(Vec4f, z),
        FTD_FIELD(Vec4f, w),
    };
    static constexpr ftdType Type = FTD_TYPE_STRUCT(Vec4f, Fields);
}
FTD_BIND_TYPE(Vec4f, Vec4fStructDef::Type)

namespace QuaternionStructDef {
    static constexpr ftdField Fields[] = {
        FTD_FIELD(Quaternion, x),
        FTD_FIELD(Quaternion, y),
        FTD_FIELD(Quaternion, z),
        FTD_FIELD(Quaternion, w),
    };
    static constexpr ftdType Type = FTD_TYPE_STRUCT(Quaternion, Fields);
}
FTD_BIND_TYPE(Quaternion, QuaternionStructDef::Type)

namespace ResourceTypeEnumDef {
    static constexpr ftdEnumValue Values[] = {
        FTD_ENUM_VALUE(ResourceType, None),
        FTD_ENUM_VALUE(ResourceType, File),
        FTD_ENUM_VALUE(ResourceType, Memory),
    };
    static constexpr ftdType Type = FTD_TYPE_ENUM(ResourceType, Values);
}

namespace FileResourceStructDef {
    static constexpr ftdField Fields[] = {
        FTD_FIELD(FileResource, filePath),
    };
    static constexpr ftdType Type = FTD_TYPE_STRUCT(FileResource, Fields);
}
FTD_BIND_TYPE(FileResource, FileResourceStructDef::Type)

namespace MemoryResourceStructDef {
    static constexpr ftdField Fields[] = {
        FTD_FIELD_KIND(MemoryResource, data, ftdFieldKind_MemoryData8),
        FTD_FIELD_KIND(MemoryResource, length, ftdFieldKind_MemorySize),
    };
    static constexpr ftdType Type = FTD_TYPE_STRUCT(MemoryResource, Fields);
}
FTD_BIND_TYPE(MemoryResource, MemoryResourceStructDef::Type)

namespace ResourceStructDef {
    static constexpr ftdField Fields[] = {
        FTD_FIELD(Resource, name),
        FTD_FIELD_ENUM(Resource, type, &ResourceTypeEnumDef::Type),
        FTD_FIELD_UNION(Resource, file, &FileResourceStructDef::Type, "type", ResourceType::File),
        FTD_FIELD_UNION(Resource, memory, &MemoryResourceStructDef::Type, "type", ResourceType::Memory),
    };
    static constexpr ftdType Type = FTD_TYPE_STRUCT(Resource, Fields);
}
FTD_BIND_TYPE(Resource, ResourceStructDef::Type)

namespace SoundDefinitionStructDef {
    static constexpr ftdField Fields[] = {
        FTD_FIELD(SoundDefinition, name),
        FTD_FIELD(SoundDefinition, startTime),
        FTD_FIELD(SoundDefinition, targetDuration),
    };
    static constexpr ftdType Type = FTD_TYPE_STRUCT(SoundDefinition, Fields);
}
FTD_BIND_TYPE(SoundDefinition, SoundDefinitionStructDef::Type)

namespace BackgroundKindEnumDef {
    static constexpr ftdEnumValue Values[] = {
        FTD_ENUM_VALUE(BackgroundKind, None),
        FTD_ENUM_VALUE(BackgroundKind, Solid),
        FTD_ENUM_VALUE(BackgroundKind, GradientHorizontal),
        FTD_ENUM_VALUE(BackgroundKind, GradientVertical),
        FTD_ENUM_VALUE(BackgroundKind, HalfGradientHorizontal),
        FTD_ENUM_VALUE(BackgroundKind, HalfGradientVertical),
    };
    static constexpr ftdType Type = FTD_TYPE_ENUM(BackgroundKind, Values);
}

namespace BackgroundStyleStructDef {
    static constexpr ftdField Fields[] = {
        FTD_FIELD_ENUM(BackgroundStyle, kind, &BackgroundKindEnumDef::Type),
        FTD_FIELD_STRUCT(BackgroundStyle, primaryColor),
        FTD_FIELD_STRUCT(BackgroundStyle, secondaryColor),
    };
    static constexpr ftdType Type = FTD_TYPE_STRUCT(BackgroundStyle, Fields);
}
FTD_BIND_TYPE(BackgroundStyle, BackgroundStyleStructDef::Type);

namespace TextStyleStructDef {
    static constexpr ftdField Fields[] = {
        FTD_FIELD_STRUCT(TextStyle, background),
        FTD_FIELD_STRUCT(TextStyle, foregroundColor),
        FTD_FIELD_STRUCT(TextStyle, shadowColor),
        FTD_FIELD_STRUCT(TextStyle, shadowOffset),
        FTD_FIELD(TextStyle, drawShadow),
    };
    static constexpr ftdType Type = FTD_TYPE_STRUCT(TextStyle, Fields);
}
FTD_BIND_TYPE(TextStyle, TextStyleStructDef::Type);

namespace FontDefinitionStructDef {
    static constexpr ftdField Fields[] = {
        FTD_FIELD(FontDefinition, name),
        FTD_FIELD(FontDefinition, size),
        FTD_FIELD(FontDefinition, lineScale),
        FTD_FIELD_STRUCT(FontDefinition, style),
    };
    static constexpr ftdType Type = FTD_TYPE_STRUCT(FontDefinition, Fields);
}
FTD_BIND_TYPE(FontDefinition, FontDefinitionStructDef::Type);

namespace HeaderDefinitionStructDef {
    static constexpr ftdField Fields[] = {
        FTD_FIELD_STRUCT(HeaderDefinition, font),
        FTD_FIELD(HeaderDefinition, height),
        FTD_FIELD(HeaderDefinition, leftText),
        FTD_FIELD(HeaderDefinition, centerText),
        FTD_FIELD(HeaderDefinition, rightText),
        FTD_FIELD_STRUCT(HeaderDefinition, padding),
    };
    static constexpr ftdType Type = FTD_TYPE_STRUCT(HeaderDefinition, Fields);
}
FTD_BIND_TYPE(HeaderDefinition, HeaderDefinitionStructDef::Type);

namespace FooterDefinitionStructDef {
    static constexpr ftdField Fields[] = {
        FTD_FIELD_STRUCT(FooterDefinition, font),
        FTD_FIELD(FooterDefinition, height),
        FTD_FIELD(FooterDefinition, leftText),
        FTD_FIELD(FooterDefinition, centerText),
        FTD_FIELD(FooterDefinition, rightText),
        FTD_FIELD_STRUCT(FooterDefinition, padding),
    };
    static constexpr ftdType Type = FTD_TYPE_STRUCT(FooterDefinition, Fields);
}
FTD_BIND_TYPE(FooterDefinition, FooterDefinitionStructDef::Type);

namespace BlockTypeEnumDef {
    static constexpr ftdEnumValue Values[] = {
        FTD_ENUM_VALUE(BlockType, None),
        FTD_ENUM_VALUE(BlockType, Text),
        FTD_ENUM_VALUE(BlockType, Image),
    };
    static constexpr ftdType Type = FTD_TYPE_ENUM(BlockType, Values);
}
FTD_BIND_TYPE(BlockType, BlockTypeEnumDef::Type);

namespace HorizontalAlignmentEnumDef {
    static constexpr ftdEnumValue Values[] = {
        FTD_ENUM_VALUE(HorizontalAlignment, Left),
        FTD_ENUM_VALUE(HorizontalAlignment, Center),
        FTD_ENUM_VALUE(HorizontalAlignment, Right),
    };
    static constexpr ftdType Type = FTD_TYPE_ENUM(HorizontalAlignment, Values);
}
FTD_BIND_TYPE(HorizontalAlignment, HorizontalAlignmentEnumDef::Type);

namespace VerticalAlignmentEnumDef {
    static constexpr ftdEnumValue Values[] = {
        FTD_ENUM_VALUE(VerticalAlignment, Top),
        FTD_ENUM_VALUE(VerticalAlignment, Middle),
        FTD_ENUM_VALUE(VerticalAlignment, Bottom),
    };
    static constexpr ftdType Type = FTD_TYPE_ENUM(VerticalAlignment, Values);
}
FTD_BIND_TYPE(VerticalAlignment, VerticalAlignmentEnumDef::Type);

namespace BlockAlignmentStructDef {
    static constexpr ftdField Fields[] = {
        FTD_FIELD_ENUM(BlockAlignment, h, &HorizontalAlignmentEnumDef::Type),
        FTD_FIELD_ENUM(BlockAlignment, v, &VerticalAlignmentEnumDef::Type),
    };
    static constexpr ftdType Type = FTD_TYPE_STRUCT(BlockAlignment, Fields);
}
FTD_BIND_TYPE(BlockAlignment, BlockAlignmentStructDef::Type);

namespace TextBlockDefinitionStructDef {
    static constexpr ftdField Fields[] = {
        FTD_FIELD_STRUCT(TextBlockDefinition, color),
        FTD_FIELD(TextBlockDefinition, text),
        FTD_FIELD(TextBlockDefinition, fontSize),
        FTD_FIELD_ENUM(TextBlockDefinition, textAlign, &HorizontalAlignmentEnumDef::Type),
    };
    static constexpr ftdType Type = FTD_TYPE_STRUCT(TextBlockDefinition, Fields);
}
FTD_BIND_TYPE(TextBlockDefinition, TextBlockDefinitionStructDef::Type);

namespace ImageBlockDefinitionStructDef {
    static constexpr ftdField Fields[] = {
        FTD_FIELD_STRUCT(ImageBlockDefinition, tintColor),
        FTD_FIELD_STRUCT(ImageBlockDefinition, size),
        FTD_FIELD_REF(ImageBlockDefinition, imageResource, &ResourceStructDef::Type),
        FTD_FIELD(ImageBlockDefinition, keepAspect),
    };
    static constexpr ftdType Type = FTD_TYPE_STRUCT(ImageBlockDefinition, Fields);
}
FTD_BIND_TYPE(ImageBlockDefinition, ImageBlockDefinitionStructDef::Type);

namespace BlockDefinitionStructDef {
    static constexpr ftdField Fields[] = {
        FTD_FIELD_STRUCT(BlockDefinition, pos),
        FTD_FIELD_STRUCT(BlockDefinition, size),
        FTD_FIELD_ENUM(BlockDefinition, type, &BlockTypeEnumDef::Type),
        FTD_FIELD_STRUCT(BlockDefinition, contentAlignment),
        FTD_FIELD_UNION(BlockDefinition, text, &TextBlockDefinitionStructDef::Type, "type", BlockType::Text),
        FTD_FIELD_UNION(BlockDefinition, image, &ImageBlockDefinitionStructDef::Type, "type", BlockType::Image),
    };
    static constexpr ftdType Type = FTD_TYPE_STRUCT(BlockDefinition, Fields);
}
FTD_BIND_TYPE(BlockDefinition, BlockDefinitionStructDef::Type);

namespace SlideDefinitionStructDef {
    static constexpr ftdField Fields[] = {
        FTD_FIELD(SlideDefinition, name),
        FTD_FIELD(SlideDefinition, talk),
        FTD_FIELD_ARRAY_DATA(SlideDefinition, blocks, blocks),
        FTD_FIELD_ARRAY_DATA(SlideDefinition, sounds, sounds),
        FTD_FIELD_STRUCT(SlideDefinition, background),
        FTD_FIELD_STRUCT(SlideDefinition, rotation),
        FTD_FIELD_ARRAY_SIZE(SlideDefinition, blocks, blockCount),
        FTD_FIELD_ARRAY_SIZE(SlideDefinition, sounds, soundCount),
        FTD_FIELD(SlideDefinition, duration),
    };
    static constexpr ftdType Type = FTD_TYPE_STRUCT(SlideDefinition, Fields);
}
FTD_BIND_TYPE(SlideDefinition, SlideDefinitionStructDef::Type);

namespace PresentationDefinitionStructDef {
    static constexpr ftdField Fields[] = {
        FTD_FIELD_ARRAY_DATA(PresentationDefinition, slides, slides),
        FTD_FIELD_ARRAY_SIZE(PresentationDefinition, slides, slideCount),
        FTD_FIELD_STRUCT(PresentationDefinition, slideSize),
        FTD_FIELD_STRUCT(PresentationDefinition, header),
        FTD_FIELD_STRUCT(PresentationDefinition, footer),
        FTD_FIELD_STRUCT(PresentationDefinition, titleFont),
        FTD_FIELD_STRUCT(PresentationDefinition, normalFont),
        FTD_FIELD_STRUCT(PresentationDefinition, consoleFont),
        FTD_FIELD(PresentationDefinition, padding),
    };
    static constexpr ftdType Type = FTD_TYPE_STRUCT(PresentationDefinition, Fields);
}
FTD_BIND_TYPE(PresentationDefinition, PresentationDefinitionStructDef::Type);

namespace PresentationFileStructDef {
    static constexpr ftdField Fields[] = {
        FTD_FIELD_STRUCT(PresentationFile, definition),
        FTD_FIELD_ARRAY_DATA(PresentationFile, fonts, fonts),
        FTD_FIELD_ARRAY_DATA(PresentationFile, sounds, sounds),
        FTD_FIELD_ARRAY_DATA(PresentationFile, images, images),
        FTD_FIELD_ARRAY_SIZE(PresentationFile, fonts, fontCount),
        FTD_FIELD_ARRAY_SIZE(PresentationFile, sounds, soundCount),
        FTD_FIELD_ARRAY_SIZE(PresentationFile, images, imageCount),
    };
    static constexpr ftdType Type = FTD_TYPE_STRUCT(PresentationFile, Fields);
}
FTD_BIND_TYPE(PresentationFile, PresentationFileStructDef::Type);

namespace ArgumentHelpers {
    static uint32_t ArgToU32(const ftdValue *v) {
        switch (v->kind) {
            case ftdValueKind_UInt:  return (uint32_t)v->as.u;
            case ftdValueKind_Int:   return (uint32_t)v->as.i;
            case ftdValueKind_Float: return (uint32_t)v->as.f;
            default:                 return 0;
        }
    }

    static uint8_t ArgToU8(const ftdValue *v) {
        switch (v->kind) {
            case ftdValueKind_UInt:  return (uint8_t)v->as.u;
            case ftdValueKind_Int:   return (uint8_t)v->as.i;
            case ftdValueKind_Float: return (uint8_t)v->as.f;
            default:                 return 0;
        }
    }

    static float ArgToFloat(const ftdValue *v) {
        switch (v->kind) {
            case ftdValueKind_UInt:  return (float)v->as.u;
            case ftdValueKind_Int:   return (float)v->as.i;
            case ftdValueKind_Float: return (float)v->as.f;
            default:                 return 0.0f;
        }
    }

    static Vec3f ArgToVec3f(const ftdValue *v) {
        if (v->kind == ftdValueKind_Struct && v->as.ptr != nullptr && v->type == &Vec3fStructDef::Type) {
            return *(const Vec3f *)v->as.ptr;
        }
        return V3fZero();
    }
}

// Color helpers callable from .ftd source: RGBA24(hex, alpha) -> Vec4f.
namespace ColorHelpers {


    // RGBA24(0xRRGGBB, alpha) packs a 24-bit color literal plus an 8-bit alpha into a Vec4f with each channel normalized to 0..1.
    // Missing alpha defaults to 255 (fully opaque).
    static bool RGBA24(ftdContext *ctx, const ftdValue *args, uint32_t argCount, void *outValue, void *userData) {
        (void)ctx; (void)userData;
        Vec4f *o = (Vec4f *)outValue;
        const uint32_t hex = (argCount >= 1) ? ArgumentHelpers::ArgToU32(&args[0]) : 0;
        const uint8_t a  = (argCount >= 2) ? ArgumentHelpers::ArgToU8(&args[1]) : 255;
        *o = RGBAToLinearHex24(hex, a);
        return true;
    }

    // RGBA32(0xRRGGBBAA) packs a 32-bit color literal into a Vec4f with each channel normalized to 0..1.
    static bool RGBA32(ftdContext *ctx, const ftdValue *args, uint32_t argCount, void *outValue, void *userData) {
        (void)ctx; (void)userData;
        Vec4f *o = (Vec4f *)outValue;
        const uint32_t hex = (argCount == 1) ? ArgumentHelpers::ArgToU32(&args[0]) : 0;
        *o = BGRAUnpack4x8(hex);
        return true;
    }

    // RGBA(r, g, b, a) -> Vec4f
    // Each argument is interpreted as an 8-bit channel value (0..255) and normalized into 0..1. Extra/missing arguments default to 0 for RGB and 255 for the optional alpha.
    static bool RGBA(ftdContext *ctx, const ftdValue *args, uint32_t argCount, void *outValue, void *userData) {
        (void)ctx; (void)userData;
        Vec4f *o = (Vec4f *)outValue;
        const uint8_t r = (argCount >= 1) ? ArgumentHelpers::ArgToU8(&args[0]) : 0;
        const uint8_t g = (argCount >= 2) ? ArgumentHelpers::ArgToU8(&args[1]) : 0;
        const uint8_t b = (argCount >= 3) ? ArgumentHelpers::ArgToU8(&args[2]) : 0;
        const uint8_t a = (argCount >= 4) ? ArgumentHelpers::ArgToU8(&args[3]) : 255;
        *o = RGBAToLinearRaw(r, g, b, a);
        return true;
    }
}

namespace MathHelpers {
    static bool QuaternionFromAngleAxis(ftdContext *ctx, const ftdValue *args, uint32_t argCount, void *outValue, void *userData) {
        (void)ctx; (void)userData;
        Quaternion *o = (Quaternion *)outValue;
        if (argCount == 2) {
            const float rotationAngleDeg = ArgumentHelpers::ArgToFloat(&args[0]);
            const Vec3f rotationDir = ArgumentHelpers::ArgToVec3f(&args[1]);
            *o = QuatFromAngleAxis(F32DegreesToRadians(rotationAngleDeg), rotationDir);
            return true;
        } else {
            return false;
        }
    }
}

inline void RegisterSerializationTypes(ftdContext *ctx) {
    // Math
    ftdRegisterStruct(ctx, &Vec2fStructDef::Type);
    ftdRegisterStruct(ctx, &Vec3fStructDef::Type);
    ftdRegisterStruct(ctx, &Vec4fStructDef::Type);
    ftdRegisterStruct(ctx, &QuaternionStructDef::Type);

    // Alignment enums
    ftdRegisterEnum(ctx, &HorizontalAlignmentEnumDef::Type);
    ftdRegisterEnum(ctx, &VerticalAlignmentEnumDef::Type);
    ftdRegisterStruct(ctx, &BlockAlignmentStructDef::Type);

    // BackgroundStyle
    ftdRegisterEnum(ctx, &BackgroundKindEnumDef::Type);
    ftdRegisterStruct(ctx, &BackgroundStyleStructDef::Type);

    // FontDefinition
    ftdRegisterStruct(ctx, &FontDefinitionStructDef::Type);

    // TextStyle
    ftdRegisterStruct(ctx, &TextStyleStructDef::Type);

    // Header/Footer Definition
    ftdRegisterStruct(ctx, &HeaderDefinitionStructDef::Type);
    ftdRegisterStruct(ctx, &FooterDefinitionStructDef::Type);

    // Resource
    ftdRegisterEnum(ctx, &ResourceTypeEnumDef::Type);
    ftdRegisterStruct(ctx, &FileResourceStructDef::Type);
    ftdRegisterStruct(ctx, &MemoryResourceStructDef::Type);
    ftdRegisterStruct(ctx, &ResourceStructDef::Type);

    // Sound
    ftdRegisterStruct(ctx, &SoundDefinitionStructDef::Type);

    // Block
    ftdRegisterEnum(ctx, &BlockTypeEnumDef::Type);
    ftdRegisterStruct(ctx, &TextBlockDefinitionStructDef::Type);
    ftdRegisterStruct(ctx, &ImageBlockDefinitionStructDef::Type);
    ftdRegisterStruct(ctx, &BlockDefinitionStructDef::Type);

    // Slide / Presentation
    ftdRegisterStruct(ctx, &SlideDefinitionStructDef::Type);
    ftdRegisterStruct(ctx, &PresentationDefinitionStructDef::Type);
    ftdRegisterStruct(ctx, &PresentationFileStructDef::Type);

    // Color helpers
    ftdRegisterHelper(ctx, "RGBA24", &Vec4fStructDef::Type, ColorHelpers::RGBA24, nullptr);
    ftdRegisterHelper(ctx, "RGBA32", &Vec4fStructDef::Type, ColorHelpers::RGBA32, nullptr);
    ftdRegisterHelper(ctx, "RGBA",   &Vec4fStructDef::Type, ColorHelpers::RGBA, nullptr);

    // Math helpers
    ftdRegisterHelper(ctx, "QuatFromAngleAxis", &QuaternionStructDef::Type, MathHelpers::QuaternionFromAngleAxis, nullptr);

    // BuiltinFonts globals
    ftdRegisterGlobal(ctx, "BuiltinFonts.Debug",             &ResourceStructDef::Type, &BuiltinFonts::Debug);
    ftdRegisterGlobal(ctx, "BuiltinFonts.Arimo",             &ResourceStructDef::Type, &BuiltinFonts::Arimo);
    ftdRegisterGlobal(ctx, "BuiltinFonts.SulphurPoint",      &ResourceStructDef::Type, &BuiltinFonts::SulphurPoint);
    ftdRegisterGlobal(ctx, "BuiltinFonts.BitStreamVerySans", &ResourceStructDef::Type, &BuiltinFonts::BitStreamVerySans);

    // BuiltinImages globals
    ftdRegisterGlobal(ctx, "BuiltinImages.FPLLogo128x128",        &ResourceStructDef::Type, &BuiltinImages::FPLLogo128x128);
    ftdRegisterGlobal(ctx, "BuiltinImages.FPLLogo512x512",        &ResourceStructDef::Type, &BuiltinImages::FPLLogo512x512);
}
