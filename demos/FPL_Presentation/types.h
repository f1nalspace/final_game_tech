#pragma once

#include <final_platform_layer.h>

#include <final_math.h>

#include <final_audiosystem.h>

#include <final_ftd.h>

typedef int32_t b32;

namespace Vec4fType {
	static constexpr ftdField Fields[] = {
		{ "x", offsetof(Vec4f, x), ftdFieldKind_F32 },
		{ "y", offsetof(Vec4f, y), ftdFieldKind_F32 },
		{ "z", offsetof(Vec4f, z), ftdFieldKind_F32 },
		{ "w", offsetof(Vec4f, w), ftdFieldKind_F32 },
	};
	static constexpr ftdType Type = {
		.name = "Vec4f", .size = sizeof(Vec4f), .align = alignof(Vec4f),
		.fields = Fields, .fieldCount = fplArrayCount(Fields),
	};
};

enum class HorizontalAlignment {
	Left = 0,
	Center,
	Right
};

enum class VerticalAlignment {
	Top = 0,
	Middle,
	Bottom
};

enum class BackgroundKind {
	None = 0,
	Solid,
	GradientHorizontal,
	GradientVertical,
	HalfGradientHorizontal,
	HalfGradientVertical,
};

namespace BackgroundKindTypeDef {
	static constexpr ftdEnumValue Values[] = {
		{ "None",   0 },
		{ "Solid",   1 },
		{ "GradientHorizontal", 2 },
		{ "GradientVertical", 3 },
		{ "HalfGradientHorizontal", 4 },
		{ "HalfGradientVertical", 5 },
	};
	static const ftdType Type = {
		.name = "BackgroundKind",
		.size = sizeof(BackgroundKind), .align = alignof(BackgroundKind),  // = 8/8 on 64-bit
		.enumValues = Values, .enumValueCount = fplArrayCount(Values),
	};
}

struct BackgroundStyle {
	BackgroundKind kind;
	Vec4f primaryColor;
	Vec4f secondaryColor;
};

namespace BackgroundStyleTypeDef {
	static constexpr ftdField Fields[] = {
		{ "kind", offsetof(BackgroundStyle, kind), ftdFieldKind_Enum },
		{ "primaryColor", offsetof(BackgroundStyle, primaryColor), ftdFieldKind_Struct },
		{ "secondaryColor", offsetof(BackgroundStyle, primaryColor), ftdFieldKind_Struct },
	};
	static constexpr ftdType Type = {
		.name = "BackgroundStyle", .size = sizeof(BackgroundStyle), .align = alignof(BackgroundStyle),
		.fields = Fields, .fieldCount = fplArrayCount(Fields),
	};
};

static BackgroundStyle MakeBackground(const Vec4f &primaryColor, const Vec4f& secondaryColor, const BackgroundKind kind = BackgroundKind::HalfGradientHorizontal) {
	BackgroundStyle result;
	result.kind = kind;
	result.primaryColor = primaryColor;
	result.secondaryColor = secondaryColor;
	return(result);
}

enum class StrokeKind {
	None = 0,
	Solid
};

struct StrokeStyle {
	StrokeKind kind;
	Vec4f color;
	float width;
};

struct TextStyle {
	BackgroundStyle background;
	Vec4f foregroundColor;
	Vec4f shadowColor;
	Vec2f shadowOffset;
	b32 drawShadow;
};

static TextStyle MakeTextStyle(const Vec4f &foregroundColor) {
	TextStyle result = {};
	result.foregroundColor = foregroundColor;
	return result;
}

static TextStyle MakeTextStyle(const Vec4f &foregroundColor, const Vec4f &backgroundColor) {
	TextStyle result = {};
	result.background.kind = BackgroundKind::Solid;
	result.background.primaryColor = backgroundColor;
	result.foregroundColor = foregroundColor;
	return result;
}

static TextStyle MakeTextStyle(const Vec4f &foregroundColor, const BackgroundKind backgroundKind, const Vec4f &primaryBackgroundColor, const Vec4f &secondaryBackgroundColor) {
	TextStyle result = {};
	result.background.kind = backgroundKind;
	result.background.primaryColor = primaryBackgroundColor;
	result.background.secondaryColor = secondaryBackgroundColor;
	result.foregroundColor = foregroundColor;
	return result;
}