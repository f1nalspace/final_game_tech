#pragma once

#include <final_platform_layer.h>

#include <final_math.h>

typedef int32_t b32;

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

struct BackgroundStyle {
	BackgroundKind kind;
	Vec4f primaryColor;
	Vec4f secondaryColor;
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

enum class ImageResourceType {
	FPLLogo128x128 = 0,
	FPLLogo512x512,
	FPLFeaturesImage,
};

struct ImageResource {
	const uint8_t *bytes;
	const char *name;
	const size_t length;
	ImageResourceType type;
};