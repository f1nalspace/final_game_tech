/***
final_ui.h

-------------------------------------------------------------------------------
	About
-------------------------------------------------------------------------------

An open source single header file immediate mode user interface C99 library.

It draws panels, menus, dialogs and widgets, and hands the result to the caller as a
batched command list plus a vertex/index buffer, so any renderer can display it:
legacy OpenGL, modern OpenGL, Vulkan, a software framebuffer, or anything else.

It has no dependencies other than the C standard library. It does not open a window,
does not read input, does not load a font and does not talk to a graphics API. The
caller supplies an input snapshot and a font that produces glyphs; the library gives
back geometry to draw.

Coordinates are in PIXELS, y-down from the top-left corner of the window - the same
convention as the mouse, a top-down framebuffer, Vulkan and D3D. There is no matrix
and no projection: what the library emits is where it goes on screen.

Status: feature complete and in use. Everything the library set out to do is in -
the draw data and its tessellator, text, input, the interaction core, the layout
engine, the widgets, text input, the color picker, tooltips, commands and their
shortcuts, menus, tool strips, the status bar, modal dialogs, the list box, the list
view, tabs and images - and it carries the entire interface of a game, its level
editor and its tools.

What it is not yet is v1.0.0, and the reason is rendering QUALITY rather than
features: the tessellator emits hard edges and nothing in it is anti-aliased, so a
diagonal, a slanted polygon and a thin line all show their steps. Polishing what the
library draws is its own pass and it is what the next version number is being kept
for. Nothing about the API is expected to move for it.

-------------------------------------------------------------------------------
	Getting started
-------------------------------------------------------------------------------

- Define FUI_IMPLEMENTATION in exactly ONE translation unit before including this file.
- Describe your font once by filling a fuiFont, see "Custom font" below.
- Create a fuiContext with fuiInit(), pass fui_null as allocator to use malloc/free.
- Once per frame: fill a fuiInput, call fuiBeginFrame(), build your widgets, call fuiEndFrame().
- Draw what fuiGetDrawData() returns, see "Rendering" below.
- Release everything with fuiRelease().

-------------------------------------------------------------------------------
	Usage
-------------------------------------------------------------------------------

#define FUI_IMPLEMENTATION
#include <final_ui.h>

fuiContext ui;
fuiInit(&ui, &myFont, fui_null);           // fui_null = default malloc/free

while (running) {
	fuiInput input = fuiZeroInput();
	MyFillInput(&input);                   // mouse, keys, typed codepoints, window size

	fuiBeginFrame(&ui, &input, FUI_PASS_BOTH);
	fuiBeginPanel(&ui, "settings", FUI_DOCK_NONE, 20, 20, 260, 180);
	fuiLabel(&ui, fuiLayoutSlot(&ui, 24), "Settings");
	fuiCheckbox(&ui, fuiLayoutSlot(&ui, 24), "Fullscreen", &fullscreen);
	if (fuiButton(&ui, fuiLayoutSlot(&ui, 28), "Quit")) running = false;
	fuiEndPanel(&ui);
	fuiEndFrame(&ui);

	const fuiDrawData *drawData = fuiGetDrawData(&ui);
	MyRenderDrawData(drawData);
}

fuiRelease(&ui);

-------------------------------------------------------------------------------
	Custom allocator
-------------------------------------------------------------------------------

fuiAllocator allocator;
allocator.allocate = MyAlloc;   // void *(*)(void *userData, size_t size)
allocator.release  = MyFree;    // void  (*)(void *userData, void *ptr)
allocator.userData = myContext;
fuiInit(&ui, &myFont, &allocator);

-------------------------------------------------------------------------------
	Custom font
-------------------------------------------------------------------------------

The library never loads a font. It asks the caller for glyphs out of an atlas the
caller has already uploaded. Glyph geometry and metrics are in FONT UNITS, where the
nominal font size is 1.0, so one fuiFont serves every text size.

fuiFont font;
font.getGlyph      = MyGetGlyph;      // required
font.getKerning    = MyGetKerning;    // optional, fui_null when the font has no kerning
font.measureText   = fui_null;        // optional fast path, else glyph advances are summed
font.metrics       = myMetrics;       // ascent, descent, lineHeight, spaceAdvance
font.atlasTexture  = myAtlasTextureId;
font.atlasSize     = fuiV2(512.0f, 512.0f);
font.userData      = myFontLibrary;

-------------------------------------------------------------------------------
	Rendering
-------------------------------------------------------------------------------

fuiGetDrawData() returns one vertex buffer, one index buffer and a list of commands.
Every command carries the geometry to draw it as triangles AND the primitive kind it
came from, so a backend picks whichever is cheaper for it:

- A modern OpenGL, Vulkan or D3D backend ignores `kind` and draws indexCount indices
  starting at indexOffset, with clipRect as the scissor and texture as the binding.
- A legacy OpenGL, software or high level backend switches on `kind` and takes the
  fast path -- FUI_DRAW_RECT is a rectangle fill, FUI_DRAW_TEXT is a text call.

Both come out of the same build, so nothing is computed twice.

-------------------------------------------------------------------------------
	Preprocessor overrides
-------------------------------------------------------------------------------

FUI_IMPLEMENTATION        Define in ONE translation unit to emit the implementation.
FUI_API_AS_PRIVATE        Set to 1 to make the public api static (single file only).
FUI_ASSERT(expr)          Override the assertion macro (defaults to assert from <assert.h>).
FUI_MALLOC(size)          Override the default allocate (defaults to malloc; skips <stdlib.h> when set with FUI_FREE).
FUI_FREE(ptr)             Override the default release (defaults to free; skips <stdlib.h> when set with FUI_MALLOC).
FUI_STRLEN(text)          Override string length (defaults to strlen; skips <string.h> when set with the memory overrides).
FUI_MEMCHR(p,v,size)      Override memory byte search (defaults to memchr; skips <string.h> when set with the others).
FUI_MEMSET(dst,val,size)  Override memory set (defaults to memset; skips <string.h> when set with FUI_MEMCPY and FUI_MEMMOVE).
FUI_MEMCPY(dst,src,size)  Override memory copy (defaults to memcpy; skips <string.h> when set with FUI_MEMSET and FUI_MEMMOVE).
FUI_MEMMOVE(dst,src,size) Override memory move (defaults to memmove; skips <string.h> when set with FUI_MEMSET and FUI_MEMCPY).
FUI_VSNPRINTF(d,n,f,a)    Override formatted printing (defaults to vsnprintf; skips <stdio.h> when set).
FUI_FABSF(v)              Override float absolute (defaults to fabsf; skips <math.h> when set with the other float overrides).
FUI_FMODF(a,b)            Override float modulo (defaults to fmodf; skips <math.h> when set with the other float overrides).
FUI_SQRTF(v)              Override float square root (defaults to sqrtf; skips <math.h> when set with the other float overrides).
FUI_TEXTURE_ID_TYPE       Type of a texture identifier (defaults to uint64_t).
FUI_ARENA_MIN_BLOCK_SIZE  Minimum arena block size in bytes (default 65536 = 64 KB).
FUI_OOM_SINK_SIZE         Size of the out-of-memory scratch slot in bytes (default 4096).
FUI_USE_16BIT_INDICES     Set to 1 to emit 16-bit draw indices for old hardware (default 0, meaning 32-bit).
FUI_MAX_CLIP_DEPTH        Maximum nesting depth of the clip stack (default 32).
FUI_MAX_TEXT_LINES        Maximum number of visual lines one text block breaks into (default 256).
FUI_MAX_ID_STACK          Maximum nesting depth of the identifier stack (default 32).
FUI_MAX_LAYOUT_DEPTH      Maximum number of containers open at once (default 32).
FUI_MAX_WIDGET_STATES     Maximum number of widgets remembering something across frames (default 256).
FUI_MAX_SORTABLE_ROWS     Maximum number of rows one list view will sort (default 1000000).
FUI_LIST_SORT_VERIFY_ROWS Up to this many rows, a sorted list notices edited cells by itself (default 4096).
FUI_MAX_TEXT_INPUT        Maximum number of codepoints typed in one frame (default 32).
FUI_MAX_TOOLTIP_TEXT      Maximum number of bytes one hover tooltip may say (default 256).
FUI_MAX_CLIPBOARD_TEXT    Maximum number of bytes one clipboard transfer may carry (default 1024).
FUI_MAX_DIALOGS           Maximum number of modal dialogs open on top of each other (default 8).
FUI_MAX_MENU_DEPTH        Maximum number of menus open inside each other (default 8).
FUI_MAX_SHORTCUT_TEXT     Maximum number of bytes one shortcut spells out to (default 32).
FUI_MAX_LIST_COLUMNS      Maximum number of columns one list view remembers a width and a sort for (default 12).
FUI_MAX_SORTABLE_ROWS     Maximum number of rows a list view will sort, a longer list keeps the caller's order (default 4096).
FUI_KEY_REPEAT_DELAY      Seconds a key must be held before it starts repeating (default 0.45).
FUI_KEY_REPEAT_INTERVAL   Seconds between repeats once it started (default 0.05).

-------------------------------------------------------------------------------
	License
-------------------------------------------------------------------------------

Final UI is released under the following license:

MIT License

Copyright (c) 2017-2026 Torsten Spaete

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
***/

/*!
	@file final_ui.h
	@version v0.9.3
	@author Torsten Spaete
	@brief Final UI (FUI) - A pure C99 single file header immediate mode user interface library.
*/

// ****************************************************************************
//
// > Changelog
//
// ****************************************************************************

/*!
	@page page_changelog Changelog
	@tableofcontents

	# v0.9.3:
	This one is about SIZE. Everything below came out of a workbench built to answer one question - what does
	a list of a million rows, a text box holding a whole file, or a menu of tens of thousands of items cost -
	and it turned out the honest answer was "far more than any of it needed to". See demos/FUI_Performance.

	- Fixed: A sorted list view sorted itself again on EVERY frame, which is n log n natural-order string
	  comparisons to arrive at the answer it already had. It is now worked out once and kept, keyed on the
	  address of the cell array, its shape and the sort. Measured on a hundred thousand rows: 41 ms a frame
	  before, 0.05 ms after.
	- New: fuiListViewInvalidateSort, which a list longer than FUI_LIST_SORT_VERIFY_ROWS needs from a caller
	  that edits cells IN PLACE - the same array, the same number of rows, different text. A shorter list
	  hashes its sorted column every frame and notices that by itself, so nothing existing has to change.
	- New: FUI_LIST_SORT_VERIFY_ROWS, the length up to which that hash is taken (default 4096).
	- Changed: FUI_MAX_SORTABLE_ROWS is 1000000 rather than 4096, and it is now a ceiling on what one sorted
	  list may make the context allocate rather than a buffer reserved up front. A list sorts an array the
	  size of ITS rows, the first time it is sorted.
	- Fixed: A list box and a list view walked EVERY row to draw the forty that fit. Both now work out which
	  rows the box can show and walk only those, so the cost of a list no longer follows its length at all.
	  A million row list view went from 1.47 ms a frame to 0.05 ms.
	- Fixed: Every label was measured TWICE and then walked twice more - once by the caller for a height the
	  font metrics already hold, once inside fuiDrawText for a clip test that needs two numbers, once to count
	  codepoints for a reservation a byte count bounds, and once to build the glyphs. Only the last of those
	  is left.
	- Fixed: fuiDrawText tessellated glyphs the clip could never show. It now skips the ones off the left edge
	  and stops at the right one, so a long path in a narrow column costs the dozen glyphs that fit rather
	  than the five hundred it holds.
	- Fixed: fui__StringLength walked a string a byte at a time. It goes through FUI_STRLEN now, which is the
	  C library's, and is on the path of every label and of a text field's whole document once a frame. A text
	  box holding two hundred thousand lines went from 5.07 ms a frame to 0.19 ms.
	- New: FUI_STRLEN, to override that the way FUI_MEMCPY and the others are overridden.
	- Fixed: A menu popup taller than the window was placed at the top and drawn off the bottom, where its
	  rows could be neither seen nor reached. A popup is now no taller than the space it opens into, scrolls
	  under the wheel, and marks the edge it was cut at.
	- Fixed: That marked edge was a picture and nothing else. Resting the cursor on it now scrolls the menu,
	  which is what an arrow on the edge of a menu has always meant, and the row underneath it no longer
	  takes the hover - pointing at the arrow used to light up, and open, whatever was behind it.
	- Fixed: A menu bar's popup covered the bar it dropped from, and every other title on it, as soon as it
	  was taller than the window. It opens into the space BELOW its trigger instead when that is most of the
	  screen, so the title stays visible and the rows past the bottom are scrolled to.
	- Fixed: A submenu with no room to its parent's right was slid back over the rows it belongs to. It
	  opens to the LEFT of its parent instead, which is what leaves both of them readable.
	- Fixed: A multiline text field showed the first FUI_MAX_TEXT_LINES lines of its buffer and had no way
	  at all to reach the rest - no scrollbar, no wheel, nothing. It lays out a WINDOW of the document now,
	  with a scrollbar and the wheel, and follows the caret when a key moves it while leaving the view alone
	  when the wheel moved it. FUI_MAX_TEXT_LINES is the size of that window, not a ceiling on the document.
	- New: fuiBreakTextLinesFrom, which breaks a window of a text's lines out of it and reports how many it
	  has in total. This is what the field above is built on.
	- Changed: An unwrapped text is split on its newlines with a memory scan rather than a decoded walk, and
	  a field remembers where its window began so the next one seeks from there. Together those are what
	  make a large document scrollable at all: two hundred thousand lines scrolled to the end went from
	  1.18 ms a frame to 0.10 ms, and 44 ms to 0.11 ms with word wrap on.
	- New: FUI_MEMCHR, to override that scan the way FUI_MEMCPY and the others are overridden.
	- Changed: A multiline field keeps a gutter for its scrollbar, so it wraps inside what is left of its
	  width rather than underneath the bar.
	- Fixed: The retained widget state table held 256 widgets and, once full, turned every lookup into a scan
	  of all of them that answered nothing - so a panel silently lost its scroll position and a list its column
	  widths. It grows and rehashes instead, at the frame boundary, where nothing is holding a pointer into it.
	- New: fuiSetDrawBatching, merging consecutive commands that share a clip and a texture into one. OFF by
	  default, because a merged command carries no fast path payload: a backend that redraws a rectangle or a
	  string through its own pipeline, rather than drawing the triangles, must not turn it on.
	- Changed: fuiContext.listSortOrder is gone, since the order now lives with the list that owns it, and
	  fuiContext.listSortScratch grows to fit rather than being reserved at FUI_MAX_SORTABLE_ROWS.

	# v0.9.2:
	- Changed: A button is drawn as a BUTTON - a vertically shaded face between a lit top-left edge and a shaded
	  bottom-right one, inside the frame every box already carried. Holding it turns the shading around so the
	  face reads as pushed into the panel, and its caption moves with it.
	- Changed: The same bevel is what a tab header, a tool strip button, a checkbox box and a radio marker are
	  drawn from, so one look covers everything that can be clicked. A lit tool strip toggle stays pushed in for
	  as long as it is on, and a checkbox box is a WELL its mark sits in rather than a face that could be pushed.
	- Changed: A box stands in one of THREE reliefs rather than two. Sunken is the shape of a widget that has
	  been pushed, so a widget nobody is pushing no longer borrows it to mean something else: the tab whose page
	  is showing is raised, the ones behind it are FLAT, and only a tab actually being held sinks.
	- Changed: Every tab caption takes fuiTheme.textColor. The muted color is what a DISABLED widget takes, and
	  a tab that is merely not showing is not disabled - wearing that color made the captions on the rest of the
	  strip the hardest thing on it to read. Which page is showing is carried by the relief, the fill and the
	  accent line under it, which is three cues without spending the caption on a fourth.
	- Changed: A DISABLED button lies flush in the panel instead of standing out of it, so what cannot be
	  clicked says so by its shape and not only by its muted caption.
	- New: fuiDrawRectVerticalGradient, a rectangle filled from a top color to a bottom one. Two identical stops
	  still go out as a plain FUI_DRAW_RECT, so a flat theme keeps the payload a backend can shortcut.
	- New: fuiColorShade, moving a color toward white or toward black and leaving its alpha alone.
	- New: fuiTheme.widgetBevelLightColor, fuiTheme.widgetBevelShadowColor, fuiTheme.widgetBevelThickness,
	  fuiTheme.widgetFaceShadingStrength and fuiTheme.widgetPressOffset. Zeroing the thickness and the shading
	  strength gives back exactly the flat widgets of v0.9.1.

	# v0.9.1:
	- Changed: ONE frame width for every box the interface draws. fuiTheme.panelBorderThickness is the thin
	  stroke the widgets already used, so a panel, a dialog, a menu popup and a group box are outlined alike.
	- Fixed: A panel, a dialog and a group box were framed everywhere but along their title bar, whose fill was
	  drawn over the outline. The frame now goes on AFTER the title bar and runs unbroken around the whole box.
	- Fixed: A menu popup lost its right hand stroke. Its box is measured from text and landed on fractions of
	  a pixel, and a backend that truncates its scissor - glScissor takes integers - cut the one edge every
	  such rounding cuts. Menu bar titles and popups are snapped onto the pixel grid, so the frame survives
	  whatever the backend rounds to. The frame is also counted into what a row asks for in width.
	- Fixed: A menu row's hover wash painted over the popup's own frame, which is now drawn after the rows.
	- Fixed: A scroll panel's content ran flush into its scrollbar. The gutter keeps a fuiTheme.panelPaddingX
	  gap to its left, the same measure the panel keeps to its left and right edges.
	- Changed: A flow container's spacing is a TRAILING margin - under a slot in a vertical flow, right of one
	  in a horizontal flow - rather than a gap inserted in front of every slot but the first.
	- Changed: fuiTheme.widgetSpacing defaults to the panel's side padding, so the gaps between widgets and the
	  gaps around them are one measure.
	- New: FUI_SPACING_FROM_THEME, passed as a stack's spacing to take fuiTheme.widgetSpacing.
	- New: The bars measure themselves - fuiMenuBarHeight, fuiToolStripThickness and fuiStatusBarHeight. A bar
	  is one row of what it holds plus the inset above and below it, so none of the three is a number a caller
	  has to pick, and a restyled theme moves all of them at once.
	- New: fuiLayoutDock, which bites a strip off one edge of the container that is open and hands it back. A
	  bar takes its WIDTH from whatever encloses it and its thickness from the theme, so a caller names no
	  pixels at all, and what is left over is what fuiLayoutRemaining answers with.
	- Changed: A tool strip insets its items across the flow axis instead of letting them fill the bar, so a
	  button never runs into the edge of the strip around it. A strip separator spans its slot end to end, the
	  way fuiSeparator does, and so stands exactly as tall as the buttons it divides.
	- Changed: EVERY dialog is dragged by its title bar now, not just a resizable one, and the bar lights up
	  under the cursor to say so. The position is bounded so the caption always stays on screen, and a dialog
	  nobody has dragged keeps its zero offset and stays centred as the window is resized under it.
	- Changed: fuiOpenDialog places the dialog, so one comes up CENTRED however far the last of its kind was
	  dragged - a desktop dialog is a new window every time it is shown. The size a resizable dialog was left
	  at still survives, being a preference rather than a placement.

	# v0.9.0:
	- New: The api is documented in full - a doxygen block on all 200 public declarations and a note on every type, field, enum value and constant
	- New: The file header carries About, Getting started, Usage, Custom allocator, Custom font, Rendering, Preprocessor overrides and License
	- Changed: Feature complete - everything the library set out to do is in, and it drives the whole interface of a shipping game
	- Note: Nothing the library DRAWS is anti-aliased yet. That pass is what v1.0.0 is being kept for, and it is not expected to move the api.

	# v0.8.1:
	- Fixed: A dock taken at the TOP level took its strip out of the whole window every time, so a menu bar,
	  a status bar and a left panel docked side by side all started in the same corner. The frame now opens a
	  ROOT container over the window and they bite out of one shrinking region.

	# v0.8.0:
	- New: List view - fuiListView, fuiListViewEx and fuiListViewButtons, with column headers over a scrolling body
	- New: Columns RESIZE by dragging the divider beside a header, and the width is remembered per list
	- New: Columns SORT by clicking the title, case insensitively and with a run of digits compared as a number
	- New: Driving a sort from code - fuiListViewSetSort, fuiListViewSetSortDefault and fuiListViewSetSortable
	- New: A column of per-row buttons, so a row can be acted on without being picked first
	- New: fuiTabControl, a row of tab headers that remembers which one is showing
	- New: fuiImage and fuiImageDesc, with nine placements, four scale modes and mirroring
	- New: fuiImageFlags and fuiDrawImageEx, the library's OWN mirror and quarter turn flags
	- New: fuiWidgetState remembers a sideways scroll, a chosen tab, the column widths and the sort
	- Changed: fuiDrawImagePayload carries the flags the quad's corners were sampled with

	# v0.7.0:
	- New: Modal dialogs - fuiOpenDialog, fuiCloseDialog, fuiBeginModal, fuiBeginModalResizable and the backdrop
	- New: Standard dialogs - message box, input box, multi-line input box, colour dialog, file dialog and file browser
	- New: fuiDialogTakeKey, so one press is answered by at most one dialog however they are stacked
	- New: fuiListBox and fuiListBoxEx, with optional row icons through fuiListIcons
	- New: fuiIsAnyDialogOpen and fuiCloseAllDialogs, the pair fuiIsMenuOpen and fuiCloseMenus already had
	- Fixed: An open menu bar behind a dialog's backdrop was still clickable

	# v0.6.0:
	- New: Commands - fuiCommand, fuiCommandTable, fuiShortcut, fuiFindCommand, fuiShortcutToText and fuiDispatchShortcuts
	- New: Menus - menu bars, nested submenus, context menus, checkable rows and rows built from a command
	- New: Tool strips along either axis, with toggles and separators, and a status bar with left and right items
	- New: fuiIsMenuOpen and fuiCloseMenus, so a screen switching away can put its menus down
	- New: fui_font_stbtt.h, a companion header baking a TrueType face with stb_truetype into a fuiFont
	- Fixed: A menu row answered the very same press that opened its popup when the bar sat at the bottom edge

	# v0.5.0:
	- New: Text input - fuiTextInput and fuiTextInputEx, with a caret, a selection, click to position, drag to select and a blinking caret
	- New: Text editing is codepoint aware throughout, so a caret never lands inside a multi-byte character
	- New: Multi-line text fields, optionally word wrapped, with per row home and end and up and down between rows
	- New: Platform services - fuiPlatform, fuiSetPlatform, fuiGetClipboardText, fuiSetClipboardText, fuiCursor, fuiSetCursor and fuiGetCursor
	- New: Clipboard editing - control with C, X, V and A, all of it through fuiPlatform
	- New: Keyboard focus walks the fields on tab, and a click on the background gives the keyboard back
	- New: fuiTooltip, a deferred hover tooltip with a rest delay, multiple lines and screen edge flipping
	- New: Color picker - fuiColorPicker, fuiHsvToRgb and fuiRgbToHsv
	- New: fuiEncodeUtf8, the counterpart of fuiDecodeUtf8
	- New: fuiKeyRepeat, held key auto repeat over a timer the context owns
	- New: fuiTheme.textSelectionColor
	- Fixed: fuiKeyRepeatEdge dropped a key that was pressed AND released inside one frame

	# v0.4.0:
	- New: Layout - fuiDock and fuiAxis, panels, stacks, fuiLayoutSlot and fuiLayoutRemaining, all of it y-down from the top-left
	- New: Panels - plain, scrolling and closable, each movable by its title bar, resizable by its grip and foldable by its arrow
	- New: Retained widget state - fuiWidgetState, so a panel keeps where it was dragged and a list how far it was scrolled
	- New: Groups - fuiBeginGroup, fuiBeginGroupBox and the self measuring fuiBeginGroupBoxAuto
	- New: Scrolling - fuiScrollbarVertical, scroll panels and scroll stacks with a reserved gutter
	- New: Splitters, drag handles and the panel chrome glyphs
	- New: Widgets - label, separator, button, buttonEx, buttonRepeat, cell, checkbox, radio, sliderFloat, sliderFloatEx and dragFloat
	- Fixed: fuiInteract reported a click when the cursor left the widget and the button was released in the same motion

	# v0.3.0:
	- New: Input - fuiInput, fuiButtonState with its press and release edges, the fuiKey vocabulary and typed codepoints
	- New: Frame passes - fuiBeginFrame / fuiEndFrame taking FUI_PASS_BOTH, FUI_PASS_INTERACT or FUI_PASS_DRAW
	- New: Identifiers - a 64-bit FNV-1a id stack with fuiPushId / fuiPushIdInt / fuiPopId
	- New: Interaction - hot, active and focused tracking through fuiInteract, plus fuiWantsMouse / fuiWantsKeyboard / fuiBlockMouse
	- New: fuiKeyRepeatEdge for held-key auto repeat

	# v0.2.0:
	- New: Draw data - fuiVertex, fuiDrawCommand and fuiDrawData, filled once per frame between fuiBeginDrawFrame and fuiEndDrawFrame
	- New: Tessellation for rectangles, rectangle outlines, lines, convex polygons, textured quads and text
	- New: Clip stack with intersect-on-push, resolved into every command instead of emitting scissor commands
	- New: Text - UTF-8 decoding, glyph shaping, fuiMeasureText, fuiTruncateTextToWidth and the multi-line breaker
	- Changed: Draw indices default to 32-bit and are absolute, set FUI_USE_16BIT_INDICES for old hardware

	# v0.1.0:
	- New: Foundation layer only - memory, math, theme and the context lifecycle
	- New: fuiAllocator with a malloc/free default and a growable block arena behind it
	- New: Math types fuiVec2, fuiVec3, fuiVec2i, fuiColor and fuiRect, with brace initializers for static tables
	- New: fuiFont, the glyph callback interface a caller fills in to supply text
	- New: fuiTheme plus fuiDefaultTheme(), every color and size the widgets draw with
	- New: fuiContext with fuiInit / fuiRelease / fuiSetFont / fuiSetTheme
*/

#ifndef FUI_INCLUDE_H
#define FUI_INCLUDE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

//! Is any IDE active, such as IntelliSense or any JetBrains IDE?
#if defined(__INTELLISENSE__) || defined(__JETBRAINS_IDE__)
#	define FUI_IS_IDE 1
#else
#	define FUI_IS_IDE 0
#endif

#if !defined(FUI_API_AS_PRIVATE)
	//! Set to 1 to make the public api static, for a single translation unit
#	define FUI_API_AS_PRIVATE 0
#endif

#if FUI_API_AS_PRIVATE
	//! Public api declaration
#	define fui_api static
#else
	//! Public api declaration
#	define fui_api extern
#endif // FUI_API_AS_PRIVATE

//! Inline helper declaration
#define fui_inline static inline

//! Null pointer
#define fui_null NULL

#if !defined(FUI_ASSERT)
#	include <assert.h>
	//! Assertion macro - define FUI_ASSERT before including to override the default (assert)
#	define FUI_ASSERT(expression) assert(expression)
#endif

#if !defined(FUI_ARENA_MIN_BLOCK_SIZE)
	//! Default minimum arena block size (64 KB)
#	define FUI_ARENA_MIN_BLOCK_SIZE 65536
#endif

#if !defined(FUI_OOM_SINK_SIZE)
	//! Size of the scratch slot handed out when the arena cannot allocate (4 KB)
#	define FUI_OOM_SINK_SIZE 4096
#endif

#if !defined(FUI_TEXTURE_ID_TYPE)
	//! Underlying type of a texture identifier - wide enough for a GL name, a pointer or a 64-bit Vulkan handle
#	define FUI_TEXTURE_ID_TYPE uint64_t
#endif

#if !defined(FUI_USE_16BIT_INDICES)
	//! Set to 1 to emit 16-bit draw indices, for hardware that cannot take 32-bit ones
#	define FUI_USE_16BIT_INDICES 0
#endif

#if !defined(FUI_MAX_CLIP_DEPTH)
	//! Maximum nesting depth of the clip stack
#	define FUI_MAX_CLIP_DEPTH 32
#endif

#if !defined(FUI_MAX_TEXT_LINES)
	//! Maximum number of visual lines one text block breaks into
#	define FUI_MAX_TEXT_LINES 256
#endif

#if !defined(FUI_MAX_ID_STACK)
	//! Maximum nesting depth of the identifier stack
#	define FUI_MAX_ID_STACK 32
#endif

#if !defined(FUI_MAX_LAYOUT_DEPTH)
	//! Maximum number of containers open at the same time, such as panels inside panels
#	define FUI_MAX_LAYOUT_DEPTH 32
#endif

#if !defined(FUI_MAX_WIDGET_STATES)
	//! Maximum number of widgets that remember something across frames, such as a panel's position
#	define FUI_MAX_WIDGET_STATES 256
#endif

#if !defined(FUI_MAX_TEXT_INPUT)
	//! Maximum number of codepoints typed in one frame
#	define FUI_MAX_TEXT_INPUT 32
#endif

#if !defined(FUI_MAX_TOOLTIP_TEXT)
	//! Maximum number of bytes one hover tooltip may say
#	define FUI_MAX_TOOLTIP_TEXT 256
#endif

#if !defined(FUI_MAX_CLIPBOARD_TEXT)
	//! Maximum number of bytes one clipboard transfer may carry
#	define FUI_MAX_CLIPBOARD_TEXT 1024
#endif

#if !defined(FUI_MAX_MENU_DEPTH)
	//! Maximum number of menus open inside each other, counting the one a menu bar opened as the first
#	define FUI_MAX_MENU_DEPTH 8
#endif

#if !defined(FUI_MAX_SHORTCUT_TEXT)
	//! Maximum number of bytes one keyboard shortcut spells out to, such as "Ctrl+Shift+PageDown"
#	define FUI_MAX_SHORTCUT_TEXT 32
#endif

#if !defined(FUI_MAX_DIALOGS)
	//! Maximum number of modal dialogs open on top of each other, such as a confirmation over a file dialog
#	define FUI_MAX_DIALOGS 8
#endif

#if !defined(FUI_MAX_LIST_COLUMNS)
	//! Maximum number of columns one list view remembers a dragged width and a sort for
#	define FUI_MAX_LIST_COLUMNS 12
#endif

#if !defined(FUI_LIST_SORT_VERIFY_ROWS)
	//! Up to this many rows, a list view HASHES its sorted column every frame and re-sorts by itself when
	//! the text changed. Past it that hash costs more than it is worth and a caller that edits cells in
	//! place has to say so with @ref fuiListViewInvalidateSort
#	define FUI_LIST_SORT_VERIFY_ROWS 4096
#endif

#if !defined(FUI_MAX_SORTABLE_ROWS)
	//! Maximum number of rows a list view will sort. A longer list stays in the order the caller gave it.
	//! This is a ceiling on what ONE sorted list may make the context allocate - two int32 arrays of this
	//! many entries in the worst case - and not a working set that is reserved up front. A list sorts an
	//! array the size of ITS rows, the first time it is sorted, and never gives it back
#	define FUI_MAX_SORTABLE_ROWS 1000000
#endif

#if !defined(FUI_KEY_REPEAT_DELAY)
	//! Seconds a key must be held before it starts repeating
#	define FUI_KEY_REPEAT_DELAY 0.45f
#endif

#if !defined(FUI_KEY_REPEAT_INTERVAL)
	//! Seconds between repeats once a held key started repeating
#	define FUI_KEY_REPEAT_INTERVAL 0.05f
#endif

#ifdef __cplusplus
extern "C" {
#endif

// ****************************************************************************
//
// > Math types
//
// ****************************************************************************

/**
* @brief Builds a fuiVec2 in a static initializer.
* @param[in] x The x coordinate.
* @param[in] y The y coordinate.
* @note A compound literal is not a constant expression, so a static table must use this brace form instead of @ref fuiV2.
*/
#define FUI_V2(x, y) { (x), (y) }

/**
* @brief Builds a fuiVec3 in a static initializer.
* @param[in] x The x coordinate.
* @param[in] y The y coordinate.
* @param[in] z The z coordinate.
*/
#define FUI_V3(x, y, z) { (x), (y), (z) }

/**
* @brief Builds a fuiVec2i in a static initializer.
* @param[in] x The x coordinate.
* @param[in] y The y coordinate.
*/
#define FUI_V2I(x, y) { (x), (y) }

/**
* @brief Builds a fuiColor in a static initializer.
* @param[in] r The red component in range 0.0 to 1.0.
* @param[in] g The green component in range 0.0 to 1.0.
* @param[in] b The blue component in range 0.0 to 1.0.
* @param[in] a The alpha component in range 0.0 to 1.0.
*/
#define FUI_COLOR(r, g, b, a) { (r), (g), (b), (a) }

/**
* @brief Builds a fuiRect in a static initializer.
* @param[in] x The left edge in pixels.
* @param[in] y The top edge in pixels.
* @param[in] w The width in pixels.
* @param[in] h The height in pixels.
*/
#define FUI_RECT(x, y, w, h) { (x), (y), (w), (h) }

//! A 2D single precision vector
typedef struct fuiVec2 {
	//! The x coordinate
	float x;
	//! The y coordinate
	float y;
} fuiVec2;

//! A 3D single precision vector
typedef struct fuiVec3 {
	//! The x coordinate
	float x;
	//! The y coordinate
	float y;
	//! The z coordinate
	float z;
} fuiVec3;

//! A 2D signed 32-bit integer vector
typedef struct fuiVec2i {
	//! The x coordinate
	int32_t x;
	//! The y coordinate
	int32_t y;
} fuiVec2i;

//! A linear RGBA color, each component in range 0.0 to 1.0
typedef struct fuiColor {
	//! The red component
	float r;
	//! The green component
	float g;
	//! The blue component
	float b;
	//! The alpha component, 0.0 is fully transparent
	float a;
} fuiColor;

//! An axis aligned rectangle in pixels, with the origin in the top-left window corner and y growing downwards
typedef struct fuiRect {
	//! The left edge
	float x;
	//! The top edge
	float y;
	//! The width, never negative
	float w;
	//! The height, never negative
	float h;
} fuiRect;

/**
* @brief Returns the smaller of two floats.
* @param[in] a The first value.
* @param[in] b The second value.
* @return Returns a when it is smaller than b, b otherwise.
*/
fui_inline float fuiMinF(const float a, const float b) {
	float result = (a < b) ? a : b;
	return(result);
}

/**
* @brief Returns the larger of two floats.
* @param[in] a The first value.
* @param[in] b The second value.
* @return Returns a when it is larger than b, b otherwise.
*/
fui_inline float fuiMaxF(const float a, const float b) {
	float result = (a > b) ? a : b;
	return(result);
}

/**
* @brief Clamps a float into an inclusive range.
* @param[in] value The value to clamp.
* @param[in] minValue The lower bound.
* @param[in] maxValue The upper bound.
* @return Returns the value moved into the range.
*/
fui_inline float fuiClampF(const float value, const float minValue, const float maxValue) {
	float lowerClamped = (value < minValue) ? minValue : value;
	float result = (lowerClamped > maxValue) ? maxValue : lowerClamped;
	return(result);
}

/**
* @brief Linearly interpolates between two floats.
* @param[in] a The value returned at t = 0.
* @param[in] b The value returned at t = 1.
* @param[in] t The interpolation factor, not clamped.
* @return Returns the interpolated value.
*/
fui_inline float fuiLerpF(const float a, const float b, const float t) {
	float result = a + (b - a) * t;
	return(result);
}

/**
* @brief Returns the smaller of two 32-bit integers.
* @param[in] a The first value.
* @param[in] b The second value.
* @return Returns a when it is smaller than b, b otherwise.
*/
fui_inline int32_t fuiMinI(const int32_t a, const int32_t b) {
	int32_t result = (a < b) ? a : b;
	return(result);
}

/**
* @brief Returns the larger of two 32-bit integers.
* @param[in] a The first value.
* @param[in] b The second value.
* @return Returns a when it is larger than b, b otherwise.
*/
fui_inline int32_t fuiMaxI(const int32_t a, const int32_t b) {
	int32_t result = (a > b) ? a : b;
	return(result);
}

/**
* @brief Clamps a 32-bit integer into an inclusive range.
* @param[in] value The value to clamp.
* @param[in] minValue The lower bound.
* @param[in] maxValue The upper bound.
* @return Returns the value moved into the range.
*/
fui_inline int32_t fuiClampI(const int32_t value, const int32_t minValue, const int32_t maxValue) {
	int32_t lowerClamped = (value < minValue) ? minValue : value;
	int32_t result = (lowerClamped > maxValue) ? maxValue : lowerClamped;
	return(result);
}

/**
* @brief Constructs a 2D vector.
* @param[in] x The x coordinate.
* @param[in] y The y coordinate.
* @return Returns the constructed vector.
*/
fui_inline fuiVec2 fuiV2(const float x, const float y) {
	fuiVec2 result;
	result.x = x;
	result.y = y;
	return(result);
}

/**
* @brief Constructs a 3D vector.
* @param[in] x The x coordinate.
* @param[in] y The y coordinate.
* @param[in] z The z coordinate.
* @return Returns the constructed vector.
*/
fui_inline fuiVec3 fuiV3(const float x, const float y, const float z) {
	fuiVec3 result;
	result.x = x;
	result.y = y;
	result.z = z;
	return(result);
}

/**
* @brief Constructs a 2D integer vector.
* @param[in] x The x coordinate.
* @param[in] y The y coordinate.
* @return Returns the constructed vector.
*/
fui_inline fuiVec2i fuiV2i(const int32_t x, const int32_t y) {
	fuiVec2i result;
	result.x = x;
	result.y = y;
	return(result);
}

/**
* @brief Adds two 2D vectors.
* @param[in] a The first vector.
* @param[in] b The second vector.
* @return Returns the sum of both vectors.
*/
fui_inline fuiVec2 fuiV2Add(const fuiVec2 a, const fuiVec2 b) {
	fuiVec2 result = fuiV2(a.x + b.x, a.y + b.y);
	return(result);
}

/**
* @brief Subtracts one 2D vector from another.
* @param[in] a The vector to subtract from.
* @param[in] b The vector to subtract.
* @return Returns the difference of both vectors.
*/
fui_inline fuiVec2 fuiV2Subtract(const fuiVec2 a, const fuiVec2 b) {
	fuiVec2 result = fuiV2(a.x - b.x, a.y - b.y);
	return(result);
}

/**
* @brief Scales a 2D vector by a scalar.
* @param[in] a The vector to scale.
* @param[in] scale The scalar factor.
* @return Returns the scaled vector.
*/
fui_inline fuiVec2 fuiV2Scale(const fuiVec2 a, const float scale) {
	fuiVec2 result = fuiV2(a.x * scale, a.y * scale);
	return(result);
}

/**
* @brief Constructs an opaque color from its red, green and blue components.
* @param[in] r The red component in range 0.0 to 1.0.
* @param[in] g The green component in range 0.0 to 1.0.
* @param[in] b The blue component in range 0.0 to 1.0.
* @return Returns the constructed color with an alpha of 1.0.
*/
fui_inline fuiColor fuiColorRGB(const float r, const float g, const float b) {
	fuiColor result;
	result.r = r;
	result.g = g;
	result.b = b;
	result.a = 1.0f;
	return(result);
}

/**
* @brief Constructs a color from its red, green, blue and alpha components.
* @param[in] r The red component in range 0.0 to 1.0.
* @param[in] g The green component in range 0.0 to 1.0.
* @param[in] b The blue component in range 0.0 to 1.0.
* @param[in] a The alpha component in range 0.0 to 1.0.
* @return Returns the constructed color.
*/
fui_inline fuiColor fuiColorRGBA(const float r, const float g, const float b, const float a) {
	fuiColor result;
	result.r = r;
	result.g = g;
	result.b = b;
	result.a = a;
	return(result);
}

/**
* @brief Returns a copy of a color with a replaced alpha component.
* @param[in] color The source color.
* @param[in] alpha The alpha component in range 0.0 to 1.0.
* @return Returns the color with the given alpha.
*/
fui_inline fuiColor fuiColorWithAlpha(const fuiColor color, const float alpha) {
	fuiColor result = fuiColorRGBA(color.r, color.g, color.b, alpha);
	return(result);
}

/**
* @brief Shades a color toward white or toward black, leaving its alpha component alone.
* @param[in] color The source color.
* @param[in] amount How far to move, from -1 for fully black through 0 for unchanged to 1 for fully white.
* @return Returns the shaded color.
* @note This is what gives a widget its lit top and its shaded bottom without a second color per state in the theme.
*/
fui_inline fuiColor fuiColorShade(const fuiColor color, const float amount) {
	float shadeTarget = (amount >= 0.0f) ? 1.0f : 0.0f;
	float shadeWeight = (amount >= 0.0f) ? amount : -amount;
	float r = fuiLerpF(color.r, shadeTarget, shadeWeight);
	float g = fuiLerpF(color.g, shadeTarget, shadeWeight);
	float b = fuiLerpF(color.b, shadeTarget, shadeWeight);
	fuiColor result = fuiColorRGBA(r, g, b, color.a);
	return(result);
}

/**
* @brief Linearly interpolates between two colors, component by component.
* @param[in] a The color returned at t = 0.
* @param[in] b The color returned at t = 1.
* @param[in] t The interpolation factor, not clamped.
* @return Returns the interpolated color.
*/
fui_inline fuiColor fuiColorLerp(const fuiColor a, const fuiColor b, const float t) {
	float r = fuiLerpF(a.r, b.r, t);
	float g = fuiLerpF(a.g, b.g, t);
	float bl = fuiLerpF(a.b, b.b, t);
	float al = fuiLerpF(a.a, b.a, t);
	fuiColor result = fuiColorRGBA(r, g, bl, al);
	return(result);
}

/**
* @brief Packs a color into 32 bits, one unsigned byte per component.
* @param[in] color The color to pack, components outside 0.0 to 1.0 are clamped.
* @return Returns the packed color in RGBA byte order, which reads as 0xAABBGGRR on a little endian machine.
* @note This is the byte order GL_RGBA with GL_UNSIGNED_BYTE expects, and the one a 32-bit software framebuffer stores.
*/
fui_inline uint32_t fuiPackColor(const fuiColor color) {
	float clampedRed = fuiClampF(color.r, 0.0f, 1.0f);
	float clampedGreen = fuiClampF(color.g, 0.0f, 1.0f);
	float clampedBlue = fuiClampF(color.b, 0.0f, 1.0f);
	float clampedAlpha = fuiClampF(color.a, 0.0f, 1.0f);
	uint32_t red = (uint32_t)(clampedRed * 255.0f + 0.5f);
	uint32_t green = (uint32_t)(clampedGreen * 255.0f + 0.5f);
	uint32_t blue = (uint32_t)(clampedBlue * 255.0f + 0.5f);
	uint32_t alpha = (uint32_t)(clampedAlpha * 255.0f + 0.5f);
	uint32_t result = red | (green << 8) | (blue << 16) | (alpha << 24);
	return(result);
}

/**
* @brief Unpacks a 32 bit color back into floating point components.
* @param[in] packed The packed color in RGBA byte order, see @ref fuiPackColor.
* @return Returns the unpacked color.
*/
fui_inline fuiColor fuiUnpackColor(const uint32_t packed) {
	float inverse255 = 1.0f / 255.0f;
	float red = (float)((packed) & 0xFFu) * inverse255;
	float green = (float)((packed >> 8) & 0xFFu) * inverse255;
	float blue = (float)((packed >> 16) & 0xFFu) * inverse255;
	float alpha = (float)((packed >> 24) & 0xFFu) * inverse255;
	fuiColor result = fuiColorRGBA(red, green, blue, alpha);
	return(result);
}

/**
* @brief Constructs a rectangle from a position and a size.
* @param[in] x The left edge in pixels.
* @param[in] y The top edge in pixels.
* @param[in] w The width in pixels.
* @param[in] h The height in pixels.
* @return Returns the constructed rectangle.
*/
fui_inline fuiRect fuiRectMake(const float x, const float y, const float w, const float h) {
	fuiRect result;
	result.x = x;
	result.y = y;
	result.w = w;
	result.h = h;
	return(result);
}

/**
* @brief Constructs a rectangle from its top-left and bottom-right corner.
* @param[in] minimum The top-left corner in pixels.
* @param[in] maximum The bottom-right corner in pixels.
* @return Returns the constructed rectangle, empty when the corners are inverted.
*/
fui_inline fuiRect fuiRectMinMax(const fuiVec2 minimum, const fuiVec2 maximum) {
	float width = fuiMaxF(0.0f, maximum.x - minimum.x);
	float height = fuiMaxF(0.0f, maximum.y - minimum.y);
	fuiRect result = fuiRectMake(minimum.x, minimum.y, width, height);
	return(result);
}

/**
* @brief Returns the x coordinate of a rectangle's right edge.
* @param[in] rect The rectangle.
* @return Returns the right edge in pixels.
*/
fui_inline float fuiRectRight(const fuiRect rect) {
	float result = rect.x + rect.w;
	return(result);
}

/**
* @brief Returns the y coordinate of a rectangle's bottom edge.
* @param[in] rect The rectangle.
* @return Returns the bottom edge in pixels.
*/
fui_inline float fuiRectBottom(const fuiRect rect) {
	float result = rect.y + rect.h;
	return(result);
}

/**
* @brief Returns the center point of a rectangle.
* @param[in] rect The rectangle.
* @return Returns the center in pixels.
*/
fui_inline fuiVec2 fuiRectCenter(const fuiRect rect) {
	fuiVec2 result = fuiV2(rect.x + rect.w * 0.5f, rect.y + rect.h * 0.5f);
	return(result);
}

/**
* @brief Tests whether a rectangle covers no area.
* @param[in] rect The rectangle.
* @return Returns true when the width or the height is zero or negative.
*/
//! Whether two rectangles are the same one. Bit exact on purpose: both sides come out of the same clip
//! arithmetic, so a tolerance here would merge two commands that really are clipped differently
fui_inline bool fui__RectEquals(const fuiRect left, const fuiRect right) {
	bool result = (left.x == right.x) && (left.y == right.y) && (left.w == right.w) && (left.h == right.h);
	return(result);
}

fui_inline bool fuiRectIsEmpty(const fuiRect rect) {
	bool result = (rect.w <= 0.0f) || (rect.h <= 0.0f);
	return(result);
}

/**
* @brief Tests whether a point lies inside a rectangle.
* @param[in] point The point in pixels.
* @param[in] rect The rectangle to test against.
* @return Returns true when the point is inside, where the left and top edges are inclusive and the right and bottom edges are not.
*/
fui_inline bool fuiPointInRect(const fuiVec2 point, const fuiRect rect) {
	bool isInsideHorizontally = (point.x >= rect.x) && (point.x < fuiRectRight(rect));
	bool isInsideVertically = (point.y >= rect.y) && (point.y < fuiRectBottom(rect));
	bool result = isInsideHorizontally && isInsideVertically;
	return(result);
}

/**
* @brief Intersects two rectangles.
* @param[in] a The first rectangle.
* @param[in] b The second rectangle.
* @return Returns the overlapping area, or an empty rectangle when they do not overlap.
* @note Nesting clip rectangles can therefore only ever shrink the visible area.
*/
fui_inline fuiRect fuiRectIntersect(const fuiRect a, const fuiRect b) {
	float left = fuiMaxF(a.x, b.x);
	float top = fuiMaxF(a.y, b.y);
	float right = fuiMinF(fuiRectRight(a), fuiRectRight(b));
	float bottom = fuiMinF(fuiRectBottom(a), fuiRectBottom(b));
	float width = fuiMaxF(0.0f, right - left);
	float height = fuiMaxF(0.0f, bottom - top);
	fuiRect result = fuiRectMake(left, top, width, height);
	return(result);
}

/**
* @brief Grows or shrinks a rectangle on every side.
* @param[in] rect The rectangle to change.
* @param[in] amount How far every edge moves outwards, negative values shrink.
* @return Returns the changed rectangle, never with a negative size.
*/
fui_inline fuiRect fuiRectInflate(const fuiRect rect, const float amount) {
	float width = fuiMaxF(0.0f, rect.w + amount * 2.0f);
	float height = fuiMaxF(0.0f, rect.h + amount * 2.0f);
	fuiRect result = fuiRectMake(rect.x - amount, rect.y - amount, width, height);
	return(result);
}

/**
* @brief Moves a rectangle by an offset.
* @param[in] rect The rectangle to move.
* @param[in] offset How far to move it in pixels.
* @return Returns the moved rectangle.
*/
fui_inline fuiRect fuiRectOffset(const fuiRect rect, const fuiVec2 offset) {
	fuiRect result = fuiRectMake(rect.x + offset.x, rect.y + offset.y, rect.w, rect.h);
	return(result);
}

// ****************************************************************************
//
// > Memory
//
// ****************************************************************************

//! Pluggable memory allocator. Hands out large blocks; the arena sub-allocates from them.
typedef struct fuiAllocator {
	//! Allocates a block of at least the given size
	void *(*allocate)(void *userData, size_t size);
	//! Releases a block previously returned by allocate
	void (*release)(void *userData, void *ptr);
	//! User supplied context passed to allocate/release
	void *userData;
} fuiAllocator;

//! One block of memory owned by an arena. Blocks form a singly linked list.
typedef struct fuiMemoryBlock {
	//! The next block in the arena, or null for the last one
	struct fuiMemoryBlock *next;
	//! Start of the usable memory in this block
	uint8_t *base;
	//! Total usable size in bytes
	size_t size;
	//! How many bytes of this block are handed out
	size_t used;
} fuiMemoryBlock;

//! A growable bump allocator. Never frees an individual allocation, only the whole arena.
typedef struct fuiArena {
	//! Resolved allocator (the default is used when the caller passed none)
	fuiAllocator allocator;
	//! First block in the list, kept so the whole chain can be released
	fuiMemoryBlock *firstBlock;
	//! Block new allocations are taken from
	fuiMemoryBlock *currentBlock;
	//! Smallest block the arena ever asks the allocator for
	size_t minBlockSize;
	//! Sum of every block size, for reporting
	size_t totalAllocatedSize;
	//! Scratch slot handed out instead of null when a small allocation fails
	uint8_t outOfMemorySink[FUI_OOM_SINK_SIZE];
	//! Set once the allocator refused a block, and never cleared until the arena is released
	bool outOfMemory;
} fuiArena;

/**
* @brief Returns the allocator that forwards to malloc and free.
* @return Returns the default allocator.
* @note This is what @ref fuiInit uses when the caller passes null for its allocator.
*/
fui_api fuiAllocator fuiDefaultAllocator(void);

// ****************************************************************************
//
// > Font
//
// ****************************************************************************

//! Identifies a texture for the backend. The library only stores and compares it, it never dereferences it.
typedef FUI_TEXTURE_ID_TYPE fuiTextureId;

//! Value of a texture identifier that means "no texture"
#define FUI_TEXTURE_ID_NONE ((fuiTextureId)0)

//! One glyph, in font units where the nominal font size is 1.0
typedef struct fuiGlyph {
	//! Offset from the pen position to the glyph quad's top-left corner
	fuiVec2 offset;
	//! Size of the glyph quad
	fuiVec2 size;
	//! Top-left texture coordinate inside the atlas
	fuiVec2 uvMin;
	//! Bottom-right texture coordinate inside the atlas
	fuiVec2 uvMax;
	//! Horizontal pen advance to the next glyph
	float advance;
} fuiGlyph;

//! Vertical font metrics, in font units where the nominal font size is 1.0
typedef struct fuiFontMetrics {
	//! Distance from the baseline up to the highest glyph
	float ascent;
	//! Distance from the baseline down to the lowest glyph
	float descent;
	//! Distance from one baseline to the next
	float lineHeight;
	//! Advance of a space character
	float spaceAdvance;
} fuiFontMetrics;

//! Everything the library needs to measure and draw text. The caller owns the font, the library only reads it.
typedef struct fuiFont {
	//! Fills outGlyph for the given codepoint - return false when the font has no such glyph. Required.
	bool (*getGlyph)(void *userData, uint32_t codePoint, fuiGlyph *outGlyph);
	//! Extra advance between two codepoints, in font units. Optional, null means no kerning.
	float (*getKerning)(void *userData, uint32_t leftCodePoint, uint32_t rightCodePoint);
	//! Measures a string in pixels. Optional fast path, null means glyph advances are summed instead.
	float (*measureText)(void *userData, const char *text, size_t textLength, float pixelHeight);
	//! Vertical metrics of this font
	fuiFontMetrics metrics;
	//! The already uploaded glyph atlas
	fuiTextureId atlasTexture;
	//! Atlas dimensions in pixels
	fuiVec2 atlasSize;
	//! User supplied context passed to every callback above
	void *userData;
} fuiFont;

/**
* @brief Returns a zeroed font, safe to fill in field by field.
* @return Returns a font with every callback and value cleared.
*/
fui_api fuiFont fuiZeroFont(void);

// ****************************************************************************
//
// > Draw data
//
// ****************************************************************************

#if FUI_USE_16BIT_INDICES
	//! One index into the vertex buffer
	typedef uint16_t fuiDrawIndex;
	//! Highest vertex index that can be addressed
#	define FUI_DRAW_INDEX_MAX 65535u
#else
	//! One index into the vertex buffer
	typedef uint32_t fuiDrawIndex;
	//! Highest vertex index that can be addressed
#	define FUI_DRAW_INDEX_MAX 4294967295u
#endif

//! One vertex of the user interface geometry
typedef struct fuiVertex {
	//! Screen position in pixels, y-down from the top-left window corner
	fuiVec2 position;
	//! Texture coordinate, unused and left at zero for untextured geometry
	fuiVec2 uv;
	//! Packed color in RGBA byte order, see @ref fuiPackColor
	uint32_t color;
} fuiVertex;

/**
* @enum fuiDrawKind
* @brief What primitive a command came from, so a backend can take a shortcut instead of drawing triangles.
* @note Every command carries valid triangles regardless of its kind. The kind and its payload are an
*       OPTIONAL fast path, never the only way to draw a command.
*/
typedef enum fuiDrawKind {
	//! No shortcut available, draw the indices
	FUI_DRAW_TRIANGLES = 0,
	//! One axis aligned filled rectangle, see fuiDrawCommand::payload::rect
	FUI_DRAW_RECT,
	//! One axis aligned rectangle outline, see fuiDrawCommand::payload::rectOutline
	FUI_DRAW_RECT_OUTLINE,
	//! One straight line segment, see fuiDrawCommand::payload::line
	FUI_DRAW_LINE,
	//! One string on one baseline, see fuiDrawCommand::payload::text
	FUI_DRAW_TEXT,
	//! One textured axis aligned quad, see fuiDrawCommand::payload::image
	FUI_DRAW_IMAGE,
} fuiDrawKind;

//! Fast path payload of a @ref FUI_DRAW_RECT command
typedef struct fuiDrawRectPayload {
	//! The rectangle to fill
	fuiRect rect;
	//! The fill color
	fuiColor color;
} fuiDrawRectPayload;

//! Fast path payload of a @ref FUI_DRAW_RECT_OUTLINE command
typedef struct fuiDrawRectOutlinePayload {
	//! The rectangle to outline, the stroke is drawn INSIDE these bounds
	fuiRect rect;
	//! Stroke width in pixels
	float thickness;
	//! The stroke color
	fuiColor color;
} fuiDrawRectOutlinePayload;

//! Fast path payload of a @ref FUI_DRAW_LINE command
typedef struct fuiDrawLinePayload {
	//! Where the line starts
	fuiVec2 start;
	//! Where the line ends
	fuiVec2 end;
	//! Stroke width in pixels
	float thickness;
	//! The stroke color
	fuiColor color;
} fuiDrawLinePayload;

//! Fast path payload of a @ref FUI_DRAW_TEXT command
typedef struct fuiDrawTextPayload {
	//! Byte offset of the string inside fuiDrawData::textBuffer
	uint32_t textOffset;
	//! Length of the string in bytes, it is NOT zero terminated
	uint32_t textLength;
	//! Top-left corner of the text box, the baseline sits at position.y + metrics.ascent * pixelHeight
	fuiVec2 position;
	//! Height one em is drawn at, in pixels
	float pixelHeight;
	//! The text color
	fuiColor color;
} fuiDrawTextPayload;

/**
* @enum fuiImageFlags
* @brief How the sampled part of a texture is turned before it is mapped onto the quad it fills.
* @note These are the library's OWN flags and match no other header's bit values - a backend that hands
*       them on to a sprite call of its own has to translate them there rather than cast them.
* @note Both rotate flags together mean no rotation at all, the same answer as neither of them.
*/
typedef enum fuiImageFlags {
	//! Drawn as it is stored
	FUI_IMAGE_FLAGS_NONE = 0,
	//! Mirrored left to right
	FUI_IMAGE_FLIP_U = 1 << 0,
	//! Mirrored top to bottom
	FUI_IMAGE_FLIP_V = 1 << 1,
	//! Turned a quarter turn clockwise on screen
	FUI_IMAGE_ROTATE_90_CW = 1 << 2,
	//! Turned a quarter turn counter clockwise on screen
	FUI_IMAGE_ROTATE_90_CCW = 1 << 3,
} fuiImageFlags;

/**
* @struct fuiDrawImagePayload
* @brief Fast path payload of a @ref FUI_DRAW_IMAGE command.
* @note `rect` is the FINAL quad, already the shape a quarter turn made it - a backend maps the texture
*       onto it as `flags` says and must not turn the rectangle a second time.
*/
typedef struct fuiDrawImagePayload {
	//! Where the image goes
	fuiRect rect;
	//! Top-left texture coordinate, before flags are applied
	fuiVec2 uvMin;
	//! Bottom-right texture coordinate, before flags are applied
	fuiVec2 uvMax;
	//! Tint multiplied onto the texture, white leaves it unchanged
	fuiColor color;
	//! How the sampled part is turned onto the quad @ref fuiImageFlags. The vertices already carry it
	fuiImageFlags flags;
} fuiDrawImagePayload;

/**
* @struct fuiDrawCommand
* @brief One batch of geometry to draw, with everything a backend needs to draw it.
* @note Index ranges of consecutive commands are contiguous, so a backend that only wants fewer draw
*       calls can merge neighbours that share clipRect and texture by extending indexCount.
*/
typedef struct fuiDrawCommand {
	//! Which primitive this came from, for backends that want a shortcut
	fuiDrawKind kind;
	//! Already intersected with every enclosing clip, so it can be used as a scissor rectangle directly
	fuiRect clipRect;
	//! Texture to sample, or @ref FUI_TEXTURE_ID_NONE for untextured geometry
	fuiTextureId texture;
	//! Index of the first vertex this command uses, for backends that draw with a base vertex
	uint32_t vertexOffset;
	//! Offset of the first index in fuiDrawData::indices
	uint32_t indexOffset;
	//! How many indices to draw, always a multiple of three
	uint32_t indexCount;
	//! Fast path payload - the member matching `kind` is valid, and is ignored entirely when drawing triangles
	union {
		//! Valid when kind is @ref FUI_DRAW_RECT
		fuiDrawRectPayload rect;
		//! Valid when kind is @ref FUI_DRAW_RECT_OUTLINE
		fuiDrawRectOutlinePayload rectOutline;
		//! Valid when kind is @ref FUI_DRAW_LINE
		fuiDrawLinePayload line;
		//! Valid when kind is @ref FUI_DRAW_TEXT
		fuiDrawTextPayload text;
		//! Valid when kind is @ref FUI_DRAW_IMAGE
		fuiDrawImagePayload image;
	} payload;
} fuiDrawCommand;

/**
* @struct fuiDrawData
* @brief Everything one frame of user interface needs to be drawn.
* @note Stays valid until the next @ref fuiBeginDrawFrame on the same context. Nothing in here is owned
*       by the caller and nothing has to be freed.
*/
typedef struct fuiDrawData {
	//! All vertices of the frame, addressed by the indices below
	const fuiVertex *vertices;
	//! Absolute indices into the vertex array, three per triangle
	const fuiDrawIndex *indices;
	//! The commands, in the order they were built, which is the order they must be drawn in
	const fuiDrawCommand *commands;
	//! Backing bytes every @ref FUI_DRAW_TEXT payload points into
	const char *textBuffer;
	//! Number of entries in the vertex array
	uint32_t vertexCount;
	//! Number of entries in the index array
	uint32_t indexCount;
	//! Number of entries in the command array
	uint32_t commandCount;
	//! Number of bytes in the text buffer
	uint32_t textBufferSize;
	//! Size of the window this frame was built for, in pixels
	fuiVec2i windowSize;
} fuiDrawData;

//! One growable array of frame geometry, sub-allocated from the context arena
typedef struct fuiDrawBuffer {
	//! Start of the array
	void *items;
	//! How many entries are used this frame
	uint32_t count;
	//! How many entries fit before the array has to grow
	uint32_t capacity;
} fuiDrawBuffer;

// ****************************************************************************
//
// > Input
//
// ****************************************************************************

//! Index of the left mouse button
#define FUI_MOUSE_LEFT 0
//! Index of the middle mouse button
#define FUI_MOUSE_MIDDLE 1
//! Index of the right mouse button
#define FUI_MOUSE_RIGHT 2
//! How many mouse buttons are tracked
#define FUI_MOUSE_BUTTON_COUNT 3

/**
* @struct fuiButtonState
* @brief State of one button for one frame, counting transitions rather than just the final position.
* @note Counting half transitions is what lets a key that was tapped AND released inside a single frame
*       still register, which a plain "is it down" flag loses.
*/
typedef struct fuiButtonState {
	//! How many times the button changed state since the last frame
	int32_t halfTransitionCount;
	//! Whether the button was down at the end of the frame
	bool endedDown;
} fuiButtonState;

/**
* @brief Tests whether a button is currently held.
* @param[in] state The button state @ref fuiButtonState.
* @return Returns true while the button is down.
*/
fui_inline bool fuiButtonIsDown(const fuiButtonState state) {
	bool result = state.endedDown;
	return(result);
}

/**
* @brief Tests whether a button was pressed this frame.
* @param[in] state The button state @ref fuiButtonState.
* @return Returns true on the frame the button went DOWN.
* @note This is the press edge. Use it for anything that should happen when a button is pushed.
*/
fui_inline bool fuiButtonWentDown(const fuiButtonState state) {
	bool result = (state.endedDown && state.halfTransitionCount >= 1) || (!state.endedDown && state.halfTransitionCount >= 2);
	return(result);
}

/**
* @brief Tests whether a button was released this frame.
* @param[in] state The button state @ref fuiButtonState.
* @return Returns true on the frame the button went UP.
*/
fui_inline bool fuiButtonWentUp(const fuiButtonState state) {
	bool result = (!state.endedDown && state.halfTransitionCount >= 1) || (state.endedDown && state.halfTransitionCount >= 2);
	return(result);
}

/**
* @enum fuiKey
* @brief The keys a user interface cares about, which is far fewer than a keyboard has.
* @note These are the library's OWN codes. A caller translates its platform's key codes into these once,
*       which is what keeps final_ui.h free of any platform header.
*/
typedef enum fuiKey {
	//! No key
	FUI_KEY_NONE = 0,

	//! The backspace key
	FUI_KEY_BACKSPACE,
	//! The tab key
	FUI_KEY_TAB,
	//! The return or enter key
	FUI_KEY_RETURN,
	//! The escape key
	FUI_KEY_ESCAPE,
	//! The space bar
	FUI_KEY_SPACE,
	//! The page up key
	FUI_KEY_PAGE_UP,
	//! The page down key
	FUI_KEY_PAGE_DOWN,
	//! The end key
	FUI_KEY_END,
	//! The home key
	FUI_KEY_HOME,
	//! The left arrow key
	FUI_KEY_LEFT,
	//! The up arrow key
	FUI_KEY_UP,
	//! The right arrow key
	FUI_KEY_RIGHT,
	//! The down arrow key
	FUI_KEY_DOWN,
	//! The insert key
	FUI_KEY_INSERT,
	//! The delete key
	FUI_KEY_DELETE,

	//! The digit 0, the digits are contiguous up to @ref FUI_KEY_9
	FUI_KEY_0,
	FUI_KEY_1, FUI_KEY_2, FUI_KEY_3, FUI_KEY_4, FUI_KEY_5, FUI_KEY_6, FUI_KEY_7, FUI_KEY_8,
	//! The digit 9
	FUI_KEY_9,

	//! The letter A, the letters are contiguous up to @ref FUI_KEY_Z
	FUI_KEY_A,
	FUI_KEY_B, FUI_KEY_C, FUI_KEY_D, FUI_KEY_E, FUI_KEY_F, FUI_KEY_G, FUI_KEY_H, FUI_KEY_I,
	FUI_KEY_J, FUI_KEY_K, FUI_KEY_L, FUI_KEY_M, FUI_KEY_N, FUI_KEY_O, FUI_KEY_P, FUI_KEY_Q,
	FUI_KEY_R, FUI_KEY_S, FUI_KEY_T, FUI_KEY_U, FUI_KEY_V, FUI_KEY_W, FUI_KEY_X, FUI_KEY_Y,
	//! The letter Z
	FUI_KEY_Z,

	//! The function key F1, the function keys are contiguous up to @ref FUI_KEY_F12
	FUI_KEY_F1,
	FUI_KEY_F2, FUI_KEY_F3, FUI_KEY_F4, FUI_KEY_F5, FUI_KEY_F6, FUI_KEY_F7, FUI_KEY_F8,
	FUI_KEY_F9, FUI_KEY_F10, FUI_KEY_F11,
	//! The function key F12
	FUI_KEY_F12,

	//! The left control key
	FUI_KEY_LEFT_CONTROL,
	//! The right control key
	FUI_KEY_RIGHT_CONTROL,
	//! The left shift key
	FUI_KEY_LEFT_SHIFT,
	//! The right shift key
	FUI_KEY_RIGHT_SHIFT,
	//! The left alt key
	FUI_KEY_LEFT_ALT,
	//! The right alt key
	FUI_KEY_RIGHT_ALT,

	//! How many keys there are, which is the size of the key array in @ref fuiInput
	FUI_KEY_COUNT,
} fuiKey;

/**
* @struct fuiInput
* @brief One frame of input, filled in by the caller and handed to @ref fuiBeginFrame.
* @note The library never reads a device. Filling this from a platform layer is the caller's job, and
*       is the whole of what an adapter has to do.
*/
typedef struct fuiInput {
	//! Cursor position in pixels, y-down from the top-left window corner
	fuiVec2 mousePosition;
	//! How far the wheel turned this frame, positive is away from the user
	float mouseWheelDelta;
	//! State of every mouse button, indexed by @ref FUI_MOUSE_LEFT and friends
	fuiButtonState mouseButtons[FUI_MOUSE_BUTTON_COUNT];
	//! State of every key, indexed by @ref fuiKey
	fuiButtonState keys[FUI_KEY_COUNT];
	//! Unicode codepoints typed this frame, in the order they arrived
	uint32_t textInput[FUI_MAX_TEXT_INPUT];
	//! How many entries of textInput are filled
	int32_t textInputLength;
	//! Size of the window in pixels
	fuiVec2i windowSize;
	//! Seconds elapsed since the previous frame
	float deltaTime;
	//! False while the window has no focus, which suppresses all hovering and pressing
	bool isActive;
} fuiInput;

/**
* @brief Returns a zeroed input, safe to fill in field by field.
* @return Returns an input with every key, button and codepoint cleared.
*/
fui_api fuiInput fuiZeroInput(void);

/**
* @brief Returns the display name of a key, for showing a shortcut.
* @param[in] key The key @ref fuiKey to name.
* @return Returns a static string such as "Ctrl" or "F5", never null.
*/
fui_api const char *fuiKeyGetName(const fuiKey key);

/**
* @brief Reports a held key as a stream of repeats, the way a text field wants an arrow key.
* @param[in] button The button state @ref fuiButtonState of the key.
* @param[in] frameTime Seconds elapsed since the previous frame.
* @param[in,out] timer Caller owned countdown, one per key being watched, zero initialized.
* @return Returns true on the press itself and then again on every repeat.
* @see @ref FUI_KEY_REPEAT_DELAY, @ref FUI_KEY_REPEAT_INTERVAL
*/
fui_api bool fuiKeyRepeatEdge(const fuiButtonState button, const float frameTime, float *timer);

// ****************************************************************************
//
// > Platform
//
// ****************************************************************************

/**
* @enum fuiCursor
* @brief The mouse cursor shape a widget would like the host to show.
* @note The library never changes a cursor itself, it only says which one it wants. A host that has no
*       cursor api at all simply leaves setCursor null and everything else keeps working.
*/
typedef enum fuiCursor {
	//! The ordinary pointer, which is what every frame starts as
	FUI_CURSOR_ARROW = 0,
	//! The I-beam of an editable text field
	FUI_CURSOR_TEXT,
	//! A left-right resize handle, such as a vertical splitter
	FUI_CURSOR_RESIZE_HORIZONTAL,
	//! An up-down resize handle, such as a horizontal splitter
	FUI_CURSOR_RESIZE_VERTICAL,
	//! A corner resize handle
	FUI_CURSOR_RESIZE_DIAGONAL,
	//! The pointing hand of something clickable
	FUI_CURSOR_HAND,
	//! How many cursor shapes there are
	FUI_CURSOR_COUNT,
} fuiCursor;

/**
* @struct fuiPlatform
* @brief The few services a user interface wants from its host, all of them optional.
* @note Every callback may be null. A missing clipboard makes copy and paste do nothing rather than fail,
*       and a missing cursor callback means the shape is only reported through @ref fuiGetCursor.
*/
typedef struct fuiPlatform {
	//! Copies the clipboard contents into destination, returns false when there is no clipboard
	bool (*getClipboardText)(void *userData, char *destination, uint32_t maxDestinationLength);
	//! Replaces the clipboard contents, returns false when there is no clipboard
	bool (*setClipboardText)(void *userData, const char *text);
	//! Asks the host to show a cursor shape, called only when the shape actually changes
	void (*setCursor)(void *userData, fuiCursor cursor);
	//! Passed back to every callback above
	void *userData;
} fuiPlatform;

// The calls that install and read these live in the Context section below, because every one of them
// takes a fuiContext and the context has to carry a fuiPlatform of its own before it can be declared.

// ****************************************************************************
//
// > Theme
//
// ****************************************************************************

//! Every color and size the widgets draw with. All sizes are in pixels.
typedef struct fuiTheme {
	//! Fill behind a panel's content
	fuiColor panelBackgroundColor;
	//! Outline of a panel
	fuiColor panelBorderColor;
	//! Primary text
	fuiColor textColor;
	//! Secondary or de-emphasized text, such as hints and disabled labels
	fuiColor textMutedColor;
	//! Highlights, active markers, carets and focus borders
	fuiColor accentColor;

	//! Idle fill of a clickable widget, such as a button face
	fuiColor widgetColor;
	//! Fill of a widget while the cursor is over it
	fuiColor widgetHoveredColor;
	//! Fill of a widget while it is pressed or dragged
	fuiColor widgetActiveColor;
	//! Recessed backing, such as a slider track, a checkbox box or a text field
	fuiColor widgetTrackColor;
	//! Lit edge along the top and left of a raised widget, and along the bottom and right of a sunken one. Translucent, so it brightens whatever face it lies on instead of painting one fixed color over all of them
	fuiColor widgetBevelLightColor;
	//! Shaded edge opposite the lit one. Translucent for the same reason the lit edge is
	fuiColor widgetBevelShadowColor;
	//! Wash drawn behind selected text, translucent so the glyphs on top of it stay readable
	fuiColor textSelectionColor;
	//! Slider knob and checkbox check mark
	fuiColor knobColor;
	//! Fill behind a hovered menu title or menu row
	fuiColor menuHighlightColor;
	//! Dim wash drawn over the whole window behind a modal dialog
	fuiColor modalBackdropColor;

	//! Fill of a hover tooltip box, nearly opaque so it stays readable over arbitrary content
	fuiColor tooltipBackgroundColor;
	//! Outline of a hover tooltip box
	fuiColor tooltipBorderColor;
	//! Text inside a hover tooltip box
	fuiColor tooltipTextColor;

	//! Base text pixel height
	float fontHeight;
	//! Row height of a vertical strip button or list box entry
	float menuItemHeight;
	//! Text pixel height of a menu bar or popup row, independent of fontHeight
	float menuItemFontHeight;
	//! Horizontal inset of a menu row's content from its left and right edges
	float menuItemPaddingX;
	//! Vertical inset above and below the text in a menu row
	float menuItemPaddingY;
	//! Starting width of a menu popup until its rows are measured
	float menuPopupMinWidth;
	//! Outline stroke width of the chrome: a panel, a dialog, a menu popup and a group box all carry this ONE frame width, so nothing in the interface reads as more heavily boxed than its neighbour
	float panelBorderThickness;
	//! Outline stroke width of a widget
	float widgetBorderThickness;
	//! Width of the lit and the shaded edge that make a widget read as raised out of the panel or pushed into it. Set to zero for a flat interface
	float widgetBevelThickness;
	//! How much lighter the top of a widget face is drawn than its bottom, as a 0 to 1 fraction of the way to white and to black. Set to zero for a flat face
	float widgetFaceShadingStrength;
	//! How far a pressed widget nudges its caption down and to the right, so a click is felt as well as seen
	float widgetPressOffset;
	//! Content inset from a panel's left and right edges
	float panelPaddingX;
	//! Content inset from a panel's top and bottom edges
	float panelPaddingY;
	//! Text inset inside a widget
	float widgetPaddingX;
	//! Margin a widget in a flow container carries on its TRAILING side, below it in a vertical flow and right of it in a horizontal one. Sized like the panel's side padding, so the gaps between the widgets and the gaps around them read as one measure
	float widgetSpacing;
	//! Slot height a group separator occupies in a flow container
	float groupSeparatorHeight;
	//! Text caret on and off transitions per second
	float caretBlinkHz;

	//! Text pixel height inside a tooltip box
	float tooltipFontHeight;
	//! Inset of the text from the tooltip box's left and right edges
	float tooltipPaddingX;
	//! Inset of the text from the tooltip box's top and bottom edges
	float tooltipPaddingY;
	//! Extra gap between stacked lines of a multi-line tooltip
	float tooltipLineSpacing;
	//! How far right of the cursor the tooltip box starts, mirrored when it flips left
	float tooltipCursorOffsetX;
	//! How far below the cursor the tooltip box's top edge sits, mirrored when it flips up
	float tooltipCursorOffsetY;
	//! How close to a window edge the tooltip box may ever come
	float tooltipScreenMargin;
	//! How long the cursor must rest on one thing before its tooltip appears
	float tooltipDelaySeconds;
} fuiTheme;

/**
* @brief Returns the built-in look, a dark theme with a blue accent.
* @return Returns the default theme.
* @note @ref fuiInit copies this onto the context; change the copy to restyle one context.
*/
fui_api fuiTheme fuiDefaultTheme(void);

// ****************************************************************************
//
// > Identity
//
// ****************************************************************************

//! Identifies one widget across frames. A 64-bit FNV-1a hash of its label, combined with its scope.
typedef uint64_t fuiId;

//! The identifier that means "no widget"
#define FUI_ID_NONE ((fuiId)0)

/**
* @enum fuiPass
* @brief What a build of the user interface is for.
* @note The whole tree is built by calling the same widget functions, so a two-pass caller runs its
*       build function twice and the library decides what each run is allowed to do.
*/
typedef enum fuiPass {
	//! Interact and draw in one build. The simple loop, at the cost of hover and click resolving against
	//! the previous frame's layout - the standard immediate mode contract, and one frame of lag.
	FUI_PASS_BOTH = 0,
	//! Hit test and run interactions, emit no geometry. Build this one where input is read, so that
	//! @ref fuiWantsMouse is already correct before the caller decides what to do with the cursor.
	FUI_PASS_INTERACT,
	//! Emit geometry only. Every input edge is neutralized, so building the same tree a second time
	//! cannot fire anything twice.
	FUI_PASS_DRAW,
} fuiPass;

// ****************************************************************************
//
// > Layout types
//
// ****************************************************************************

/**
* @enum fuiDock
* @brief Where a container places itself inside its parent's remaining content box.
* @note Each docked child bites a strip off the chosen edge and hands the rest to the next one, so the
*       ORDER the children are opened in is what decides the layout.
*/
typedef enum fuiDock {
	//! Free floating at an explicit position and size, taking nothing away from the parent
	FUI_DOCK_NONE = 0,
	//! Bite a strip of the given width off the left edge
	FUI_DOCK_LEFT,
	//! Bite a strip of the given width off the right edge
	FUI_DOCK_RIGHT,
	//! Bite a strip of the given height off the top edge
	FUI_DOCK_TOP,
	//! Bite a strip of the given height off the bottom edge
	FUI_DOCK_BOTTOM,
	//! Consume everything the parent has left
	FUI_DOCK_FILL,
} fuiDock;

//! Direction a container hands its slots out in.
typedef enum fuiAxis {
	//! Slots stack downwards, each taking the full content width
	FUI_AXIS_VERTICAL = 0,
	//! Slots stack to the right, each taking the full content height
	FUI_AXIS_HORIZONTAL,
} fuiAxis;

/**
* @struct fuiWidgetState
* @brief What one widget remembers between frames, looked up by its identifier.
* @note An immediate mode widget is gone the moment it is built, so anything the USER decided about it -
*       where they dragged a panel, how far they scrolled a list, whether they folded a section away - has
*       to live here instead. Slots are created when a widget is first seen and never expire.
*/
typedef struct fuiWidgetState {
	//! Identifier of the widget this state belongs to, zero while the slot is free
	fuiId id;
	//! How far a floating panel has been dragged from where the caller put it, in pixels
	fuiVec2 moveOffset;
	//! How much a resize grip has grown the container beyond the size the caller asked for, in pixels
	fuiVec2 sizeOffset;
	//! Flow length the content consumed the last time it was built, which is what auto sizing reads
	float measuredExtent;
	//! Scroll offset of a scrolling container, in pixels from the top of its content
	float scroll;
	//! Sideways scroll offset of a container wider than its box, in pixels from the left of its content
	float scrollX;
	//! Hue a color picker last showed, kept because rgb cannot express one at zero saturation or value
	float pickerHue;
	//! Size a menu popup measured itself at the last time it was open, zero until it has been open once
	fuiVec2 menuSize;
	//! When the last row of a list was clicked, which is what a second click is timed against
	float lastClickTime;
	//! Which row that last click landed on, minus one when no row has been clicked yet
	int32_t lastClickIndex;
	//! Which tab of a tab control is showing
	int32_t activeTab;
	//! First VISIBLE line of a multiline text field, which is what scrolling one moves
	int32_t textFirstLine;
	//! How many lines its whole buffer broke into, the last time that was counted
	int32_t textTotalLines;
	//! What that count was taken from. Counting walks the whole document, so it is taken again only when
	//! one of these changed - a different buffer, a different length, a different width to wrap at
	const void *textBuffer;
	int32_t textBufferLength;
	float textWrapWidth;
	bool textWordWrap;
	//! Whether textTotalLines holds a real count yet
	bool textTotalIsCounted;
	//! Byte offset the window laid out last began at, and which line that was. Seeking to the next window
	//! from HERE is what stops every scroll from laying the whole document out again
	int32_t textWindowOffset;
	int32_t textWindowLine;
	bool textWindowIsKnown;
	//! Cached display order of a sorted list, one source row per display position, or null while nothing
	//! has sorted this list yet. Owned by the context arena, which never gives an allocation back
	int32_t *sortOrder;
	//! How many entries sortOrder has room for, which only ever grows
	int32_t sortOrderCapacity;
	//! The cell array sortOrder was built from, compared by ADDRESS to notice a caller handing over a different one
	const void *sortOrderCells;
	//! Hash of the whole sorted column as it read when sortOrder was built, for a list short enough to check
	fuiId sortOrderFingerprint;
	//! Whether that hash means anything, which it does not for a list too long to hash every frame
	bool sortOrderWasFingerprinted;
	//! How many rows, how many columns, which column and which direction sortOrder was built for
	int32_t sortOrderRowCount;
	int32_t sortOrderColumnCount;
	int32_t sortOrderColumn;
	bool sortOrderIsAscending;
	//! Whether sortOrder holds a finished order at all, which is what an invalidation clears
	bool sortOrderIsBuilt;
	//! LIVE width of every column of a list view, seeded from the caller's defaults and then owned by the drags
	float columnWidths[FUI_MAX_LIST_COLUMNS];
	//! How many of columnWidths are tracked. Zero until seeded, reseeded when the column COUNT changes
	int32_t columnWidthCount;
	//! Which column a list view is sorted by, meaningful only while sortIsActive
	int32_t sortColumn;
	//! Whether the rows are sorted at all. False - what a fresh list is - is the caller's own row order
	bool sortIsActive;
	//! Whether that sort reads the way a list is read, rather than backwards
	bool sortIsAscending;
	//! Whether a starting sort has already been applied once, which is what makes seeding it every build safe
	bool sortDefaultWasSeeded;
	//! Whether the USER may not sort by clicking a header. Inverted, so a fresh list is sortable
	bool sortIsLocked;
	//! Whether a panel is folded away to its title bar
	bool isCollapsed;
	//! Whether measuredExtent holds a real measurement yet
	bool hasMeasured;
	//! Whether pickerHue holds a remembered hue yet
	bool hasPickerHue;
	//! Whether this slot is taken
	bool isUsed;
} fuiWidgetState;

//! Open addressed map of @ref fuiId to @ref fuiWidgetState, allocated the first time a widget needs it.
typedef struct fuiWidgetStateMap {
	//! The slots, or null while nothing has needed retained state yet
	fuiWidgetState *entries;
	//! How many slots there are, zero when the allocation was refused
	uint32_t capacity;
	//! How many of them are taken. Slots are never given back, so this only ever grows
	uint32_t count;
	//! Set when the table filled up or got crowded, which is what makes the NEXT frame grow it. Growing
	//! mid frame would move every entry while widgets are holding pointers into them
	bool growthIsWanted;
} fuiWidgetStateMap;

/**
* @struct fuiLayoutNode
* @brief One container that is open right now.
* @note Children take their space out of `remaining`, so a container shrinks as it is filled. A docked
*       child bites an edge strip off it, a flowing child takes a slot off the leading edge.
*/
typedef struct fuiLayoutNode {
	//! Identifier of the container
	fuiId id;
	//! Retained state of the container, or null when it remembers nothing
	fuiWidgetState *state;
	//! The container's full inner content box
	fuiRect contentBox;
	//! What is still free, shrinking as children are placed
	fuiRect remaining;
	//! Visible part of a scrolling container, which is what the content is clipped to
	fuiRect scrollViewport;
	//! Gutter a scrolling container's scrollbar is drawn in
	fuiRect scrollTrack;
	//! Direction slot children flow in
	fuiAxis axis;
	//! Gap inserted between two consecutive slots
	float spacing;
	//! Total flow length the slots consumed this build
	float usedExtent;
	//! Whether this container drew a panel background and pushed a clip that has to be undone
	bool isPanel;
	//! Whether this container is folded away, so slots come back empty and nothing draws
	bool isHidden;
	//! Whether this container scrolls its content instead of clipping the overflow away
	bool isScrollable;
	//! False until the first slot, so no gap is inserted in front of it
	bool hasFlowStarted;
} fuiLayoutNode;

/**
* @struct fuiMenuFrame
* @brief One menu container that is open right now: a menu bar, or a popup a title or a row opened.
* @note A popup is measured as its rows are emitted, which is one row too late to have sized its box. So the
*       measurement is remembered in @ref fuiWidgetState and the background is drawn at the size the PREVIOUS
*       build came to - the usual immediate mode bargain, and invisible as long as the rows do not change.
*/
typedef struct fuiMenuFrame {
	//! Identifies the menu, which is also what the open path holds while its popup is up
	fuiId id;
	//! Menu bar: the strip the titles flow across
	fuiRect barRect;
	//! Popup: the whole box, at the size the previous build measured
	fuiRect popupRect;
	//! The title or the row that opens this popup, which is what a submenu is placed against
	fuiRect triggerRect;
	//! Menu bar: where the next title starts
	float barCursorX;
	//! Popup: top edge of the next row, growing downward
	float cursorY;
	//! Popup: the widest row this build wants, which is the width the next build opens at
	float widestItem;
	//! Popup: how far its rows are lifted, for a menu with more rows than the screen is tall
	float scrollOffset;
	//! Popup: how tall all of its rows were the last time it was open, which is what the scroll is bounded by
	float contentHeight;
	//! Popup: the strip along the top that says there are rows above and scrolls back to them, empty when there are none
	fuiRect scrollUpStrip;
	//! Popup: the same along the bottom, empty when every remaining row is already showing
	fuiRect scrollDownStrip;
	//! How deep this menu sits in the open path, so a row knows which submenus to close
	uint32_t depth;
	//! Whether this frame is the horizontal bar rather than a popup
	bool isBar;
	//! Whether this menu's popup is up, so its rows are really being emitted
	bool isOpen;
} fuiMenuFrame;

/**
* @struct fuiModalFrame
* @brief One @ref fuiBeginModal .. @ref fuiEndModal pair that is open right now during this build.
* @note A frame is pushed whether or not the dialog is open, so the pair always unwinds evenly and a caller
*       never has to guard @ref fuiEndModal. `isOpen` is what tells the end which pushes there are to undo.
*/
typedef struct fuiModalFrame {
	//! Identifies the dialog, which is also what the open stack holds while it is up
	fuiId id;
	//! The modal scope this one was opened inside, restored when it ends so dialogs may nest
	fuiId savedModalScope;
	//! Whether this dialog was open, and so whether the begin pushed a layout, a clip and an identifier
	bool isOpen;
} fuiModalFrame;

// ****************************************************************************
//
// > Context
//
// ****************************************************************************

/**
* @struct fuiContext
* @brief Holds everything one user interface needs. Create one per screen that draws its own interface.
* @note All fields are public for inspection, but only the library writes them. There is no global state,
*       so several contexts can be built in parallel and none of them talk to each other.
*/
typedef struct fuiContext {
	//! Owns every dynamic allocation this context makes
	fuiArena arena;
	//! Live look, copied from @ref fuiDefaultTheme at init and free to change afterwards
	fuiTheme theme;
	//! Font text is measured and drawn with, owned by the caller and allowed to be null
	const fuiFont *font;

	//! Vertices built this frame
	fuiDrawBuffer vertexBuffer;
	//! Indices built this frame
	fuiDrawBuffer indexBuffer;
	//! Commands built this frame
	fuiDrawBuffer commandBuffer;
	//! Bytes of every text command built this frame
	fuiDrawBuffer textBuffer;
	//! Snapshot handed out by @ref fuiGetDrawData, rebuilt by @ref fuiEndDrawFrame
	fuiDrawData drawData;

	//! Clip rectangles, each already intersected with the one below it
	fuiRect clipStack[FUI_MAX_CLIP_DEPTH];
	//! How many entries of the clip stack are in use
	uint32_t clipDepth;

	//! Size of the window the current frame is built for
	fuiVec2i windowSize;
	//! True between @ref fuiBeginDrawFrame and @ref fuiEndDrawFrame
	bool isBuildingFrame;
	//! Set when geometry had to be dropped because the index type ran out of range
	bool indexRangeExceeded;
	//! Whether consecutive commands sharing a clip and a texture are merged into one, see @ref fuiSetDrawBatching
	bool drawBatchingIsOn;

	//! What the current build is for
	fuiPass pass;
	//! False during a draw pass, when every input edge is neutralized
	bool isInteractive;
	//! Seconds elapsed since the previous frame, zero during a draw pass so timers do not advance twice
	float frameTime;
	//! Seconds since the context was initialized, which is the clock a double click is measured against
	float timeSeconds;

	//! Cursor position this frame, in pixels
	fuiVec2 mousePosition;
	//! How far the cursor moved since the previous frame
	fuiVec2 mouseDelta;
	//! How far the wheel turned this frame
	float mouseWheelDelta;
	//! Whether each mouse button is held
	bool mouseIsDown[FUI_MOUSE_BUTTON_COUNT];
	//! Whether each mouse button went down this frame
	bool mouseWentDown[FUI_MOUSE_BUTTON_COUNT];
	//! Whether each mouse button went up this frame
	bool mouseWentUp[FUI_MOUSE_BUTTON_COUNT];
	//! Latched once a widget took a press, cleared when that button is released again. Stops a click
	//! whose popup closed under the cursor from leaking through to whatever is behind it.
	bool mouseDownConsumed[FUI_MOUSE_BUTTON_COUNT];
	//! Whether the window had focus this frame
	bool inputIsActive;
	//! Key states this frame, indexed by @ref fuiKey
	fuiButtonState keys[FUI_KEY_COUNT];
	//! Codepoints typed this frame
	uint32_t textInput[FUI_MAX_TEXT_INPUT];
	//! How many entries of textInput are filled
	int32_t textInputLength;
	//! Countdown to the next repeat of each held key, so an arrow key held down keeps stepping
	float keyRepeatTimers[FUI_KEY_COUNT];
	//! Whether each key fired this frame counting repeats, resolved once so asking twice cannot double the rate
	bool keyRepeated[FUI_KEY_COUNT];

	//! Host services, all callbacks null until @ref fuiSetPlatform is called
	fuiPlatform platform;
	//! Cursor shape asked for this frame, reset to the arrow by @ref fuiBeginFrame
	fuiCursor cursor;
	//! Shape last handed to the host, so the callback fires on a change and not every frame
	fuiCursor appliedCursor;

	//! Widget the cursor is over, resolved at the end of the previous build
	fuiId hot;
	//! Widget the cursor is over according to THIS build, promoted to hot by @ref fuiEndFrame
	fuiId hotCandidate;
	//! Widget that captured the press, held across frames while a button is down
	fuiId active;
	//! Widget that owns the keyboard
	fuiId focused;
	//! Scopes that widget identifiers are combined with, so the same label under two panels stays distinct
	fuiId idStack[FUI_MAX_ID_STACK];
	//! How many entries of the identifier stack are in use
	uint32_t idDepth;
	//! Set as soon as anything claimed the cursor this build, which is what @ref fuiWantsMouse reports
	bool mouseIsOverUi;
	//! Cleared by the first frame, so the first mouse delta is not the whole screen
	bool hasSeenAFrame;

	//! Byte offset of the caret inside the focused text field's buffer
	int32_t caretPosition;
	//! Byte offset the selection was started at. Equal to caretPosition when nothing is selected
	int32_t selectionAnchor;
	//! Which text field the caret and the selection belong to, so they re-anchor when focus moves
	fuiId caretOwner;
	//! Seconds since the caret last moved, which restarts the blink so the caret is solid while typing
	float caretBlinkTime;

	//! First focusable widget built this frame, which is where a tab off the last one wraps back to
	fuiId firstFocusableThisFrame;
	//! Focusable widget built just before the current one, so tab knows what comes next
	fuiId previousFocusableThisFrame;
	//! Set once a tab has moved the focus this frame, so one press cannot walk the whole chain
	bool tabWasConsumedThisFrame;
	//! Set once a dialog answered the enter or escape of this frame, so one press cannot close two of them
	bool dialogKeyWasConsumedThisFrame;

	//! Text of the tooltip requested this build, empty when nothing asked for one
	char tooltipText[FUI_MAX_TOOLTIP_TEXT];
	//! Where the cursor was when the tooltip was requested, which is what the box is placed against
	fuiVec2 tooltipCursor;
	//! How long the cursor has rested on this same text, counting against the theme's delay
	float tooltipHoverSeconds;
	//! Set by @ref fuiTooltip and cleared by the next @ref fuiBeginFrame
	bool tooltipRequested;

	//! Containers open right now, the innermost one last
	fuiLayoutNode layoutStack[FUI_MAX_LAYOUT_DEPTH];
	//! How many entries of the layout stack are in use
	uint32_t layoutDepth;
	//! What widgets remember between frames, allocated the first time one asks for it
	fuiWidgetStateMap widgetStates;
	//! Countdown to the next repeat of the held widget. One widget is active at a time, so one timer serves them all
	float activeRepeatTimer;
	//! Where the merge sort lands its halves before they go back into the order. Shared, because one sort
	//! runs at a time, and grown to fit whichever list needed the most rows. Null until one is sorted
	int32_t *listSortScratch;
	//! How many entries listSortScratch has room for
	int32_t listSortScratchCapacity;

	//! Menus open right now, outermost first. A menu stays open across frames until something closes it
	fuiId menuOpenPath[FUI_MAX_MENU_DEPTH];
	//! How many entries of menuOpenPath are open, zero when no menu is up at all
	uint32_t menuOpenDepth;
	//! Where the cursor was when a context menu was opened, which is the corner its popup grows from
	fuiVec2 contextMenuAnchor;
	//! Set once the cursor was over a menu bar or an open popup this build, which is what tells a press
	//! that landed anywhere else that it is a dismissal
	bool menuOwnsTheMouse;
	//! Menu containers open right now during this build, the innermost one last
	fuiMenuFrame menuStack[FUI_MAX_MENU_DEPTH];
	//! How many entries of menuStack are in use
	uint32_t menuStackDepth;

	//! Dialogs open right now, the front one last. While this holds anything the interface is modal
	fuiId modalStack[FUI_MAX_DIALOGS];
	//! How many entries of modalStack are open, zero when no dialog is up at all
	uint32_t modalDepth;
	//! The dialog whose content is being built right now, zero while the build is outside every dialog
	fuiId modalScopeId;
	//! Whether a dialog was already up when this frame's interacting build began, which is what
	//! @ref fuiModalOwnedInputThisFrame reports to code that reads the same keys the dialogs do
	bool modalOpenAtFrameStart;
	//! Dialog builders open right now during this build, the innermost one last
	fuiModalFrame modalBuildStack[FUI_MAX_DIALOGS];
	//! How many entries of modalBuildStack are in use
	uint32_t modalBuildDepth;

	//! The tool strip being built, if one is open
	fuiRect stripRect;
	//! Direction the open tool strip hands its slots out in
	fuiAxis stripAxis;
	//! Leading edge of the next tool strip item, along the strip's own axis
	float stripCursor;
	//! Whether a tool strip is open right now
	bool stripIsActive;

	//! The status bar being built, if one is open
	fuiRect statusRect;
	//! Where the next left aligned status item starts, moving right
	float statusLeftCursor;
	//! Where the next right aligned status item ends, moving left
	float statusRightCursor;

	//! Set once @ref fuiInit succeeded, cleared again by @ref fuiRelease
	bool isInitialized;
} fuiContext;

/**
* @brief Initializes a user interface context.
* @param[out] context Reference to the context @ref fuiContext to initialize.
* @param[in] font Reference to the font @ref fuiFont text is drawn with, may be null and set later with @ref fuiSetFont.
* @param[in] allocator Reference to the allocator @ref fuiAllocator to take memory from, pass null to use malloc and free.
* @return Returns true when the context is ready to use.
* @note The context is caller owned, on the stack or embedded in another struct, and must be released with @ref fuiRelease.
* @see @ref fuiRelease
*/
fui_api bool fuiInit(fuiContext *context, const fuiFont *font, const fuiAllocator *allocator);

/**
* @brief Releases every block of memory the context owns and resets it to a safe empty state.
* @param[in,out] context Reference to the context @ref fuiContext to release.
* @note Calling this on an already released or never initialized context does nothing.
* @see @ref fuiInit
*/
fui_api void fuiRelease(fuiContext *context);

/**
* @brief Replaces the font the context measures and draws text with.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] font Reference to the font @ref fuiFont, may be null to draw no text at all.
* @note The font is not copied, so it has to stay alive for as long as the context uses it.
*/
fui_api void fuiSetFont(fuiContext *context, const fuiFont *font);

/**
* @brief Replaces the context's look.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] theme Reference to the theme @ref fuiTheme to copy in, pass null to restore the default.
*/
fui_api void fuiSetTheme(fuiContext *context, const fuiTheme *theme);

/**
* @brief Returns the context's live theme for reading or changing single values.
* @param[in,out] context Reference to the context @ref fuiContext.
* @return Returns a reference to the theme @ref fuiTheme, or null when the context is not initialized.
*/
fui_api fuiTheme *fuiGetTheme(fuiContext *context);

/**
* @brief Tests whether the context ran out of memory at any point since it was initialized.
* @param[in] context Reference to the context @ref fuiContext.
* @return Returns true when at least one allocation was refused.
* @note The library keeps working when this happens, it just leaves things out. Raise @ref FUI_ARENA_MIN_BLOCK_SIZE or check your allocator.
*/
fui_api bool fuiHasOutOfMemory(const fuiContext *context);

/**
* @brief Returns how many bytes the context has taken from its allocator.
* @param[in] context Reference to the context @ref fuiContext.
* @return Returns the sum of every arena block size in bytes.
*/
fui_api size_t fuiGetAllocatedSize(const fuiContext *context);

/**
* @brief Installs the host services a context may use, see @ref fuiPlatform.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] platform Reference to the services @ref fuiPlatform, pass null to remove them again.
* @note The struct is copied, so it does not have to outlive the call.
*/
fui_api void fuiSetPlatform(fuiContext *context, const fuiPlatform *platform);

/**
* @brief Asks for a cursor shape for the rest of this frame.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] cursor The shape @ref fuiCursor to show.
* @note Later calls win, and the shape resets to @ref FUI_CURSOR_ARROW at the start of every frame.
*/
fui_api void fuiSetCursor(fuiContext *context, const fuiCursor cursor);

/**
* @brief Returns the cursor shape the interface would like to be showing.
* @param[in] context Reference to the context @ref fuiContext.
* @return Returns the shape @ref fuiCursor.
* @note For a host that would rather set the cursor itself than hand over a callback.
*/
fui_api fuiCursor fuiGetCursor(const fuiContext *context);

/**
* @brief Reads the host clipboard.
* @param[in] context Reference to the context @ref fuiContext.
* @param[out] destination Receives the zero terminated clipboard text.
* @param[in] maxDestinationLength Size of the destination buffer in bytes.
* @return Returns true when the clipboard had text and it fit.
*/
fui_api bool fuiGetClipboardText(const fuiContext *context, char *destination, const uint32_t maxDestinationLength);

/**
* @brief Writes the host clipboard.
* @param[in] context Reference to the context @ref fuiContext.
* @param[in] text The zero terminated text to put on the clipboard.
* @return Returns true when the host took it.
*/
fui_api bool fuiSetClipboardText(const fuiContext *context, const char *text);

// ****************************************************************************
//
// > Frame
//
// ****************************************************************************

/**
* @brief Starts building one frame, taking a snapshot of the given input.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] input Reference to this frame's input @ref fuiInput.
* @param[in] pass What this build is for, see @ref fuiPass.
* @note A one pass caller passes @ref FUI_PASS_BOTH and builds once. A two pass caller builds the SAME
*       tree twice, once with @ref FUI_PASS_INTERACT where it reads input and once with
*       @ref FUI_PASS_DRAW where it renders, which is what removes the one frame of hover lag.
* @see @ref fuiEndFrame
*/
fui_api void fuiBeginFrame(fuiContext *context, const fuiInput *input, const fuiPass pass);

/**
* @brief Finishes the frame, resolving what the cursor ended up over.
* @param[in,out] context Reference to the context @ref fuiContext.
* @see @ref fuiBeginFrame
*/
fui_api void fuiEndFrame(fuiContext *context);

/**
* @brief Starts building one frame of geometry and throws away the previous one.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] windowSize Size of the window in pixels, which becomes the outermost clip rectangle.
* @note This is the drawing half of a frame on its own, without any input. @ref fuiBeginFrame calls it
*       for you, so reach for this only when there is genuinely nothing to interact with.
* @see @ref fuiEndDrawFrame
*/
fui_api void fuiBeginDrawFrame(fuiContext *context, const fuiVec2i windowSize);

/**
* @brief Finishes the frame and publishes what @ref fuiGetDrawData returns.
* @param[in,out] context Reference to the context @ref fuiContext.
* @note Any clip rectangle still pushed is dropped here, so an unbalanced push cannot leak into the next frame.
* @see @ref fuiBeginDrawFrame
*/
fui_api void fuiEndDrawFrame(fuiContext *context);

/**
* @brief Returns the geometry of the last finished frame.
* @param[in] context Reference to the context @ref fuiContext.
* @return Returns a reference to the draw data @ref fuiDrawData, never null but possibly empty.
* @note Stays valid until the next @ref fuiBeginDrawFrame on this context.
*/
fui_api const fuiDrawData *fuiGetDrawData(const fuiContext *context);

/**
* @brief Turns the merging of consecutive draw commands on or off. Off is the default.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] enabled True to merge, false to emit one command per primitive.
* @note With this ON, two commands that follow each other with the SAME clip rectangle and the SAME
*       texture become one command of kind @ref FUI_DRAW_TRIANGLES. A backend that drains the geometry -
*       binds the texture, sets the scissor, draws the indices - makes far fewer draw calls for it,
*       which is what a dense table of hundreds of little rectangles wants.
* @note It is OFF by default because it destroys the fast path payloads. A backend that reads
*       fuiDrawCommand::payload to redraw a rectangle or a string through its own pipeline, rather than
*       drawing the triangles, must NOT turn this on: a merged command carries no payload it can use.
* @note Indices are absolute, so a merged run needs nothing special of a backend that draws them.
* @see @ref fuiGetDrawData
*/
fui_api void fuiSetDrawBatching(fuiContext *context, const bool enabled);

// ****************************************************************************
//
// > Clipping
//
// ****************************************************************************

/**
* @brief Pushes a clip rectangle, intersected with the one already active.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] rect The rectangle to clip to, in pixels.
* @note Nesting can therefore only ever shrink the visible area. Geometry outside the clip is dropped
*       entirely rather than emitted and scissored away.
* @see @ref fuiPopClip
*/
fui_api void fuiPushClip(fuiContext *context, const fuiRect rect);

/**
* @brief Pops the most recently pushed clip rectangle.
* @param[in,out] context Reference to the context @ref fuiContext.
* @see @ref fuiPushClip
*/
fui_api void fuiPopClip(fuiContext *context);

/**
* @brief Returns the clip rectangle currently in effect.
* @param[in] context Reference to the context @ref fuiContext.
* @return Returns the active clip rectangle, which is the whole window when nothing was pushed.
*/
fui_api fuiRect fuiGetClipRect(const fuiContext *context);

// ****************************************************************************
//
// > Interaction
//
// ****************************************************************************

//! What one widget's hit test came back with
typedef struct fuiInteraction {
	//! The cursor is over this widget and nothing is covering it
	bool isHovered;
	//! This widget captured the press and still holds it, even if the cursor has since moved off
	bool isHeld;
	//! The press edge landed on this widget this frame
	bool wasPressed;
	//! The button was released over this widget after being pressed on it, which is a completed click
	bool wasClicked;
} fuiInteraction;

/**
* @brief Combines a label with the current scope into a stable widget identifier.
* @param[in] context Reference to the context @ref fuiContext.
* @param[in] label The label to hash, which does not have to be what the widget displays.
* @return Returns the identifier @ref fuiId, never @ref FUI_ID_NONE.
*/
fui_api fuiId fuiGetId(const fuiContext *context, const char *label);

/**
* @brief Pushes a named scope, so identical labels inside it stay distinct from those outside.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] label The scope name.
* @see @ref fuiPopId
*/
fui_api void fuiPushId(fuiContext *context, const char *label);

/**
* @brief Pushes a numbered scope, for one iteration of a loop over rows or entities.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] value The scope number.
* @see @ref fuiPopId
*/
fui_api void fuiPushIdInt(fuiContext *context, const int32_t value);

/**
* @brief Pops the most recently pushed scope.
* @param[in,out] context Reference to the context @ref fuiContext.
* @see @ref fuiPushId
*/
fui_api void fuiPopId(fuiContext *context);

/**
* @brief Hit tests one widget and advances its press, hold and click state.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id The widget's identifier, from @ref fuiGetId.
* @param[in] rect The widget's bounds in pixels.
* @return Returns what the widget should show and report, see @ref fuiInteraction.
* @note Later calls win the cursor, so a widget built on top of another one takes the hover from it.
*       The result is safe to use during a draw pass, where it reports state without changing any.
*/
fui_api fuiInteraction fuiInteract(fuiContext *context, const fuiId id, const fuiRect rect);

/**
* @brief Claims the cursor for an area that has no widget of its own, such as a panel background.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] rect The area to swallow the cursor in, in pixels.
* @note Without this a click that lands on the empty part of a panel falls through to whatever is behind it.
*/
fui_api void fuiBlockMouse(fuiContext *context, const fuiRect rect);

/**
* @brief Tests whether the user interface is using the cursor this frame.
* @param[in] context Reference to the context @ref fuiContext.
* @return Returns true when the cursor is over a widget, or a widget is still holding a press.
* @note Read this after the interact build to decide whether the cursor means anything to the world behind.
*/
fui_api bool fuiWantsMouse(const fuiContext *context);

/**
* @brief Tests whether the user interface is using the keyboard this frame.
* @param[in] context Reference to the context @ref fuiContext.
* @return Returns true while a widget has keyboard focus, such as a text field being typed into.
*/
fui_api bool fuiWantsKeyboard(const fuiContext *context);

/**
* @brief Returns the widget the cursor is over.
* @param[in] context Reference to the context @ref fuiContext.
* @return Returns the identifier @ref fuiId, or @ref FUI_ID_NONE.
*/
fui_api fuiId fuiGetHotId(const fuiContext *context);

/**
* @brief Returns the widget that captured the press.
* @param[in] context Reference to the context @ref fuiContext.
* @return Returns the identifier @ref fuiId, or @ref FUI_ID_NONE.
*/
fui_api fuiId fuiGetActiveId(const fuiContext *context);

/**
* @brief Returns the widget that owns the keyboard.
* @param[in] context Reference to the context @ref fuiContext.
* @return Returns the identifier @ref fuiId, or @ref FUI_ID_NONE.
*/
fui_api fuiId fuiGetFocusedId(const fuiContext *context);

/**
* @brief Gives the keyboard to a widget, or takes it away.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id The widget to focus, or @ref FUI_ID_NONE to focus nothing.
*/
fui_api void fuiSetFocusedId(fuiContext *context, const fuiId id);

/**
* @brief Returns where the cursor is this frame.
* @param[in] context Reference to the context @ref fuiContext.
* @return Returns the cursor position in pixels.
*/
fui_api fuiVec2 fuiGetMousePosition(const fuiContext *context);

/**
* @brief Returns how far the cursor moved since the previous frame.
* @param[in] context Reference to the context @ref fuiContext.
* @return Returns the movement in pixels, which is zero on the very first frame.
*/
fui_api fuiVec2 fuiGetMouseDelta(const fuiContext *context);

/**
* @brief Returns how far the wheel turned this frame.
* @param[in] context Reference to the context @ref fuiContext.
* @return Returns the wheel delta, positive away from the user.
*/
fui_api float fuiGetMouseWheelDelta(const fuiContext *context);

/**
* @brief Tests whether a key is currently held.
* @param[in] context Reference to the context @ref fuiContext.
* @param[in] key The key @ref fuiKey to test.
* @return Returns true while the key is down.
*/
fui_api bool fuiIsKeyDown(const fuiContext *context, const fuiKey key);

/**
* @brief Tests whether a key was pressed this frame.
* @param[in] context Reference to the context @ref fuiContext.
* @param[in] key The key @ref fuiKey to test.
* @return Returns true on the press edge, and always false during a draw pass.
*/
fui_api bool fuiKeyWentDown(const fuiContext *context, const fuiKey key);

/**
* @brief Tests whether a key fired, counting the repeats of a key that is being held down.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] key The key @ref fuiKey to test.
* @return Returns true on the press itself and then again on every repeat.
* @note This is @ref fuiKeyRepeatEdge over a timer the context owns, which is what an arrow key in a text
*       field wants. A draw pass advances no timer and reports no edge, so a two-pass caller cannot fire twice.
*/
fui_api bool fuiKeyRepeat(fuiContext *context, const fuiKey key);

/**
* @brief Tests whether either control key is held.
* @param[in] context Reference to the context @ref fuiContext.
* @return Returns true while control is down.
* @note Modifiers arrive as separate left and right keys on X11, so asking for one of them is a bug waiting to happen.
*/
fui_api bool fuiIsControlDown(const fuiContext *context);

/**
* @brief Tests whether either shift key is held.
* @param[in] context Reference to the context @ref fuiContext.
* @return Returns true while shift is down.
*/
fui_api bool fuiIsShiftDown(const fuiContext *context);

/**
* @brief Tests whether either alt key is held.
* @param[in] context Reference to the context @ref fuiContext.
* @return Returns true while alt is down.
*/
fui_api bool fuiIsAltDown(const fuiContext *context);

/**
* @brief Returns the codepoints typed this frame.
* @param[in] context Reference to the context @ref fuiContext.
* @param[out] outCount Receives how many codepoints there are.
* @return Returns the codepoints, or null when nothing was typed. Empty during a draw pass.
*/
fui_api const uint32_t *fuiGetTextInput(const fuiContext *context, int32_t *outCount);

// ****************************************************************************
//
// > Drawing
//
// ****************************************************************************

/**
* @brief Draws a filled rectangle.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] rect The rectangle to fill, in pixels.
* @param[in] color The fill color.
*/
fui_api void fuiDrawRect(fuiContext *context, const fuiRect rect, const fuiColor color);

/**
* @brief Draws a rectangle filled with a vertical gradient, running from its top edge to its bottom one.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] rect The rectangle to fill, in pixels.
* @param[in] topColor The color along the top edge.
* @param[in] bottomColor The color along the bottom edge.
* @note Two identical stops are drawn as a plain @ref FUI_DRAW_RECT, so a flat theme keeps the fast path a
*       backend can shortcut. A real gradient goes out as @ref FUI_DRAW_TRIANGLES, because a per-corner color
*       is something no rectangle payload can carry.
*/
fui_api void fuiDrawRectVerticalGradient(fuiContext *context, const fuiRect rect, const fuiColor topColor, const fuiColor bottomColor);

/**
* @brief Draws a rectangle outline, with the stroke lying inside the given bounds.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] rect The rectangle to outline, in pixels.
* @param[in] color The stroke color.
* @param[in] thickness The stroke width in pixels.
*/
fui_api void fuiDrawRectOutline(fuiContext *context, const fuiRect rect, const fuiColor color, const float thickness);

/**
* @brief Draws a straight line segment.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] start Where the line starts, in pixels.
* @param[in] end Where the line ends, in pixels.
* @param[in] color The stroke color.
* @param[in] thickness The stroke width in pixels.
*/
fui_api void fuiDrawLine(fuiContext *context, const fuiVec2 start, const fuiVec2 end, const fuiColor color, const float thickness);

/**
* @brief Draws a filled convex polygon.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] points The corner points in pixels, in any winding order.
* @param[in] pointCount How many corner points there are, fewer than three draws nothing.
* @param[in] color The fill color.
* @note The polygon has to be convex - it is filled as a triangle fan around its first point.
*/
fui_api void fuiDrawPolygon(fuiContext *context, const fuiVec2 *points, const uint32_t pointCount, const fuiColor color);

/**
* @brief Draws the outline of a closed polygon.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] points The corner points in pixels.
* @param[in] pointCount How many corner points there are, fewer than two draws nothing.
* @param[in] color The stroke color.
* @param[in] thickness The stroke width in pixels.
*/
fui_api void fuiDrawPolygonOutline(fuiContext *context, const fuiVec2 *points, const uint32_t pointCount, const fuiColor color, const float thickness);

/**
* @brief Draws a textured quad.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] rect Where the image goes, in pixels.
* @param[in] texture The texture to sample.
* @param[in] uvMin Top-left texture coordinate.
* @param[in] uvMax Bottom-right texture coordinate.
* @param[in] color Tint multiplied onto the texture, use white to leave it unchanged.
*/
fui_api void fuiDrawImage(fuiContext *context, const fuiRect rect, const fuiTextureId texture, const fuiVec2 uvMin, const fuiVec2 uvMax, const fuiColor color);

/**
* @brief Draws a textured quad, mirrored or turned a quarter turn.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] rect Where the image goes, in pixels. Already the final shape, the flags do not reshape it.
* @param[in] texture The texture to sample.
* @param[in] uvMin Top-left texture coordinate, before the flags are applied.
* @param[in] uvMax Bottom-right texture coordinate, before the flags are applied.
* @param[in] color Tint multiplied onto the texture, use white to leave it unchanged.
* @param[in] flags How the sampled part is turned onto the quad @ref fuiImageFlags.
* @note The emitted vertices already carry the turn, so a backend drawing triangles needs to know nothing
*       about the flags. They ride along in the payload only for a backend taking the sprite fast path.
* @see @ref fuiDrawImage
*/
fui_api void fuiDrawImageEx(fuiContext *context, const fuiRect rect, const fuiTextureId texture, const fuiVec2 uvMin, const fuiVec2 uvMax, const fuiColor color, const fuiImageFlags flags);

/**
* @brief Draws one line of text.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] text The string to draw, does not have to be zero terminated.
* @param[in] textLength Length of the string in bytes, pass 0 to measure it up to the terminating zero.
* @param[in] position Top-left corner of the text box, in pixels.
* @param[in] pixelHeight Height one em is drawn at, in pixels.
* @param[in] color The text color.
* @note A newline is NOT a glyph and is not broken on here, see @ref fuiBreakTextLines for multiple lines.
*/
fui_api void fuiDrawText(fuiContext *context, const char *text, const size_t textLength, const fuiVec2 position, const float pixelHeight, const fuiColor color);

// ****************************************************************************
//
// > Text
//
// ****************************************************************************

//! One visual line of a broken text block, pointing INTO the caller's own buffer without copying it
typedef struct fuiTextLine {
	//! First byte of the line, excluding any leading break
	const char *start;
	//! Length of the line in bytes, excluding any trailing newline
	size_t length;
} fuiTextLine;

/**
* @brief Decodes one UTF-8 encoded codepoint.
* @param[in] text The string to read from.
* @param[in] textLength Length of the string in bytes.
* @param[in,out] inOutOffset Byte offset to read at, advanced past the decoded codepoint.
* @return Returns the decoded codepoint, or 0xFFFD for a malformed sequence.
* @note A malformed sequence still advances the offset by one byte, so a decoding loop always terminates.
*/
fui_api uint32_t fuiDecodeUtf8(const char *text, const size_t textLength, size_t *inOutOffset);

/**
* @brief Encodes one codepoint as UTF-8.
* @param[in] codePoint The codepoint to encode.
* @param[out] outBytes Receives the encoded bytes, which needs room for four.
* @return Returns how many bytes were written, or 0 when the codepoint cannot be encoded.
* @note The counterpart of @ref fuiDecodeUtf8, and what a text field inserts a typed character with.
*/
fui_api uint32_t fuiEncodeUtf8(const uint32_t codePoint, char *outBytes);

/**
* @brief Measures one line of text.
* @param[in] context Reference to the context @ref fuiContext.
* @param[in] text The string to measure.
* @param[in] textLength Length of the string in bytes, pass 0 to measure up to the terminating zero.
* @param[in] pixelHeight Height one em would be drawn at, in pixels.
* @return Returns the width and height of the text in pixels, or a zero size without a font.
*/
fui_api fuiVec2 fuiMeasureText(const fuiContext *context, const char *text, const size_t textLength, const float pixelHeight);

/**
* @brief Returns the distance between two baselines.
* @param[in] context Reference to the context @ref fuiContext.
* @param[in] pixelHeight Height one em would be drawn at, in pixels.
* @return Returns the line height in pixels.
*/
fui_api float fuiGetLineHeight(const fuiContext *context, const float pixelHeight);

/**
* @brief Shortens text with a trailing ellipsis until it fits a width.
* @param[in] context Reference to the context @ref fuiContext.
* @param[in] text The string to shorten.
* @param[in] textLength Length of the string in bytes, pass 0 to measure up to the terminating zero.
* @param[in] maxWidth The width to fit into, in pixels.
* @param[in] pixelHeight Height one em would be drawn at, in pixels.
* @param[out] buffer Receives the shortened, zero terminated string.
* @param[in] bufferCapacity Size of the buffer in bytes.
* @return Returns true when the text had to be shortened.
* @note Cuts on a codepoint boundary, so a multi-byte character is never split in half.
*/
fui_api bool fuiTruncateTextToWidth(const fuiContext *context, const char *text, const size_t textLength, const float maxWidth, const float pixelHeight, char *buffer, const size_t bufferCapacity);

/**
* @brief Breaks text into visual lines on newlines and, optionally, on word wrap.
* @param[in] context Reference to the context @ref fuiContext.
* @param[in] text The string to break.
* @param[in] textLength Length of the string in bytes, pass 0 to measure up to the terminating zero.
* @param[in] pixelHeight Height one em would be drawn at, in pixels.
* @param[in] wordWrap Set to true to also wrap long lines at a space so they fit wrapWidth.
* @param[in] wrapWidth The width to wrap at, in pixels, ignored when wordWrap is false.
* @param[out] outLines Receives the visual lines, each pointing into the caller's own text buffer.
* @param[in] maxLines How many lines the output array holds.
* @return Returns the number of lines written, which is at least one even for empty text.
* @note The lines point INTO text and copy nothing, so text has to outlive them.
*/
fui_api uint32_t fuiBreakTextLines(const fuiContext *context, const char *text, const size_t textLength, const float pixelHeight, const bool wordWrap, const float wrapWidth, fuiTextLine *outLines, const uint32_t maxLines);

/**
* @brief Breaks a WINDOW of a text's visual lines out of it, and reports how many lines it has in total.
* @param[in] context Reference to the context @ref fuiContext.
* @param[in] text The text to break into lines.
* @param[in] textLength Length of the text in bytes, or 0 to measure it.
* @param[in] pixelHeight Height one em is drawn at.
* @param[in] wordWrap Whether a line too wide for wrapWidth is broken at a space.
* @param[in] wrapWidth Width to wrap at in pixels, ignored when wordWrap is false.
* @param[in] firstLine Which visual line the output starts at.
* @param[out] outLines Receives up to maxLines lines, the first of them being line firstLine.
* @param[in] maxLines Capacity of outLines.
* @param[out] outTotalLineCount Receives how many lines the whole text has, or null to stop as soon as the
*             window is full - which is far cheaper, and all a caller showing the top of a text needs.
* @return Returns the number of lines written to outLines.
* @note This is what lets a text field scroll through a document instead of showing only its first
*       FUI_MAX_TEXT_LINES lines. Counting the total walks the whole text, so pass null when you can.
* @see @ref fuiBreakTextLines
*/
fui_api uint32_t fuiBreakTextLinesFrom(const fuiContext *context, const char *text, const size_t textLength, const float pixelHeight, const bool wordWrap, const float wrapWidth, const uint32_t firstLine, fuiTextLine *outLines, const uint32_t maxLines, uint32_t *outTotalLineCount);

/**
* @brief Measures a whole block of text, breaking it the same way @ref fuiBreakTextLines does.
* @param[in] context Reference to the context @ref fuiContext.
* @param[in] text The string to measure.
* @param[in] textLength Length of the string in bytes, pass 0 to measure up to the terminating zero.
* @param[in] pixelHeight Height one em would be drawn at, in pixels.
* @param[in] wordWrap Set to true to also wrap long lines at a space.
* @param[in] wrapWidth The width to wrap at, in pixels, ignored when wordWrap is false.
* @return Returns the width of the widest line and the stacked height of every line, in pixels.
*/
fui_api fuiVec2 fuiMeasureTextBlock(const fuiContext *context, const char *text, const size_t textLength, const float pixelHeight, const bool wordWrap, const float wrapWidth);

/**
* @brief Draws a whole block of text, one visual line under the other.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] text The string to draw.
* @param[in] textLength Length of the string in bytes, pass 0 to measure up to the terminating zero.
* @param[in] position Top-left corner of the block, in pixels.
* @param[in] pixelHeight Height one em is drawn at, in pixels.
* @param[in] color The text color.
* @param[in] wordWrap Set to true to also wrap long lines at a space.
* @param[in] wrapWidth The width to wrap at, in pixels, ignored when wordWrap is false.
* @note A single line block emits exactly what @ref fuiDrawText would, so the two cannot drift apart.
*/
fui_api void fuiDrawTextBlock(fuiContext *context, const char *text, const size_t textLength, const fuiVec2 position, const float pixelHeight, const fuiColor color, const bool wordWrap, const float wrapWidth);

// ****************************************************************************
//
// > Chrome
//
// ****************************************************************************

/**
* @brief Draws the diagonal ticks that mark a corner as a resize handle.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] grip The square the ticks are drawn into, in pixels.
* @param[in] color The tick color, muted while idle and the accent color while hovered by convention.
* @param[in] thickness Stroke width of one tick, in pixels.
* @note Panels draw this for themselves. It is public so a widget that lays its own frame out marks its
*       corner with the SAME glyph instead of inventing a second one.
*/
fui_api void fuiDrawResizeGrip(fuiContext *context, const fuiRect grip, const fuiColor color, const float thickness);

/**
* @brief Returns the side of the square box a title bar icon is drawn into, in pixels.
* @param[in] context Reference to the context @ref fuiContext.
* @return Returns the icon size, derived from the theme's font height.
* @note Both title bar marks fill the box they are given, so handing each one a box of this size is what
*       makes them come out matched.
*/
fui_api float fuiTitleBarIconSize(const fuiContext *context);

/**
* @brief Draws the disclosure triangle of a title bar.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] glyphBox The square the triangle fills exactly, in pixels.
* @param[in] isCollapsed Set to true to point it right, at folded away content, instead of down.
* @param[in] color The triangle color.
* @note Drawn rather than typed, so it is the size the caller asks for and not whatever the font makes of a character.
*/
fui_api void fuiDrawCollapseGlyph(fuiContext *context, const fuiRect glyphBox, const bool isCollapsed, const fuiColor color);

/**
* @brief Draws the close cross of a title bar.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] glyphBox The square the cross spans corner to corner, in pixels.
* @param[in] color The stroke color.
* @param[in] thickness Stroke width, in pixels.
*/
fui_api void fuiDrawCloseGlyph(fuiContext *context, const fuiRect glyphBox, const fuiColor color, const float thickness);

// ****************************************************************************
//
// > Tooltip
//
// ****************************************************************************

/**
* @brief Asks for a hover tooltip over an area, shown once the cursor has rested on it.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] hoverRect The area the cursor has to be inside, in pixels.
* @param[in] text What to say, where a newline starts another line of the box.
* @note Call it right after building the widget the tooltip belongs to, while its rectangle is at hand.
*       Nothing is drawn here: the box is drawn by @ref fuiEndFrame, after every panel and popup, so
*       nothing can clip it or sit on top of it. The rest timer belongs to the TEXT rather than the area,
*       so sliding between two rows that say different things starts the wait over while sliding between
*       two that say the same thing keeps it counting.
* @note The area is also tested against the current clip, so a row scrolled out of its panel does not
*       raise a tooltip for the hole it left behind.
*/
fui_api void fuiTooltip(fuiContext *context, const fuiRect hoverRect, const char *text);

// ****************************************************************************
//
// > Layout
//
// ****************************************************************************

/**
* @brief Begins a panel: a titled box with a background, a border and a vertical flow inside it.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id Identifies the panel and doubles as the caption on its title bar.
* @param[in] dock Where to place it inside the enclosing container, see @ref fuiDock.
* @param[in] x Left edge in pixels, used only when dock is @ref FUI_DOCK_NONE.
* @param[in] y Top edge in pixels, used only when dock is @ref FUI_DOCK_NONE.
* @param[in] width Width in pixels for a left or right dock, or for a floating panel.
* @param[in] height Height in pixels for a top or bottom dock, or for a floating panel.
* @return Returns false when the panel is folded away, so the caller may skip building hidden content.
* @note ALWAYS pair this with @ref fuiEndPanel, including when it returned false. A floating panel is moved
*       by its title bar and resized by its corner, and both are remembered under `id` across frames.
* @note The rectangle is resolved HERE and the drag is handled further down, so under @ref FUI_PASS_BOTH a
*       panel being dragged trails the cursor by one frame. A two pass caller sees none of that, because the
*       draw pass re-reads what the interact pass already applied.
* @see @ref fuiEndPanel
*/
fui_api bool fuiBeginPanel(fuiContext *context, const char *id, const fuiDock dock, const float x, const float y, const float width, const float height);

/**
* @brief Begins a panel whose content SCROLLS when it does not fit, instead of being clipped away.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id Identifies the panel and doubles as the caption on its title bar.
* @param[in] dock Where to place it inside the enclosing container, see @ref fuiDock.
* @param[in] x Left edge in pixels, used only when dock is @ref FUI_DOCK_NONE.
* @param[in] y Top edge in pixels, used only when dock is @ref FUI_DOCK_NONE.
* @param[in] width Width in pixels for a left or right dock, or for a floating panel.
* @param[in] height Height in pixels for a top or bottom dock, or for a floating panel.
* @return Returns false when the panel is folded away.
* @note A scrollbar always sits in a reserved gutter on the right, with its thumb disabled while the content
*       fits, so the panel does not change width the moment one row is added. Closed by @ref fuiEndPanel.
* @note The content is laid out HERE from the scroll @ref fuiEndPanel resolves, so under @ref FUI_PASS_BOTH
*       a wheel notch lands one frame later. A two pass caller sees none of that.
*/
fui_api bool fuiBeginScrollPanel(fuiContext *context, const char *id, const fuiDock dock, const float x, const float y, const float width, const float height);

/**
* @brief Begins a panel the user can dismiss, which grows a close button at the right of its title bar.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id Identifies the panel and doubles as the caption on its title bar.
* @param[in] dock Where to place it inside the enclosing container, see @ref fuiDock.
* @param[in] x Left edge in pixels, used only when dock is @ref FUI_DOCK_NONE.
* @param[in] y Top edge in pixels, used only when dock is @ref FUI_DOCK_NONE.
* @param[in] width Width in pixels for a left or right dock, or for a floating panel.
* @param[in] height Height in pixels for a top or bottom dock, or for a floating panel.
* @param[in,out] isOpen Reference to the caller's own visibility flag, cleared when the button is clicked. Pass null for no button.
* @return Returns false when the panel is folded away.
* @note The flag is cleared DURING the build, so this panel still finishes the frame and is simply not built by the caller on the next one.
*/
fui_api bool fuiBeginPanelClosable(fuiContext *context, const char *id, const fuiDock dock, const float x, const float y, const float width, const float height, bool *isOpen);

/**
* @brief Begins a scrolling panel the user can dismiss. @ref fuiBeginScrollPanel and @ref fuiBeginPanelClosable in one.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id Identifies the panel and doubles as the caption on its title bar.
* @param[in] dock Where to place it inside the enclosing container, see @ref fuiDock.
* @param[in] x Left edge in pixels, used only when dock is @ref FUI_DOCK_NONE.
* @param[in] y Top edge in pixels, used only when dock is @ref FUI_DOCK_NONE.
* @param[in] width Width in pixels for a left or right dock, or for a floating panel.
* @param[in] height Height in pixels for a top or bottom dock, or for a floating panel.
* @param[in,out] isOpen Reference to the caller's own visibility flag, cleared when the button is clicked. Pass null for no button.
* @return Returns false when the panel is folded away.
*/
fui_api bool fuiBeginScrollPanelClosable(fuiContext *context, const char *id, const fuiDock dock, const float x, const float y, const float width, const float height, bool *isOpen);

/**
* @brief Closes the panel that is open, drawing its scrollbar when it has one.
* @param[in,out] context Reference to the context @ref fuiContext.
* @see @ref fuiBeginPanel
*/
fui_api void fuiEndPanel(fuiContext *context);

//! Pass this as a flow container's spacing to take the theme's @ref fuiTheme.widgetSpacing, which is what keeps one stack looking like the panel it sits in. Any value of zero or more is used as given, a hard zero included
#define FUI_SPACING_FROM_THEME (-1.0f)

/**
* @brief Begins a plain flow container, with no background, border or clip of its own.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id Identifies the container and scopes the identifiers of everything inside it.
* @param[in] axis Direction its slot children flow in, see @ref fuiAxis.
* @param[in] dock Which edge of the enclosing container to bite a strip off, see @ref fuiDock.
* @param[in] sizeAcross Thickness of that strip in pixels, ignored for @ref FUI_DOCK_FILL.
* @param[in] spacing Trailing margin every slot carries, in pixels, or @ref FUI_SPACING_FROM_THEME for the theme's.
* @note This is how a row of buttons is built inside a column of rows. ALWAYS pair with @ref fuiEndStack.
* @see @ref fuiEndStack
*/
fui_api void fuiBeginStack(fuiContext *context, const char *id, const fuiAxis axis, const fuiDock dock, const float sizeAcross, const float spacing);

/**
* @brief Begins a flow container over an explicit rectangle, taking nothing away from the enclosing container.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id Identifies the container and scopes the identifiers of everything inside it.
* @param[in] axis Direction its slot children flow in, see @ref fuiAxis.
* @param[in] rect The region to lay out in, in pixels.
* @param[in] spacing Trailing margin every slot carries, in pixels, or @ref FUI_SPACING_FROM_THEME for the theme's.
* @note For a region the caller places itself and still wants slot flow inside. Closed by @ref fuiEndStack.
*/
fui_api void fuiBeginStackAt(fuiContext *context, const char *id, const fuiAxis axis, const fuiRect rect, const float spacing);

/**
* @brief Begins a flow container over an explicit rectangle whose content SCROLLS when it overflows.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id Identifies the container and scopes the identifiers of everything inside it.
* @param[in] rect The region to lay out in, in pixels.
* @param[in] spacing Trailing margin every slot carries, in pixels, or @ref FUI_SPACING_FROM_THEME for the theme's.
* @note Vertical flow only, since there is nothing to scroll along the other axis. Closed by @ref fuiEndStack.
*/
fui_api void fuiBeginScrollStackAt(fuiContext *context, const char *id, const fuiRect rect, const float spacing);

/**
* @brief Closes the flow container that is open, drawing its scrollbar when it has one.
* @param[in,out] context Reference to the context @ref fuiContext.
* @see @ref fuiBeginStack
*/
fui_api void fuiEndStack(fuiContext *context);

/**
* @brief Hands out the next slot of the container that is open and advances its flow cursor.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] thickness Length of the slot along the flow axis, in pixels.
* @return Returns the slot rectangle, spanning the container across the other axis. Empty when there is no room left.
* @note Feed the result straight to a widget. The container's spacing is a TRAILING margin, taken under a slot in a vertical flow and right of one in a horizontal flow, so the first slot still starts flush against the container's leading edge.
*/
fui_api fuiRect fuiLayoutSlot(fuiContext *context, const float thickness);

/**
* @brief Bites a strip off ONE EDGE of the container that is open and hands it back.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] dock Which edge to take the strip off, see @ref fuiDock.
* @param[in] thickness How deep to cut into the container, in pixels.
* @return Returns the strip, spanning the container across the other axis. Empty when there is no container or no room left.
* @note This is how a bar gets its rectangle without knowing the window: the WIDTH comes from whatever encloses it and the thickness from the theme, so @ref fuiMenuBarHeight, @ref fuiToolStripThickness and @ref fuiStatusBarHeight are all a caller ever has to name.
* @note The strip leaves the container's flow alone, unlike @ref fuiLayoutSlot: what is docked is chrome around the content rather than one more row of it, and @ref fuiLayoutRemaining answers with the region that is left.
*/
fui_api fuiRect fuiLayoutDock(fuiContext *context, const fuiDock dock, const float thickness);

/**
* @brief Returns what the container that is open has left, in pixels.
* @param[in] context Reference to the context @ref fuiContext.
* @return Returns the remaining content box, or the whole window when no container is open.
* @note Use it to size a filling child by hand, or as the region for a custom drawing.
*/
fui_api fuiRect fuiLayoutRemaining(const fuiContext *context);

/**
* @brief Begins a titled section, which emits its label as a slot and opens an identifier scope.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] label The heading, which also scopes the identifiers of the rows under it.
* @note So a "Radius" row in two different sections stays two different widgets. ALWAYS pair with @ref fuiEndGroup.
* @see @ref fuiEndGroup
*/
fui_api void fuiBeginGroup(fuiContext *context, const char *label);

/**
* @brief Closes a titled section by emitting a separator slot under its rows.
* @param[in,out] context Reference to the context @ref fuiContext.
* @see @ref fuiBeginGroup
*/
fui_api void fuiEndGroup(fuiContext *context);

/**
* @brief Begins a FRAMED titled section of a fixed height, with a border and a filled title bar.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] label The heading, which also scopes the identifiers of the rows inside the frame.
* @param[in] contentHeight Height of the rows inside the frame INCLUDING the gaps between them, in pixels.
* @note Ask @ref fuiGroupBoxContentHeight for that height rather than tallying it by hand. ALWAYS pair with @ref fuiEndGroupBox.
* @see @ref fuiBeginGroupBoxAuto
*/
fui_api void fuiBeginGroupBox(fuiContext *context, const char *label, const float contentHeight);

/**
* @brief Begins a framed titled section sized to WHATEVER the caller puts in it.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] label The heading, which also scopes the identifiers of the rows inside the frame.
* @note @ref fuiEndGroupBox measures what the rows actually consumed and remembers it, so the next build lays
*       out at the right height. That removes a hand tallied row count, which is a number that rots the moment
*       a row is added or made conditional. The one imperfect frame is the very first the box appears in.
* @see @ref fuiBeginGroupBox
*/
fui_api void fuiBeginGroupBoxAuto(fuiContext *context, const char *label);

/**
* @brief Closes a framed titled section and remembers what its rows consumed.
* @param[in,out] context Reference to the context @ref fuiContext.
* @see @ref fuiBeginGroupBox
*/
fui_api void fuiEndGroupBox(fuiContext *context);

/**
* @brief Returns how tall a run of uniform rows is, gaps included.
* @param[in] context Reference to the context @ref fuiContext.
* @param[in] rowCount How many rows there are.
* @param[in] rowHeight Height of one row, in pixels.
* @return Returns the total height in pixels, which is what @ref fuiBeginGroupBox wants.
*/
fui_api float fuiGroupBoxContentHeight(const fuiContext *context, const uint32_t rowCount, const float rowHeight);

/**
* @brief Returns the whole slot a framed section of uniform rows occupies in its parent.
* @param[in] context Reference to the context @ref fuiContext.
* @param[in] rowCount How many rows there are.
* @param[in] rowHeight Height of one row, in pixels.
* @return Returns the total height in pixels, title bar and frame padding included.
*/
fui_api float fuiGroupBoxHeight(const fuiContext *context, const uint32_t rowCount, const float rowHeight);

/**
* @brief Returns how much height a panel spends on its own chrome.
* @param[in] context Reference to the context @ref fuiContext.
* @return Returns the title bar plus the top and bottom content padding, in pixels.
* @note So a caller sizing a panel from its rows adds this instead of guessing a constant.
*/
fui_api float fuiPanelChromeHeight(const fuiContext *context);

/**
* @brief A grabbable vertical bar that resizes a WIDTH the caller owns.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id Identifies the drag across frames.
* @param[in] grip The thin strip that can be grabbed, in pixels.
* @param[in,out] width Reference to the width to change, dragging right grows it.
* @param[in] minSize Smallest the width may become, in pixels.
* @param[in] maxSize Largest the width may become, in pixels.
* @param[in] highlightThickness Width of the accent line drawn on the grip while hovered or dragged, in pixels.
* @return Returns true while it is being dragged.
* @note For a region the caller draws itself. A panel has its own resize grip and does not need this.
*/
fui_api bool fuiVerticalSplitter(fuiContext *context, const char *id, const fuiRect grip, float *width, const float minSize, const float maxSize, const float highlightThickness);

/**
* @brief A grabbable horizontal bar that resizes a HEIGHT the caller owns.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id Identifies the drag across frames.
* @param[in] grip The thin strip that can be grabbed, in pixels.
* @param[in,out] height Reference to the height to change, dragging down grows it.
* @param[in] minSize Smallest the height may become, in pixels.
* @param[in] maxSize Largest the height may become, in pixels.
* @param[in] highlightThickness Height of the accent line drawn on the grip while hovered or dragged, in pixels.
* @return Returns true while it is being dragged.
* @note Negate the delta yourself for a bottom anchored region, where dragging down has to shrink it.
*/
fui_api bool fuiHorizontalSplitter(fuiContext *context, const char *id, const fuiRect grip, float *height, const float minSize, const float maxSize, const float highlightThickness);

/**
* @brief A grabbable area that accumulates its drag into an offset the caller owns, and draws nothing.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id Identifies the drag across frames.
* @param[in] handle The area that can be grabbed, in pixels.
* @param[in,out] offset Reference to the offset the motion is added to.
* @return Returns true while it is being dragged.
* @note The move by its title bar gesture, for a floating widget the caller draws itself.
*/
fui_api bool fuiDragHandle(fuiContext *context, const char *id, const fuiRect handle, fuiVec2 *offset);

/**
* @brief An always visible vertical scrollbar.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] track The gutter to draw it in, in pixels.
* @param[in] id Identifies the thumb across frames.
* @param[in] scroll The current scroll offset in pixels.
* @param[in] viewportLength How much of the content is visible, in pixels.
* @param[in] contentLength How long the content is in total, in pixels.
* @return Returns the new scroll offset, clamped to what there is to scroll.
* @note The thumb is interactive while the content overflows and drawn disabled while it fits, so the
*       layout does not jump the moment one row is added. Wheel handling stays with the caller.
*/
fui_api float fuiScrollbarVertical(fuiContext *context, const fuiRect track, const char *id, const float scroll, const float viewportLength, const float contentLength);

/**
* @brief Returns how wide a gutter every scrolling container reserves for its scrollbar, in pixels.
* @return Returns the gutter width.
* @note Public so a caller laying a fixed header over a scrolling table insets it by exactly what the rows below lose.
* @note A scroll panel and a scroll stack take a further @ref fuiTheme.panelPaddingX beside the gutter, so their content never runs flush into the bar.
*/
fui_api float fuiScrollGutterWidth(void);

// ****************************************************************************
//
// > Widgets
//
// ****************************************************************************

/**
* @brief Static text, left aligned and vertically centered in a rectangle.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] rect The area to draw in, in pixels.
* @param[in] text The caption.
* @note Clipped to the rectangle, so a caption too long for its slot is cropped there instead of spilling into the next one.
*/
fui_api void fuiLabel(fuiContext *context, const fuiRect rect, const char *text);

/**
* @brief Static text with explicit control over line breaking.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] rect The area to draw in, in pixels.
* @param[in] text The caption.
* @param[in] multiline Set to true to honour line breaks and anchor the rows to the TOP of the rectangle.
* @param[in] wordWrap Set to true to additionally wrap long lines to the content width, ignored unless multiline.
*/
fui_api void fuiLabelEx(fuiContext *context, const fuiRect rect, const char *text, const bool multiline, const bool wordWrap);

/**
* @brief A thin divider line, centered vertically in a rectangle.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] rect The area to draw in, in pixels.
*/
fui_api void fuiSeparator(fuiContext *context, const fuiRect rect);

/**
* @brief A clickable button.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] rect The area to draw in, in pixels.
* @param[in] label The caption, which also identifies the button.
* @return Returns true on the frame the click completes, meaning pressed AND released over it.
* @note Releasing away from the button cancels the click, which is what lets a user change their mind.
*/
fui_api bool fuiButton(fuiContext *context, const fuiRect rect, const char *label);

/**
* @brief A clickable button that can be disabled.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] rect The area to draw in, in pixels.
* @param[in] label The caption, which also identifies the button.
* @param[in] enabled Set to false to draw it muted and ignore all input, so it can never click.
* @return Returns true on the frame the click completes.
*/
fui_api bool fuiButtonEx(fuiContext *context, const fuiRect rect, const char *label, const bool enabled);

/**
* @brief A button that keeps firing while it is held down.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] rect The area to draw in, in pixels.
* @param[in] label The caption, which also identifies the button.
* @return Returns true on the press and then again on the repeat cadence, so a plain click still yields exactly one hit.
* @note For a stepper whose action is meant to be applied over and over, such as a nudge or a spinner.
*       Everything a user should not be able to hold down stays on @ref fuiButton.
*/
fui_api bool fuiButtonRepeat(fuiContext *context, const fuiRect rect, const char *label);

/**
* @brief A clickable area that draws NOTHING, for content the caller paints itself.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] rect The area to hit test, in pixels.
* @param[in] id Identifies the cell, scope it with @ref fuiPushIdInt inside a loop.
* @param[out] outIsHovered Set to whether the cursor is over the cell, may be null.
* @return Returns true on the frame the click completes.
* @note The cell of an icon or swatch grid: the caller draws the content and its highlight.
*/
fui_api bool fuiCell(fuiContext *context, const fuiRect rect, const char *id, bool *outIsHovered);

/**
* @brief A checkbox bound to a boolean.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] rect The area to draw in, in pixels.
* @param[in] label The caption, which also identifies the checkbox.
* @param[in,out] value Reference to the boolean to toggle.
* @return Returns true on the frame the value changed.
*/
fui_api bool fuiCheckbox(fuiContext *context, const fuiRect rect, const char *label, bool *value);

/**
* @brief One option of a radio group, bound to the shared selection.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] rect The area to draw in, in pixels.
* @param[in] label The caption, which also identifies this option.
* @param[in,out] selected Reference to the shared selection every option of the group writes.
* @param[in] option The value this option stands for.
* @return Returns true on the frame the selection changed.
*/
fui_api bool fuiRadio(fuiContext *context, const fuiRect rect, const char *label, int32_t *selected, const int32_t option);

/**
* @brief A horizontal slider bound to a float.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] rect The area to draw in, in pixels.
* @param[in] id Identifies the slider, which carries no caption of its own.
* @param[in,out] value Reference to the value to drag, clamped to the range.
* @param[in] minValue Lowest value, at the left end of the track.
* @param[in] maxValue Highest value, at the right end of the track.
* @return Returns true on the frame the value changed.
* @note Pressing anywhere on the track jumps the knob there, rather than only the knob being grabbable.
*/
fui_api bool fuiSliderFloat(fuiContext *context, const fuiRect rect, const char *id, float *value, const float minValue, const float maxValue);

/**
* @brief A horizontal slider with quantization, a caption, a disabled state and a drag lifecycle.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] rect The area to draw in, in pixels.
* @param[in] id Identifies the slider.
* @param[in,out] value Reference to the value to drag, clamped to the range.
* @param[in] minValue Lowest value, at the left end of the track.
* @param[in] maxValue Highest value, at the right end of the track.
* @param[in] step Increment to snap the value to, pass 0 for a continuous one.
* @param[in] liveUpdate Set to true to report every change during the drag, false to report once on release.
* @param[in] enabled Set to false to draw it muted and ignore all input.
* @param[in] displayText Caption drawn over the track, may be null.
* @param[out] outDidBegin Set to true on the frame a drag starts, may be null.
* @param[out] outDidEnd Set to true on the frame a drag ends, may be null.
* @return Returns true on the frame the caller should apply the value.
* @note A deferred slider still tracks the cursor, so the knob and the caption follow it during the drag -
*       only the caller's apply waits for the release, which is what makes one drag ONE undoable edit.
*/
fui_api bool fuiSliderFloatEx(fuiContext *context, const fuiRect rect, const char *id, float *value, const float minValue, const float maxValue, const float step, const bool liveUpdate, const bool enabled, const char *displayText, bool *outDidBegin, bool *outDidEnd);

/**
* @brief A drag field: horizontal cursor motion changes the value, and the value is printed on it.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] rect The area to draw in, in pixels.
* @param[in] id Identifies the field.
* @param[in,out] value Reference to the value to change, clamped to the range.
* @param[in] speed How much one pixel of motion changes the value by.
* @param[in] minValue Lowest value.
* @param[in] maxValue Highest value.
* @return Returns true on the frame the value changed.
*/
fui_api bool fuiDragFloat(fuiContext *context, const fuiRect rect, const char *id, float *value, const float speed, const float minValue, const float maxValue);

/**
* @brief An editable single line text field.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] rect The area to draw in, in pixels.
* @param[in] id Identifies the field, which carries no caption of its own.
* @param[in,out] buffer Reference to the zero terminated text to edit, written in place.
* @param[in] capacity Size of the buffer in bytes, INCLUDING the terminating zero.
* @return Returns true on the frame the buffer changed.
* @note Focused by a click or by tab. Enter and escape give the focus back up.
* @see @ref fuiTextInputEx
*/
fui_api bool fuiTextInput(fuiContext *context, const fuiRect rect, const char *id, char *buffer, const int32_t capacity);

/**
* @brief An editable text field with explicit control over line breaking.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] rect The area to draw in, in pixels.
* @param[in] id Identifies the field.
* @param[in,out] buffer Reference to the zero terminated text to edit, written in place.
* @param[in] capacity Size of the buffer in bytes, INCLUDING the terminating zero.
* @param[in] multiline Set to true for a text AREA: enter inserts a line break, up and down walk the rows,
*            home and end act per row, and the rows are anchored to the TOP of the rectangle.
* @param[in] wordWrap Set to true to additionally wrap long rows to the field's content width, ignored unless multiline.
* @return Returns true on the frame the buffer changed.
* @note A multi-line field swallows the plain enter it consumed, so a dialog hosting one does not also
*       commit on it. Ctrl and enter together insert nothing and leave the edge for the dialog to commit on.
* @note Editing is codepoint aware throughout: the caret only ever lands on a character boundary, so a
*       multi-byte character is never cut in half by an arrow key or a backspace.
*/
fui_api bool fuiTextInputEx(fuiContext *context, const fuiRect rect, const char *id, char *buffer, const int32_t capacity, const bool multiline, const bool wordWrap);

/**
* @brief Converts a hue, saturation and value triple into red, green and blue.
* @param[in] hsv The color as hue, saturation and value, every component in 0 to 1.
* @return Returns the color as red, green and blue, every component in 0 to 1.
* @note The hue is in 0 to 1 rather than degrees: 0 is red, 1/3 is green, 2/3 is blue, and 1 wraps back to red.
*/
fui_api fuiVec3 fuiHsvToRgb(const fuiVec3 hsv);

/**
* @brief Converts a red, green and blue triple into hue, saturation and value.
* @param[in] rgb The color as red, green and blue, every component in 0 to 1.
* @return Returns the color as hue, saturation and value, every component in 0 to 1.
* @note A gray or a black has no hue to recover, and comes back with a hue of zero.
*/
fui_api fuiVec3 fuiRgbToHsv(const fuiVec3 rgb);

/**
* @brief A color picker: a saturation and value square, a hue strip, numeric channels and an optional alpha band.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] rect The area to draw in, in pixels.
* @param[in] id Identifies the picker.
* @param[in,out] color Reference to the color to edit, written as it is dragged so the caller sees it live.
* @param[in] hasAlpha Set to true to add the alpha band, false to leave the color's alpha untouched.
* @param[in] enabled Set to false to draw it without any interaction at all.
* @param[out] outDidBegin Set to true on the frame a drag starts, may be null.
* @param[out] outDidEnd Set to true on the frame a drag ends, may be null.
* @return Returns true on the frame the color changed.
* @note The lifecycle is reported the way @ref fuiSliderFloatEx reports it, so "commit the edit as ONE
*       undoable operation on release" needs nothing new.
* @note The hue is REMEMBERED per picker, because rgb loses it at zero saturation or zero value. Without
*       that, dragging into the black corner would snap the hue strip back to red.
*/
fui_api bool fuiColorPicker(fuiContext *context, const fuiRect rect, const char *id, fuiColor *color, const bool hasAlpha, const bool enabled, bool *outDidBegin, bool *outDidEnd);

// ****************************************************************************
//
// > Commands
//
// ****************************************************************************

//! Identifies one action. Zero means "no command"; a caller numbers its own actions from one upwards.
typedef int32_t fuiCommandId;

//! The identifier that means "no command"
#define FUI_COMMAND_NONE ((fuiCommandId)0)

//! Modifier keys a shortcut requires, combined with a bitwise or.
typedef enum fuiModifier {
	//! The bare key, with no modifier held
	FUI_MOD_NONE = 0,
	//! Either control key
	FUI_MOD_CONTROL = 1 << 0,
	//! Either shift key
	FUI_MOD_SHIFT = 1 << 1,
	//! Either alt key
	FUI_MOD_ALT = 1 << 2,
} fuiModifier;

/**
* @struct fuiShortcut
* @brief A keyboard shortcut: one key plus the modifiers that must be held with it.
* @note A key of @ref FUI_KEY_NONE means the command has no shortcut, so nothing is dispatched and nothing
*       is written next to its label.
*/
typedef struct fuiShortcut {
	//! The key that triggers the command
	fuiKey key;
	//! Which modifiers must be held, as @ref fuiModifier flags
	uint32_t modifiers;
} fuiShortcut;

/**
* @struct fuiCommand
* @brief One action, named once and referenced everywhere: by a menu row, a tool strip button and the keyboard.
* @note The predicates and the callback all take the caller's own context pointer, which is handed to every
*       call that names a command. The library never looks inside it.
*/
typedef struct fuiCommand {
	//! What menus and strips reference this command by
	fuiCommandId id;
	//! What is written on the menu row or the button
	const char *label;
	//! The keyboard shortcut, displayed next to the label and dispatched by @ref fuiDispatchShortcuts
	fuiShortcut shortcut;
	//! Answers whether the command can be run right now. Null means always
	bool (*isEnabled)(void *userData);
	//! Answers whether the command is offered at all right now. Null means always
	bool (*isVisible)(void *userData);
	//! Runs the action. Null means the command draws and dispatches but does nothing
	void (*invoke)(void *userData);
} fuiCommand;

//! A caller's whole set of actions: a plain array and its count, usually one static table per screen.
typedef struct fuiCommandTable {
	//! The commands, in the order shortcuts are dispatched in
	const fuiCommand *commands;
	//! How many commands there are
	uint32_t count;
} fuiCommandTable;

/**
* @brief Finds one command in a table by its identifier.
* @param[in] table Reference to the command table @ref fuiCommandTable, may be null.
* @param[in] id The identifier to look for.
* @return Returns the command, or null when the table does not carry it.
*/
fui_api const fuiCommand *fuiFindCommand(const fuiCommandTable *table, const fuiCommandId id);

/**
* @brief Answers whether a command can be run right now.
* @param[in] command Reference to the command @ref fuiCommand, may be null.
* @param[in] userData The caller's context, handed to the command's predicate.
* @return Returns true when the command exists and its predicate says yes.
* @note A null command counts as disabled, so a lookup can be passed straight in without a null check.
*/
fui_api bool fuiCommandIsEnabled(const fuiCommand *command, void *userData);

/**
* @brief Answers whether a command is offered at all right now.
* @param[in] command Reference to the command @ref fuiCommand, may be null.
* @param[in] userData The caller's context, handed to the command's predicate.
* @return Returns true when the command exists and its predicate says yes.
* @note A null command counts as invisible, so a lookup can be passed straight in without a null check.
*/
fui_api bool fuiCommandIsVisible(const fuiCommand *command, void *userData);

/**
* @brief Writes a shortcut out the way it is displayed, such as "Ctrl+Shift+S".
* @param[in] shortcut The shortcut to spell out.
* @param[out] buffer Destination for the text, which is left empty when the shortcut has no key.
* @param[in] bufferCapacity Size of the destination in bytes, @ref FUI_MAX_SHORTCUT_TEXT is always enough.
* @return Returns buffer, so the call can be used as an argument.
*/
fui_api const char *fuiShortcutToText(const fuiShortcut shortcut, char *buffer, const size_t bufferCapacity);

/**
* @brief Runs the first command in a table whose shortcut was pressed this frame.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] table Reference to the command table @ref fuiCommandTable, may be null.
* @param[in] userData The caller's context, handed to the predicates and the callback.
* @return Returns the identifier of the command that ran, or @ref FUI_COMMAND_NONE when none did.
* @note Modifiers must match EXACTLY, so Ctrl+S does not fire while Ctrl+Shift+S is held.
* @note Nothing is dispatched while a text field has the keyboard - typing an S must not save the level.
*/
fui_api fuiCommandId fuiDispatchShortcuts(fuiContext *context, const fuiCommandTable *table, void *userData);

/**
* @brief A button that runs a command, labelled from the table and with its shortcut written on the right.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] rect The area to draw in, in pixels.
* @param[in] table Reference to the command table @ref fuiCommandTable.
* @param[in] id Identifies the command.
* @param[in] userData The caller's context, handed to the predicates and the callback.
* @return Returns true on the frame the command ran.
* @note An invisible command draws nothing and takes no input, and a disabled one draws muted and takes none.
*/
fui_api bool fuiCommandButton(fuiContext *context, const fuiRect rect, const fuiCommandTable *table, const fuiCommandId id, void *userData);

// ****************************************************************************
//
// > Menus
//
// ****************************************************************************

/**
* @brief Begins a horizontal menu bar, whose titles flow from the left.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id Identifies the bar, and scopes the identifiers of the menus in it.
* @param[in] rect The strip the titles flow across, in pixels.
* @note Popups float ABOVE everything else, so build the bar AFTER the panels it covers.
* @note Ask @ref fuiMenuBarHeight for the height rather than picking one, and @ref fuiLayoutDock for the strip itself.
* @note Always pair with @ref fuiEndMenuBar.
*/
fui_api void fuiBeginMenuBar(fuiContext *context, const char *id, const fuiRect rect);

/**
* @brief Returns how tall a menu bar needs to be, in pixels.
* @param[in] context Reference to the context @ref fuiContext.
* @return Returns one menu row: the title's text plus the inset above and below it.
* @note The same row an open popup hands its items, so a title and the rows under it are the same size. There is nothing in a menu bar that a taller strip would show more of.
*/
fui_api float fuiMenuBarHeight(const fuiContext *context);

/**
* @brief Ends a menu bar.
* @param[in,out] context Reference to the context @ref fuiContext.
*/
fui_api void fuiEndMenuBar(fuiContext *context);

/**
* @brief Begins one menu: a title in the bar, or a row with a submenu arrow inside an open popup.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] label What is written on the title or the row, which also identifies the menu.
* @return Returns true while this menu's popup is open, which is when its rows belong.
* @note Always pair with @ref fuiEndMenu, INCLUDING when it returned false.
*/
fui_api bool fuiBeginMenu(fuiContext *context, const char *label);

/**
* @brief Ends one menu.
* @param[in,out] context Reference to the context @ref fuiContext.
*/
fui_api void fuiEndMenu(fuiContext *context);

/**
* @brief One selectable row inside an open menu.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] label What is written on the row.
* @param[in] shortcutText Muted text on the right of the row, usually a shortcut. May be null.
* @param[in] enabled Set to false to draw it muted and take no input.
* @return Returns true on the frame the row was chosen, which also closes the whole menu.
*/
fui_api bool fuiMenuItem(fuiContext *context, const char *label, const char *shortcutText, const bool enabled);

/**
* @brief One row inside an open menu that shows a check box, for a setting that is on or off.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] label What is written on the row.
* @param[in] isChecked Whether the box is filled.
* @param[in] enabled Set to false to draw it muted and take no input.
* @return Returns true on the frame the row was chosen. The caller flips its own value.
*/
fui_api bool fuiMenuItemCheck(fuiContext *context, const char *label, const bool isChecked, const bool enabled);

/**
* @brief One row inside an open menu that runs a command, with its shortcut written on the right.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] table Reference to the command table @ref fuiCommandTable.
* @param[in] id Identifies the command.
* @param[in] userData The caller's context, handed to the predicates and the callback.
* @return Returns true on the frame the command ran.
* @note An invisible command emits no row at all, so a menu shrinks around what is not on offer.
*/
fui_api bool fuiMenuItemCommand(fuiContext *context, const fuiCommandTable *table, const fuiCommandId id, void *userData);

/**
* @brief A dividing line between two groups of rows inside an open menu.
* @param[in,out] context Reference to the context @ref fuiContext.
*/
fui_api void fuiMenuSeparator(fuiContext *context);

/**
* @brief Opens a context menu at the cursor, usually from a right click.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id Identifies the context menu, and must match the @ref fuiBeginContextMenu that builds it.
* @note The identifier is hashed on its own rather than against the surrounding scope, so opening it and
*       building it do not have to happen at the same place in the tree.
*/
fui_api void fuiOpenContextMenu(fuiContext *context, const char *id);

/**
* @brief Begins a context menu, which builds its rows only while it is open.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id Identifies the context menu, and must match the @ref fuiOpenContextMenu that opened it.
* @return Returns true while this context menu is open.
* @note Always pair with @ref fuiEndContextMenu, INCLUDING when it returned false.
*/
fui_api bool fuiBeginContextMenu(fuiContext *context, const char *id);

/**
* @brief Ends a context menu.
* @param[in,out] context Reference to the context @ref fuiContext.
*/
fui_api void fuiEndContextMenu(fuiContext *context);

/**
* @brief Answers whether any menu or context menu is open.
* @param[in] context Reference to the context @ref fuiContext.
* @return Returns true while a popup is up.
*/
fui_api bool fuiIsMenuOpen(const fuiContext *context);

/**
* @brief Closes every open menu at once.
* @param[in,out] context Reference to the context @ref fuiContext.
* @note Escape and a press outside already do this. This is for a caller that has to close them itself,
*       such as one that just switched to another screen.
*/
fui_api void fuiCloseMenus(fuiContext *context);

// ****************************************************************************
//
// > Tool strip
//
// ****************************************************************************

/**
* @brief Begins a tool strip: a row or a column of buttons that size themselves.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id Identifies the strip, and scopes the identifiers of the items in it.
* @param[in] rect The area the items flow across, in pixels.
* @param[in] axis Which way the items flow.
* @note A horizontal strip sizes each item to its label; a vertical one gives every item the theme's row height.
* @note The items are inset ACROSS the flow axis by the same measure that separates them along it, so they never run into the edge of the strip. Ask @ref fuiToolStripThickness for the strip's own thickness and @ref fuiLayoutDock for the strip itself.
* @note Always pair with @ref fuiEndToolStrip.
*/
fui_api void fuiBeginToolStrip(fuiContext *context, const char *id, const fuiRect rect, const fuiAxis axis);

/**
* @brief Returns how thick a tool strip needs to be ACROSS its flow axis, in pixels.
* @param[in] context Reference to the context @ref fuiContext.
* @return Returns one strip item plus the inset above and below it.
* @note For a horizontal strip that thickness is its HEIGHT, which is all a strip along the top of a window needs. A vertical one is as wide as the caller makes it, since only the caller knows how long the labels are, and this is the least it may be.
*/
fui_api float fuiToolStripThickness(const fuiContext *context);

/**
* @brief Ends a tool strip.
* @param[in,out] context Reference to the context @ref fuiContext.
*/
fui_api void fuiEndToolStrip(fuiContext *context);

/**
* @brief One button in the open tool strip.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] label What is written on the button, which also identifies it.
* @return Returns true on the frame it was clicked.
*/
fui_api bool fuiToolStripButton(fuiContext *context, const char *label);

/**
* @brief One button in the open tool strip that runs a command.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] table Reference to the command table @ref fuiCommandTable.
* @param[in] id Identifies the command.
* @param[in] userData The caller's context, handed to the predicates and the callback.
* @return Returns true on the frame the command ran.
* @note The shortcut is NOT written on it - a strip sizes itself to the label, and a tooltip is the place
*       for the shortcut.
*/
fui_api bool fuiToolStripCommand(fuiContext *context, const fuiCommandTable *table, const fuiCommandId id, void *userData);

/**
* @brief One button in the open tool strip that stays lit while its mode is the active one.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] label What is written on the button, which also identifies it.
* @param[in] isActive Whether to draw it lit in the accent color.
* @param[in] enabled Set to false to draw it muted and take no input.
* @return Returns true on the frame it was clicked. The caller changes its own mode.
*/
fui_api bool fuiToolStripToggle(fuiContext *context, const char *label, const bool isActive, const bool enabled);

/**
* @brief A dividing line between two groups of items in the open tool strip.
* @param[in,out] context Reference to the context @ref fuiContext.
*/
fui_api void fuiToolStripSeparator(fuiContext *context);

// ****************************************************************************
//
// > Status bar
//
// ****************************************************************************

/**
* @brief Begins a status bar, which items fill in from both ends towards the middle.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id Identifies the bar.
* @param[in] rect The strip the items sit in, in pixels.
* @note Ask @ref fuiStatusBarHeight for the height rather than picking one, and @ref fuiLayoutDock for the strip itself.
* @note Always pair with @ref fuiEndStatusBar.
*/
fui_api void fuiBeginStatusBar(fuiContext *context, const char *id, const fuiRect rect);

/**
* @brief Returns how tall a status bar needs to be, in pixels.
* @param[in] context Reference to the context @ref fuiContext.
* @return Returns one line of text plus the inset above and below it.
* @note A status bar holds text and nothing else, so this is as tall as it can usefully get.
*/
fui_api float fuiStatusBarHeight(const fuiContext *context);

/**
* @brief Ends a status bar.
* @param[in,out] context Reference to the context @ref fuiContext.
*/
fui_api void fuiEndStatusBar(fuiContext *context);

/**
* @brief One item on the left of the open status bar. The next left item starts after it.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] text What the item says.
*/
fui_api void fuiStatusText(fuiContext *context, const char *text);

/**
* @brief One item on the right of the open status bar. The next right item ends before it.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] text What the item says.
*/
fui_api void fuiStatusTextRight(fuiContext *context, const char *text);

/**
* @brief A dividing line between two left aligned items of the open status bar.
* @param[in,out] context Reference to the context @ref fuiContext.
*/
fui_api void fuiStatusSeparator(fuiContext *context);

// ****************************************************************************
//
// > Modal dialogs
//
// ****************************************************************************

/**
* @brief Opens a dialog, putting it in front of everything that was already open.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id Identifies the dialog, and has to be the same string its build is given.
* @note Opening a dialog that is already open does nothing, so a menu row may call this every time it is
*       clicked. While any dialog is open the interface is modal: nothing outside the front one interacts.
* @see @ref fuiCloseDialog, @ref fuiBeginModal
*/
fui_api void fuiOpenDialog(fuiContext *context, const char *id);

/**
* @brief Closes a dialog, wherever in the open stack it sits.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id Identifies the dialog.
* @note The built in dialogs close themselves once the user chooses something. This is for a dialog whose
*       caller decides when it is done, such as one that stays up while a save is confirmed.
*/
fui_api void fuiCloseDialog(fuiContext *context, const char *id);

/**
* @brief Answers whether a dialog is open.
* @param[in] context Reference to the context @ref fuiContext.
* @param[in] id Identifies the dialog.
* @return Returns true while the dialog is on the open stack.
*/
fui_api bool fuiDialogIsOpen(const fuiContext *context, const char *id);

/**
* @brief Answers whether any dialog at all is open.
* @param[in] context Reference to the context @ref fuiContext.
* @return Returns true while at least one dialog is up, and so while the interface is modal.
* @note This is what a game asks before it steps its world, so play pauses under a dialog.
*/
fui_api bool fuiIsAnyDialogOpen(const fuiContext *context);

/**
* @brief Closes every open dialog at once.
* @param[in,out] context Reference to the context @ref fuiContext.
* @note For a screen that switches away under its own dialogs rather than for anything a user does.
*/
fui_api void fuiCloseAllDialogs(fuiContext *context);

/**
* @brief Begins the content of a dialog, drawing its backdrop, box and title bar.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id Identifies the dialog, the same string @ref fuiOpenDialog was given.
* @param[in] title What the title bar says.
* @param[in] width Width of the box in pixels.
* @param[in] height Height of the box in pixels.
* @return Returns true when the dialog is open and its content should be built.
* @note ALWAYS pair with @ref fuiEndModal, including when this returned false. The box is centred in the
*       window and cannot be moved, which is what keeps a prompt where the eye already is.
* @see @ref fuiBeginModalResizable, @ref fuiEndModal
*/
fui_api bool fuiBeginModal(fuiContext *context, const char *id, const char *title, const float width, const float height);

/**
* @brief Begins the content of a dialog the user may move by its title bar and resize by its corner.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id Identifies the dialog, the same string @ref fuiOpenDialog was given.
* @param[in] title What the title bar says.
* @param[in] width Width the box opens at in pixels, before whatever the user dragged.
* @param[in] height Height the box opens at in pixels, before whatever the user dragged.
* @return Returns true when the dialog is open and its content should be built.
* @note The box opens centred and is then pinned by its top left corner, so dragging the grip moves that
*       corner alone. Keep this for a dialog holding a list or a wide table; a prompt stays fixed.
* @see @ref fuiBeginModal, @ref fuiEndModal
*/
fui_api bool fuiBeginModalResizable(fuiContext *context, const char *id, const char *title, const float width, const float height);

/**
* @brief Ends the content of a dialog.
* @param[in,out] context Reference to the context @ref fuiContext.
* @note Call this for every @ref fuiBeginModal, whatever it returned.
*/
fui_api void fuiEndModal(fuiContext *context);

/**
* @brief Answers whether the code building right now is inside the dialog that owns input.
* @param[in] context Reference to the context @ref fuiContext.
* @return Returns true when no dialog is open, or when this build is inside the front one.
* @note What a dialog's own content asks before it acts on a key, so a dialog stacked underneath another
*       one does not answer the same keystroke.
*/
fui_api bool fuiIsFrontModal(const fuiContext *context);

/**
* @brief Answers whether a dialog was up when this frame's interacting build began.
* @param[in] context Reference to the context @ref fuiContext.
* @return Returns true when a dialog owned this frame's input.
* @note For code OUTSIDE the interface that reads the same raw keys a dialog does. By the time such code
*       runs, a dialog dismissed this frame has already closed and @ref fuiIsAnyDialogOpen would say no.
*/
fui_api bool fuiModalOwnedInputThisFrame(const fuiContext *context);

/**
* @brief Takes the key a dialog answers with, for at most ONE dialog per press.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] key The key being answered, in practice @ref FUI_KEY_RETURN or @ref FUI_KEY_ESCAPE.
* @return Returns true when this build may act on that press.
* @note Two dialogs stacked on each other are built one after the other, and the moment the front one
*       closes the one underneath BECOMES the front one - inside the same build. Asking plainly whether
*       escape went down would then close both on one press. The built in dialogs answer through here; a
*       dialog built by hand out of @ref fuiBeginModal should too. Never answers while a menu is open,
*       since an open menu is what escape closes first.
*/
fui_api bool fuiDialogTakeKey(fuiContext *context, const fuiKey key);

//! Which buttons a message box offers, and so what it can answer.
typedef enum fuiMessageBoxButtons {
	//! A single OK, for a message that is only to be acknowledged
	FUI_MESSAGE_BOX_OK = 0,
	//! OK and Cancel, for an action the user may still call off
	FUI_MESSAGE_BOX_OK_CANCEL,
	//! Yes and No, for a question with no way out of answering
	FUI_MESSAGE_BOX_YES_NO,
	//! Yes, No and Cancel, for a question that may also be called off
	FUI_MESSAGE_BOX_YES_NO_CANCEL,
} fuiMessageBoxButtons;

//! What a dialog was answered with. Everything but @ref FUI_DIALOG_RESULT_NONE means it closed this frame.
typedef enum fuiDialogResult {
	//! Still open, or not open at all
	FUI_DIALOG_RESULT_NONE = 0,
	//! Accepted
	FUI_DIALOG_RESULT_OK,
	//! Dismissed, by the button, by the escape key or by the title bar
	FUI_DIALOG_RESULT_CANCEL,
	//! Answered yes
	FUI_DIALOG_RESULT_YES,
	//! Answered no
	FUI_DIALOG_RESULT_NO,
} fuiDialogResult;

//! Whether a file dialog is picking something that exists or naming something that does not.
typedef enum fuiFileDialogMode {
	//! Pick one of the listed entries
	FUI_FILE_DIALOG_OPEN = 0,
	//! Type a name, with the list there to pick one to overwrite
	FUI_FILE_DIALOG_SAVE,
} fuiFileDialogMode;

//! What a @ref fuiFileBrowser build came to. Only Descend leaves the dialog open in both modes.
typedef enum fuiFileBrowserResult {
	//! Nothing happened this frame
	FUI_FILE_BROWSER_NONE = 0,
	//! A folder row was activated. The dialog stays open for the caller to list that folder instead
	FUI_FILE_BROWSER_DESCEND,
	//! A file was accepted. An opening browser closes, a saving one stays up for the caller to close
	FUI_FILE_BROWSER_ACCEPT,
	//! Dismissed by the button or by the escape key. The dialog closes
	FUI_FILE_BROWSER_CANCEL,
} fuiFileBrowserResult;

/**
* @struct fuiListIcons
* @brief A sheet of equally sized icon cells, plus which cell each row of a list draws.
* @note The interface stays ignorant of what an icon MEANS: it draws cell N for the row whose entry says N,
*       and which cell a row gets is the caller's table. A cell index is `column + row * columns`, so a
*       single row sheet indexes straight by column. Leave `sheet` zero to draw no icons at all.
*/
typedef struct fuiListIcons {
	//! The sheet every cell is cut out of, zero to disable icons
	fuiTextureId sheet;
	//! Full size of the sheet in pixels, which the cell rectangles are measured out of
	fuiVec2 sheetSize;
	//! How many cells the sheet holds across
	int32_t columns;
	//! How many cells the sheet holds down
	int32_t rows;
	//! One cell index per list entry. A NEGATIVE entry is one row opting out of a list that has icons
	const int32_t *cellForRow;
	//! How many entries cellForRow holds. A row past the end draws no icon
	int32_t cellForRowCount;
	//! How much taller an icon row is than a plain one, and so how big the glyph draws. Zero takes 2.0
	float rowScale;
} fuiListIcons;

/**
* @brief A message box with a title, one line of text and a set of buttons.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id Identifies the dialog, the same string @ref fuiOpenDialog was given.
* @param[in] title What the title bar says.
* @param[in] text The message. The box grows wider than its minimum to fit a long one.
* @param[in] buttons Which buttons to offer @ref fuiMessageBoxButtons.
* @return Returns what the user chose, and @ref FUI_DIALOG_RESULT_NONE while it is still up.
* @note Closes itself as soon as it is answered. Enter takes the first button, escape the dismissing one.
*/
fui_api fuiDialogResult fuiMessageBox(fuiContext *context, const char *id, const char *title, const char *text, const fuiMessageBoxButtons buttons);

/**
* @brief A dialog asking for one line of text.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id Identifies the dialog, the same string @ref fuiOpenDialog was given.
* @param[in] title What the title bar says.
* @param[in] label What the field is asking for.
* @param[in,out] buffer The text, edited in place. Seed it before opening the dialog to offer a default.
* @param[in] capacity How many bytes buffer holds, counting the terminator.
* @return Returns @ref FUI_DIALOG_RESULT_OK or @ref FUI_DIALOG_RESULT_CANCEL once answered.
* @note The field takes the keyboard as the dialog opens, so the user can type straight away.
*/
fui_api fuiDialogResult fuiInputBox(fuiContext *context, const char *id, const char *title, const char *label, char *buffer, const int32_t capacity);

/**
* @brief A dialog asking for several lines of text.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id Identifies the dialog, the same string @ref fuiOpenDialog was given.
* @param[in] title What the title bar says.
* @param[in] label What the area is asking for.
* @param[in,out] buffer The text, edited in place.
* @param[in] capacity How many bytes buffer holds, counting the terminator.
* @param[in] wordWrap True to fold a long line at a word rather than let it run off the right edge.
* @return Returns @ref FUI_DIALOG_RESULT_OK or @ref FUI_DIALOG_RESULT_CANCEL once answered.
*/
fui_api fuiDialogResult fuiInputBoxMultiline(fuiContext *context, const char *id, const char *title, const char *label, char *buffer, const int32_t capacity, const bool wordWrap);

/**
* @brief A dialog holding a color picker over a preview swatch.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id Identifies the dialog, the same string @ref fuiOpenDialog was given.
* @param[in] title What the title bar says.
* @param[in,out] color The color, edited in place while the dialog is up.
* @param[in] hasAlpha True to offer the alpha band as well.
* @return Returns @ref FUI_DIALOG_RESULT_OK or @ref FUI_DIALOG_RESULT_CANCEL once answered.
* @note The color is written as it is dragged, so a caller that means to restore it on cancel has to keep
*       its own copy of what it was.
*/
fui_api fuiDialogResult fuiColorDialog(fuiContext *context, const char *id, const char *title, fuiColor *color, const bool hasAlpha);

/**
* @brief A color dialog that also offers a live update switch.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id Identifies the dialog, the same string @ref fuiOpenDialog was given.
* @param[in] title What the title bar says.
* @param[in,out] color The color, edited in place while the dialog is up.
* @param[in] hasAlpha True to offer the alpha band as well.
* @param[in,out] liveUpdate The switch, owned by the caller so it survives the dialog closing. Null is
*                exactly @ref fuiColorDialog, and the row is not drawn at all.
* @return Returns @ref FUI_DIALOG_RESULT_OK or @ref FUI_DIALOG_RESULT_CANCEL once answered.
* @note The dialog only owns the switch's UI. ACTING on it - pushing every drag through to the real thing
*       a swatch cannot show, such as a light - is the caller's, and is the whole reason it exists.
*/
fui_api fuiDialogResult fuiColorDialogEx(fuiContext *context, const char *id, const char *title, fuiColor *color, const bool hasAlpha, bool *liveUpdate);

/**
* @brief A scrolling list of strings, one of which is selected.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] rect The box the list sits in, in pixels.
* @param[in] id Identifies the list.
* @param[in] items The strings, one per row.
* @param[in] count How many strings items holds.
* @param[in,out] selectedIndex Which row is selected, changed in place. Minus one for none.
* @return Returns true on the frame the selection changed.
*/
fui_api bool fuiListBox(fuiContext *context, const fuiRect rect, const char *id, const char *const *items, const int32_t count, int32_t *selectedIndex);

/**
* @brief A list box that may carry a row icon and reports a row being activated.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] rect The box the list sits in, in pixels.
* @param[in] id Identifies the list.
* @param[in] items The strings, one per row.
* @param[in] count How many strings items holds.
* @param[in,out] selectedIndex Which row is selected, changed in place. Minus one for none.
* @param[in] icons Reference to the icon sheet @ref fuiListIcons, or null for a list of plain text.
* @param[out] outWasActivated Set when a row was double clicked, which is the open this gesture. May be null.
* @return Returns true on the frame the selection changed.
*/
fui_api bool fuiListBoxEx(fuiContext *context, const fuiRect rect, const char *id, const char *const *items, const int32_t count, int32_t *selectedIndex, const fuiListIcons *icons, bool *outWasActivated);

/**
* @brief A dialog picking one entry out of a flat list, or naming a new one.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id Identifies the dialog, the same string @ref fuiOpenDialog was given.
* @param[in] title What the title bar says.
* @param[in] items The entries, one per row. The library reads no directory of its own.
* @param[in] count How many entries items holds.
* @param[in,out] selectedIndex Which entry is selected, changed in place. Minus one for none.
* @param[in] mode Whether an entry is being picked or a name typed @ref fuiFileDialogMode.
* @param[in,out] nameBuffer The typed name in save mode, ignored when opening. May be null when opening.
* @param[in] nameCapacity How many bytes nameBuffer holds, counting the terminator.
* @param[in] icons Reference to the icon sheet @ref fuiListIcons, or null for a list of plain text.
* @return Returns @ref FUI_DIALOG_RESULT_OK or @ref FUI_DIALOG_RESULT_CANCEL once answered.
* @note Accepting with nothing selected is ignored in open mode, since there is nothing to open.
*/
fui_api fuiDialogResult fuiFileDialog(fuiContext *context, const char *id, const char *title, const char *const *items, const int32_t count, int32_t *selectedIndex, const fuiFileDialogMode mode, char *nameBuffer, const int32_t nameCapacity, const fuiListIcons *icons);

/**
* @brief A dialog browsing one folder at a time, which the caller lists and relists.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id Identifies the dialog, the same string @ref fuiOpenDialog was given.
* @param[in] title What the title bar says.
* @param[in] locationLabel Where the listing came from, shown above the rows.
* @param[in] items The entries of the folder being shown, one per row.
* @param[in] isFolder One flag per entry, saying which rows may be descended into.
* @param[in] count How many entries items and isFolder hold.
* @param[in,out] selectedIndex Which entry is selected, changed in place. Minus one for none.
* @param[in] icons Reference to the icon sheet @ref fuiListIcons, or null for a list of plain text.
* @param[in,out] nameBuffer The typed name, which is also what makes this a SAVING browser. Null opens.
* @param[in] nameCapacity How many bytes nameBuffer holds, counting the terminator.
* @param[out] outIndex Which row the answer is about, or minus one when a saving browser accepted a typed
*             name that matched no row. May be null.
* @return Returns what happened this frame @ref fuiFileBrowserResult.
* @note Descend leaves the dialog OPEN, on purpose: the caller lists the folder and hands the new entries
*       to the same call next frame. Reset selectedIndex when it does, or the new folder opens part way down.
*/
fui_api fuiFileBrowserResult fuiFileBrowser(fuiContext *context, const char *id, const char *title, const char *locationLabel, const char *const *items, const bool *isFolder, const int32_t count, int32_t *selectedIndex, const fuiListIcons *icons, char *nameBuffer, const int32_t nameCapacity, int32_t *outIndex);

// ****************************************************************************
//
// > Image
//
// ****************************************************************************

//! Where the drawn image sits in its box when it does not fill it - the nine anchors of a three by three
//! grid, with the middle of them first so that a zeroed description asks for the sensible one
typedef enum fuiImagePlacement {
	//! Centered both ways
	FUI_IMAGE_PLACE_CENTER = 0,
	//! Against the top left corner
	FUI_IMAGE_PLACE_TOP_LEFT,
	//! Centered across, against the top
	FUI_IMAGE_PLACE_TOP,
	//! Against the top right corner
	FUI_IMAGE_PLACE_TOP_RIGHT,
	//! Centered down, against the left
	FUI_IMAGE_PLACE_LEFT,
	//! Centered down, against the right
	FUI_IMAGE_PLACE_RIGHT,
	//! Against the bottom left corner
	FUI_IMAGE_PLACE_BOTTOM_LEFT,
	//! Centered across, against the bottom
	FUI_IMAGE_PLACE_BOTTOM,
	//! Against the bottom right corner
	FUI_IMAGE_PLACE_BOTTOM_RIGHT,
} fuiImagePlacement;

/**
* @enum fuiImageScaleMode
* @brief How big the image is drawn inside the box it was given.
* @note Origin is what a zeroed description asks for, and is the only mode that may overflow the box - which
*       the enclosing panel then clips.
*/
typedef enum fuiImageScaleMode {
	//! At its own size, placed by the placement. May be larger than the box
	FUI_IMAGE_SCALE_ORIGIN = 0,
	//! Stretched to fill the box exactly, distorting it
	FUI_IMAGE_SCALE_STRETCH,
	//! Origin with the placement forced to the center
	FUI_IMAGE_SCALE_CENTER,
	//! Fitted inside the box keeping its shape, leaving bars on the two sides that came up short
	FUI_IMAGE_SCALE_LETTERBOX,
} fuiImageScaleMode;

/**
* @struct fuiImageDesc
* @brief Everything one image draw needs. Zero it and fill in what matters.
* @note An all-zero description draws the whole texture at its own pixel size, centered in its box. Every
*       "zero takes the default" field says so on its own line.
*/
typedef struct fuiImageDesc {
	//! The texture to sample, zero draws nothing at all
	fuiTextureId texture;
	//! Full size of the texture in pixels, which is what gives the sampled part a size of its own
	fuiVec2 textureSize;
	//! Top-left texture coordinate of the sampled part
	fuiVec2 uvMin;
	//! Bottom-right texture coordinate of the sampled part. Leave both zero to take the whole texture
	fuiVec2 uvMax;
	//! Tint multiplied onto the texture. A fully transparent one - which a zeroed description is - takes white
	fuiColor tint;
	//! Size to draw at in the caller's OWN units, converted by pixelsPerUnit. Zero takes the sampled pixel size
	fuiVec2 renderSize;
	//! How many pixels one of those units is. Zero ignores renderSize
	float pixelsPerUnit;
	//! Multiplier on the size before it is placed or fitted, so 0.5 is half size. Zero takes 1.0
	float scaleFactor;
	//! Origin and Center only - shrink to keep the sampled part's shape rather than take the size as asked
	bool keepAspectRatio;
	//! Where it sits when it is smaller than its box @ref fuiImagePlacement
	fuiImagePlacement placement;
	//! How big it is drawn @ref fuiImageScaleMode
	fuiImageScaleMode scaleMode;
	//! Mirroring and quarter turns @ref fuiImageFlags. A quarter turn swaps the shape it is fitted by
	fuiImageFlags flags;
} fuiImageDesc;

/**
* @brief Draws a texture, or a part of one, into a box.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] rect The box it goes in, in pixels.
* @param[in] desc Reference to what to draw and how @ref fuiImageDesc.
* @note Purely visual - it hit tests nothing and swallows no click, so a widget may be built over it. Does
*       nothing at all in the interact pass, like every other drawing call.
*/
fui_api void fuiImage(fuiContext *context, const fuiRect rect, const fuiImageDesc *desc);

// ****************************************************************************
//
// > Tab control
//
// ****************************************************************************

/**
* @brief A row of tab headers, one of which is chosen.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] rect The strip the headers fill, in pixels. A @ref fuiLayoutSlot off the top of a panel fits.
* @param[in] id Identifies the tab control, and is what remembers which tab is showing.
* @param[in] tabs What each header says.
* @param[in] tabCount How many headers there are.
* @return Returns the index of the tab that is showing, always inside [0, tabCount).
* @note The control draws the HEADERS and nothing else. Build the chosen tab's content underneath it
*       yourself, which is what lets a tab hold anything at all.
*/
fui_api int32_t fuiTabControl(fuiContext *context, const fuiRect rect, const char *id, const char *const *tabs, const int32_t tabCount);

// ****************************************************************************
//
// > List view
//
// ****************************************************************************

/**
* @struct fuiColumn
* @brief One column of a list view: what its header says and how wide it starts out.
* @note The width is a STARTING point, not the live one - dragging the divider beside a header changes that,
*       and the list remembers it. Which is what lets this stay a static table shared by every build.
*/
typedef struct fuiColumn {
	//! What the header says
	const char *title;
	//! How wide the column starts out, in pixels
	float width;
} fuiColumn;

/**
* @struct fuiListRowButtons
* @brief Buttons filling one column of a list view, one per row.
* @note The named column draws buttons instead of its text, so a row can be acted on without being picked
*       first. A row whose label is null or empty gets no button, which is how one row opts out.
*/
typedef struct fuiListRowButtons {
	//! Which column the buttons fill
	int32_t column;
	//! One label per row. Null or empty is a row without a button
	const char *const *labelForRow;
	//! OUT: which row's button was pressed this build, minus one when none was
	int32_t clickedRow;
} fuiListRowButtons;

/**
* @brief A list of rows under a row of column headers.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] rect The box the list sits in, in pixels.
* @param[in] id Identifies the list, and is what remembers its widths, its sort and how far it is scrolled.
* @param[in] columns The columns @ref fuiColumn, at most @ref FUI_MAX_LIST_COLUMNS of them.
* @param[in] columnCount How many columns there are.
* @param[in] cells Row major, so cell (row, column) is cells[row * columnCount + column].
* @param[in] rowCount How many rows there are.
* @param[in,out] selectedIndex Which row is selected, changed in place. Minus one for none.
* @param[out] outWasActivated Set when a row was double clicked, which is the open this gesture. May be null.
* @return Returns true on the frame the selection changed.
* @note Columns RESIZE by dragging the divider beside a header, and SORT by clicking the title itself.
*       A cell neither wraps nor shortens its text, it clips - dragging the column wider is the way to read it.
* @note selectedIndex is always the CALLER's row index and never a display position, so sorting moves where a
*       row is drawn and never what it is. The same is true of the activated row and of a button's row.
*/
fui_api bool fuiListView(fuiContext *context, const fuiRect rect, const char *id, const fuiColumn *columns, const int32_t columnCount, const char *const *cells, const int32_t rowCount, int32_t *selectedIndex, bool *outWasActivated);

/**
* @brief A list view whose rows may carry an icon.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] rect The box the list sits in, in pixels.
* @param[in] id Identifies the list.
* @param[in] columns The columns @ref fuiColumn.
* @param[in] columnCount How many columns there are.
* @param[in] cells Row major cell strings, rowCount * columnCount of them.
* @param[in] rowCount How many rows there are.
* @param[in,out] selectedIndex Which row is selected, changed in place. Minus one for none.
* @param[in] icons Reference to the icon sheet @ref fuiListIcons, or null for rows of plain text.
* @param[out] outWasActivated Set when a row was double clicked. May be null.
* @return Returns true on the frame the selection changed.
* @note The icon goes in the FIRST column ahead of its text, the same place a list box puts it, and makes
*       every row taller by the sheet's `rowScale`. The header keeps the plain height whatever the rows do.
*/
fui_api bool fuiListViewEx(fuiContext *context, const fuiRect rect, const char *id, const fuiColumn *columns, const int32_t columnCount, const char *const *cells, const int32_t rowCount, int32_t *selectedIndex, const fuiListIcons *icons, bool *outWasActivated);

/**
* @brief A list view with icons and a column of per-row buttons.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] rect The box the list sits in, in pixels.
* @param[in] id Identifies the list.
* @param[in] columns The columns @ref fuiColumn.
* @param[in] columnCount How many columns there are.
* @param[in] cells Row major cell strings, rowCount * columnCount of them.
* @param[in] rowCount How many rows there are.
* @param[in,out] selectedIndex Which row is selected, changed in place. Minus one for none.
* @param[in] icons Reference to the icon sheet @ref fuiListIcons, or null for rows of plain text.
* @param[in,out] rowButtons Reference to the button column @ref fuiListRowButtons, or null for none. Read its
*                clickedRow straight after this call, exactly like a return value.
* @param[out] outWasActivated Set when a row was double clicked. May be null.
* @return Returns true on the frame the selection changed.
* @note A press inside a button never also picks the row - the button owns that click.
*/
fui_api bool fuiListViewButtons(fuiContext *context, const fuiRect rect, const char *id, const fuiColumn *columns, const int32_t columnCount, const char *const *cells, const int32_t rowCount, int32_t *selectedIndex, const fuiListIcons *icons, fuiListRowButtons *rowButtons, bool *outWasActivated);

/**
* @brief Sorts a list view by one of its columns right now.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id The list's identifier, the same string the list itself is given.
* @param[in] sortColumn Which column to sort by. A NEGATIVE one puts the caller's own row order back.
* @param[in] ascending True to sort the way a list is read, false to sort backwards.
* @note Unconditional, so it overrides whatever the user last clicked. It belongs on an explicit action - a
*       reset item, data that invalidated the sort - and NOT in the per frame build, where it would fight
*       every header click. For a starting sort use @ref fuiListViewSetSortDefault.
* @note Must be called in the same identifier scope as the list, which calling it right before the list is.
*/
fui_api void fuiListViewSetSort(fuiContext *context, const char *id, const int32_t sortColumn, const bool ascending);

/**
* @brief The sort a list view starts out in.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id The list's identifier, the same string the list itself is given.
* @param[in] sortColumn Which column to start sorted by. A negative one is the caller's own row order.
* @param[in] ascending True to start sorted the way a list is read.
* @note Applied on the first build only and ignored ever after, so it is safe to state every build - which is
*       how immediate mode states everything. A default is a starting point, and the user's own choice of
*       column has to outlive it.
*/
fui_api void fuiListViewSetSortDefault(fuiContext *context, const char *id, const int32_t sortColumn, const bool ascending);

/**
* @brief Whether the USER may sort a list view by clicking its headers.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id The list's identifier, the same string the list itself is given.
* @param[in] sortable False to leave the headers plain and make clicking one do nothing.
* @note Turn it off for a list whose ROW ORDER IS THE DATA - a playlist, an ordered chain, anything where
*       "the third row" means something - because re-ordering the view there does not present the same thing
*       differently, it misrepresents it. Code can still set a sort on a locked list.
*/
fui_api void fuiListViewSetSortable(fuiContext *context, const char *id, const bool sortable);

/**
* @brief Throws away the sorted order a list view has cached, so the next build works it out again.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] id Identifier of the list view, the same string it is built with.
* @note Only needed for a list longer than FUI_LIST_SORT_VERIFY_ROWS whose cells were edited IN PLACE -
*       the same array, the same number of rows, different text. A shorter list hashes its sorted column
*       every frame and notices that by itself, and a different array, length or sort is noticed either way.
* @see @ref fuiListView
*/
fui_api void fuiListViewInvalidateSort(fuiContext *context, const char *id);

#ifdef __cplusplus
}
#endif

#endif // FUI_INCLUDE_H

// ****************************************************************************
// ****************************************************************************
//
// > IMPLEMENTATION
//
// ****************************************************************************
// ****************************************************************************
#if (defined(FUI_IMPLEMENTATION) && !defined(FUI_IMPLEMENTED)) || (FUI_IS_IDE)

#ifndef FUI_IMPLEMENTED
#	define FUI_IMPLEMENTED
#endif

#if !defined(FUI_MALLOC) || !defined(FUI_FREE)
#	include <stdlib.h>
#	if !defined(FUI_MALLOC)
#		define FUI_MALLOC(size) malloc(size)
#	endif
#	if !defined(FUI_FREE)
#		define FUI_FREE(ptr) free(ptr)
#	endif
#endif

#if !defined(FUI_MEMSET) || !defined(FUI_MEMCPY) || !defined(FUI_MEMMOVE) || !defined(FUI_STRLEN) || !defined(FUI_MEMCHR)
#	include <string.h>
#	if !defined(FUI_STRLEN)
#		define FUI_STRLEN(text) strlen(text)
#	endif
#	if !defined(FUI_MEMCHR)
#		define FUI_MEMCHR(ptr, value, size) memchr(ptr, value, size)
#	endif
#	if !defined(FUI_MEMSET)
#		define FUI_MEMSET(dst, value, size) memset(dst, value, size)
#	endif
#	if !defined(FUI_MEMCPY)
#		define FUI_MEMCPY(dst, src, size) memcpy(dst, src, size)
#	endif
#	if !defined(FUI_MEMMOVE)
#		define FUI_MEMMOVE(dst, src, size) memmove(dst, src, size)
#	endif
#endif

#include <stdarg.h>

#if !defined(FUI_VSNPRINTF)
#	include <stdio.h>
#	define FUI_VSNPRINTF(dest, size, format, argList) vsnprintf(dest, size, format, argList)
#endif

#if !defined(FUI_FABSF) || !defined(FUI_FMODF) || !defined(FUI_SQRTF)
#	include <math.h>
#	if !defined(FUI_FABSF)
#		define FUI_FABSF(value) fabsf(value)
#	endif
#	if !defined(FUI_FMODF)
#		define FUI_FMODF(a, b) fmodf(a, b)
#	endif
#	if !defined(FUI_SQRTF)
#		define FUI_SQRTF(value) sqrtf(value)
#	endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

//! Memory alignment used by the arena
#define FUI__ALIGNMENT 16

// ----------------------------------------------------------------------------
// > Memory helpers
// ----------------------------------------------------------------------------

//! Rounds a size up to the next multiple of the given alignment
fui_inline size_t fui__AlignUp(const size_t value, const size_t alignment) {
	size_t result = (value + (alignment - 1)) & ~(alignment - 1);
	return(result);
}

//! Clears a range of memory to zero
fui_inline void fui__ClearMemory(void *destination, const size_t size) {
	if(destination != fui_null && size > 0) {
		FUI_MEMSET(destination, 0, size);
	}
}

//! Copies a range of memory, source and destination must not overlap
fui_inline void fui__CopyMemory(void *destination, const void *source, const size_t size) {
	if(destination != fui_null && source != fui_null && size > 0) {
		FUI_MEMCPY(destination, source, size);
	}
}

//! Copies a range of memory that is allowed to overlap, which is what shifting a text buffer needs
fui_inline void fui__MoveMemory(void *destination, const void *source, const size_t size) {
	if(destination != fui_null && source != fui_null && size > 0) {
		FUI_MEMMOVE(destination, source, size);
	}
}

static void *fui__DefaultAllocate(void *userData, size_t size) {
	(void)userData;
	return FUI_MALLOC(size);
}

static void fui__DefaultRelease(void *userData, void *ptr) {
	(void)userData;
	FUI_FREE(ptr);
}

fui_api fuiAllocator fuiDefaultAllocator(void) {
	fuiAllocator result;
	result.allocate = fui__DefaultAllocate;
	result.release = fui__DefaultRelease;
	result.userData = fui_null;
	return(result);
}

// ----------------------------------------------------------------------------
// > Arena
// ----------------------------------------------------------------------------

//! Prepares an arena to hand out memory from the given allocator
fui_inline void fui__ArenaInit(fuiArena *arena, const fuiAllocator *allocator, const size_t minBlockSize) {
	FUI_ASSERT(arena != fui_null);
	fui__ClearMemory(arena, sizeof(*arena));
	arena->allocator = (allocator != fui_null) ? *allocator : fuiDefaultAllocator();
	arena->minBlockSize = (minBlockSize > 0) ? minBlockSize : (size_t)FUI_ARENA_MIN_BLOCK_SIZE;
	arena->firstBlock = fui_null;
	arena->currentBlock = fui_null;
	arena->totalAllocatedSize = 0;
	arena->outOfMemory = false;
}

//! Asks the allocator for one more block, big enough to hold at least requiredSize bytes
fui_inline bool fui__ArenaAddBlock(fuiArena *arena, const size_t requiredSize) {
	FUI_ASSERT(arena != fui_null);
	size_t headerSize = fui__AlignUp(sizeof(fuiMemoryBlock), FUI__ALIGNMENT);
	size_t payloadSize = fui__AlignUp(requiredSize, FUI__ALIGNMENT);
	size_t wantedSize = headerSize + payloadSize;
	size_t totalSize = (wantedSize > arena->minBlockSize) ? wantedSize : arena->minBlockSize;

	void *memory = arena->allocator.allocate(arena->allocator.userData, totalSize);
	if(memory == fui_null) {
		arena->outOfMemory = true;
		return(false);
	}

	fuiMemoryBlock *block = (fuiMemoryBlock *)memory;
	block->next = fui_null;
	block->base = (uint8_t *)memory + headerSize;
	block->size = totalSize - headerSize;
	block->used = 0;

	if(arena->firstBlock == fui_null) {
		arena->firstBlock = block;
	} else {
		FUI_ASSERT(arena->currentBlock != fui_null);
		arena->currentBlock->next = block;
	}
	arena->currentBlock = block;
	arena->totalAllocatedSize += totalSize;
	return(true);
}

//! Bump allocates size bytes out of the arena, growing it by a block when needed
fui_inline void *fui__ArenaPush(fuiArena *arena, const size_t size, const bool clear) {
	FUI_ASSERT(arena != fui_null);
	if(size == 0) {
		return(fui_null);
	}

	size_t alignedSize = fui__AlignUp(size, FUI__ALIGNMENT);
	bool hasRoom = (arena->currentBlock != fui_null) && ((arena->currentBlock->used + alignedSize) <= arena->currentBlock->size);
	if(!hasRoom) {
		bool blockAdded = fui__ArenaAddBlock(arena, alignedSize);
		if(!blockAdded) {
			// Hand out the scratch slot rather than null, so a small allocation that fails still has
			// somewhere safe to write. Anything bigger than the slot has to be checked by its caller.
			if(alignedSize <= (size_t)FUI_OOM_SINK_SIZE) {
				fui__ClearMemory(arena->outOfMemorySink, alignedSize);
				return(arena->outOfMemorySink);
			}
			return(fui_null);
		}
	}

	FUI_ASSERT(arena->currentBlock != fui_null);
	uint8_t *result = arena->currentBlock->base + arena->currentBlock->used;
	arena->currentBlock->used += alignedSize;
	if(clear) {
		fui__ClearMemory(result, alignedSize);
	}
	return(result);
}

//! Hands every byte back to the arena without releasing its blocks, so the next frame reuses them
fui_inline void fui__ArenaReset(fuiArena *arena) {
	FUI_ASSERT(arena != fui_null);
	fuiMemoryBlock *block = arena->firstBlock;
	while(block != fui_null) {
		block->used = 0;
		block = block->next;
	}
	arena->currentBlock = arena->firstBlock;
}

//! Releases every block back to the allocator and leaves the arena empty
fui_inline void fui__ArenaFree(fuiArena *arena) {
	FUI_ASSERT(arena != fui_null);
	fuiMemoryBlock *block = arena->firstBlock;
	while(block != fui_null) {
		fuiMemoryBlock *next = block->next;
		arena->allocator.release(arena->allocator.userData, block);
		block = next;
	}
	arena->firstBlock = fui_null;
	arena->currentBlock = fui_null;
	arena->totalAllocatedSize = 0;
}

// ----------------------------------------------------------------------------
// > String helpers
// ----------------------------------------------------------------------------

/*
	Returns the number of bytes before the terminating zero, or zero for a null string.

	This goes through FUI_STRLEN rather than walking the bytes itself, because it is on the path of every
	single label: a widget handed a caption and no length asks for one here, and so does the multiline
	text field, once per frame, over the whole document it is showing. A byte at a time loop and the C
	library's own are an order of magnitude apart on a long string, and the difference showed up as
	milliseconds a frame on a text field holding a large file.
*/
fui_inline size_t fui__StringLength(const char *text) {
	if(text == fui_null) {
		return(0);
	}
	size_t result = FUI_STRLEN(text);
	return(result);
}

//! Copies a string and always terminates it, truncating when it does not fit. Returns the bytes written.
fui_inline size_t fui__CopyString(char *destination, const size_t destinationCapacity, const char *source) {
	if(destination == fui_null || destinationCapacity == 0) {
		return(0);
	}
	size_t sourceLength = fui__StringLength(source);
	size_t copyLength = (sourceLength < (destinationCapacity - 1)) ? sourceLength : (destinationCapacity - 1);
	if(copyLength > 0) {
		fui__CopyMemory(destination, source, copyLength);
	}
	destination[copyLength] = 0;
	return(copyLength);
}

//! Appends a string to another and always terminates it, truncating when it does not fit. Returns the total length.
fui_inline size_t fui__AppendString(char *destination, const size_t destinationCapacity, const char *source) {
	if(destination == fui_null || destinationCapacity == 0) {
		return(0);
	}
	size_t existingLength = fui__StringLength(destination);
	if(existingLength >= (destinationCapacity - 1)) {
		return(existingLength);
	}
	size_t remainingCapacity = destinationCapacity - existingLength;
	size_t appendedLength = fui__CopyString(destination + existingLength, remainingCapacity, source);
	size_t result = existingLength + appendedLength;
	return(result);
}

//! Compares two strings byte by byte, where two null strings count as equal
fui_inline bool fui__StringsAreEqual(const char *a, const char *b) {
	if(a == b) {
		return(true);
	}
	if(a == fui_null || b == fui_null) {
		return(false);
	}
	while(*a != 0 && *b != 0) {
		if(*a != *b) {
			return(false);
		}
		++a;
		++b;
	}
	bool result = (*a == *b);
	return(result);
}

//! Formats a string into a caller buffer and always terminates it. Returns the bytes written.
fui_inline size_t fui__FormatString(char *destination, const size_t destinationCapacity, const char *format, ...) {
	if(destination == fui_null || destinationCapacity == 0 || format == fui_null) {
		return(0);
	}
	va_list argList;
	va_start(argList, format);
	int written = FUI_VSNPRINTF(destination, destinationCapacity, format, argList);
	va_end(argList);
	destination[destinationCapacity - 1] = 0;
	if(written < 0) {
		destination[0] = 0;
		return(0);
	}
	size_t result = ((size_t)written < destinationCapacity) ? (size_t)written : (destinationCapacity - 1);
	return(result);
}

// ----------------------------------------------------------------------------
// > Font
// ----------------------------------------------------------------------------

fui_api fuiFont fuiZeroFont(void) {
	fuiFont result;
	fui__ClearMemory(&result, sizeof(result));
	return(result);
}

// ----------------------------------------------------------------------------
// > Theme
// ----------------------------------------------------------------------------

fui_api fuiTheme fuiDefaultTheme(void) {
	fuiTheme result;
	fui__ClearMemory(&result, sizeof(result));

	result.panelBackgroundColor = fuiColorRGBA(0.10f, 0.11f, 0.14f, 0.92f);
	result.panelBorderColor = fuiColorRGBA(0.30f, 0.45f, 0.70f, 0.90f);
	result.textColor = fuiColorRGBA(0.90f, 0.92f, 0.95f, 1.0f);
	result.textMutedColor = fuiColorRGBA(0.55f, 0.60f, 0.66f, 1.0f);
	result.accentColor = fuiColorRGBA(0.95f, 0.75f, 0.35f, 1.0f);

	result.widgetColor = fuiColorRGBA(0.20f, 0.22f, 0.28f, 1.0f);
	result.widgetHoveredColor = fuiColorRGBA(0.28f, 0.32f, 0.40f, 1.0f);
	result.widgetActiveColor = fuiColorRGBA(0.36f, 0.44f, 0.58f, 1.0f);
	result.widgetTrackColor = fuiColorRGBA(0.06f, 0.07f, 0.09f, 1.0f);
	// Translucent white and translucent black rather than two fixed greys, so ONE pair of edges lifts the idle
	// face, the hovered face and the accented face of a toggle alike.
	result.widgetBevelLightColor = fuiColorRGBA(1.0f, 1.0f, 1.0f, 0.22f);
	result.widgetBevelShadowColor = fuiColorRGBA(0.0f, 0.0f, 0.0f, 0.38f);
	result.textSelectionColor = fuiColorRGBA(0.24f, 0.38f, 0.60f, 0.75f);
	result.knobColor = fuiColorRGBA(0.85f, 0.88f, 0.92f, 1.0f);
	result.menuHighlightColor = fuiColorRGBA(0.24f, 0.38f, 0.60f, 1.0f);
	result.modalBackdropColor = fuiColorRGBA(0.0f, 0.0f, 0.0f, 0.55f);

	result.tooltipBackgroundColor = fuiColorRGBA(0.06f, 0.07f, 0.09f, 0.97f);
	result.tooltipBorderColor = fuiColorRGBA(0.45f, 0.52f, 0.62f, 0.95f);
	result.tooltipTextColor = fuiColorRGBA(0.92f, 0.94f, 0.97f, 1.0f);

	result.fontHeight = 18.0f;
	result.menuItemHeight = 24.0f;
	result.menuItemFontHeight = 20.0f;
	result.menuItemPaddingX = 12.0f;
	result.menuItemPaddingY = 6.0f;
	result.menuPopupMinWidth = 160.0f;
	// One frame width for every box the interface draws, and it is the THIN one: a panel outlined twice as
	// heavily as the widgets inside it reads as a different design language on the same screen.
	result.panelBorderThickness = 1.0f;
	result.widgetBorderThickness = 1.0f;
	// Two pixels of bevel is what reads as a chamfer at the sizes the widgets are drawn at. One pixel almost
	// disappears against the outline beside it, and three starts to eat the face it is supposed to frame.
	result.widgetBevelThickness = 2.0f;
	result.widgetFaceShadingStrength = 0.16f;
	result.widgetPressOffset = 1.0f;
	result.panelPaddingX = 10.0f;
	result.panelPaddingY = 8.0f;
	result.widgetPaddingX = 8.0f;
	// The gap between two widgets is the same measure as the gap between a widget and the panel edge beside it.
	result.widgetSpacing = 10.0f;
	result.groupSeparatorHeight = 10.0f;
	result.caretBlinkHz = 2.0f;

	result.tooltipFontHeight = 16.0f;
	result.tooltipPaddingX = 8.0f;
	result.tooltipPaddingY = 6.0f;
	result.tooltipLineSpacing = 3.0f;
	result.tooltipCursorOffsetX = 14.0f;
	result.tooltipCursorOffsetY = 18.0f;
	result.tooltipScreenMargin = 4.0f;
	result.tooltipDelaySeconds = 0.4f;

	return(result);
}

// ----------------------------------------------------------------------------
// > Context
// ----------------------------------------------------------------------------

fui_api bool fuiInit(fuiContext *context, const fuiFont *font, const fuiAllocator *allocator) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return(false);
	}
	if(allocator != fui_null) {
		FUI_ASSERT(allocator->allocate != fui_null);
		FUI_ASSERT(allocator->release != fui_null);
		bool allocatorIsUsable = (allocator->allocate != fui_null) && (allocator->release != fui_null);
		if(!allocatorIsUsable) {
			return(false);
		}
	}

	fui__ClearMemory(context, sizeof(*context));
	fui__ArenaInit(&context->arena, allocator, (size_t)FUI_ARENA_MIN_BLOCK_SIZE);
	context->theme = fuiDefaultTheme();
	context->font = font;
	context->isInitialized = true;
	return(true);
}

fui_api void fuiRelease(fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null || !context->isInitialized) {
		return;
	}
	fui__ArenaFree(&context->arena);
	fui__ClearMemory(context, sizeof(*context));
}

fui_api void fuiSetFont(fuiContext *context, const fuiFont *font) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return;
	}
	context->font = font;
}

fui_api void fuiSetTheme(fuiContext *context, const fuiTheme *theme) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return;
	}
	if(theme != fui_null) {
		context->theme = *theme;
	} else {
		context->theme = fuiDefaultTheme();
	}
}

fui_api fuiTheme *fuiGetTheme(fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null || !context->isInitialized) {
		return(fui_null);
	}
	fuiTheme *result = &context->theme;
	return(result);
}

fui_api bool fuiHasOutOfMemory(const fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return(false);
	}
	bool result = context->arena.outOfMemory;
	return(result);
}

fui_api size_t fuiGetAllocatedSize(const fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return(0);
	}
	size_t result = context->arena.totalAllocatedSize;
	return(result);
}

// ----------------------------------------------------------------------------
// > Platform
// ----------------------------------------------------------------------------

fui_api void fuiSetPlatform(fuiContext *context, const fuiPlatform *platform) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return;
	}
	if(platform != fui_null) {
		context->platform = *platform;
	} else {
		fui__ClearMemory(&context->platform, sizeof(context->platform));
	}
}

fui_api void fuiSetCursor(fuiContext *context, const fuiCursor cursor) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null || cursor >= FUI_CURSOR_COUNT) {
		return;
	}
	context->cursor = cursor;
}

fui_api fuiCursor fuiGetCursor(const fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return(FUI_CURSOR_ARROW);
	}
	return(context->cursor);
}

fui_api bool fuiGetClipboardText(const fuiContext *context, char *destination, const uint32_t maxDestinationLength) {
	FUI_ASSERT(context != fui_null && destination != fui_null);
	if(context == fui_null || destination == fui_null || maxDestinationLength == 0) {
		return(false);
	}
	destination[0] = '\0';
	if(context->platform.getClipboardText == fui_null) {
		return(false);
	}
	bool result = context->platform.getClipboardText(context->platform.userData, destination, maxDestinationLength);
	if(!result) {
		destination[0] = '\0';
	}
	return(result);
}

fui_api bool fuiSetClipboardText(const fuiContext *context, const char *text) {
	FUI_ASSERT(context != fui_null && text != fui_null);
	if(context == fui_null || text == fui_null || context->platform.setClipboardText == fui_null) {
		return(false);
	}
	bool result = context->platform.setClipboardText(context->platform.userData, text);
	return(result);
}

// ----------------------------------------------------------------------------
// > Draw buffers
// ----------------------------------------------------------------------------

//! Number of entries a draw buffer starts at before it starts doubling
#define FUI__DRAW_BUFFER_START_CAPACITY 256

//! Like fui__ArenaPush, but never hands back the shared out-of-memory slot
fui_inline void *fui__ArenaPushExact(fuiArena *arena, const size_t size, const bool clear) {
	void *result = fui__ArenaPush(arena, size, clear);
	if(result == (void *)arena->outOfMemorySink) {
		// The scratch slot is shared by everyone, so it is fine for a value that is written and dropped
		// but never for an array we are about to keep and index into.
		return(fui_null);
	}
	return(result);
}

//! Makes sure a draw buffer holds at least neededCount entries, doubling its capacity to get there
fui_inline bool fui__DrawBufferReserve(fuiArena *arena, fuiDrawBuffer *buffer, const uint32_t neededCount, const size_t elementSize) {
	FUI_ASSERT(buffer != fui_null);
	if(neededCount <= buffer->capacity) {
		return(true);
	}

	uint32_t newCapacity = (buffer->capacity > 0) ? buffer->capacity : (uint32_t)FUI__DRAW_BUFFER_START_CAPACITY;
	while(newCapacity < neededCount) {
		if(newCapacity > (UINT32_MAX / 2u)) {
			newCapacity = neededCount;
			break;
		}
		newCapacity *= 2u;
	}

	// The arena never frees a single allocation, so the old array is simply left behind. Doubling caps
	// the waste below the final size and the growth stops after a handful of frames.
	void *newItems = fui__ArenaPushExact(arena, (size_t)newCapacity * elementSize, false);
	if(newItems == fui_null) {
		return(false);
	}
	if(buffer->count > 0 && buffer->items != fui_null) {
		fui__CopyMemory(newItems, buffer->items, (size_t)buffer->count * elementSize);
	}
	buffer->items = newItems;
	buffer->capacity = newCapacity;
	return(true);
}

//! What a draw call writes its geometry through. Nothing is written when isValid is false.
typedef struct fui__GeometryWriter {
	//! Where to write this call's vertices
	fuiVertex *vertices;
	//! Where to write this call's indices, which are absolute and so start at baseVertex
	fuiDrawIndex *indices;
	//! Index of this call's first vertex in the whole frame
	uint32_t baseVertex;
	//! The command this geometry belongs to, for filling in the fast path payload
	fuiDrawCommand *command;
	//! How many indices that command already held before this call was appended to it, which is what a
	//! trim has to fall back to when the call turns out to draw nothing
	uint32_t indexCountBeforeThisCall;
	//! Whether this call was appended to a command that already existed rather than getting one of its own
	bool wasMergedIntoTheCommandBefore;
	//! False when there was no room, the clip rejected the call, or there is no frame being built
	bool isValid;
} fui__GeometryWriter;

//! Reserves room for one draw call and appends its command
fui_inline fui__GeometryWriter fui__BeginGeometry(fuiContext *context, const fuiDrawKind kind, const fuiTextureId texture, const uint32_t vertexCount, const uint32_t indexCount) {
	fui__GeometryWriter result;
	fui__ClearMemory(&result, sizeof(result));

	FUI_ASSERT(context != fui_null);
	if(context == fui_null || !context->isBuildingFrame || vertexCount == 0 || indexCount == 0) {
		return(result);
	}

	// An interact pass exists to hit test, and its geometry would be thrown away by the draw pass that
	// follows. Skipping it here, at the one place geometry is reserved, is what keeps a two pass caller
	// from tessellating the whole interface twice per frame.
	if(context->pass == FUI_PASS_INTERACT) {
		return(result);
	}

	uint32_t baseVertex = context->vertexBuffer.count;
	uint64_t highestIndex = (uint64_t)baseVertex + (uint64_t)vertexCount - 1u;
	if(highestIndex > (uint64_t)FUI_DRAW_INDEX_MAX) {
		// Only reachable in a FUI_USE_16BIT_INDICES build. Drop the geometry rather than write an index
		// that wraps around and draws a triangle across the whole window.
		context->indexRangeExceeded = true;
		return(result);
	}

	bool verticesReserved = fui__DrawBufferReserve(&context->arena, &context->vertexBuffer, baseVertex + vertexCount, sizeof(fuiVertex));
	bool indicesReserved = fui__DrawBufferReserve(&context->arena, &context->indexBuffer, context->indexBuffer.count + indexCount, sizeof(fuiDrawIndex));
	bool commandReserved = fui__DrawBufferReserve(&context->arena, &context->commandBuffer, context->commandBuffer.count + 1u, sizeof(fuiDrawCommand));
	if(!verticesReserved || !indicesReserved || !commandReserved) {
		return(result);
	}

	fuiVertex *vertexArray = (fuiVertex *)context->vertexBuffer.items;
	fuiDrawIndex *indexArray = (fuiDrawIndex *)context->indexBuffer.items;
	fuiDrawCommand *commandArray = (fuiDrawCommand *)context->commandBuffer.items;
	fuiRect clipRect = fuiGetClipRect(context);

	/*
		Merging, when the caller asked for it.

		A command is a draw call. A dense table is hundreds of little rectangles that share a clip and a
		texture and follow each other with nothing in between, and each of them used to be its own call
		into the driver for geometry that could just as well go out in one.

		A merged command becomes FUI_DRAW_TRIANGLES, because no payload can describe two primitives at
		once. That is exactly why this is not the default: a backend that draws through the payloads
		rather than through the geometry would find nothing left in them to read.
	*/
	fuiDrawCommand *command = fui_null;
	bool wasMerged = false;
	if(context->drawBatchingIsOn && context->commandBuffer.count > 0) {
		fuiDrawCommand *previous = &commandArray[context->commandBuffer.count - 1u];
		bool sameClip = fui__RectEquals(previous->clipRect, clipRect);
		bool sameTexture = (previous->texture == texture);
		// The two runs of indices have to touch, or one offset and one count cannot name both of them.
		bool isContiguous = ((previous->indexOffset + previous->indexCount) == context->indexBuffer.count);
		if(sameClip && sameTexture && isContiguous) {
			command = previous;
			wasMerged = true;
		}
	}

	if(wasMerged) {
		result.indexCountBeforeThisCall = command->indexCount;
		command->kind = FUI_DRAW_TRIANGLES;
		command->indexCount += indexCount;
	} else {
		command = &commandArray[context->commandBuffer.count];
		fui__ClearMemory(command, sizeof(*command));
		command->kind = kind;
		command->clipRect = clipRect;
		command->texture = texture;
		command->vertexOffset = baseVertex;
		command->indexOffset = context->indexBuffer.count;
		command->indexCount = indexCount;
	}

	result.vertices = &vertexArray[baseVertex];
	result.indices = &indexArray[context->indexBuffer.count];
	result.baseVertex = baseVertex;
	result.command = command;
	result.wasMergedIntoTheCommandBefore = wasMerged;
	result.isValid = true;

	context->vertexBuffer.count += vertexCount;
	context->indexBuffer.count += indexCount;
	if(!wasMerged) {
		context->commandBuffer.count += 1u;
	}
	return(result);
}

//! Gives back the tail of a reservation that turned out not to be needed
fui_inline void fui__TrimGeometry(fuiContext *context, fui__GeometryWriter *writer, const uint32_t usedVertexCount, const uint32_t usedIndexCount) {
	FUI_ASSERT(context != fui_null && writer != fui_null);
	if(!writer->isValid) {
		return;
	}

	// A merged call shares its command with everything that went into it before, so what it gives back is
	// its OWN share of the indices. Only a call that owns its command may take the command away with it.
	uint32_t keptIndexCount = writer->indexCountBeforeThisCall;
	if(usedIndexCount == 0 || usedVertexCount == 0) {
		context->vertexBuffer.count = writer->baseVertex;
		context->indexBuffer.count = writer->command->indexOffset + keptIndexCount;
		if(writer->wasMergedIntoTheCommandBefore) {
			writer->command->indexCount = keptIndexCount;
		} else {
			// Nothing was drawn after all, so drop the command entirely rather than leave an empty one.
			context->commandBuffer.count -= 1u;
		}
		writer->isValid = false;
		return;
	}
	context->vertexBuffer.count = writer->baseVertex + usedVertexCount;
	context->indexBuffer.count = writer->command->indexOffset + keptIndexCount + usedIndexCount;
	writer->command->indexCount = keptIndexCount + usedIndexCount;
}

//! Writes one textured quad as four vertices and six indices, each corner sampling where it is told to.
//! The corners are ordered top-left, top-right, bottom-right, bottom-left.
fui_inline void fui__WriteQuadUvCorners(fui__GeometryWriter *writer, const uint32_t vertexAt, const uint32_t indexAt, const fuiRect rect, const fuiVec2 *cornerUvs, const uint32_t packedColor) {
	FUI_ASSERT(writer != fui_null && writer->isValid);
	float left = rect.x;
	float top = rect.y;
	float right = rect.x + rect.w;
	float bottom = rect.y + rect.h;

	fuiVertex *vertices = &writer->vertices[vertexAt];
	vertices[0].position = fuiV2(left, top);
	vertices[0].uv = cornerUvs[0];
	vertices[0].color = packedColor;
	vertices[1].position = fuiV2(right, top);
	vertices[1].uv = cornerUvs[1];
	vertices[1].color = packedColor;
	vertices[2].position = fuiV2(right, bottom);
	vertices[2].uv = cornerUvs[2];
	vertices[2].color = packedColor;
	vertices[3].position = fuiV2(left, bottom);
	vertices[3].uv = cornerUvs[3];
	vertices[3].color = packedColor;

	fuiDrawIndex base = (fuiDrawIndex)(writer->baseVertex + vertexAt);
	fuiDrawIndex *indices = &writer->indices[indexAt];
	indices[0] = base;
	indices[1] = (fuiDrawIndex)(base + 1u);
	indices[2] = (fuiDrawIndex)(base + 2u);
	indices[3] = base;
	indices[4] = (fuiDrawIndex)(base + 2u);
	indices[5] = (fuiDrawIndex)(base + 3u);
}

//! Writes one textured quad sampling an upright rectangle of the texture
fui_inline void fui__WriteQuad(fui__GeometryWriter *writer, const uint32_t vertexAt, const uint32_t indexAt, const fuiRect rect, const fuiVec2 uvMin, const fuiVec2 uvMax, const uint32_t packedColor) {
	fuiVec2 cornerUvs[4];
	cornerUvs[0] = fuiV2(uvMin.x, uvMin.y);
	cornerUvs[1] = fuiV2(uvMax.x, uvMin.y);
	cornerUvs[2] = fuiV2(uvMax.x, uvMax.y);
	cornerUvs[3] = fuiV2(uvMin.x, uvMax.y);
	fui__WriteQuadUvCorners(writer, vertexAt, indexAt, rect, cornerUvs, packedColor);
}

//! The uv pair used by untextured geometry
fui_inline fuiVec2 fui__ZeroUv(void) {
	fuiVec2 result = fuiV2(0.0f, 0.0f);
	return(result);
}

//! Writes one untextured quad whose two top corners carry one color and whose two bottom corners carry another,
//! which is a vertical gradient once the rasterizer interpolates between them
fui_inline void fui__WriteQuadVerticalGradient(fui__GeometryWriter *writer, const uint32_t vertexAt, const uint32_t indexAt, const fuiRect rect, const uint32_t packedTopColor, const uint32_t packedBottomColor) {
	FUI_ASSERT(writer != fui_null && writer->isValid);
	float left = rect.x;
	float top = rect.y;
	float right = rect.x + rect.w;
	float bottom = rect.y + rect.h;
	fuiVec2 zeroUv = fui__ZeroUv();

	fuiVertex *vertices = &writer->vertices[vertexAt];
	vertices[0].position = fuiV2(left, top);
	vertices[0].uv = zeroUv;
	vertices[0].color = packedTopColor;
	vertices[1].position = fuiV2(right, top);
	vertices[1].uv = zeroUv;
	vertices[1].color = packedTopColor;
	vertices[2].position = fuiV2(right, bottom);
	vertices[2].uv = zeroUv;
	vertices[2].color = packedBottomColor;
	vertices[3].position = fuiV2(left, bottom);
	vertices[3].uv = zeroUv;
	vertices[3].color = packedBottomColor;

	fuiDrawIndex base = (fuiDrawIndex)(writer->baseVertex + vertexAt);
	fuiDrawIndex *indices = &writer->indices[indexAt];
	indices[0] = base;
	indices[1] = (fuiDrawIndex)(base + 1u);
	indices[2] = (fuiDrawIndex)(base + 2u);
	indices[3] = base;
	indices[4] = (fuiDrawIndex)(base + 2u);
	indices[5] = (fuiDrawIndex)(base + 3u);
}

//! True when a primitive's bounds cannot show up inside the active clip, so it can be dropped outright
fui_inline bool fui__IsClippedAway(const fuiContext *context, const fuiRect bounds) {
	fuiRect clip = fuiGetClipRect(context);
	fuiRect visible = fuiRectIntersect(clip, bounds);
	bool result = fuiRectIsEmpty(visible);
	return(result);
}

// ----------------------------------------------------------------------------
// > Input
// ----------------------------------------------------------------------------

fui_api fuiInput fuiZeroInput(void) {
	fuiInput result;
	fui__ClearMemory(&result, sizeof(result));
	result.isActive = true;
	return(result);
}

static const char *fui__DigitKeyNames[10] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };
static const char *fui__LetterKeyNames[26] = { "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z" };
static const char *fui__FunctionKeyNames[12] = { "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12" };

fui_api const char *fuiKeyGetName(const fuiKey key) {
	if(key >= FUI_KEY_0 && key <= FUI_KEY_9) {
		return(fui__DigitKeyNames[(int)key - (int)FUI_KEY_0]);
	}
	if(key >= FUI_KEY_A && key <= FUI_KEY_Z) {
		return(fui__LetterKeyNames[(int)key - (int)FUI_KEY_A]);
	}
	if(key >= FUI_KEY_F1 && key <= FUI_KEY_F12) {
		return(fui__FunctionKeyNames[(int)key - (int)FUI_KEY_F1]);
	}
	switch(key) {
		case FUI_KEY_BACKSPACE: return("Backspace");
		case FUI_KEY_TAB: return("Tab");
		case FUI_KEY_RETURN: return("Enter");
		case FUI_KEY_ESCAPE: return("Esc");
		case FUI_KEY_SPACE: return("Space");
		case FUI_KEY_PAGE_UP: return("PageUp");
		case FUI_KEY_PAGE_DOWN: return("PageDown");
		case FUI_KEY_END: return("End");
		case FUI_KEY_HOME: return("Home");
		case FUI_KEY_LEFT: return("Left");
		case FUI_KEY_UP: return("Up");
		case FUI_KEY_RIGHT: return("Right");
		case FUI_KEY_DOWN: return("Down");
		case FUI_KEY_INSERT: return("Insert");
		case FUI_KEY_DELETE: return("Delete");
		case FUI_KEY_LEFT_CONTROL: return("Ctrl");
		case FUI_KEY_RIGHT_CONTROL: return("Ctrl");
		case FUI_KEY_LEFT_SHIFT: return("Shift");
		case FUI_KEY_RIGHT_SHIFT: return("Shift");
		case FUI_KEY_LEFT_ALT: return("Alt");
		case FUI_KEY_RIGHT_ALT: return("Alt");
		default: return("");
	}
}

fui_api bool fuiKeyRepeatEdge(const fuiButtonState button, const float frameTime, float *timer) {
	FUI_ASSERT(timer != fui_null);
	if(timer == fui_null) {
		return(false);
	}
	// The press edge is asked for FIRST, so a key that was pushed and let go again inside one frame still
	// fires once. Counting half transitions is the whole reason that keystroke is knowable at all, and
	// checking "is it down" ahead of it would throw the fast ones away.
	if(fuiButtonWentDown(button)) {
		*timer = fuiButtonIsDown(button) ? FUI_KEY_REPEAT_DELAY : 0.0f;
		return(true);
	}
	if(!fuiButtonIsDown(button)) {
		*timer = 0.0f;
		return(false);
	}
	*timer -= frameTime;
	if(*timer <= 0.0f) {
		*timer = FUI_KEY_REPEAT_INTERVAL;
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
// > Identity
// ----------------------------------------------------------------------------

//! Offset basis of the 64-bit FNV-1a hash
#define FUI__HASH_OFFSET_BASIS 14695981039346656037ULL
//! Prime of the 64-bit FNV-1a hash
#define FUI__HASH_PRIME 1099511628211ULL

//! Folds a run of bytes into a hash, seeded by the enclosing scope
fui_inline fuiId fui__HashBytes(const fuiId seed, const void *bytes, const size_t byteCount) {
	fuiId hash = (seed != FUI_ID_NONE) ? seed : (fuiId)FUI__HASH_OFFSET_BASIS;
	const unsigned char *cursor = (const unsigned char *)bytes;
	for(size_t index = 0; index < byteCount; ++index) {
		hash ^= (fuiId)cursor[index];
		hash *= (fuiId)FUI__HASH_PRIME;
	}
	// Zero is reserved for "no widget", so the one hash that would collide with it is nudged.
	if(hash == FUI_ID_NONE) {
		hash = 1;
	}
	return(hash);
}

//! The scope new identifiers are combined with
fui_inline fuiId fui__CurrentIdSeed(const fuiContext *context) {
	if(context == fui_null || context->idDepth == 0) {
		return((fuiId)FUI__HASH_OFFSET_BASIS);
	}
	uint32_t topIndex = (context->idDepth <= (uint32_t)FUI_MAX_ID_STACK) ? (context->idDepth - 1u) : ((uint32_t)FUI_MAX_ID_STACK - 1u);
	fuiId result = context->idStack[topIndex];
	return(result);
}

fui_api fuiId fuiGetId(const fuiContext *context, const char *label) {
	FUI_ASSERT(label != fui_null);
	if(label == fui_null) {
		return(FUI_ID_NONE);
	}
	fuiId seed = fui__CurrentIdSeed(context);
	size_t labelLength = fui__StringLength(label);
	fuiId result = fui__HashBytes(seed, label, labelLength);
	return(result);
}

//! Shared by both push entry points, so a scope that does not fit still counts and pops stay balanced
fui_inline void fui__PushIdValue(fuiContext *context, const fuiId id) {
	if(context->idDepth < (uint32_t)FUI_MAX_ID_STACK) {
		context->idStack[context->idDepth] = id;
	}
	context->idDepth += 1u;
}

fui_api void fuiPushId(fuiContext *context, const char *label) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return;
	}
	fuiId scopeId = fuiGetId(context, label);
	fui__PushIdValue(context, scopeId);
}

fui_api void fuiPushIdInt(fuiContext *context, const int32_t value) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return;
	}
	fuiId seed = fui__CurrentIdSeed(context);
	fuiId scopeId = fui__HashBytes(seed, &value, sizeof(value));
	fui__PushIdValue(context, scopeId);
}

fui_api void fuiPopId(fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return;
	}
	FUI_ASSERT(context->idDepth > 0);
	if(context->idDepth > 0) {
		context->idDepth -= 1u;
	}
}

// ----------------------------------------------------------------------------
// > Frame
// ----------------------------------------------------------------------------

// Both are the LAST things a frame does and both are written far below, next to the drawing and the
// platform code they belong with. Declared here because fuiEndFrame is the only caller of either.
static void fui__DrawPendingTooltip(fuiContext *context);
static void fui__ApplyCursor(fuiContext *context);

//! Declared ahead of its definition, because the frame boundary is the one place it may be called from
//! and that is here, well above where the widget state table itself is written
fui_inline void fui__WidgetStateMapGrow(fuiContext *context);

fui_api void fuiBeginFrame(fuiContext *context, const fuiInput *input, const fuiPass pass) {
	FUI_ASSERT(context != fui_null && input != fui_null);
	if(context == fui_null || input == fui_null || !context->isInitialized) {
		return;
	}

	// Before anything can take a pointer into it. See fui__WidgetStateMapGrow for why it can only be here.
	if(context->widgetStates.growthIsWanted) {
		fui__WidgetStateMapGrow(context);
	}

	// The drawing half resets the buffers and the clip stack, and defaults the pass. Everything below
	// then says what this build really is, so the order here is load bearing.
	fuiBeginDrawFrame(context, input->windowSize);

	bool isDrawPass = (pass == FUI_PASS_DRAW);
	context->pass = pass;
	context->isInteractive = !isDrawPass;

	// A draw pass rebuilds the identical tree, so it must not advance time either -- a repeat timer or
	// a caret blink stepped twice per frame would run at double speed.
	context->frameTime = isDrawPass ? 0.0f : input->deltaTime;

	fuiVec2 previousMousePosition = context->mousePosition;
	context->mousePosition = input->mousePosition;
	context->mouseDelta = context->hasSeenAFrame ? fuiV2Subtract(input->mousePosition, previousMousePosition) : fuiV2(0.0f, 0.0f);
	context->mouseWheelDelta = isDrawPass ? 0.0f : input->mouseWheelDelta;
	context->inputIsActive = input->isActive;

	for(uint32_t buttonIndex = 0; buttonIndex < (uint32_t)FUI_MOUSE_BUTTON_COUNT; ++buttonIndex) {
		fuiButtonState button = input->mouseButtons[buttonIndex];
		context->mouseIsDown[buttonIndex] = fuiButtonIsDown(button);
		context->mouseWentDown[buttonIndex] = !isDrawPass && fuiButtonWentDown(button);
		context->mouseWentUp[buttonIndex] = !isDrawPass && fuiButtonWentUp(button);
		if(!context->mouseIsDown[buttonIndex]) {
			context->mouseDownConsumed[buttonIndex] = false;
		}
	}

	for(uint32_t keyIndex = 0; keyIndex < (uint32_t)FUI_KEY_COUNT; ++keyIndex) {
		context->keys[keyIndex] = input->keys[keyIndex];
		if(isDrawPass) {
			// Keep whether it is held, drop the edge, so the second build cannot fire a shortcut twice.
			context->keys[keyIndex].halfTransitionCount = 0;
			context->keyRepeated[keyIndex] = false;
		} else {
			context->keyRepeated[keyIndex] = fuiKeyRepeatEdge(input->keys[keyIndex], input->deltaTime, &context->keyRepeatTimers[keyIndex]);
		}
	}

	context->textInputLength = 0;
	if(!isDrawPass) {
		int32_t typedCount = input->textInputLength;
		if(typedCount > (int32_t)FUI_MAX_TEXT_INPUT) {
			typedCount = (int32_t)FUI_MAX_TEXT_INPUT;
		}
		for(int32_t typedIndex = 0; typedIndex < typedCount; ++typedIndex) {
			context->textInput[typedIndex] = input->textInput[typedIndex];
		}
		context->textInputLength = typedCount;
	}

	context->hotCandidate = FUI_ID_NONE;
	context->mouseIsOverUi = false;
	context->idDepth = 0;

	// The caret blink runs on its own clock so that it is reset to SOLID whenever the caret moves. A caret
	// that happens to be in its dark half while somebody is typing looks like the keystroke was lost.
	context->caretBlinkTime += context->frameTime;

	// Both are rebuilt from scratch by this build. Whoever is hovered asks for the tooltip again; the
	// timer and the text below outlive the frame, which is what lets the rest delay count at all.
	context->tooltipRequested = false;
	context->cursor = FUI_CURSOR_ARROW;

	context->firstFocusableThisFrame = FUI_ID_NONE;
	context->previousFocusableThisFrame = FUI_ID_NONE;
	context->tabWasConsumedThisFrame = false;
	context->dialogKeyWasConsumedThisFrame = false;

	// Latched only on the interacting build, so a two pass caller's draw pass still reports what its
	// interact pass saw rather than the state the interact pass left behind.
	if(!isDrawPass) {
		context->modalOpenAtFrameStart = (context->modalDepth > 0);
	}
	context->timeSeconds += context->frameTime;
}

fui_api void fuiEndFrame(fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null || !context->isInitialized) {
		return;
	}

	context->hot = context->hotCandidate;

	// A press released over nothing still has to let go, or the widget that captured it stays stuck the
	// moment it is not rebuilt.
	if(context->active != FUI_ID_NONE && !context->mouseIsDown[FUI_MOUSE_LEFT]) {
		context->active = FUI_ID_NONE;
	}

	if(context->menuOpenDepth > 0) {
		// Escape closes the whole tree, and so does a press that landed on no bar and no popup. Resolved here
		// rather than where the popup is built, because "outside every menu" is only knowable once they all are.
		bool anyButtonWentDown = context->mouseWentDown[FUI_MOUSE_LEFT] || context->mouseWentDown[FUI_MOUSE_MIDDLE] || context->mouseWentDown[FUI_MOUSE_RIGHT];
		bool dismissedByAPress = anyButtonWentDown && !context->menuOwnsTheMouse;
		if(fuiKeyWentDown(context, FUI_KEY_ESCAPE) || dismissedByAPress) {
			context->menuOpenDepth = 0;
		}

		// While a menu is up the interface owns the mouse EVERYWHERE, so the press that dismisses it never
		// also acts on whatever is behind it. Latching it here is what keeps the still held button from
		// leaking through the moment the popup vanishes from under the cursor.
		context->mouseIsOverUi = true;
		for(uint32_t buttonIndex = 0; buttonIndex < (uint32_t)FUI_MOUSE_BUTTON_COUNT; ++buttonIndex) {
			if(context->mouseWentDown[buttonIndex]) {
				context->mouseDownConsumed[buttonIndex] = true;
			}
		}
	}

	// A press that no widget took is a click on the background, which gives the keyboard back. Without
	// this a text field would keep swallowing keystrokes long after the user clicked away from it.
	if(context->mouseWentDown[FUI_MOUSE_LEFT] && !context->mouseDownConsumed[FUI_MOUSE_LEFT]) {
		context->focused = FUI_ID_NONE;
	}

	// Tab off the LAST focusable field wraps around to the first. The walk itself happens as the fields
	// are built, and reaching the end unconsumed is exactly the case that walk cannot see.
	bool tabWentUnanswered = fuiKeyWentDown(context, FUI_KEY_TAB) && !context->tabWasConsumedThisFrame;
	if(tabWentUnanswered && context->firstFocusableThisFrame != FUI_ID_NONE) {
		context->focused = context->firstFocusableThisFrame;
		context->tabWasConsumedThisFrame = true;
	}

	// A dialog owns the mouse EVERYWHERE it is not, exactly as an open menu does: the backdrop is there to
	// swallow the click that lands on it. Latched after the focus rule above, so clicking a dialog's empty
	// background still gives the keyboard back the way clicking any other background does.
	if(context->modalDepth > 0) {
		context->mouseIsOverUi = true;
		for(uint32_t buttonIndex = 0; buttonIndex < (uint32_t)FUI_MOUSE_BUTTON_COUNT; ++buttonIndex) {
			if(context->mouseWentDown[buttonIndex]) {
				context->mouseDownConsumed[buttonIndex] = true;
			}
		}
	}

	fui__DrawPendingTooltip(context);
	fui__ApplyCursor(context);

	// An unbalanced push must not leak into the next frame.
	context->idDepth = 0;
	context->hasSeenAFrame = true;

	fuiEndDrawFrame(context);
}

fui_api void fuiBeginDrawFrame(fuiContext *context, const fuiVec2i windowSize) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null || !context->isInitialized) {
		return;
	}

	context->windowSize = windowSize;
	context->vertexBuffer.count = 0;
	context->indexBuffer.count = 0;
	context->commandBuffer.count = 0;
	context->textBuffer.count = 0;
	context->clipDepth = 0;
	context->indexRangeExceeded = false;
	context->isBuildingFrame = true;

	// Everything the build itself opens starts empty. Which menus and dialogs are OPEN survives the frame;
	// which ones are being built does not, and an unbalanced end must not leak into the next frame either.
	context->menuStackDepth = 0;
	context->menuOwnsTheMouse = false;
	context->modalBuildDepth = 0;
	context->modalScopeId = FUI_ID_NONE;
	context->stripIsActive = false;

	// Default to the one-pass contract. fuiBeginFrame calls this first and then says what the pass really
	// is, so a caller reaching for the drawing half on its own can never land in a pass that draws nothing.
	context->pass = FUI_PASS_BOTH;
	context->isInteractive = true;

	// The ROOT container: the whole window, opened here and never closed by the caller. Without it a dock
	// taken at the TOP level would take its strip out of a box nothing remembers - fui__ParentBox would
	// hand every one of them the whole window again - so a menu bar, a status bar and a left panel docked
	// side by side would all start in the same corner. With it they bite out of one shrinking region, which
	// is what "dock" means. fuiLayoutRemaining reads the same box, so a caller that opens no container of
	// its own still gets the whole window on the first ask.
	fuiRect wholeWindow = fuiRectMake(0.0f, 0.0f, (float)windowSize.x, (float)windowSize.y);
	fuiLayoutNode *rootLayout = &context->layoutStack[0];
	fui__ClearMemory(rootLayout, sizeof(*rootLayout));
	rootLayout->axis = FUI_AXIS_VERTICAL;
	rootLayout->contentBox = wholeWindow;
	rootLayout->remaining = wholeWindow;
	context->layoutDepth = 1;
}

fui_api void fuiEndDrawFrame(fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null || !context->isInitialized) {
		return;
	}

	// An unbalanced push must not leak into the next frame, so both stacks are emptied rather than trusted.
	// The root container goes with them: the next frame opens its own.
	context->clipDepth = 0;
	context->layoutDepth = 0;
	context->isBuildingFrame = false;

	context->drawData.vertices = (const fuiVertex *)context->vertexBuffer.items;
	context->drawData.indices = (const fuiDrawIndex *)context->indexBuffer.items;
	context->drawData.commands = (const fuiDrawCommand *)context->commandBuffer.items;
	context->drawData.textBuffer = (const char *)context->textBuffer.items;
	context->drawData.vertexCount = context->vertexBuffer.count;
	context->drawData.indexCount = context->indexBuffer.count;
	context->drawData.commandCount = context->commandBuffer.count;
	context->drawData.textBufferSize = context->textBuffer.count;
	context->drawData.windowSize = context->windowSize;
}

fui_api void fuiSetDrawBatching(fuiContext *context, const bool enabled) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return;
	}
	context->drawBatchingIsOn = enabled;
}

fui_api const fuiDrawData *fuiGetDrawData(const fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return(fui_null);
	}
	const fuiDrawData *result = &context->drawData;
	return(result);
}

// ----------------------------------------------------------------------------
// > Clipping
// ----------------------------------------------------------------------------

fui_api fuiRect fuiGetClipRect(const fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		fuiRect empty = fuiRectMake(0.0f, 0.0f, 0.0f, 0.0f);
		return(empty);
	}
	if(context->clipDepth == 0) {
		fuiRect wholeWindow = fuiRectMake(0.0f, 0.0f, (float)context->windowSize.x, (float)context->windowSize.y);
		return(wholeWindow);
	}
	// A push past the maximum depth still counted, so that pops stay balanced. Reading the deepest
	// rectangle we did record keeps the clip conservative instead of losing it.
	uint32_t topIndex = (context->clipDepth <= (uint32_t)FUI_MAX_CLIP_DEPTH) ? (context->clipDepth - 1u) : ((uint32_t)FUI_MAX_CLIP_DEPTH - 1u);
	fuiRect result = context->clipStack[topIndex];
	return(result);
}

fui_api void fuiPushClip(fuiContext *context, const fuiRect rect) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return;
	}
	fuiRect current = fuiGetClipRect(context);
	fuiRect clipped = fuiRectIntersect(current, rect);
	if(context->clipDepth < (uint32_t)FUI_MAX_CLIP_DEPTH) {
		context->clipStack[context->clipDepth] = clipped;
	}
	context->clipDepth += 1u;
}

fui_api void fuiPopClip(fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return;
	}
	FUI_ASSERT(context->clipDepth > 0);
	if(context->clipDepth > 0) {
		context->clipDepth -= 1u;
	}
}

// ----------------------------------------------------------------------------
// > Interaction
// ----------------------------------------------------------------------------

//! True when a dialog is open and the code building right now is NOT inside the front one
fui_inline bool fui__ModalBlocksInput(const fuiContext *context) {
	if(context->modalDepth == 0) {
		return(false);
	}
	fuiId frontModal = context->modalStack[context->modalDepth - 1u];
	bool result = (context->modalScopeId != frontModal);
	return(result);
}

//! True when the cursor is inside a rectangle AND inside whatever is clipping it
fui_inline bool fui__CursorIsOver(const fuiContext *context, const fuiRect rect) {
	if(!context->inputIsActive) {
		return(false);
	}
	// A dialog owns the cursor while it is up. Everything built outside the front one - the panels beneath
	// it and any dialog stacked under it - is frozen, which is the whole of what modal means. Widgets built
	// INSIDE it carry its scope and so pass straight through. Gating it here rather than in each widget is
	// the same bargain the menus made: one place to be right, and nothing new to remember.
	if(fui__ModalBlocksInput(context)) {
		return(false);
	}
	// An open menu owns the cursor. Its popup floats above the panels but is BUILT last, so a widget under
	// it would take the very same click a moment before the row does - clicking File > Save would also press
	// whatever sits beneath the popup. The menu itself never asks through here, so gating ordinary widgets
	// leaves the menu live. It is what every desktop menu does: while one is open, a click anywhere else
	// only dismisses it.
	if(context->menuOpenDepth > 0) {
		return(false);
	}
	fuiRect clip = fuiGetClipRect(context);
	fuiRect visible = fuiRectIntersect(clip, rect);
	if(fuiRectIsEmpty(visible)) {
		return(false);
	}
	bool result = fuiPointInRect(context->mousePosition, visible);
	return(result);
}

fui_api fuiInteraction fuiInteract(fuiContext *context, const fuiId id, const fuiRect rect) {
	fuiInteraction result;
	fui__ClearMemory(&result, sizeof(result));

	FUI_ASSERT(context != fui_null);
	if(context == fui_null || !context->isBuildingFrame || id == FUI_ID_NONE) {
		return(result);
	}

	// Later calls overwrite the candidate, so a widget built on top of another takes the cursor from it.
	// This is why a popup drawn last wins over the panel underneath without any z ordering.
	bool cursorIsOverNow = fui__CursorIsOver(context, rect);
	if(cursorIsOverNow) {
		context->hotCandidate = id;
		context->mouseIsOverUi = true;
	}

	result.isHovered = (context->hot == id);
	result.isHeld = (context->active == id);

	bool canTakeThePress = (context->hot == id) && (context->active == FUI_ID_NONE) && context->mouseWentDown[FUI_MOUSE_LEFT];
	if(canTakeThePress) {
		context->active = id;
		context->focused = id;
		context->mouseDownConsumed[FUI_MOUSE_LEFT] = true;
		result.wasPressed = true;
		result.isHeld = true;
	}

	if((context->active == id) && context->mouseWentUp[FUI_MOUSE_LEFT]) {
		// Releasing away from the widget cancels the click, which is what lets a user change their mind
		// after pressing a button. This asks where the cursor is NOW rather than reading `hot`, which is a
		// frame behind: flicking off a button and releasing in one motion has to cancel, and reading the
		// stale answer there would fire it.
		result.wasClicked = cursorIsOverNow;
		context->active = FUI_ID_NONE;
		result.isHeld = false;
	}

	return(result);
}

fui_api void fuiBlockMouse(fuiContext *context, const fuiRect rect) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null || !context->isBuildingFrame) {
		return;
	}
	if(fui__CursorIsOver(context, rect)) {
		context->mouseIsOverUi = true;
	}
}

fui_api bool fuiWantsMouse(const fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return(false);
	}
	bool somethingIsHeld = (context->active != FUI_ID_NONE);
	bool pressWasTakenByUs = context->mouseDownConsumed[FUI_MOUSE_LEFT] || context->mouseDownConsumed[FUI_MOUSE_MIDDLE] || context->mouseDownConsumed[FUI_MOUSE_RIGHT];
	bool result = context->mouseIsOverUi || somethingIsHeld || pressWasTakenByUs;
	return(result);
}

fui_api bool fuiWantsKeyboard(const fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return(false);
	}
	bool result = (context->focused != FUI_ID_NONE);
	return(result);
}

fui_api fuiId fuiGetHotId(const fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	return((context != fui_null) ? context->hot : FUI_ID_NONE);
}

fui_api fuiId fuiGetActiveId(const fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	return((context != fui_null) ? context->active : FUI_ID_NONE);
}

fui_api fuiId fuiGetFocusedId(const fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	return((context != fui_null) ? context->focused : FUI_ID_NONE);
}

fui_api void fuiSetFocusedId(fuiContext *context, const fuiId id) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return;
	}
	context->focused = id;
}

fui_api fuiVec2 fuiGetMousePosition(const fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	return((context != fui_null) ? context->mousePosition : fuiV2(0.0f, 0.0f));
}

fui_api fuiVec2 fuiGetMouseDelta(const fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	return((context != fui_null) ? context->mouseDelta : fuiV2(0.0f, 0.0f));
}

fui_api float fuiGetMouseWheelDelta(const fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	return((context != fui_null) ? context->mouseWheelDelta : 0.0f);
}

fui_api bool fuiIsKeyDown(const fuiContext *context, const fuiKey key) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null || key <= FUI_KEY_NONE || key >= FUI_KEY_COUNT) {
		return(false);
	}
	bool result = fuiButtonIsDown(context->keys[key]);
	return(result);
}

fui_api bool fuiKeyWentDown(const fuiContext *context, const fuiKey key) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null || key <= FUI_KEY_NONE || key >= FUI_KEY_COUNT) {
		return(false);
	}
	bool result = fuiButtonWentDown(context->keys[key]);
	return(result);
}

fui_api bool fuiKeyRepeat(fuiContext *context, const fuiKey key) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null || key <= FUI_KEY_NONE || key >= FUI_KEY_COUNT) {
		return(false);
	}
	// Resolved once per frame rather than here, so asking twice in one build cannot step the timer twice
	// and make a held arrow key run at double speed.
	bool result = context->keyRepeated[key];
	return(result);
}

fui_api bool fuiIsControlDown(const fuiContext *context) {
	bool result = fuiIsKeyDown(context, FUI_KEY_LEFT_CONTROL) || fuiIsKeyDown(context, FUI_KEY_RIGHT_CONTROL);
	return(result);
}

fui_api bool fuiIsShiftDown(const fuiContext *context) {
	bool result = fuiIsKeyDown(context, FUI_KEY_LEFT_SHIFT) || fuiIsKeyDown(context, FUI_KEY_RIGHT_SHIFT);
	return(result);
}

fui_api bool fuiIsAltDown(const fuiContext *context) {
	bool result = fuiIsKeyDown(context, FUI_KEY_LEFT_ALT) || fuiIsKeyDown(context, FUI_KEY_RIGHT_ALT);
	return(result);
}

fui_api const uint32_t *fuiGetTextInput(const fuiContext *context, int32_t *outCount) {
	FUI_ASSERT(context != fui_null);
	if(outCount != fui_null) {
		*outCount = 0;
	}
	if(context == fui_null || context->textInputLength <= 0) {
		return(fui_null);
	}
	if(outCount != fui_null) {
		*outCount = context->textInputLength;
	}
	return(context->textInput);
}

// ----------------------------------------------------------------------------
// > Drawing
// ----------------------------------------------------------------------------

fui_api void fuiDrawRect(fuiContext *context, const fuiRect rect, const fuiColor color) {
	if(fuiRectIsEmpty(rect) || color.a <= 0.0f) {
		return;
	}
	if(fui__IsClippedAway(context, rect)) {
		return;
	}

	fui__GeometryWriter writer = fui__BeginGeometry(context, FUI_DRAW_RECT, FUI_TEXTURE_ID_NONE, 4u, 6u);
	if(!writer.isValid) {
		return;
	}
	uint32_t packedColor = fuiPackColor(color);
	fuiVec2 zeroUv = fui__ZeroUv();
	fui__WriteQuad(&writer, 0u, 0u, rect, zeroUv, zeroUv, packedColor);
	writer.command->payload.rect.rect = rect;
	writer.command->payload.rect.color = color;
}

fui_api void fuiDrawRectVerticalGradient(fuiContext *context, const fuiRect rect, const fuiColor topColor, const fuiColor bottomColor) {
	bool bothStopsAreInvisible = (topColor.a <= 0.0f) && (bottomColor.a <= 0.0f);
	if(fuiRectIsEmpty(rect) || bothStopsAreInvisible) {
		return;
	}
	if(fui__IsClippedAway(context, rect)) {
		return;
	}

	uint32_t packedTopColor = fuiPackColor(topColor);
	uint32_t packedBottomColor = fuiPackColor(bottomColor);
	// Two stops that round to the same bytes are a flat fill, and a flat fill is worth handing over as one:
	// a FUI_DRAW_RECT carries a payload a backend can take a shortcut on, where a gradient never can.
	if(packedTopColor == packedBottomColor) {
		fuiDrawRect(context, rect, topColor);
		return;
	}

	fui__GeometryWriter writer = fui__BeginGeometry(context, FUI_DRAW_TRIANGLES, FUI_TEXTURE_ID_NONE, 4u, 6u);
	if(!writer.isValid) {
		return;
	}
	fui__WriteQuadVerticalGradient(&writer, 0u, 0u, rect, packedTopColor, packedBottomColor);
}

fui_api void fuiDrawRectOutline(fuiContext *context, const fuiRect rect, const fuiColor color, const float thickness) {
	if(fuiRectIsEmpty(rect) || color.a <= 0.0f || thickness <= 0.0f) {
		return;
	}
	if(fui__IsClippedAway(context, rect)) {
		return;
	}

	// Too thin to have a hole left in the middle, so it is really a fill.
	float maximumThickness = fuiMinF(rect.w, rect.h) * 0.5f;
	if(thickness >= maximumThickness) {
		fuiDrawRect(context, rect, color);
		return;
	}

	fui__GeometryWriter writer = fui__BeginGeometry(context, FUI_DRAW_RECT_OUTLINE, FUI_TEXTURE_ID_NONE, 16u, 24u);
	if(!writer.isValid) {
		return;
	}

	uint32_t packedColor = fuiPackColor(color);
	fuiVec2 zeroUv = fui__ZeroUv();
	float innerHeight = rect.h - thickness * 2.0f;

	fuiRect topEdge = fuiRectMake(rect.x, rect.y, rect.w, thickness);
	fuiRect bottomEdge = fuiRectMake(rect.x, rect.y + rect.h - thickness, rect.w, thickness);
	fuiRect leftEdge = fuiRectMake(rect.x, rect.y + thickness, thickness, innerHeight);
	fuiRect rightEdge = fuiRectMake(rect.x + rect.w - thickness, rect.y + thickness, thickness, innerHeight);

	fui__WriteQuad(&writer, 0u, 0u, topEdge, zeroUv, zeroUv, packedColor);
	fui__WriteQuad(&writer, 4u, 6u, bottomEdge, zeroUv, zeroUv, packedColor);
	fui__WriteQuad(&writer, 8u, 12u, leftEdge, zeroUv, zeroUv, packedColor);
	fui__WriteQuad(&writer, 12u, 18u, rightEdge, zeroUv, zeroUv, packedColor);

	writer.command->payload.rectOutline.rect = rect;
	writer.command->payload.rectOutline.thickness = thickness;
	writer.command->payload.rectOutline.color = color;
}

//! Writes the four corners of a thick line segment into a quad
fui_inline void fui__WriteLineQuad(fui__GeometryWriter *writer, const uint32_t vertexAt, const uint32_t indexAt, const fuiVec2 start, const fuiVec2 end, const float thickness, const uint32_t packedColor) {
	float deltaX = end.x - start.x;
	float deltaY = end.y - start.y;
	float lengthSquared = deltaX * deltaX + deltaY * deltaY;
	float length = FUI_SQRTF(lengthSquared);
	float inverseLength = (length > 0.0f) ? (1.0f / length) : 0.0f;
	float directionX = deltaX * inverseLength;
	float directionY = deltaY * inverseLength;
	float halfThickness = thickness * 0.5f;
	float normalX = -directionY * halfThickness;
	float normalY = directionX * halfThickness;

	fuiVec2 zeroUv = fui__ZeroUv();
	fuiVertex *vertices = &writer->vertices[vertexAt];
	vertices[0].position = fuiV2(start.x + normalX, start.y + normalY);
	vertices[0].uv = zeroUv;
	vertices[0].color = packedColor;
	vertices[1].position = fuiV2(end.x + normalX, end.y + normalY);
	vertices[1].uv = zeroUv;
	vertices[1].color = packedColor;
	vertices[2].position = fuiV2(end.x - normalX, end.y - normalY);
	vertices[2].uv = zeroUv;
	vertices[2].color = packedColor;
	vertices[3].position = fuiV2(start.x - normalX, start.y - normalY);
	vertices[3].uv = zeroUv;
	vertices[3].color = packedColor;

	fuiDrawIndex base = (fuiDrawIndex)(writer->baseVertex + vertexAt);
	fuiDrawIndex *indices = &writer->indices[indexAt];
	indices[0] = base;
	indices[1] = (fuiDrawIndex)(base + 1u);
	indices[2] = (fuiDrawIndex)(base + 2u);
	indices[3] = base;
	indices[4] = (fuiDrawIndex)(base + 2u);
	indices[5] = (fuiDrawIndex)(base + 3u);
}

fui_api void fuiDrawLine(fuiContext *context, const fuiVec2 start, const fuiVec2 end, const fuiColor color, const float thickness) {
	if(color.a <= 0.0f || thickness <= 0.0f) {
		return;
	}
	float deltaX = end.x - start.x;
	float deltaY = end.y - start.y;
	if(deltaX == 0.0f && deltaY == 0.0f) {
		return;
	}

	float halfThickness = thickness * 0.5f;
	fuiVec2 boundsMin = fuiV2(fuiMinF(start.x, end.x) - halfThickness, fuiMinF(start.y, end.y) - halfThickness);
	fuiVec2 boundsMax = fuiV2(fuiMaxF(start.x, end.x) + halfThickness, fuiMaxF(start.y, end.y) + halfThickness);
	fuiRect bounds = fuiRectMinMax(boundsMin, boundsMax);
	if(fui__IsClippedAway(context, bounds)) {
		return;
	}

	fui__GeometryWriter writer = fui__BeginGeometry(context, FUI_DRAW_LINE, FUI_TEXTURE_ID_NONE, 4u, 6u);
	if(!writer.isValid) {
		return;
	}
	uint32_t packedColor = fuiPackColor(color);
	fui__WriteLineQuad(&writer, 0u, 0u, start, end, thickness, packedColor);

	writer.command->payload.line.start = start;
	writer.command->payload.line.end = end;
	writer.command->payload.line.thickness = thickness;
	writer.command->payload.line.color = color;
}

//! Bounding rectangle of a point list
fui_inline fuiRect fui__PointsBounds(const fuiVec2 *points, const uint32_t pointCount, const float margin) {
	FUI_ASSERT(points != fui_null && pointCount > 0);
	float minimumX = points[0].x;
	float minimumY = points[0].y;
	float maximumX = points[0].x;
	float maximumY = points[0].y;
	for(uint32_t index = 1; index < pointCount; ++index) {
		minimumX = fuiMinF(minimumX, points[index].x);
		minimumY = fuiMinF(minimumY, points[index].y);
		maximumX = fuiMaxF(maximumX, points[index].x);
		maximumY = fuiMaxF(maximumY, points[index].y);
	}
	fuiVec2 boundsMin = fuiV2(minimumX - margin, minimumY - margin);
	fuiVec2 boundsMax = fuiV2(maximumX + margin, maximumY + margin);
	fuiRect result = fuiRectMinMax(boundsMin, boundsMax);
	return(result);
}

fui_api void fuiDrawPolygon(fuiContext *context, const fuiVec2 *points, const uint32_t pointCount, const fuiColor color) {
	if(points == fui_null || pointCount < 3u || color.a <= 0.0f) {
		return;
	}
	fuiRect bounds = fui__PointsBounds(points, pointCount, 0.0f);
	if(fui__IsClippedAway(context, bounds)) {
		return;
	}

	uint32_t triangleCount = pointCount - 2u;
	uint32_t indexCount = triangleCount * 3u;
	fui__GeometryWriter writer = fui__BeginGeometry(context, FUI_DRAW_TRIANGLES, FUI_TEXTURE_ID_NONE, pointCount, indexCount);
	if(!writer.isValid) {
		return;
	}

	uint32_t packedColor = fuiPackColor(color);
	fuiVec2 zeroUv = fui__ZeroUv();
	for(uint32_t index = 0; index < pointCount; ++index) {
		writer.vertices[index].position = points[index];
		writer.vertices[index].uv = zeroUv;
		writer.vertices[index].color = packedColor;
	}

	// A fan around the first point, which is why the polygon has to be convex.
	for(uint32_t triangle = 0; triangle < triangleCount; ++triangle) {
		fuiDrawIndex base = (fuiDrawIndex)writer.baseVertex;
		writer.indices[triangle * 3u + 0u] = base;
		writer.indices[triangle * 3u + 1u] = (fuiDrawIndex)(base + triangle + 1u);
		writer.indices[triangle * 3u + 2u] = (fuiDrawIndex)(base + triangle + 2u);
	}
}

fui_api void fuiDrawPolygonOutline(fuiContext *context, const fuiVec2 *points, const uint32_t pointCount, const fuiColor color, const float thickness) {
	if(points == fui_null || pointCount < 2u || color.a <= 0.0f || thickness <= 0.0f) {
		return;
	}
	fuiRect bounds = fui__PointsBounds(points, pointCount, thickness * 0.5f);
	if(fui__IsClippedAway(context, bounds)) {
		return;
	}

	uint32_t segmentCount = pointCount;
	fui__GeometryWriter writer = fui__BeginGeometry(context, FUI_DRAW_TRIANGLES, FUI_TEXTURE_ID_NONE, segmentCount * 4u, segmentCount * 6u);
	if(!writer.isValid) {
		return;
	}

	uint32_t packedColor = fuiPackColor(color);
	uint32_t usedVertexCount = 0;
	uint32_t usedIndexCount = 0;
	for(uint32_t segment = 0; segment < segmentCount; ++segment) {
		fuiVec2 start = points[segment];
		fuiVec2 end = points[(segment + 1u) % pointCount];
		if(start.x == end.x && start.y == end.y) {
			continue;
		}
		fui__WriteLineQuad(&writer, usedVertexCount, usedIndexCount, start, end, thickness, packedColor);
		usedVertexCount += 4u;
		usedIndexCount += 6u;
	}
	fui__TrimGeometry(context, &writer, usedVertexCount, usedIndexCount);
}

//! The texture coordinate of each of a quad's four corners, in the order the quad writer lays them down:
//! top-left, top-right, bottom-right, bottom-left. A quarter turn is a rotation of that order.
fui_inline void fui__ImageCornerUvs(const fuiVec2 uvMin, const fuiVec2 uvMax, const fuiImageFlags flags, fuiVec2 *outCornerUvs) {
	bool flipU = (flags & FUI_IMAGE_FLIP_U) == FUI_IMAGE_FLIP_U;
	bool flipV = (flags & FUI_IMAGE_FLIP_V) == FUI_IMAGE_FLIP_V;
	bool rotateClockwise = (flags & FUI_IMAGE_ROTATE_90_CW) == FUI_IMAGE_ROTATE_90_CW;
	bool rotateCounterClockwise = (flags & FUI_IMAGE_ROTATE_90_CCW) == FUI_IMAGE_ROTATE_90_CCW;

	// Mirroring first, by swapping the edge each side samples from, and the quarter turn on top of that -
	// the same order the sprite backends this mirrors apply them in.
	float left = flipU ? uvMax.x : uvMin.x;
	float right = flipU ? uvMin.x : uvMax.x;
	float top = flipV ? uvMax.y : uvMin.y;
	float bottom = flipV ? uvMin.y : uvMax.y;

	// Turning both ways at once is turning neither way, which is the answer that costs a caller combining
	// flag sets nothing. It is also what the sprite backends already answer.
	bool isTurned = rotateClockwise != rotateCounterClockwise;
	if(!isTurned) {
		outCornerUvs[0] = fuiV2(left, top);
		outCornerUvs[1] = fuiV2(right, top);
		outCornerUvs[2] = fuiV2(right, bottom);
		outCornerUvs[3] = fuiV2(left, bottom);
	} else if(rotateClockwise) {
		// The top-left of the source comes out at the top-RIGHT of the quad, which is what a clockwise turn is.
		outCornerUvs[0] = fuiV2(left, bottom);
		outCornerUvs[1] = fuiV2(left, top);
		outCornerUvs[2] = fuiV2(right, top);
		outCornerUvs[3] = fuiV2(right, bottom);
	} else {
		outCornerUvs[0] = fuiV2(right, top);
		outCornerUvs[1] = fuiV2(right, bottom);
		outCornerUvs[2] = fuiV2(left, bottom);
		outCornerUvs[3] = fuiV2(left, top);
	}
}

fui_api void fuiDrawImageEx(fuiContext *context, const fuiRect rect, const fuiTextureId texture, const fuiVec2 uvMin, const fuiVec2 uvMax, const fuiColor color, const fuiImageFlags flags) {
	if(fuiRectIsEmpty(rect) || color.a <= 0.0f) {
		return;
	}
	if(fui__IsClippedAway(context, rect)) {
		return;
	}

	fui__GeometryWriter writer = fui__BeginGeometry(context, FUI_DRAW_IMAGE, texture, 4u, 6u);
	if(!writer.isValid) {
		return;
	}
	uint32_t packedColor = fuiPackColor(color);
	fuiVec2 cornerUvs[4];
	fui__ImageCornerUvs(uvMin, uvMax, flags, cornerUvs);
	fui__WriteQuadUvCorners(&writer, 0u, 0u, rect, cornerUvs, packedColor);

	// The payload keeps the uv rectangle as it was HANDED IN, with the turn beside it in the flags, so a
	// backend taking the sprite fast path has the two halves its own sprite call wants. The vertices above
	// already carry the turn, so a backend drawing triangles never looks at the flags at all.
	writer.command->payload.image.rect = rect;
	writer.command->payload.image.uvMin = uvMin;
	writer.command->payload.image.uvMax = uvMax;
	writer.command->payload.image.color = color;
	writer.command->payload.image.flags = flags;
}

fui_api void fuiDrawImage(fuiContext *context, const fuiRect rect, const fuiTextureId texture, const fuiVec2 uvMin, const fuiVec2 uvMax, const fuiColor color) {
	fuiDrawImageEx(context, rect, texture, uvMin, uvMax, color, FUI_IMAGE_FLAGS_NONE);
}

// ----------------------------------------------------------------------------
// > Text
// ----------------------------------------------------------------------------

//! Codepoint substituted for a byte sequence that is not valid UTF-8
#define FUI__REPLACEMENT_CODEPOINT 0xFFFDu

fui_api uint32_t fuiDecodeUtf8(const char *text, const size_t textLength, size_t *inOutOffset) {
	FUI_ASSERT(text != fui_null && inOutOffset != fui_null);
	if(text == fui_null || inOutOffset == fui_null) {
		return(FUI__REPLACEMENT_CODEPOINT);
	}
	size_t offset = *inOutOffset;
	if(offset >= textLength) {
		return(FUI__REPLACEMENT_CODEPOINT);
	}

	unsigned char firstByte = (unsigned char)text[offset];
	uint32_t codePoint = 0;
	size_t extraByteCount = 0;

	if(firstByte < 0x80u) {
		*inOutOffset = offset + 1u;
		return((uint32_t)firstByte);
	} else if((firstByte & 0xE0u) == 0xC0u) {
		codePoint = (uint32_t)(firstByte & 0x1Fu);
		extraByteCount = 1;
	} else if((firstByte & 0xF0u) == 0xE0u) {
		codePoint = (uint32_t)(firstByte & 0x0Fu);
		extraByteCount = 2;
	} else if((firstByte & 0xF8u) == 0xF0u) {
		codePoint = (uint32_t)(firstByte & 0x07u);
		extraByteCount = 3;
	} else {
		// A stray continuation byte or an invalid lead. Advance by one so a decoding loop cannot stall.
		*inOutOffset = offset + 1u;
		return(FUI__REPLACEMENT_CODEPOINT);
	}

	if((offset + extraByteCount) >= textLength) {
		*inOutOffset = offset + 1u;
		return(FUI__REPLACEMENT_CODEPOINT);
	}
	for(size_t extraIndex = 1; extraIndex <= extraByteCount; ++extraIndex) {
		unsigned char continuationByte = (unsigned char)text[offset + extraIndex];
		if((continuationByte & 0xC0u) != 0x80u) {
			*inOutOffset = offset + 1u;
			return(FUI__REPLACEMENT_CODEPOINT);
		}
		codePoint = (codePoint << 6) | (uint32_t)(continuationByte & 0x3Fu);
	}

	// Reject surrogates and anything past the last plane, both of which are invalid UTF-8.
	bool isSurrogate = (codePoint >= 0xD800u) && (codePoint <= 0xDFFFu);
	if(isSurrogate || codePoint > 0x10FFFFu) {
		*inOutOffset = offset + 1u;
		return(FUI__REPLACEMENT_CODEPOINT);
	}

	*inOutOffset = offset + extraByteCount + 1u;
	return(codePoint);
}

fui_api uint32_t fuiEncodeUtf8(const uint32_t codePoint, char *outBytes) {
	FUI_ASSERT(outBytes != fui_null);
	if(outBytes == fui_null) {
		return(0);
	}
	// A surrogate half is not a character, and nothing above the last plane exists at all. Refusing both
	// here is what keeps an encode of a decode from ever producing bytes the decoder would reject.
	bool isSurrogate = (codePoint >= 0xD800u) && (codePoint <= 0xDFFFu);
	if(isSurrogate || codePoint > 0x10FFFFu) {
		return(0);
	}
	if(codePoint < 0x80u) {
		outBytes[0] = (char)codePoint;
		return(1u);
	}
	if(codePoint < 0x800u) {
		outBytes[0] = (char)(0xC0u | (codePoint >> 6));
		outBytes[1] = (char)(0x80u | (codePoint & 0x3Fu));
		return(2u);
	}
	if(codePoint < 0x10000u) {
		outBytes[0] = (char)(0xE0u | (codePoint >> 12));
		outBytes[1] = (char)(0x80u | ((codePoint >> 6) & 0x3Fu));
		outBytes[2] = (char)(0x80u | (codePoint & 0x3Fu));
		return(3u);
	}
	outBytes[0] = (char)(0xF0u | (codePoint >> 18));
	outBytes[1] = (char)(0x80u | ((codePoint >> 12) & 0x3Fu));
	outBytes[2] = (char)(0x80u | ((codePoint >> 6) & 0x3Fu));
	outBytes[3] = (char)(0x80u | (codePoint & 0x3Fu));
	return(4u);
}

//! Resolves the "pass 0 to measure it yourself" convention every text entry point takes
fui_inline size_t fui__ResolveTextLength(const char *text, const size_t textLength) {
	if(text == fui_null) {
		return(0);
	}
	size_t result = (textLength > 0) ? textLength : fui__StringLength(text);
	return(result);
}

//! How tall one line of text stands, which is a property of the FONT and never of the string. Asking a
//! measurement for it used to walk the whole string for a number two metrics already hold
fui_inline float fui__LineExtent(const fuiFont *font, const float pixelHeight) {
	float result = (font->metrics.ascent + font->metrics.descent) * pixelHeight;
	return(result);
}

//! How far the pen moves for one codepoint, including the kerning against the one before it
fui_inline float fui__CodepointAdvance(const fuiFont *font, const uint32_t codePoint, const uint32_t previousCodePoint, const float pixelHeight, fuiGlyph *outGlyph, bool *outHasGlyph) {
	FUI_ASSERT(font != fui_null);
	float kerning = 0.0f;
	if(font->getKerning != fui_null && previousCodePoint != 0) {
		kerning = font->getKerning(font->userData, previousCodePoint, codePoint);
	}

	fuiGlyph glyph;
	bool hasGlyph = false;
	if(font->getGlyph != fui_null) {
		hasGlyph = font->getGlyph(font->userData, codePoint, &glyph);
	}
	if(!hasGlyph) {
		// Cleared only when the callback did not fill it. This runs once per character of every string
		// the interface measures, and a hit has no reason to pay for a clear it immediately overwrites.
		fui__ClearMemory(&glyph, sizeof(glyph));
	}

	if(outGlyph != fui_null) {
		*outGlyph = glyph;
	}
	if(outHasGlyph != fui_null) {
		*outHasGlyph = hasGlyph;
	}

	// A codepoint the font has no glyph for still takes up room, or missing characters would silently
	// close up the line and every caret offset behind them would be wrong.
	float advance = hasGlyph ? glyph.advance : font->metrics.spaceAdvance;
	float result = (kerning + advance) * pixelHeight;
	return(result);
}

fui_api float fuiGetLineHeight(const fuiContext *context, const float pixelHeight) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null || context->font == fui_null) {
		return(0.0f);
	}
	float result = context->font->metrics.lineHeight * pixelHeight;
	return(result);
}

fui_api fuiVec2 fuiMeasureText(const fuiContext *context, const char *text, const size_t textLength, const float pixelHeight) {
	FUI_ASSERT(context != fui_null);
	fuiVec2 result = fuiV2(0.0f, 0.0f);
	if(context == fui_null || context->font == fui_null || text == fui_null) {
		return(result);
	}

	const fuiFont *font = context->font;
	size_t resolvedLength = fui__ResolveTextLength(text, textLength);
	float height = (font->metrics.ascent + font->metrics.descent) * pixelHeight;
	if(resolvedLength == 0) {
		result = fuiV2(0.0f, height);
		return(result);
	}

	if(font->measureText != fui_null) {
		float width = font->measureText(font->userData, text, resolvedLength, pixelHeight);
		result = fuiV2(width, height);
		return(result);
	}

	float penX = 0.0f;
	uint32_t previousCodePoint = 0;
	size_t offset = 0;
	while(offset < resolvedLength) {
		uint32_t codePoint = fuiDecodeUtf8(text, resolvedLength, &offset);
		if(codePoint == (uint32_t)'\n' || codePoint == (uint32_t)'\r') {
			continue;
		}
		penX += fui__CodepointAdvance(font, codePoint, previousCodePoint, pixelHeight, fui_null, fui_null);
		previousCodePoint = codePoint;
	}

	result = fuiV2(penX, height);
	return(result);
}

fui_api void fuiDrawText(fuiContext *context, const char *text, const size_t textLength, const fuiVec2 position, const float pixelHeight, const fuiColor color) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null || context->font == fui_null || text == fui_null || color.a <= 0.0f || pixelHeight <= 0.0f) {
		return;
	}

	const fuiFont *font = context->font;
	size_t resolvedLength = fui__ResolveTextLength(text, textLength);
	if(resolvedLength == 0) {
		return;
	}

	/*
		Whether any of this can be seen is answered from the clip and two font metrics, without measuring
		the string. The measurement that used to stand here was one glyph lookup and one kerning lookup
		per character, thrown away as soon as it had produced a rectangle - and the caller had almost
		always measured the same string already to work out where to put it.
	*/
	float lineHeight = fui__LineExtent(font, pixelHeight);
	fuiRect clip = fuiGetClipRect(context);
	float clipRight = clip.x + clip.w;
	float clipBottom = clip.y + clip.h;
	bool isAboveTheClip = (position.y + lineHeight) <= clip.y;
	bool isBelowTheClip = position.y >= clipBottom;
	bool startsRightOfTheClip = position.x >= clipRight;
	if(isAboveTheClip || isBelowTheClip || startsRightOfTheClip) {
		return;
	}

	// One quad per BYTE is an upper bound on one quad per codepoint, which is what lets the reservation
	// skip a counting pass over the whole string. Everything the blanks, the control characters and the
	// culling below did not use is handed straight back by the trim.
	uint32_t reservedQuadCount = (uint32_t)resolvedLength;

	uint32_t textByteOffset = context->textBuffer.count;
	bool textReserved = fui__DrawBufferReserve(&context->arena, &context->textBuffer, textByteOffset + (uint32_t)resolvedLength, sizeof(char));
	if(!textReserved) {
		return;
	}

	fui__GeometryWriter writer = fui__BeginGeometry(context, FUI_DRAW_TEXT, font->atlasTexture, reservedQuadCount * 4u, reservedQuadCount * 6u);
	if(!writer.isValid) {
		return;
	}

	char *textBytes = (char *)context->textBuffer.items;
	fui__CopyMemory(&textBytes[textByteOffset], text, resolvedLength);
	context->textBuffer.count = textByteOffset + (uint32_t)resolvedLength;

	uint32_t packedColor = fuiPackColor(color);
	float baselineY = position.y + font->metrics.ascent * pixelHeight;
	float penX = position.x;
	uint32_t previousCodePoint = 0;
	uint32_t usedVertexCount = 0;
	uint32_t usedIndexCount = 0;
	size_t offset = 0;
	while(offset < resolvedLength) {
		// The pen only ever moves right, so once it is past the clip nothing after this can be seen
		// either. A cell holding a long path in a narrow column stops after the dozen glyphs that fit
		// instead of tessellating the whole string into triangles the scissor throws away.
		if(penX >= clipRight) {
			break;
		}

		uint32_t codePoint = fuiDecodeUtf8(text, resolvedLength, &offset);
		if(codePoint == (uint32_t)'\n' || codePoint == (uint32_t)'\r') {
			continue;
		}

		fuiGlyph glyph;
		bool hasGlyph = false;
		float advance = fui__CodepointAdvance(font, codePoint, previousCodePoint, pixelHeight, &glyph, &hasGlyph);
		bool hasQuad = hasGlyph && (glyph.size.x > 0.0f) && (glyph.size.y > 0.0f);
		if(hasQuad) {
			float quadX = penX + glyph.offset.x * pixelHeight;
			float quadWidth = glyph.size.x * pixelHeight;
			// The other end of the same idea: a row scrolled sideways is mostly glyphs off its left edge,
			// and those cost nothing here beyond the advance that has to be summed anyway.
			bool isLeftOfTheClip = (quadX + quadWidth) <= clip.x;
			if(!isLeftOfTheClip) {
				float quadY = baselineY + glyph.offset.y * pixelHeight;
				float quadHeight = glyph.size.y * pixelHeight;
				fuiRect quad = fuiRectMake(quadX, quadY, quadWidth, quadHeight);
				fui__WriteQuad(&writer, usedVertexCount, usedIndexCount, quad, glyph.uvMin, glyph.uvMax, packedColor);
				usedVertexCount += 4u;
				usedIndexCount += 6u;
			}
		}

		penX += advance;
		previousCodePoint = codePoint;
	}

	fui__TrimGeometry(context, &writer, usedVertexCount, usedIndexCount);
	if(!writer.isValid) {
		// Every codepoint was blank, so the text bytes are not referenced by anything anymore.
		context->textBuffer.count = textByteOffset;
		return;
	}

	writer.command->payload.text.textOffset = textByteOffset;
	writer.command->payload.text.textLength = (uint32_t)resolvedLength;
	writer.command->payload.text.position = position;
	writer.command->payload.text.pixelHeight = pixelHeight;
	writer.command->payload.text.color = color;
}

fui_api bool fuiTruncateTextToWidth(const fuiContext *context, const char *text, const size_t textLength, const float maxWidth, const float pixelHeight, char *buffer, const size_t bufferCapacity) {
	FUI_ASSERT(context != fui_null);
	if(buffer == fui_null || bufferCapacity == 0) {
		return(false);
	}
	buffer[0] = 0;
	if(context == fui_null || context->font == fui_null || text == fui_null) {
		return(false);
	}

	size_t resolvedLength = fui__ResolveTextLength(text, textLength);
	fuiVec2 fullSize = fuiMeasureText(context, text, resolvedLength, pixelHeight);
	if(fullSize.x <= maxWidth) {
		size_t copyLength = (resolvedLength < (bufferCapacity - 1)) ? resolvedLength : (bufferCapacity - 1);
		fui__CopyMemory(buffer, text, copyLength);
		buffer[copyLength] = 0;
		return(copyLength < resolvedLength);
	}

	const char *ellipsis = "...";
	fuiVec2 ellipsisSize = fuiMeasureText(context, ellipsis, 3, pixelHeight);
	float availableWidth = maxWidth - ellipsisSize.x;

	const fuiFont *font = context->font;
	float penX = 0.0f;
	uint32_t previousCodePoint = 0;
	size_t offset = 0;
	size_t lastFittingOffset = 0;
	while(offset < resolvedLength) {
		size_t codePointStart = offset;
		uint32_t codePoint = fuiDecodeUtf8(text, resolvedLength, &offset);
		float advance = fui__CodepointAdvance(font, codePoint, previousCodePoint, pixelHeight, fui_null, fui_null);
		if((penX + advance) > availableWidth) {
			lastFittingOffset = codePointStart;
			break;
		}
		penX += advance;
		previousCodePoint = codePoint;
		lastFittingOffset = offset;
	}

	size_t keptLength = (lastFittingOffset < (bufferCapacity - 1)) ? lastFittingOffset : (bufferCapacity - 1);
	fui__CopyMemory(buffer, text, keptLength);
	buffer[keptLength] = 0;
	fui__AppendString(buffer, bufferCapacity, ellipsis);
	return(true);
}

/*
	Where a line breaker puts its lines.

	A text field showing part of a large document needs a WINDOW of the lines rather than the first few of
	them, so this counts every line the text has and stores only the ones the window asked for. When
	nothing wants the total, it says so by asking to stop as soon as the window is full - which is what
	every caller that just wants a tooltip broken into rows does, and what keeps that cheap.
*/
typedef struct fui__TextLineSink {
	//! Where the stored lines go, or null to count only
	fuiTextLine *lines;
	//! How many of them fit
	uint32_t capacity;
	//! Which line the storing starts at, so the window can begin part way into the text
	uint32_t firstStoredLine;
	//! How many lines the text has had so far, stored or not
	uint32_t totalCount;
	//! How many are really in the array
	uint32_t storedCount;
	//! Whether counting goes on after the array is full, which is what a scrollbar needs and a tooltip does not
	bool countsPastTheWindow;
	//! Start of the text, so a byte offset into it can be compared against the line being emitted
	const char *textBase;
	//! Byte offset to find the line of, or (size_t)-1 when nothing is being looked for
	size_t offsetToFind;
	//! Which line that offset landed on, meaningful once offsetWasFound
	uint32_t offsetLine;
	//! Whether the offset has been reached yet
	bool offsetWasFound;
} fui__TextLineSink;

//! Takes one visual line, returning false once there is no reason to keep breaking
fui_inline bool fui__AppendTextLine(fui__TextLineSink *sink, const char *start, const size_t length) {
	uint32_t lineIndex = sink->totalCount;
	sink->totalCount += 1u;

	bool isInsideTheWindow = (lineIndex >= sink->firstStoredLine) && (sink->storedCount < sink->capacity);
	if(isInsideTheWindow && sink->lines != fui_null) {
		sink->lines[sink->storedCount].start = start;
		sink->lines[sink->storedCount].length = length;
		sink->storedCount += 1u;
	}

	// A caret sitting somewhere in a document the field is not showing needs to know which LINE it is on,
	// and the only thing that knows where the lines fall is the breaker itself. A boundary belongs to the
	// line that ends there, which is the same rule fui__CaretRowIndex follows.
	if(!sink->offsetWasFound && sink->offsetToFind != (size_t)-1) {
		size_t lineStartOffset = (size_t)(start - sink->textBase);
		if(sink->offsetToFind <= (lineStartOffset + length)) {
			sink->offsetLine = lineIndex;
			sink->offsetWasFound = true;
		}
	}

	bool windowIsFull = (sink->storedCount >= sink->capacity);
	bool nothingLeftToLookFor = sink->offsetWasFound || (sink->offsetToFind == (size_t)-1);
	if(windowIsFull && !sink->countsPastTheWindow && nothingLeftToLookFor) {
		return(false);
	}
	return(true);
}

//! The whole line breaker, working into a sink so one implementation serves the plain call and the window
//! a scrolling text field needs
fui_inline void fui__BreakTextLinesInto(const fuiContext *context, const char *text, const size_t textLength, const float pixelHeight, const bool wordWrap, const float wrapWidth, fui__TextLineSink *sink) {
	if(context == fui_null || text == fui_null) {
		(void)fui__AppendTextLine(sink, "", 0);
		return;
	}

	size_t resolvedLength = fui__ResolveTextLength(text, textLength);
	const fuiFont *font = context->font;
	bool canWrap = wordWrap && (font != fui_null) && (wrapWidth > 0.0f);

	if(!canWrap) {
		/*
			Without wrapping a line is exactly what the newlines say it is, so the lines can be FOUND
			rather than walked to. That matters far more than it looks: a field showing the ten thousandth
			line of a document has to get past the first nine thousand nine hundred and ninety nine of
			them first, and doing that one decoded codepoint at a time is the difference between a text
			field that can hold a file and one that cannot.
		*/
		size_t scanStart = 0;
		while(scanStart <= resolvedLength) {
			const char *newline = (const char *)FUI_MEMCHR(&text[scanStart], '\n', resolvedLength - scanStart);
			size_t lineEnd = (newline != fui_null) ? (size_t)(newline - text) : resolvedLength;
			if(!fui__AppendTextLine(sink, &text[scanStart], lineEnd - scanStart)) {
				return;
			}
			if(newline == fui_null) {
				return;
			}
			scanStart = lineEnd + 1;
		}
		return;
	}

	size_t lineStart = 0;
	size_t offset = 0;
	float lineWidth = 0.0f;
	uint32_t previousCodePoint = 0;
	size_t lastSpaceStart = (size_t)-1;
	size_t lastSpaceEnd = (size_t)-1;

	while(offset < resolvedLength) {
		size_t codePointStart = offset;
		uint32_t codePoint = fuiDecodeUtf8(text, resolvedLength, &offset);

		if(codePoint == (uint32_t)'\n') {
			if(!fui__AppendTextLine(sink, &text[lineStart], codePointStart - lineStart)) {
				return;
			}
			lineStart = offset;
			lineWidth = 0.0f;
			previousCodePoint = 0;
			lastSpaceStart = (size_t)-1;
			lastSpaceEnd = (size_t)-1;
			continue;
		}
		if(codePoint == (uint32_t)'\r') {
			continue;
		}

		// A space is a break OPPORTUNITY, never content that overflows: letting one trigger the break
		// would cut the line before the space is even recorded, and the next line would then start with
		// it. Trailing spaces are free, exactly as every text layout engine treats them.
		bool isSpace = (codePoint == (uint32_t)' ');
		float advance = canWrap ? fui__CodepointAdvance(font, codePoint, previousCodePoint, pixelHeight, fui_null, fui_null) : 0.0f;
		bool overflows = canWrap && !isSpace && ((lineWidth + advance) > wrapWidth) && (codePointStart > lineStart);
		if(overflows) {
			if(lastSpaceStart != (size_t)-1) {
				// Break at the last space, and drop the space itself rather than start the next line with it.
				if(!fui__AppendTextLine(sink, &text[lineStart], lastSpaceStart - lineStart)) {
					return;
				}
				lineStart = lastSpaceEnd;
			} else {
				// One word longer than the whole wrap width, so it has to be cut mid-word.
				if(!fui__AppendTextLine(sink, &text[lineStart], codePointStart - lineStart)) {
					return;
				}
				lineStart = codePointStart;
			}
			lastSpaceStart = (size_t)-1;
			lastSpaceEnd = (size_t)-1;

			// Re-measure what is now on the new line, which is at most one word long.
			lineWidth = 0.0f;
			previousCodePoint = 0;
			size_t remeasureOffset = lineStart;
			while(remeasureOffset < offset) {
				uint32_t remeasuredCodePoint = fuiDecodeUtf8(text, resolvedLength, &remeasureOffset);
				lineWidth += fui__CodepointAdvance(font, remeasuredCodePoint, previousCodePoint, pixelHeight, fui_null, fui_null);
				previousCodePoint = remeasuredCodePoint;
			}
			continue;
		}

		if(isSpace) {
			lastSpaceStart = codePointStart;
			lastSpaceEnd = offset;
		}
		lineWidth += advance;
		previousCodePoint = codePoint;
	}

	// Always emit a final line, so empty text still gives a caret somewhere to sit and a trailing
	// newline still produces the empty line it asked for.
	(void)fui__AppendTextLine(sink, &text[lineStart], resolvedLength - lineStart);
}

fui_api uint32_t fuiBreakTextLinesFrom(const fuiContext *context, const char *text, const size_t textLength, const float pixelHeight, const bool wordWrap, const float wrapWidth, const uint32_t firstLine, fuiTextLine *outLines, const uint32_t maxLines, uint32_t *outTotalLineCount) {
	bool wantsTheLines = (outLines != fui_null) && (maxLines > 0);
	bool wantsTheTotal = (outTotalLineCount != fui_null);
	if(!wantsTheLines && !wantsTheTotal) {
		return(0);
	}

	fui__TextLineSink sink;
	fui__ClearMemory(&sink, sizeof(sink));
	sink.lines = wantsTheLines ? outLines : fui_null;
	sink.capacity = wantsTheLines ? maxLines : 0u;
	sink.firstStoredLine = firstLine;
	sink.offsetToFind = (size_t)-1;
	// Counting past the window means walking the whole text, which only a caller that needs the total has
	// any reason to pay for.
	sink.countsPastTheWindow = (outTotalLineCount != fui_null);

	fui__BreakTextLinesInto(context, text, textLength, pixelHeight, wordWrap, wrapWidth, &sink);

	if(outTotalLineCount != fui_null) {
		*outTotalLineCount = sink.totalCount;
	}
	return(sink.storedCount);
}

fui_api uint32_t fuiBreakTextLines(const fuiContext *context, const char *text, const size_t textLength, const float pixelHeight, const bool wordWrap, const float wrapWidth, fuiTextLine *outLines, const uint32_t maxLines) {
	const uint32_t fromTheFirstLine = 0;
	uint32_t *doNotCountPastTheWindow = fui_null;
	uint32_t result = fuiBreakTextLinesFrom(context, text, textLength, pixelHeight, wordWrap, wrapWidth, fromTheFirstLine, outLines, maxLines, doNotCountPastTheWindow);
	return(result);
}

fui_api fuiVec2 fuiMeasureTextBlock(const fuiContext *context, const char *text, const size_t textLength, const float pixelHeight, const bool wordWrap, const float wrapWidth) {
	FUI_ASSERT(context != fui_null);
	fuiVec2 result = fuiV2(0.0f, 0.0f);
	if(context == fui_null || context->font == fui_null) {
		return(result);
	}

	fuiTextLine lines[FUI_MAX_TEXT_LINES];
	uint32_t lineCount = fuiBreakTextLines(context, text, textLength, pixelHeight, wordWrap, wrapWidth, lines, (uint32_t)FUI_MAX_TEXT_LINES);
	if(lineCount == 0) {
		return(result);
	}

	float widestLine = 0.0f;
	for(uint32_t lineIndex = 0; lineIndex < lineCount; ++lineIndex) {
		fuiVec2 lineSize = fuiMeasureText(context, lines[lineIndex].start, lines[lineIndex].length, pixelHeight);
		widestLine = fuiMaxF(widestLine, lineSize.x);
	}

	const fuiFont *font = context->font;
	float singleLineHeight = (font->metrics.ascent + font->metrics.descent) * pixelHeight;
	float lineStep = font->metrics.lineHeight * pixelHeight;
	float totalHeight = singleLineHeight + (float)(lineCount - 1u) * lineStep;
	result = fuiV2(widestLine, totalHeight);
	return(result);
}

fui_api void fuiDrawTextBlock(fuiContext *context, const char *text, const size_t textLength, const fuiVec2 position, const float pixelHeight, const fuiColor color, const bool wordWrap, const float wrapWidth) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null || context->font == fui_null) {
		return;
	}

	fuiTextLine lines[FUI_MAX_TEXT_LINES];
	uint32_t lineCount = fuiBreakTextLines(context, text, textLength, pixelHeight, wordWrap, wrapWidth, lines, (uint32_t)FUI_MAX_TEXT_LINES);
	float lineStep = context->font->metrics.lineHeight * pixelHeight;
	for(uint32_t lineIndex = 0; lineIndex < lineCount; ++lineIndex) {
		if(lines[lineIndex].length == 0) {
			continue;
		}
		fuiVec2 linePosition = fuiV2(position.x, position.y + (float)lineIndex * lineStep);
		fuiDrawText(context, lines[lineIndex].start, lines[lineIndex].length, linePosition, pixelHeight, color);
	}
}

// ----------------------------------------------------------------------------
// > Layout and widget tuning
//
// What is here and not in fuiTheme are the numbers that are STRUCTURE rather than look - a title bar
// button has to be wide enough to hit, a resize grip large enough to grab. A caller restyling the
// library changes colors, fonts and paddings; these hold the widgets together.
// ----------------------------------------------------------------------------

//! Vertical padding above and below the text of a panel title bar
#define FUI__PANEL_TITLE_BAR_PADDING 5.0f
//! Side of the square resize grip at a floating panel's corner
#define FUI__PANEL_RESIZE_GRIP_SIZE 14.0f
//! Thickness of the accent line a hovered edge grip is marked with
#define FUI__PANEL_GRIP_HIGHLIGHT_THICKNESS 6.0f
//! Smallest a panel may be shrunk to, so it stays grabbable
#define FUI__PANEL_MIN_WIDTH 60.0f
//! Smallest height a panel may be shrunk to, so it stays grabbable
#define FUI__PANEL_MIN_HEIGHT 40.0f
//! Width of the collapse arrow button at the left of a title bar
#define FUI__PANEL_COLLAPSE_BUTTON_WIDTH 22.0f
//! Width of the close button at the right of a title bar, matched to the collapse arrow so the pair mirrors
#define FUI__PANEL_CLOSE_BUTTON_WIDTH 22.0f
//! How many menu rows one wheel notch scrolls a scrolling container by
#define FUI__SCROLL_WHEEL_ROWS 3.0f
//! Width of the gutter every scrolling container reserves for its scrollbar
#define FUI__SCROLLBAR_WIDTH 20.0f
//! Smallest the draggable thumb may shrink to on a very long list
#define FUI__SCROLLBAR_MIN_THUMB 18.0f
//! Alpha the thumb of a disabled scrollbar is drawn at, so it reads as inactive
#define FUI__SCROLLBAR_DISABLED_ALPHA 0.35f
//! Height of the box a scrolling container flows its content inside, far larger than any real content
#define FUI__SCROLL_LAYOUT_HEIGHT 1000000.0f
//! Side of a title bar icon, as a fraction of the font height. Half an em is about what the character it replaces inked
#define FUI__TITLE_BAR_ICON_FONT_FRACTION 0.5f
//! How many diagonal ticks a resize grip is drawn from
#define FUI__RESIZE_GRIP_TICK_COUNT 3
//! Vertical padding above and below the text of a group box title bar
#define FUI__GROUP_BOX_TITLE_PADDING 3.0f
//! Inset of the first row from the top edge of a multi-line widget
#define FUI__MULTILINE_TOP_PADDING 4.0f
//! Smallest inset of the check mark inside a checkbox box, and of the dot inside a radio marker
#define FUI__CHECK_MARK_INSET 4.0f
//! How much bare well is left showing between the bevel of a checkbox box and the mark inside it
#define FUI__CHECK_MARK_WELL_GAP 2.0f
//! Gap between a checkbox or radio marker and the caption beside it
#define FUI__WIDGET_LABEL_GAP 6.0f
//! Width of the knob a slider marks its value with
#define FUI__SLIDER_KNOB_WIDTH 8.0f
//! Bytes reserved for the number a drag field prints on itself
#define FUI__VALUE_TEXT_CAPACITY 32

// ----------------------------------------------------------------------------
// > Retained widget state
// ----------------------------------------------------------------------------

//! Above this share of the slots being taken, an open addressed table stops being a lookup and starts
//! being a linear scan, so it is grown instead. Four fifths, expressed as a numerator and a denominator
#define FUI__WIDGET_STATE_LOAD_NUMERATOR 4
#define FUI__WIDGET_STATE_LOAD_DENOMINATOR 5

//! Where an identifier's probe starts. The identifier is already a hash, and a power of two capacity lets
//! this be a mask rather than a division, which matters because it runs for every widget every frame
fui_inline uint32_t fui__WidgetStateStartIndex(const fuiId id, const uint32_t capacity) {
	uint32_t result = (uint32_t)(id % (fuiId)capacity);
	return(result);
}

/*
	Grows the table and puts everything back into it.

	Called from fuiBeginFrame and from nowhere else, on purpose. Every lookup hands back a POINTER into
	this array, and a panel keeps one of those on the layout stack for as long as it is open - so moving
	the array while a frame is being built would leave those pointing at freed memory. Between frames
	nothing is holding one.
*/
fui_inline void fui__WidgetStateMapGrow(fuiContext *context) {
	fuiWidgetStateMap *map = &context->widgetStates;
	map->growthIsWanted = false;

	uint32_t newCapacity = (map->capacity > 0) ? (map->capacity * 2u) : (uint32_t)FUI_MAX_WIDGET_STATES;
	if(newCapacity <= map->capacity) {
		return; // wrapped around, which no real interface can reach
	}

	size_t byteCount = (size_t)newCapacity * sizeof(fuiWidgetState);
	fuiWidgetState *newEntries = (fuiWidgetState *)fui__ArenaPushExact(&context->arena, byteCount, true);
	if(newEntries == fui_null) {
		// Out of memory. The old table is still perfectly good, just full, so widgets that already have a
		// slot keep it and new ones go without - which is what happened before there was any growth at all.
		return;
	}

	uint32_t moved = 0;
	for(uint32_t oldIndex = 0; oldIndex < map->capacity; ++oldIndex) {
		fuiWidgetState *oldEntry = &map->entries[oldIndex];
		if(!oldEntry->isUsed) {
			continue;
		}
		uint32_t startIndex = fui__WidgetStateStartIndex(oldEntry->id, newCapacity);
		for(uint32_t probe = 0; probe < newCapacity; ++probe) {
			uint32_t index = (startIndex + probe) % newCapacity;
			if(newEntries[index].isUsed) {
				continue;
			}
			newEntries[index] = *oldEntry;
			moved += 1u;
			break;
		}
	}

	// The arena never gives an allocation back, so the old table is simply left behind. Doubling is what
	// keeps everything left behind smaller than what is finally in use.
	map->entries = newEntries;
	map->capacity = newCapacity;
	map->count = moved;
}

//! Looks up the retained state of a widget, creating its slot the first time the widget is seen
fui_inline fuiWidgetState *fui__WidgetStateGet(fuiContext *context, const fuiId id) {
	FUI_ASSERT(context != fui_null);
	fuiWidgetStateMap *map = &context->widgetStates;

	// Allocated on first use rather than at init, so a context that never opens a panel takes nothing.
	if(map->entries == fui_null && map->capacity == 0) {
		size_t byteCount = (size_t)FUI_MAX_WIDGET_STATES * sizeof(fuiWidgetState);
		map->entries = (fuiWidgetState *)fui__ArenaPushExact(&context->arena, byteCount, true);
		map->capacity = (map->entries != fui_null) ? (uint32_t)FUI_MAX_WIDGET_STATES : 0u;
	}
	if(map->entries == fui_null || map->capacity == 0 || id == FUI_ID_NONE) {
		return(fui_null);
	}

	uint32_t startIndex = fui__WidgetStateStartIndex(id, map->capacity);
	for(uint32_t probe = 0; probe < map->capacity; ++probe) {
		uint32_t index = (startIndex + probe) % map->capacity;
		fuiWidgetState *entry = &map->entries[index];
		if(entry->isUsed && entry->id == id) {
			return(entry);
		}
		if(!entry->isUsed) {
			fui__ClearMemory(entry, sizeof(*entry));
			entry->id = id;
			entry->isUsed = true;
			// Row zero is a real row, so "nothing was clicked yet" cannot be spelled as a cleared field. A
			// fresh list would otherwise read its very first click on row zero as the second half of one.
			entry->lastClickIndex = -1;
			map->count += 1u;
			// Asked for at the frame boundary rather than taken now, because everything already handed out
			// is a pointer into the array this would move.
			if((map->count * FUI__WIDGET_STATE_LOAD_DENOMINATOR) >= (map->capacity * FUI__WIDGET_STATE_LOAD_NUMERATOR)) {
				map->growthIsWanted = true;
			}
			return(entry);
		}
	}

	// Every slot taken by another widget, and the table could not be grown. The caller falls back to
	// un-remembered behaviour rather than evicting somebody else's panel position, which would move a
	// window the user never touched.
	map->growthIsWanted = true;
	return(fui_null);
}

// ----------------------------------------------------------------------------
// > Shared widget helpers
// ----------------------------------------------------------------------------

//! Marks a rectangle as claimed by a widget when the cursor is over it, without any interaction
fui_inline bool fui__ClaimCursor(fuiContext *context, const fuiId id, const fuiRect rect) {
	bool cursorIsOver = fui__CursorIsOver(context, rect);
	if(cursorIsOver) {
		context->hotCandidate = id;
		context->mouseIsOverUi = true;
	}
	return(cursorIsOver);
}

//! The fill of a clickable widget in its current interaction state
fui_inline fuiColor fui__WidgetFillColor(const fuiContext *context, const fuiInteraction interaction) {
	if(interaction.isHeld) {
		return(context->theme.widgetActiveColor);
	}
	if(interaction.isHovered) {
		return(context->theme.widgetHoveredColor);
	}
	return(context->theme.widgetColor);
}

//! Draws the lit and the shaded edge INSIDE a box: two strips along the top and the left, two along the bottom
//! and the right. The corners are butted rather than mitred, because at one or two pixels a diagonal seam is a
//! staircase rather than a corner - and nothing the library draws is anti-aliased yet
fui_inline void fui__DrawBevelEdges(fuiContext *context, const fuiRect box, const float thickness, const fuiColor topLeftColor, const fuiColor bottomRightColor) {
	float smallestSide = fuiMinF(box.w, box.h);
	float maximumThickness = smallestSide * 0.5f;
	float edgeThickness = fuiMinF(thickness, maximumThickness);
	if(edgeThickness <= 0.0f) {
		return;
	}

	float innerHeight = box.h - edgeThickness * 2.0f;
	fuiRect topEdge = fuiRectMake(box.x, box.y, box.w, edgeThickness);
	fuiRect leftEdge = fuiRectMake(box.x, box.y + edgeThickness, edgeThickness, innerHeight);
	fuiRect bottomEdge = fuiRectMake(box.x, box.y + box.h - edgeThickness, box.w, edgeThickness);
	fuiRect rightEdge = fuiRectMake(box.x + box.w - edgeThickness, box.y + edgeThickness, edgeThickness, innerHeight);

	fuiDrawRect(context, topEdge, topLeftColor);
	fuiDrawRect(context, leftEdge, topLeftColor);
	fuiDrawRect(context, bottomEdge, bottomRightColor);
	fuiDrawRect(context, rightEdge, bottomRightColor);
}

//! How a widget box stands relative to the surface around it. A box says what can be done to it by its SHAPE,
//! so the three are not interchangeable: sunken is what a widget looks like once it has been pushed, and a
//! widget nobody has pushed must not borrow that shape to mean something else
typedef enum fui__Relief {
	//! Standing out of the panel. What a button at rest looks like, and the tab whose page is showing
	FUI__RELIEF_RAISED = 0,
	//! Pushed into the panel. A button being held, a lit toggle, and the well a checkbox puts its mark in
	FUI__RELIEF_SUNKEN,
	//! Neither pushed nor standing out, just a filled box. What sits BEHIND the front surface rather than on it,
	//! such as a tab whose page is not showing
	FUI__RELIEF_FLAT,
} fui__Relief;

//! One widget box in its relief: a vertically shaded face between a lit and a shaded edge, framed by the same
//! outline every box in the interface carries. The light is overhead, so a raised face catches it along its top
//! and a sunken one lies in its own shadow there. A flat box takes neither and is drawn as the plain fill it is
fui_inline void fui__DrawBevelBox(fuiContext *context, const fuiRect rect, const fuiColor faceColor, const fui__Relief relief) {
	const fuiTheme *theme = &context->theme;

	if(relief == FUI__RELIEF_FLAT) {
		fuiDrawRect(context, rect, faceColor);
		fuiDrawRectOutline(context, rect, theme->panelBorderColor, theme->widgetBorderThickness);
		return;
	}

	bool isSunken = (relief == FUI__RELIEF_SUNKEN);
	float shadingStrength = theme->widgetFaceShadingStrength;
	fuiColor litFace = fuiColorShade(faceColor, shadingStrength);
	fuiColor shadedFace = fuiColorShade(faceColor, -shadingStrength);
	fuiColor topFace = isSunken ? shadedFace : litFace;
	fuiColor bottomFace = isSunken ? litFace : shadedFace;
	fuiDrawRectVerticalGradient(context, rect, topFace, bottomFace);

	// Inside the outline rather than under it, so the frame stays the outermost thing on the box and the
	// bevel reads as the chamfer it is meant to be.
	fuiRect bevelBox = fuiRectInflate(rect, -theme->widgetBorderThickness);
	fuiColor topLeftEdgeColor = isSunken ? theme->widgetBevelShadowColor : theme->widgetBevelLightColor;
	fuiColor bottomRightEdgeColor = isSunken ? theme->widgetBevelLightColor : theme->widgetBevelShadowColor;
	fui__DrawBevelEdges(context, bevelBox, theme->widgetBevelThickness, topLeftEdgeColor, bottomRightEdgeColor);

	fuiDrawRectOutline(context, rect, theme->panelBorderColor, theme->widgetBorderThickness);
}

//! The box the mark inside a checkbox well or a radio marker is drawn in. Inset far enough to clear the well's
//! frame AND its bevel, so a theme with a thicker bevel grows the well around the mark rather than over it
fui_inline fuiRect fui__CheckMarkRect(const fuiContext *context, const fuiRect well) {
	const fuiTheme *theme = &context->theme;
	float wellChromeThickness = theme->widgetBorderThickness + theme->widgetBevelThickness;
	float clearingInset = wellChromeThickness + FUI__CHECK_MARK_WELL_GAP;
	float markInset = fuiMaxF(FUI__CHECK_MARK_INSET, clearingInset);
	fuiRect result = fuiRectInflate(well, -markInset);
	return(result);
}

//! Where the content of a widget goes: level with its box, or nudged down and to the right while the box is
//! pressed in, which is the other half of what makes a click feel like one
fui_inline fuiRect fui__PressedContentRect(const fuiContext *context, const fuiRect rect, const bool isPressed) {
	if(!isPressed) {
		return(rect);
	}
	float pressOffset = context->theme.widgetPressOffset;
	fuiVec2 pressShift = fuiV2(pressOffset, pressOffset);
	fuiRect result = fuiRectOffset(rect, pressShift);
	return(result);
}

//! Draws one line of text at an explicit x, vertically centered on a rectangle and clipped to it
fui_inline void fui__DrawTextLeftAligned(fuiContext *context, const fuiRect bounds, const float textX, const char *text, const fuiColor color) {
	if(context->font == fui_null || text == fui_null) {
		return;
	}
	float pixelHeight = context->theme.fontHeight;
	// The height of a line is the font's, not the string's, so nothing is measured here at all. This is
	// the one label path every widget goes down - a table of a few hundred visible cells came through it
	// a few hundred times a frame, and each of those used to walk its whole string for this one number.
	float lineHeight = fui__LineExtent(context->font, pixelHeight);
	float textY = bounds.y + (bounds.h - lineHeight) * 0.5f;
	// Clipped rather than left to spill: a caption too long for its widget is cropped at the widget edge
	// instead of running into the next one.
	fuiPushClip(context, bounds);
	fuiDrawText(context, text, 0, fuiV2(textX, textY), pixelHeight, color);
	fuiPopClip(context);
}

//! Draws a caption inside a widget, inset by the theme's horizontal padding. The one label path every widget uses
fui_inline void fui__DrawTextInRect(fuiContext *context, const fuiRect rect, const char *text, const fuiColor color) {
	float textX = rect.x + context->theme.widgetPaddingX;
	fui__DrawTextLeftAligned(context, rect, textX, text, color);
}

//! Draws a caption centred across a widget rather than inset from its left edge. What a BUTTON wants: its
//! caption names the whole box and reads as belonging to it, where a left inset one reads as a list row.
fui_inline void fui__DrawTextCenteredInRect(fuiContext *context, const fuiRect rect, const char *text, const fuiColor color) {
	if(context->font == fui_null || text == fui_null) {
		return;
	}
	float pixelHeight = context->theme.fontHeight;
	fuiVec2 textSize = fuiMeasureText(context, text, 0, pixelHeight);
	// A caption too wide for its box falls back to the left inset, so it is cropped at the far edge rather
	// than losing the same amount off BOTH ends and becoming unreadable from either side.
	float textX = rect.x + context->theme.widgetPaddingX;
	if(textSize.x < (rect.w - context->theme.widgetPaddingX * 2.0f)) {
		textX = rect.x + (rect.w - textSize.x) * 0.5f;
	}
	fui__DrawTextLeftAligned(context, rect, textX, text, color);
}

//! Draws a caption as top anchored rows inside a widget, clipped to it. The length is taken rather than
//! measured, because the one caller that holds a large buffer has already paid for it once this frame and
//! a second walk of a document sized string is not free
fui_inline void fui__DrawTextBlockInRect(fuiContext *context, const fuiRect rect, const char *text, const size_t textLength, const bool wordWrap, const fuiColor color) {
	if(context->font == fui_null || text == fui_null) {
		return;
	}
	const fuiTheme *theme = &context->theme;
	float contentWidth = rect.w - theme->widgetPaddingX * 2.0f;
	fuiVec2 blockPosition = fuiV2(rect.x + theme->widgetPaddingX, rect.y + FUI__MULTILINE_TOP_PADDING);
	fuiPushClip(context, rect);
	fuiDrawTextBlock(context, text, textLength, blockPosition, theme->fontHeight, color, wordWrap, contentWidth);
	fuiPopClip(context);
}

// ----------------------------------------------------------------------------
// > Chrome
// ----------------------------------------------------------------------------

fui_api void fuiDrawResizeGrip(fuiContext *context, const fuiRect grip, const fuiColor color, const float thickness) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return;
	}
	// Stacked diagonal ticks in the corner, longest at the corner itself. Drawn from lines rather than an
	// icon, so it needs no texture and comes out at whatever size the caller picked for the grip.
	for(int32_t tickIndex = 0; tickIndex < FUI__RESIZE_GRIP_TICK_COUNT; ++tickIndex) {
		float inset = grip.w * ((float)(tickIndex + 1) / (float)(FUI__RESIZE_GRIP_TICK_COUNT + 1));
		fuiVec2 tickStart = fuiV2(grip.x + grip.w - inset, grip.y + grip.h);
		fuiVec2 tickEnd = fuiV2(grip.x + grip.w, grip.y + grip.h - inset);
		fuiDrawLine(context, tickStart, tickEnd, color, thickness);
	}
}

fui_api float fuiTitleBarIconSize(const fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return(0.0f);
	}
	float result = context->theme.fontHeight * FUI__TITLE_BAR_ICON_FONT_FRACTION;
	return(result);
}

fui_api void fuiDrawCollapseGlyph(fuiContext *context, const fuiRect glyphBox, const bool isCollapsed, const fuiColor color) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return;
	}
	float left = glyphBox.x;
	float right = glyphBox.x + glyphBox.w;
	float top = glyphBox.y;
	float bottom = glyphBox.y + glyphBox.h;
	fuiVec2 corners[3];
	if(isCollapsed) {
		// Pointing right, at content that is folded away.
		corners[0] = fuiV2(left, top);
		corners[1] = fuiV2(left, bottom);
		corners[2] = fuiV2(right, (top + bottom) * 0.5f);
	} else {
		// Pointing down, at content that is open.
		corners[0] = fuiV2(left, top);
		corners[1] = fuiV2(right, top);
		corners[2] = fuiV2((left + right) * 0.5f, bottom);
	}
	fuiDrawPolygon(context, corners, 3, color);
}

fui_api void fuiDrawCloseGlyph(fuiContext *context, const fuiRect glyphBox, const fuiColor color, const float thickness) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return;
	}
	// Two strokes corner to corner, so the cross comes out the same size as the collapse triangle whenever
	// both are handed a box of fuiTitleBarIconSize.
	float left = glyphBox.x;
	float right = glyphBox.x + glyphBox.w;
	float top = glyphBox.y;
	float bottom = glyphBox.y + glyphBox.h;
	fuiDrawLine(context, fuiV2(left, top), fuiV2(right, bottom), color, thickness);
	fuiDrawLine(context, fuiV2(left, bottom), fuiV2(right, top), color, thickness);
}

// ----------------------------------------------------------------------------
// > Scrollbar
// ----------------------------------------------------------------------------

fui_api float fuiScrollGutterWidth(void) {
	return(FUI__SCROLLBAR_WIDTH);
}

//! Length of the thumb, proportional to how much of the content is visible and floored so it stays grabbable
fui_inline float fui__ScrollThumbExtent(const float trackLength, const float visibleLength, const float contentLength) {
	float visibleRatio = (contentLength > 0.0f) ? (visibleLength / contentLength) : 1.0f;
	float result = fuiMaxF(trackLength * visibleRatio, FUI__SCROLLBAR_MIN_THUMB);
	return(result);
}

//! Where the thumb sits for a scroll offset. No scroll puts it at the START of its track, which is the top
//! of a vertical bar and the left of a horizontal one - the low coordinate either way, y-down
fui_inline fuiRect fui__ScrollThumbRect(const fuiRect track, const float scroll, const float maxScroll, const float thumbExtent, const bool isVertical) {
	float trackLength = isVertical ? track.h : track.w;
	float trackRange = trackLength - thumbExtent;
	float scrollRatio = (maxScroll > 0.0f) ? (scroll / maxScroll) : 0.0f;
	if(isVertical) {
		fuiRect result = fuiRectMake(track.x, track.y + scrollRatio * trackRange, track.w, thumbExtent);
		return(result);
	}
	fuiRect result = fuiRectMake(track.x + scrollRatio * trackRange, track.y, thumbExtent, track.h);
	return(result);
}

//! One scrollbar along either axis. The two differ in which extent is the track's length and which delta
//! drags the thumb - everything else about them, disabled look included, is the same bar
fui_inline float fui__Scrollbar(fuiContext *context, const fuiRect track, const char *id, const float scroll, const float viewportLength, const float contentLength, const bool isVertical) {
	const fuiTheme *theme = &context->theme;
	float maxScroll = contentLength - viewportLength;
	if(maxScroll <= 0.0f) {
		// Drawn but disabled. The alternative - hiding it - changes the width of everything beside it the
		// moment one row is added, which makes a list twitch as it fills.
		fuiColor dimmedThumb = fuiColorWithAlpha(theme->widgetColor, theme->widgetColor.a * FUI__SCROLLBAR_DISABLED_ALPHA);
		fuiDrawRect(context, track, theme->widgetTrackColor);
		fuiDrawRect(context, track, dimmedThumb);
		fuiDrawRectOutline(context, track, theme->panelBorderColor, theme->widgetBorderThickness);
		return(0.0f);
	}

	fuiId thumbId = fuiGetId(context, id);
	float trackLength = isVertical ? track.h : track.w;
	float thumbExtent = fui__ScrollThumbExtent(trackLength, viewportLength, contentLength);
	fuiRect thumbBeforeDrag = fui__ScrollThumbRect(track, scroll, maxScroll, thumbExtent, isVertical);
	fuiInteraction interaction = fuiInteract(context, thumbId, thumbBeforeDrag);

	float newScroll = scroll;
	if(interaction.isHeld) {
		float trackRange = trackLength - thumbExtent;
		if(trackRange > 0.0f) {
			float scrollPerPixel = maxScroll / trackRange;
			float dragged = isVertical ? context->mouseDelta.y : context->mouseDelta.x;
			newScroll += dragged * scrollPerPixel;
		}
	}
	newScroll = fuiClampF(newScroll, 0.0f, maxScroll);

	fuiRect thumb = fui__ScrollThumbRect(track, newScroll, maxScroll, thumbExtent, isVertical);
	fuiColor thumbColor = interaction.isHeld ? theme->accentColor : (interaction.isHovered ? theme->widgetHoveredColor : theme->widgetColor);
	fuiDrawRect(context, track, theme->widgetTrackColor);
	fuiDrawRect(context, thumb, thumbColor);
	fuiDrawRectOutline(context, thumb, theme->panelBorderColor, theme->widgetBorderThickness);
	return(newScroll);
}

fui_api float fuiScrollbarVertical(fuiContext *context, const fuiRect track, const char *id, const float scroll, const float viewportLength, const float contentLength) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return(0.0f);
	}
	return(fui__Scrollbar(context, track, id, scroll, viewportLength, contentLength, true));
}

// ----------------------------------------------------------------------------
// > Layout core
// ----------------------------------------------------------------------------

//! The innermost container that is open, or null when there is none
fui_inline fuiLayoutNode *fui__CurrentLayout(fuiContext *context) {
	// A push past the maximum depth still counted so that pops stay balanced, but it recorded no node.
	// Answering null there is what keeps an over-deep child from writing into its grandparent's box.
	if(context->layoutDepth == 0 || context->layoutDepth > (uint32_t)FUI_MAX_LAYOUT_DEPTH) {
		return(fui_null);
	}
	fuiLayoutNode *result = &context->layoutStack[context->layoutDepth - 1u];
	return(result);
}

//! The box a new child is placed inside: whatever the enclosing container has left, or the whole window
fui_inline fuiRect fui__ParentBox(fuiContext *context) {
	fuiLayoutNode *parent = fui__CurrentLayout(context);
	if(parent != fui_null) {
		return(parent->remaining);
	}
	fuiRect wholeWindow = fuiRectMake(0.0f, 0.0f, (float)context->windowSize.x, (float)context->windowSize.y);
	return(wholeWindow);
}

//! Bites a strip off one edge of a box, returning the strip and shrinking the box
fui_inline fuiRect fui__TakeDockRegion(fuiRect *box, const fuiDock dock, const float width, const float height) {
	fuiRect region = *box;
	switch(dock) {
		case FUI_DOCK_LEFT:
		{
			float takenWidth = fuiMinF(width, box->w);
			region = fuiRectMake(box->x, box->y, takenWidth, box->h);
			box->x += takenWidth;
			box->w -= takenWidth;
		} break;

		case FUI_DOCK_RIGHT:
		{
			float takenWidth = fuiMinF(width, box->w);
			region = fuiRectMake(box->x + box->w - takenWidth, box->y, takenWidth, box->h);
			box->w -= takenWidth;
		} break;

		case FUI_DOCK_TOP:
		{
			// y-down: the top edge is the LOW y one, so a top strip starts where the box starts.
			float takenHeight = fuiMinF(height, box->h);
			region = fuiRectMake(box->x, box->y, box->w, takenHeight);
			box->y += takenHeight;
			box->h -= takenHeight;
		} break;

		case FUI_DOCK_BOTTOM:
		{
			float takenHeight = fuiMinF(height, box->h);
			region = fuiRectMake(box->x, box->y + box->h - takenHeight, box->w, takenHeight);
			box->h -= takenHeight;
		} break;

		case FUI_DOCK_NONE:
		case FUI_DOCK_FILL:
		default:
		{
			region = *box;
			box->w = 0.0f;
			box->h = 0.0f;
		} break;
	}
	return(region);
}

//! Rounds a coordinate DOWN to a whole pixel. Cast based rather than floorf, since interface coordinates never leave the range an int32 holds
fui_inline float fui__FloorToPixel(const float value) {
	int32_t truncated = (int32_t)value;
	if((float)truncated > value) {
		truncated -= 1;
	}
	return((float)truncated);
}

//! Rounds a coordinate UP to a whole pixel
fui_inline float fui__CeilToPixel(const float value) {
	int32_t truncated = (int32_t)value;
	if((float)truncated < value) {
		truncated += 1;
	}
	return((float)truncated);
}

//! Snaps a box onto the pixel grid, growing it outwards so nothing it holds is lost to the rounding. A box
//! whose edges land BETWEEN two pixels loses part of its outline to whatever the backend rounds its scissor
//! to - a truncated glScissor cuts the right and bottom stroke off - and there is no thickness a caller can
//! pick that hides that. Snapping the box is the fix that holds for every backend.
fui_inline fuiRect fui__SnapRectToPixels(const fuiRect rect) {
	float left = fui__FloorToPixel(rect.x);
	float top = fui__FloorToPixel(rect.y);
	float right = fui__CeilToPixel(rect.x + rect.w);
	float bottom = fui__CeilToPixel(rect.y + rect.h);
	fuiRect result = fuiRectMake(left, top, right - left, bottom - top);
	return(result);
}

//! The ONE frame every box of chrome carries - a panel, a dialog, a menu popup, a group box. Snapped onto
//! the pixel grid first, because a one pixel stroke sitting on a half pixel boundary is a stroke the
//! rasterizer is free to drop entirely, and a frame that comes and goes with where a box happens to land is
//! worse than no frame at all
fui_inline void fui__DrawChromeFrame(fuiContext *context, const fuiRect rect) {
	const fuiTheme *theme = &context->theme;
	fuiRect snapped = fui__SnapRectToPixels(rect);
	fuiDrawRectOutline(context, snapped, theme->panelBorderColor, theme->panelBorderThickness);
}

//! How thick a strip of chrome is that holds ONE row of something: the row plus the inset it keeps above and
//! below itself. Every bar in the interface is this shape - a menu bar over its titles, a tool strip over its
//! buttons, a status bar over its text - so they are all measured the same way and none of them needs a
//! number picked by hand
fui_inline float fui__BarThickness(const fuiContext *context, const float rowThickness) {
	float result = rowThickness + context->theme.menuItemPaddingY * 2.0f;
	return(result);
}

//! Shrinks a rectangle by an inset on each axis, never past empty
fui_inline fuiRect fui__RectInset(const fuiRect rect, const float insetX, const float insetY) {
	float width = fuiMaxF(0.0f, rect.w - insetX * 2.0f);
	float height = fuiMaxF(0.0f, rect.h - insetY * 2.0f);
	fuiRect result = fuiRectMake(rect.x + insetX, rect.y + insetY, width, height);
	return(result);
}

//! Opens a container. Returns null past the maximum depth, but the depth counts either way so pops stay balanced
fui_inline fuiLayoutNode *fui__PushLayout(fuiContext *context, const fuiId id, fuiWidgetState *state, const bool isPanel, const fuiAxis axis, const float spacing, const fuiRect contentBox) {
	fuiLayoutNode *node = fui_null;
	if(context->layoutDepth < (uint32_t)FUI_MAX_LAYOUT_DEPTH) {
		node = &context->layoutStack[context->layoutDepth];
		fui__ClearMemory(node, sizeof(*node));
		node->id = id;
		node->state = state;
		node->isPanel = isPanel;
		node->axis = axis;
		node->spacing = spacing;
		node->contentBox = contentBox;
		node->remaining = contentBox;
	}
	context->layoutDepth += 1u;
	return(node);
}

//! Closes a container
fui_inline void fui__PopLayout(fuiContext *context) {
	FUI_ASSERT(context->layoutDepth > 0);
	if(context->layoutDepth > 0) {
		context->layoutDepth -= 1u;
	}
}

//! Takes a strip off the LEADING edge along the flow axis, clamped to what is left
fui_inline fuiRect fui__ConsumeLeading(fuiLayoutNode *node, const float amount) {
	fuiRect *remaining = &node->remaining;
	fuiRect strip;
	if(node->axis == FUI_AXIS_VERTICAL) {
		// y-down makes both axes read the same: the leading edge is the low coordinate one either way.
		float taken = fuiMinF(amount, remaining->h);
		strip = fuiRectMake(remaining->x, remaining->y, remaining->w, taken);
		remaining->y += taken;
		remaining->h -= taken;
	} else {
		float taken = fuiMinF(amount, remaining->w);
		strip = fuiRectMake(remaining->x, remaining->y, taken, remaining->h);
		remaining->x += taken;
		remaining->w -= taken;
	}
	return(strip);
}

fui_api fuiRect fuiLayoutSlot(fuiContext *context, const float thickness) {
	FUI_ASSERT(context != fui_null);
	fuiRect empty = fuiRectMake(0.0f, 0.0f, 0.0f, 0.0f);
	if(context == fui_null) {
		return(empty);
	}
	fuiLayoutNode *node = fui__CurrentLayout(context);
	if(node == fui_null || node->isHidden) {
		return(empty);
	}

	// The gap is a TRAILING margin - under a slot in a vertical flow, right of one in a horizontal flow - so
	// a container never opens with one and every widget carries its own separation from the next. The box was
	// already advanced past it when the previous slot was handed out; all that is left here is to COUNT it,
	// and only once another slot actually follows, so the margin hanging off the last widget never inflates
	// what the container reports as measured.
	if(node->hasFlowStarted && node->spacing > 0.0f) {
		node->usedExtent += node->spacing;
	}

	fuiRect slot = fui__ConsumeLeading(node, thickness);
	node->hasFlowStarted = true;
	node->usedExtent += thickness;

	if(node->spacing > 0.0f) {
		(void)fui__ConsumeLeading(node, node->spacing);
	}
	return(slot);
}

fui_api fuiRect fuiLayoutDock(fuiContext *context, const fuiDock dock, const float thickness) {
	FUI_ASSERT(context != fui_null);
	fuiRect empty = fuiRectMake(0.0f, 0.0f, 0.0f, 0.0f);
	if(context == fui_null) {
		return(empty);
	}
	fuiLayoutNode *node = fui__CurrentLayout(context);
	if(node == fui_null || node->isHidden) {
		return(empty);
	}

	// A dock cuts into the box the same way for either axis, so one call covers all four edges.
	fuiRect box = node->remaining;
	fuiRect strip = fui__TakeDockRegion(&box, dock, thickness, thickness);
	node->remaining = box;

	// The flow is deliberately left alone: hasFlowStarted and usedExtent describe the rows a container holds,
	// and a bar docked around them is not one of those rows. A panel that opens a slot after docking its own
	// tool strip still gets a first slot with no margin in front of it.
	return(strip);
}

fui_api fuiRect fuiLayoutRemaining(const fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	fuiRect empty = fuiRectMake(0.0f, 0.0f, 0.0f, 0.0f);
	if(context == fui_null) {
		return(empty);
	}
	if(context->layoutDepth > 0 && context->layoutDepth <= (uint32_t)FUI_MAX_LAYOUT_DEPTH) {
		fuiRect result = context->layoutStack[context->layoutDepth - 1u].remaining;
		return(result);
	}
	fuiRect wholeWindow = fuiRectMake(0.0f, 0.0f, (float)context->windowSize.x, (float)context->windowSize.y);
	return(wholeWindow);
}

// ----------------------------------------------------------------------------
// > Panels
// ----------------------------------------------------------------------------

//! Height of a panel's title bar, which follows the font so a restyled context keeps its captions readable
fui_inline float fui__PanelTitleBarHeight(const fuiContext *context) {
	float result = context->theme.fontHeight + FUI__PANEL_TITLE_BAR_PADDING * 2.0f;
	return(result);
}

fui_api float fuiPanelChromeHeight(const fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return(0.0f);
	}
	float result = fui__PanelTitleBarHeight(context) + context->theme.panelPaddingY * 2.0f;
	return(result);
}

//! Turns a content box into the viewport, gutter and tall flow box a scrolling container needs. The gutter
//! keeps a padding to its LEFT so the bar never sits flush against the widgets it scrolls: a button ending
//! exactly where the scrollbar starts reads as one control cut in half, and the gap is the same measure the
//! panel already keeps to its left and right edges
fui_inline void fui__ResolveScrollBoxes(const fuiRect contentBox, const float scroll, const float gutterPadding, fuiRect *outViewport, fuiRect *outTrack, fuiRect *outLayoutBox) {
	float gutter = fuiScrollGutterWidth();
	float viewportWidth = fuiMaxF(0.0f, contentBox.w - gutter - gutterPadding);
	*outTrack = fuiRectMake(contentBox.x + contentBox.w - gutter, contentBox.y, gutter, contentBox.h);
	*outViewport = fuiRectMake(contentBox.x, contentBox.y, viewportWidth, contentBox.h);
	// The content flows inside a box far taller than the viewport and lifted by the scroll, so rows past
	// the bottom are CLIPPED rather than squeezed into whatever height was left.
	*outLayoutBox = fuiRectMake(outViewport->x, outViewport->y - scroll, outViewport->w, FUI__SCROLL_LAYOUT_HEIGHT);
}

//! Wheel over the viewport plus the gutter scrollbar, resolved into the new scroll offset
fui_inline float fui__ResolveScroll(fuiContext *context, const char *scrollbarId, const fuiRect viewport, const fuiRect track, const float scroll, const float contentLength) {
	float scrolled = scroll;
	// The wheel answers over the padding between the two as well, which is neither the viewport nor the track
	// but is still plainly inside the scrolling area.
	float wheelRegionWidth = fuiMaxF(track.x + track.w, viewport.x + viewport.w) - viewport.x;
	fuiRect wheelRegion = fuiRectMake(viewport.x, viewport.y, wheelRegionWidth, viewport.h);
	if(context->mouseWheelDelta != 0.0f && fui__CursorIsOver(context, wheelRegion)) {
		scrolled -= context->mouseWheelDelta * context->theme.menuItemHeight * FUI__SCROLL_WHEEL_ROWS;
	}
	float result = fuiScrollbarVertical(context, track, scrollbarId, scrolled, viewport.h, contentLength);
	return(result);
}

//! Shared body of all four panel openers
fui_inline bool fui__BeginPanelEx(fuiContext *context, const char *id, const fuiDock dock, const float x, const float y, const float width, const float height, const bool isScrollable, bool *isOpen) {
	const fuiTheme *theme = &context->theme;

	fuiId panelId = fuiGetId(context, id);
	fuiWidgetState *state = fui__WidgetStateGet(context, panelId);

	// Scope the children AND the panel's own controls under the panel, so two panels may both hold a
	// "Reset" button and both have a collapse arrow without the two colliding.
	fuiPushId(context, id);
	fuiId moveId = fuiGetId(context, "__panelMove");
	fuiId resizeId = fuiGetId(context, "__panelResize");
	fuiId collapseId = fuiGetId(context, "__panelCollapse");
	fuiId closeId = fuiGetId(context, "__panelClose");

	bool isFloating = (dock == FUI_DOCK_NONE);
	bool isCollapsed = (state != fui_null) ? state->isCollapsed : false;
	float titleBarHeight = fui__PanelTitleBarHeight(context);

	// Effective size = what the caller asked for plus whatever the user dragged, floored so it stays grabbable.
	float effectiveWidth = width;
	float effectiveHeight = height;
	if(state != fui_null) {
		effectiveWidth += state->sizeOffset.x;
		effectiveHeight += state->sizeOffset.y;
	}
	if(isFloating || dock == FUI_DOCK_LEFT || dock == FUI_DOCK_RIGHT) {
		effectiveWidth = fuiMaxF(effectiveWidth, FUI__PANEL_MIN_WIDTH);
	}
	if(isFloating || dock == FUI_DOCK_TOP || dock == FUI_DOCK_BOTTOM) {
		float minimumHeight = isCollapsed ? titleBarHeight : FUI__PANEL_MIN_HEIGHT;
		effectiveHeight = fuiMaxF(effectiveHeight, minimumHeight);
		if(isCollapsed) {
			effectiveHeight = titleBarHeight;
		}
	}

	fuiRect rect;
	if(isFloating) {
		// Anchored by its TOP LEFT corner, which y-down makes the plain reading of (x, y): growing the
		// panel extends it down and right, following the corner grip under the cursor.
		rect = fuiRectMake(x, y, effectiveWidth, effectiveHeight);
		if(state != fui_null) {
			rect = fuiRectOffset(rect, state->moveOffset);
		}
	} else {
		fuiRect box = fui__ParentBox(context);
		rect = fui__TakeDockRegion(&box, dock, effectiveWidth, effectiveHeight);
		// The parent has to see the strip as taken before its next child is placed. The panel's own node is
		// not pushed yet, so this still reaches the PARENT.
		fuiLayoutNode *parent = fui__CurrentLayout(context);
		if(parent != fui_null) {
			parent->remaining = box;
		}
	}

	fuiDrawRect(context, rect, theme->panelBackgroundColor);
	// Claim the whole panel, so it takes the cursor from a panel underneath it and swallows clicks even
	// where it holds no widget at all.
	(void)fui__ClaimCursor(context, panelId, rect);

	fuiRect titleBar = fuiRectMake(rect.x, rect.y, rect.w, titleBarHeight);
	fuiDrawRect(context, titleBar, theme->widgetTrackColor);

	// Both marks are drawn into a box of the SAME size, one inset from either end of the bar, so the pair
	// mirrors and the collapse triangle lines up with the caption that starts at that same inset.
	float iconSize = fuiTitleBarIconSize(context);
	float iconY = titleBar.y + (titleBar.h - iconSize) * 0.5f;

	fuiRect collapseButton = fuiRectMake(titleBar.x, titleBar.y, FUI__PANEL_COLLAPSE_BUTTON_WIDTH, titleBar.h);
	fuiInteraction collapseInteraction = fuiInteract(context, collapseId, collapseButton);
	if(collapseInteraction.wasClicked && state != fui_null) {
		state->isCollapsed = !state->isCollapsed;
	}
	fuiRect collapseIcon = fuiRectMake(collapseButton.x + theme->widgetPaddingX, iconY, iconSize, iconSize);
	fuiColor collapseColor = collapseInteraction.isHovered ? theme->accentColor : theme->textColor;
	fuiDrawCollapseGlyph(context, collapseIcon, isCollapsed, collapseColor);

	// A panel whose caller handed over the flag its menu row toggles grows a close button, because a panel
	// reached from a menu has to be dismissable without going back to one.
	bool isClosable = (isOpen != fui_null);
	float closeButtonWidth = isClosable ? FUI__PANEL_CLOSE_BUTTON_WIDTH : 0.0f;
	fuiRect closeButton = fuiRectMake(titleBar.x + titleBar.w - closeButtonWidth, titleBar.y, closeButtonWidth, titleBar.h);
	if(isClosable) {
		fuiInteraction closeInteraction = fuiInteract(context, closeId, closeButton);
		if(closeInteraction.wasClicked) {
			*isOpen = false;
		}
		float closeIconX = closeButton.x + closeButton.w - theme->widgetPaddingX - iconSize;
		fuiRect closeIcon = fuiRectMake(closeIconX, iconY, iconSize, iconSize);
		fuiColor closeColor = closeInteraction.isHovered ? theme->accentColor : theme->textColor;
		fuiDrawCloseGlyph(context, closeIcon, closeColor, theme->widgetBorderThickness);
	}

	float captionWidth = titleBar.w - collapseButton.w - closeButtonWidth;
	fuiRect captionRect = fuiRectMake(collapseButton.x + collapseButton.w, titleBar.y, captionWidth, titleBar.h);
	fui__DrawTextInRect(context, captionRect, id, theme->textColor);

	// Dragging the caption - not the two buttons at either end - moves a floating panel.
	if(isFloating && state != fui_null) {
		fuiInteraction moveInteraction = fuiInteract(context, moveId, captionRect);
		if(moveInteraction.isHeld) {
			state->moveOffset = fuiV2Add(state->moveOffset, context->mouseDelta);
		}
	}

	// The frame goes on LAST of the chrome, so it runs unbroken around the whole panel. Drawn before the
	// title bar it would be painted over along the top and down either side of the caption, leaving a box
	// that is framed everywhere but where the eye starts reading it.
	fui__DrawChromeFrame(context, rect);

	// The resize affordance: the bottom-right corner of a floating panel, and the free EDGE of a docked one -
	// a left dock is resized by its right edge, a top dock by its bottom edge, and so on.
	if(!isCollapsed && state != fui_null) {
		fuiRect grip = fuiRectMake(0.0f, 0.0f, 0.0f, 0.0f);
		fuiRect highlight = grip;
		bool resizesX = false;
		bool resizesY = false;
		float edgeGrip = theme->panelPaddingX;
		float lineThickness = FUI__PANEL_GRIP_HIGHLIGHT_THICKNESS;
		float cornerSize = FUI__PANEL_RESIZE_GRIP_SIZE;
		if(isFloating) {
			grip = fuiRectMake(rect.x + rect.w - cornerSize, rect.y + rect.h - cornerSize, cornerSize, cornerSize);
			highlight = grip;
			resizesX = true;
			resizesY = true;
		} else if(dock == FUI_DOCK_LEFT) {
			grip = fuiRectMake(rect.x + rect.w - edgeGrip, rect.y, edgeGrip, rect.h);
			highlight = fuiRectMake(rect.x + rect.w - lineThickness, rect.y, lineThickness, rect.h);
			resizesX = true;
		} else if(dock == FUI_DOCK_RIGHT) {
			grip = fuiRectMake(rect.x, rect.y, edgeGrip, rect.h);
			highlight = fuiRectMake(rect.x, rect.y, lineThickness, rect.h);
			resizesX = true;
		} else if(dock == FUI_DOCK_TOP) {
			grip = fuiRectMake(rect.x, rect.y + rect.h - edgeGrip, rect.w, edgeGrip);
			highlight = fuiRectMake(rect.x, rect.y + rect.h - lineThickness, rect.w, lineThickness);
			resizesY = true;
		} else if(dock == FUI_DOCK_BOTTOM) {
			grip = fuiRectMake(rect.x, rect.y, rect.w, edgeGrip);
			highlight = fuiRectMake(rect.x, rect.y, rect.w, lineThickness);
			resizesY = true;
		}

		if(resizesX || resizesY) {
			// An edge grip runs the whole side of the panel, so on a docked panel it passes straight through
			// the close button sitting in that corner. The button wins there, or the press starts a resize
			// and the panel can never be closed at all. A drag already under way is untouched.
			bool cursorIsOverCloseButton = isClosable && fuiPointInRect(context->mousePosition, closeButton);
			fuiInteraction resizeInteraction;
			fui__ClearMemory(&resizeInteraction, sizeof(resizeInteraction));
			if(!cursorIsOverCloseButton || context->active == resizeId) {
				resizeInteraction = fuiInteract(context, resizeId, grip);
			}
			if(resizeInteraction.isHeld) {
				if(resizesX) {
					// A right dock grows toward decreasing x, so its drag reads the other way round.
					float horizontalSign = (dock == FUI_DOCK_RIGHT) ? -1.0f : 1.0f;
					state->sizeOffset.x += context->mouseDelta.x * horizontalSign;
				}
				if(resizesY) {
					// A bottom dock is gripped by its TOP edge, so dragging up is what grows it.
					float verticalSign = (dock == FUI_DOCK_BOTTOM) ? -1.0f : 1.0f;
					state->sizeOffset.y += context->mouseDelta.y * verticalSign;
				}
			}
			bool gripIsHot = resizeInteraction.isHovered || resizeInteraction.isHeld;
			if(isFloating) {
				// The corner carries the tick glyph ALWAYS: a resize affordance nobody can see until they
				// happen to hover the right few pixels may as well not exist.
				fuiColor gripColor = gripIsHot ? theme->accentColor : theme->textMutedColor;
				fuiDrawResizeGrip(context, highlight, gripColor, theme->widgetBorderThickness);
			} else if(gripIsHot) {
				// An edge stays a thin line: the corner glyph would read as a corner handle on what is
				// really a full height or full width edge.
				fuiDrawRect(context, highlight, theme->accentColor);
			}
		}
	}

	fuiRect belowTitleBar = fuiRectMake(rect.x, rect.y + titleBarHeight, rect.w, rect.h - titleBarHeight);
	fuiRect contentBox = fui__RectInset(belowTitleBar, theme->panelPaddingX, theme->panelPaddingY);
	if(isCollapsed) {
		contentBox = fuiRectMake(rect.x, rect.y, 0.0f, 0.0f);
	}

	bool doesScroll = isScrollable && !isCollapsed;
	fuiRect clipBox = contentBox;
	fuiRect layoutBox = contentBox;
	fuiRect scrollTrack = fuiRectMake(0.0f, 0.0f, 0.0f, 0.0f);
	if(doesScroll) {
		float scroll = (state != fui_null) ? state->scroll : 0.0f;
		fui__ResolveScrollBoxes(contentBox, scroll, theme->panelPaddingX, &clipBox, &scrollTrack, &layoutBox);
	}

	fuiLayoutNode *node = fui__PushLayout(context, panelId, state, true, FUI_AXIS_VERTICAL, theme->widgetSpacing, layoutBox);
	if(node != fui_null) {
		node->isHidden = isCollapsed;
		node->isScrollable = doesScroll && (state != fui_null);
		node->scrollViewport = clipBox;
		node->scrollTrack = scrollTrack;
		// Pushed only alongside a real node, so fuiEndPanel can tell from the node alone whether to pop it.
		fuiPushClip(context, clipBox);
	}
	return(!isCollapsed);
}

fui_api bool fuiBeginPanel(fuiContext *context, const char *id, const fuiDock dock, const float x, const float y, const float width, const float height) {
	FUI_ASSERT(context != fui_null && id != fui_null);
	if(context == fui_null || id == fui_null) {
		return(false);
	}
	bool *noCloseFlag = fui_null;
	return(fui__BeginPanelEx(context, id, dock, x, y, width, height, false, noCloseFlag));
}

fui_api bool fuiBeginScrollPanel(fuiContext *context, const char *id, const fuiDock dock, const float x, const float y, const float width, const float height) {
	FUI_ASSERT(context != fui_null && id != fui_null);
	if(context == fui_null || id == fui_null) {
		return(false);
	}
	bool *noCloseFlag = fui_null;
	return(fui__BeginPanelEx(context, id, dock, x, y, width, height, true, noCloseFlag));
}

fui_api bool fuiBeginPanelClosable(fuiContext *context, const char *id, const fuiDock dock, const float x, const float y, const float width, const float height, bool *isOpen) {
	FUI_ASSERT(context != fui_null && id != fui_null);
	if(context == fui_null || id == fui_null) {
		return(false);
	}
	return(fui__BeginPanelEx(context, id, dock, x, y, width, height, false, isOpen));
}

fui_api bool fuiBeginScrollPanelClosable(fuiContext *context, const char *id, const fuiDock dock, const float x, const float y, const float width, const float height, bool *isOpen) {
	FUI_ASSERT(context != fui_null && id != fui_null);
	if(context == fui_null || id == fui_null) {
		return(false);
	}
	return(fui__BeginPanelEx(context, id, dock, x, y, width, height, true, isOpen));
}

fui_api void fuiEndPanel(fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return;
	}

	fuiLayoutNode *node = fui__CurrentLayout(context);
	if(node != fui_null) {
		FUI_ASSERT(node->isPanel);
		fuiWidgetState *state = node->state;
		bool doesScroll = node->isScrollable && (state != fui_null);
		fuiRect scrollViewport = node->scrollViewport;
		fuiRect scrollTrack = node->scrollTrack;
		float contentLength = node->usedExtent;
		if(state != fui_null) {
			state->measuredExtent = node->usedExtent;
			state->hasMeasured = true;
		}

		fui__PopLayout(context);
		// The content clip goes first, or the scrollbar in the gutter beside it would be clipped away.
		fuiPopClip(context);

		if(doesScroll) {
			state->scroll = fui__ResolveScroll(context, "__panelScrollbar", scrollViewport, scrollTrack, state->scroll, contentLength);
		}
	} else {
		fui__PopLayout(context);
	}

	fuiPopId(context);
}

// ----------------------------------------------------------------------------
// > Stacks
// ----------------------------------------------------------------------------

//! Resolves what a flow container was asked for into the gap it actually lays out with
fui_inline float fui__ResolveSpacing(const fuiContext *context, const float spacing) {
	if(spacing < 0.0f) {
		return(context->theme.widgetSpacing);
	}
	return(spacing);
}

fui_api void fuiBeginStack(fuiContext *context, const char *id, const fuiAxis axis, const fuiDock dock, const float sizeAcross, const float spacing) {
	FUI_ASSERT(context != fui_null && id != fui_null);
	if(context == fui_null || id == fui_null) {
		return;
	}

	fuiId stackId = fuiGetId(context, id);
	fuiPushId(context, id);

	fuiRect box = fui__ParentBox(context);
	// sizeAcross is the strip THICKNESS, so it is a width for a side dock and a height for a top or bottom one.
	fuiRect region = fui__TakeDockRegion(&box, dock, sizeAcross, sizeAcross);
	fuiLayoutNode *parent = fui__CurrentLayout(context);
	if(parent != fui_null) {
		parent->remaining = box;
	}

	fuiWidgetState *noState = fui_null;
	float resolvedSpacing = fui__ResolveSpacing(context, spacing);
	(void)fui__PushLayout(context, stackId, noState, false, axis, resolvedSpacing, region);
}

//! Shared body of both explicit-rectangle stacks. Unlike fuiBeginStack this takes nothing from the parent
fui_inline void fui__PushStackAt(fuiContext *context, const char *id, const fuiAxis axis, const fuiRect rect, const float spacing, fuiWidgetState *state) {
	fuiId stackId = fuiGetId(context, id);
	fuiPushId(context, id);
	float resolvedSpacing = fui__ResolveSpacing(context, spacing);
	(void)fui__PushLayout(context, stackId, state, false, axis, resolvedSpacing, rect);
}

fui_api void fuiBeginStackAt(fuiContext *context, const char *id, const fuiAxis axis, const fuiRect rect, const float spacing) {
	FUI_ASSERT(context != fui_null && id != fui_null);
	if(context == fui_null || id == fui_null) {
		return;
	}
	fuiWidgetState *noState = fui_null;
	fui__PushStackAt(context, id, axis, rect, spacing, noState);
}

fui_api void fuiBeginScrollStackAt(fuiContext *context, const char *id, const fuiRect rect, const float spacing) {
	FUI_ASSERT(context != fui_null && id != fui_null);
	if(context == fui_null || id == fui_null) {
		return;
	}

	fuiId stackId = fuiGetId(context, id);
	fuiWidgetState *state = fui__WidgetStateGet(context, stackId);
	fuiPushId(context, id);

	float scroll = (state != fui_null) ? state->scroll : 0.0f;
	fuiRect viewport;
	fuiRect track;
	fuiRect layoutBox;
	fui__ResolveScrollBoxes(rect, scroll, context->theme.panelPaddingX, &viewport, &track, &layoutBox);

	float resolvedSpacing = fui__ResolveSpacing(context, spacing);
	fuiLayoutNode *node = fui__PushLayout(context, stackId, state, false, FUI_AXIS_VERTICAL, resolvedSpacing, layoutBox);
	if(node != fui_null) {
		node->isScrollable = (state != fui_null);
		node->scrollViewport = viewport;
		node->scrollTrack = track;
		fuiPushClip(context, viewport);
	}
}

fui_api void fuiEndStack(fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return;
	}

	fuiLayoutNode *node = fui__CurrentLayout(context);
	if(node != fui_null) {
		FUI_ASSERT(!node->isPanel);
		fuiWidgetState *state = node->state;
		bool doesScroll = node->isScrollable && (state != fui_null);
		fuiRect scrollViewport = node->scrollViewport;
		fuiRect scrollTrack = node->scrollTrack;
		float contentLength = node->usedExtent;

		fui__PopLayout(context);

		if(doesScroll) {
			fuiPopClip(context);
			state->scroll = fui__ResolveScroll(context, "__stackScrollbar", scrollViewport, scrollTrack, state->scroll, contentLength);
		}
	} else {
		fui__PopLayout(context);
	}

	fuiPopId(context);
}

// ----------------------------------------------------------------------------
// > Splitters and drag handles
// ----------------------------------------------------------------------------

//! Shared body of both splitters, along whichever axis the caller picked
fui_inline bool fui__Splitter(fuiContext *context, const char *id, const fuiRect grip, float *size, const float minSize, const float maxSize, const float highlightThickness, const bool resizesWidth) {
	fuiId splitterId = fuiGetId(context, id);
	fuiInteraction interaction = fuiInteract(context, splitterId, grip);
	if(interaction.isHeld) {
		float motion = resizesWidth ? context->mouseDelta.x : context->mouseDelta.y;
		*size = fuiClampF(*size + motion, minSize, maxSize);
	}
	if(interaction.isHovered || interaction.isHeld) {
		fuiRect highlight;
		if(resizesWidth) {
			highlight = fuiRectMake(grip.x + (grip.w - highlightThickness) * 0.5f, grip.y, highlightThickness, grip.h);
		} else {
			highlight = fuiRectMake(grip.x, grip.y + (grip.h - highlightThickness) * 0.5f, grip.w, highlightThickness);
		}
		fuiDrawRect(context, highlight, context->theme.accentColor);
	}
	return(interaction.isHeld);
}

fui_api bool fuiVerticalSplitter(fuiContext *context, const char *id, const fuiRect grip, float *width, const float minSize, const float maxSize, const float highlightThickness) {
	FUI_ASSERT(context != fui_null && id != fui_null && width != fui_null);
	if(context == fui_null || id == fui_null || width == fui_null) {
		return(false);
	}
	return(fui__Splitter(context, id, grip, width, minSize, maxSize, highlightThickness, true));
}

fui_api bool fuiHorizontalSplitter(fuiContext *context, const char *id, const fuiRect grip, float *height, const float minSize, const float maxSize, const float highlightThickness) {
	FUI_ASSERT(context != fui_null && id != fui_null && height != fui_null);
	if(context == fui_null || id == fui_null || height == fui_null) {
		return(false);
	}
	return(fui__Splitter(context, id, grip, height, minSize, maxSize, highlightThickness, false));
}

fui_api bool fuiDragHandle(fuiContext *context, const char *id, const fuiRect handle, fuiVec2 *offset) {
	FUI_ASSERT(context != fui_null && id != fui_null && offset != fui_null);
	if(context == fui_null || id == fui_null || offset == fui_null) {
		return(false);
	}
	fuiId handleId = fuiGetId(context, id);
	fuiInteraction interaction = fuiInteract(context, handleId, handle);
	if(interaction.isHeld) {
		*offset = fuiV2Add(*offset, context->mouseDelta);
	}
	return(interaction.isHeld);
}

// ----------------------------------------------------------------------------
// > Groups
// ----------------------------------------------------------------------------

fui_api void fuiBeginGroup(fuiContext *context, const char *label) {
	FUI_ASSERT(context != fui_null && label != fui_null);
	if(context == fui_null || label == fui_null) {
		return;
	}
	// The heading also opens an identifier scope, so a "State" checkbox under a "State" group hashes
	// distinctly from one under its neighbour.
	fuiPushId(context, label);
	fuiRect labelSlot = fuiLayoutSlot(context, context->theme.menuItemHeight);
	fuiLabel(context, labelSlot, label);
}

fui_api void fuiEndGroup(fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return;
	}
	fuiRect separatorSlot = fuiLayoutSlot(context, context->theme.groupSeparatorHeight);
	fuiSeparator(context, separatorSlot);
	fuiPopId(context);
}

//! Height of a group box's title bar, following the font the same way a panel's does
fui_inline float fui__GroupBoxTitleHeight(const fuiContext *context) {
	float result = context->theme.fontHeight + FUI__GROUP_BOX_TITLE_PADDING * 2.0f;
	return(result);
}

fui_api float fuiGroupBoxContentHeight(const fuiContext *context, const uint32_t rowCount, const float rowHeight) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null || rowCount == 0) {
		return(0.0f);
	}
	float gapCount = (float)(rowCount - 1u);
	float result = (float)rowCount * rowHeight + gapCount * context->theme.widgetSpacing;
	return(result);
}

fui_api float fuiGroupBoxHeight(const fuiContext *context, const uint32_t rowCount, const float rowHeight) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return(0.0f);
	}
	float titleHeight = fui__GroupBoxTitleHeight(context);
	float contentHeight = fuiGroupBoxContentHeight(context, rowCount, rowHeight);
	float result = titleHeight + contentHeight + context->theme.panelPaddingY * 2.0f;
	return(result);
}

//! Shared body of both group box openers: reserve the slot, draw the frame and open the inner flow
fui_inline void fui__PushGroupBox(fuiContext *context, const char *label, const float contentHeight, fuiWidgetState *state) {
	const fuiTheme *theme = &context->theme;
	float titleHeight = fui__GroupBoxTitleHeight(context);
	float boxHeight = titleHeight + contentHeight + theme->panelPaddingY * 2.0f;
	fuiRect box = fuiLayoutSlot(context, boxHeight);

	// A FILLED title bar captioned in the accent color. There is no bold face to reach for, so the fill is
	// what makes a heading read as a heading rather than as one more label row.
	fuiRect titleBar = fuiRectMake(box.x, box.y, box.w, titleHeight);
	fuiDrawRect(context, titleBar, theme->widgetTrackColor);
	fui__DrawTextInRect(context, titleBar, label, theme->accentColor);

	// The same frame a panel and a dialog carry, drawn AFTER the title bar so it runs unbroken around the
	// whole box: a group box is a panel that happens to sit inside another one, and it should read as one.
	fui__DrawChromeFrame(context, box);

	float contentX = box.x + theme->panelPaddingX;
	float contentY = box.y + titleHeight + theme->panelPaddingY;
	float contentWidth = fuiMaxF(0.0f, box.w - theme->panelPaddingX * 2.0f);
	fuiRect contentBox = fuiRectMake(contentX, contentY, contentWidth, contentHeight);
	fui__PushStackAt(context, label, FUI_AXIS_VERTICAL, contentBox, theme->widgetSpacing, state);
}

fui_api void fuiBeginGroupBox(fuiContext *context, const char *label, const float contentHeight) {
	FUI_ASSERT(context != fui_null && label != fui_null);
	if(context == fui_null || label == fui_null) {
		return;
	}
	fuiWidgetState *noState = fui_null;
	fui__PushGroupBox(context, label, contentHeight, noState);
}

fui_api void fuiBeginGroupBoxAuto(fuiContext *context, const char *label) {
	FUI_ASSERT(context != fui_null && label != fui_null);
	if(context == fui_null || label == fui_null) {
		return;
	}
	// The inner flow's identifier is the one fui__PushStackAt derives from the same label under the same
	// scope, so looking the state up here and handing it down keeps ONE identifier for measure and lookup.
	fuiId contentId = fuiGetId(context, label);
	fuiWidgetState *state = fui__WidgetStateGet(context, contentId);
	float measuredHeight = (state != fui_null && state->hasMeasured) ? state->measuredExtent : 0.0f;
	fui__PushGroupBox(context, label, measuredHeight, state);
}

fui_api void fuiEndGroupBox(fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return;
	}
	// An auto sized box remembers what its rows actually consumed, which is what the next build lays out with.
	fuiLayoutNode *node = fui__CurrentLayout(context);
	if(node != fui_null && node->state != fui_null) {
		node->state->measuredExtent = node->usedExtent;
		node->state->hasMeasured = true;
	}
	fuiEndStack(context);
}

// ----------------------------------------------------------------------------
// > Widgets
// ----------------------------------------------------------------------------

fui_api void fuiLabelEx(fuiContext *context, const fuiRect rect, const char *text, const bool multiline, const bool wordWrap) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return;
	}
	if(multiline) {
		size_t textLength = fui__StringLength(text);
		fui__DrawTextBlockInRect(context, rect, text, textLength, wordWrap, context->theme.textColor);
	} else {
		fui__DrawTextInRect(context, rect, text, context->theme.textColor);
	}
}

fui_api void fuiLabel(fuiContext *context, const fuiRect rect, const char *text) {
	fuiLabelEx(context, rect, text, false, false);
}

fui_api void fuiSeparator(fuiContext *context, const fuiRect rect) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return;
	}
	float lineY = rect.y + rect.h * 0.5f;
	fuiVec2 lineStart = fuiV2(rect.x, lineY);
	fuiVec2 lineEnd = fuiV2(rect.x + rect.w, lineY);
	fuiDrawLine(context, lineStart, lineEnd, context->theme.panelBorderColor, context->theme.widgetBorderThickness);
}

//! A button's box and caption in its current state. Shared by the plain and the repeating button, which
//! differ only in WHEN they fire.
fui_inline void fui__DrawButton(fuiContext *context, const fuiRect rect, const char *label, const bool enabled, const fuiInteraction interaction) {
	const fuiTheme *theme = &context->theme;
	fuiColor fill = enabled ? fui__WidgetFillColor(context, interaction) : theme->widgetTrackColor;
	fuiColor labelColor = enabled ? theme->textColor : theme->textMutedColor;
	// A disabled button is not a button that can be pushed, so it lies flush in the panel rather than standing
	// out of it - the shape says as much as the muted caption does.
	bool buttonIsPushed = interaction.isHeld || !enabled;
	fui__Relief relief = buttonIsPushed ? FUI__RELIEF_SUNKEN : FUI__RELIEF_RAISED;
	fui__DrawBevelBox(context, rect, fill, relief);
	fuiRect labelRect = fui__PressedContentRect(context, rect, interaction.isHeld);
	fui__DrawTextCenteredInRect(context, labelRect, label, labelColor);
}

fui_api bool fuiButtonEx(fuiContext *context, const fuiRect rect, const char *label, const bool enabled) {
	FUI_ASSERT(context != fui_null && label != fui_null);
	if(context == fui_null || label == fui_null) {
		return(false);
	}
	fuiInteraction interaction;
	fui__ClearMemory(&interaction, sizeof(interaction));
	if(enabled) {
		fuiId id = fuiGetId(context, label);
		interaction = fuiInteract(context, id, rect);
	}
	fui__DrawButton(context, rect, label, enabled, interaction);
	return(interaction.wasClicked);
}

fui_api bool fuiButton(fuiContext *context, const fuiRect rect, const char *label) {
	return(fuiButtonEx(context, rect, label, true));
}

fui_api bool fuiButtonRepeat(fuiContext *context, const fuiRect rect, const char *label) {
	FUI_ASSERT(context != fui_null && label != fui_null);
	if(context == fui_null || label == fui_null) {
		return(false);
	}

	fuiId id = fuiGetId(context, label);
	fuiInteraction interaction = fuiInteract(context, id, rect);

	// Fires on the PRESS edge rather than on release, then again on the cadence every held key in the
	// library repeats at. One widget is active at a time, so one timer on the context serves them all.
	bool didFire = interaction.wasPressed;
	if(interaction.wasPressed) {
		context->activeRepeatTimer = FUI_KEY_REPEAT_DELAY;
	} else if(interaction.isHeld) {
		if(!interaction.isHovered) {
			// Sliding off while held stops the repeat and rearms the initial delay, so coming back onto the
			// button does not immediately resume firing.
			context->activeRepeatTimer = FUI_KEY_REPEAT_DELAY;
		} else {
			context->activeRepeatTimer -= context->frameTime;
			if(context->activeRepeatTimer <= 0.0f) {
				context->activeRepeatTimer = FUI_KEY_REPEAT_INTERVAL;
				didFire = true;
			}
		}
	}

	const bool enabled = true;
	fui__DrawButton(context, rect, label, enabled, interaction);
	return(didFire);
}

fui_api bool fuiCell(fuiContext *context, const fuiRect rect, const char *id, bool *outIsHovered) {
	FUI_ASSERT(context != fui_null && id != fui_null);
	if(context == fui_null || id == fui_null) {
		if(outIsHovered != fui_null) {
			*outIsHovered = false;
		}
		return(false);
	}
	fuiId cellId = fuiGetId(context, id);
	fuiInteraction interaction = fuiInteract(context, cellId, rect);
	if(outIsHovered != fui_null) {
		*outIsHovered = interaction.isHovered;
	}
	return(interaction.wasClicked);
}

fui_api bool fuiCheckbox(fuiContext *context, const fuiRect rect, const char *label, bool *value) {
	FUI_ASSERT(context != fui_null && label != fui_null && value != fui_null);
	if(context == fui_null || label == fui_null || value == fui_null) {
		return(false);
	}

	fuiId id = fuiGetId(context, label);
	fuiInteraction interaction = fuiInteract(context, id, rect);
	bool didChange = interaction.wasClicked;
	if(didChange) {
		*value = !*value;
	}

	const fuiTheme *theme = &context->theme;
	float boxSide = theme->fontHeight;
	fuiRect box = fuiRectMake(rect.x + theme->widgetPaddingX, rect.y + (rect.h - boxSide) * 0.5f, boxSide, boxSide);
	fuiColor boxFill = interaction.isHovered ? theme->widgetHoveredColor : theme->widgetTrackColor;
	// A checkbox box is a WELL the mark sits in rather than a face that can be pushed, so it is bevelled the
	// other way round from a button and stays that way whether it is ticked or not.
	fui__DrawBevelBox(context, box, boxFill, FUI__RELIEF_SUNKEN);
	if(*value) {
		fuiRect checkMark = fui__CheckMarkRect(context, box);
		fuiDrawRect(context, checkMark, theme->accentColor);
	}

	float labelX = box.x + box.w + FUI__WIDGET_LABEL_GAP;
	fui__DrawTextLeftAligned(context, rect, labelX, label, theme->textColor);
	return(didChange);
}

fui_api bool fuiRadio(fuiContext *context, const fuiRect rect, const char *label, int32_t *selected, const int32_t option) {
	FUI_ASSERT(context != fui_null && label != fui_null && selected != fui_null);
	if(context == fui_null || label == fui_null || selected == fui_null) {
		return(false);
	}

	fuiId id = fuiGetId(context, label);
	fuiInteraction interaction = fuiInteract(context, id, rect);
	bool didChange = interaction.wasClicked && (*selected != option);
	if(didChange) {
		*selected = option;
	}

	const fuiTheme *theme = &context->theme;
	// A square marker rather than a circle, so it reuses the rectangle path and stays crisp at any size.
	float markerSide = theme->fontHeight;
	fuiRect marker = fuiRectMake(rect.x + theme->widgetPaddingX, rect.y + (rect.h - markerSide) * 0.5f, markerSide, markerSide);
	fuiColor markerFill = interaction.isHovered ? theme->widgetHoveredColor : theme->widgetTrackColor;
	// The same well a checkbox puts its tick in, so the two read as one family of widget.
	fui__DrawBevelBox(context, marker, markerFill, FUI__RELIEF_SUNKEN);
	if(*selected == option) {
		fuiRect dot = fui__CheckMarkRect(context, marker);
		fuiDrawRect(context, dot, theme->accentColor);
	}

	float labelX = marker.x + marker.w + FUI__WIDGET_LABEL_GAP;
	fui__DrawTextLeftAligned(context, rect, labelX, label, theme->textColor);
	return(didChange);
}

//! Sets a value from where the cursor sits across a track. Returns true when it moved
fui_inline bool fui__SliderSetFromCursor(fuiContext *context, const fuiRect rect, float *value, const float minValue, const float maxValue) {
	if(rect.w <= 0.0f) {
		return(false);
	}
	float fraction = fuiClampF((context->mousePosition.x - rect.x) / rect.w, 0.0f, 1.0f);
	float newValue = minValue + fraction * (maxValue - minValue);
	if(newValue != *value) {
		*value = newValue;
		return(true);
	}
	return(false);
}

//! The track, its knob and the caption over it
fui_inline void fui__DrawSlider(fuiContext *context, const fuiRect rect, const float value, const float minValue, const float maxValue, const char *displayText, const fuiColor textColor, const fuiInteraction interaction) {
	const fuiTheme *theme = &context->theme;
	fuiDrawRect(context, rect, theme->widgetTrackColor);
	fuiDrawRectOutline(context, rect, theme->panelBorderColor, theme->widgetBorderThickness);

	float range = maxValue - minValue;
	float clampedValue = fuiClampF(value, minValue, maxValue);
	float fraction = (range != 0.0f) ? ((clampedValue - minValue) / range) : 0.0f;
	fuiRect knob = fuiRectMake(rect.x + fraction * (rect.w - FUI__SLIDER_KNOB_WIDTH), rect.y, FUI__SLIDER_KNOB_WIDTH, rect.h);
	fuiColor knobColor = interaction.isHeld ? theme->accentColor : (interaction.isHovered ? theme->widgetHoveredColor : theme->knobColor);
	fuiDrawRect(context, knob, knobColor);

	if(displayText != fui_null) {
		fui__DrawTextInRect(context, rect, displayText, textColor);
	}
}

fui_api bool fuiSliderFloat(fuiContext *context, const fuiRect rect, const char *id, float *value, const float minValue, const float maxValue) {
	FUI_ASSERT(context != fui_null && id != fui_null && value != fui_null);
	if(context == fui_null || id == fui_null || value == fui_null) {
		return(false);
	}

	fuiId sliderId = fuiGetId(context, id);
	fuiInteraction interaction = fuiInteract(context, sliderId, rect);
	bool didChange = false;
	if(interaction.isHeld) {
		// Pressing anywhere on the track jumps the knob there, rather than only the knob being grabbable.
		didChange = fui__SliderSetFromCursor(context, rect, value, minValue, maxValue);
	}

	const char *noDisplayText = fui_null;
	fui__DrawSlider(context, rect, *value, minValue, maxValue, noDisplayText, context->theme.textColor, interaction);
	return(didChange);
}

fui_api bool fuiSliderFloatEx(fuiContext *context, const fuiRect rect, const char *id, float *value, const float minValue, const float maxValue, const float step, const bool liveUpdate, const bool enabled, const char *displayText, bool *outDidBegin, bool *outDidEnd) {
	FUI_ASSERT(context != fui_null && id != fui_null && value != fui_null);
	if(outDidBegin != fui_null) {
		*outDidBegin = false;
	}
	if(outDidEnd != fui_null) {
		*outDidEnd = false;
	}
	if(context == fui_null || id == fui_null || value == fui_null) {
		return(false);
	}

	const fuiTheme *theme = &context->theme;
	if(!enabled) {
		// Drawn so the row still shows its value, but with no interaction at all - which is what a caller
		// relies on to keep a mixed selection from being edited by a control that shows only one of them.
		fuiDrawRect(context, rect, theme->widgetTrackColor);
		fuiDrawRectOutline(context, rect, theme->panelBorderColor, theme->widgetBorderThickness);
		if(displayText != fui_null) {
			fui__DrawTextInRect(context, rect, displayText, theme->textMutedColor);
		}
		return(false);
	}

	fuiId sliderId = fuiGetId(context, id);
	// Captured on either side of the interaction, because a drag that ends with the cursor OFF the track
	// still ends - so "released" cannot be read off wasClicked, which asks whether it ended ON the widget.
	bool wasHeldBefore = (context->active == sliderId);
	fuiInteraction interaction = fuiInteract(context, sliderId, rect);
	bool isHeldNow = (context->active == sliderId);

	bool didChange = false;
	if(interaction.isHeld) {
		didChange = fui__SliderSetFromCursor(context, rect, value, minValue, maxValue);
	}

	// Snap onto the step grid, so the value lands on discrete increments instead of wherever the pixel fell.
	if(didChange && step > 0.0f) {
		float stepsFromMinimum = (*value - minValue) / step;
		float roundedSteps = (float)(int32_t)(stepsFromMinimum + 0.5f);
		*value = fuiClampF(minValue + roundedSteps * step, minValue, maxValue);
	}

	bool dragDidBegin = (!wasHeldBefore && isHeldNow);
	bool dragDidEnd = (wasHeldBefore && !isHeldNow);
	if(outDidBegin != fui_null) {
		*outDidBegin = dragDidBegin;
	}
	if(outDidEnd != fui_null) {
		*outDidEnd = dragDidEnd;
	}

	// A deferred slider keeps tracking the cursor - the knob and the caption follow it - and only the
	// caller's apply waits for the release, which is what makes one drag ONE undoable edit.
	bool shouldApply = liveUpdate ? didChange : dragDidEnd;
	fui__DrawSlider(context, rect, *value, minValue, maxValue, displayText, theme->textColor, interaction);
	return(shouldApply);
}

fui_api bool fuiDragFloat(fuiContext *context, const fuiRect rect, const char *id, float *value, const float speed, const float minValue, const float maxValue) {
	FUI_ASSERT(context != fui_null && id != fui_null && value != fui_null);
	if(context == fui_null || id == fui_null || value == fui_null) {
		return(false);
	}

	fuiId dragId = fuiGetId(context, id);
	fuiInteraction interaction = fuiInteract(context, dragId, rect);
	bool didChange = false;
	if(interaction.isHeld && context->mouseDelta.x != 0.0f) {
		float newValue = fuiClampF(*value + context->mouseDelta.x * speed, minValue, maxValue);
		if(newValue != *value) {
			*value = newValue;
			didChange = true;
		}
	}

	const fuiTheme *theme = &context->theme;
	fuiColor fill = fui__WidgetFillColor(context, interaction);
	fuiDrawRect(context, rect, fill);
	fuiDrawRectOutline(context, rect, theme->panelBorderColor, theme->widgetBorderThickness);

	char valueText[FUI__VALUE_TEXT_CAPACITY];
	(void)fui__FormatString(valueText, sizeof(valueText), "%.3f", (double)*value);
	fui__DrawTextInRect(context, rect, valueText, theme->textColor);
	return(didChange);
}

// ----------------------------------------------------------------------------
// > Cursor
// ----------------------------------------------------------------------------

static void fui__ApplyCursor(fuiContext *context) {
	if(context->platform.setCursor == fui_null) {
		return;
	}
	// Only on a change. A host that turns this into a system call would otherwise make one every frame,
	// and a two-pass caller would make two.
	if(context->cursor == context->appliedCursor) {
		return;
	}
	context->appliedCursor = context->cursor;
	context->platform.setCursor(context->platform.userData, context->cursor);
}

// ----------------------------------------------------------------------------
// > Tooltip
// ----------------------------------------------------------------------------

fui_api void fuiTooltip(fuiContext *context, const fuiRect hoverRect, const char *text) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null || !context->isBuildingFrame || text == fui_null || text[0] == '\0') {
		return;
	}
	// The same gate an ordinary widget hit tests under, clip included: a row scrolled out of its panel is
	// not under the cursor, so it must not speak for the hole it left behind either.
	if(!fui__CursorIsOver(context, hoverRect)) {
		return;
	}
	// The rest timer belongs to the TEXT, not to the area: sliding onto something that says something else
	// starts the wait over, while staying on one thing keeps it counting.
	bool sameAsPending = fui__StringsAreEqual(text, context->tooltipText);
	if(!sameAsPending) {
		(void)fui__CopyString(context->tooltipText, sizeof(context->tooltipText), text);
		context->tooltipHoverSeconds = 0.0f;
	}
	context->tooltipRequested = true;
	context->tooltipCursor = context->mousePosition;
}

static void fui__DrawPendingTooltip(fuiContext *context) {
	if(!context->tooltipRequested || context->tooltipText[0] == '\0') {
		context->tooltipHoverSeconds = 0.0f;
		context->tooltipText[0] = '\0';
		return;
	}

	// Advanced here rather than in fuiBeginFrame so that one frame of hovering counts once. A draw pass
	// carries no frame time, so a two-pass caller does not count its hover twice.
	context->tooltipHoverSeconds += context->frameTime;
	if(context->tooltipHoverSeconds < context->theme.tooltipDelaySeconds) {
		return;
	}
	if(context->font == fui_null) {
		return;
	}

	const fuiTheme *theme = &context->theme;
	fuiTextLine lines[FUI_MAX_TEXT_LINES];
	const bool tooltipsDoNotWrap = false;
	const float noWrapWidth = 0.0f;
	uint32_t lineCount = fuiBreakTextLines(context, context->tooltipText, 0, theme->tooltipFontHeight, tooltipsDoNotWrap, noWrapWidth, lines, (uint32_t)FUI_MAX_TEXT_LINES);
	if(lineCount == 0) {
		return;
	}

	float textHeight = theme->tooltipFontHeight;
	float widestLine = 0.0f;
	for(uint32_t lineIndex = 0; lineIndex < lineCount; ++lineIndex) {
		fuiVec2 lineSize = fuiMeasureText(context, lines[lineIndex].start, lines[lineIndex].length, textHeight);
		widestLine = fuiMaxF(widestLine, lineSize.x);
	}
	float lineStep = textHeight + theme->tooltipLineSpacing;
	float linesHeight = (float)lineCount * textHeight + (float)(lineCount - 1u) * theme->tooltipLineSpacing;
	float boxWidth = widestLine + theme->tooltipPaddingX * 2.0f;
	float boxHeight = linesHeight + theme->tooltipPaddingY * 2.0f;

	// Placed below and to the right of the cursor. When that would run off an edge the box FLIPS to the
	// other side of the cursor rather than creeping along the edge, and the clamp after that is what makes
	// "always fully visible" true rather than a hope.
	fuiVec2 cursorPosition = context->tooltipCursor;
	float windowWidth = (float)context->windowSize.x;
	float windowHeight = (float)context->windowSize.y;
	float margin = theme->tooltipScreenMargin;
	float boxLeft = cursorPosition.x + theme->tooltipCursorOffsetX;
	if(boxLeft + boxWidth > windowWidth - margin) {
		boxLeft = cursorPosition.x - theme->tooltipCursorOffsetX - boxWidth;
	}
	float boxTop = cursorPosition.y + theme->tooltipCursorOffsetY;
	if(boxTop + boxHeight > windowHeight - margin) {
		boxTop = cursorPosition.y - theme->tooltipCursorOffsetY - boxHeight;
	}
	float furthestLeft = windowWidth - margin - boxWidth;
	float furthestTop = windowHeight - margin - boxHeight;
	boxLeft = fuiClampF(boxLeft, margin, fuiMaxF(furthestLeft, margin));
	boxTop = fuiClampF(boxTop, margin, fuiMaxF(furthestTop, margin));

	// The clip stack is emptied first: a tooltip is the last thing drawn and belongs over every panel,
	// popup and modal, and the stack is already unwound to nothing by the time a frame ends anyway.
	context->clipDepth = 0;

	fuiRect box = fuiRectMake(boxLeft, boxTop, boxWidth, boxHeight);
	fuiDrawRect(context, box, theme->tooltipBackgroundColor);
	fuiDrawRectOutline(context, box, theme->tooltipBorderColor, theme->widgetBorderThickness);
	for(uint32_t lineIndex = 0; lineIndex < lineCount; ++lineIndex) {
		float lineTop = box.y + theme->tooltipPaddingY + (float)lineIndex * lineStep;
		fuiVec2 linePosition = fuiV2(box.x + theme->tooltipPaddingX, lineTop);
		fuiDrawText(context, lines[lineIndex].start, lines[lineIndex].length, linePosition, textHeight, theme->tooltipTextColor);
	}
}

// ----------------------------------------------------------------------------
// > Keyboard focus
// ----------------------------------------------------------------------------

//! Puts one widget into the tab chain, and moves the focus onto it when tab pointed here
fui_inline void fui__RegisterFocusable(fuiContext *context, const fuiId id) {
	if(context->firstFocusableThisFrame == FUI_ID_NONE) {
		context->firstFocusableThisFrame = id;
	}
	if(fuiKeyWentDown(context, FUI_KEY_TAB) && !context->tabWasConsumedThisFrame) {
		bool takeTheFirstOne = (context->focused == FUI_ID_NONE);
		bool comesAfterTheFocusedOne = (context->previousFocusableThisFrame == context->focused);
		if(takeTheFirstOne || comesAfterTheFocusedOne) {
			context->focused = id;
			context->tabWasConsumedThisFrame = true;
		}
	}
	context->previousFocusableThisFrame = id;
}

// ----------------------------------------------------------------------------
// > Text editing
// ----------------------------------------------------------------------------

//! Width of the blinking text caret
#define FUI__CARET_WIDTH 2.0f
//! How far right of the character boundary the caret is drawn, so it sits between two letters
#define FUI__CARET_X_OFFSET 1.0f

//! True for a byte that continues a multi-byte character rather than starting one
fui_inline bool fui__IsUtf8Continuation(const char byte) {
	bool result = (((unsigned char)byte & 0xC0u) == 0x80u);
	return(result);
}

//! The character boundary before the given byte offset, which is where the left arrow key goes
fui_inline int32_t fui__PreviousCodepointStart(const char *text, const int32_t offset) {
	int32_t result = offset - 1;
	while(result > 0 && fui__IsUtf8Continuation(text[result])) {
		--result;
	}
	return(result < 0 ? 0 : result);
}

//! The character boundary after the given byte offset, which is where the right arrow key goes
fui_inline int32_t fui__NextCodepointStart(const char *text, const int32_t length, const int32_t offset) {
	int32_t result = offset + 1;
	while(result < length && fui__IsUtf8Continuation(text[result])) {
		++result;
	}
	return(result > length ? length : result);
}

//! Pulls a byte offset back onto the nearest character boundary at or before it
fui_inline int32_t fui__SnapToCodepointStart(const char *text, const int32_t length, const int32_t offset) {
	int32_t result = offset;
	if(result < 0) {
		result = 0;
	}
	if(result > length) {
		result = length;
	}
	while(result > 0 && result < length && fui__IsUtf8Continuation(text[result])) {
		--result;
	}
	return(result);
}

/*!
	Pixel width of the first byteCount bytes of a string.

	This exists because fuiMeasureText reads a length of 0 as "measure the whole zero terminated string",
	which is exactly the wrong answer for the prefix in front of a caret sitting at position 0.
*/
fui_inline float fui__TextPrefixWidth(const fuiContext *context, const char *text, const int32_t byteCount, const float pixelHeight) {
	if(byteCount <= 0) {
		return(0.0f);
	}
	fuiVec2 prefixSize = fuiMeasureText(context, text, (size_t)byteCount, pixelHeight);
	return(prefixSize.x);
}

//! True while a range of the focused field's text is selected
fui_inline bool fui__HasSelection(const fuiContext *context) {
	bool result = (context->selectionAnchor != context->caretPosition);
	return(result);
}

//! Lower byte offset of the selection, which is where the caret is put when it collapses to the left
fui_inline int32_t fui__SelectionStart(const fuiContext *context) {
	int32_t result = (context->selectionAnchor < context->caretPosition) ? context->selectionAnchor : context->caretPosition;
	return(result);
}

//! Upper byte offset of the selection
fui_inline int32_t fui__SelectionEnd(const fuiContext *context) {
	int32_t result = (context->selectionAnchor > context->caretPosition) ? context->selectionAnchor : context->caretPosition;
	return(result);
}

//! Moves the caret, taking the selection anchor with it unless the move is meant to extend the selection
fui_inline void fui__SetCaret(fuiContext *context, const int32_t position, const bool extendSelection) {
	context->caretPosition = position;
	if(!extendSelection) {
		context->selectionAnchor = position;
	}
	// A caret that happens to be in the dark half of its blink while somebody is typing reads as a lost
	// keystroke, so every move starts the blink over at solid.
	context->caretBlinkTime = 0.0f;
}

//! Removes a byte range from a text buffer, moving the tail and its terminator down over it
fui_inline void fui__TextEraseRange(char *buffer, int32_t *length, const int32_t from, const int32_t to) {
	int32_t currentLength = *length;
	if(from >= to || from < 0 || to > currentLength) {
		return;
	}
	fui__MoveMemory(&buffer[from], &buffer[to], (size_t)(currentLength - to));
	*length = currentLength - (to - from);
	buffer[*length] = '\0';
}

//! Inserts bytes at an offset, moving the tail and its terminator up out of the way. False when it does not fit
fui_inline bool fui__TextInsertBytes(char *buffer, const int32_t capacity, int32_t *length, const int32_t at, const char *bytes, const int32_t byteCount) {
	int32_t currentLength = *length;
	if(byteCount <= 0 || (currentLength + byteCount) > (capacity - 1)) {
		return(false);
	}
	fui__MoveMemory(&buffer[at + byteCount], &buffer[at], (size_t)(currentLength - at));
	fui__CopyMemory(&buffer[at], bytes, (size_t)byteCount);
	*length = currentLength + byteCount;
	buffer[*length] = '\0';
	return(true);
}

//! Deletes whatever is selected and collapses the caret onto the gap. False when nothing was selected
fui_inline bool fui__DeleteSelection(fuiContext *context, char *buffer, int32_t *length) {
	if(!fui__HasSelection(context)) {
		return(false);
	}
	int32_t from = fui__SelectionStart(context);
	int32_t to = fui__SelectionEnd(context);
	fui__TextEraseRange(buffer, length, from, to);
	const bool collapseTheSelection = false;
	fui__SetCaret(context, from, collapseTheSelection);
	return(true);
}

//! Replaces the selection with one codepoint and steps the caret past it
fui_inline bool fui__InsertCodepoint(fuiContext *context, char *buffer, const int32_t capacity, int32_t *length, const uint32_t codePoint) {
	char encoded[4];
	uint32_t encodedLength = fuiEncodeUtf8(codePoint, encoded);
	if(encodedLength == 0) {
		return(false);
	}
	bool didChange = fui__DeleteSelection(context, buffer, length);
	if(!fui__TextInsertBytes(buffer, capacity, length, context->caretPosition, encoded, (int32_t)encodedLength)) {
		return(didChange);
	}
	const bool collapseTheSelection = false;
	fui__SetCaret(context, context->caretPosition + (int32_t)encodedLength, collapseTheSelection);
	return(true);
}

//! Replaces the selection with a whole string, dropping the control characters a field cannot hold
fui_inline bool fui__InsertText(fuiContext *context, char *buffer, const int32_t capacity, int32_t *length, const char *text, const bool allowNewLines) {
	bool didChange = fui__DeleteSelection(context, buffer, length);
	size_t textLength = fui__StringLength(text);
	size_t offset = 0;
	while(offset < textLength) {
		uint32_t codePoint = fuiDecodeUtf8(text, textLength, &offset);
		bool isNewLine = (codePoint == (uint32_t)'\n');
		bool isControlCode = (codePoint < 0x20u) || (codePoint == 0x7Fu);
		if(isControlCode && !(isNewLine && allowNewLines)) {
			continue;
		}
		if(fui__InsertCodepoint(context, buffer, capacity, length, codePoint)) {
			didChange = true;
		}
	}
	return(didChange);
}

//! The visual row a byte offset falls on, plus how far into that row it is. A boundary belongs to the row that ENDS there
fui_inline uint32_t fui__CaretRowIndex(const fuiTextLine *lines, const uint32_t lineCount, const char *buffer, const int32_t caret, int32_t *outColumn) {
	for(uint32_t lineIndex = 0; lineIndex < lineCount; ++lineIndex) {
		int32_t rowStart = (int32_t)(lines[lineIndex].start - buffer);
		int32_t rowEnd = rowStart + (int32_t)lines[lineIndex].length;
		if(caret <= rowEnd) {
			if(outColumn != fui_null) {
				*outColumn = caret - rowStart;
			}
			return(lineIndex);
		}
	}
	if(lineCount == 0) {
		if(outColumn != fui_null) {
			*outColumn = 0;
		}
		return(0);
	}
	uint32_t lastLine = lineCount - 1u;
	if(outColumn != fui_null) {
		*outColumn = caret - (int32_t)(lines[lastLine].start - buffer);
	}
	return(lastLine);
}

//! The character boundary in one row of text nearest to the cursor's x
fui_inline int32_t fui__ColumnFromCursorX(const fuiContext *context, const float rowLeftX, const char *rowText, const int32_t rowLength) {
	float relativeX = context->mousePosition.x - rowLeftX;
	if(relativeX <= 0.0f) {
		return(0);
	}
	float pixelHeight = context->theme.fontHeight;
	int32_t bestColumn = 0;
	float bestDistance = relativeX;
	int32_t column = 0;
	while(column < rowLength) {
		column = fui__NextCodepointStart(rowText, rowLength, column);
		float boundaryX = fui__TextPrefixWidth(context, rowText, column, pixelHeight);
		float distance = FUI_FABSF(boundaryX - relativeX);
		if(distance < bestDistance) {
			bestDistance = distance;
			bestColumn = column;
		}
	}
	return(bestColumn);
}

//! Maps the cursor to a byte offset in a single line field
fui_inline int32_t fui__CaretFromCursorSingleLine(const fuiContext *context, const fuiRect rect, const char *buffer, const int32_t length) {
	float textLeftX = rect.x + context->theme.widgetPaddingX;
	int32_t result = fui__ColumnFromCursorX(context, textLeftX, buffer, length);
	return(result);
}

//! How many rows a scrolling field keeps ABOVE the one at the top of its box. One is enough, and it is
//! what lets the up arrow step off the top edge onto a row that is already laid out
#define FUI__TEXT_OVERSCAN_LINES 1

/**
* @struct fui__TextWindow
* @brief The rows of a multiline field that are laid out right now, and where the first of them is drawn.
* @note A field showing part of a large document lays out a WINDOW of its lines rather than the first few
*       of them, so every row index below is an index into that window and not into the document.
*/
typedef struct fui__TextWindow {
	//! Which document line the stored rows begin at
	int32_t storedFirstLine;
	//! Which document line is the top one the box shows
	int32_t firstVisibleLine;
	//! How many lines the whole document has
	int32_t totalLineCount;
	//! How many rows the box shows at once
	int32_t visibleLineCount;
	//! Where the FIRST STORED row is drawn, which is above the box by whatever was kept for the arrow keys
	float firstRowTopY;
} fui__TextWindow;

//! How many lines a whole buffer breaks into. Counted only when something it depends on changed, because
//! counting means walking the whole document and a field is rebuilt every frame
fui_inline int32_t fui__TextCountLines(fuiContext *context, fuiWidgetState *state, const char *buffer, const int32_t length, const float pixelHeight, const bool wordWrap, const float wrapWidth) {
	bool countStillHolds = (state != fui_null) && state->textTotalIsCounted
		&& (state->textBuffer == (const void *)buffer)
		&& (state->textBufferLength == length)
		&& (state->textWordWrap == wordWrap)
		&& (state->textWrapWidth == wrapWidth);
	if(countStillHolds) {
		return(state->textTotalLines);
	}

	uint32_t totalLineCount = 0;
	fuiTextLine *countOnly = fui_null;
	const uint32_t noStorage = 0;
	const uint32_t fromTheFirstLine = 0;
	(void)fuiBreakTextLinesFrom(context, buffer, (size_t)length, pixelHeight, wordWrap, wrapWidth, fromTheFirstLine, countOnly, noStorage, &totalLineCount);

	if(state != fui_null) {
		state->textTotalLines = (int32_t)totalLineCount;
		state->textBuffer = (const void *)buffer;
		state->textBufferLength = length;
		state->textWordWrap = wordWrap;
		state->textWrapWidth = wrapWidth;
		state->textTotalIsCounted = true;
		// A different document, or the same one laid out differently, puts every remembered offset in it wrong.
		state->textWindowIsKnown = false;
	}
	return((int32_t)totalLineCount);
}

//! Which visual line a byte offset falls on. Used only when the caret is somewhere the field is not
//! showing - after a click into a fresh field, which anchors it at the very end of the document
fui_inline int32_t fui__TextLineOfOffset(const fuiContext *context, const char *buffer, const int32_t length, const float pixelHeight, const bool wordWrap, const float wrapWidth, const int32_t offset) {
	fui__TextLineSink sink;
	fui__ClearMemory(&sink, sizeof(sink));
	sink.textBase = buffer;
	sink.offsetToFind = (size_t)offset;
	fui__BreakTextLinesInto(context, buffer, (size_t)length, pixelHeight, wordWrap, wrapWidth, &sink);
	if(!sink.offsetWasFound) {
		return((sink.totalCount > 0u) ? (int32_t)(sink.totalCount - 1u) : 0);
	}
	return((int32_t)sink.offsetLine);
}

//! Forgets that count, for an edit the field itself has just made to the buffer
fui_inline void fui__TextForgetLineCount(fuiWidgetState *state) {
	if(state != fui_null) {
		state->textTotalIsCounted = false;
		state->textWindowIsKnown = false;
	}
}

/*
	Where a line starts, seeking from a line whose start is already known.

	Only right for text that is NOT wrapped, where a line is exactly what the newlines say and so can be
	stepped over in either direction without laying anything out. That is what makes scrolling a large
	document cost the rows moved rather than the rows passed.
*/
fui_inline int32_t fui__TextOffsetOfLineUnwrapped(const char *buffer, const int32_t length, const int32_t fromOffset, const int32_t fromLine, const int32_t wantedLine) {
	int32_t offset = fuiClampI(fromOffset, 0, length);
	int32_t line = fromLine;

	while(line < wantedLine && offset < length) {
		const char *newline = (const char *)FUI_MEMCHR(&buffer[offset], '\n', (size_t)(length - offset));
		if(newline == fui_null) {
			offset = length;
			break;
		}
		offset = (int32_t)(newline - buffer) + 1;
		line += 1;
	}

	while(line > wantedLine && offset > 0) {
		// The byte before a line start is the newline that ended the one before it, so the search for the
		// start of THAT one begins one further back again.
		int32_t scan = offset - 2;
		while(scan >= 0 && buffer[scan] != '\n') {
			scan -= 1;
		}
		offset = scan + 1;
		line -= 1;
	}
	return(offset);
}

/*
	Lays out the rows around a given first visible line and says where the first stored one is drawn.

	Reaching line ten thousand means getting past the nine thousand nine hundred and ninety nine before
	it, and doing that from the top of the document every frame is what makes a big one unscrollable. So
	the window remembers where it began, and the next one seeks from THERE:

	  - unwrapped, either way: the lines are what the newlines say, so it is a scan of the rows moved
	  - wrapped, forwards:     a wrapped line starts fresh, so laying out from the remembered start is
	                           the same answer as laying out from the top would have been
	  - wrapped, backwards:    there is nothing to seek from, and the document is laid out again

	That last one is the honest cost of word wrap: where a line begins depends on the width it is being
	wrapped to, and nothing but the layout knows it.
*/
fui_inline uint32_t fui__TextLayOutWindow(const fuiContext *context, fuiWidgetState *state, const char *buffer, const int32_t length, const float pixelHeight, const bool wordWrap, const float wrapWidth, const float boxTopY, const float rowStep, fuiTextLine *outLines, fui__TextWindow *window) {
	int32_t storedFirstLine = window->firstVisibleLine - FUI__TEXT_OVERSCAN_LINES;
	if(storedFirstLine < 0) {
		storedFirstLine = 0;
	}

	int32_t startOffset = 0;
	int32_t startLine = 0;
	if(state != fui_null && state->textWindowIsKnown) {
		bool canSeekFromIt = !wordWrap || (storedFirstLine >= state->textWindowLine);
		if(canSeekFromIt) {
			startOffset = state->textWindowOffset;
			startLine = state->textWindowLine;
		}
	}
	if(!wordWrap) {
		startOffset = fui__TextOffsetOfLineUnwrapped(buffer, length, startOffset, startLine, storedFirstLine);
		startLine = storedFirstLine;
	}

	uint32_t linesToSkip = (uint32_t)(storedFirstLine - startLine);
	uint32_t *doNotCountThemAgain = fui_null;
	uint32_t storedCount = fuiBreakTextLinesFrom(context, &buffer[startOffset], (size_t)(length - startOffset), pixelHeight, wordWrap, wrapWidth, linesToSkip, outLines, (uint32_t)FUI_MAX_TEXT_LINES, doNotCountThemAgain);

	if(state != fui_null && storedCount > 0) {
		state->textWindowOffset = (int32_t)(outLines[0].start - buffer);
		state->textWindowLine = storedFirstLine;
		state->textWindowIsKnown = true;
	}

	window->storedFirstLine = storedFirstLine;
	window->firstRowTopY = boxTopY - (float)(window->firstVisibleLine - storedFirstLine) * rowStep;
	return(storedCount);
}

//! Maps the cursor to a byte offset in a multi-line field: the row by y, then the boundary by x inside it
fui_inline int32_t fui__CaretFromCursorMultiline(const fuiContext *context, const fuiRect rect, const char *buffer, const fuiTextLine *lines, const uint32_t lineCount, const float firstRowTopY) {
	if(lineCount == 0) {
		return(0);
	}
	float lineStep = fuiGetLineHeight(context, context->theme.fontHeight);
	if(lineStep <= 0.0f) {
		return(0);
	}
	float rowsDown = (context->mousePosition.y - firstRowTopY) / lineStep;
	int32_t rowIndex = (int32_t)rowsDown;
	if(rowsDown < 0.0f) {
		rowIndex = 0;
	}
	if(rowIndex >= (int32_t)lineCount) {
		rowIndex = (int32_t)lineCount - 1;
	}
	const fuiTextLine *row = &lines[rowIndex];
	float rowLeftX = rect.x + context->theme.widgetPaddingX;
	int32_t column = fui__ColumnFromCursorX(context, rowLeftX, row->start, (int32_t)row->length);
	int32_t rowStart = (int32_t)(row->start - buffer);
	return(rowStart + column);
}

//! Highlights one row's share of the selection, given the byte range that row covers
fui_inline void fui__DrawSelectionRow(fuiContext *context, const float rowLeftX, const float rowTopY, const char *rowText, const int32_t rowLength, const int32_t rowStart, const int32_t selectionStart, const int32_t selectionEnd) {
	int32_t rowEnd = rowStart + rowLength;
	int32_t from = (selectionStart > rowStart) ? selectionStart : rowStart;
	int32_t to = (selectionEnd < rowEnd) ? selectionEnd : rowEnd;
	if(from >= to) {
		return;
	}
	const fuiTheme *theme = &context->theme;
	float startX = fui__TextPrefixWidth(context, rowText, from - rowStart, theme->fontHeight);
	float endX = fui__TextPrefixWidth(context, rowText, to - rowStart, theme->fontHeight);
	fuiRect highlight = fuiRectMake(rowLeftX + startX, rowTopY, endX - startX, theme->fontHeight);
	fuiDrawRect(context, highlight, theme->textSelectionColor);
}

fui_api bool fuiTextInputEx(fuiContext *context, const fuiRect rect, const char *id, char *buffer, const int32_t capacity, const bool multiline, const bool wordWrap) {
	FUI_ASSERT(context != fui_null && id != fui_null && buffer != fui_null);
	if(context == fui_null || id == fui_null || buffer == fui_null || capacity <= 1) {
		return(false);
	}

	const fuiTheme *theme = &context->theme;
	fuiId fieldId = fuiGetId(context, id);
	// Only a multiline field remembers anything: a single line one has no scroll and nothing to count.
	fuiWidgetState *state = multiline ? fui__WidgetStateGet(context, fieldId) : fui_null;
	// Where the caret stood before this build touched anything. What follows it around is gated on it
	// having MOVED - a view dragged away from the caret with the wheel has to stay where it was put, and
	// snapping it back on the next frame would make the scrollbar unusable.
	int32_t caretBeforeThisBuild = context->caretPosition;
	bool caretWasAlreadyThisFields = (context->caretOwner == fieldId);
	int32_t length = (int32_t)fui__StringLength(buffer);
	// A multiline field keeps a gutter for its scrollbar, and wraps inside what is left rather than under it.
	float gutterWidth = multiline ? fuiScrollGutterWidth() : 0.0f;
	float contentWidth = rect.w - theme->widgetPaddingX * 2.0f - gutterWidth;
	float rowStep = fuiGetLineHeight(context, theme->fontHeight);

	/*
		The rows.

		A multiline field lays out a WINDOW of its document rather than the front of it. That is the whole
		difference between a field that can hold a file and one that shows its first two hundred and fifty
		six lines and hides the rest: what is laid out is the rows the box can show, plus one above them so
		the up arrow has somewhere to step, and the scroll says which line that window starts at.

		Everything below indexes into the WINDOW. Nothing has to know about the document, because the caret
		is kept inside the window - see the scroll-to-caret further down.
	*/
	fuiTextLine lines[FUI_MAX_TEXT_LINES];
	uint32_t lineCount = 0;
	fui__TextWindow window;
	fui__ClearMemory(&window, sizeof(window));
	window.firstRowTopY = rect.y + FUI__MULTILINE_TOP_PADDING;

	fuiInteraction interaction = fuiInteract(context, fieldId, rect);
	fui__RegisterFocusable(context, fieldId);
	bool isFocused = (context->focused == fieldId);

	// (Re-)anchor whenever the focus lands here from somewhere else, to the end of the text by default.
	// A click overrides that a few lines below with the character that was clicked.
	if(isFocused && context->caretOwner != fieldId) {
		context->caretOwner = fieldId;
		context->caretPosition = length;
		context->selectionAnchor = length;
		context->caretBlinkTime = 0.0f;
	}
	bool ownsCaret = (context->caretOwner == fieldId);

	// The buffer may have shrunk since the last frame, and only the field that OWNS the caret may pull it
	// back into range: the caret is shared by every text field, so a field clamping it against ITS length
	// would drag the focused field's caret back to the shortest sibling's end. That is invisible in a
	// dialog with one field and wrong the moment it has two.
	if(ownsCaret) {
		context->caretPosition = fui__SnapToCodepointStart(buffer, length, context->caretPosition);
		context->selectionAnchor = fui__SnapToCodepointStart(buffer, length, context->selectionAnchor);
	}

	// The box is drawn HERE rather than with the text, because the scrollbar goes in front of it and the
	// rows have to be laid out from a scroll that the bar has already had its say in.
	fuiDrawRect(context, rect, theme->widgetTrackColor);
	fuiColor borderColor = isFocused ? theme->accentColor : theme->panelBorderColor;
	fuiDrawRectOutline(context, rect, borderColor, theme->widgetBorderThickness);

	float rowsBoxTopY = rect.y + FUI__MULTILINE_TOP_PADDING;
	if(multiline) {
		window.totalLineCount = fui__TextCountLines(context, state, buffer, length, theme->fontHeight, wordWrap, contentWidth);
		float rowsBoxHeight = fuiMaxF(rect.h - FUI__MULTILINE_TOP_PADDING * 2.0f, 0.0f);
		int32_t rowsThatFit = (rowStep > 0.0f) ? (int32_t)(rowsBoxHeight / rowStep) : 1;
		window.visibleLineCount = fuiMaxI(rowsThatFit, 1);

		// The wheel first and the bar second, so the bar draws where the wheel has just put it rather than
		// one frame behind. Same order a list box resolves its own two.
		float scroll = (state != fui_null) ? ((float)state->textFirstLine * rowStep) : 0.0f;
		if(context->mouseWheelDelta != 0.0f && fui__CursorIsOver(context, rect)) {
			scroll -= context->mouseWheelDelta * rowStep * FUI__SCROLL_WHEEL_ROWS;
		}
		float contentLength = (float)window.totalLineCount * rowStep;
		fuiRect scrollTrack = fuiRectMake(rect.x + rect.w - gutterWidth, rect.y, gutterWidth, rect.h);
		fuiPushId(context, id);
		scroll = fuiScrollbarVertical(context, scrollTrack, "__textScrollbar", scroll, rect.h, contentLength);
		fuiPopId(context);

		int32_t lastPossibleFirstLine = fuiMaxI(window.totalLineCount - window.visibleLineCount, 0);
		const float roundToTheNearestLine = 0.5f;
		int32_t askedForFirstLine = (rowStep > 0.0f) ? (int32_t)((scroll / rowStep) + roundToTheNearestLine) : 0;
		window.firstVisibleLine = fuiClampI(askedForFirstLine, 0, lastPossibleFirstLine);
		if(state != fui_null) {
			state->textFirstLine = window.firstVisibleLine;
		}
		lineCount = fui__TextLayOutWindow(context, state, buffer, length, theme->fontHeight, wordWrap, contentWidth, rowsBoxTopY, rowStep, lines, &window);
	}

	if(isFocused && ownsCaret) {
		bool extendFromClick = fuiIsShiftDown(context);
		if(interaction.wasPressed) {
			int32_t clickedAt = multiline
				? fui__CaretFromCursorMultiline(context, rect, buffer, lines, lineCount, window.firstRowTopY)
				: fui__CaretFromCursorSingleLine(context, rect, buffer, length);
			fui__SetCaret(context, clickedAt, extendFromClick);
		} else if(interaction.isHeld) {
			// Holding and moving drags a selection out, which is the same gesture as a click plus shift.
			int32_t draggedTo = multiline
				? fui__CaretFromCursorMultiline(context, rect, buffer, lines, lineCount, window.firstRowTopY)
				: fui__CaretFromCursorSingleLine(context, rect, buffer, length);
			const bool extendTheSelection = true;
			fui__SetCaret(context, draggedTo, extendTheSelection);
		}
	}

	bool didChange = false;
	if(isFocused && ownsCaret) {
		// Backspace erases in front of the caret and delete erases at it, and both take the whole selection
		// instead when there is one. A line break erases like any other character, merging the two rows.
		if(fuiKeyRepeat(context, FUI_KEY_BACKSPACE)) {
			if(fui__DeleteSelection(context, buffer, &length)) {
				didChange = true;
			} else if(context->caretPosition > 0) {
				int32_t eraseFrom = fui__PreviousCodepointStart(buffer, context->caretPosition);
				fui__TextEraseRange(buffer, &length, eraseFrom, context->caretPosition);
				const bool collapseTheSelection = false;
				fui__SetCaret(context, eraseFrom, collapseTheSelection);
				didChange = true;
			}
		}
		if(fuiKeyRepeat(context, FUI_KEY_DELETE)) {
			if(fui__DeleteSelection(context, buffer, &length)) {
				didChange = true;
			} else if(context->caretPosition < length) {
				int32_t eraseTo = fui__NextCodepointStart(buffer, length, context->caretPosition);
				fui__TextEraseRange(buffer, &length, context->caretPosition, eraseTo);
				didChange = true;
			}
		}

		// Re-break before the movement keys read the rows, or home in the same frame as a backspace would
		// answer for a layout that no longer exists. The document is a different length now, so what was
		// counted about it is gone too.
		if(didChange && multiline) {
			fui__TextForgetLineCount(state);
			window.totalLineCount = fui__TextCountLines(context, state, buffer, length, theme->fontHeight, wordWrap, contentWidth);
			lineCount = fui__TextLayOutWindow(context, state, buffer, length, theme->fontHeight, wordWrap, contentWidth, rowsBoxTopY, rowStep, lines, &window);
		}

		bool extendSelection = fuiIsShiftDown(context);
		if(fuiKeyRepeat(context, FUI_KEY_LEFT)) {
			if(fui__HasSelection(context) && !extendSelection) {
				// A plain arrow key over a selection collapses it onto its edge rather than moving on from
				// the caret, which is what every text field does and what makes the selection feel like one thing.
				fui__SetCaret(context, fui__SelectionStart(context), false);
			} else if(context->caretPosition > 0) {
				fui__SetCaret(context, fui__PreviousCodepointStart(buffer, context->caretPosition), extendSelection);
			}
		}
		if(fuiKeyRepeat(context, FUI_KEY_RIGHT)) {
			if(fui__HasSelection(context) && !extendSelection) {
				fui__SetCaret(context, fui__SelectionEnd(context), false);
			} else if(context->caretPosition < length) {
				fui__SetCaret(context, fui__NextCodepointStart(buffer, length, context->caretPosition), extendSelection);
			}
		}

		bool wantsHome = fuiKeyRepeat(context, FUI_KEY_HOME);
		bool wantsEnd = fuiKeyRepeat(context, FUI_KEY_END);
		if(multiline && lineCount > 0) {
			// Home and end act on the CURRENT row; up and down keep the column and step to the next one.
			if(wantsHome || wantsEnd) {
				uint32_t rowIndex = fui__CaretRowIndex(lines, lineCount, buffer, context->caretPosition, fui_null);
				int32_t rowStart = (int32_t)(lines[rowIndex].start - buffer);
				int32_t target = wantsHome ? rowStart : (rowStart + (int32_t)lines[rowIndex].length);
				fui__SetCaret(context, target, extendSelection);
			}
			bool wantsUp = fuiKeyRepeat(context, FUI_KEY_UP);
			bool wantsDown = fuiKeyRepeat(context, FUI_KEY_DOWN);
			if(wantsUp || wantsDown) {
				int32_t column = 0;
				uint32_t rowIndex = fui__CaretRowIndex(lines, lineCount, buffer, context->caretPosition, &column);
				int32_t targetRow = wantsUp ? ((int32_t)rowIndex - 1) : ((int32_t)rowIndex + 1);
				if(targetRow >= 0 && targetRow < (int32_t)lineCount) {
					int32_t targetStart = (int32_t)(lines[targetRow].start - buffer);
					int32_t targetLength = (int32_t)lines[targetRow].length;
					int32_t targetColumn = (column < targetLength) ? column : targetLength;
					int32_t target = fui__SnapToCodepointStart(buffer, length, targetStart + targetColumn);
					fui__SetCaret(context, target, extendSelection);
				}
			}
		} else {
			if(wantsHome) {
				fui__SetCaret(context, 0, extendSelection);
			}
			if(wantsEnd) {
				fui__SetCaret(context, length, extendSelection);
			}
		}

		if(fuiIsControlDown(context)) {
			if(fuiKeyWentDown(context, FUI_KEY_A)) {
				context->selectionAnchor = 0;
				const bool extendToTheEnd = true;
				fui__SetCaret(context, length, extendToTheEnd);
			}
			bool wantsCopy = fuiKeyWentDown(context, FUI_KEY_C);
			bool wantsCut = fuiKeyWentDown(context, FUI_KEY_X);
			if(wantsCopy || wantsCut) {
				// With nothing selected there is nothing to copy but the whole field, which is what the
				// keystroke almost always means there.
				bool hasSelection = fui__HasSelection(context);
				int32_t copyFrom = hasSelection ? fui__SelectionStart(context) : 0;
				int32_t copyTo = hasSelection ? fui__SelectionEnd(context) : length;
				int32_t copyLength = copyTo - copyFrom;
				if(copyLength > (int32_t)FUI_MAX_CLIPBOARD_TEXT - 1) {
					copyLength = (int32_t)FUI_MAX_CLIPBOARD_TEXT - 1;
				}
				// Truncating in the middle of a multi-byte character would put a broken sequence on the
				// clipboard, so the cut is pulled back onto a character boundary.
				copyLength = fui__SnapToCodepointStart(&buffer[copyFrom], copyTo - copyFrom, copyLength);
				if(copyLength > 0) {
					char clipboardText[FUI_MAX_CLIPBOARD_TEXT];
					fui__CopyMemory(clipboardText, &buffer[copyFrom], (size_t)copyLength);
					clipboardText[copyLength] = '\0';
					(void)fuiSetClipboardText(context, clipboardText);
				}
				if(wantsCut && hasSelection && fui__DeleteSelection(context, buffer, &length)) {
					didChange = true;
				}
			}
			if(fuiKeyWentDown(context, FUI_KEY_V)) {
				char clipboardText[FUI_MAX_CLIPBOARD_TEXT];
				if(fuiGetClipboardText(context, clipboardText, (uint32_t)FUI_MAX_CLIPBOARD_TEXT)) {
					if(fui__InsertText(context, buffer, capacity, &length, clipboardText, multiline)) {
						didChange = true;
					}
				}
			}
		}

		// A platform sends the control codes down the typed-character path as well, and a chord like
		// ctrl and v is a keystroke rather than a character, so neither may reach the buffer. Alt is the
		// exception that has to be let through: ctrl with alt is how a keyboard types AltGr characters.
		bool isAChord = fuiIsControlDown(context) && !fuiIsAltDown(context);
		if(!isAChord) {
			for(int32_t typedIndex = 0; typedIndex < context->textInputLength; ++typedIndex) {
				uint32_t typed = context->textInput[typedIndex];
				bool isControlCode = (typed < 0x20u) || (typed == 0x7Fu);
				if(isControlCode) {
					continue;
				}
				if(fui__InsertCodepoint(context, buffer, capacity, &length, typed)) {
					didChange = true;
				}
			}
		}

		if(multiline) {
			// A plain enter inserts a line break and the edge is SWALLOWED, so a modal hosting the field
			// does not also commit on it. Ctrl and enter inserts nothing and leaves the edge for the modal.
			if(fuiKeyWentDown(context, FUI_KEY_RETURN) && !fuiIsControlDown(context)) {
				if(fui__InsertCodepoint(context, buffer, capacity, &length, (uint32_t)'\n')) {
					didChange = true;
				}
				context->keys[FUI_KEY_RETURN].halfTransitionCount = 0;
			}
			// Escape is left to the host, which is usually a modal that cancels on it.
		} else if(fuiKeyWentDown(context, FUI_KEY_RETURN) || fuiKeyWentDown(context, FUI_KEY_ESCAPE)) {
			context->focused = FUI_ID_NONE;
			isFocused = false;
		}
	}

	if(interaction.isHovered) {
		fuiSetCursor(context, FUI_CURSOR_TEXT);
	}

	/*
		Scroll to the caret, and lay the window out one last time.

		Everything above indexes into the WINDOW, and that only stays true while the caret is inside it.
		A key that moved the caret past the top or the bottom row is what moves the window, exactly as it
		does in every editor - and doing it here, after every key has had its turn, means it happens once
		however many of them fired.
	*/
	if(multiline) {
		// Typing, a click, an arrow key or the caret landing here for the first time all move it. The wheel
		// and the scrollbar do not, and are what this leaves alone.
		bool caretMoved = (context->caretPosition != caretBeforeThisBuild) || !caretWasAlreadyThisFields || didChange;
		bool windowMoved = false;
		if(isFocused && ownsCaret && caretMoved && lineCount > 0) {
			// Which line the caret is on. Reading it off the window is a handful of comparisons and is
			// right almost every time; only a caret the window does not reach - the end of a document a
			// field was just focused on - is worth walking the text for.
			int32_t windowFirstByte = (int32_t)(lines[0].start - buffer);
			int32_t windowLastByte = (int32_t)(lines[lineCount - 1u].start - buffer) + (int32_t)lines[lineCount - 1u].length;
			bool caretIsInsideTheWindow = (context->caretPosition >= windowFirstByte) && (context->caretPosition <= windowLastByte);
			int32_t caretLine;
			if(caretIsInsideTheWindow) {
				uint32_t caretRow = fui__CaretRowIndex(lines, lineCount, buffer, context->caretPosition, fui_null);
				caretLine = window.storedFirstLine + (int32_t)caretRow;
			} else {
				caretLine = fui__TextLineOfOffset(context, buffer, length, theme->fontHeight, wordWrap, contentWidth, context->caretPosition);
			}
			int32_t lastVisibleLine = window.firstVisibleLine + window.visibleLineCount - 1;
			int32_t movedToLine = window.firstVisibleLine;
			if(caretLine < window.firstVisibleLine) {
				movedToLine = caretLine;
			} else if(caretLine > lastVisibleLine) {
				movedToLine = caretLine - window.visibleLineCount + 1;
			}
			int32_t lastPossibleFirstLine = fuiMaxI(window.totalLineCount - window.visibleLineCount, 0);
			movedToLine = fuiClampI(movedToLine, 0, lastPossibleFirstLine);
			if(movedToLine != window.firstVisibleLine) {
				window.firstVisibleLine = movedToLine;
				if(state != fui_null) {
					state->textFirstLine = movedToLine;
				}
				windowMoved = true;
			}
		}
		if(windowMoved) {
			lineCount = fui__TextLayOutWindow(context, state, buffer, length, theme->fontHeight, wordWrap, contentWidth, rowsBoxTopY, rowStep, lines, &window);
		}
	}

	float textLeftX = rect.x + theme->widgetPaddingX;
	float singleRowTopY = rect.y + (rect.h - theme->fontHeight) * 0.5f;
	float firstRowTopY = window.firstRowTopY;

	fuiPushClip(context, rect);
	bool showsCaretAndSelection = isFocused && ownsCaret;
	if(showsCaretAndSelection && fui__HasSelection(context)) {
		int32_t selectionStart = fui__SelectionStart(context);
		int32_t selectionEnd = fui__SelectionEnd(context);
		if(multiline) {
			for(uint32_t rowIndex = 0; rowIndex < lineCount; ++rowIndex) {
				int32_t rowStart = (int32_t)(lines[rowIndex].start - buffer);
				float rowTopY = firstRowTopY + (float)rowIndex * rowStep;
				fui__DrawSelectionRow(context, textLeftX, rowTopY, lines[rowIndex].start, (int32_t)lines[rowIndex].length, rowStart, selectionStart, selectionEnd);
			}
		} else {
			const int32_t theWholeBufferIsOneRow = 0;
			fui__DrawSelectionRow(context, textLeftX, singleRowTopY, buffer, length, theWholeBufferIsOneRow, selectionStart, selectionEnd);
		}
	}

	if(multiline) {
		// The rows that are already laid out, rather than the document broken a third time. A row above or
		// below the box is dropped by the clip, which fuiDrawText answers from two numbers.
		for(uint32_t rowIndex = 0; rowIndex < lineCount; ++rowIndex) {
			if(lines[rowIndex].length == 0) {
				continue;
			}
			fuiVec2 rowPosition = fuiV2(textLeftX, firstRowTopY + (float)rowIndex * rowStep);
			fuiDrawText(context, lines[rowIndex].start, lines[rowIndex].length, rowPosition, theme->fontHeight, theme->textColor);
		}
	} else {
		fui__DrawTextInRect(context, rect, buffer, theme->textColor);
	}

	if(showsCaretAndSelection) {
		float blinkPeriod = (theme->caretBlinkHz > 0.0f) ? (1.0f / theme->caretBlinkHz) : 0.5f;
		bool caretIsVisible = FUI_FMODF(context->caretBlinkTime, blinkPeriod * 2.0f) < blinkPeriod;
		if(caretIsVisible) {
			float caretX = textLeftX + FUI__CARET_X_OFFSET;
			float caretY = singleRowTopY;
			if(multiline) {
				int32_t column = 0;
				uint32_t rowIndex = fui__CaretRowIndex(lines, lineCount, buffer, context->caretPosition, &column);
				caretX += fui__TextPrefixWidth(context, lines[rowIndex].start, column, theme->fontHeight);
				caretY = firstRowTopY + (float)rowIndex * rowStep;
			} else {
				caretX += fui__TextPrefixWidth(context, buffer, context->caretPosition, theme->fontHeight);
			}
			fuiRect caret = fuiRectMake(caretX, caretY, FUI__CARET_WIDTH, theme->fontHeight);
			fuiDrawRect(context, caret, theme->accentColor);
		}
	}
	fuiPopClip(context);

	return(didChange);
}

fui_api bool fuiTextInput(fuiContext *context, const fuiRect rect, const char *id, char *buffer, const int32_t capacity) {
	const bool singleLine = false;
	const bool noWordWrap = false;
	bool result = fuiTextInputEx(context, rect, id, buffer, capacity, singleLine, noWordWrap);
	return(result);
}

// ----------------------------------------------------------------------------
// > Color picker
// ----------------------------------------------------------------------------

//! Width of the vertical hue strip beside the saturation and value square
#define FUI__PICKER_HUE_STRIP_WIDTH 22.0f
//! Gap between the square, the hue strip and the numeric channel column
#define FUI__PICKER_COLUMN_GAP 8.0f
//! Height of the alpha band under the square
#define FUI__PICKER_ALPHA_HEIGHT 20.0f
//! Width of the column the numeric channel sliders are stacked in
#define FUI__PICKER_CHANNEL_COLUMN_WIDTH 150.0f
//! Height of one numeric channel slider
#define FUI__PICKER_CHANNEL_ROW_HEIGHT 24.0f
//! How many numeric channel sliders there are, which is red, green and blue
#define FUI__PICKER_CHANNEL_COUNT 3
//! How many flat strips a gradient is approximated by, which is what keeps this on the rectangle path
#define FUI__PICKER_RAMP_STEPS 64
//! Side of one square of the checkerboard the alpha band is drawn over
#define FUI__PICKER_CHECKER_CELL 6.0f
//! Half the side of the inner square of the marker ring
#define FUI__PICKER_MARKER_RADIUS 5.0f
//! Stroke width of each of the marker ring's two outlines
#define FUI__PICKER_MARKER_THICKNESS 1.5f
//! Bytes reserved for one channel slider's caption
#define FUI__PICKER_CHANNEL_TEXT_CAPACITY 32

// These are NOT theme colors, and that is the point of them: a checkerboard has to read as "nothing is
// here" against any color the user picks, and a marker ring has to stay visible on top of one. Both are
// fixed by what they have to do rather than by taste.
static const fuiColor fui__PickerCheckerLight = FUI_COLOR(0.62f, 0.62f, 0.62f, 1.0f);
static const fuiColor fui__PickerCheckerDark = FUI_COLOR(0.38f, 0.38f, 0.38f, 1.0f);
static const fuiColor fui__PickerMarkerLight = FUI_COLOR(1.0f, 1.0f, 1.0f, 1.0f);
static const fuiColor fui__PickerMarkerDark = FUI_COLOR(0.0f, 0.0f, 0.0f, 1.0f);

static const char *fui__PickerChannelLabels[FUI__PICKER_CHANNEL_COUNT] = { "R", "G", "B" };
static const char *fui__PickerChannelIds[FUI__PICKER_CHANNEL_COUNT] = { "__red", "__green", "__blue" };

fui_api fuiVec3 fuiHsvToRgb(const fuiVec3 hsv) {
	float wrappedHue = hsv.x - (float)(int32_t)hsv.x;
	if(wrappedHue < 0.0f) {
		wrappedHue += 1.0f;
	}
	float saturation = fuiClampF(hsv.y, 0.0f, 1.0f);
	float value = fuiClampF(hsv.z, 0.0f, 1.0f);
	float sector = wrappedHue * 6.0f;
	int32_t sectorIndex = (int32_t)sector;
	float sectorFraction = sector - (float)sectorIndex;
	float falling = value * (1.0f - saturation);
	float descending = value * (1.0f - saturation * sectorFraction);
	float ascending = value * (1.0f - saturation * (1.0f - sectorFraction));
	switch(sectorIndex % 6) {
		case 0: return(fuiV3(value, ascending, falling));
		case 1: return(fuiV3(descending, value, falling));
		case 2: return(fuiV3(falling, value, ascending));
		case 3: return(fuiV3(falling, descending, value));
		case 4: return(fuiV3(ascending, falling, value));
		default: return(fuiV3(value, falling, descending));
	}
}

fui_api fuiVec3 fuiRgbToHsv(const fuiVec3 rgb) {
	float red = fuiClampF(rgb.x, 0.0f, 1.0f);
	float green = fuiClampF(rgb.y, 0.0f, 1.0f);
	float blue = fuiClampF(rgb.z, 0.0f, 1.0f);
	float largest = fuiMaxF(red, fuiMaxF(green, blue));
	float smallest = fuiMinF(red, fuiMinF(green, blue));
	float spread = largest - smallest;
	float hue = 0.0f;
	if(spread > 0.0f) {
		if(largest == red) {
			hue = (green - blue) / spread;
		} else if(largest == green) {
			hue = 2.0f + (blue - red) / spread;
		} else {
			hue = 4.0f + (red - green) / spread;
		}
		hue /= 6.0f;
		if(hue < 0.0f) {
			hue += 1.0f;
		}
	}
	float saturation = (largest > 0.0f) ? (spread / largest) : 0.0f;
	return(fuiV3(hue, saturation, largest));
}

//! One axis of a two-color ramp. Both the color AND the alpha are interpolated, so a fade to transparent works
fui_inline void fui__DrawRampX(fuiContext *context, const fuiRect rect, const fuiColor leftColor, const fuiColor rightColor) {
	if(context->pass == FUI_PASS_INTERACT) {
		return; // an interact pass emits nothing, so skip the whole run rather than dropping 64 rectangles
	}
	for(int32_t stepIndex = 0; stepIndex < FUI__PICKER_RAMP_STEPS; ++stepIndex) {
		float t = ((float)stepIndex + 0.5f) / (float)FUI__PICKER_RAMP_STEPS;
		fuiColor color = fuiColorRGBA(fuiLerpF(leftColor.r, rightColor.r, t), fuiLerpF(leftColor.g, rightColor.g, t), fuiLerpF(leftColor.b, rightColor.b, t), fuiLerpF(leftColor.a, rightColor.a, t));
		// Each strip ends exactly where the next one starts, computed from the two edges rather than from a
		// width. Overlapping them to hide a rounding gap would draw a TRANSLUCENT ramp twice along every
		// seam, and sixty-four double blended seams read as banding.
		float stripStart = rect.x + rect.w * ((float)stepIndex / (float)FUI__PICKER_RAMP_STEPS);
		float stripEnd = rect.x + rect.w * ((float)(stepIndex + 1) / (float)FUI__PICKER_RAMP_STEPS);
		fuiRect strip = fuiRectMake(stripStart, rect.y, stripEnd - stripStart, rect.h);
		fuiDrawRect(context, strip, color);
	}
}

//! One axis of a two-color ramp, running downwards
fui_inline void fui__DrawRampY(fuiContext *context, const fuiRect rect, const fuiColor topColor, const fuiColor bottomColor) {
	if(context->pass == FUI_PASS_INTERACT) {
		return;
	}
	for(int32_t stepIndex = 0; stepIndex < FUI__PICKER_RAMP_STEPS; ++stepIndex) {
		float t = ((float)stepIndex + 0.5f) / (float)FUI__PICKER_RAMP_STEPS;
		fuiColor color = fuiColorRGBA(fuiLerpF(topColor.r, bottomColor.r, t), fuiLerpF(topColor.g, bottomColor.g, t), fuiLerpF(topColor.b, bottomColor.b, t), fuiLerpF(topColor.a, bottomColor.a, t));
		float stripStart = rect.y + rect.h * ((float)stepIndex / (float)FUI__PICKER_RAMP_STEPS);
		float stripEnd = rect.y + rect.h * ((float)(stepIndex + 1) / (float)FUI__PICKER_RAMP_STEPS);
		fuiRect strip = fuiRectMake(rect.x, stripStart, rect.w, stripEnd - stripStart);
		fuiDrawRect(context, strip, color);
	}
}

//! The whole hue wheel, red at the bottom. Not a two-color ramp: it walks all six sectors
fui_inline void fui__DrawHueStrip(fuiContext *context, const fuiRect rect) {
	if(context->pass == FUI_PASS_INTERACT) {
		return;
	}
	for(int32_t stepIndex = 0; stepIndex < FUI__PICKER_RAMP_STEPS; ++stepIndex) {
		float hue = 1.0f - ((float)stepIndex + 0.5f) / (float)FUI__PICKER_RAMP_STEPS;
		fuiVec3 rgb = fuiHsvToRgb(fuiV3(hue, 1.0f, 1.0f));
		fuiColor color = fuiColorRGBA(rgb.x, rgb.y, rgb.z, 1.0f);
		float stripStart = rect.y + rect.h * ((float)stepIndex / (float)FUI__PICKER_RAMP_STEPS);
		float stripEnd = rect.y + rect.h * ((float)(stepIndex + 1) / (float)FUI__PICKER_RAMP_STEPS);
		fuiRect strip = fuiRectMake(rect.x, stripStart, rect.w, stripEnd - stripStart);
		fuiDrawRect(context, strip, color);
	}
}

//! The grey chequer that says "there is nothing behind this", drawn under anything translucent
fui_inline void fui__DrawCheckerboard(fuiContext *context, const fuiRect rect) {
	if(context->pass == FUI_PASS_INTERACT) {
		return;
	}
	fuiDrawRect(context, rect, fui__PickerCheckerLight);
	int32_t columnCount = (int32_t)(rect.w / FUI__PICKER_CHECKER_CELL) + 1;
	int32_t rowCount = (int32_t)(rect.h / FUI__PICKER_CHECKER_CELL) + 1;
	for(int32_t rowIndex = 0; rowIndex < rowCount; ++rowIndex) {
		for(int32_t columnIndex = (rowIndex % 2); columnIndex < columnCount; columnIndex += 2) {
			float cellX = rect.x + (float)columnIndex * FUI__PICKER_CHECKER_CELL;
			float cellY = rect.y + (float)rowIndex * FUI__PICKER_CHECKER_CELL;
			float cellWidth = fuiMinF(FUI__PICKER_CHECKER_CELL, rect.x + rect.w - cellX);
			float cellHeight = fuiMinF(FUI__PICKER_CHECKER_CELL, rect.y + rect.h - cellY);
			fuiRect cell = fuiRectMake(cellX, cellY, cellWidth, cellHeight);
			fuiDrawRect(context, cell, fui__PickerCheckerDark);
		}
	}
}

//! The ring marking a picked spot: a light square outline with a dark one just outside it, so it shows on anything
fui_inline void fui__DrawPickerMarker(fuiContext *context, const fuiVec2 center) {
	float inner = FUI__PICKER_MARKER_RADIUS;
	float outer = inner + FUI__PICKER_MARKER_THICKNESS;
	fuiRect outerRect = fuiRectMake(center.x - outer, center.y - outer, outer * 2.0f, outer * 2.0f);
	fuiRect innerRect = fuiRectMake(center.x - inner, center.y - inner, inner * 2.0f, inner * 2.0f);
	fuiDrawRectOutline(context, outerRect, fui__PickerMarkerDark, FUI__PICKER_MARKER_THICKNESS);
	fuiDrawRectOutline(context, innerRect, fui__PickerMarkerLight, FUI__PICKER_MARKER_THICKNESS);
}

//! Tracks one draggable region of the picker and folds its grab and let-go into the picker-wide lifecycle
fui_inline bool fui__PickerRegionDrag(fuiContext *context, const fuiRect rect, const char *label, bool *anyDidBegin, bool *anyDidEnd) {
	fuiId regionId = fuiGetId(context, label);
	bool wasHeldBefore = (context->active == regionId);
	fuiInteraction interaction = fuiInteract(context, regionId, rect);
	bool isHeldNow = (context->active == regionId);
	if(!wasHeldBefore && isHeldNow) {
		*anyDidBegin = true;
	}
	if(wasHeldBefore && !isHeldNow) {
		*anyDidEnd = true;
	}
	return(interaction.isHeld);
}

//! How far across a region the cursor sits, from its left edge
fui_inline float fui__PickerFractionX(const fuiContext *context, const fuiRect rect) {
	if(rect.w <= 0.0f) {
		return(0.0f);
	}
	float fraction = (context->mousePosition.x - rect.x) / rect.w;
	return(fuiClampF(fraction, 0.0f, 1.0f));
}

//! How far up a region the cursor sits, from its BOTTOM edge. Value and hue both grow upwards on screen
fui_inline float fui__PickerFractionUp(const fuiContext *context, const fuiRect rect) {
	if(rect.h <= 0.0f) {
		return(0.0f);
	}
	float fraction = 1.0f - (context->mousePosition.y - rect.y) / rect.h;
	return(fuiClampF(fraction, 0.0f, 1.0f));
}

fui_api bool fuiColorPicker(fuiContext *context, const fuiRect rect, const char *id, fuiColor *color, const bool hasAlpha, const bool enabled, bool *outDidBegin, bool *outDidEnd) {
	FUI_ASSERT(context != fui_null && id != fui_null && color != fui_null);
	if(outDidBegin != fui_null) {
		*outDidBegin = false;
	}
	if(outDidEnd != fui_null) {
		*outDidEnd = false;
	}
	if(context == fui_null || id == fui_null || color == fui_null) {
		return(false);
	}

	const fuiTheme *theme = &context->theme;
	fuiId pickerId = fuiGetId(context, id);
	fuiWidgetState *state = fui__WidgetStateGet(context, pickerId);
	bool didBegin = false;
	bool didEnd = false;

	// Split the box: the square and the hue strip share the upper left, the numeric channels take a column
	// down the right, and the alpha band takes a strip under the square when there is one.
	float alphaBandHeight = hasAlpha ? (FUI__PICKER_ALPHA_HEIGHT + theme->widgetSpacing) : 0.0f;
	float upperHeight = fuiMaxF(rect.h - alphaBandHeight, 1.0f);
	float visualWidth = fuiMaxF(rect.w - FUI__PICKER_CHANNEL_COLUMN_WIDTH - FUI__PICKER_COLUMN_GAP, 1.0f);
	float squareWidth = fuiMaxF(visualWidth - FUI__PICKER_HUE_STRIP_WIDTH - FUI__PICKER_COLUMN_GAP, 1.0f);
	fuiRect squareRect = fuiRectMake(rect.x, rect.y, squareWidth, upperHeight);
	fuiRect hueRect = fuiRectMake(rect.x + visualWidth - FUI__PICKER_HUE_STRIP_WIDTH, rect.y, FUI__PICKER_HUE_STRIP_WIDTH, upperHeight);
	fuiRect alphaRect = fuiRectMake(rect.x, rect.y + rect.h - FUI__PICKER_ALPHA_HEIGHT, visualWidth, FUI__PICKER_ALPHA_HEIGHT);
	fuiRect channelColumn = fuiRectMake(rect.x + rect.w - FUI__PICKER_CHANNEL_COLUMN_WIDTH, rect.y, FUI__PICKER_CHANNEL_COLUMN_WIDTH, upperHeight);

	// The hue the strip shows comes from the color, except where rgb cannot express one at all - a grey or
	// a black has no hue - where the remembered one carries over.
	fuiVec3 hsv = fuiRgbToHsv(fuiV3(color->r, color->g, color->b));
	bool hueIsMeaningless = (hsv.y <= 0.0f) || (hsv.z <= 0.0f);
	if(hueIsMeaningless && state != fui_null && state->hasPickerHue) {
		hsv.x = state->pickerHue;
	}
	float alpha = hasAlpha ? color->a : 1.0f;

	bool didChange = false;
	fuiPushId(context, id);
	if(enabled) {
		if(fui__PickerRegionDrag(context, squareRect, "__saturationValue", &didBegin, &didEnd)) {
			hsv.y = fui__PickerFractionX(context, squareRect);
			hsv.z = fui__PickerFractionUp(context, squareRect);
			didChange = true;
		}
		if(fui__PickerRegionDrag(context, hueRect, "__hue", &didBegin, &didEnd)) {
			hsv.x = fui__PickerFractionUp(context, hueRect);
			didChange = true;
		}
		if(hasAlpha && fui__PickerRegionDrag(context, alphaRect, "__alpha", &didBegin, &didEnd)) {
			alpha = fui__PickerFractionX(context, alphaRect);
			didChange = true;
		}
	}

	if(didChange) {
		fuiVec3 draggedRgb = fuiHsvToRgb(hsv);
		color->r = draggedRgb.x;
		color->g = draggedRgb.y;
		color->b = draggedRgb.z;
		if(hasAlpha) {
			color->a = alpha;
		}
	}

	// The channel sliders edit the color DIRECTLY rather than through hsv, which is the whole point of
	// having them: an exact triple can be dialled in without a round trip through a hue the square would
	// then have to re-derive.
	float channelValues[FUI__PICKER_CHANNEL_COUNT] = { color->r, color->g, color->b };
	bool aChannelChanged = false;
	for(int32_t channelIndex = 0; channelIndex < FUI__PICKER_CHANNEL_COUNT; ++channelIndex) {
		float rowY = channelColumn.y + (float)channelIndex * (FUI__PICKER_CHANNEL_ROW_HEIGHT + theme->widgetSpacing);
		fuiRect rowRect = fuiRectMake(channelColumn.x, rowY, channelColumn.w, FUI__PICKER_CHANNEL_ROW_HEIGHT);
		char rowText[FUI__PICKER_CHANNEL_TEXT_CAPACITY];
		(void)fui__FormatString(rowText, sizeof(rowText), "%s: %.2f", fui__PickerChannelLabels[channelIndex], (double)channelValues[channelIndex]);
		bool rowDidBegin = false;
		bool rowDidEnd = false;
		const float noStep = 0.0f; // a color channel has no natural increment to snap to
		const bool channelUpdatesLive = true; // the picker writes into the color as it is dragged
		if(fuiSliderFloatEx(context, rowRect, fui__PickerChannelIds[channelIndex], &channelValues[channelIndex], 0.0f, 1.0f, noStep, channelUpdatesLive, enabled, rowText, &rowDidBegin, &rowDidEnd)) {
			aChannelChanged = true;
		}
		didBegin = didBegin || rowDidBegin;
		didEnd = didEnd || rowDidEnd;
	}
	if(aChannelChanged) {
		color->r = channelValues[0];
		color->g = channelValues[1];
		color->b = channelValues[2];
		didChange = true;
		// Re-derive what the square and the strip show, keeping the hue where the new rgb has none:
		// dragging red down to black must not throw the hue away.
		fuiVec3 slidHsv = fuiRgbToHsv(fuiV3(color->r, color->g, color->b));
		bool slidHueIsMeaningless = (slidHsv.y <= 0.0f) || (slidHsv.z <= 0.0f);
		if(slidHueIsMeaningless) {
			slidHsv.x = hsv.x;
		}
		hsv = slidHsv;
	}
	fuiPopId(context);

	if(state != fui_null) {
		state->pickerHue = hsv.x;
		state->hasPickerHue = true;
	}

	// The square: the pure hue as a base, whitened to the left by saturation and blackened downward by value.
	fuiVec3 pureHueRgb = fuiHsvToRgb(fuiV3(hsv.x, 1.0f, 1.0f));
	fuiColor pureHueColor = fuiColorRGBA(pureHueRgb.x, pureHueRgb.y, pureHueRgb.z, 1.0f);
	fuiColor opaqueWhite = fuiColorRGBA(1.0f, 1.0f, 1.0f, 1.0f);
	fuiColor clearWhite = fuiColorRGBA(1.0f, 1.0f, 1.0f, 0.0f);
	fuiColor opaqueBlack = fuiColorRGBA(0.0f, 0.0f, 0.0f, 1.0f);
	fuiColor clearBlack = fuiColorRGBA(0.0f, 0.0f, 0.0f, 0.0f);
	fuiDrawRect(context, squareRect, pureHueColor);
	fui__DrawRampX(context, squareRect, opaqueWhite, clearWhite);
	fui__DrawRampY(context, squareRect, clearBlack, opaqueBlack);
	fuiDrawRectOutline(context, squareRect, theme->panelBorderColor, theme->widgetBorderThickness);

	fui__DrawHueStrip(context, hueRect);
	fuiDrawRectOutline(context, hueRect, theme->panelBorderColor, theme->widgetBorderThickness);

	if(hasAlpha) {
		fuiColor transparentColor = fuiColorRGBA(color->r, color->g, color->b, 0.0f);
		fuiColor opaqueColor = fuiColorRGBA(color->r, color->g, color->b, 1.0f);
		fui__DrawCheckerboard(context, alphaRect);
		fui__DrawRampX(context, alphaRect, transparentColor, opaqueColor);
		fuiDrawRectOutline(context, alphaRect, theme->panelBorderColor, theme->widgetBorderThickness);
	}

	fuiVec2 squareMarker = fuiV2(squareRect.x + hsv.y * squareRect.w, squareRect.y + (1.0f - hsv.z) * squareRect.h);
	fui__DrawPickerMarker(context, squareMarker);
	fuiVec2 hueMarker = fuiV2(hueRect.x + hueRect.w * 0.5f, hueRect.y + (1.0f - hsv.x) * hueRect.h);
	fui__DrawPickerMarker(context, hueMarker);
	if(hasAlpha) {
		fuiVec2 alphaMarker = fuiV2(alphaRect.x + alpha * alphaRect.w, alphaRect.y + alphaRect.h * 0.5f);
		fui__DrawPickerMarker(context, alphaMarker);
	}

	if(outDidBegin != fui_null) {
		*outDidBegin = didBegin;
	}
	if(outDidEnd != fui_null) {
		*outDidEnd = didEnd;
	}
	return(didChange);
}

// ----------------------------------------------------------------------------
// > Commands
// ----------------------------------------------------------------------------

fui_api const fuiCommand *fuiFindCommand(const fuiCommandTable *table, const fuiCommandId id) {
	if(table == fui_null || table->commands == fui_null) {
		return(fui_null);
	}
	for(uint32_t index = 0; index < table->count; ++index) {
		const fuiCommand *command = &table->commands[index];
		if(command->id == id) {
			return(command);
		}
	}
	return(fui_null);
}

fui_api bool fuiCommandIsEnabled(const fuiCommand *command, void *userData) {
	if(command == fui_null) {
		return(false);
	}
	if(command->isEnabled == fui_null) {
		return(true);
	}
	bool result = command->isEnabled(userData);
	return(result);
}

fui_api bool fuiCommandIsVisible(const fuiCommand *command, void *userData) {
	if(command == fui_null) {
		return(false);
	}
	if(command->isVisible == fui_null) {
		return(true);
	}
	bool result = command->isVisible(userData);
	return(result);
}

//! What a key with no name in the table is written as, so its modifiers still read right
#define FUI__UNNAMED_KEY_TEXT "?"

fui_api const char *fuiShortcutToText(const fuiShortcut shortcut, char *buffer, const size_t bufferCapacity) {
	FUI_ASSERT(buffer != fui_null);
	if(buffer == fui_null || bufferCapacity == 0) {
		return(buffer);
	}
	buffer[0] = '\0';
	if(shortcut.key == FUI_KEY_NONE) {
		return(buffer);
	}
	// Modifiers first, in the order every desktop writes them in, each followed by a plus.
	if((shortcut.modifiers & (uint32_t)FUI_MOD_CONTROL) != 0u) {
		(void)fui__AppendString(buffer, bufferCapacity, "Ctrl+");
	}
	if((shortcut.modifiers & (uint32_t)FUI_MOD_SHIFT) != 0u) {
		(void)fui__AppendString(buffer, bufferCapacity, "Shift+");
	}
	if((shortcut.modifiers & (uint32_t)FUI_MOD_ALT) != 0u) {
		(void)fui__AppendString(buffer, bufferCapacity, "Alt+");
	}
	const char *keyName = fuiKeyGetName(shortcut.key);
	if(keyName == fui_null || keyName[0] == '\0') {
		keyName = FUI__UNNAMED_KEY_TEXT;
	}
	(void)fui__AppendString(buffer, bufferCapacity, keyName);
	return(buffer);
}

//! Exactly which modifiers are held, so a shortcut matches on the whole set rather than on a subset of it
fui_inline uint32_t fui__HeldModifiers(const fuiContext *context) {
	uint32_t modifiers = (uint32_t)FUI_MOD_NONE;
	if(fuiIsControlDown(context)) {
		modifiers |= (uint32_t)FUI_MOD_CONTROL;
	}
	if(fuiIsShiftDown(context)) {
		modifiers |= (uint32_t)FUI_MOD_SHIFT;
	}
	if(fuiIsAltDown(context)) {
		modifiers |= (uint32_t)FUI_MOD_ALT;
	}
	return(modifiers);
}

fui_api fuiCommandId fuiDispatchShortcuts(fuiContext *context, const fuiCommandTable *table, void *userData) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null || table == fui_null || table->commands == fui_null) {
		return(FUI_COMMAND_NONE);
	}
	// A focused text field owns the keyboard. Typing an S into a name must not save the level.
	if(fuiWantsKeyboard(context)) {
		return(FUI_COMMAND_NONE);
	}

	uint32_t heldModifiers = fui__HeldModifiers(context);
	for(uint32_t index = 0; index < table->count; ++index) {
		const fuiCommand *command = &table->commands[index];
		fuiShortcut shortcut = command->shortcut;
		if(shortcut.key == FUI_KEY_NONE) {
			continue;
		}
		// An EXACT match, so Ctrl+S stays quiet while Ctrl+Shift+S is held and the two can mean two things.
		if(shortcut.modifiers != heldModifiers) {
			continue;
		}
		if(!fuiKeyWentDown(context, shortcut.key)) {
			continue;
		}
		if(!fuiCommandIsVisible(command, userData) || !fuiCommandIsEnabled(command, userData)) {
			continue;
		}
		if(command->invoke != fui_null) {
			command->invoke(userData);
		}
		return(command->id);
	}
	return(FUI_COMMAND_NONE);
}

//! Draws one line of text with its LEFT edge at an explicit x, centered on a row
fui_inline void fui__DrawTextVCentered(fuiContext *context, const char *text, const float leftX, const float centerY, const float pixelHeight, const fuiColor color) {
	if(context->font == fui_null || text == fui_null) {
		return;
	}
	float lineHeight = fui__LineExtent(context->font, pixelHeight);
	fuiVec2 position = fuiV2(leftX, centerY - lineHeight * 0.5f);
	fuiDrawText(context, text, 0, position, pixelHeight, color);
}

//! Draws one line of text with its RIGHT edge at an explicit x, centered on a row
fui_inline void fui__DrawTextVCenteredRight(fuiContext *context, const char *text, const float rightX, const float centerY, const float pixelHeight, const fuiColor color) {
	if(context->font == fui_null || text == fui_null) {
		return;
	}
	// The WIDTH is still measured, because right alignment is a fact about the string. The height is not.
	fuiVec2 textSize = fuiMeasureText(context, text, 0, pixelHeight);
	float lineHeight = fui__LineExtent(context->font, pixelHeight);
	fuiVec2 position = fuiV2(rightX - textSize.x, centerY - lineHeight * 0.5f);
	fuiDrawText(context, text, 0, position, pixelHeight, color);
}

fui_api bool fuiCommandButton(fuiContext *context, const fuiRect rect, const fuiCommandTable *table, const fuiCommandId id, void *userData) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return(false);
	}
	const fuiCommand *command = fuiFindCommand(table, id);
	if(!fuiCommandIsVisible(command, userData)) {
		return(false);
	}

	const fuiTheme *theme = &context->theme;
	bool enabled = fuiCommandIsEnabled(command, userData);

	// Identified by the COMMAND rather than by the label, so two buttons for two commands that happen to
	// read the same stay two buttons.
	fuiId widgetId = fui__HashBytes(fui__CurrentIdSeed(context), &id, sizeof(id));
	fuiInteraction interaction;
	fui__ClearMemory(&interaction, sizeof(interaction));
	if(enabled) {
		interaction = fuiInteract(context, widgetId, rect);
	}
	fui__DrawButton(context, rect, command->label, enabled, interaction);

	// The shortcut, muted and right aligned inside the button, so the keyboard way is discoverable.
	char shortcutText[FUI_MAX_SHORTCUT_TEXT];
	(void)fuiShortcutToText(command->shortcut, shortcutText, sizeof(shortcutText));
	if(shortcutText[0] != '\0') {
		float rightX = rect.x + rect.w - theme->widgetPaddingX;
		float centerY = rect.y + rect.h * 0.5f;
		fui__DrawTextVCenteredRight(context, shortcutText, rightX, centerY, theme->fontHeight, theme->textMutedColor);
	}

	bool didFire = false;
	if(interaction.wasClicked && command->invoke != fui_null) {
		command->invoke(userData);
		didFire = true;
	}
	return(didFire);
}

// ----------------------------------------------------------------------------
// > Menus
// ----------------------------------------------------------------------------

//! What marks a row that opens a submenu
#define FUI__MENU_SUBMENU_MARK ">"

//! How tall a separator row is, as a fraction of a normal row
#define FUI__MENU_SEPARATOR_FRACTION 0.5f

//! The menus' OWN hit test. They never ask through fui__CursorIsOver, which an open menu blocks
fui_inline bool fui__MenuCursorIsOver(const fuiContext *context, const fuiRect rect) {
	if(!context->inputIsActive) {
		return(false);
	}
	// A menu asks with the cursor rather than through fui__CursorIsOver, so the one gate that stops every
	// other widget under a dialog would leave a menu bar behind the backdrop live. A menu bar built INSIDE
	// a dialog still works, since it carries the dialog's scope like any other widget of it.
	if(fui__ModalBlocksInput(context)) {
		return(false);
	}
	bool result = fuiPointInRect(context->mousePosition, rect);
	return(result);
}

//! Everything the mouse touching a menu means: the interface owns the cursor, and this press is not a dismissal
fui_inline void fui__MenuTakeTheMouse(fuiContext *context) {
	context->mouseIsOverUi = true;
	context->menuOwnsTheMouse = true;
}

//! The menu being built right now, or null when there is none or the stack ran past its maximum
fui_inline fuiMenuFrame *fui__CurrentMenu(fuiContext *context) {
	if(context->menuStackDepth == 0 || context->menuStackDepth > (uint32_t)FUI_MAX_MENU_DEPTH) {
		return(fui_null);
	}
	fuiMenuFrame *result = &context->menuStack[context->menuStackDepth - 1u];
	return(result);
}

//! Opens one menu frame. A push past the maximum still COUNTS, so an unwind stays balanced with no frame
fui_inline fuiMenuFrame *fui__PushMenu(fuiContext *context) {
	fuiMenuFrame *frame = fui_null;
	if(context->menuStackDepth < (uint32_t)FUI_MAX_MENU_DEPTH) {
		frame = &context->menuStack[context->menuStackDepth];
		fui__ClearMemory(frame, sizeof(*frame));
	}
	context->menuStackDepth += 1u;
	return(frame);
}

fui_inline void fui__PopMenu(fuiContext *context) {
	if(context->menuStackDepth > 0) {
		context->menuStackDepth -= 1u;
	}
}

//! Pushes a clip that IGNORES whatever is clipping right now, bounded only by the window
fui_inline void fui__PushClipAbsolute(fuiContext *context, const fuiRect rect) {
	fuiRect wholeWindow = fuiRectMake(0.0f, 0.0f, (float)context->windowSize.x, (float)context->windowSize.y);
	fuiRect clipped = fuiRectIntersect(wholeWindow, rect);
	if(context->clipDepth < (uint32_t)FUI_MAX_CLIP_DEPTH) {
		context->clipStack[context->clipDepth] = clipped;
	}
	context->clipDepth += 1u;
}

//! The height of one menu row, which the popup's whole geometry is derived from
fui_inline float fui__MenuRowHeight(const fuiContext *context) {
	float result = fui__BarThickness(context, context->theme.menuItemFontHeight);
	return(result);
}

fui_api float fuiMenuBarHeight(const fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return(0.0f);
	}
	// A title in the bar and a row in the popup it drops are the same row, so the bar is exactly one of them
	// tall. Anything more is empty strip, and anything less crops the caption it was made for.
	float result = fui__MenuRowHeight(context);
	return(result);
}

//! Places an open popup at the size the previous build measured, draws its box and clips the rows to it
//! Clearance a popup keeps from the top and bottom of the window, so a cut one does not sit flush on the edge
#define FUI__MENU_SCREEN_MARGIN 4.0f

//! How tall the strip is that a cut popup marks its cut edge with. Big enough to be a target, because
//! resting on it is what scrolls the menu - which is what the arrow on one has always meant
#define FUI__MENU_SCROLL_HINT_HEIGHT 14.0f

//! How many rows a second the menu moves while the cursor rests on one of those strips
#define FUI__MENU_HOVER_SCROLL_ROWS_PER_SECOND 10.0f

//! Below this share of the window, the space under a menu's trigger is not worth opening into and the
//! popup takes the whole height instead, covering whatever it drops from
#define FUI__MENU_SPACE_BELOW_FRACTION 0.5f

//! Where the strip along one edge of a popup sits, whether or not there is anything beyond that edge
fui_inline fuiRect fui__MenuScrollStrip(const fuiRect popupRect, const bool isTheTopEdge) {
	float stripTop = isTheTopEdge ? popupRect.y : (popupRect.y + popupRect.h - FUI__MENU_SCROLL_HINT_HEIGHT);
	fuiRect result = fuiRectMake(popupRect.x, stripTop, popupRect.w, FUI__MENU_SCROLL_HINT_HEIGHT);
	return(result);
}

//! Marks one edge of a popup that has rows beyond it, with the triangle every desktop menu uses for it
static void fui__MenuDrawScrollHint(fuiContext *context, const fuiRect strip, const bool isTheTopEdge, const bool isHovered) {
	const fuiTheme *theme = &context->theme;
	fuiDrawRect(context, strip, isHovered ? theme->menuHighlightColor : theme->widgetTrackColor);

	const float arrowHalfWidth = 5.0f;
	const float arrowInset = 4.0f;
	float centerX = strip.x + strip.w * 0.5f;
	float pointY = isTheTopEdge ? (strip.y + arrowInset) : (strip.y + strip.h - arrowInset);
	float baseY = isTheTopEdge ? (strip.y + strip.h - arrowInset) : (strip.y + arrowInset);
	fuiVec2 arrow[3];
	arrow[0] = fuiV2(centerX, pointY);
	arrow[1] = fuiV2(centerX - arrowHalfWidth, baseY);
	arrow[2] = fuiV2(centerX + arrowHalfWidth, baseY);
	fuiDrawPolygon(context, arrow, 3u, theme->textColor);
}

static void fui__MenuBeginPopup(fuiContext *context, fuiMenuFrame *frame, const fuiId id, const float leftX, const float topY, const bool opensSideways) {
	const fuiTheme *theme = &context->theme;
	float width = theme->menuPopupMinWidth;
	float contentHeight = fui__MenuRowHeight(context);
	fuiWidgetState *state = fui__WidgetStateGet(context, id);
	if(state != fui_null) {
		width = fuiMaxF(width, state->menuSize.x);
		if(state->menuSize.y > 0.0f) {
			contentHeight = state->menuSize.y;
		}
	}

	/*
		A menu with more rows than the screen is tall.

		Moving it up until it fits is what keeps a normal menu whole near the bottom edge, and it is all
		that used to happen here. A menu of four hundred rows does not fit anywhere: it was placed at the
		top of the window and everything past the bottom of it was drawn where nobody could see or reach
		it. A generated menu, a list of open documents, a font picker - any of those is that menu.

		So a popup is no taller than the window, and one that had to be cut scrolls under the wheel.
	*/
	float windowWidth = (float)context->windowSize.x;
	float windowHeight = (float)context->windowSize.y;
	float wholeHeight = fuiMaxF(windowHeight - FUI__MENU_SCREEN_MARGIN * 2.0f, fui__MenuRowHeight(context));
	float spaceBelowTheTrigger = fuiMaxF(windowHeight - FUI__MENU_SCREEN_MARGIN - topY, 0.0f);

	/*
		Where the box goes, in the order a desktop menu decides it.

		It drops from its trigger when it fits below one. It is moved UP when it does not, which is what
		keeps a normal menu whole near the bottom edge. And when it is taller than the screen itself it
		opens into the space below the trigger anyway, as long as that is most of the screen - because a
		menu bar title whose popup is forced to the top of the window has its popup drawn over the title,
		and over every other title on the bar with it.
	*/
	float boxHeight = contentHeight;
	float placedY = topY;
	if(contentHeight > spaceBelowTheTrigger) {
		bool fitsOnScreenSomewhere = (contentHeight <= wholeHeight);
		if(fitsOnScreenSomewhere) {
			placedY = windowHeight - FUI__MENU_SCREEN_MARGIN - contentHeight;
		} else if(spaceBelowTheTrigger >= (wholeHeight * FUI__MENU_SPACE_BELOW_FRACTION)) {
			boxHeight = spaceBelowTheTrigger;
			placedY = topY;
		} else {
			boxHeight = wholeHeight;
			placedY = FUI__MENU_SCREEN_MARGIN;
		}
	}
	placedY = fuiClampF(placedY, FUI__MENU_SCREEN_MARGIN, fuiMaxF(windowHeight - boxHeight - FUI__MENU_SCREEN_MARGIN, FUI__MENU_SCREEN_MARGIN));
	float hiddenHeight = fuiMaxF(contentHeight - boxHeight, 0.0f);

	// Sideways is the same idea: a submenu with no room to its parent's right opens to the LEFT of it
	// rather than being slid back over the rows it belongs to.
	float placedX = fuiClampF(leftX, 0.0f, fuiMaxF(windowWidth - width, 0.0f));
	if(opensSideways && (leftX + width) > windowWidth) {
		float flippedX = frame->triggerRect.x - width;
		if(flippedX >= 0.0f) {
			placedX = flippedX;
		}
	}

	// A popup is measured from text, so its box lands on fractions of a pixel: the title it drops from is as
	// wide as its caption, and the widest row it opens at is as wide as ITS caption. Snapped onto the pixel
	// grid the frame survives whatever the backend rounds its clip to - without this the right hand stroke
	// is the one that goes, since it is the edge every truncating scissor cuts.
	frame->popupRect = fui__SnapRectToPixels(fuiRectMake(placedX, placedY, width, boxHeight));
	frame->contentHeight = contentHeight;

	fuiRect upStrip = fui__MenuScrollStrip(frame->popupRect, true);
	fuiRect downStrip = fui__MenuScrollStrip(frame->popupRect, false);
	fuiRect noStrip = fuiRectMake(0.0f, 0.0f, 0.0f, 0.0f);
	frame->scrollUpStrip = noStrip;
	frame->scrollDownStrip = noStrip;

	float scroll = 0.0f;
	if(hiddenHeight > 0.0f && state != fui_null) {
		scroll = state->scroll;
		if(context->mouseWheelDelta != 0.0f && fui__MenuCursorIsOver(context, frame->popupRect)) {
			scroll -= context->mouseWheelDelta * fui__MenuRowHeight(context) * FUI__SCROLL_WHEEL_ROWS;
			// Taken, so a panel or a parent popup underneath does not scroll to the same turn of the wheel.
			context->mouseWheelDelta = 0.0f;
		}

		// Resting the cursor on a strip scrolls the menu, steadily and in real time. An arrow that only
		// says there is more, and does nothing when it is pointed at, is a button that lies.
		float hoverStep = fui__MenuRowHeight(context) * FUI__MENU_HOVER_SCROLL_ROWS_PER_SECOND * context->frameTime;
		if(fui__MenuCursorIsOver(context, upStrip)) {
			scroll -= hoverStep;
		} else if(fui__MenuCursorIsOver(context, downStrip)) {
			scroll += hoverStep;
		}

		scroll = fuiClampF(scroll, 0.0f, hiddenHeight);
		state->scroll = scroll;

		// Worked out from the SETTLED scroll, so the strip disappears on the frame the last row arrives.
		if(scroll > 0.0f) {
			frame->scrollUpStrip = upStrip;
		}
		if(scroll < hiddenHeight) {
			frame->scrollDownStrip = downStrip;
		}
	} else if(state != fui_null) {
		state->scroll = 0.0f;
	}
	frame->scrollOffset = scroll;
	frame->cursorY = frame->popupRect.y - scroll;
	frame->widestItem = 0.0f;

	// The clip ignores the enclosing one on purpose: a submenu's box sits OUTSIDE its parent popup, and
	// clipping it to the parent would leave nothing of it but the text.
	fui__PushClipAbsolute(context, frame->popupRect);
	fuiDrawRect(context, frame->popupRect, theme->panelBackgroundColor);

	// Anywhere inside the box belongs to the menu, including the empty space under the last row.
	if(fui__MenuCursorIsOver(context, frame->popupRect)) {
		fui__MenuTakeTheMouse(context);
	}
}

//! Closes an open popup and remembers what its rows measured, which is the size it opens at next build
static void fui__MenuEndPopup(fuiContext *context, const fuiMenuFrame *frame, const fuiId id) {
	// A popup that had to be cut says so at the edge it was cut at, or a menu with three hundred rows
	// below the fold looks exactly like one with twelve. Drawn AFTER the rows, so a row sliding under a
	// strip goes behind it rather than through it.
	if(!fuiRectIsEmpty(frame->scrollUpStrip)) {
		bool isHovered = fui__MenuCursorIsOver(context, frame->scrollUpStrip);
		fui__MenuDrawScrollHint(context, frame->scrollUpStrip, true, isHovered);
	}
	if(!fuiRectIsEmpty(frame->scrollDownStrip)) {
		bool isHovered = fui__MenuCursorIsOver(context, frame->scrollDownStrip);
		fui__MenuDrawScrollHint(context, frame->scrollDownStrip, false, isHovered);
	}

	// The frame goes on after the rows, or the hover wash on the top and bottom row - which runs the full
	// width of the popup - would paint over the stroke it sits against.
	fui__DrawChromeFrame(context, frame->popupRect);
	fuiPopClip(context);
	fuiWidgetState *state = fui__WidgetStateGet(context, id);
	if(state != fui_null) {
		// The scroll is added back, because what is remembered is how tall ALL the rows are and not how
		// much of them this build happened to show.
		float rowsEnd = frame->cursorY + frame->scrollOffset;
		state->menuSize.y = fui__CeilToPixel(rowsEnd - frame->popupRect.y);
		if(frame->widestItem > 0.0f) {
			state->menuSize.x = frame->widestItem;
		}
	}
}

//! One row of an open popup: hover wash, an optional check box, the label, and a shortcut or a submenu mark
static fuiRect fui__MenuEmitRow(fuiContext *context, fuiMenuFrame *frame, const char *label, const char *shortcutText, const bool hasSubmenuMark, const bool enabled, const bool hasCheck, const bool isChecked, bool *outIsHovered, bool *outWasClicked) {
	const fuiTheme *theme = &context->theme;
	// Menu rows are sized by their OWN theme knobs, so a menu can read at a different size than the widgets.
	const float rowFontHeight = theme->menuItemFontHeight;
	const float paddingX = theme->menuItemPaddingX;
	const float rowHeight = fui__MenuRowHeight(context);
	const float columnGap = paddingX * 2.0f;
	const float markWidth = rowFontHeight;
	const float checkBoxSide = rowFontHeight;
	// A row's content is inset from the INSIDE of the popup's frame, so the padding it keeps is clearance
	// from the stroke rather than a measure the stroke eats a pixel out of.
	const float contentInsetX = theme->panelBorderThickness + paddingX;
	float checkColumnWidth = hasCheck ? (checkBoxSide + FUI__WIDGET_LABEL_GAP) : 0.0f;

	fuiRect rowRect = fuiRectMake(frame->popupRect.x, frame->cursorY, frame->popupRect.w, rowHeight);
	frame->cursorY += rowHeight;

	// What this row would LIKE to be. The popup is already drawn by the time its rows say so, so the widest
	// of them is what the NEXT build opens at.
	fuiVec2 labelSize = fuiMeasureText(context, label, 0, rowFontHeight);
	float extrasWidth = 0.0f;
	if(shortcutText != fui_null && shortcutText[0] != '\0') {
		fuiVec2 shortcutSize = fuiMeasureText(context, shortcutText, 0, rowFontHeight);
		extrasWidth += columnGap + shortcutSize.x;
	}
	if(hasSubmenuMark) {
		extrasWidth += columnGap + markWidth;
	}
	// Whole pixels, since this is the width the popup opens at on the next build.
	float desiredWidth = fui__CeilToPixel(contentInsetX * 2.0f + checkColumnWidth + labelSize.x + extrasWidth);
	frame->widestItem = fuiMaxF(frame->widestItem, desiredWidth);

	fuiRect popupClip = fuiGetClipRect(context);
	// A row lying under a scroll strip does not take the hover: the cursor resting there is aiming at the
	// strip, and letting the row behind it light up - or open its submenu - would make the strip unusable.
	bool cursorIsOnAStrip = fuiPointInRect(context->mousePosition, frame->scrollUpStrip) || fuiPointInRect(context->mousePosition, frame->scrollDownStrip);
	bool cursorIsOnTheRow = !cursorIsOnAStrip && fui__MenuCursorIsOver(context, rowRect) && fuiPointInRect(context->mousePosition, popupClip);
	bool isHovered = enabled && cursorIsOnTheRow;
	if(isHovered) {
		fui__MenuTakeTheMouse(context);
		// Sliding onto a plain row closes whatever submenu a SIBLING row had opened.
		uint32_t childDepth = frame->depth + 1u;
		if(context->menuOpenDepth > childDepth) {
			context->menuOpenDepth = childDepth;
		}
		fuiDrawRect(context, rowRect, theme->menuHighlightColor);
	}

	fuiColor textColor = enabled ? theme->textColor : theme->textMutedColor;
	float centerY = rowRect.y + rowRect.h * 0.5f;
	float labelX = rowRect.x + contentInsetX;
	if(hasCheck) {
		// The same box a checkbox draws, so a checked menu row and a checked widget mean the same thing.
		float boxTop = rowRect.y + (rowRect.h - checkBoxSide) * 0.5f;
		fuiRect box = fuiRectMake(rowRect.x + contentInsetX, boxTop, checkBoxSide, checkBoxSide);
		fuiColor boxFill = isHovered ? theme->widgetHoveredColor : theme->widgetTrackColor;
		fui__DrawBevelBox(context, box, boxFill, FUI__RELIEF_SUNKEN);
		if(isChecked) {
			fuiRect checkMark = fui__CheckMarkRect(context, box);
			fuiDrawRect(context, checkMark, theme->accentColor);
		}
		labelX = box.x + box.w + FUI__WIDGET_LABEL_GAP;
	}
	fui__DrawTextVCentered(context, label, labelX, centerY, rowFontHeight, textColor);

	float rightEdgeX = rowRect.x + rowRect.w - contentInsetX;
	if(hasSubmenuMark) {
		fui__DrawTextVCenteredRight(context, FUI__MENU_SUBMENU_MARK, rightEdgeX, centerY, rowFontHeight, textColor);
	} else if(shortcutText != fui_null && shortcutText[0] != '\0') {
		fui__DrawTextVCenteredRight(context, shortcutText, rightEdgeX, centerY, rowFontHeight, theme->textMutedColor);
	}

	// A menu row answers the PRESS, the way a menu always has. There is nothing to change your mind about
	// halfway through, and waiting for the release would leave the popup up under the finger that chose.
	// It has to be a press nothing has taken yet: a popup that OPENED on this very press can land under the
	// cursor - a bar along the bottom of the window opens upwards over its own title - and the row it puts
	// there would otherwise answer the same press that opened it and close the menu again instantly.
	bool pressIsStillUnclaimed = context->mouseWentDown[FUI_MOUSE_LEFT] && !context->mouseDownConsumed[FUI_MOUSE_LEFT];
	bool wasClicked = isHovered && pressIsStillUnclaimed;
	if(wasClicked) {
		context->mouseDownConsumed[FUI_MOUSE_LEFT] = true;
	}
	if(outIsHovered != fui_null) {
		*outIsHovered = isHovered;
	}
	if(outWasClicked != fui_null) {
		*outWasClicked = wasClicked;
	}
	return(rowRect);
}

fui_api void fuiBeginMenuBar(fuiContext *context, const char *id, const fuiRect rect) {
	FUI_ASSERT(context != fui_null && id != fui_null);
	if(context == fui_null || id == fui_null) {
		return;
	}
	fuiDrawRect(context, rect, context->theme.panelBackgroundColor);
	if(fui__MenuCursorIsOver(context, rect)) {
		fui__MenuTakeTheMouse(context);
	}

	fuiPushId(context, id);
	fuiMenuFrame *frame = fui__PushMenu(context);
	if(frame == fui_null) {
		return;
	}
	frame->isBar = true;
	frame->barRect = rect;
	// Whole pixels from here on, because every popup in the bar is placed against this cursor and inherits
	// whatever fraction it carries.
	frame->barCursorX = fui__FloorToPixel(rect.x);
}

fui_api void fuiEndMenuBar(fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return;
	}
	fui__PopMenu(context);
	fuiPopId(context);
}

fui_api bool fuiBeginMenu(fuiContext *context, const char *label) {
	FUI_ASSERT(context != fui_null && label != fui_null);
	if(context == fui_null || label == fui_null) {
		return(false);
	}

	const fuiTheme *theme = &context->theme;
	fuiMenuFrame *parent = fui__CurrentMenu(context);
	fuiId id = fuiGetId(context, label);
	bool parentIsTheBar = (parent != fui_null) && parent->isBar;
	uint32_t myDepth = parentIsTheBar ? 0u : ((parent != fui_null) ? (parent->depth + 1u) : 0u);
	bool isOpen = (parent != fui_null) && (context->menuOpenDepth > myDepth) && (context->menuOpenPath[myDepth] == id);
	fuiRect triggerRect = fuiRectMake(0.0f, 0.0f, 0.0f, 0.0f);

	if(parentIsTheBar) {
		fuiVec2 labelSize = fuiMeasureText(context, label, 0, theme->menuItemFontHeight);
		float titleWidth = fui__CeilToPixel(labelSize.x + theme->menuItemPaddingX * 2.0f);
		triggerRect = fuiRectMake(parent->barCursorX, parent->barRect.y, titleWidth, parent->barRect.h);
		parent->barCursorX += titleWidth;

		fuiRect barClip = fuiGetClipRect(context);
		bool isHovered = fui__MenuCursorIsOver(context, triggerRect) && fuiPointInRect(context->mousePosition, barClip);
		if(isHovered) {
			fui__MenuTakeTheMouse(context);
		}
		if(isOpen || isHovered) {
			fuiDrawRect(context, triggerRect, theme->menuHighlightColor);
		}
		float titleCenterY = triggerRect.y + triggerRect.h * 0.5f;
		fui__DrawTextVCentered(context, label, triggerRect.x + theme->menuItemPaddingX, titleCenterY, theme->menuItemFontHeight, theme->textColor);

		bool someMenuIsAlreadyOpen = (context->menuOpenDepth > 0);
		if(isHovered && context->mouseWentDown[FUI_MOUSE_LEFT]) {
			context->mouseDownConsumed[FUI_MOUSE_LEFT] = true;
			if(isOpen) {
				context->menuOpenDepth = 0;
				isOpen = false;
			} else {
				context->menuOpenPath[0] = id;
				context->menuOpenDepth = 1;
				isOpen = true;
			}
		} else if(someMenuIsAlreadyOpen && isHovered && !isOpen) {
			// Once the bar is live, sliding along it switches menus without another click, the way every
			// desktop menu bar behaves.
			context->menuOpenPath[0] = id;
			context->menuOpenDepth = 1;
			isOpen = true;
		}
	} else if(parent != fui_null && parent->isOpen) {
		bool rowIsHovered = false;
		triggerRect = fui__MenuEmitRow(context, parent, label, fui_null, true, true, false, false, &rowIsHovered, fui_null);
		if(rowIsHovered && myDepth < (uint32_t)FUI_MAX_MENU_DEPTH) {
			// A submenu opens on HOVER. There is nothing else a row with an arrow could mean, and requiring
			// a click on it would be the one menu on the machine that does.
			context->menuOpenPath[myDepth] = id;
			context->menuOpenDepth = myDepth + 1u;
			isOpen = true;
		}
	} else {
		// Nested inside a menu whose popup is closed, or outside a menu bar entirely: nothing to place a
		// popup against, so this menu cannot be open.
		isOpen = false;
	}

	fuiMenuFrame *frame = fui__PushMenu(context);
	fuiPushId(context, label);
	if(frame == fui_null) {
		return(false);
	}
	frame->id = id;
	frame->depth = myDepth;
	frame->isBar = false;
	frame->isOpen = isOpen;
	frame->triggerRect = triggerRect;

	if(isOpen && parent != fui_null) {
		float leftX;
		float topY;
		if(parent->isBar) {
			leftX = triggerRect.x;
			topY = triggerRect.y + triggerRect.h;   // drops BELOW its title
		} else {
			leftX = triggerRect.x + triggerRect.w;  // opens to the RIGHT of its row
			topY = triggerRect.y;                   // with its top on the row's top
		}
		// A submenu opens to the side of its row and may have to flip; a bar menu drops straight down.
		bool opensSideways = !parent->isBar;
		fui__MenuBeginPopup(context, frame, id, leftX, topY, opensSideways);
	}
	return(isOpen);
}

fui_api void fuiEndMenu(fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return;
	}
	fuiMenuFrame *frame = fui__CurrentMenu(context);
	if(frame != fui_null && frame->isOpen) {
		fui__MenuEndPopup(context, frame, frame->id);
	}
	fui__PopMenu(context);
	fuiPopId(context);
}

fui_api bool fuiMenuItem(fuiContext *context, const char *label, const char *shortcutText, const bool enabled) {
	FUI_ASSERT(context != fui_null && label != fui_null);
	if(context == fui_null || label == fui_null) {
		return(false);
	}
	fuiMenuFrame *frame = fui__CurrentMenu(context);
	if(frame == fui_null || !frame->isOpen) {
		return(false);
	}
	bool wasClicked = false;
	(void)fui__MenuEmitRow(context, frame, label, shortcutText, false, enabled, false, false, fui_null, &wasClicked);
	if(wasClicked) {
		context->menuOpenDepth = 0;   // choosing a leaf closes the whole tree, submenus included
	}
	return(wasClicked);
}

fui_api bool fuiMenuItemCheck(fuiContext *context, const char *label, const bool isChecked, const bool enabled) {
	FUI_ASSERT(context != fui_null && label != fui_null);
	if(context == fui_null || label == fui_null) {
		return(false);
	}
	fuiMenuFrame *frame = fui__CurrentMenu(context);
	if(frame == fui_null || !frame->isOpen) {
		return(false);
	}
	bool wasClicked = false;
	(void)fui__MenuEmitRow(context, frame, label, fui_null, false, enabled, true, isChecked, fui_null, &wasClicked);
	if(wasClicked) {
		context->menuOpenDepth = 0;
	}
	return(wasClicked);
}

fui_api bool fuiMenuItemCommand(fuiContext *context, const fuiCommandTable *table, const fuiCommandId id, void *userData) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return(false);
	}
	const fuiCommand *command = fuiFindCommand(table, id);
	if(!fuiCommandIsVisible(command, userData)) {
		return(false);   // an invisible command emits no row at all, so the menu shrinks around it
	}
	bool enabled = fuiCommandIsEnabled(command, userData);
	char shortcutText[FUI_MAX_SHORTCUT_TEXT];
	(void)fuiShortcutToText(command->shortcut, shortcutText, sizeof(shortcutText));
	bool wasClicked = fuiMenuItem(context, command->label, shortcutText, enabled);
	if(wasClicked && command->invoke != fui_null) {
		command->invoke(userData);
		return(true);
	}
	return(false);
}

fui_api void fuiMenuSeparator(fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return;
	}
	fuiMenuFrame *frame = fui__CurrentMenu(context);
	if(frame == fui_null || !frame->isOpen) {
		return;
	}
	const fuiTheme *theme = &context->theme;
	float separatorHeight = fui__MenuRowHeight(context) * FUI__MENU_SEPARATOR_FRACTION;
	float lineY = frame->cursorY + separatorHeight * 0.5f;
	// Inset like a row is, so the rule lines up with the labels above and below it.
	float contentInsetX = theme->panelBorderThickness + theme->menuItemPaddingX;
	fuiVec2 lineStart = fuiV2(frame->popupRect.x + contentInsetX, lineY);
	fuiVec2 lineEnd = fuiV2(frame->popupRect.x + frame->popupRect.w - contentInsetX, lineY);
	fuiDrawLine(context, lineStart, lineEnd, theme->panelBorderColor, theme->widgetBorderThickness);
	frame->cursorY += separatorHeight;
}

//! A context menu's identity, hashed on its own rather than against the surrounding scope: opening one and
//! building it happen at two unrelated places in the tree, and they still have to agree.
fui_inline fuiId fui__ContextMenuId(const char *id) {
	size_t idLength = fui__StringLength(id);
	fuiId result = fui__HashBytes((fuiId)FUI__HASH_OFFSET_BASIS, id, idLength);
	return(result);
}

fui_api void fuiOpenContextMenu(fuiContext *context, const char *id) {
	FUI_ASSERT(context != fui_null && id != fui_null);
	if(context == fui_null || id == fui_null) {
		return;
	}
	context->menuOpenPath[0] = fui__ContextMenuId(id);
	context->menuOpenDepth = 1;
	context->contextMenuAnchor = context->mousePosition;
	// The press that OPENED the menu belongs to the menu. Without this the same press reaches the end of the
	// frame looking like a click on the background and closes what it just opened.
	fui__MenuTakeTheMouse(context);
}

fui_api bool fuiBeginContextMenu(fuiContext *context, const char *id) {
	FUI_ASSERT(context != fui_null && id != fui_null);
	if(context == fui_null || id == fui_null) {
		return(false);
	}
	fuiId contextId = fui__ContextMenuId(id);
	bool isOpen = (context->menuOpenDepth > 0) && (context->menuOpenPath[0] == contextId);

	fuiPushId(context, id);
	fuiMenuFrame *frame = fui__PushMenu(context);
	if(frame == fui_null) {
		return(false);
	}
	frame->id = contextId;
	frame->depth = 0;
	frame->isBar = false;
	frame->isOpen = isOpen;

	if(isOpen) {
		// Anchored at the cursor as it was when the menu opened, growing down and to the right of it.
		const bool aContextMenuDropsDownwards = false;
		fui__MenuBeginPopup(context, frame, contextId, context->contextMenuAnchor.x, context->contextMenuAnchor.y, aContextMenuDropsDownwards);
	}
	return(isOpen);
}

fui_api void fuiEndContextMenu(fuiContext *context) {
	fuiEndMenu(context);   // identical teardown: finish the popup, pop the frame, pop the scope
}

fui_api bool fuiIsMenuOpen(const fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return(false);
	}
	bool result = (context->menuOpenDepth > 0);
	return(result);
}

fui_api void fuiCloseMenus(fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return;
	}
	context->menuOpenDepth = 0;
}

// ----------------------------------------------------------------------------
// > Tool strip
// ----------------------------------------------------------------------------

//! How wide a strip separator is, as a multiple of the gap between two items
#define FUI__STRIP_SEPARATOR_SPACING_FACTOR 2.0f

//! Hands out the next slot along the open strip and advances the cursor past it
fui_inline fuiRect fui__StripNextSlot(fuiContext *context, const float thickness) {
	float spacing = context->theme.widgetSpacing;
	fuiRect slot;
	if(context->stripAxis == FUI_AXIS_HORIZONTAL) {
		slot = fuiRectMake(context->stripCursor, context->stripRect.y, thickness, context->stripRect.h);
	} else {
		slot = fuiRectMake(context->stripRect.x, context->stripCursor, context->stripRect.w, thickness);
	}
	// Both axes cut from the LOW coordinate now that the origin is the top left, so there is one line here
	// rather than one per direction.
	context->stripCursor += thickness + spacing;
	return(slot);
}

//! How much of the flow axis a labelled strip item takes: its text on a row, the theme's row height in a column
fui_inline float fui__StripItemThickness(const fuiContext *context, const char *label) {
	if(context->stripAxis == FUI_AXIS_HORIZONTAL) {
		fuiVec2 labelSize = fuiMeasureText(context, label, 0, context->theme.fontHeight);
		return(labelSize.x + context->theme.widgetPaddingX * 2.0f);
	}
	return(context->theme.menuItemHeight);
}

fui_api float fuiToolStripThickness(const fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return(0.0f);
	}
	// One strip item plus the inset it keeps to either edge of the bar. A strip is only ever as thick as the
	// buttons in it, and the theme already says how tall a strip button is.
	float result = fui__BarThickness(context, context->theme.menuItemHeight);
	return(result);
}

fui_api void fuiBeginToolStrip(fuiContext *context, const char *id, const fuiRect rect, const fuiAxis axis) {
	FUI_ASSERT(context != fui_null && id != fui_null);
	if(context == fui_null || id == fui_null) {
		return;
	}
	const fuiTheme *theme = &context->theme;
	fuiPushId(context, id);
	fuiDrawRect(context, rect, theme->panelBackgroundColor);
	fuiBlockMouse(context, rect);

	// The items sit in an INSET of the strip rather than filling it, so a button never runs into the edge of
	// the bar around it. Across the flow axis that is the inset fuiToolStripThickness reserved; along it, it
	// is the same gap that separates two items. A strip made taller than it needs to be simply centres its
	// items in what it was given.
	float crossInset = theme->menuItemPaddingY;
	fuiRect itemBox = (axis == FUI_AXIS_HORIZONTAL) ? fui__RectInset(rect, 0.0f, crossInset) : fui__RectInset(rect, crossInset, 0.0f);
	float naturalItemThickness = theme->menuItemHeight;
	if(axis == FUI_AXIS_HORIZONTAL && itemBox.h > naturalItemThickness) {
		itemBox = fuiRectMake(itemBox.x, itemBox.y + (itemBox.h - naturalItemThickness) * 0.5f, itemBox.w, naturalItemThickness);
	} else if(axis == FUI_AXIS_VERTICAL && itemBox.w > naturalItemThickness) {
		itemBox = fuiRectMake(itemBox.x + (itemBox.w - naturalItemThickness) * 0.5f, itemBox.y, naturalItemThickness, itemBox.h);
	}

	context->stripIsActive = true;
	context->stripRect = itemBox;
	context->stripAxis = axis;
	float leadingInset = theme->widgetSpacing;
	context->stripCursor = (axis == FUI_AXIS_HORIZONTAL) ? (itemBox.x + leadingInset) : (itemBox.y + leadingInset);
}

fui_api void fuiEndToolStrip(fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return;
	}
	context->stripIsActive = false;
	fuiPopId(context);
}

fui_api bool fuiToolStripButton(fuiContext *context, const char *label) {
	FUI_ASSERT(context != fui_null && label != fui_null);
	if(context == fui_null || label == fui_null) {
		return(false);
	}
	float thickness = fui__StripItemThickness(context, label);
	fuiRect slot = fui__StripNextSlot(context, thickness);
	bool result = fuiButton(context, slot, label);
	return(result);
}

fui_api bool fuiToolStripCommand(fuiContext *context, const fuiCommandTable *table, const fuiCommandId id, void *userData) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return(false);
	}
	const fuiCommand *command = fuiFindCommand(table, id);
	if(!fuiCommandIsVisible(command, userData)) {
		return(false);
	}
	bool enabled = fuiCommandIsEnabled(command, userData);

	float thickness = fui__StripItemThickness(context, command->label);
	fuiRect slot = fui__StripNextSlot(context, thickness);

	// The same interaction as fuiCommandButton, WITHOUT the shortcut text: a strip sizes itself to the label.
	fuiId widgetId = fui__HashBytes(fui__CurrentIdSeed(context), &id, sizeof(id));
	fuiInteraction interaction;
	fui__ClearMemory(&interaction, sizeof(interaction));
	if(enabled) {
		interaction = fuiInteract(context, widgetId, slot);
	}
	fui__DrawButton(context, slot, command->label, enabled, interaction);

	bool didFire = false;
	if(interaction.wasClicked && command->invoke != fui_null) {
		command->invoke(userData);
		didFire = true;
	}
	return(didFire);
}

fui_api bool fuiToolStripToggle(fuiContext *context, const char *label, const bool isActive, const bool enabled) {
	FUI_ASSERT(context != fui_null && label != fui_null);
	if(context == fui_null || label == fui_null) {
		return(false);
	}
	const fuiTheme *theme = &context->theme;
	float thickness = fui__StripItemThickness(context, label);
	fuiRect slot = fui__StripNextSlot(context, thickness);

	fuiId widgetId = fuiGetId(context, label);
	fuiInteraction interaction;
	fui__ClearMemory(&interaction, sizeof(interaction));
	if(enabled) {
		interaction = fuiInteract(context, widgetId, slot);
	}

	// A lit button takes the accent, a disabled one the recessed track, and the rest follow the usual states.
	fuiColor fill;
	if(!enabled) {
		fill = theme->widgetTrackColor;
	} else if(isActive) {
		fill = theme->accentColor;
	} else {
		fill = fui__WidgetFillColor(context, interaction);
	}
	fuiColor labelColor = enabled ? theme->textColor : theme->textMutedColor;
	// A lit toggle stays pushed IN for as long as it is on, which is what tells a toggle apart from a button
	// that happens to be under the cursor.
	bool buttonIsPushed = isActive || interaction.isHeld || !enabled;
	fui__Relief relief = buttonIsPushed ? FUI__RELIEF_SUNKEN : FUI__RELIEF_RAISED;
	fui__DrawBevelBox(context, slot, fill, relief);
	bool labelIsPressed = enabled && (isActive || interaction.isHeld);
	fuiRect labelRect = fui__PressedContentRect(context, slot, labelIsPressed);
	fui__DrawTextInRect(context, labelRect, label, labelColor);
	return(interaction.wasClicked);
}

fui_api void fuiToolStripSeparator(fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return;
	}
	const fuiTheme *theme = &context->theme;
	float thickness = theme->widgetSpacing * FUI__STRIP_SEPARATOR_SPACING_FACTOR;
	fuiRect slot = fui__StripNextSlot(context, thickness);
	fuiVec2 lineStart;
	fuiVec2 lineEnd;
	// The rule spans its slot end to end, the way fuiSeparator does in a flow container, so it stands exactly
	// as tall as the buttons it divides. The strip is already inset from the bar around it, so there is
	// nothing left here to trim.
	if(context->stripAxis == FUI_AXIS_HORIZONTAL) {
		float lineX = slot.x + slot.w * 0.5f;
		lineStart = fuiV2(lineX, slot.y);
		lineEnd = fuiV2(lineX, slot.y + slot.h);
	} else {
		float lineY = slot.y + slot.h * 0.5f;
		lineStart = fuiV2(slot.x, lineY);
		lineEnd = fuiV2(slot.x + slot.w, lineY);
	}
	fuiDrawLine(context, lineStart, lineEnd, theme->panelBorderColor, theme->widgetBorderThickness);
}

// ----------------------------------------------------------------------------
// > Status bar
// ----------------------------------------------------------------------------

fui_api float fuiStatusBarHeight(const fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return(0.0f);
	}
	// One line of text plus the inset above and below it. A status bar carries nothing taller than its text.
	float result = fui__BarThickness(context, context->theme.fontHeight);
	return(result);
}

fui_api void fuiBeginStatusBar(fuiContext *context, const char *id, const fuiRect rect) {
	FUI_ASSERT(context != fui_null && id != fui_null);
	if(context == fui_null || id == fui_null) {
		return;
	}
	fuiPushId(context, id);
	fuiDrawRect(context, rect, context->theme.panelBackgroundColor);
	fuiBlockMouse(context, rect);

	context->statusRect = rect;
	context->statusLeftCursor = rect.x + context->theme.widgetPaddingX;
	context->statusRightCursor = rect.x + rect.w - context->theme.widgetPaddingX;
}

fui_api void fuiEndStatusBar(fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return;
	}
	fuiPopId(context);
}

fui_api void fuiStatusText(fuiContext *context, const char *text) {
	FUI_ASSERT(context != fui_null && text != fui_null);
	if(context == fui_null || text == fui_null) {
		return;
	}
	const fuiTheme *theme = &context->theme;
	fuiVec2 textSize = fuiMeasureText(context, text, 0, theme->fontHeight);
	float centerY = context->statusRect.y + context->statusRect.h * 0.5f;
	fui__DrawTextVCentered(context, text, context->statusLeftCursor, centerY, theme->fontHeight, theme->textColor);
	context->statusLeftCursor += textSize.x + theme->widgetPaddingX * 2.0f;
}

fui_api void fuiStatusTextRight(fuiContext *context, const char *text) {
	FUI_ASSERT(context != fui_null && text != fui_null);
	if(context == fui_null || text == fui_null) {
		return;
	}
	const fuiTheme *theme = &context->theme;
	fuiVec2 textSize = fuiMeasureText(context, text, 0, theme->fontHeight);
	float leftX = context->statusRightCursor - textSize.x;
	float centerY = context->statusRect.y + context->statusRect.h * 0.5f;
	fui__DrawTextVCentered(context, text, leftX, centerY, theme->fontHeight, theme->textColor);
	context->statusRightCursor = leftX - theme->widgetPaddingX * 2.0f;
}

fui_api void fuiStatusSeparator(fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return;
	}
	const fuiTheme *theme = &context->theme;
	// Trimmed by the bar's own inset, so the rule is exactly as tall as the text on either side of it.
	float lineX = context->statusLeftCursor;
	fuiVec2 lineStart = fuiV2(lineX, context->statusRect.y + theme->menuItemPaddingY);
	fuiVec2 lineEnd = fuiV2(lineX, context->statusRect.y + context->statusRect.h - theme->menuItemPaddingY);
	fuiDrawLine(context, lineStart, lineEnd, theme->panelBorderColor, theme->widgetBorderThickness);
	context->statusLeftCursor += theme->widgetPaddingX * 2.0f;
}

// ----------------------------------------------------------------------------
// > Modal dialogs
// ----------------------------------------------------------------------------

//! Width of one button in a dialog's button row
#define FUI__DIALOG_BUTTON_WIDTH 90.0f
//! Height of one button in a dialog's button row
#define FUI__DIALOG_BUTTON_HEIGHT 28.0f
//! Height of a text field or a labelled row inside a dialog
#define FUI__DIALOG_FIELD_HEIGHT 28.0f
//! How much taller one line of dialog text is than the font it is set in
#define FUI__DIALOG_TEXT_ROW_PADDING 6.0f
//! How much window a dialog leaves free around itself, on every side
#define FUI__MODAL_WINDOW_MARGIN 24.0f
//! How much of a moved dialog stays on screen horizontally, which is the handle it is recovered by
#define FUI__MODAL_MIN_VISIBLE_WIDTH 160.0f
//! Narrowest a message box gets, so a short prompt does not come out as a slot
#define FUI__MESSAGE_BOX_MIN_WIDTH 360.0f
//! How much room a message box keeps either side of its text before it grows wider
#define FUI__MESSAGE_BOX_SIDE_MARGIN 24.0f
//! How many buttons the widest button row holds, which is Yes, No and Cancel
#define FUI__DIALOG_MAX_BUTTONS 3

//! An identifier that does NOT depend on the identifier stack, so a dialog is the same one wherever it
//! is opened from. A menu row lives under the menu's scope and the build under the screen's.
fui_inline fuiId fui__StableId(const char *label) {
	size_t labelLength = fui__StringLength(label);
	fuiId result = fui__HashBytes((fuiId)FUI__HASH_OFFSET_BASIS, label, labelLength);
	return(result);
}

//! Where a dialog sits in the open stack, or minus one when it is not open at all
fui_inline int32_t fui__ModalStackFind(const fuiContext *context, const fuiId id) {
	for(uint32_t index = 0; index < context->modalDepth; ++index) {
		if(context->modalStack[index] == id) {
			return((int32_t)index);
		}
	}
	return(-1);
}

fui_api void fuiOpenDialog(fuiContext *context, const char *id) {
	FUI_ASSERT(context != fui_null && id != fui_null);
	if(context == fui_null || id == fui_null) {
		return;
	}
	fuiId dialogId = fui__StableId(id);
	int32_t alreadyOpenAt = fui__ModalStackFind(context, dialogId);
	if(alreadyOpenAt >= 0) {
		return;
	}
	if(context->modalDepth >= (uint32_t)FUI_MAX_DIALOGS) {
		return;
	}
	context->modalStack[context->modalDepth] = dialogId;
	context->modalDepth += 1u;

	// Opening is the moment a dialog is PLACED, so it comes up centred however far the last one of its kind
	// was dragged. A desktop dialog is a new window every time it is shown and lands wherever its template
	// says; a prompt that reappears in the corner somebody parked the last one in is a prompt nobody sees.
	// The SIZE a resizable dialog was left at survives, since that is a preference rather than a placement.
	fuiWidgetState *state = fui__WidgetStateGet(context, dialogId);
	if(state != fui_null) {
		state->moveOffset = fuiV2(0.0f, 0.0f);
	}
}

fui_api void fuiCloseDialog(fuiContext *context, const char *id) {
	FUI_ASSERT(context != fui_null && id != fui_null);
	if(context == fui_null || id == fui_null) {
		return;
	}
	fuiId dialogId = fui__StableId(id);
	int32_t openAt = fui__ModalStackFind(context, dialogId);
	if(openAt < 0) {
		return;
	}
	// Anything stacked ON TOP of it shifts down one. Rare, since a dialog is almost always the front one
	// when it closes, but a caller may close one from underneath and the stack has to stay a stack.
	for(uint32_t shiftIndex = (uint32_t)openAt; shiftIndex + 1u < context->modalDepth; ++shiftIndex) {
		context->modalStack[shiftIndex] = context->modalStack[shiftIndex + 1u];
	}
	context->modalDepth -= 1u;
}

fui_api bool fuiDialogIsOpen(const fuiContext *context, const char *id) {
	FUI_ASSERT(context != fui_null && id != fui_null);
	if(context == fui_null || id == fui_null) {
		return(false);
	}
	fuiId dialogId = fui__StableId(id);
	int32_t openAt = fui__ModalStackFind(context, dialogId);
	return(openAt >= 0);
}

fui_api bool fuiIsAnyDialogOpen(const fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return(false);
	}
	return(context->modalDepth > 0);
}

fui_api void fuiCloseAllDialogs(fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return;
	}
	context->modalDepth = 0;
}

fui_api bool fuiIsFrontModal(const fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return(false);
	}
	bool blocked = fui__ModalBlocksInput(context);
	return(!blocked);
}

fui_api bool fuiModalOwnedInputThisFrame(const fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return(false);
	}
	return(context->modalOpenAtFrameStart);
}

fui_api bool fuiDialogTakeKey(fuiContext *context, const fuiKey key) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null) {
		return(false);
	}
	if(context->dialogKeyWasConsumedThisFrame) {
		return(false);
	}
	// An open menu is what escape closes first, and a menu opened from inside a dialog is exactly the case
	// where both would otherwise answer the same press.
	if(context->menuOpenDepth > 0) {
		return(false);
	}
	if(!fuiKeyWentDown(context, key)) {
		return(false);
	}
	context->dialogKeyWasConsumedThisFrame = true;
	return(true);
}

//! The box a dialog of this size sits in when nothing has moved it
fui_inline fuiRect fui__DialogCenteredRect(const fuiContext *context, const float width, const float height) {
	float centeredX = ((float)context->windowSize.x - width) * 0.5f;
	float centeredY = ((float)context->windowSize.y - height) * 0.5f;
	fuiRect result = fuiRectMake(centeredX, centeredY, width, height);
	return(result);
}

//! Shared body of both dialog openers
fui_inline bool fui__BeginModalEx(fuiContext *context, const char *id, const char *title, const float width, const float height, const bool isResizable) {
	fuiId modalId = fui__StableId(id);
	int32_t openAt = fui__ModalStackFind(context, modalId);
	bool isOpen = (openAt >= 0);

	// A builder frame is pushed whether or not the dialog is open, so fuiEndModal always pairs and a caller
	// never has to guard it.
	if(context->modalBuildDepth < (uint32_t)FUI_MAX_DIALOGS) {
		fuiModalFrame *frame = &context->modalBuildStack[context->modalBuildDepth];
		frame->id = modalId;
		frame->isOpen = isOpen;
		frame->savedModalScope = context->modalScopeId;
	}
	context->modalBuildDepth += 1u;
	if(!isOpen) {
		return(false);
	}

	const fuiTheme *theme = &context->theme;
	float titleBarHeight = fui__PanelTitleBarHeight(context);

	// What the user dragged, kept in the same retained slot a floating panel's is and keyed by the dialog.
	// EVERY dialog has one, because every dialog is dragged by its caption the way a desktop dialog is. Only
	// the SIZE half of it belongs to a resizable one; a fixed dialog keeps the size its caller asked for.
	fuiWidgetState *state = fui__WidgetStateGet(context, modalId);

	// Centred at its DEFAULT size and then pinned by its top left corner, which the title bar drags around.
	// Every desktop dialog behaves this way: dragging a corner moves that corner and nothing else. Re-centring
	// each frame instead grew the box in all four directions at once. A dialog nobody has dragged keeps a zero
	// offset, so it stays centred as the window is resized under it.
	fuiRect rect = fui__DialogCenteredRect(context, width, height);
	float defaultAnchorX = rect.x;
	float defaultAnchorY = rect.y;
	float effectiveWidth = width;
	float effectiveHeight = height;
	float anchorX = defaultAnchorX;
	float anchorY = defaultAnchorY;
	if(state != fui_null) {
		if(isResizable) {
			effectiveWidth += state->sizeOffset.x;
			effectiveHeight += state->sizeOffset.y;
		}
		anchorX += state->moveOffset.x;
		anchorY += state->moveOffset.y;
	}

	// SIZE is bounded by the window itself and not by where the dialog sits, since growing must never shove
	// the box sideways away from the cursor holding its corner. In practice the cursor is the real bound;
	// this only catches a window made smaller under an already large dialog.
	if(isResizable) {
		float widestAllowed = fuiMaxF((float)context->windowSize.x - FUI__MODAL_WINDOW_MARGIN * 2.0f, FUI__PANEL_MIN_WIDTH);
		float tallestAllowed = fuiMaxF((float)context->windowSize.y - FUI__MODAL_WINDOW_MARGIN * 2.0f, FUI__PANEL_MIN_HEIGHT);
		effectiveWidth = fuiClampF(effectiveWidth, FUI__PANEL_MIN_WIDTH, widestAllowed);
		effectiveHeight = fuiClampF(effectiveHeight, FUI__PANEL_MIN_HEIGHT, tallestAllowed);
	}

	// POSITION is bounded so the TITLE BAR is always fully on screen, since that is the handle everything
	// else is recovered by - a dialog dragged so far right that its grip left the window included. The
	// body may hang off the right or the bottom, exactly as a desktop window may.
	float furthestRight = fuiMaxF((float)context->windowSize.x - FUI__MODAL_WINDOW_MARGIN - FUI__MODAL_MIN_VISIBLE_WIDTH, FUI__MODAL_WINDOW_MARGIN);
	float furthestDown = fuiMaxF((float)context->windowSize.y - FUI__MODAL_WINDOW_MARGIN - titleBarHeight, FUI__MODAL_WINDOW_MARGIN);
	anchorX = fuiClampF(anchorX, FUI__MODAL_WINDOW_MARGIN, furthestRight);
	anchorY = fuiClampF(anchorY, FUI__MODAL_WINDOW_MARGIN, furthestDown);

	if(state != fui_null) {
		// Both offsets go back CLAMPED, so a drag that ran past an edge banks no overshoot that has to be
		// unwound before the dialog answers the cursor again.
		if(isResizable) {
			state->sizeOffset = fuiV2(effectiveWidth - width, effectiveHeight - height);
		}
		state->moveOffset = fuiV2(anchorX - defaultAnchorX, anchorY - defaultAnchorY);
	}
	rect = fuiRectMake(anchorX, anchorY, effectiveWidth, effectiveHeight);

	// The chrome is drawn against the WHOLE WINDOW rather than against whatever was clipping the code that
	// opened the dialog. A dialog opened from inside a panel is still a dialog.
	fuiRect wholeWindow = fuiRectMake(0.0f, 0.0f, (float)context->windowSize.x, (float)context->windowSize.y);
	fui__PushClipAbsolute(context, wholeWindow);
	fuiDrawRect(context, wholeWindow, theme->modalBackdropColor);

	// This is the interactive scope from here on, and it is declared BEFORE the dialog's own chrome: the
	// title bar and the resize grip are widgets of this dialog like any other, and fui__CursorIsOver freezes
	// everything built outside the front modal. Setting it afterwards leaves both of them dead.
	context->modalScopeId = modalId;

	fuiPushId(context, id);
	fuiId moveId = fuiGetId(context, "__modalMove");
	fuiId resizeId = fuiGetId(context, "__modalResize");

	fuiDrawRect(context, rect, theme->panelBackgroundColor);
	(void)fui__ClaimCursor(context, modalId, rect);

	fuiRect titleBar = fuiRectMake(rect.x, rect.y, rect.w, titleBarHeight);
	bool titleBarIsLit = false;
	if(state != fui_null) {
		// Every dialog moves, resizable or not. A message box the user cannot shove aside is a message box
		// that hides whatever it is asking about, and there is nothing behind a caption bar to click anyway.
		fuiInteraction moveInteraction = fuiInteract(context, moveId, titleBar);
		if(moveInteraction.isHeld) {
			state->moveOffset = fuiV2Add(state->moveOffset, context->mouseDelta);
		}
		titleBarIsLit = moveInteraction.isHovered || moveInteraction.isHeld;
	}
	// The bar lighting up is the whole of how a dialog says it can be moved, there being no cursor shape here.
	fuiColor titleBarColor = titleBarIsLit ? theme->widgetHoveredColor : theme->widgetTrackColor;
	fuiDrawRect(context, titleBar, titleBarColor);
	fui__DrawTextInRect(context, titleBar, title, theme->textColor);

	// After the title bar, the same way a panel does it, so the dialog is framed all the way round.
	fui__DrawChromeFrame(context, rect);

	fuiRect belowTitleBar = fuiRectMake(rect.x, rect.y + titleBarHeight, rect.w, rect.h - titleBarHeight);
	fuiRect contentBox = fui__RectInset(belowTitleBar, theme->panelPaddingX, theme->panelPaddingY);

	if(isResizable && state != fui_null) {
		// The grip owns the bottom right corner and the content gives up whatever of it the padding does not
		// already cover. RESERVED rather than overlapped: a dialog's buttons sit along the content bottom, and
		// a grip sharing pixels with a Cancel button is a coin flip over which one a click reaches.
		float gripSize = FUI__PANEL_RESIZE_GRIP_SIZE;
		float reservedBelowContent = fuiMaxF(gripSize - theme->panelPaddingY, 0.0f);
		contentBox.h -= reservedBelowContent;

		fuiRect grip = fuiRectMake(rect.x + rect.w - gripSize, rect.y + rect.h - gripSize, gripSize, gripSize);
		fuiInteraction resizeInteraction = fuiInteract(context, resizeId, grip);
		if(resizeInteraction.isHeld) {
			// One for one, because the top left is pinned: the corner being dragged is the only thing that
			// moves, so it tracks the cursor exactly.
			state->sizeOffset = fuiV2Add(state->sizeOffset, context->mouseDelta);
		}
		bool gripIsLit = resizeInteraction.isHovered || resizeInteraction.isHeld;
		fuiColor gripColor = gripIsLit ? theme->accentColor : theme->textMutedColor;
		fuiDrawResizeGrip(context, grip, gripColor, theme->widgetBorderThickness);
	}

	// Done with the chrome, so the window wide clip goes and the content gets one of its own - also absolute,
	// so the content is never scissored away by whatever hosted the code that opened the dialog.
	fuiPopClip(context);
	fui__PushClipAbsolute(context, contentBox);
	(void)fui__PushLayout(context, modalId, state, false, FUI_AXIS_VERTICAL, theme->widgetSpacing, contentBox);
	return(true);
}

fui_api bool fuiBeginModal(fuiContext *context, const char *id, const char *title, const float width, const float height) {
	FUI_ASSERT(context != fui_null && id != fui_null && title != fui_null);
	if(context == fui_null || id == fui_null || title == fui_null) {
		return(false);
	}
	const bool isResizable = false;
	return(fui__BeginModalEx(context, id, title, width, height, isResizable));
}

fui_api bool fuiBeginModalResizable(fuiContext *context, const char *id, const char *title, const float width, const float height) {
	FUI_ASSERT(context != fui_null && id != fui_null && title != fui_null);
	if(context == fui_null || id == fui_null || title == fui_null) {
		return(false);
	}
	const bool isResizable = true;
	return(fui__BeginModalEx(context, id, title, width, height, isResizable));
}

fui_api void fuiEndModal(fuiContext *context) {
	FUI_ASSERT(context != fui_null);
	if(context == fui_null || context->modalBuildDepth == 0) {
		return;
	}
	context->modalBuildDepth -= 1u;
	// A push past the maximum counted but recorded no frame, so there is nothing of it to unwind either.
	if(context->modalBuildDepth >= (uint32_t)FUI_MAX_DIALOGS) {
		return;
	}
	fuiModalFrame *frame = &context->modalBuildStack[context->modalBuildDepth];
	if(!frame->isOpen) {
		return;
	}
	fui__PopLayout(context);
	fuiPopId(context);
	fuiPopClip(context);
	context->modalScopeId = frame->savedModalScope;
}

// ----------------------------------------------------------------------------
// > List box
// ----------------------------------------------------------------------------

//! Gap between an icon and the edges of its row
#define FUI__LIST_ICON_PADDING 2.0f
//! How much taller an icon row is than a plain one when the caller did not say
#define FUI__LIST_ICON_ROW_SCALE 2.0f
//! How long after a click a second one on the same row still counts as an activation
#define FUI__DOUBLE_CLICK_SECONDS 0.35f

//! The part of the sheet one cell covers, in texture coordinates
fui_inline void fui__ListIconCellUv(const fuiListIcons *icons, const int32_t cell, fuiVec2 *outUvMin, fuiVec2 *outUvMax) {
	int32_t columnCount = (icons->columns > 0) ? icons->columns : 1;
	int32_t rowCount = (icons->rows > 0) ? icons->rows : 1;
	int32_t cellColumn = cell % columnCount;
	int32_t cellRow = (cell / columnCount) % rowCount;
	float cellWidth = 1.0f / (float)columnCount;
	float cellHeight = 1.0f / (float)rowCount;
	float uvLeft = (float)cellColumn * cellWidth;
	float uvTop = (float)cellRow * cellHeight;
	*outUvMin = fuiV2(uvLeft, uvTop);
	*outUvMax = fuiV2(uvLeft + cellWidth, uvTop + cellHeight);
}

//! Draws one row's icon and answers the rectangle its text has left
/*
	Which rows a box of a given height can actually show.

	Both list widgets used to run their loop over EVERY row and `continue` past the ones out of view. That
	is correct and it is what an immediate mode list usually does, because a list usually has thirty rows
	in it. At a million it is a million iterations a frame to draw the forty that fit.

	The range is widened by one row at each end on purpose: the first partly visible row is the one the
	scroll offset lands inside, and the last is the one the bottom edge cuts through. The per-row test
	further down is kept as well, so a row this arithmetic is generous with still draws nothing.
*/
fui_inline void fui__ListVisibleRange(const float scroll, const float viewportHeight, const float rowHeight, const int32_t rowCount, int32_t *outFirstRow, int32_t *outEndRow) {
	*outFirstRow = 0;
	*outEndRow = rowCount;
	if(rowHeight <= 0.0f) {
		return;
	}

	int32_t firstRow = (int32_t)(scroll / rowHeight);
	if(firstRow < 0) {
		firstRow = 0;
	}
	if(firstRow > rowCount) {
		firstRow = rowCount;
	}

	int32_t rowsThatFit = (int32_t)(viewportHeight / rowHeight) + 2;
	int32_t endRow = firstRow + rowsThatFit;
	if(endRow > rowCount) {
		endRow = rowCount;
	}

	*outFirstRow = firstRow;
	*outEndRow = endRow;
}

fui_inline fuiRect fui__ListBoxDrawRowIcon(fuiContext *context, const fuiRect rowRect, const float rowHeight, const fuiListIcons *icons, const int32_t rowIndex) {
	if(icons == fui_null || icons->sheet == 0 || icons->cellForRow == fui_null || rowIndex >= icons->cellForRowCount) {
		return(rowRect);
	}
	float iconBoxSize = rowHeight - FUI__LIST_ICON_PADDING * 2.0f;
	int32_t cell = icons->cellForRow[rowIndex];
	if(cell < 0) {
		// A NEGATIVE cell is one row saying it has no icon, for the odd entry out in a list that otherwise
		// does - a "none" row, a separator. Its text still starts where the icons do, so the column of labels
		// below it stays a straight edge.
		float shiftWithoutIcon = iconBoxSize + FUI__LIST_ICON_PADDING;
		fuiRect textRect = fuiRectMake(rowRect.x + shiftWithoutIcon, rowRect.y, rowRect.w - shiftWithoutIcon, rowRect.h);
		return(textRect);
	}

	fuiVec2 uvMin;
	fuiVec2 uvMax;
	fui__ListIconCellUv(icons, cell, &uvMin, &uvMax);

	// Fitted to the cell's own shape inside a square box, so a sheet of tall cells is not squashed into one.
	// A sheet that did not say how big it is has nothing to fit by and fills the box.
	float drawWidth = iconBoxSize;
	float drawHeight = iconBoxSize;
	float cellPixelWidth = icons->sheetSize.x * (uvMax.x - uvMin.x);
	float cellPixelHeight = icons->sheetSize.y * (uvMax.y - uvMin.y);
	if(cellPixelWidth > 0.0f && cellPixelHeight > 0.0f) {
		float cellAspect = cellPixelWidth / cellPixelHeight;
		if(cellAspect > 1.0f) {
			drawHeight = iconBoxSize / cellAspect;
		} else {
			drawWidth = iconBoxSize * cellAspect;
		}
	}
	float iconLeft = rowRect.x + FUI__LIST_ICON_PADDING + (iconBoxSize - drawWidth) * 0.5f;
	float iconTop = rowRect.y + FUI__LIST_ICON_PADDING + (iconBoxSize - drawHeight) * 0.5f;
	fuiRect iconRect = fuiRectMake(iconLeft, iconTop, drawWidth, drawHeight);
	fuiColor iconTint = fuiColorRGBA(1.0f, 1.0f, 1.0f, 1.0f);
	fuiDrawImage(context, iconRect, icons->sheet, uvMin, uvMax, iconTint);

	float shiftWithIcon = iconBoxSize + FUI__LIST_ICON_PADDING;
	fuiRect textRect = fuiRectMake(rowRect.x + shiftWithIcon, rowRect.y, rowRect.w - shiftWithIcon, rowRect.h);
	return(textRect);
}

fui_api bool fuiListBoxEx(fuiContext *context, const fuiRect rect, const char *id, const char *const *items, const int32_t count, int32_t *selectedIndex, const fuiListIcons *icons, bool *outWasActivated) {
	FUI_ASSERT(context != fui_null && id != fui_null && selectedIndex != fui_null);
	if(context == fui_null || id == fui_null || selectedIndex == fui_null) {
		return(false);
	}
	if(outWasActivated != fui_null) {
		*outWasActivated = false;
	}
	if(items == fui_null && count > 0) {
		return(false);
	}

	const fuiTheme *theme = &context->theme;
	fuiId listId = fuiGetId(context, id);
	fuiWidgetState *state = fui__WidgetStateGet(context, listId);

	fuiDrawRect(context, rect, theme->widgetTrackColor);
	fuiDrawRectOutline(context, rect, theme->panelBorderColor, theme->widgetBorderThickness);

	bool hasIcons = (icons != fui_null) && (icons->sheet != 0) && (icons->cellForRow != fui_null);
	float rowScale = FUI__LIST_ICON_ROW_SCALE;
	if(hasIcons && icons->rowScale > 0.0f) {
		rowScale = icons->rowScale;
	}
	float rowHeight = hasIcons ? (theme->menuItemHeight * rowScale) : theme->menuItemHeight;
	float contentLength = (float)count * rowHeight;

	// The gutter is ALWAYS reserved, its thumb simply disabled when the rows fit, so the row width does not
	// change the moment one row is added and the list does not twitch as it fills.
	float gutterWidth = fuiScrollGutterWidth();
	float rowWidth = rect.w - gutterWidth;

	bool cursorIsOverList = fui__ClaimCursor(context, listId, rect);
	float scroll = (state != fui_null) ? state->scroll : 0.0f;
	if(cursorIsOverList && context->mouseWheelDelta != 0.0f) {
		scroll -= context->mouseWheelDelta * rowHeight * FUI__SCROLL_WHEEL_ROWS;
	}

	fuiPushId(context, id);

	// Resolved and drawn BEFORE the rows, so the rows are laid out from the final offset rather than from one
	// the wheel is about to change.
	fuiRect scrollTrack = fuiRectMake(rect.x + rect.w - gutterWidth, rect.y, gutterWidth, rect.h);
	scroll = fuiScrollbarVertical(context, scrollTrack, "__listScrollbar", scroll, rect.h, contentLength);
	if(state != fui_null) {
		state->scroll = scroll;
	}

	fuiPushClip(context, rect);
	bool selectionChanged = false;
	int32_t firstVisibleRow = 0;
	int32_t endVisibleRow = count;
	fui__ListVisibleRange(scroll, rect.h, rowHeight, count, &firstVisibleRow, &endVisibleRow);
	for(int32_t rowIndex = firstVisibleRow; rowIndex < endVisibleRow; ++rowIndex) {
		float rowTop = rect.y - scroll + (float)rowIndex * rowHeight;
		bool isAboveTheBox = (rowTop + rowHeight) < rect.y;
		bool isBelowTheBox = rowTop > (rect.y + rect.h);
		if(isAboveTheBox || isBelowTheBox) {
			continue;
		}

		fuiRect rowRect = fuiRectMake(rect.x, rowTop, rowWidth, rowHeight);
		bool rowIsHovered = fui__CursorIsOver(context, rowRect);
		bool rowIsSelected = (*selectedIndex == rowIndex);
		if(rowIsSelected) {
			fuiDrawRect(context, rowRect, theme->menuHighlightColor);
		} else if(rowIsHovered) {
			fuiDrawRect(context, rowRect, theme->widgetHoveredColor);
		}
		fuiRect textRect = fui__ListBoxDrawRowIcon(context, rowRect, rowHeight, icons, rowIndex);
		fui__DrawTextInRect(context, textRect, items[rowIndex], theme->textColor);

		if(rowIsHovered && context->mouseWentDown[FUI_MOUSE_LEFT] && !context->mouseDownConsumed[FUI_MOUSE_LEFT]) {
			// A second click on the row that is already selected, inside the double click window, is the open
			// this gesture - which is what a file browser descends a folder on.
			bool isSecondClick = false;
			if(state != fui_null) {
				float secondsSinceLastClick = context->timeSeconds - state->lastClickTime;
				isSecondClick = (state->lastClickIndex == rowIndex) && (secondsSinceLastClick <= FUI__DOUBLE_CLICK_SECONDS);
			}
			if(*selectedIndex != rowIndex) {
				*selectedIndex = rowIndex;
				selectionChanged = true;
			}
			if(isSecondClick && outWasActivated != fui_null) {
				*outWasActivated = true;
			}
			if(state != fui_null) {
				state->lastClickTime = context->timeSeconds;
				state->lastClickIndex = rowIndex;
			}
			context->mouseDownConsumed[FUI_MOUSE_LEFT] = true;
			context->focused = listId;
		}
	}
	fuiPopClip(context);
	fuiPopId(context);
	return(selectionChanged);
}

fui_api bool fuiListBox(fuiContext *context, const fuiRect rect, const char *id, const char *const *items, const int32_t count, int32_t *selectedIndex) {
	const fuiListIcons *noIcons = fui_null;
	bool *noActivation = fui_null;
	return(fuiListBoxEx(context, rect, id, items, count, selectedIndex, noIcons, noActivation));
}

// ----------------------------------------------------------------------------
// > Standard dialogs
// ----------------------------------------------------------------------------

//! One button of a dialog's button row: what it says and what it answers with
typedef struct fui__DialogButtonSpec {
	//! What the button says
	const char *label;
	//! What choosing it answers
	fuiDialogResult result;
} fui__DialogButtonSpec;

//! Fills the button row of a message box and reports what enter and escape stand for
fui_inline int32_t fui__MessageBoxButtonSpecs(const fuiMessageBoxButtons buttons, fui__DialogButtonSpec *specs, fuiDialogResult *outAcceptResult, fuiDialogResult *outDismissResult) {
	int32_t count = 0;
	switch(buttons) {
		case FUI_MESSAGE_BOX_OK:
		{
			specs[count].label = "OK";
			specs[count].result = FUI_DIALOG_RESULT_OK;
			count += 1;
			*outAcceptResult = FUI_DIALOG_RESULT_OK;
			// With one button there is nothing else escape could mean.
			*outDismissResult = FUI_DIALOG_RESULT_OK;
		} break;

		case FUI_MESSAGE_BOX_OK_CANCEL:
		{
			specs[count].label = "OK";
			specs[count].result = FUI_DIALOG_RESULT_OK;
			count += 1;
			specs[count].label = "Cancel";
			specs[count].result = FUI_DIALOG_RESULT_CANCEL;
			count += 1;
			*outAcceptResult = FUI_DIALOG_RESULT_OK;
			*outDismissResult = FUI_DIALOG_RESULT_CANCEL;
		} break;

		case FUI_MESSAGE_BOX_YES_NO:
		{
			specs[count].label = "Yes";
			specs[count].result = FUI_DIALOG_RESULT_YES;
			count += 1;
			specs[count].label = "No";
			specs[count].result = FUI_DIALOG_RESULT_NO;
			count += 1;
			*outAcceptResult = FUI_DIALOG_RESULT_YES;
			*outDismissResult = FUI_DIALOG_RESULT_NO;
		} break;

		case FUI_MESSAGE_BOX_YES_NO_CANCEL:
		default:
		{
			specs[count].label = "Yes";
			specs[count].result = FUI_DIALOG_RESULT_YES;
			count += 1;
			specs[count].label = "No";
			specs[count].result = FUI_DIALOG_RESULT_NO;
			count += 1;
			specs[count].label = "Cancel";
			specs[count].result = FUI_DIALOG_RESULT_CANCEL;
			count += 1;
			*outAcceptResult = FUI_DIALOG_RESULT_YES;
			*outDismissResult = FUI_DIALOG_RESULT_CANCEL;
		} break;
	}
	return(count);
}

//! A right aligned row of buttons along the bottom of a dialog's content, plus the keys that stand for them
fui_inline fuiDialogResult fui__DialogButtonRow(fuiContext *context, const fuiRect contentBox, const fui__DialogButtonSpec *specs, const int32_t count, const fuiDialogResult acceptResult, const fuiDialogResult dismissResult) {
	const fuiTheme *theme = &context->theme;
	float spacing = theme->widgetSpacing;
	fuiDialogResult chosen = FUI_DIALOG_RESULT_NONE;

	float rowWidth = (float)count * FUI__DIALOG_BUTTON_WIDTH + (float)(count - 1) * spacing;
	float buttonX = contentBox.x + contentBox.w - rowWidth;
	float buttonY = contentBox.y + contentBox.h - FUI__DIALOG_BUTTON_HEIGHT;
	for(int32_t buttonIndex = 0; buttonIndex < count; ++buttonIndex) {
		fuiRect buttonRect = fuiRectMake(buttonX, buttonY, FUI__DIALOG_BUTTON_WIDTH, FUI__DIALOG_BUTTON_HEIGHT);
		if(fuiButton(context, buttonRect, specs[buttonIndex].label)) {
			chosen = specs[buttonIndex].result;
		}
		buttonX += FUI__DIALOG_BUTTON_WIDTH + spacing;
	}

	// Only the FRONT dialog answers a key, and only one dialog answers each press - see fuiDialogTakeKey for
	// why the second half of that is not the same statement as the first.
	if(fuiIsFrontModal(context)) {
		if(fuiDialogTakeKey(context, FUI_KEY_RETURN)) {
			chosen = acceptResult;
		} else if(fuiDialogTakeKey(context, FUI_KEY_ESCAPE)) {
			chosen = dismissResult;
		}
	}
	return(chosen);
}

//! The row a single line of dialog text occupies
fui_inline float fui__DialogTextRowHeight(const fuiContext *context) {
	float result = context->theme.fontHeight + FUI__DIALOG_TEXT_ROW_PADDING;
	return(result);
}

//! Gives the keyboard to a dialog's field while nothing inside the dialog holds it, so it can be typed into
//! straight away. Called every build, because the focus is given up again by a click on the background.
fui_inline void fui__DialogFocusField(fuiContext *context, const char *fieldId) {
	if(context->focused != FUI_ID_NONE) {
		return;
	}
	fuiId id = fuiGetId(context, fieldId);
	context->focused = id;
}

fui_api fuiDialogResult fuiMessageBox(fuiContext *context, const char *id, const char *title, const char *text, const fuiMessageBoxButtons buttons) {
	FUI_ASSERT(context != fui_null && id != fui_null && title != fui_null && text != fui_null);
	if(context == fui_null || id == fui_null || title == fui_null || text == fui_null) {
		return(FUI_DIALOG_RESULT_NONE);
	}
	if(!fuiDialogIsOpen(context, id)) {
		return(FUI_DIALOG_RESULT_NONE);
	}

	// The message is drawn as one line, so a box narrower than it would silently clip the tail. The box grows
	// to fit instead, with the minimum as a floor so a short prompt stays compact.
	fuiVec2 messageSize = fuiMeasureText(context, text, 0, context->theme.fontHeight);
	float fittedWidth = messageSize.x + FUI__MESSAGE_BOX_SIDE_MARGIN * 2.0f;
	float dialogWidth = fuiMaxF(fittedWidth, FUI__MESSAGE_BOX_MIN_WIDTH);
	const float dialogHeight = 160.0f;

	fuiDialogResult result = FUI_DIALOG_RESULT_NONE;
	if(fuiBeginModal(context, id, title, dialogWidth, dialogHeight)) {
		fuiRect content = fuiLayoutRemaining(context);
		float textRowHeight = fui__DialogTextRowHeight(context);
		fuiRect textRect = fuiRectMake(content.x, content.y, content.w, textRowHeight);
		fuiLabel(context, textRect, text);

		fui__DialogButtonSpec specs[FUI__DIALOG_MAX_BUTTONS];
		fuiDialogResult acceptResult = FUI_DIALOG_RESULT_NONE;
		fuiDialogResult dismissResult = FUI_DIALOG_RESULT_NONE;
		int32_t buttonCount = fui__MessageBoxButtonSpecs(buttons, specs, &acceptResult, &dismissResult);
		result = fui__DialogButtonRow(context, content, specs, buttonCount, acceptResult, dismissResult);
	}
	fuiEndModal(context);

	if(result != FUI_DIALOG_RESULT_NONE) {
		fuiCloseDialog(context, id);
	}
	return(result);
}

fui_api fuiDialogResult fuiInputBox(fuiContext *context, const char *id, const char *title, const char *label, char *buffer, const int32_t capacity) {
	FUI_ASSERT(context != fui_null && id != fui_null && title != fui_null && label != fui_null && buffer != fui_null);
	if(context == fui_null || id == fui_null || title == fui_null || label == fui_null || buffer == fui_null) {
		return(FUI_DIALOG_RESULT_NONE);
	}
	if(!fuiDialogIsOpen(context, id)) {
		return(FUI_DIALOG_RESULT_NONE);
	}

	const float dialogWidth = 360.0f;
	const float dialogHeight = 180.0f;
	fuiDialogResult result = FUI_DIALOG_RESULT_NONE;
	if(fuiBeginModal(context, id, title, dialogWidth, dialogHeight)) {
		const fuiTheme *theme = &context->theme;
		fuiRect content = fuiLayoutRemaining(context);
		float labelRowHeight = fui__DialogTextRowHeight(context);

		fuiRect labelRect = fuiRectMake(content.x, content.y, content.w, labelRowHeight);
		fuiLabel(context, labelRect, label);

		float fieldTop = labelRect.y + labelRect.h + theme->widgetSpacing;
		fuiRect fieldRect = fuiRectMake(content.x, fieldTop, content.w, FUI__DIALOG_FIELD_HEIGHT);
		fui__DialogFocusField(context, "__inputField");
		(void)fuiTextInput(context, fieldRect, "__inputField", buffer, capacity);

		fui__DialogButtonSpec specs[2];
		specs[0].label = "OK";
		specs[0].result = FUI_DIALOG_RESULT_OK;
		specs[1].label = "Cancel";
		specs[1].result = FUI_DIALOG_RESULT_CANCEL;
		result = fui__DialogButtonRow(context, content, specs, 2, FUI_DIALOG_RESULT_OK, FUI_DIALOG_RESULT_CANCEL);
	}
	fuiEndModal(context);

	if(result != FUI_DIALOG_RESULT_NONE) {
		context->focused = FUI_ID_NONE;
		fuiCloseDialog(context, id);
	}
	return(result);
}

fui_api fuiDialogResult fuiInputBoxMultiline(fuiContext *context, const char *id, const char *title, const char *label, char *buffer, const int32_t capacity, const bool wordWrap) {
	FUI_ASSERT(context != fui_null && id != fui_null && title != fui_null && label != fui_null && buffer != fui_null);
	if(context == fui_null || id == fui_null || title == fui_null || label == fui_null || buffer == fui_null) {
		return(FUI_DIALOG_RESULT_NONE);
	}
	if(!fuiDialogIsOpen(context, id)) {
		return(FUI_DIALOG_RESULT_NONE);
	}

	const float dialogWidth = 460.0f;
	const float dialogHeight = 340.0f;
	fuiDialogResult result = FUI_DIALOG_RESULT_NONE;
	if(fuiBeginModal(context, id, title, dialogWidth, dialogHeight)) {
		const fuiTheme *theme = &context->theme;
		fuiRect content = fuiLayoutRemaining(context);
		float labelRowHeight = fui__DialogTextRowHeight(context);

		// Label pinned to the top, button row on the content bottom, the text AREA filling what is between.
		fuiRect labelRect = fuiRectMake(content.x, content.y, content.w, labelRowHeight);
		fuiLabel(context, labelRect, label);

		float areaTop = labelRect.y + labelRect.h + theme->widgetSpacing;
		float areaBottom = content.y + content.h - FUI__DIALOG_BUTTON_HEIGHT - theme->widgetSpacing;
		fuiRect areaRect = fuiRectMake(content.x, areaTop, content.w, areaBottom - areaTop);
		fui__DialogFocusField(context, "__inputArea");
		const bool isMultiline = true;
		(void)fuiTextInputEx(context, areaRect, "__inputArea", buffer, capacity, isMultiline, wordWrap);

		fui__DialogButtonSpec specs[2];
		specs[0].label = "OK";
		specs[0].result = FUI_DIALOG_RESULT_OK;
		specs[1].label = "Cancel";
		specs[1].result = FUI_DIALOG_RESULT_CANCEL;
		result = fui__DialogButtonRow(context, content, specs, 2, FUI_DIALOG_RESULT_OK, FUI_DIALOG_RESULT_CANCEL);
	}
	fuiEndModal(context);

	if(result != FUI_DIALOG_RESULT_NONE) {
		context->focused = FUI_ID_NONE;
		fuiCloseDialog(context, id);
	}
	return(result);
}

fui_api fuiDialogResult fuiColorDialogEx(fuiContext *context, const char *id, const char *title, fuiColor *color, const bool hasAlpha, bool *liveUpdate) {
	FUI_ASSERT(context != fui_null && id != fui_null && title != fui_null && color != fui_null);
	if(context == fui_null || id == fui_null || title == fui_null || color == fui_null) {
		return(FUI_DIALOG_RESULT_NONE);
	}
	if(!fuiDialogIsOpen(context, id)) {
		return(FUI_DIALOG_RESULT_NONE);
	}

	// Wide enough that the saturation square stays roughly square once the channel column takes its share.
	const float dialogWidth = 500.0f;
	const float dialogBaseHeight = 380.0f;
	const float previewRowHeight = 26.0f;
	// The live update row exists only when the caller offered the switch, so a dialog without one keeps
	// exactly the height it had.
	bool showLiveUpdateRow = (liveUpdate != fui_null);
	float liveUpdateRowHeight = showLiveUpdateRow ? previewRowHeight : 0.0f;
	float liveUpdateSpacing = showLiveUpdateRow ? context->theme.widgetSpacing : 0.0f;
	float dialogHeight = dialogBaseHeight + liveUpdateRowHeight + liveUpdateSpacing;

	fuiDialogResult result = FUI_DIALOG_RESULT_NONE;
	if(fuiBeginModal(context, id, title, dialogWidth, dialogHeight)) {
		const fuiTheme *theme = &context->theme;
		fuiRect content = fuiLayoutRemaining(context);

		// The picker takes everything the swatch, the optional switch and the button row leave.
		float reservedHeight = previewRowHeight + liveUpdateRowHeight + liveUpdateSpacing + FUI__DIALOG_BUTTON_HEIGHT + theme->widgetSpacing * 2.0f;
		float pickerHeight = content.h - reservedHeight;
		fuiRect pickerRect = fuiRectMake(content.x, content.y, content.w, pickerHeight);
		bool *noBeginFlag = fui_null;
		bool *noEndFlag = fui_null;
		(void)fuiColorPicker(context, pickerRect, "__colorPicker", color, hasAlpha, true, noBeginFlag, noEndFlag);

		// The result, over a chequer so a low alpha reads as transparency rather than as a darker colour.
		float previewTop = pickerRect.y + pickerRect.h + theme->widgetSpacing;
		fuiRect previewRect = fuiRectMake(content.x, previewTop, content.w, previewRowHeight);
		fui__DrawCheckerboard(context, previewRect);
		float previewAlpha = hasAlpha ? color->a : 1.0f;
		fuiColor previewColor = fuiColorRGBA(color->r, color->g, color->b, previewAlpha);
		fuiDrawRect(context, previewRect, previewColor);
		fuiDrawRectOutline(context, previewRect, theme->panelBorderColor, theme->widgetBorderThickness);

		// A swatch says what the colour IS; it cannot say what it DOES. Live update lets the caller push every
		// drag straight through to the real thing, which for a light is the only way to judge one. The switch
		// belongs to the caller, so it survives the dialog being reopened.
		if(showLiveUpdateRow) {
			float liveUpdateTop = previewRect.y + previewRect.h + liveUpdateSpacing;
			fuiRect liveUpdateRect = fuiRectMake(content.x, liveUpdateTop, content.w, liveUpdateRowHeight);
			(void)fuiCheckbox(context, liveUpdateRect, "Live update", liveUpdate);
		}

		fui__DialogButtonSpec specs[2];
		specs[0].label = "OK";
		specs[0].result = FUI_DIALOG_RESULT_OK;
		specs[1].label = "Cancel";
		specs[1].result = FUI_DIALOG_RESULT_CANCEL;
		result = fui__DialogButtonRow(context, content, specs, 2, FUI_DIALOG_RESULT_OK, FUI_DIALOG_RESULT_CANCEL);
	}
	fuiEndModal(context);

	if(result != FUI_DIALOG_RESULT_NONE) {
		fuiCloseDialog(context, id);
	}
	return(result);
}

fui_api fuiDialogResult fuiColorDialog(fuiContext *context, const char *id, const char *title, fuiColor *color, const bool hasAlpha) {
	bool *noLiveUpdateFlag = fui_null;
	return(fuiColorDialogEx(context, id, title, color, hasAlpha, noLiveUpdateFlag));
}

fui_api fuiDialogResult fuiFileDialog(fuiContext *context, const char *id, const char *title, const char *const *items, const int32_t count, int32_t *selectedIndex, const fuiFileDialogMode mode, char *nameBuffer, const int32_t nameCapacity, const fuiListIcons *icons) {
	FUI_ASSERT(context != fui_null && id != fui_null && title != fui_null && selectedIndex != fui_null);
	if(context == fui_null || id == fui_null || title == fui_null || selectedIndex == fui_null) {
		return(FUI_DIALOG_RESULT_NONE);
	}
	if(!fuiDialogIsOpen(context, id)) {
		return(FUI_DIALOG_RESULT_NONE);
	}

	const float dialogWidth = 840.0f;
	const float dialogHeight = 680.0f;
	bool isSaving = (mode == FUI_FILE_DIALOG_SAVE) && (nameBuffer != fui_null);
	fuiDialogResult result = FUI_DIALOG_RESULT_NONE;
	if(fuiBeginModalResizable(context, id, title, dialogWidth, dialogHeight)) {
		const fuiTheme *theme = &context->theme;
		fuiRect content = fuiLayoutRemaining(context);
		float gap = theme->widgetSpacing;

		// Budgeted from the bottom up: the button row, then the name field when saving, then the list takes
		// everything above them.
		float buttonRowTop = content.y + content.h - FUI__DIALOG_BUTTON_HEIGHT;
		float nameFieldTop = buttonRowTop - gap - FUI__DIALOG_FIELD_HEIGHT;
		float listBottom = (isSaving ? nameFieldTop : buttonRowTop) - gap;
		fuiRect listRect = fuiRectMake(content.x, content.y, content.w, listBottom - content.y);

		bool *noActivation = fui_null;
		if(fuiListBoxEx(context, listRect, "__fileList", items, count, selectedIndex, icons, noActivation)) {
			// Picking a row while saving copies its name into the field, ready to be edited or overwritten.
			bool selectionIsReal = (*selectedIndex >= 0) && (*selectedIndex < count);
			if(isSaving && selectionIsReal) {
				fui__CopyString(nameBuffer, (size_t)nameCapacity, items[*selectedIndex]);
			}
		}

		if(isSaving) {
			fuiRect nameRect = fuiRectMake(content.x, nameFieldTop, content.w, FUI__DIALOG_FIELD_HEIGHT);
			(void)fuiTextInput(context, nameRect, "__fileName", nameBuffer, nameCapacity);
		}

		fui__DialogButtonSpec specs[2];
		specs[0].label = isSaving ? "Save" : "Open";
		specs[0].result = FUI_DIALOG_RESULT_OK;
		specs[1].label = "Cancel";
		specs[1].result = FUI_DIALOG_RESULT_CANCEL;
		result = fui__DialogButtonRow(context, content, specs, 2, FUI_DIALOG_RESULT_OK, FUI_DIALOG_RESULT_CANCEL);

		// Opening needs something to open. An accept with nothing picked is ignored rather than answered.
		if(result == FUI_DIALOG_RESULT_OK && !isSaving) {
			bool selectionIsReal = (*selectedIndex >= 0) && (*selectedIndex < count);
			if(!selectionIsReal) {
				result = FUI_DIALOG_RESULT_NONE;
			}
		}
	}
	fuiEndModal(context);

	if(result != FUI_DIALOG_RESULT_NONE) {
		context->focused = FUI_ID_NONE;
		fuiCloseDialog(context, id);
	}
	return(result);
}

fui_api fuiFileBrowserResult fuiFileBrowser(fuiContext *context, const char *id, const char *title, const char *locationLabel, const char *const *items, const bool *isFolder, const int32_t count, int32_t *selectedIndex, const fuiListIcons *icons, char *nameBuffer, const int32_t nameCapacity, int32_t *outIndex) {
	FUI_ASSERT(context != fui_null && id != fui_null && title != fui_null && selectedIndex != fui_null);
	if(context == fui_null || id == fui_null || title == fui_null || selectedIndex == fui_null) {
		return(FUI_FILE_BROWSER_NONE);
	}
	if(isFolder == fui_null && count > 0) {
		return(FUI_FILE_BROWSER_NONE);
	}
	if(!fuiDialogIsOpen(context, id)) {
		return(FUI_FILE_BROWSER_NONE);
	}

	// The name field is what makes this a SAVING browser: there is nothing else to type into.
	bool isSaving = (nameBuffer != fui_null);
	const float dialogWidth = 840.0f;
	const float dialogHeight = 680.0f;
	fuiFileBrowserResult result = FUI_FILE_BROWSER_NONE;
	int32_t activatedRow = -1;
	bool acceptedTypedName = false;
	if(fuiBeginModalResizable(context, id, title, dialogWidth, dialogHeight)) {
		const fuiTheme *theme = &context->theme;
		fuiRect content = fuiLayoutRemaining(context);
		float gap = theme->widgetSpacing;

		fuiRect locationRect = fuiRectMake(content.x, content.y, content.w, FUI__DIALOG_FIELD_HEIGHT);
		fuiLabel(context, locationRect, locationLabel);

		float buttonRowTop = content.y + content.h - FUI__DIALOG_BUTTON_HEIGHT;
		float nameFieldTop = buttonRowTop - gap - FUI__DIALOG_FIELD_HEIGHT;
		float listTop = locationRect.y + locationRect.h + gap;
		float listBottom = (isSaving ? nameFieldTop : buttonRowTop) - gap;
		fuiRect listRect = fuiRectMake(content.x, listTop, content.w, listBottom - listTop);

		bool rowWasActivated = false;
		bool selectionChanged = fuiListBoxEx(context, listRect, "__browserList", items, count, selectedIndex, icons, &rowWasActivated);
		bool selectionIsReal = (*selectedIndex >= 0) && (*selectedIndex < count);
		// Clicking a FILE row while saving copies its name into the field, ready to overwrite it.
		if(isSaving && selectionChanged && selectionIsReal && !isFolder[*selectedIndex]) {
			fui__CopyString(nameBuffer, (size_t)nameCapacity, items[*selectedIndex]);
		}
		if(rowWasActivated) {
			activatedRow = *selectedIndex;
		}

		if(isSaving) {
			fuiRect nameRect = fuiRectMake(content.x, nameFieldTop, content.w, FUI__DIALOG_FIELD_HEIGHT);
			(void)fuiTextInput(context, nameRect, "__fileName", nameBuffer, nameCapacity);
		}

		fui__DialogButtonSpec specs[2];
		specs[0].label = isSaving ? "Save" : "Open";
		specs[0].result = FUI_DIALOG_RESULT_OK;
		specs[1].label = "Cancel";
		specs[1].result = FUI_DIALOG_RESULT_CANCEL;
		fuiDialogResult buttonResult = fui__DialogButtonRow(context, content, specs, 2, FUI_DIALOG_RESULT_OK, FUI_DIALOG_RESULT_CANCEL);
		if(buttonResult == FUI_DIALOG_RESULT_OK) {
			if(isSaving) {
				// Saving accepts the TYPED name, which is not necessarily any row.
				acceptedTypedName = (nameBuffer[0] != '\0');
			} else if(selectionIsReal) {
				activatedRow = *selectedIndex;
			}
		} else if(buttonResult == FUI_DIALOG_RESULT_CANCEL) {
			result = FUI_FILE_BROWSER_CANCEL;
		}
	}
	fuiEndModal(context);

	// An activated row is a folder to descend into or a file to accept. Cancel already won above.
	bool activatedRowIsReal = (activatedRow >= 0) && (activatedRow < count);
	if(result == FUI_FILE_BROWSER_NONE && activatedRowIsReal) {
		if(isFolder[activatedRow]) {
			if(outIndex != fui_null) {
				*outIndex = activatedRow;
			}
			result = FUI_FILE_BROWSER_DESCEND;
		} else if(isSaving) {
			// A file double clicked while saving is the one being overwritten.
			fui__CopyString(nameBuffer, (size_t)nameCapacity, items[activatedRow]);
			acceptedTypedName = true;
		} else {
			if(outIndex != fui_null) {
				*outIndex = activatedRow;
			}
			result = FUI_FILE_BROWSER_ACCEPT;
		}
	}
	if(result == FUI_FILE_BROWSER_NONE && acceptedTypedName) {
		if(outIndex != fui_null) {
			// The accept is the typed name, which may match no listed row at all.
			*outIndex = -1;
		}
		result = FUI_FILE_BROWSER_ACCEPT;
	}

	// Descend keeps the dialog OPEN, since the caller lists the folder and comes straight back. Cancel closes
	// it. An opening accept closes; a saving one stays up, so the caller can run an overwrite prompt first.
	bool closesNow = (result == FUI_FILE_BROWSER_CANCEL) || (result == FUI_FILE_BROWSER_ACCEPT && !isSaving);
	if(closesNow) {
		context->focused = FUI_ID_NONE;
		fuiCloseDialog(context, id);
	}
	return(result);
}

// ----------------------------------------------------------------------------
// > Image
// ----------------------------------------------------------------------------

//! The largest box of the given shape that fits inside the space it is offered, which is what a letterbox is
fui_inline void fui__FitAspect(const float aspect, const float availableWidth, const float availableHeight, float *outWidth, float *outHeight) {
	float fittedWidth = availableWidth;
	float fittedHeight = (aspect > 0.0f) ? (availableWidth / aspect) : availableHeight;
	if(fittedHeight > availableHeight) {
		fittedHeight = availableHeight;
		fittedWidth = availableHeight * aspect;
	}
	*outWidth = fittedWidth;
	*outHeight = fittedHeight;
}

//! Which third of its box the image is anchored in, across and down, as 0, 1 or 2
fui_inline void fui__ImagePlacementCells(const fuiImagePlacement placement, int32_t *outAcross, int32_t *outDown) {
	switch(placement) {
		case FUI_IMAGE_PLACE_TOP_LEFT: *outAcross = 0; *outDown = 0; break;
		case FUI_IMAGE_PLACE_TOP: *outAcross = 1; *outDown = 0; break;
		case FUI_IMAGE_PLACE_TOP_RIGHT: *outAcross = 2; *outDown = 0; break;
		case FUI_IMAGE_PLACE_LEFT: *outAcross = 0; *outDown = 1; break;
		case FUI_IMAGE_PLACE_RIGHT: *outAcross = 2; *outDown = 1; break;
		case FUI_IMAGE_PLACE_BOTTOM_LEFT: *outAcross = 0; *outDown = 2; break;
		case FUI_IMAGE_PLACE_BOTTOM: *outAcross = 1; *outDown = 2; break;
		case FUI_IMAGE_PLACE_BOTTOM_RIGHT: *outAcross = 2; *outDown = 2; break;
		case FUI_IMAGE_PLACE_CENTER:
		default: *outAcross = 1; *outDown = 1; break;
	}
}

fui_api void fuiImage(fuiContext *context, const fuiRect rect, const fuiImageDesc *desc) {
	FUI_ASSERT(context != fui_null && desc != fui_null);
	if(context == fui_null || desc == fui_null || desc->texture == FUI_TEXTURE_ID_NONE) {
		return;
	}
	if(context->pass == FUI_PASS_INTERACT) {
		return;
	}

	// An all-zero uv pair is the caller saying nothing about which part they want, which is the whole texture.
	fuiVec2 uvMin = desc->uvMin;
	fuiVec2 uvMax = desc->uvMax;
	bool uvWasNotGiven = (uvMin.x == uvMax.x) && (uvMin.y == uvMax.y);
	if(uvWasNotGiven) {
		uvMin = fuiV2(0.0f, 0.0f);
		uvMax = fuiV2(1.0f, 1.0f);
	}

	// Pixel size of the sampled part, which is what "its own size" means for a part of a sheet.
	float sampledWidth = FUI_FABSF(uvMax.x - uvMin.x) * desc->textureSize.x;
	float sampledHeight = FUI_FABSF(uvMax.y - uvMin.y) * desc->textureSize.y;
	if(sampledWidth <= 0.0f || sampledHeight <= 0.0f) {
		return;
	}

	// The size asked for: the caller's own units turned into pixels, or the sampled size, and then the plain
	// multiplier on top - all of it BEFORE the scale mode places or fits it.
	bool hasRenderSize = (desc->renderSize.x > 0.0f) && (desc->renderSize.y > 0.0f) && (desc->pixelsPerUnit > 0.0f);
	float scaleFactor = (desc->scaleFactor > 0.0f) ? desc->scaleFactor : 1.0f;
	float requestedWidth = (hasRenderSize ? (desc->renderSize.x * desc->pixelsPerUnit) : sampledWidth) * scaleFactor;
	float requestedHeight = (hasRenderSize ? (desc->renderSize.y * desc->pixelsPerUnit) : sampledHeight) * scaleFactor;

	// A quarter turn swaps what the shape IS, so everything below fits and places the turned shape. Without
	// this a tall cell laid on its side would be fitted as though it were still tall and come out squashed.
	bool turnsClockwise = (desc->flags & FUI_IMAGE_ROTATE_90_CW) == FUI_IMAGE_ROTATE_90_CW;
	bool turnsCounterClockwise = (desc->flags & FUI_IMAGE_ROTATE_90_CCW) == FUI_IMAGE_ROTATE_90_CCW;
	bool isTurned = turnsClockwise != turnsCounterClockwise;
	if(isTurned) {
		float swappedRequestedWidth = requestedWidth;
		requestedWidth = requestedHeight;
		requestedHeight = swappedRequestedWidth;
		float swappedSampledWidth = sampledWidth;
		sampledWidth = sampledHeight;
		sampledHeight = swappedSampledWidth;
	}
	float sampledAspect = sampledWidth / sampledHeight;

	float drawWidth = requestedWidth;
	float drawHeight = requestedHeight;
	switch(desc->scaleMode) {
		case FUI_IMAGE_SCALE_STRETCH:
		{
			drawWidth = rect.w;
			drawHeight = rect.h;
		} break;

		case FUI_IMAGE_SCALE_LETTERBOX:
		{
			fui__FitAspect(sampledAspect, rect.w, rect.h, &drawWidth, &drawHeight);
		} break;

		case FUI_IMAGE_SCALE_ORIGIN:
		case FUI_IMAGE_SCALE_CENTER:
		default:
		{
			if(desc->keepAspectRatio) {
				// Keeping the shape, inside both what was asked for and what the box has room for.
				float boundedWidth = fuiMinF(requestedWidth, rect.w);
				float boundedHeight = fuiMinF(requestedHeight, rect.h);
				fui__FitAspect(sampledAspect, boundedWidth, boundedHeight, &drawWidth, &drawHeight);
			}
		} break;
	}

	fuiImagePlacement placement = (desc->scaleMode == FUI_IMAGE_SCALE_CENTER) ? FUI_IMAGE_PLACE_CENTER : desc->placement;
	int32_t placeAcross = 1;
	int32_t placeDown = 1;
	fui__ImagePlacementCells(placement, &placeAcross, &placeDown);
	float freeWidth = rect.w - drawWidth;
	float freeHeight = rect.h - drawHeight;
	float drawLeft = rect.x + freeWidth * ((float)placeAcross * 0.5f);
	float drawTop = rect.y + freeHeight * ((float)placeDown * 0.5f);

	// A fully transparent tint is what a zeroed description holds, and nobody asks to draw an image nobody
	// can see - so that is the one value taken as "no tint was chosen".
	fuiColor tint = desc->tint;
	if(tint.a <= 0.0f) {
		tint = fuiColorRGBA(1.0f, 1.0f, 1.0f, 1.0f);
	}
	fuiRect drawRect = fuiRectMake(drawLeft, drawTop, drawWidth, drawHeight);
	fuiDrawImageEx(context, drawRect, desc->texture, uvMin, uvMax, tint, desc->flags);
}

// ----------------------------------------------------------------------------
// > Tab control
// ----------------------------------------------------------------------------

//! Thickness of the accent line under the tab that is showing
#define FUI__TAB_ACCENT_THICKNESS 3.0f
//! Gap between two tab headers
#define FUI__TAB_HEADER_SPACING 2.0f

fui_api int32_t fuiTabControl(fuiContext *context, const fuiRect rect, const char *id, const char *const *tabs, const int32_t tabCount) {
	FUI_ASSERT(context != fui_null && id != fui_null);
	if(context == fui_null || id == fui_null || tabs == fui_null || tabCount <= 0) {
		return(0);
	}

	const fuiTheme *theme = &context->theme;
	fuiId controlId = fuiGetId(context, id);
	fuiWidgetState *state = fui__WidgetStateGet(context, controlId);

	int32_t activeTab = (state != fui_null) ? state->activeTab : 0;
	activeTab = (int32_t)fuiClampF((float)activeTab, 0.0f, (float)(tabCount - 1));

	// The headers flow left to right, each as wide as what it says - a horizontal tool strip with a memory.
	fuiPushId(context, id);
	float headerLeft = rect.x;
	for(int32_t tabIndex = 0; tabIndex < tabCount; ++tabIndex) {
		fuiVec2 labelSize = fuiMeasureText(context, tabs[tabIndex], 0, theme->fontHeight);
		float headerWidth = labelSize.x + theme->widgetPaddingX * 2.0f;
		fuiRect headerRect = fuiRectMake(headerLeft, rect.y, headerWidth, rect.h);

		fuiPushIdInt(context, tabIndex);
		fuiId headerId = fuiGetId(context, "__tabHeader");
		fuiPopId(context);
		fuiInteraction interaction = fuiInteract(context, headerId, headerRect);

		bool isShowing = (tabIndex == activeTab);
		fuiColor fill = isShowing ? theme->widgetActiveColor : fui__WidgetFillColor(context, interaction);
		// The tab in front stands out of the strip and the ones behind it lie FLAT in it. Flat rather than
		// sunken: sunken is the shape of a widget that has been pushed, and a tab nobody is pushing must not
		// wear it - a whole row of them reads as a row of dead buttons. Pushing one really does sink it.
		fui__Relief relief = FUI__RELIEF_FLAT;
		if(isShowing) {
			relief = FUI__RELIEF_RAISED;
		} else if(interaction.isHeld) {
			relief = FUI__RELIEF_SUNKEN;
		}
		fui__DrawBevelBox(context, headerRect, fill, relief);
		// Every caption in the strip is drawn in the FULL text color. The muted one is what a disabled widget
		// takes, and a tab that is merely not showing is not disabled - it was the hardest thing on the strip
		// to read while it wore that color. Which page is showing is carried by the relief, the fill and the
		// accent line under it, which is three cues without spending the caption on a fourth.
		// The captions are NOT nudged with the headers they sit on either. They stand in a row, and a row of
		// labels stepping up and down as the selection moves is a jitter rather than a depth cue.
		fui__DrawTextInRect(context, headerRect, tabs[tabIndex], theme->textColor);
		if(isShowing) {
			// Under the header rather than over it, which is where an underline goes now that down is down.
			fuiRect underline = fuiRectMake(headerRect.x, headerRect.y + headerRect.h - FUI__TAB_ACCENT_THICKNESS, headerRect.w, FUI__TAB_ACCENT_THICKNESS);
			fuiDrawRect(context, underline, theme->accentColor);
		}
		if(interaction.wasClicked) {
			activeTab = tabIndex;
		}
		headerLeft += headerWidth + FUI__TAB_HEADER_SPACING;
	}
	fuiPopId(context);

	if(state != fui_null) {
		state->activeTab = activeTab;
	}
	return(activeTab);
}

// ----------------------------------------------------------------------------
// > List view
// ----------------------------------------------------------------------------

//! Narrowest a column may be dragged, so a column cannot be lost altogether
#define FUI__LIST_COLUMN_MIN_WIDTH 24.0f
//! Width of the grabbable strip on a column divider. Wider than the line it sits on, because a one pixel
//! target is not a target
#define FUI__LIST_COLUMN_GRIP_WIDTH 8.0f
//! How much smaller than its cell a per-row button is drawn on each side, so it reads as a button IN the row
#define FUI__LIST_ROW_BUTTON_INSET 3.0f
//! Half the width of the little triangle marking the sorted column
#define FUI__LIST_SORT_ARROW_HALF_WIDTH 4.0f
//! Height of that triangle
#define FUI__LIST_SORT_ARROW_HEIGHT 5.0f
//! How far in from the right edge of the header cell the triangle sits
#define FUI__LIST_SORT_ARROW_RIGHT_INSET 8.0f

//! Compares two cells the way a person reads them: case insensitively, and any run of DIGITS as a NUMBER
//! rather than character by character. Without the second half "sound 10" sorts before "sound 9" and a volume
//! of "1.00" before "0.25" - the sort looking wrong for exactly the columns most worth sorting. A null cell
//! reads as an empty one, which puts the blanks together at one end. Answers below, equal to or above zero
fui_inline int32_t fui__CompareNatural(const char *left, const char *right) {
	if(left == fui_null) {
		left = "";
	}
	if(right == fui_null) {
		right = "";
	}
	while(*left != '\0' && *right != '\0') {
		bool leftIsDigit = (*left >= '0' && *left <= '9');
		bool rightIsDigit = (*right >= '0' && *right <= '9');
		if(leftIsDigit && rightIsDigit) {
			// Both sides start a number: past the leading zeros the LONGER run is the larger value, and only a
			// pair of equally long runs comes down to the digits themselves.
			while(*left == '0') {
				++left;
			}
			while(*right == '0') {
				++right;
			}
			const char *leftDigits = left;
			const char *rightDigits = right;
			while(*left >= '0' && *left <= '9') {
				++left;
			}
			while(*right >= '0' && *right <= '9') {
				++right;
			}
			ptrdiff_t leftDigitCount = left - leftDigits;
			ptrdiff_t rightDigitCount = right - rightDigits;
			if(leftDigitCount != rightDigitCount) {
				return((leftDigitCount < rightDigitCount) ? -1 : 1);
			}
			for(ptrdiff_t digitIndex = 0; digitIndex < leftDigitCount; ++digitIndex) {
				if(leftDigits[digitIndex] != rightDigits[digitIndex]) {
					return((leftDigits[digitIndex] < rightDigits[digitIndex]) ? -1 : 1);
				}
			}
			continue; // the two numbers read the same, so what follows them decides
		}
		char leftChar = *left;
		char rightChar = *right;
		if(leftChar >= 'A' && leftChar <= 'Z') {
			leftChar = (char)(leftChar - 'A' + 'a');
		}
		if(rightChar >= 'A' && rightChar <= 'Z') {
			rightChar = (char)(rightChar - 'A' + 'a');
		}
		if(leftChar != rightChar) {
			return((leftChar < rightChar) ? -1 : 1);
		}
		++left;
		++right;
	}
	if(*left == *right) {
		return(0);
	}
	return((*left == '\0') ? -1 : 1);
}

//! Rounds a row count up to the capacity an array is grown to, so a list that gains a row now and then does
//! not reallocate every time. The arena never gives an allocation back, and doubling is what keeps what is
//! left behind below the size of what is finally in use
fui_inline int32_t fui__ListSortCapacityFor(const int32_t rowCount) {
	const int32_t smallestUsefulCapacity = 256;
	int32_t capacity = smallestUsefulCapacity;
	while(capacity < rowCount) {
		if(capacity > (FUI_MAX_SORTABLE_ROWS / 2)) {
			capacity = rowCount;
			break;
		}
		capacity *= 2;
	}
	return(capacity);
}

//! Makes sure the shared scratch has room for one sort of this many rows
fui_inline bool fui__EnsureListSortScratch(fuiContext *context, const int32_t rowCount) {
	if(context->listSortScratch != fui_null && context->listSortScratchCapacity >= rowCount) {
		return(true);
	}
	int32_t capacity = fui__ListSortCapacityFor(rowCount);
	size_t byteCount = (size_t)capacity * sizeof(int32_t);
	int32_t *scratch = (int32_t *)fui__ArenaPushExact(&context->arena, byteCount, false);
	if(scratch == fui_null) {
		return(false);
	}
	context->listSortScratch = scratch;
	context->listSortScratchCapacity = capacity;
	return(true);
}

//! Makes sure this list's own order array has room for its rows
fui_inline bool fui__EnsureListSortOrder(fuiContext *context, fuiWidgetState *state, const int32_t rowCount) {
	if(state->sortOrder != fui_null && state->sortOrderCapacity >= rowCount) {
		return(true);
	}
	int32_t capacity = fui__ListSortCapacityFor(rowCount);
	size_t byteCount = (size_t)capacity * sizeof(int32_t);
	int32_t *order = (int32_t *)fui__ArenaPushExact(&context->arena, byteCount, false);
	if(order == fui_null) {
		return(false);
	}
	state->sortOrder = order;
	state->sortOrderCapacity = capacity;
	// A bigger array holds none of what the old one did, so whatever was cached in it is gone.
	state->sortOrderIsBuilt = false;
	return(true);
}

/*
	Hashes the column a list is sorted by, which is how a SHORT list notices its own cells being edited.

	The cache below is keyed on the address of the cell array and its shape, and a caller that rewrites a
	cell in place changes neither. For a list of a few thousand rows the whole sorted column can simply be
	hashed every frame - tens of microseconds - and then nothing has to be said at all. For a list of a
	million it cannot, and that is what fuiListViewInvalidateSort is for.
*/
fui_inline fuiId fui__ListSortFingerprint(const char *const *cells, const int32_t rowCount, const int32_t columnCount, const int32_t sortColumn) {
	fuiId hash = (fuiId)FUI__HASH_OFFSET_BASIS;
	for(int32_t row = 0; row < rowCount; ++row) {
		const char *cell = cells[row * columnCount + sortColumn];
		if(cell == fui_null) {
			continue;
		}
		size_t length = fui__StringLength(cell);
		hash = fui__HashBytes(hash, cell, length);
	}
	return(hash);
}

//! Whether the order this list has cached still describes the list it is being asked about
fui_inline bool fui__ListSortOrderIsStillGood(const fuiWidgetState *state, const char *const *cells, const int32_t rowCount, const int32_t columnCount, const bool canFingerprint, const fuiId fingerprint) {
	if(!state->sortOrderIsBuilt || state->sortOrder == fui_null) {
		return(false);
	}
	bool sameShape = (state->sortOrderRowCount == rowCount) && (state->sortOrderColumnCount == columnCount);
	bool sameSort = (state->sortOrderColumn == state->sortColumn) && (state->sortOrderIsAscending == state->sortIsAscending);
	if(!sameShape || !sameSort) {
		return(false);
	}
	if(canFingerprint && state->sortOrderWasFingerprinted) {
		// The hash covers the cells whatever array they came in, so the address does not have to agree too.
		return(state->sortOrderFingerprint == fingerprint);
	}
	bool sameCells = (state->sortOrderCells == (const void *)cells);
	return(sameCells);
}

/*
	Builds the DISPLAY ORDER of a list, one source row per display position, and answers null for a list
	that is not sorted at all - where a display position and a row are the same thing and no array is
	needed. The sort is a bottom up merge sort and STABLE on purpose, so rows reading the same in the
	sorted column keep the caller's own order instead of shuffling every frame.

	The order is CACHED. This used to run in full on every frame of every sorted list, which is n log n
	natural-order string comparisons to produce the identical answer it produced the frame before - by
	far the most expensive thing the library did, and the reason a list could not be allowed to be large.

	What the cache is keyed on is the shape of the list, the sort, and - up to FUI_LIST_SORT_VERIFY_ROWS
	rows - a hash of the sorted column itself, so a list of a few thousand rows notices its own cells being
	edited and needs nothing said. Past that the hash is dropped for the address of the cell array, and a
	caller that edits a cell IN PLACE has to say so with fuiListViewInvalidateSort. Rebuilding the array,
	or changing its length, is noticed either way.
*/
fui_inline const int32_t *fui__BuildListSortOrder(fuiContext *context, fuiWidgetState *state, const char *const *cells, const int32_t rowCount, const int32_t columnCount) {
	bool canSort = (state != fui_null) && state->sortIsActive && (state->sortColumn >= 0) && (state->sortColumn < columnCount);
	if(!canSort || rowCount <= 0 || rowCount > FUI_MAX_SORTABLE_ROWS) {
		return(fui_null);
	}
	bool canFingerprint = (rowCount <= (int32_t)FUI_LIST_SORT_VERIFY_ROWS);
	fuiId fingerprint = 0;
	if(canFingerprint) {
		fingerprint = fui__ListSortFingerprint(cells, rowCount, columnCount, state->sortColumn);
	}
	if(fui__ListSortOrderIsStillGood(state, cells, rowCount, columnCount, canFingerprint, fingerprint)) {
		return(state->sortOrder);
	}
	if(!fui__EnsureListSortOrder(context, state, rowCount)) {
		return(fui_null);
	}
	if(!fui__EnsureListSortScratch(context, rowCount)) {
		return(fui_null);
	}

	int32_t *order = state->sortOrder;
	int32_t *scratch = context->listSortScratch;
	for(int32_t row = 0; row < rowCount; ++row) {
		order[row] = row;
	}

	const int32_t sortColumn = state->sortColumn;
	const int32_t direction = state->sortIsAscending ? 1 : -1;
	for(int32_t runWidth = 1; runWidth < rowCount; runWidth *= 2) {
		for(int32_t runStart = 0; runStart < rowCount; runStart += runWidth * 2) {
			int32_t leftIndex = runStart;
			int32_t leftEnd = fuiMinI(runStart + runWidth, rowCount);
			int32_t rightIndex = leftEnd;
			int32_t rightEnd = fuiMinI(leftEnd + runWidth, rowCount);
			int32_t writeIndex = runStart;
			while(leftIndex < leftEnd && rightIndex < rightEnd) {
				const char *leftCell = cells[order[leftIndex] * columnCount + sortColumn];
				const char *rightCell = cells[order[rightIndex] * columnCount + sortColumn];
				int32_t comparison = fui__CompareNatural(leftCell, rightCell) * direction;
				// Taking the left row on a tie is what keeps it stable.
				scratch[writeIndex++] = (comparison <= 0) ? order[leftIndex++] : order[rightIndex++];
			}
			while(leftIndex < leftEnd) {
				scratch[writeIndex++] = order[leftIndex++];
			}
			while(rightIndex < rightEnd) {
				scratch[writeIndex++] = order[rightIndex++];
			}
		}
		for(int32_t row = 0; row < rowCount; ++row) {
			order[row] = scratch[row];
		}
	}

	state->sortOrderCells = (const void *)cells;
	state->sortOrderFingerprint = fingerprint;
	state->sortOrderWasFingerprinted = canFingerprint;
	state->sortOrderRowCount = rowCount;
	state->sortOrderColumnCount = columnCount;
	state->sortOrderColumn = state->sortColumn;
	state->sortOrderIsAscending = state->sortIsAscending;
	state->sortOrderIsBuilt = true;
	return(order);
}

//! Which source row a display position shows. A null order is an unsorted list, where they are the same
fui_inline int32_t fui__ListSourceRow(const int32_t *order, const int32_t displayIndex) {
	return((order != fui_null) ? order[displayIndex] : displayIndex);
}

//! The live width of one column: what the user dragged it to, or the caller's own default
fui_inline float fui__ListColumnWidth(const fuiWidgetState *state, const fuiColumn *columns, const int32_t columnIndex) {
	if(state != fui_null && columnIndex < state->columnWidthCount) {
		return(state->columnWidths[columnIndex]);
	}
	return(columns[columnIndex].width);
}

//! Seeds the remembered widths from the caller's defaults - the first time a list is built, and again
//! whenever its column COUNT changes, because a different set of columns is a different list and the widths
//! kept for the old one would land on the wrong titles
fui_inline void fui__ListSeedColumnWidths(fuiWidgetState *state, const fuiColumn *columns, const int32_t columnCount) {
	if(state == fui_null) {
		return;
	}
	int32_t trackedCount = fuiMinI(columnCount, (int32_t)FUI_MAX_LIST_COLUMNS);
	if(state->columnWidthCount == trackedCount) {
		return;
	}
	for(int32_t columnIndex = 0; columnIndex < trackedCount; ++columnIndex) {
		state->columnWidths[columnIndex] = columns[columnIndex].width;
	}
	state->columnWidthCount = trackedCount;
}

//! The little triangle marking the sorted column, pointing up for ascending. Drawn at the RIGHT of the header
//! cell, where it cannot be mistaken for part of the title
fui_inline void fui__ListDrawSortArrow(fuiContext *context, const fuiRect headerCell, const bool ascending, const fuiColor color) {
	float centerX = headerCell.x + headerCell.w - FUI__LIST_SORT_ARROW_RIGHT_INSET;
	float centerY = headerCell.y + headerCell.h * 0.5f;
	float halfHeight = FUI__LIST_SORT_ARROW_HEIGHT * 0.5f;
	fuiVec2 corners[3];
	if(ascending) {
		corners[0] = fuiV2(centerX, centerY - halfHeight);
		corners[1] = fuiV2(centerX + FUI__LIST_SORT_ARROW_HALF_WIDTH, centerY + halfHeight);
		corners[2] = fuiV2(centerX - FUI__LIST_SORT_ARROW_HALF_WIDTH, centerY + halfHeight);
	} else {
		corners[0] = fuiV2(centerX, centerY + halfHeight);
		corners[1] = fuiV2(centerX - FUI__LIST_SORT_ARROW_HALF_WIDTH, centerY - halfHeight);
		corners[2] = fuiV2(centerX + FUI__LIST_SORT_ARROW_HALF_WIDTH, centerY - halfHeight);
	}
	fuiDrawPolygon(context, corners, 3, color);
}

//! The ONE place a list's sort is written, so the setters below cannot drift apart
fui_inline void fui__ListApplySort(fuiWidgetState *state, const int32_t sortColumn, const bool ascending) {
	state->sortIsActive = (sortColumn >= 0); // negative puts the caller's own row order back
	state->sortColumn = (sortColumn >= 0) ? sortColumn : 0;
	state->sortIsAscending = ascending;
}

fui_api void fuiListViewSetSort(fuiContext *context, const char *id, const int32_t sortColumn, const bool ascending) {
	FUI_ASSERT(context != fui_null && id != fui_null);
	if(context == fui_null || id == fui_null) {
		return;
	}
	fuiId listId = fuiGetId(context, id);
	fuiWidgetState *state = fui__WidgetStateGet(context, listId);
	if(state == fui_null) {
		return;
	}
	fui__ListApplySort(state, sortColumn, ascending);
	// Saying it outright counts as the starting sort having happened: a seed landing on top of a deliberate
	// act a frame later would undo it, which is the wrong way round.
	state->sortDefaultWasSeeded = true;
}

fui_api void fuiListViewSetSortDefault(fuiContext *context, const char *id, const int32_t sortColumn, const bool ascending) {
	FUI_ASSERT(context != fui_null && id != fui_null);
	if(context == fui_null || id == fui_null) {
		return;
	}
	fuiId listId = fuiGetId(context, id);
	fuiWidgetState *state = fui__WidgetStateGet(context, listId);
	if(state == fui_null || state->sortDefaultWasSeeded) {
		return;
	}
	fui__ListApplySort(state, sortColumn, ascending);
	state->sortDefaultWasSeeded = true;
}

fui_api void fuiListViewSetSortable(fuiContext *context, const char *id, const bool sortable) {
	FUI_ASSERT(context != fui_null && id != fui_null);
	if(context == fui_null || id == fui_null) {
		return;
	}
	fuiId listId = fuiGetId(context, id);
	fuiWidgetState *state = fui__WidgetStateGet(context, listId);
	if(state == fui_null) {
		return;
	}
	state->sortIsLocked = !sortable;
}

fui_api void fuiListViewInvalidateSort(fuiContext *context, const char *id) {
	FUI_ASSERT(context != fui_null && id != fui_null);
	if(context == fui_null || id == fui_null) {
		return;
	}
	fuiId listId = fuiGetId(context, id);
	fuiWidgetState *state = fui__WidgetStateGet(context, listId);
	if(state == fui_null) {
		return;
	}
	// The array itself is kept. It is the right size already, and it is the CONTENT of it that is stale.
	state->sortOrderIsBuilt = false;
}

fui_api bool fuiListViewButtons(fuiContext *context, const fuiRect rect, const char *id, const fuiColumn *columns, const int32_t columnCount, const char *const *cells, const int32_t rowCount, int32_t *selectedIndex, const fuiListIcons *icons, fuiListRowButtons *rowButtons, bool *outWasActivated) {
	FUI_ASSERT(context != fui_null && id != fui_null && selectedIndex != fui_null);
	if(context == fui_null || id == fui_null || selectedIndex == fui_null) {
		return(false);
	}
	if(outWasActivated != fui_null) {
		*outWasActivated = false;
	}
	if(rowButtons != fui_null) {
		rowButtons->clickedRow = -1;
	}
	if(columns == fui_null || columnCount <= 0) {
		return(false);
	}
	if(cells == fui_null && rowCount > 0) {
		return(false);
	}

	const fuiTheme *theme = &context->theme;
	fuiId listId = fuiGetId(context, id);
	fuiWidgetState *state = fui__WidgetStateGet(context, listId);
	bool hasRowButtons = (rowButtons != fui_null) && (rowButtons->labelForRow != fui_null) && (rowButtons->column >= 0) && (rowButtons->column < columnCount);
	fui__ListSeedColumnWidths(state, columns, columnCount);

	fuiDrawRect(context, rect, theme->widgetTrackColor);
	fuiDrawRectOutline(context, rect, theme->panelBorderColor, theme->widgetBorderThickness);

	// A row carrying an ICON is taller, so the glyph has somewhere to be - by the caller's own factor, because
	// "as tall as a file browser's row" is far too much for a list of hundreds. The HEADER keeps the plain
	// text height whatever the rows do.
	bool hasIcons = (icons != fui_null) && (icons->sheet != 0) && (icons->cellForRow != fui_null);
	float rowHeight = theme->menuItemHeight;
	if(hasIcons) {
		float iconRowScale = (icons->rowScale > 0.0f) ? icons->rowScale : FUI__LIST_ICON_ROW_SCALE;
		rowHeight *= iconRowScale;
	}
	float headerHeight = theme->menuItemHeight;

	float totalColumnsWidth = 0.0f;
	for(int32_t columnIndex = 0; columnIndex < columnCount; ++columnIndex) {
		totalColumnsWidth += fui__ListColumnWidth(state, columns, columnIndex);
	}
	float rowsHeight = (float)rowCount * rowHeight;

	// The vertical gutter is ALWAYS reserved and its thumb simply disabled when the rows fit, so the rows do
	// not change width the moment one row is added. The horizontal bar is the other way round - it appears
	// only when the columns really are wider than the box, because an empty strip along the bottom of every
	// list would be a permanent cost for a rare case.
	float gutterWidth = fuiScrollGutterWidth();
	float contentWidth = rect.w - gutterWidth;
	bool needsHorizontalBar = totalColumnsWidth > contentWidth;
	float bodyHeight = fuiMaxF(rect.h - headerHeight - (needsHorizontalBar ? gutterWidth : 0.0f), 0.0f);

	fuiRect headerRect = fuiRectMake(rect.x, rect.y, contentWidth, headerHeight);
	fuiRect bodyRect = fuiRectMake(rect.x, rect.y + headerHeight, contentWidth, bodyHeight);

	float maxScrollX = fuiMaxF(totalColumnsWidth - contentWidth, 0.0f);
	float scrollY = (state != fui_null) ? state->scroll : 0.0f;
	float scrollX = (state != fui_null) ? state->scrollX : 0.0f;

	// The wheel over the body scrolls it down, and with shift held, sideways.
	bool cursorIsOverBody = fui__ClaimCursor(context, listId, bodyRect);
	if(cursorIsOverBody && context->mouseWheelDelta != 0.0f) {
		float wheelDistance = context->mouseWheelDelta * rowHeight * FUI__SCROLL_WHEEL_ROWS;
		if(fuiIsShiftDown(context)) {
			scrollX -= wheelDistance;
		} else {
			scrollY -= wheelDistance;
		}
	}

	// Both bars are resolved and drawn BEFORE the rows, so the rows are laid out from the final offsets rather
	// than from ones the wheel is about to change.
	fuiPushId(context, id);
	fuiRect verticalTrack = fuiRectMake(rect.x + rect.w - gutterWidth, bodyRect.y, gutterWidth, bodyHeight);
	scrollY = fui__Scrollbar(context, verticalTrack, "__listScrollbarY", scrollY, bodyHeight, rowsHeight, true);
	if(needsHorizontalBar) {
		fuiRect horizontalTrack = fuiRectMake(rect.x, bodyRect.y + bodyHeight, contentWidth, gutterWidth);
		scrollX = fui__Scrollbar(context, horizontalTrack, "__listScrollbarX", scrollX, contentWidth, totalColumnsWidth, false);
	} else {
		scrollX = fuiClampF(scrollX, 0.0f, maxScrollX);
	}
	fuiPopId(context);
	if(state != fui_null) {
		state->scroll = scrollY;
		state->scrollX = scrollX;
	}

	// The header: titles laid out left to right, moved by the sideways scroll, clipped to the header so no
	// title spills into the scrollbar gutter.
	fuiDrawRect(context, headerRect, theme->widgetColor);
	fuiPushClip(context, headerRect);
	float columnLeft = headerRect.x - scrollX;
	// Every header cell, kept for the sort click below. That test runs AFTER this loop so that every divider
	// has already had its chance at the press: a grip straddles the line between two columns, and deciding
	// "title or divider" while only half the grips exist would answer wrongly near an edge.
	fuiRect headerCellRects[FUI_MAX_LIST_COLUMNS];
	int32_t headerCellCount = 0;
	for(int32_t columnIndex = 0; columnIndex < columnCount; ++columnIndex) {
		float columnWidth = fui__ListColumnWidth(state, columns, columnIndex);
		fuiRect cellRect = fuiRectMake(columnLeft, headerRect.y, columnWidth, headerRect.h);
		if(headerCellCount < (int32_t)FUI_MAX_LIST_COLUMNS) {
			headerCellRects[headerCellCount++] = cellRect;
		}

		fuiPushClip(context, cellRect);
		bool isTheSortedColumn = (state != fui_null) && state->sortIsActive && (state->sortColumn == columnIndex);
		fui__DrawTextInRect(context, cellRect, columns[columnIndex].title, isTheSortedColumn ? theme->accentColor : theme->textColor);
		if(isTheSortedColumn) {
			fui__ListDrawSortArrow(context, cellRect, state->sortIsAscending, theme->accentColor);
		}
		fuiPopClip(context);

		columnLeft += columnWidth;
		fuiVec2 dividerTop = fuiV2(columnLeft, headerRect.y);
		fuiVec2 dividerBottom = fuiV2(columnLeft, headerRect.y + headerRect.h);
		fuiDrawLine(context, dividerTop, dividerBottom, theme->panelBorderColor, theme->widgetBorderThickness);

		// That divider is also a GRAB HANDLE that resizes the column to its left. Only a tracked column can be
		// dragged, since there is nowhere to remember an untracked one's width.
		bool isTrackedColumn = (state != fui_null) && (columnIndex < state->columnWidthCount);
		if(isTrackedColumn) {
			fuiRect grip = fuiRectMake(columnLeft - FUI__LIST_COLUMN_GRIP_WIDTH * 0.5f, headerRect.y, FUI__LIST_COLUMN_GRIP_WIDTH, headerRect.h);
			fuiPushId(context, id);
			fuiPushIdInt(context, columnIndex);
			fuiId gripId = fuiGetId(context, "__columnGrip");
			fuiPopId(context);
			fuiPopId(context);
			fuiInteraction interaction = fuiInteract(context, gripId, grip);
			if(interaction.isHeld) {
				float draggedWidth = state->columnWidths[columnIndex] + context->mouseDelta.x;
				state->columnWidths[columnIndex] = fuiMaxF(draggedWidth, FUI__LIST_COLUMN_MIN_WIDTH);
			}
			if(interaction.isHovered || interaction.isHeld) {
				fuiDrawRect(context, grip, theme->accentColor);
			}
		}
	}
	fuiPopClip(context);

	// Clicking a header TITLE sorts by that column, and clicking the one already sorted turns it round. The
	// press has to be one NOTHING has taken yet, which is what tells a title click from a divider drag - the
	// grip above claims the press it takes, and this must not act on that same one.
	bool listCanBeSorted = (state != fui_null) && !state->sortIsLocked;
	bool pressIsFree = context->mouseWentDown[FUI_MOUSE_LEFT] && !context->mouseDownConsumed[FUI_MOUSE_LEFT];
	if(listCanBeSorted && pressIsFree && fui__CursorIsOver(context, headerRect)) {
		for(int32_t columnIndex = 0; columnIndex < headerCellCount; ++columnIndex) {
			if(!fuiPointInRect(context->mousePosition, headerCellRects[columnIndex])) {
				continue;
			}
			if(state->sortIsActive && state->sortColumn == columnIndex) {
				state->sortIsAscending = !state->sortIsAscending;
			} else {
				state->sortIsActive = true;
				state->sortColumn = columnIndex;
				state->sortIsAscending = true; // a fresh column always starts the way a list is read
			}
			context->mouseDownConsumed[FUI_MOUSE_LEFT] = true;
			break;
		}
	}

	// The display order, resolved once for the whole body: it maps a display POSITION to the caller's own row
	// index, and is null for a list nobody has sorted, where the two are the same thing.
	const int32_t *sortOrder = fui__BuildListSortOrder(context, state, cells, rowCount, columnCount);

	bool selectionChanged = false;
	fuiPushClip(context, bodyRect);
	int32_t firstVisibleRow = 0;
	int32_t endVisibleRow = rowCount;
	fui__ListVisibleRange(scrollY, bodyRect.h, rowHeight, rowCount, &firstVisibleRow, &endVisibleRow);
	for(int32_t displayIndex = firstVisibleRow; displayIndex < endVisibleRow; ++displayIndex) {
		const int32_t row = fui__ListSourceRow(sortOrder, displayIndex);
		float rowTop = bodyRect.y - scrollY + (float)displayIndex * rowHeight;
		bool isAboveTheBody = (rowTop + rowHeight) < bodyRect.y;
		bool isBelowTheBody = rowTop > (bodyRect.y + bodyRect.h);
		if(isAboveTheBody || isBelowTheBody) {
			continue;
		}

		fuiRect rowRect = fuiRectMake(bodyRect.x, rowTop, contentWidth, rowHeight);
		bool rowIsHovered = fui__CursorIsOver(context, bodyRect) && fui__CursorIsOver(context, rowRect);
		bool rowIsSelected = (*selectedIndex == row);
		if(rowIsSelected) {
			fuiDrawRect(context, rowRect, theme->menuHighlightColor);
		} else if(rowIsHovered) {
			fuiDrawRect(context, rowRect, theme->widgetHoveredColor);
		}

		// The cells, moved by the sideways scroll and each clipped to its column. An icon goes in the FIRST
		// column ahead of its text, the same place a list box puts it.
		float cellLeft = rowRect.x - scrollX;
		bool rowHasButton = false;
		fuiRect rowButtonRect = fuiRectMake(0.0f, 0.0f, 0.0f, 0.0f);
		for(int32_t columnIndex = 0; columnIndex < columnCount; ++columnIndex) {
			float columnWidth = fui__ListColumnWidth(state, columns, columnIndex);
			fuiRect cellRect = fuiRectMake(cellLeft, rowRect.y, columnWidth, rowRect.h);
			cellLeft += columnWidth;
			fuiPushClip(context, cellRect);

			// A BUTTON column draws a button in place of its text, on the rows that have a label for one.
			if(hasRowButtons && columnIndex == rowButtons->column) {
				const char *buttonLabel = rowButtons->labelForRow[row];
				bool hasLabel = (buttonLabel != fui_null) && (buttonLabel[0] != '\0');
				if(hasLabel) {
					fuiRect buttonRect = fuiRectMake(cellRect.x + FUI__LIST_ROW_BUTTON_INSET, cellRect.y + FUI__LIST_ROW_BUTTON_INSET, cellRect.w - FUI__LIST_ROW_BUTTON_INSET * 2.0f, cellRect.h - FUI__LIST_ROW_BUTTON_INSET * 2.0f);
					// Scoped by the list AND the row, so every row's button is a widget of its own - they share
					// a label, and a widget's identifier is hashed from that label.
					fuiPushId(context, id);
					fuiPushIdInt(context, row);
					if(fuiButton(context, buttonRect, buttonLabel)) {
						rowButtons->clickedRow = row;
					}
					fuiPopId(context);
					fuiPopId(context);
					rowHasButton = true;
					rowButtonRect = buttonRect;
				}
				fuiPopClip(context);
				continue; // the cell's own text is what the button replaced
			}

			fuiRect textRect = cellRect;
			if(hasIcons && columnIndex == 0) {
				textRect = fui__ListBoxDrawRowIcon(context, cellRect, rowHeight, icons, row);
			}
			fui__DrawTextInRect(context, textRect, cells[row * columnCount + columnIndex], theme->textColor);
			fuiPopClip(context);
		}

		// A press that landed on the row's button belongs to that button alone: without this, auditioning a row
		// would also pick it, and a second press on the button would "open" it.
		bool pressTakenByButton = rowHasButton && fuiPointInRect(context->mousePosition, rowButtonRect);
		bool rowTakesThePress = rowIsHovered && !pressTakenByButton && context->mouseWentDown[FUI_MOUSE_LEFT] && !context->mouseDownConsumed[FUI_MOUSE_LEFT];
		if(rowTakesThePress) {
			bool isSecondClick = false;
			if(state != fui_null) {
				float secondsSinceLastClick = context->timeSeconds - state->lastClickTime;
				isSecondClick = (state->lastClickIndex == row) && (secondsSinceLastClick <= FUI__DOUBLE_CLICK_SECONDS);
			}
			if(*selectedIndex != row) {
				*selectedIndex = row;
				selectionChanged = true;
			}
			if(isSecondClick && outWasActivated != fui_null) {
				*outWasActivated = true;
			}
			if(state != fui_null) {
				state->lastClickTime = context->timeSeconds;
				state->lastClickIndex = row;
			}
			context->mouseDownConsumed[FUI_MOUSE_LEFT] = true;
			context->focused = listId;
		}
	}
	fuiPopClip(context);
	return(selectionChanged);
}

fui_api bool fuiListViewEx(fuiContext *context, const fuiRect rect, const char *id, const fuiColumn *columns, const int32_t columnCount, const char *const *cells, const int32_t rowCount, int32_t *selectedIndex, const fuiListIcons *icons, bool *outWasActivated) {
	fuiListRowButtons *noRowButtons = fui_null;
	return(fuiListViewButtons(context, rect, id, columns, columnCount, cells, rowCount, selectedIndex, icons, noRowButtons, outWasActivated));
}

fui_api bool fuiListView(fuiContext *context, const fuiRect rect, const char *id, const fuiColumn *columns, const int32_t columnCount, const char *const *cells, const int32_t rowCount, int32_t *selectedIndex, bool *outWasActivated) {
	const fuiListIcons *noIcons = fui_null;
	return(fuiListViewEx(context, rect, id, columns, columnCount, cells, rowCount, selectedIndex, noIcons, outWasActivated));
}

#ifdef __cplusplus
}
#endif

#endif // FUI_IMPLEMENTATION
