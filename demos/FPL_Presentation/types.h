#pragma once

#include <final_platform_layer.h>

#include <final_math.h>

#include <final_audiosystem.h>

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