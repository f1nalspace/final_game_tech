#pragma once

#include <final_ftd.h>

#include "types.h"
#include "presentation.h"
#include "ftd_helper.h"

namespace Vec2fStructDef {
    static constexpr ftdField Fields[] = {
        FTD_FIELD(Vec2f, x),
        FTD_FIELD(Vec2f, y),
    };
    static constexpr ftdType Type = FTD_TYPE_STRUCT(Vec2f, Fields);
}
FTD_BIND_TYPE(Vec2f, Vec2fStructDef::Type)

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

namespace MemoryResourceStructDef {
    static constexpr ftdField Fields[] = {
        FTD_FIELD_KIND(MemoryResource, data, ftdFieldKind_MemoryData8),
        FTD_FIELD_KIND(MemoryResource, length, ftdFieldKind_MemorySize),
    };
    static constexpr ftdType Type = FTD_TYPE_STRUCT(MemoryResource, Fields);
}

namespace ResourceStructDef {
    static constexpr ftdField Fields[] = {
        FTD_FIELD(Resource, name),
        FTD_FIELD_ENUM(Resource, type, &ResourceTypeEnumDef::Type),
        FTD_FIELD_UNION(Resource, file, &FileResourceStructDef::Type, "type", ResourceType::File),
        FTD_FIELD_UNION(Resource, memory, &MemoryResourceStructDef::Type, "type", ResourceType::Memory),
    };
    static constexpr ftdType Type = FTD_TYPE_STRUCT(Resource, Fields);
}

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
FTD_BIND_TYPE(BackgroundStyle, BackgroundStyleStructDef::Type)

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
FTD_BIND_TYPE(TextStyle, TextStyleStructDef::Type)

namespace SlideDefinitionStructDef {
    static constexpr ftdField Fields[] = {
        FTD_FIELD(SlideDefinition, name),
        FTD_FIELD_ARRAY_DATA(SlideDefinition, blocks),
        FTD_FIELD_ARRAY_DATA(SlideDefinition, sounds),
        FTD_FIELD_STRUCT(SlideDefinition, background),
        FTD_FIELD_STRUCT(SlideDefinition, rotation),
        FTD_FIELD_ARRAY_SIZE(SlideDefinition, blockCount),
        FTD_FIELD_ARRAY_SIZE(SlideDefinition, soundCount),
        FTD_FIELD(SlideDefinition, autoTransitionInSeconds),
    };
    static constexpr ftdType Type = {
        .name = "Slide", .size = sizeof(SlideDefinition), .align = alignof(SlideDefinition),
        .fields = (Fields), .fieldCount = (uint32_t)fplArrayCount(Fields)
    };
}
FTD_BIND_TYPE(SlideDefinition, SlideDefinitionStructDef::Type)

// Color helpers callable from .ftd source: RGBA24(hex, alpha) -> Vec4f.
namespace ColorHelpers {
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

    // RGBA24(0xRRGGBB, alpha) packs a 24-bit color literal plus an 8-bit alpha into a Vec4f with each channel normalized to 0..1.
    // Missing alpha defaults to 255 (fully opaque).
    static bool RGBA24(ftdContext *ctx, const ftdValue *args, uint32_t argCount, void *outValue, void *userData) {
        (void)ctx; (void)userData;
        Vec4f *o = (Vec4f *)outValue;
        const uint32_t hex = (argCount >= 1) ? ArgToU32(&args[0]) : 0;
        const uint8_t a  = (argCount >= 2) ? ArgToU8(&args[1]) : 255;
        *o = RGBAToLinearHex24(hex, a);
        return true;
    }

    // RGBA32(0xRRGGBBAA) packs a 32-bit color literal into a Vec4f with each channel normalized to 0..1.
    static bool RGBA32(ftdContext *ctx, const ftdValue *args, uint32_t argCount, void *outValue, void *userData) {
        (void)ctx; (void)userData;
        Vec4f *o = (Vec4f *)outValue;
        const uint32_t hex = (argCount == 1) ? ArgToU32(&args[0]) : 0;
        *o = BGRAUnpack4x8(hex);
        return true;
    }

    // RGBA(r, g, b, a) -> Vec4f
    // Each argument is interpreted as an 8-bit channel value (0..255) and normalized into 0..1. Extra/missing arguments default to 0 for RGB and 255 for the optional alpha.
    static bool RGBA(ftdContext *ctx, const ftdValue *args, uint32_t argCount, void *outValue, void *userData) {
        (void)ctx; (void)userData;
        Vec4f *o = (Vec4f *)outValue;
        const uint8_t r = (argCount >= 1) ? ArgToU8(&args[0]) : 0;
        const uint8_t g = (argCount >= 2) ? ArgToU8(&args[1]) : 0;
        const uint8_t b = (argCount >= 3) ? ArgToU8(&args[2]) : 0;
        const uint8_t a = (argCount >= 4) ? ArgToU8(&args[3]) : 255;
        *o = RGBAToLinearRaw(r, g, b, a);
        return true;
    }
}

inline void RegisterSerializationTypes(ftdContext *ctx) {
    // Math
    ftdRegisterStruct(ctx, &Vec2fStructDef::Type);
    ftdRegisterStruct(ctx, &Vec4fStructDef::Type);
    ftdRegisterStruct(ctx, &QuaternionStructDef::Type);

    // BackgroundStyle
    ftdRegisterEnum(ctx, &BackgroundKindEnumDef::Type);
    ftdRegisterStruct(ctx, &BackgroundStyleStructDef::Type);

    // TextStyle
    ftdRegisterStruct(ctx, &TextStyleStructDef::Type);

    // Resource
    ftdRegisterEnum(ctx, &ResourceTypeEnumDef::Type);
    ftdRegisterStruct(ctx, &FileResourceStructDef::Type);
    ftdRegisterStruct(ctx, &MemoryResourceStructDef::Type);
    ftdRegisterStruct(ctx, &ResourceStructDef::Type);

    // SlideDefinition
    ftdRegisterStruct(ctx, &SlideDefinitionStructDef::Type);

    // Color helpers
    ftdRegisterHelper(ctx, "RGBA24", &Vec4fStructDef::Type, ColorHelpers::RGBA24, nullptr);
    ftdRegisterHelper(ctx, "RGBA32", &Vec4fStructDef::Type, ColorHelpers::RGBA32, nullptr);
    ftdRegisterHelper(ctx, "RGBA",   &Vec4fStructDef::Type, ColorHelpers::RGBA, nullptr);
}
