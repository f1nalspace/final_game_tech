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

struct Background {
	BackgroundKind kind;
	Vec4f primaryColor;
	Vec4f secondaryColor;
};

struct TextStyle {
	Background background;
	Vec4f foregroundColor;
	Vec4f shadowColor;
	Vec2f shadowOffset;
	b32 drawShadow;
};