/*
Name:
	Final UI Adapter

Description:
	Bridges final_ui.h into the Final Framework.

	final_ui.h knows nothing about FPL, final_render.h, final_assets.h or final_math.h - which is what
	makes it usable from any project with a C99 compiler. This file is the OTHER half of that bargain: it
	is the ONE file that knows both worlds, and it carries five bridges.

	  fuiInputFromFinalGame       Input           -> fuiInput      (fplKey table, typed codepoints)
	  fuiFontFromFontAsset        FontAsset       -> fuiFont       (glyphs, kerning, measurement)
	  fuiPlatformFromFPL                          -> fuiPlatform   (FPL's clipboard)
	  fuiAllocatorFromMemoryBlock fmemMemoryBlock -> fuiAllocator  (so the caller still does no malloc)
	  fuiRenderDrawData           fuiDrawData     -> RenderState   (the render backend)

	Plus the three value conversions a caller needs at nearly every draw call, because its own colors and
	points are final_math.h types and the interface has its own:

	  fuiVec2FromVec2f            Vec2f           -> fuiVec2
	  fuiColorFromVec4f           Vec4f           -> fuiColor
	  fuiButtonStateFromFinalGame ButtonState     -> fuiButtonState
	  V2fFromFuiVec2              fuiVec2         -> Vec2f

	Nothing here is required to USE final_ui.h. It is required to use it in a program that is already
	built on the Final Framework, which is what every game in this family is.

	Requires final_ui.h v0.9.0 or later.

	This file is part of the final_framework.

Changelog:
	## 2026-08-23
	- Changed: States the final_ui.h version it is built against (v0.9.0)
	- New: Value conversions fuiVec2FromVec2f, fuiColorFromVec4f, fuiButtonStateFromFinalGame and V2fFromFuiVec2
	- Initial version: the input, font, platform, allocator and render bridges

License:
	MIT License
	Copyright 2017-2026 Torsten Spaete
*/

#ifndef FINAL_UI_ADAPTER_H
#define FINAL_UI_ADAPTER_H

#include <final_platform_layer.h>
#include <final_math.h>
#include <final_memory.h>
#include <final_render.h>
#include <final_assets.h>
#include <final_game.h>
#include <final_ui.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
* @brief Converts a final_math.h point into the interface's own.
* @param[in] value The point in pixels, y-down from the top-left window corner.
* @return Returns the same point as a @ref fuiVec2.
* @note Screen space only. The interface is y-down from the top-left and knows nothing about the world
*       or the camera, so a WORLD point has to be projected before it means anything here.
*/
fpl_extern_inline fuiVec2 fuiVec2FromVec2f(const Vec2f value) {
	fuiVec2 result;
	result.x = value.x;
	result.y = value.y;
	return(result);
}

/**
* @brief Converts the interface's point back into a final_math.h one.
* @param[in] value The point @ref fuiVec2 in pixels, y-down from the top-left window corner.
* @return Returns the same point as a Vec2f.
*/
fpl_extern_inline Vec2f V2fFromFuiVec2(const fuiVec2 value) {
	Vec2f result = V2fInit(value.x, value.y);
	return(result);
}

/**
* @brief Converts a final_math.h color into the interface's own.
* @param[in] value The color, each component in range 0.0 to 1.0.
* @return Returns the same color as a @ref fuiColor.
* @note The two are laid out identically, but they are DIFFERENT TYPES - a Vec4f is also a point and a
*       plane - so the conversion is written out rather than cast. It compiles to nothing.
*/
fpl_extern_inline fuiColor fuiColorFromVec4f(const Vec4f value) {
	fuiColor result;
	result.r = value.r;
	result.g = value.g;
	result.b = value.b;
	result.a = value.a;
	return(result);
}

/**
* @brief Converts one button of the game platform's input into the interface's own.
* @param[in] state The button state as final_game.h reports it.
* @return Returns the same state as a @ref fuiButtonState.
* @note What a caller reaching PAST fuiInput needs - a key the interface has no name for, driving one of
*       its own helpers such as fuiKeyRepeatEdge. The whole snapshot goes through fuiInputFromFinalGame.
*/
fpl_extern_inline fuiButtonState fuiButtonStateFromFinalGame(const ButtonState state) {
	fuiButtonState result;
	result.halfTransitionCount = (int32_t)state.halfTransitionCount;
	result.endedDown = (state.endedDown != 0);
	return(result);
}

/**
* @brief Fills a UI input snapshot from the game platform's own input for this frame.
* @param[in] input Reference to the platform input @ref Input.
* @param[out] outInput Receives the snapshot @ref fuiInput.
* @note The mouse needs NO flip: FPL already reports y-down from the top-left corner, and so does the
*       library. That agreement is the whole reason the library chose y-down.
* @note Only the keys a user interface can act on are carried over. A key final_ui.h has no name for is
*       simply not in the table, which is what keeps the game's own bindings out of the interface.
* @note Typed characters are WIDENED to codepoints. final_gameplatform.h truncates them to a byte on the
*       way in, so anything above U+00FF is already lost by the time this is called - but a byte is read
*       back UNSIGNED here, so at least the Latin-1 range survives rather than turning negative.
*/
fpl_extern void fuiInputFromFinalGame(const Input *input, fuiInput *outInput);

/**
* @brief Fills a UI font from a loaded font asset.
* @param[in] fontAsset Reference to the font asset @ref FontAsset, which has to outlive the font.
* @param[out] outFont Receives the font @ref fuiFont.
* @return Returns false when the asset carries no glyphs, in which case the font is left cleared.
* @note Nothing is copied. The asset - and the atlas texture it names - has to stay alive for as long as
*       the interface uses it.
* @note `measureText` is wired to FontGetTextSize on purpose: the interface then measures a string exactly
*       the way RenderPushText will lay it out, so a caption sits where the layout said it would.
*/
fpl_extern bool fuiFontFromFontAsset(const FontAsset *fontAsset, fuiFont *outFont);

/**
* @brief Fills the platform services with what FPL can provide.
* @param[out] outPlatform Receives the services @ref fuiPlatform.
* @note `setCursor` is left null: FPL has no cursor-SHAPE API, only visibility. A caller that has one of
*       its own can fill the field in afterwards, or read the wanted shape with fuiGetCursor.
*/
fpl_extern void fuiPlatformFromFPL(fuiPlatform *outPlatform);

/**
* @brief Fills a UI allocator that hands out memory from a game memory block.
* @param[in] memoryBlock Reference to the block @ref fmemMemoryBlock to allocate from.
* @param[out] outAllocator Receives the allocator @ref fuiAllocator.
* @note This is how a game that does no malloc keeps that promise. The release callback does NOTHING - a
*       bump allocator cannot give one block back - so the interface's arena grows to its high water mark
*       inside the block and stays there. That is the same deal every other subsystem in this family has
*       with the game arena, and the interface's own buffers stop growing after the first few frames.
*/
fpl_extern void fuiAllocatorFromMemoryBlock(fmemMemoryBlock *memoryBlock, fuiAllocator *outAllocator);

/**
* @brief Draws one finished frame of user interface through the render command stream.
* @param[in,out] renderState Reference to the render state @ref RenderState to push commands into.
* @param[in] drawData The finished frame from fuiGetDrawData.
* @param[in] fontAsset Reference to the font asset @ref FontAsset the interface was given, or null to
*            draw no text at all. Text commands are pushed as RenderPushText, which needs the loaded font.
* @note Installs its own screen projection and leaves the scissor at the full window when it is done.
* @note Every command is taken down its FAST PATH - a rectangle becomes RenderPushRectangle, a string
*       becomes RenderPushText - so the pipeline sees exactly the commands the old in-tree toolkit pushed,
*       rather than a wall of triangles. Only convex polygons are drawn from the tessellated geometry.
*/
fpl_extern void fuiRenderDrawData(RenderState *renderState, const fuiDrawData *drawData, const FontAsset *fontAsset);

#ifdef __cplusplus
}
#endif

#endif // FINAL_UI_ADAPTER_H

// ****************************************************************************
//
// > IMPLEMENTATION
//
// ****************************************************************************
#if (defined(FINAL_UI_ADAPTER_IMPLEMENTATION) && !defined(FINAL_UI_ADAPTER_IMPLEMENTED)) || FPL_IS_IDE

#ifndef FINAL_UI_ADAPTER_IMPLEMENTED
#define FINAL_UI_ADAPTER_IMPLEMENTED
#endif

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------
// > Input
// ----------------------------------------------------------------------------

//! One row of the key table: a platform key and what the interface calls it
typedef struct fui__KeyMapping {
	//! The platform's key
	fplKey platformKey;
	//! What final_ui.h calls it
	fuiKey uiKey;
} fui__KeyMapping;

// Only the keys an interface acts on. A key that is not in here reaches the interface as nothing at all,
// which is deliberate: the game's own bindings are the game's business.
static const fui__KeyMapping fui__globalKeyMappings[] = {
	{ fplKey_Backspace, FUI_KEY_BACKSPACE },
	{ fplKey_Tab, FUI_KEY_TAB },
	{ fplKey_Return, FUI_KEY_RETURN },
	{ fplKey_Escape, FUI_KEY_ESCAPE },
	{ fplKey_Space, FUI_KEY_SPACE },
	{ fplKey_PageUp, FUI_KEY_PAGE_UP },
	{ fplKey_PageDown, FUI_KEY_PAGE_DOWN },
	{ fplKey_End, FUI_KEY_END },
	{ fplKey_Home, FUI_KEY_HOME },
	{ fplKey_Left, FUI_KEY_LEFT },
	{ fplKey_Up, FUI_KEY_UP },
	{ fplKey_Right, FUI_KEY_RIGHT },
	{ fplKey_Down, FUI_KEY_DOWN },
	{ fplKey_Insert, FUI_KEY_INSERT },
	{ fplKey_Delete, FUI_KEY_DELETE },

	{ fplKey_0, FUI_KEY_0 },
	{ fplKey_1, FUI_KEY_1 },
	{ fplKey_2, FUI_KEY_2 },
	{ fplKey_3, FUI_KEY_3 },
	{ fplKey_4, FUI_KEY_4 },
	{ fplKey_5, FUI_KEY_5 },
	{ fplKey_6, FUI_KEY_6 },
	{ fplKey_7, FUI_KEY_7 },
	{ fplKey_8, FUI_KEY_8 },
	{ fplKey_9, FUI_KEY_9 },

	{ fplKey_A, FUI_KEY_A },
	{ fplKey_B, FUI_KEY_B },
	{ fplKey_C, FUI_KEY_C },
	{ fplKey_D, FUI_KEY_D },
	{ fplKey_E, FUI_KEY_E },
	{ fplKey_F, FUI_KEY_F },
	{ fplKey_G, FUI_KEY_G },
	{ fplKey_H, FUI_KEY_H },
	{ fplKey_I, FUI_KEY_I },
	{ fplKey_J, FUI_KEY_J },
	{ fplKey_K, FUI_KEY_K },
	{ fplKey_L, FUI_KEY_L },
	{ fplKey_M, FUI_KEY_M },
	{ fplKey_N, FUI_KEY_N },
	{ fplKey_O, FUI_KEY_O },
	{ fplKey_P, FUI_KEY_P },
	{ fplKey_Q, FUI_KEY_Q },
	{ fplKey_R, FUI_KEY_R },
	{ fplKey_S, FUI_KEY_S },
	{ fplKey_T, FUI_KEY_T },
	{ fplKey_U, FUI_KEY_U },
	{ fplKey_V, FUI_KEY_V },
	{ fplKey_W, FUI_KEY_W },
	{ fplKey_X, FUI_KEY_X },
	{ fplKey_Y, FUI_KEY_Y },
	{ fplKey_Z, FUI_KEY_Z },

	{ fplKey_F1, FUI_KEY_F1 },
	{ fplKey_F2, FUI_KEY_F2 },
	{ fplKey_F3, FUI_KEY_F3 },
	{ fplKey_F4, FUI_KEY_F4 },
	{ fplKey_F5, FUI_KEY_F5 },
	{ fplKey_F6, FUI_KEY_F6 },
	{ fplKey_F7, FUI_KEY_F7 },
	{ fplKey_F8, FUI_KEY_F8 },
	{ fplKey_F9, FUI_KEY_F9 },
	{ fplKey_F10, FUI_KEY_F10 },
	{ fplKey_F11, FUI_KEY_F11 },
	{ fplKey_F12, FUI_KEY_F12 },

	{ fplKey_LeftControl, FUI_KEY_LEFT_CONTROL },
	{ fplKey_RightControl, FUI_KEY_RIGHT_CONTROL },
	{ fplKey_LeftShift, FUI_KEY_LEFT_SHIFT },
	{ fplKey_RightShift, FUI_KEY_RIGHT_SHIFT },
	{ fplKey_LeftAlt, FUI_KEY_LEFT_ALT },
	{ fplKey_RightAlt, FUI_KEY_RIGHT_ALT },
};

// The generic modifier codes, which are what WINDOWS delivers while X11 sends the Left/Right ones. Folded
// onto the LEFT half of each pair, because the library always asks about a modifier as a pair
// (fuiIsControlDown covers both) and a generic press has to answer that question.
static const fui__KeyMapping fui__globalGenericModifierMappings[] = {
	{ fplKey_Control, FUI_KEY_LEFT_CONTROL },
	{ fplKey_Shift, FUI_KEY_LEFT_SHIFT },
	{ fplKey_Alt, FUI_KEY_LEFT_ALT },
};

//! Copies one button, keeping the half transition count the library's press and release edges are built on
fpl_internal fuiButtonState fui__ButtonStateFromFinalGame(const ButtonState state) {
	fuiButtonState result = fplZeroInit;
	result.halfTransitionCount = (int32_t)state.halfTransitionCount;
	result.endedDown = (state.endedDown != 0);
	return(result);
}

fpl_extern void fuiInputFromFinalGame(const Input *input, fuiInput *outInput) {
	fplAssert(outInput != fpl_null);
	if(outInput == fpl_null) {
		return;
	}
	*outInput = fuiZeroInput();
	if(input == fpl_null) {
		return;
	}

	// No flip. FPL reports the cursor y-down from the top-left corner and so does the library.
	outInput->mousePosition = fuiV2((float)input->mouse.pos.x, (float)input->mouse.pos.y);
	outInput->mouseWheelDelta = input->mouse.wheelDelta;
	for(int32_t buttonIndex = 0; buttonIndex < FUI_MOUSE_BUTTON_COUNT; ++buttonIndex) {
		outInput->mouseButtons[buttonIndex] = fui__ButtonStateFromFinalGame(input->mouse.buttons[buttonIndex]);
	}

	const Keyboard *keyboard = &input->tastatur;
	size_t keyMappingCount = fplArrayCount(fui__globalKeyMappings);
	for(size_t mappingIndex = 0; mappingIndex < keyMappingCount; ++mappingIndex) {
		const fui__KeyMapping *mapping = &fui__globalKeyMappings[mappingIndex];
		ButtonState state = keyboard->keys[mapping->platformKey];
		outInput->keys[mapping->uiKey] = fui__ButtonStateFromFinalGame(state);
	}

	// The generic modifier is merged rather than assigned: on a platform that sends BOTH, overwriting would
	// throw away the Left key's own state and with it the edge the library reads.
	size_t genericMappingCount = fplArrayCount(fui__globalGenericModifierMappings);
	for(size_t mappingIndex = 0; mappingIndex < genericMappingCount; ++mappingIndex) {
		const fui__KeyMapping *mapping = &fui__globalGenericModifierMappings[mappingIndex];
		ButtonState state = keyboard->keys[mapping->platformKey];
		fuiButtonState generic = fui__ButtonStateFromFinalGame(state);
		fuiButtonState *target = &outInput->keys[mapping->uiKey];
		target->endedDown = target->endedDown || generic.endedDown;
		target->halfTransitionCount += generic.halfTransitionCount;
	}

	// Widened to codepoints. The byte is read UNSIGNED, so a Latin-1 character does not arrive negative.
	int32_t typedCount = input->textInputLength;
	if(typedCount > (int32_t)FUI_MAX_TEXT_INPUT) {
		typedCount = (int32_t)FUI_MAX_TEXT_INPUT;
	}
	for(int32_t typedIndex = 0; typedIndex < typedCount; ++typedIndex) {
		outInput->textInput[typedIndex] = (uint32_t)(unsigned char)input->textInput[typedIndex];
	}
	outInput->textInputLength = typedCount;

	outInput->windowSize = fuiV2i(input->windowSize.x, input->windowSize.y);
	outInput->deltaTime = input->dynamicFrameTime;
	outInput->isActive = input->isActive;
}

// ----------------------------------------------------------------------------
// > Font
// ----------------------------------------------------------------------------

//! Answers the glyph for a codepoint the loaded font actually carries
fpl_internal bool fui__FontAssetGetGlyph(void *userData, uint32_t codePoint, fuiGlyph *outGlyph) {
	const LoadedFont *font = (const LoadedFont *)userData;
	if(font == fpl_null || font->charCount == 0 || outGlyph == fpl_null) {
		return(false);
	}
	uint32_t lastCharPastOne = font->firstChar + font->charCount;
	if(codePoint < font->firstChar || codePoint >= lastCharPastOne) {
		// Asked BEFORE FontGetGlyph rather than after: that one asserts the range rather than answering it.
		return(false);
	}

	uint32_t glyphIndex = codePoint - font->firstChar;
	const FontGlyph *glyph = &font->glyphs[glyphIndex];

	// The loaded font is y-UP: its offset names the quad's top-left corner as a height ABOVE the baseline.
	// The library is y-down and wants that same corner measured DOWNWARD from the baseline, which is the
	// same number with the other sign.
	outGlyph->offset = fuiV2(glyph->offset.x, -glyph->offset.y);
	outGlyph->size = fuiV2(glyph->charSize.x, glyph->charSize.y);

	// And its atlas is y-up too: the TOP of a glyph samples the larger v. Swapping the two here is what
	// makes the library's own tessellation come out the right way up on a backend that draws triangles.
	outGlyph->uvMin = fuiV2(glyph->uvMin.x, glyph->uvMax.y);
	outGlyph->uvMax = fuiV2(glyph->uvMax.x, glyph->uvMin.y);

	const uint32_t noFollowingCodePoint = 0;
	float advance = FontGetCharacterAdvance(font, codePoint, noFollowingCodePoint);
	outGlyph->advance = advance;
	return(true);
}

//! The extra advance between two codepoints, which is the kerning entry on its own
fpl_internal float fui__FontAssetGetKerning(void *userData, uint32_t leftCodePoint, uint32_t rightCodePoint) {
	const LoadedFont *font = (const LoadedFont *)userData;
	if(font == fpl_null || !font->hasKerningTable) {
		return(0.0f);
	}
	const uint32_t noFollowingCodePoint = 0;
	float advanceWithKerning = FontGetCharacterAdvance(font, leftCodePoint, rightCodePoint);
	float advanceOnItsOwn = FontGetCharacterAdvance(font, leftCodePoint, noFollowingCodePoint);
	float result = advanceWithKerning - advanceOnItsOwn;
	return(result);
}

//! Measures a string the way the renderer will lay it out, which is what keeps a caption where the layout put it
fpl_internal float fui__FontAssetMeasureText(void *userData, const char *text, size_t textLength, float pixelHeight) {
	const LoadedFont *font = (const LoadedFont *)userData;
	if(font == fpl_null || text == fpl_null || textLength == 0) {
		return(0.0f);
	}
	Vec2f size = FontGetTextSize(font, text, textLength, pixelHeight);
	return(size.x);
}

fpl_extern bool fuiFontFromFontAsset(const FontAsset *fontAsset, fuiFont *outFont) {
	fplAssert(outFont != fpl_null);
	if(outFont == fpl_null) {
		return(false);
	}
	fplClearStruct(outFont);
	if(fontAsset == fpl_null || fontAsset->desc.charCount == 0 || fontAsset->desc.glyphs == fpl_null) {
		return(false);
	}

	const LoadedFont *font = &fontAsset->desc;
	outFont->getGlyph = fui__FontAssetGetGlyph;
	outFont->getKerning = fui__FontAssetGetKerning;
	outFont->measureText = fui__FontAssetMeasureText;
	// Both are stored as a positive height in font units where the nominal size is 1.0, which is exactly
	// the convention the library asks for - so nothing is scaled here.
	outFont->metrics.ascent = font->info.ascent;
	outFont->metrics.descent = font->info.descent;
	outFont->metrics.lineHeight = font->info.lineHeight;
	outFont->metrics.spaceAdvance = font->info.spaceAdvance;
	outFont->atlasTexture = (fuiTextureId)(uintptr_t)fontAsset->texture;
	outFont->atlasSize = fuiV2((float)font->atlasWidth, (float)font->atlasHeight);
	// The LOADED FONT, not the asset: every callback above only ever reads the description.
	outFont->userData = (void *)font;
	return(true);
}

// ----------------------------------------------------------------------------
// > Platform
// ----------------------------------------------------------------------------

fpl_internal bool fui__PlatformGetClipboardText(void *userData, char *dest, uint32_t maxDestLength) {
	(void)userData;
	if(dest == fpl_null || maxDestLength == 0) {
		return(false);
	}
	bool result = fplGetClipboardText(dest, maxDestLength);
	return(result);
}

fpl_internal bool fui__PlatformSetClipboardText(void *userData, const char *text) {
	(void)userData;
	if(text == fpl_null) {
		return(false);
	}
	bool result = fplSetClipboardText(text);
	return(result);
}

fpl_extern void fuiPlatformFromFPL(fuiPlatform *outPlatform) {
	fplAssert(outPlatform != fpl_null);
	if(outPlatform == fpl_null) {
		return;
	}
	fplClearStruct(outPlatform);
	outPlatform->getClipboardText = fui__PlatformGetClipboardText;
	outPlatform->setClipboardText = fui__PlatformSetClipboardText;
	// setCursor stays null: FPL can hide the cursor but cannot change its SHAPE.
}

// ----------------------------------------------------------------------------
// > Allocator
// ----------------------------------------------------------------------------

fpl_internal void *fui__MemoryBlockAllocate(void *userData, size_t size) {
	fmemMemoryBlock *memoryBlock = (fmemMemoryBlock *)userData;
	if(memoryBlock == fpl_null || size == 0) {
		return(fpl_null);
	}
	uint8_t *result = fmemPush(memoryBlock, size, fmemPushFlags_Clear);
	return((void *)result);
}

fpl_internal void fui__MemoryBlockRelease(void *userData, void *pointer) {
	// Nothing to do. A bump allocator gives its memory back all at once or not at all, and the interface
	// only ever releases at shutdown - by which point the whole block is going anyway.
	(void)userData;
	(void)pointer;
}

fpl_extern void fuiAllocatorFromMemoryBlock(fmemMemoryBlock *memoryBlock, fuiAllocator *outAllocator) {
	fplAssert(outAllocator != fpl_null);
	if(outAllocator == fpl_null) {
		return;
	}
	fplClearStruct(outAllocator);
	outAllocator->allocate = fui__MemoryBlockAllocate;
	outAllocator->release = fui__MemoryBlockRelease;
	outAllocator->userData = memoryBlock;
}

// ----------------------------------------------------------------------------
// > Render backend
//
// The library draws y-DOWN from the top-left corner and the render pipeline is y-UP from the bottom-left,
// so something has to turn one into the other. This backend flips every COMMAND rather than installing a
// flipped projection, and that is a deliberate choice: RenderPushText builds its glyph quads y-up, so does
// RenderPushSprite, and RenderPushScissor is glScissor and always wants y-up window pixels. A flipped
// matrix would have to be undone in all three places, which is more work than flipping a rectangle.
// ----------------------------------------------------------------------------

//! Horizontal alignment value that starts the text AT the position given, rather than centering it there
#define FUI__RENDER_TEXT_LEFT_ALIGN 1.0f
//! Vertical alignment value that centers the text's own box on the position given
#define FUI__RENDER_TEXT_CENTER_ALIGN 0.0f

fpl_internal Vec4f fui__ToRenderColor(const fuiColor color) {
	Vec4f result = V4fInit(color.r, color.g, color.b, color.a);
	return(result);
}

//! One point, from the interface's y-down space into the pipeline's y-up space
fpl_internal Vec2f fui__ToRenderPoint(const fuiVec2 point, const float windowHeight) {
	Vec2f result = V2fInit(point.x, windowHeight - point.y);
	return(result);
}

//! The bottom-left corner of a rectangle, which is the corner the pipeline names one by
fpl_internal Vec2f fui__ToRenderBottomLeft(const fuiRect rect, const float windowHeight) {
	float bottomInUiSpace = rect.y + rect.h;
	Vec2f result = V2fInit(rect.x, windowHeight - bottomInUiSpace);
	return(result);
}

/*
	The sprite flags an image command comes out as.

	Worth the derivation, because it is not obvious and it is exactly the kind of thing that silently draws
	a preview mirrored. The pipeline maps the TOP of an unflipped sprite to the LARGER v; the library maps
	the top of an unflipped image to uvMin. So the plain case already needs SpriteFlags_FlipV, which is why
	every call site in the old toolkit passed it by hand for a top-down sheet. A library FLIP_V then turns
	that back off, and a library FLIP_U passes straight through.

	The two quarter turns need no swapping at all: the pipeline turns the quad the same way round the
	library does once both are read in their own vertical direction. Each of the four corner assignments was
	checked against _ExpandSpriteInstance rather than assumed.
*/
fpl_internal SpriteFlags fui__ToSpriteFlags(const fuiImageFlags flags) {
	SpriteFlags result = SpriteFlags_None;
	bool mirrorsAcross = (flags & FUI_IMAGE_FLIP_U) == FUI_IMAGE_FLIP_U;
	bool mirrorsDown = (flags & FUI_IMAGE_FLIP_V) == FUI_IMAGE_FLIP_V;
	if(mirrorsAcross) {
		result |= SpriteFlags_FlipU;
	}
	if(!mirrorsDown) {
		result |= SpriteFlags_FlipV;
	}
	if((flags & FUI_IMAGE_ROTATE_90_CW) == FUI_IMAGE_ROTATE_90_CW) {
		result |= SpriteFlags_Rotate_90_CW;
	}
	if((flags & FUI_IMAGE_ROTATE_90_CCW) == FUI_IMAGE_ROTATE_90_CCW) {
		result |= SpriteFlags_Rotate_90_CCW;
	}
	return(result);
}

//! Installs a clip rectangle as the pipeline's scissor, in the integer y-up window pixels glScissor wants
fpl_internal void fui__PushRenderScissor(RenderState *renderState, const fuiRect clipRect, const float windowHeight) {
	float bottomInUiSpace = clipRect.y + clipRect.h;
	int scissorX = (int)clipRect.x;
	int scissorY = (int)(windowHeight - bottomInUiSpace);
	int scissorWidth = (int)clipRect.w;
	int scissorHeight = (int)clipRect.h;
	RenderPushScissor(renderState, scissorX, scissorY, scissorWidth, scissorHeight);
}

//! Draws one command's tessellated geometry, one triangle at a time. The fallback for a convex polygon,
//! which is the only primitive the library hands over with no fast path of its own
fpl_internal void fui__RenderCommandTriangles(RenderState *renderState, const fuiDrawData *drawData, const fuiDrawCommand *command, const float windowHeight) {
	for(uint32_t indexAt = 0; indexAt + 2u < command->indexCount; indexAt += 3u) {
		fuiDrawIndex firstIndex = drawData->indices[command->indexOffset + indexAt + 0u];
		fuiDrawIndex secondIndex = drawData->indices[command->indexOffset + indexAt + 1u];
		fuiDrawIndex thirdIndex = drawData->indices[command->indexOffset + indexAt + 2u];
		const fuiVertex *firstVertex = &drawData->vertices[firstIndex];
		const fuiVertex *secondVertex = &drawData->vertices[secondIndex];
		const fuiVertex *thirdVertex = &drawData->vertices[thirdIndex];

		Vec2f corners[3];
		corners[0] = fui__ToRenderPoint(firstVertex->position, windowHeight);
		corners[1] = fui__ToRenderPoint(secondVertex->position, windowHeight);
		corners[2] = fui__ToRenderPoint(thirdVertex->position, windowHeight);

		// Every vertex of one command carries the same colour, so the first one speaks for the triangle.
		fuiColor unpackedColor = fuiUnpackColor(firstVertex->color);
		Vec4f color = fui__ToRenderColor(unpackedColor);
		const bool copyVertices = true;
		const bool isLoop = true;
		const float noLineWidth = 0.0f;
		RenderPushVertices(renderState, corners, 3, copyVertices, color, DrawMode_Polygon, isLoop, noLineWidth);
	}
}

fpl_extern void fuiRenderDrawData(RenderState *renderState, const fuiDrawData *drawData, const FontAsset *fontAsset) {
	fplAssert(renderState != fpl_null);
	if(renderState == fpl_null || drawData == fpl_null || drawData->commandCount == 0) {
		return;
	}

	float windowWidth = (float)drawData->windowSize.x;
	float windowHeight = (float)drawData->windowSize.y;
	if(windowWidth <= 0.0f || windowHeight <= 0.0f) {
		return;
	}

	// Screen ortho in pixels, y-up from the bottom-left corner - the same projection every other screen
	// space overlay in this family installs.
	Mat4f screenProjection = M4fOrthoRH(0.0f, windowWidth, 0.0f, windowHeight, 0.0f, 1.0f);
	RenderSetMatrix(renderState, &screenProjection);

	fuiRect fullWindow = fuiRectMake(0.0f, 0.0f, windowWidth, windowHeight);
	fui__PushRenderScissor(renderState, fullWindow, windowHeight);
	fuiRect activeClip = fullWindow;

	for(uint32_t commandIndex = 0; commandIndex < drawData->commandCount; ++commandIndex) {
		const fuiDrawCommand *command = &drawData->commands[commandIndex];

		// The scissor is pushed only when the clip actually moves. Every command carries its own resolved
		// clip, so this is the whole of the clip handling - there is no stack to keep on this side.
		bool clipMoved = (command->clipRect.x != activeClip.x) || (command->clipRect.y != activeClip.y) || (command->clipRect.w != activeClip.w) || (command->clipRect.h != activeClip.h);
		if(clipMoved) {
			fui__PushRenderScissor(renderState, command->clipRect, windowHeight);
			activeClip = command->clipRect;
		}

		switch(command->kind) {
			case FUI_DRAW_RECT:
			{
				const fuiDrawRectPayload *payload = &command->payload.rect;
				Vec2f bottomLeft = fui__ToRenderBottomLeft(payload->rect, windowHeight);
				Vec2f size = V2fInit(payload->rect.w, payload->rect.h);
				Vec4f color = fui__ToRenderColor(payload->color);
				const bool isFilled = true;
				const float noLineWidth = 0.0f;
				RenderPushRectangle(renderState, bottomLeft, size, color, isFilled, noLineWidth);
			} break;

			case FUI_DRAW_RECT_OUTLINE:
			{
				const fuiDrawRectOutlinePayload *payload = &command->payload.rectOutline;
				Vec2f bottomLeft = fui__ToRenderBottomLeft(payload->rect, windowHeight);
				Vec2f size = V2fInit(payload->rect.w, payload->rect.h);
				Vec4f color = fui__ToRenderColor(payload->color);
				const bool isFilled = false;
				RenderPushRectangle(renderState, bottomLeft, size, color, isFilled, payload->thickness);
			} break;

			case FUI_DRAW_LINE:
			{
				const fuiDrawLinePayload *payload = &command->payload.line;
				Vec2f start = fui__ToRenderPoint(payload->start, windowHeight);
				Vec2f end = fui__ToRenderPoint(payload->end, windowHeight);
				Vec4f color = fui__ToRenderColor(payload->color);
				RenderPushLine(renderState, start, end, color, payload->thickness);
			} break;

			case FUI_DRAW_TEXT:
			{
				const fuiDrawTextPayload *payload = &command->payload.text;
				if(fontAsset == fpl_null || fontAsset->texture == fpl_null) {
					break;
				}
				const LoadedFont *font = &fontAsset->desc;
				const char *text = &drawData->textBuffer[payload->textOffset];

				/*
					The library hands over the TOP-LEFT corner of the text's box, and the pipeline centers a
					string vertically on the position it is given. So the box's centre is what goes across,
					and the two never have to agree about how tall a line is: whatever height the library
					measured, half of it below the top edge is the same point either way.

					That is also what makes this pixel-identical to the toolkit this replaces, which passed
					the widget's centre for exactly the same reason.
				*/
				float boxHeight = (font->info.ascent + font->info.descent) * payload->pixelHeight;
				fuiVec2 boxCentre = fuiV2(payload->position.x, payload->position.y + boxHeight * 0.5f);
				Vec2f position = fui__ToRenderPoint(boxCentre, windowHeight);
				Vec4f color = fui__ToRenderColor(payload->color);
				RenderPushText(renderState, text, (size_t)payload->textLength, font, fontAsset->texture, position, payload->pixelHeight, FUI__RENDER_TEXT_LEFT_ALIGN, FUI__RENDER_TEXT_CENTER_ALIGN, color);
			} break;

			case FUI_DRAW_IMAGE:
			{
				const fuiDrawImagePayload *payload = &command->payload.image;
				TextureHandle texture = (TextureHandle)(uintptr_t)command->texture;
				if(texture == fpl_null) {
					break;
				}
				float centreX = payload->rect.x + payload->rect.w * 0.5f;
				float centreY = payload->rect.y + payload->rect.h * 0.5f;
				fuiVec2 centreInUiSpace = fuiV2(centreX, centreY);
				Vec2f centre = fui__ToRenderPoint(centreInUiSpace, windowHeight);

				// A quarter turn makes the pipeline swap the two extents for itself, and the library's
				// rectangle is ALREADY the turned shape - so it is handed the unswapped half extent and the
				// two swaps cancel.
				bool turnsClockwise = (payload->flags & FUI_IMAGE_ROTATE_90_CW) == FUI_IMAGE_ROTATE_90_CW;
				bool turnsCounterClockwise = (payload->flags & FUI_IMAGE_ROTATE_90_CCW) == FUI_IMAGE_ROTATE_90_CCW;
				bool isTurned = turnsClockwise != turnsCounterClockwise;
				float halfWidth = payload->rect.w * 0.5f;
				float halfHeight = payload->rect.h * 0.5f;
				Vec2f halfExtent = isTurned ? V2fInit(halfHeight, halfWidth) : V2fInit(halfWidth, halfHeight);

				Vec4f tint = fui__ToRenderColor(payload->color);
				UVRect uvRect = UVRectInit(payload->uvMin.x, payload->uvMin.y, payload->uvMax.x, payload->uvMax.y);
				SpriteFlags spriteFlags = fui__ToSpriteFlags(payload->flags);
				RenderPushSprite(renderState, centre, halfExtent, texture, tint, uvRect, spriteFlags);
			} break;

			case FUI_DRAW_TRIANGLES:
			default:
			{
				fui__RenderCommandTriangles(renderState, drawData, command, windowHeight);
			} break;
		}
	}

	// Left at the full window: a scissor outlives the command that set it, and whatever draws next has no
	// reason to inherit the last panel's box.
	fui__PushRenderScissor(renderState, fullWindow, windowHeight);
}

#ifdef __cplusplus
}
#endif

#endif // FINAL_UI_ADAPTER_IMPLEMENTATION
