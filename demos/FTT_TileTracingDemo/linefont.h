/*
-------------------------------------------------------------------------------
Name:
	FTT | Line-stroke ASCII font

Description:
	A minimal ASCII font for legacy/fixed-function OpenGL where every glyph
	is defined as a small set of straight line segments on a fixed grid -
	no curves, no glyph atlas/texture, just GL_LINES.

	Include this header after final_dynamic_opengl.h (or any header that
	brings in the legacy GL entry points), since the draw functions call
	straight into glBegin/glVertex2f/glEnd/glLineWidth.

Requirements:
	- C17 Compiler
	- Legacy/fixed-function OpenGL (glBegin/glEnd)

License:
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#ifndef FTT_LINEFONT_H
#define FTT_LINEFONT_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

typedef struct fttFontLineSegment {
	uint8_t x0, y0, x1, y1;
} fttFontLineSegment;

typedef struct fttFontGlyph {
	const fttFontLineSegment *segments;
	uint8_t segmentCount;
} fttFontGlyph;

enum {
	FontGridColumns = 5,
	FontGridRows = 9,
	FontGlyphSpacingColumns = 1,

	FontColL = 0,
	FontColLM = 1,
	FontColM = 2,
	FontColRM = 3,
	FontColR = 4,

	// Row values increase downward (0 = cap top, 8 = descender bottom) so glyphs render
	// upright directly under a y-down pixel-space ortho (originY + row * scale).
	FontRowCap = 0,
	FontRowUpperMid = 1,
	FontRowXHeight = 2,
	FontRowMiddle = 3,
	FontRowLowerMid = 4,
	FontRowLowQuarter = 5,
	FontRowBase = 6,
	FontRowSubBase = 7,
	FontRowDescender = 8,
};

#define FttFontCountOf(array) (uint8_t)(sizeof(array) / sizeof((array)[0]))

// Uppercase A-Z

static const fttFontLineSegment FontGlyphSegments_A[] = {
	{ FontColM, FontRowCap, FontColL, FontRowBase },
	{ FontColM, FontRowCap, FontColR, FontRowBase },
	{ FontColLM, FontRowLowerMid, FontColRM, FontRowLowerMid },
};
static const fttFontLineSegment FontGlyphSegments_B[] = {
	{ FontColL, FontRowBase, FontColL, FontRowCap },
	{ FontColL, FontRowCap, FontColRM, FontRowCap },
	{ FontColRM, FontRowCap, FontColRM, FontRowMiddle },
	{ FontColL, FontRowMiddle, FontColRM, FontRowMiddle },
	{ FontColRM, FontRowMiddle, FontColRM, FontRowBase },
	{ FontColL, FontRowBase, FontColRM, FontRowBase },
};
static const fttFontLineSegment FontGlyphSegments_C[] = {
	{ FontColR, FontRowCap, FontColL, FontRowCap },
	{ FontColL, FontRowCap, FontColL, FontRowBase },
	{ FontColL, FontRowBase, FontColR, FontRowBase },
};
static const fttFontLineSegment FontGlyphSegments_D[] = {
	{ FontColL, FontRowBase, FontColL, FontRowCap },
	{ FontColL, FontRowCap, FontColM, FontRowCap },
	{ FontColM, FontRowCap, FontColR, FontRowUpperMid },
	{ FontColR, FontRowUpperMid, FontColR, FontRowLowQuarter },
	{ FontColR, FontRowLowQuarter, FontColM, FontRowBase },
	{ FontColM, FontRowBase, FontColL, FontRowBase },
};
static const fttFontLineSegment FontGlyphSegments_E[] = {
	{ FontColR, FontRowCap, FontColL, FontRowCap },
	{ FontColL, FontRowCap, FontColL, FontRowBase },
	{ FontColL, FontRowBase, FontColR, FontRowBase },
	{ FontColL, FontRowMiddle, FontColRM, FontRowMiddle },
};
static const fttFontLineSegment FontGlyphSegments_F[] = {
	{ FontColL, FontRowBase, FontColL, FontRowCap },
	{ FontColL, FontRowCap, FontColR, FontRowCap },
	{ FontColL, FontRowMiddle, FontColRM, FontRowMiddle },
};
static const fttFontLineSegment FontGlyphSegments_G[] = {
	{ FontColR, FontRowCap, FontColL, FontRowCap },
	{ FontColL, FontRowCap, FontColL, FontRowBase },
	{ FontColL, FontRowBase, FontColR, FontRowBase },
	{ FontColR, FontRowBase, FontColR, FontRowMiddle },
	{ FontColRM, FontRowMiddle, FontColR, FontRowMiddle },
};
static const fttFontLineSegment FontGlyphSegments_H[] = {
	{ FontColL, FontRowBase, FontColL, FontRowCap },
	{ FontColR, FontRowBase, FontColR, FontRowCap },
	{ FontColL, FontRowMiddle, FontColR, FontRowMiddle },
};
static const fttFontLineSegment FontGlyphSegments_I[] = {
	{ FontColL, FontRowCap, FontColR, FontRowCap },
	{ FontColM, FontRowCap, FontColM, FontRowBase },
	{ FontColL, FontRowBase, FontColR, FontRowBase },
};
static const fttFontLineSegment FontGlyphSegments_J[] = {
	{ FontColR, FontRowCap, FontColR, FontRowLowQuarter },
	{ FontColR, FontRowLowQuarter, FontColM, FontRowBase },
	{ FontColM, FontRowBase, FontColL, FontRowLowQuarter },
};
static const fttFontLineSegment FontGlyphSegments_K[] = {
	{ FontColL, FontRowBase, FontColL, FontRowCap },
	{ FontColR, FontRowCap, FontColL, FontRowMiddle },
	{ FontColL, FontRowMiddle, FontColR, FontRowBase },
};
static const fttFontLineSegment FontGlyphSegments_L[] = {
	{ FontColL, FontRowCap, FontColL, FontRowBase },
	{ FontColL, FontRowBase, FontColR, FontRowBase },
};
static const fttFontLineSegment FontGlyphSegments_M[] = {
	{ FontColL, FontRowBase, FontColL, FontRowCap },
	{ FontColL, FontRowCap, FontColM, FontRowMiddle },
	{ FontColM, FontRowMiddle, FontColR, FontRowCap },
	{ FontColR, FontRowCap, FontColR, FontRowBase },
};
static const fttFontLineSegment FontGlyphSegments_N[] = {
	{ FontColL, FontRowBase, FontColL, FontRowCap },
	{ FontColL, FontRowCap, FontColR, FontRowBase },
	{ FontColR, FontRowBase, FontColR, FontRowCap },
};
static const fttFontLineSegment FontGlyphSegments_O[] = {
	{ FontColL, FontRowBase, FontColL, FontRowCap },
	{ FontColL, FontRowCap, FontColR, FontRowCap },
	{ FontColR, FontRowCap, FontColR, FontRowBase },
	{ FontColR, FontRowBase, FontColL, FontRowBase },
};
static const fttFontLineSegment FontGlyphSegments_P[] = {
	{ FontColL, FontRowBase, FontColL, FontRowCap },
	{ FontColL, FontRowCap, FontColR, FontRowCap },
	{ FontColR, FontRowCap, FontColR, FontRowMiddle },
	{ FontColR, FontRowMiddle, FontColL, FontRowMiddle },
};
static const fttFontLineSegment FontGlyphSegments_Q[] = {
	{ FontColL, FontRowBase, FontColL, FontRowCap },
	{ FontColL, FontRowCap, FontColR, FontRowCap },
	{ FontColR, FontRowCap, FontColR, FontRowBase },
	{ FontColR, FontRowBase, FontColL, FontRowBase },
	{ FontColM, FontRowLowerMid, FontColR, FontRowDescender },
};
static const fttFontLineSegment FontGlyphSegments_R[] = {
	{ FontColL, FontRowBase, FontColL, FontRowCap },
	{ FontColL, FontRowCap, FontColR, FontRowCap },
	{ FontColR, FontRowCap, FontColR, FontRowMiddle },
	{ FontColR, FontRowMiddle, FontColL, FontRowMiddle },
	{ FontColL, FontRowMiddle, FontColR, FontRowBase },
};
static const fttFontLineSegment FontGlyphSegments_S[] = {
	{ FontColR, FontRowCap, FontColL, FontRowCap },
	{ FontColL, FontRowCap, FontColL, FontRowMiddle },
	{ FontColL, FontRowMiddle, FontColR, FontRowMiddle },
	{ FontColR, FontRowMiddle, FontColR, FontRowBase },
	{ FontColR, FontRowBase, FontColL, FontRowBase },
};
static const fttFontLineSegment FontGlyphSegments_T[] = {
	{ FontColL, FontRowCap, FontColR, FontRowCap },
	{ FontColM, FontRowCap, FontColM, FontRowBase },
};
static const fttFontLineSegment FontGlyphSegments_U[] = {
	{ FontColL, FontRowCap, FontColL, FontRowLowQuarter },
	{ FontColL, FontRowLowQuarter, FontColM, FontRowBase },
	{ FontColM, FontRowBase, FontColR, FontRowLowQuarter },
	{ FontColR, FontRowLowQuarter, FontColR, FontRowCap },
};
static const fttFontLineSegment FontGlyphSegments_V[] = {
	{ FontColL, FontRowCap, FontColM, FontRowBase },
	{ FontColM, FontRowBase, FontColR, FontRowCap },
};
static const fttFontLineSegment FontGlyphSegments_W[] = {
	{ FontColL, FontRowCap, FontColLM, FontRowBase },
	{ FontColLM, FontRowBase, FontColM, FontRowMiddle },
	{ FontColM, FontRowMiddle, FontColRM, FontRowBase },
	{ FontColRM, FontRowBase, FontColR, FontRowCap },
};
static const fttFontLineSegment FontGlyphSegments_X[] = {
	{ FontColL, FontRowCap, FontColR, FontRowBase },
	{ FontColR, FontRowCap, FontColL, FontRowBase },
};
static const fttFontLineSegment FontGlyphSegments_Y[] = {
	{ FontColL, FontRowCap, FontColM, FontRowMiddle },
	{ FontColR, FontRowCap, FontColM, FontRowMiddle },
	{ FontColM, FontRowMiddle, FontColM, FontRowBase },
};
static const fttFontLineSegment FontGlyphSegments_Z[] = {
	{ FontColL, FontRowCap, FontColR, FontRowCap },
	{ FontColR, FontRowCap, FontColL, FontRowBase },
	{ FontColL, FontRowBase, FontColR, FontRowBase },
};

// Lowercase a-z

static const fttFontLineSegment FontGlyphSegments_a[] = {
	{ FontColR, FontRowXHeight, FontColR, FontRowBase },
	{ FontColR, FontRowXHeight, FontColLM, FontRowXHeight },
	{ FontColR, FontRowBase, FontColL, FontRowBase },
	{ FontColL, FontRowBase, FontColL, FontRowLowerMid },
	{ FontColL, FontRowLowerMid, FontColR, FontRowLowerMid },
};
static const fttFontLineSegment FontGlyphSegments_b[] = {
	{ FontColL, FontRowBase, FontColL, FontRowCap },
	{ FontColL, FontRowXHeight, FontColR, FontRowXHeight },
	{ FontColR, FontRowXHeight, FontColR, FontRowBase },
	{ FontColR, FontRowBase, FontColL, FontRowBase },
};
static const fttFontLineSegment FontGlyphSegments_c[] = {
	{ FontColR, FontRowXHeight, FontColL, FontRowXHeight },
	{ FontColL, FontRowXHeight, FontColL, FontRowBase },
	{ FontColL, FontRowBase, FontColR, FontRowBase },
};
static const fttFontLineSegment FontGlyphSegments_d[] = {
	{ FontColR, FontRowBase, FontColR, FontRowCap },
	{ FontColR, FontRowXHeight, FontColL, FontRowXHeight },
	{ FontColL, FontRowXHeight, FontColL, FontRowBase },
	{ FontColL, FontRowBase, FontColR, FontRowBase },
};
static const fttFontLineSegment FontGlyphSegments_e[] = {
	{ FontColL, FontRowXHeight, FontColR, FontRowXHeight },
	{ FontColL, FontRowXHeight, FontColL, FontRowBase },
	{ FontColL, FontRowBase, FontColR, FontRowBase },
	{ FontColR, FontRowXHeight, FontColR, FontRowLowerMid },
	{ FontColL, FontRowLowerMid, FontColR, FontRowLowerMid },
};
static const fttFontLineSegment FontGlyphSegments_f[] = {
	{ FontColM, FontRowBase, FontColM, FontRowUpperMid },
	{ FontColM, FontRowUpperMid, FontColR, FontRowCap },
	{ FontColL, FontRowXHeight, FontColR, FontRowXHeight },
};
static const fttFontLineSegment FontGlyphSegments_g[] = {
	{ FontColL, FontRowXHeight, FontColR, FontRowXHeight },
	{ FontColR, FontRowXHeight, FontColR, FontRowBase },
	{ FontColR, FontRowBase, FontColL, FontRowBase },
	{ FontColL, FontRowBase, FontColL, FontRowXHeight },
	{ FontColR, FontRowBase, FontColR, FontRowSubBase },
	{ FontColR, FontRowSubBase, FontColL, FontRowDescender },
};
static const fttFontLineSegment FontGlyphSegments_h[] = {
	{ FontColL, FontRowBase, FontColL, FontRowCap },
	{ FontColL, FontRowXHeight, FontColR, FontRowXHeight },
	{ FontColR, FontRowXHeight, FontColR, FontRowBase },
};
static const fttFontLineSegment FontGlyphSegments_i[] = {
	{ FontColLM, FontRowCap, FontColRM, FontRowCap },
	{ FontColM, FontRowBase, FontColM, FontRowXHeight },
};
static const fttFontLineSegment FontGlyphSegments_j[] = {
	{ FontColLM, FontRowCap, FontColRM, FontRowCap },
	{ FontColM, FontRowXHeight, FontColM, FontRowSubBase },
	{ FontColM, FontRowSubBase, FontColL, FontRowDescender },
};
static const fttFontLineSegment FontGlyphSegments_k[] = {
	{ FontColL, FontRowBase, FontColL, FontRowCap },
	{ FontColR, FontRowXHeight, FontColL, FontRowLowerMid },
	{ FontColL, FontRowLowerMid, FontColR, FontRowBase },
};
static const fttFontLineSegment FontGlyphSegments_l[] = {
	{ FontColM, FontRowBase, FontColM, FontRowCap },
};
static const fttFontLineSegment FontGlyphSegments_m[] = {
	{ FontColL, FontRowBase, FontColL, FontRowXHeight },
	{ FontColL, FontRowXHeight, FontColM, FontRowXHeight },
	{ FontColM, FontRowXHeight, FontColM, FontRowBase },
	{ FontColM, FontRowXHeight, FontColR, FontRowXHeight },
	{ FontColR, FontRowXHeight, FontColR, FontRowBase },
};
static const fttFontLineSegment FontGlyphSegments_n[] = {
	{ FontColL, FontRowBase, FontColL, FontRowXHeight },
	{ FontColL, FontRowXHeight, FontColR, FontRowXHeight },
	{ FontColR, FontRowXHeight, FontColR, FontRowBase },
};
static const fttFontLineSegment FontGlyphSegments_o[] = {
	{ FontColL, FontRowBase, FontColL, FontRowXHeight },
	{ FontColL, FontRowXHeight, FontColR, FontRowXHeight },
	{ FontColR, FontRowXHeight, FontColR, FontRowBase },
	{ FontColR, FontRowBase, FontColL, FontRowBase },
};
static const fttFontLineSegment FontGlyphSegments_p[] = {
	{ FontColL, FontRowDescender, FontColL, FontRowXHeight },
	{ FontColL, FontRowXHeight, FontColR, FontRowXHeight },
	{ FontColR, FontRowXHeight, FontColR, FontRowBase },
	{ FontColR, FontRowBase, FontColL, FontRowBase },
};
static const fttFontLineSegment FontGlyphSegments_q[] = {
	{ FontColR, FontRowDescender, FontColR, FontRowXHeight },
	{ FontColR, FontRowXHeight, FontColL, FontRowXHeight },
	{ FontColL, FontRowXHeight, FontColL, FontRowBase },
	{ FontColL, FontRowBase, FontColR, FontRowBase },
};
static const fttFontLineSegment FontGlyphSegments_r[] = {
	{ FontColL, FontRowBase, FontColL, FontRowXHeight },
	{ FontColL, FontRowXHeight, FontColR, FontRowXHeight },
	{ FontColR, FontRowXHeight, FontColR, FontRowLowerMid },
};
static const fttFontLineSegment FontGlyphSegments_s[] = {
	{ FontColR, FontRowXHeight, FontColL, FontRowXHeight },
	{ FontColL, FontRowXHeight, FontColL, FontRowLowerMid },
	{ FontColL, FontRowLowerMid, FontColR, FontRowLowerMid },
	{ FontColR, FontRowLowerMid, FontColR, FontRowBase },
	{ FontColR, FontRowBase, FontColL, FontRowBase },
};
static const fttFontLineSegment FontGlyphSegments_t[] = {
	{ FontColM, FontRowUpperMid, FontColM, FontRowBase },
	{ FontColL, FontRowXHeight, FontColR, FontRowXHeight },
	{ FontColM, FontRowBase, FontColRM, FontRowSubBase },
};
static const fttFontLineSegment FontGlyphSegments_u[] = {
	{ FontColL, FontRowXHeight, FontColL, FontRowBase },
	{ FontColL, FontRowBase, FontColR, FontRowBase },
	{ FontColR, FontRowBase, FontColR, FontRowXHeight },
};
static const fttFontLineSegment FontGlyphSegments_v[] = {
	{ FontColL, FontRowXHeight, FontColM, FontRowBase },
	{ FontColM, FontRowBase, FontColR, FontRowXHeight },
};
static const fttFontLineSegment FontGlyphSegments_w[] = {
	{ FontColL, FontRowXHeight, FontColLM, FontRowBase },
	{ FontColLM, FontRowBase, FontColM, FontRowLowerMid },
	{ FontColM, FontRowLowerMid, FontColRM, FontRowBase },
	{ FontColRM, FontRowBase, FontColR, FontRowXHeight },
};
static const fttFontLineSegment FontGlyphSegments_x[] = {
	{ FontColL, FontRowXHeight, FontColR, FontRowBase },
	{ FontColR, FontRowXHeight, FontColL, FontRowBase },
};
static const fttFontLineSegment FontGlyphSegments_y[] = {
	{ FontColL, FontRowXHeight, FontColM, FontRowLowerMid },
	{ FontColR, FontRowXHeight, FontColM, FontRowLowerMid },
	{ FontColM, FontRowLowerMid, FontColLM, FontRowDescender },
};
static const fttFontLineSegment FontGlyphSegments_z[] = {
	{ FontColL, FontRowXHeight, FontColR, FontRowXHeight },
	{ FontColR, FontRowXHeight, FontColL, FontRowBase },
	{ FontColL, FontRowBase, FontColR, FontRowBase },
};

// Digits 0-9

static const fttFontLineSegment FontGlyphSegments_0[] = {
	{ FontColL, FontRowBase, FontColL, FontRowCap },
	{ FontColL, FontRowCap, FontColR, FontRowCap },
	{ FontColR, FontRowCap, FontColR, FontRowBase },
	{ FontColR, FontRowBase, FontColL, FontRowBase },
	{ FontColL, FontRowBase, FontColR, FontRowCap },
};
static const fttFontLineSegment FontGlyphSegments_1[] = {
	{ FontColLM, FontRowUpperMid, FontColM, FontRowCap },
	{ FontColM, FontRowCap, FontColM, FontRowBase },
	{ FontColL, FontRowBase, FontColR, FontRowBase },
};
static const fttFontLineSegment FontGlyphSegments_2[] = {
	{ FontColL, FontRowUpperMid, FontColM, FontRowCap },
	{ FontColM, FontRowCap, FontColR, FontRowUpperMid },
	{ FontColR, FontRowUpperMid, FontColL, FontRowBase },
	{ FontColL, FontRowBase, FontColR, FontRowBase },
};
static const fttFontLineSegment FontGlyphSegments_3[] = {
	{ FontColL, FontRowCap, FontColR, FontRowCap },
	{ FontColR, FontRowCap, FontColM, FontRowMiddle },
	{ FontColM, FontRowMiddle, FontColR, FontRowBase },
	{ FontColR, FontRowBase, FontColL, FontRowBase },
};
static const fttFontLineSegment FontGlyphSegments_4[] = {
	{ FontColR, FontRowCap, FontColR, FontRowBase },
	{ FontColR, FontRowCap, FontColL, FontRowMiddle },
	{ FontColL, FontRowMiddle, FontColR, FontRowMiddle },
};
static const fttFontLineSegment FontGlyphSegments_5[] = {
	{ FontColR, FontRowCap, FontColL, FontRowCap },
	{ FontColL, FontRowCap, FontColL, FontRowMiddle },
	{ FontColL, FontRowMiddle, FontColR, FontRowMiddle },
	{ FontColR, FontRowMiddle, FontColR, FontRowBase },
	{ FontColR, FontRowBase, FontColL, FontRowBase },
};
static const fttFontLineSegment FontGlyphSegments_6[] = {
	{ FontColR, FontRowCap, FontColL, FontRowUpperMid },
	{ FontColL, FontRowUpperMid, FontColL, FontRowBase },
	{ FontColL, FontRowBase, FontColR, FontRowBase },
	{ FontColR, FontRowBase, FontColR, FontRowMiddle },
	{ FontColR, FontRowMiddle, FontColL, FontRowMiddle },
};
static const fttFontLineSegment FontGlyphSegments_7[] = {
	{ FontColL, FontRowCap, FontColR, FontRowCap },
	{ FontColR, FontRowCap, FontColLM, FontRowBase },
};
static const fttFontLineSegment FontGlyphSegments_8[] = {
	{ FontColL, FontRowBase, FontColL, FontRowCap },
	{ FontColL, FontRowCap, FontColR, FontRowCap },
	{ FontColR, FontRowCap, FontColR, FontRowBase },
	{ FontColR, FontRowBase, FontColL, FontRowBase },
	{ FontColL, FontRowMiddle, FontColR, FontRowMiddle },
};
static const fttFontLineSegment FontGlyphSegments_9[] = {
	{ FontColR, FontRowBase, FontColR, FontRowCap },
	{ FontColR, FontRowCap, FontColL, FontRowCap },
	{ FontColL, FontRowCap, FontColL, FontRowMiddle },
	{ FontColL, FontRowMiddle, FontColR, FontRowMiddle },
};

// Punctuation

static const fttFontLineSegment FontGlyphSegments_Period[] = {
	{ FontColLM, FontRowSubBase, FontColRM, FontRowSubBase },
	{ FontColRM, FontRowSubBase, FontColRM, FontRowDescender },
	{ FontColRM, FontRowDescender, FontColLM, FontRowDescender },
	{ FontColLM, FontRowDescender, FontColLM, FontRowSubBase },
};
static const fttFontLineSegment FontGlyphSegments_Underscore[] = {
	{ FontColL, FontRowDescender, FontColR, FontRowDescender },
};
static const fttFontLineSegment FontGlyphSegments_Minus[] = {
	{ FontColL, FontRowMiddle, FontColR, FontRowMiddle },
};
static const fttFontLineSegment FontGlyphSegments_Slash[] = {
	{ FontColL, FontRowBase, FontColR, FontRowCap },
};
static const fttFontLineSegment FontGlyphSegments_Backslash[] = {
	{ FontColL, FontRowCap, FontColR, FontRowBase },
};
static const fttFontLineSegment FontGlyphSegments_Colon[] = {
	// Upper dot
	{ FontColLM, FontRowMiddle, FontColRM, FontRowMiddle },
	{ FontColRM, FontRowMiddle, FontColRM, FontRowLowerMid },
	{ FontColRM, FontRowLowerMid, FontColLM, FontRowLowerMid },
	{ FontColLM, FontRowLowerMid, FontColLM, FontRowMiddle },
	// Lower dot
	{ FontColLM, FontRowLowQuarter, FontColRM, FontRowLowQuarter },
	{ FontColRM, FontRowLowQuarter, FontColRM, FontRowBase },
	{ FontColRM, FontRowBase, FontColLM, FontRowBase },
	{ FontColLM, FontRowBase, FontColLM, FontRowLowQuarter },
};

static const fttFontGlyph FontGlyphTable[256] = {
	['A'] = { FontGlyphSegments_A, FttFontCountOf(FontGlyphSegments_A) },
	['B'] = { FontGlyphSegments_B, FttFontCountOf(FontGlyphSegments_B) },
	['C'] = { FontGlyphSegments_C, FttFontCountOf(FontGlyphSegments_C) },
	['D'] = { FontGlyphSegments_D, FttFontCountOf(FontGlyphSegments_D) },
	['E'] = { FontGlyphSegments_E, FttFontCountOf(FontGlyphSegments_E) },
	['F'] = { FontGlyphSegments_F, FttFontCountOf(FontGlyphSegments_F) },
	['G'] = { FontGlyphSegments_G, FttFontCountOf(FontGlyphSegments_G) },
	['H'] = { FontGlyphSegments_H, FttFontCountOf(FontGlyphSegments_H) },
	['I'] = { FontGlyphSegments_I, FttFontCountOf(FontGlyphSegments_I) },
	['J'] = { FontGlyphSegments_J, FttFontCountOf(FontGlyphSegments_J) },
	['K'] = { FontGlyphSegments_K, FttFontCountOf(FontGlyphSegments_K) },
	['L'] = { FontGlyphSegments_L, FttFontCountOf(FontGlyphSegments_L) },
	['M'] = { FontGlyphSegments_M, FttFontCountOf(FontGlyphSegments_M) },
	['N'] = { FontGlyphSegments_N, FttFontCountOf(FontGlyphSegments_N) },
	['O'] = { FontGlyphSegments_O, FttFontCountOf(FontGlyphSegments_O) },
	['P'] = { FontGlyphSegments_P, FttFontCountOf(FontGlyphSegments_P) },
	['Q'] = { FontGlyphSegments_Q, FttFontCountOf(FontGlyphSegments_Q) },
	['R'] = { FontGlyphSegments_R, FttFontCountOf(FontGlyphSegments_R) },
	['S'] = { FontGlyphSegments_S, FttFontCountOf(FontGlyphSegments_S) },
	['T'] = { FontGlyphSegments_T, FttFontCountOf(FontGlyphSegments_T) },
	['U'] = { FontGlyphSegments_U, FttFontCountOf(FontGlyphSegments_U) },
	['V'] = { FontGlyphSegments_V, FttFontCountOf(FontGlyphSegments_V) },
	['W'] = { FontGlyphSegments_W, FttFontCountOf(FontGlyphSegments_W) },
	['X'] = { FontGlyphSegments_X, FttFontCountOf(FontGlyphSegments_X) },
	['Y'] = { FontGlyphSegments_Y, FttFontCountOf(FontGlyphSegments_Y) },
	['Z'] = { FontGlyphSegments_Z, FttFontCountOf(FontGlyphSegments_Z) },

	['a'] = { FontGlyphSegments_a, FttFontCountOf(FontGlyphSegments_a) },
	['b'] = { FontGlyphSegments_b, FttFontCountOf(FontGlyphSegments_b) },
	['c'] = { FontGlyphSegments_c, FttFontCountOf(FontGlyphSegments_c) },
	['d'] = { FontGlyphSegments_d, FttFontCountOf(FontGlyphSegments_d) },
	['e'] = { FontGlyphSegments_e, FttFontCountOf(FontGlyphSegments_e) },
	['f'] = { FontGlyphSegments_f, FttFontCountOf(FontGlyphSegments_f) },
	['g'] = { FontGlyphSegments_g, FttFontCountOf(FontGlyphSegments_g) },
	['h'] = { FontGlyphSegments_h, FttFontCountOf(FontGlyphSegments_h) },
	['i'] = { FontGlyphSegments_i, FttFontCountOf(FontGlyphSegments_i) },
	['j'] = { FontGlyphSegments_j, FttFontCountOf(FontGlyphSegments_j) },
	['k'] = { FontGlyphSegments_k, FttFontCountOf(FontGlyphSegments_k) },
	['l'] = { FontGlyphSegments_l, FttFontCountOf(FontGlyphSegments_l) },
	['m'] = { FontGlyphSegments_m, FttFontCountOf(FontGlyphSegments_m) },
	['n'] = { FontGlyphSegments_n, FttFontCountOf(FontGlyphSegments_n) },
	['o'] = { FontGlyphSegments_o, FttFontCountOf(FontGlyphSegments_o) },
	['p'] = { FontGlyphSegments_p, FttFontCountOf(FontGlyphSegments_p) },
	['q'] = { FontGlyphSegments_q, FttFontCountOf(FontGlyphSegments_q) },
	['r'] = { FontGlyphSegments_r, FttFontCountOf(FontGlyphSegments_r) },
	['s'] = { FontGlyphSegments_s, FttFontCountOf(FontGlyphSegments_s) },
	['t'] = { FontGlyphSegments_t, FttFontCountOf(FontGlyphSegments_t) },
	['u'] = { FontGlyphSegments_u, FttFontCountOf(FontGlyphSegments_u) },
	['v'] = { FontGlyphSegments_v, FttFontCountOf(FontGlyphSegments_v) },
	['w'] = { FontGlyphSegments_w, FttFontCountOf(FontGlyphSegments_w) },
	['x'] = { FontGlyphSegments_x, FttFontCountOf(FontGlyphSegments_x) },
	['y'] = { FontGlyphSegments_y, FttFontCountOf(FontGlyphSegments_y) },
	['z'] = { FontGlyphSegments_z, FttFontCountOf(FontGlyphSegments_z) },

	['0'] = { FontGlyphSegments_0, FttFontCountOf(FontGlyphSegments_0) },
	['1'] = { FontGlyphSegments_1, FttFontCountOf(FontGlyphSegments_1) },
	['2'] = { FontGlyphSegments_2, FttFontCountOf(FontGlyphSegments_2) },
	['3'] = { FontGlyphSegments_3, FttFontCountOf(FontGlyphSegments_3) },
	['4'] = { FontGlyphSegments_4, FttFontCountOf(FontGlyphSegments_4) },
	['5'] = { FontGlyphSegments_5, FttFontCountOf(FontGlyphSegments_5) },
	['6'] = { FontGlyphSegments_6, FttFontCountOf(FontGlyphSegments_6) },
	['7'] = { FontGlyphSegments_7, FttFontCountOf(FontGlyphSegments_7) },
	['8'] = { FontGlyphSegments_8, FttFontCountOf(FontGlyphSegments_8) },
	['9'] = { FontGlyphSegments_9, FttFontCountOf(FontGlyphSegments_9) },

	['.'] = { FontGlyphSegments_Period, FttFontCountOf(FontGlyphSegments_Period) },
	['_'] = { FontGlyphSegments_Underscore, FttFontCountOf(FontGlyphSegments_Underscore) },
	['-'] = { FontGlyphSegments_Minus, FttFontCountOf(FontGlyphSegments_Minus) },
	['/'] = { FontGlyphSegments_Slash, FttFontCountOf(FontGlyphSegments_Slash) },
	['\\'] = { FontGlyphSegments_Backslash, FttFontCountOf(FontGlyphSegments_Backslash) },
	[':'] = { FontGlyphSegments_Colon, FttFontCountOf(FontGlyphSegments_Colon) },
};

static void DrawLineChar(char character, float originX, float originY, float scale, float lineThickness) {
	if (character == ' ') {
		return;
	}
	const fttFontGlyph *glyph = &FontGlyphTable[(unsigned char)character];
	if (glyph->segmentCount == 0) {
		// Unmapped character: draw a placeholder box so a missing glyph is obvious rather than silently blank.
		glLineWidth(lineThickness);
		glBegin(GL_LINE_LOOP);
		glVertex2f(originX + FontColL * scale, originY + FontRowCap * scale);
		glVertex2f(originX + FontColR * scale, originY + FontRowCap * scale);
		glVertex2f(originX + FontColR * scale, originY + FontRowBase * scale);
		glVertex2f(originX + FontColL * scale, originY + FontRowBase * scale);
		glEnd();
		return;
	}
	glLineWidth(lineThickness);
	glBegin(GL_LINES);
	for (uint8_t segmentIndex = 0; segmentIndex < glyph->segmentCount; ++segmentIndex) {
		const fttFontLineSegment *segment = &glyph->segments[segmentIndex];
		glVertex2f(originX + segment->x0 * scale, originY + segment->y0 * scale);
		glVertex2f(originX + segment->x1 * scale, originY + segment->y1 * scale);
	}
	glEnd();
}

// originY is the top (row 0) of the glyph cell in a y-down pixel space, not the text baseline.
static void DrawLineText(const char *text, size_t textLength, float originX, float originY, float scale, float lineThickness) {
	float advancePerChar = (float)(FontGridColumns + FontGlyphSpacingColumns) * scale;
	float cursorX = originX;
	for (size_t index = 0; index < textLength; ++index) {
		DrawLineChar(text[index], cursorX, originY, scale, lineThickness);
		cursorX += advancePerChar;
	}
}

static void DrawLineTextZ(const char *text, float originX, float originY, float scale, float lineThickness) {
	DrawLineText(text, strlen(text), originX, originY, scale, lineThickness);
}

static float MeasureLineTextWidth(const char *text, size_t textLength, float scale) {
	if (textLength == 0) {
		return 0.0f;
	}
	float advancePerChar = (float)(FontGridColumns + FontGlyphSpacingColumns) * scale;
	return (float)textLength * advancePerChar - (float)FontGlyphSpacingColumns * scale;
}

static float MeasureLineTextWidthZ(const char *text, float scale) {
	return MeasureLineTextWidth(text, strlen(text), scale);
}

#endif // FTT_LINEFONT_H
