/***
final_ui_texteditor.h

-------------------------------------------------------------------------------
	About
-------------------------------------------------------------------------------

A code and text editor widget for final_ui.h, as a single header C99 add-on.

It is what final_ui.h's own multiline text field is not: a document with a line
index behind it, so line ten thousand costs the same as line ten; a style byte
per text byte, so code can be coloured by a lexer and a diff by whole lines; a
gutter with line numbers; tab stops; whitespace made visible; overwrite mode;
find and replace; undo; and a document that remembers which encoding and which
line ending it arrived with.

The library owns the document. That is the one place this add-on departs from
final_ui.h's rule that the caller owns everything: an editor keeps a gap buffer,
a line index and a style array that only it may maintain, and handing those to
the caller would hand over invariants rather than data. Everything ELSE - the
colours, the metrics, the callbacks, the shortcuts - is a config struct the
caller fills in, and passing none is allowed.

Status: under construction. The view is there and can be read, scrolled, selected from, copied out
of and coloured by a lexer; typing into it, find and replace and undo are not in yet. See the
changelog for what is.

-------------------------------------------------------------------------------
	Getting started
-------------------------------------------------------------------------------

- Include final_ui.h BEFORE this file. It is a hard prerequisite.
- Define FUI_TEXTEDITOR_IMPLEMENTATION in exactly ONE translation unit before including this file.
- Create one fuiEditor per document with fuiEditorInit(), pass fui_null as allocator to use malloc/free.
- Fill it with fuiEditorSetText() or fuiEditorLoadFromMemory().
- Draw it once a frame with fuiTextEditor(), which is where everything about the view is remembered.
- Read the caret and the selection back with fuiEditorGetCaretOffset() and fuiEditorCopySelection().
- Colour it by handing fuiEditorSetLexer() a callback that colours ONE line, and fuiEditorSetDecorations()
  the arrays for everything that needs no history - a diff, an error marker, a search hit.
- Change what it looks like with fuiEditorSetConfig(), or pass none and take fuiEditorDefaultConfig().
- Release it with fuiEditorRelease().

This file uses only the PUBLIC api of final_ui.h, so its implementation may live
in a translation unit of its own.

-------------------------------------------------------------------------------
	Usage
-------------------------------------------------------------------------------

#define FUI_IMPLEMENTATION
#include <final_ui.h>

#define FUI_TEXTEDITOR_IMPLEMENTATION
#include <final_ui_texteditor.h>

// Once
fuiEditor editor;
fuiEditorInit(&editor, fui_null);            // fui_null = default malloc/free
fuiEditorSetText(&editor, sourceCode, 0);    // 0 = measure up to the terminating zero

// Per frame. The CONTEXT's font is what the text is drawn in, so a monospace face is swapped in around it
fuiRect editorRect = fuiLayoutRemaining(&ui);
fuiSetFont(&ui, &monospaceFont);
fuiEditorAction action = fuiTextEditor(&ui, editorRect, "source", &editor);
fuiSetFont(&ui, &interfaceFont);

// Once
fuiEditorRelease(&editor);

-------------------------------------------------------------------------------
	Preprocessor overrides
-------------------------------------------------------------------------------

FUI_TEXTEDITOR_IMPLEMENTATION   Define in ONE translation unit to emit the implementation.
FUI_TEXTEDITOR_ASSERT(expr)     Override the assertion macro (defaults to FUI_ASSERT from final_ui.h).
FUI_TEXTEDITOR_MEMSET(d,v,n)    Override memory set (defaults to memset).
FUI_TEXTEDITOR_MEMCPY(d,s,n)    Override memory copy (defaults to memcpy).
FUI_TEXTEDITOR_MEMMOVE(d,s,n)   Override memory move (defaults to memmove).
FUI_TEXTEDITOR_MEMCHR(p,v,n)    Override memory byte search (defaults to memchr).
FUI_TEXTEDITOR_MEMCMP(a,b,n)    Override memory compare (defaults to memcmp).
FUI_TEXTEDITOR_STRLEN(s)        Override string length (defaults to strlen).
FUI_TEXTEDITOR_MIN_TEXT_BYTES   Smallest byte capacity a document is ever allocated at (default 4096).
FUI_TEXTEDITOR_MIN_LINE_SLOTS   Smallest number of line slots the line index is ever allocated at (default 256).
FUI_TEXTEDITOR_MIN_GAP_BYTES    How much room an insert leaves behind for the next one (default 1024).
FUI_TEXTEDITOR_MIN_GAP_SLOTS    How many line slots an insert leaves behind for the next one (default 64).
FUI_TEXTEDITOR_MAX_LEX_LINES_PER_FRAME  How many lines one build may colour before leaving the rest for the next (default 50000).

-------------------------------------------------------------------------------
	License
-------------------------------------------------------------------------------

Final UI Text Editor is released under the following license:

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
	@file final_ui_texteditor.h
	@version v0.4.0
	@author Torsten Spaete
	@brief Final UI Text Editor - A code and text editor widget add-on for final_ui.h.
*/

// ****************************************************************************
//
// > Changelog
//
// ****************************************************************************

/*!
	@page page_texteditor_changelog Changelog
	@tableofcontents

	# v0.4.0:
	Colour, in two layers, because two very different things are meant by it. And whitespace made visible,
	which is the other thing a code editor is looked at for.

	- New: A LEXER, asked one line at a time and told the state that line starts in - fuiEditorSetLexer,
	  fuiEditorLexer, fuiEditorLexLine, fuiEditorLexRequest and fuiEditorStyleDef. Whether line five
	  thousand is inside a block comment is only knowable to somebody who has seen line four thousand nine
	  hundred and ninety nine, so the state a line ends in is kept, one int32 per line.
	- New: Those states live in the SAME slots as the line starts do, in the same split array with the same
	  hole. That is what keeps a line's state attached to the line through every edit: an insert in the
	  middle pushes the states behind it along with their lines, and one number moves rather than all of them.
	- New: Colouring stops the moment it can. A recomputed state that comes out equal to the one already
	  stored says that everything behind it was worked out from an unchanged start and is still right - so
	  a change at the top of a fourteen thousand line file costs TWO calls into the lexer rather than
	  fourteen thousand. There is a test that counts them.
	- New: fuiEditorInvalidateStyles, for a lexer whose answer changed without the text changing.
	- New: DECORATIONS - fuiEditorSetDecorations, fuiEditorLineDecoration and fuiEditorRangeDecoration.
	  A whole line gets a wash and a marker in the gutter, a stretch inside one gets a wash. They carry no
	  state at all, so they are simply arrays the caller owns and keeps sorted, and the visible window is
	  found in them by binary search rather than by walking them.
	- New: fuiEditorConfig.toggles.showWhitespace draws a dot in every blank and an arrow across the full
	  width of every tab - which is what says how far a tab really reached. And .showLineEndings writes LF
	  or CRLF after every line, which is what makes a file with mixed endings show itself.
	- New: A line is cut into runs wherever what it is drawn WITH changes, and each run is measured as a
	  PREFIX of the piece it belongs to rather than on its own, so the widths telescope back to exactly
	  what the whole piece measures. Without that a coloured line and the caret on it would drift apart by
	  one kerning pair per style boundary.
	- New: FUI_TEXTEDITOR_MAX_LEX_LINES_PER_FRAME. A file opened and jumped straight to the end of has to
	  be walked once; doing that in a single frame is a stall, so it is spread over as many as it takes and
	  what has not been reached yet is drawn plain. Set high enough that an ordinary file never notices.
	- Changed: fuiEditorLineIndex carries a second array. A document costs one more int32 per line, whether
	  it has a lexer or not - four megabytes on a million line file, beside the four the line starts
	  already take. Paying it always is what keeps every gap operation free of a null check.

	# v0.3.0:
	A caret in it. The view from v0.2.0 can now be moved through, selected from and copied out of - by the
	keyboard, by the mouse, and by the caller - and it still writes nothing back. That is the whole of the
	read-only editor: what is left after this is the part that changes the text.

	- New: A caret and a selection, both of them the EDITOR's rather than the context's. final_ui.h keeps
	  one caret for every text field there is, which is right for fields that are only ever typed into one
	  at a time and wrong for two editors side by side. fuiEditorGetCaretOffset, fuiEditorSetCaretOffset,
	  fuiEditorGetCaretColumn, fuiEditorSetSelection, fuiEditorSelectAll, fuiEditorClearSelection,
	  fuiEditorHasSelection, fuiEditorGetSelectionStart, fuiEditorGetSelectionEnd and fuiEditorCopySelection.
	- New: The keyboard. Arrows, home and end, page up and down, ctrl with the arrows for whole words, ctrl
	  with home and end for the whole document, shift with any of them to drag the selection along, ctrl+a
	  and ctrl+c. All of it over fuiKeyRepeat, so holding a key repeats at the same rate everything else in
	  the library does.
	- New: The caret remembers the column it WANTS. Walked off the end of a long line, down across a short
	  one and on, it comes back out where it started - a caret that only remembered where it landed would
	  be stuck at the short line's width from there on.
	- New: The mouse. Click, drag, shift and click to reach out, double click for a word, triple click for
	  a whole line, and a drag that began on a word or a line stays on whole ones. A drag that runs off the
	  top or the bottom keeps scrolling by itself, faster the further out it is.
	- New: The caret is brought back into view when it MOVED and at no other time. Doing it unconditionally
	  is the classic way to nail a document down: the wheel scrolls away from the caret, the next frame
	  drags it back, and the wheel looks broken. There is a test for exactly that.
	- New: One walk over a line answers every question asked about it - where to draw it, how wide it is,
	  how far into it an offset sits, which offset a distance lands on. They come out of the same pieces,
	  which is what keeps the caret standing where the glyphs really are, tabs and all.
	- New: The caret's own line is measured before the layout, so an arrow key walking into a long line can
	  scroll sideways to it even though that line has never been on screen.
	- New: fuiEditorConfig.colors.selectionBackground and .caret, .metrics.caretWidth, and
	  .toggles.isInteractive - which turns the keyboard and the mouse off and leaves a view that is only read.
	- New: The editor is in the tab chain, through fuiRegisterFocusable.
	- Note: ctrl+c hands the WHOLE selection to fuiSetClipboardText, in one allocation of exactly its size,
	  so nothing is ever cut off - least of all in the middle of a codepoint. What happens to it after that
	  is the platform hook's business, and a hook with a size limit is worth writing carefully: FPL's X11
	  backend copies into a buffer of FPL_MAX_BUFFER_LENGTH bytes through fplCopyString, which writes
	  NOTHING at all when the text does not fit and then takes the selection ownership anyway - so a big
	  copy through it leaves the system clipboard EMPTY rather than shortened, and reports success. A hook
	  that cannot take the whole text should refuse it and answer false. fuiEditorCopySelection is always
	  the way to get all of it inside the process.
	- Changed: final_ui.h v0.9.7 also carries fuiRegisterFocusable and fuiGetFrameTime now, which this needs.

	# v0.2.0:
	Something to look at. The document from v0.1.0 gets a widget over it that can be READ - a gutter with
	line numbers, tab stops, two scrollbars and a status line - and nothing that can be typed into yet.
	final_ui.h itself, at over fourteen thousand lines, is what it is built against, because a line index
	is only worth having on something that size.

	- New: fuiTextEditor, which lays out and draws only the lines that can be SEEN. Nothing in it is per
	  document line and everything is per visible line, which is the whole reason a document of a million
	  lines costs what one of twenty costs. The one exception is the caret's line, which is a binary search.
	- New: fuiEditorConfig with its colors, metrics and toggles, fuiEditorDefaultConfig and
	  fuiEditorSetConfig. A zeroed field means "take the default", the same as everywhere else in
	  final_ui.h - but it is resolved ONCE when the configuration is set rather than at every use, because
	  forty tests per frame to arrive at the same forty answers is not a bargain. The theme it was resolved
	  from is kept beside it, so a context that is restyled between two frames is noticed.
	- New: A gutter whose line numbers are right aligned and NOT padded out with blanks or zeroes, so that
	  a jump in the numbers reads as a jump rather than as a change of width. It is sized by the widest
	  number the document can show, and it takes the current line wash with it - a highlight that stopped at
	  the number would read as two things beside each other rather than as one line.
	- New: Tab stops. fuiDrawText has no idea what a tab is, so a line is cut at every one of them and the
	  pen is put on the next stop, counted from the start of the line rather than from the left edge.
	- New: A monospace fast path. The face is measured once per build - "W" against "i" - and where they
	  come out the same width, measuring a run becomes counting its codepoints and multiplying. That is the
	  difference between a long line of code costing what its length is and costing the square of it.
	- New: Both scrollbars, each Auto, Always or Never. The vertical one is reserved whether it is needed or
	  not by default, because a document being typed into crosses the "one line more than fits" boundary
	  constantly and every crossing would shift every line sideways; the horizontal one appears only when it
	  is needed. The wheel scrolls down and, with shift, sideways - the same gesture the list view uses.
	- New: The editor's OWN status line, rather than fuiBeginStatusBar, which docks against the bottom of
	  the window. An editor is rarely the whole window.
	- New: fuiEditorSetCaretLine, fuiEditorGetCaretLine, fuiEditorScrollToLine, fuiEditorGetFirstVisibleLine
	  and fuiEditorGetVisibleLineCount. The caret is a line and nothing more so far; moving it by the mouse
	  and the keyboard is the next iteration.
	- New: SCREEN lines and DOCUMENT lines are told apart everywhere, although they are still the same
	  thing. Optional word wrap makes them differ, and introducing that distinction afterwards would mean
	  touching every line of the widget at once.
	- New: The horizontal range goes by the widest line SEEN so far, not by the widest line there is.
	  Measuring every line every frame would be the walk over the whole document this widget exists not to
	  do. So the range grows as the document is scrolled through, and an edit resets it.
	- Changed: final_ui.h v0.9.7 is now the minimum, for fuiScrollbarHorizontal.

	# v0.1.0:
	The foundation, and nothing that can be seen yet. What a real editor is or is not able to do is
	decided by how it stores its text, so that is what this first version is: a document, and the two
	indices that make it cheap to ask anything about it.

	- New: fuiEditor, a document the CALLER holds by value. Every other final_ui.h widget keeps what it
	  remembers in the shared fuiWidgetState, which is one flat struct that every widget pays for. An
	  editor brings a gap buffer, a line index, a style array and an undo stack, and it is also the one
	  widget that must not share final_ui.h's single context wide caret - two editors beside each other
	  each keep their own.
	- New: A GAP BUFFER as the text store. The document is one allocation with a hole in it that sits
	  where the last edit was, so typing writes into the hole and costs nothing, and moving somewhere
	  else costs one move of the hole rather than one move per character.
	- New: A line index that is a SPLIT array of its own, with a shared offset for everything behind its
	  hole. A plain array of line starts would have to add the edit's byte difference to every entry
	  behind it - a million additions per keystroke in a million line file. Moving the hole and changing
	  one number does the same job, so only lines that really appeared or vanished move at all.
	- New: fuiEditorEncoding, the seam every encoding goes through, with fuiEditorEncodingUtf8 and
	  fuiEditorEncodingAscii behind it. The document itself is ALWAYS utf-8; an encoding is asked to
	  convert only when text is loaded and when it is saved. Everything in between - measuring, drawing,
	  moving the caret, searching - would otherwise have to go through a codec, and for utf-7, whose
	  meaning depends on what came before it, "byte number n" would stop meaning anything at all.
	- New: fuiEditorInit, fuiEditorRelease, fuiEditorSetText, fuiEditorLoadFromMemory, fuiEditorInsert,
	  fuiEditorErase, fuiEditorGetByte, fuiEditorCopyText, fuiEditorGetContiguousText, and the line
	  queries fuiEditorGetLineCount, fuiEditorGetLineStart, fuiEditorGetLineEnd, fuiEditorGetLineLength,
	  fuiEditorCopyLine and fuiEditorGetLineOfOffset.
	- New: A line is ended by a LINE FEED and by nothing else, so a carriage return before it belongs to
	  the line it ends. Which ending the text arrived with is remembered as fuiEditorEol and reported by
	  fuiEditorGetEol, because that is what saving has to put back.
*/

#ifndef FUI_TEXTEDITOR_INCLUDE_H
#define FUI_TEXTEDITOR_INCLUDE_H

#if !defined(FUI_INCLUDE_H)
#	error "final_ui.h must be included before final_ui_texteditor.h"
#endif

//
// Version
//

//! Version of this add-on, so an application can report which build it was compiled against
#define FUI_TEXTEDITOR_VERSION_MAJOR 0
#define FUI_TEXTEDITOR_VERSION_MINOR 4
#define FUI_TEXTEDITOR_VERSION_PATCH 0

//! Full version as a string literal, in the form of "major.minor.patch"
#define FUI_TEXTEDITOR_VERSION_STRING FUI__STRINGIFY(FUI_TEXTEDITOR_VERSION_MAJOR) "." FUI__STRINGIFY(FUI_TEXTEDITOR_VERSION_MINOR) "." FUI__STRINGIFY(FUI_TEXTEDITOR_VERSION_PATCH)

//! Returns the null-terminated version string of this add-on, in the form of "major.minor.patch"
fui_api const char *fuiEditorGetVersion(void);

//
// Tunables
//

#if !defined(FUI_TEXTEDITOR_ASSERT)
	//! Assertion macro - defaults to whatever final_ui.h asserts with
#	define FUI_TEXTEDITOR_ASSERT(expression) FUI_ASSERT(expression)
#endif

#if !defined(FUI_TEXTEDITOR_MIN_TEXT_BYTES)
	//! Smallest byte capacity a document is ever allocated at
#	define FUI_TEXTEDITOR_MIN_TEXT_BYTES 4096
#endif

#if !defined(FUI_TEXTEDITOR_MIN_LINE_SLOTS)
	//! Smallest number of line slots the line index is ever allocated at
#	define FUI_TEXTEDITOR_MIN_LINE_SLOTS 256
#endif

#if !defined(FUI_TEXTEDITOR_MIN_GAP_BYTES)
	//! How much room an insert leaves behind, so a run of typing reallocates once rather than once a character
#	define FUI_TEXTEDITOR_MIN_GAP_BYTES 1024
#endif

#if !defined(FUI_TEXTEDITOR_MIN_GAP_SLOTS)
	//! How many line slots an insert leaves behind, for the same reason
#	define FUI_TEXTEDITOR_MIN_GAP_SLOTS 64
#endif

#if !defined(FUI_TEXTEDITOR_MAX_LEX_LINES_PER_FRAME)
	//! How many lines one build may colour before it leaves the rest for the next one. A file that is
	//! opened and jumped straight to the end of has to be walked once, and doing that in a single frame
	//! is a stall - so it is spread over as many frames as it takes, showing plain text until it arrives
#	define FUI_TEXTEDITOR_MAX_LEX_LINES_PER_FRAME 50000
#endif

// ****************************************************************************
//
// > Encoding
//
// ****************************************************************************

/**
* @enum fuiEditorEol
* @brief Which line ending a text arrived with, which is what saving it has to put back.
* @note A LINE FEED and nothing else ends a line inside the document, so a carriage return in front of
*       one is part of the line it ends. A text of carriage returns alone is one single line here, and
*       is turned into line feeds when it is loaded rather than being understood as it stands.
*/
typedef enum fuiEditorEol {
	//! Line feed alone, the unix ending, and what the document always holds internally
	fuiEditorEol_Lf = 0,
	//! Carriage return followed by a line feed, the windows ending
	fuiEditorEol_CrLf,
	//! Carriage return alone, the classic macintosh ending
	fuiEditorEol_Cr,
	//! More than one of the above in the same text
	fuiEditorEol_Mixed,
} fuiEditorEol;

/**
* @brief Returns the display name of a line ending, for showing it in a status bar.
* @param[in] eol The line ending @ref fuiEditorEol to name.
* @return Returns a static string such as "CRLF", never null.
*/
fui_api const char *fuiEditorEolGetName(const fuiEditorEol eol);

/**
* @brief Returns the bytes a line ending is written as.
* @param[in] eol The line ending @ref fuiEditorEol to spell out.
* @param[out] outLength Receives how many bytes it is, one or two.
* @return Returns a static string, never null. @ref fuiEditorEol_Mixed answers as @ref fuiEditorEol_Lf.
*/
fui_api const char *fuiEditorEolGetBytes(const fuiEditorEol eol, int32_t *outLength);

/**
* @struct fuiEditorEncoding
* @brief How bytes on the outside become the utf-8 the document is made of, and how they become bytes again.
* @note The document is ALWAYS utf-8. An encoding runs when text is loaded and when it is saved, and at
*       no other moment - which is why measuring, drawing and moving the caret never have to know one.
* @note Both converters follow the same sizing rule: when destination is null, or destinationCapacity is
*       too small, they write NOTHING and still return how many bytes the whole result would take. So a
*       caller asks once with no buffer, allocates, and asks again.
*/
typedef struct fuiEditorEncoding {
	//! What this encoding is called, for a status bar and for a menu
	const char *name;
	//! How many bytes of byte order mark sit at the front of the data, zero when there is none
	int32_t (*getBomLength)(void *userData, const uint8_t *data, const int32_t dataLength);
	//! Converts data into utf-8, returning how many bytes the result takes
	int32_t (*toUtf8)(void *userData, const uint8_t *source, const int32_t sourceLength, char *destination, const int32_t destinationCapacity);
	//! Converts utf-8 back into data, returning how many bytes the result takes
	int32_t (*fromUtf8)(void *userData, const char *source, const int32_t sourceLength, uint8_t *destination, const int32_t destinationCapacity);
	//! Passed back to every callback above
	void *userData;
} fuiEditorEncoding;

/**
* @brief Returns the utf-8 encoding, which is a validating copy in both directions.
* @return Returns the encoding @ref fuiEditorEncoding.
* @note A malformed sequence becomes the replacement character rather than being carried through, so a
*       document is always well formed utf-8 however badly the file that filled it was written.
*/
fui_api fuiEditorEncoding fuiEditorEncodingUtf8(void);

/**
* @brief Returns the 7 bit ascii encoding.
* @return Returns the encoding @ref fuiEditorEncoding.
* @note A byte with its top bit set becomes the replacement character on the way in, and a codepoint
*       above 127 becomes a question mark on the way out.
*/
fui_api fuiEditorEncoding fuiEditorEncodingAscii(void);

// ****************************************************************************
//
// > Configuration
//
// ****************************************************************************

/**
* @enum fuiEditorScrollbarMode
* @brief When one of the two scrollbars is there at all.
* @note Reserving a gutter that is not needed costs a strip of width or height; NOT reserving one costs
*       every line moving sideways the moment the document grows past the box. Which of those is worse
*       depends on the axis, so each axis is asked separately.
*/
typedef enum fuiEditorScrollbarMode {
	//! Reserve the gutter and draw the bar only while the content really overflows
	fuiEditorScrollbarMode_Auto = 0,
	//! Always reserve the gutter, and draw the bar disabled while the content fits
	fuiEditorScrollbarMode_Always,
	//! Never reserve a gutter and never draw a bar. The wheel still scrolls
	fuiEditorScrollbarMode_Never,
} fuiEditorScrollbarMode;

/**
* @struct fuiEditorColors
* @brief What the editor paints with. Every color left at zero alpha is taken from @ref fuiTheme instead.
* @note A color that is meant to be invisible is switched off by its toggle rather than by a zero alpha,
*       because zero alpha is what "the caller named none" is spelled as.
*/
typedef struct fuiEditorColors {
	//! Fill behind the text
	fuiColor background;
	//! Outline around the whole widget
	fuiColor border;
	//! The text itself, until a lexer has something else to say about a run of it
	fuiColor text;
	//! Fill behind the line numbers
	fuiColor gutterBackground;
	//! A line number that is not the current one
	fuiColor gutterText;
	//! The line number of the line the caret is on
	fuiColor gutterCurrentLineText;
	//! The hairline that divides the gutter from the text
	fuiColor gutterSeparator;
	//! Wash over the whole width of the line the caret is on, the gutter included
	fuiColor currentLineBackground;
	//! Wash behind selected text, translucent so the glyphs on top of it stay readable
	fuiColor selectionBackground;
	//! The caret itself
	fuiColor caret;
	//! The dots, arrows and line ending marks that make whitespace visible. Faint on purpose - they are
	//! there to be checked, not to be read
	fuiColor whitespace;
	//! Fill behind the editor's own status line
	fuiColor statusBarBackground;
	//! Text of the editor's own status line
	fuiColor statusBarText;
} fuiEditorColors;

/**
* @struct fuiEditorMetrics
* @brief Every measurement the editor lays itself out by. Zero takes the default named on each field.
*/
typedef struct fuiEditorMetrics {
	//! Pixel height the text is drawn at. Zero takes @ref fuiTheme.fontHeight
	float fontHeight;
	//! Extra pixels between one line and the next, on top of the font's own line height. Zero is none
	float lineSpacing;
	//! Inset of the text from the left edge of its area. Zero takes @ref fuiTheme.widgetPaddingX
	float textPaddingX;
	//! Inset of a line number from both edges of the gutter. Zero takes @ref fuiTheme.widgetPaddingX
	float gutterPaddingX;
	//! Height of the editor's own status line. Zero takes @ref fuiTheme.menuItemHeight
	float statusBarHeight;
	//! How wide the caret is drawn. Zero is two pixels
	float caretWidth;
	//! How many characters wide one tab stop is. Zero is four
	int32_t tabSize;
	//! How many digits the gutter is wide even when the document is shorter than that. Zero is three
	int32_t gutterMinDigits;
} fuiEditorMetrics;

/**
* @struct fuiEditorToggles
* @brief What the editor shows and what it leaves out. A zeroed one is the plainest editor there is.
*/
typedef struct fuiEditorToggles {
	//! Draw the gutter and the line numbers in it
	bool showLineNumbers;
	//! Draw the editor's own status line along the bottom of its rectangle
	bool showStatusBar;
	//! Wash the line the caret is on, across the gutter as well as the text
	bool highlightCurrentLine;
	//! Let the keyboard and the mouse move the caret and the selection, and put the editor in the tab chain
	bool isInteractive;
	//! Draw a dot in every blank and an arrow across every tab
	bool showWhitespace;
	//! Write CR, LF or CRLF at the end of every line, which is what tells a mixed file apart from a clean one
	bool showLineEndings;
	//! When the vertical scrollbar is there @ref fuiEditorScrollbarMode
	fuiEditorScrollbarMode verticalScrollbar;
	//! When the horizontal scrollbar is there @ref fuiEditorScrollbarMode
	fuiEditorScrollbarMode horizontalScrollbar;
} fuiEditorToggles;

/**
* @struct fuiEditorConfig
* @brief Everything about an editor that is the CALLER's rather than the document's.
* @note Handed to @ref fuiEditorSetConfig, which COPIES it and resolves every zeroed field once. This is
*       the one place the add-on departs from final_ui.h's "zero means the default, worked out where it is
*       used" rule: a description of some sixty fields would be sixty tests per frame instead of one.
*/
typedef struct fuiEditorConfig {
	//! What it paints with
	fuiEditorColors colors;
	//! What it measures itself by
	fuiEditorMetrics metrics;
	//! What it shows and leaves out
	fuiEditorToggles toggles;
} fuiEditorConfig;

// ****************************************************************************
//
// > Colouring
//
// ****************************************************************************

/*
	Two layers, because two very different things are meant by "colour this".

	A LEXER carries state: whether line five thousand sits inside a block comment is only knowable to
	somebody who has seen line four thousand nine hundred and ninety nine. So it is asked line by line,
	from the last line whose state is known up to the one being drawn, and what it answers is kept.

	A DECORATION carries none: a diff, an error marker, a search hit all know their answer without any
	history at all. So they are simply handed over as arrays, and the caller owns them.
*/

//! How many styles a lexer may hand out, which is what fits in the one byte per character it writes
#define FUI_TEXTEDITOR_MAX_STYLES 256

//! The style a lexer means when it says nothing, drawn in the editor's plain text colour
#define FUI_TEXTEDITOR_STYLE_DEFAULT 0

/**
* @struct fuiEditorStyleDef
* @brief One entry of a lexer's style table, which is what a style byte is looked up in.
*/
typedef struct fuiEditorStyleDef {
	//! What text of this style is drawn in. A zero alpha takes the editor's plain text colour
	fuiColor color;
} fuiEditorStyleDef;

/**
* @struct fuiEditorLexRequest
* @brief One line handed to a lexer, and the two things it writes back.
* @note The text is a COPY, so it is contiguous however the document happens to be laid out - and it is
*       NOT null terminated, because a line that ends the document has nothing after it to terminate with.
*/
typedef struct fuiEditorLexRequest {
	//! The line's bytes, without its line ending
	const char *text;
	//! How many bytes there are
	int32_t textLength;
	//! Which document line this is, counted from zero
	int32_t lineIndex;
	//! The state this line starts in, which is what the line before it answered
	int32_t startState;
	//! OUT: one style byte per byte of text, already cleared to @ref FUI_TEXTEDITOR_STYLE_DEFAULT
	uint8_t *styles;
	//! Whatever was hung on the lexer
	void *userData;
} fuiEditorLexRequest;

/**
* @brief Colours one line and answers the state the NEXT line starts in.
* @param[in,out] request Reference to the line @ref fuiEditorLexRequest.
* @return Returns the parser state the following line begins in. Zero is as good a state as any other.
* @note The same line and the same start state must always give the same answer. The whole incremental
*       scheme rests on that: a state that comes out equal to the one already stored is what says that
*       everything behind it is still right and does not have to be looked at again.
*/
typedef int32_t (*fuiEditorLexLine)(fuiEditorLexRequest *request);

/**
* @struct fuiEditorLexer
* @brief A lexer and the styles it hands out, owned by the caller.
*/
typedef struct fuiEditorLexer {
	//! Colours one line. Null is no lexer at all
	fuiEditorLexLine lexLine;
	//! The style table, indexed by the style bytes lexLine writes
	const fuiEditorStyleDef *styles;
	//! How many entries the table has
	int32_t styleCount;
	//! Passed back to lexLine
	void *userData;
} fuiEditorLexer;

/**
* @struct fuiEditorLineDecoration
* @brief What a decoration says about one whole line.
*/
typedef struct fuiEditorLineDecoration {
	//! Which document line, counted from zero
	int32_t line;
	//! Wash across the whole line, gutter included. A zero alpha draws none
	fuiColor background;
	//! Fill of the marker drawn at the left edge of the gutter. A zero alpha draws none
	fuiColor gutterMarker;
} fuiEditorLineDecoration;

/**
* @struct fuiEditorRangeDecoration
* @brief What a decoration says about a stretch of text that is not a whole line.
*/
typedef struct fuiEditorRangeDecoration {
	//! First byte it covers
	int32_t startOffset;
	//! One past the last byte it covers
	int32_t endOffset;
	//! Wash behind those bytes. A zero alpha draws none
	fuiColor background;
} fuiEditorRangeDecoration;

/**
* @struct fuiEditorDecorations
* @brief The colouring that needs no history, handed over as arrays the CALLER owns.
* @note Both arrays must be sorted - the lines by their line, the ranges by their startOffset - because
*       the visible window is found in them by binary search. A diff over a large file is one entry per
*       changed line, and walking all of them once a frame is the cost this add-on exists to avoid.
*/
typedef struct fuiEditorDecorations {
	//! One entry per decorated line, sorted by line. Null for none
	const fuiEditorLineDecoration *lines;
	//! How many there are
	int32_t lineCount;
	//! One entry per decorated stretch, sorted by startOffset. Null for none
	const fuiEditorRangeDecoration *ranges;
	//! How many there are
	int32_t rangeCount;
} fuiEditorDecorations;

// ****************************************************************************
//
// > Document
//
// ****************************************************************************

/**
* @struct fuiEditorLineIndex
* @brief Where every line of the document begins, held as a split array with a hole in it.
* @note Entry i is the byte offset the i-th line starts at, and entry zero is always zero. Entries in
*       front of the hole are stored as they are; entries behind it are stored SHORT by tailDelta, so an
*       edit that made the document longer or shorter changes one number instead of every entry behind it.
* @note Internal. Read it through @ref fuiEditorGetLineStart and its neighbours rather than by hand.
*/
typedef struct fuiEditorLineIndex {
	//! The slots, capacity of them, with a hole between gapStart and gapEnd
	int32_t *starts;
	//! What state a lexer STARTS this line in, one per entry and in the same slots as the starts. Only
	//! entry zero is true by definition; the rest is worked out and tracked by a watermark on the editor
	int32_t *lexerStates;
	//! How many slots are allocated in total, holes included
	int32_t capacity;
	//! How many entries sit in front of the hole
	int32_t gapStart;
	//! Which slot the entries behind the hole begin at
	int32_t gapEnd;
	//! What every entry behind the hole is short by
	int32_t tailDelta;
} fuiEditorLineIndex;

/**
* @struct fuiEditorDocument
* @brief The text itself, as one allocation with a hole where the last edit happened.
* @note Internal. The bytes are NOT contiguous while the hole sits in the middle of them, so reach them
*       through @ref fuiEditorGetByte, @ref fuiEditorCopyText or @ref fuiEditorGetContiguousText.
*/
typedef struct fuiEditorDocument {
	//! The bytes, capacity of them, with a hole between gapStart and gapEnd
	char *bytes;
	//! How many bytes are allocated in total, the hole included
	int32_t capacity;
	//! Where the hole begins, which is also the document offset the hole sits at
	int32_t gapStart;
	//! The byte after the hole
	int32_t gapEnd;
	//! Where every line begins
	fuiEditorLineIndex lines;
} fuiEditorDocument;

/**
* @struct fuiEditor
* @brief One document, held BY VALUE by the caller and passed to every call below.
* @note Zero it, hand it to @ref fuiEditorInit, and hand it to @ref fuiEditorRelease when it is done.
*       Two editors beside each other are two of these and share nothing.
*/
typedef struct fuiEditor {
	//! Where the document's memory comes from, resolved to malloc/free when the caller passed none
	fuiAllocator allocator;
	//! The text and its line index
	fuiEditorDocument document;
	//! Which encoding the text was loaded from, which is what saving it converts back to
	fuiEditorEncoding encoding;
	//! Which line ending the text arrived with, which is what saving it writes
	fuiEditorEol eol;
	//! Bumped by every change to the text, so anything worked out from the document can tell that it went stale
	int32_t version;

	//! The lexer, as the caller gave it. Zeroed means no colouring at all
	fuiEditorLexer lexer;
	//! The decorations, as the caller gave them
	fuiEditorDecorations decorations;
	//! How many lines have a parser state that is believed. States [0, styledUpToLine) are good
	int32_t styledUpToLine;
	//! No re-colouring may stop before it is past this line, because the lines up to here are NEW and
	//! whatever their state slots happen to hold was never written by a lexer
	int32_t lexConvergenceFloor;
	//! One line's bytes, copied out so a lexer gets them in one piece however the hole happens to sit
	char *lineScratch;
	//! One style byte per byte of that line
	uint8_t *styleScratch;
	//! How much room both of them have
	int32_t scratchCapacity;

	//! What the widget draws with, as the CALLER gave it - a zeroed field still means "take the default"
	fuiEditorConfig config;
	//! The same configuration with every zero already filled in, which is what a build actually reads
	fuiEditorConfig resolvedConfig;
	//! The theme resolvedConfig was filled in from, so a restyled context is noticed rather than ignored
	fuiTheme resolvedTheme;
	//! Whether resolvedConfig has been filled in at all yet
	bool hasResolvedConfig;

	//! How far the view is scrolled sideways, in pixels
	float scrollX;
	//! How far the view is scrolled down, in pixels
	float scrollY;
	//! The widest line MEASURED so far, in pixels, which is what the horizontal bar has to go by
	float widestMeasuredLineWidth;
	//! Which document version that width was measured against, so an edit throws it away
	int32_t widestMeasuredVersion;
	//! Where the caret sits, as a byte offset into the document
	int32_t caretOffset;
	//! Where the selection was started, as a byte offset. Equal to the caret when nothing is selected
	int32_t selectionAnchor;
	//! How far into its line the caret WANTS to be while it is being moved up and down, in pixels
	float desiredDistance;
	//! Whether that wish is standing, which every sideways move drops
	bool hasDesiredDistance;
	//! How long the caret has been in its current blink phase
	float caretBlinkTime;
	//! How long ago the last press was, which is what makes the next one a double click
	float timeSinceLastPress;
	//! Where that press was
	fuiVec2 lastPressPosition;
	//! One for a click, two for a double click, three for a triple one
	int32_t pressCount;
	//! The range the press established, which a drag extends away from rather than collapsing
	int32_t dragAnchorStart;
	//! The other end of it
	int32_t dragAnchorEnd;
	//! Whether the editor is the one holding the press
	bool isDraggingSelection;
	//! Which screen line a fuiEditorScrollToLine is waiting to put at the top
	int32_t pendingScrollScreenLine;
	//! Whether there is such a request waiting at all
	bool hasPendingScroll;
	//! Which screen line the last build had at the top, so the caller can ask about the view
	int32_t firstVisibleScreenLine;
	//! How many screen lines the last build had room for
	int32_t visibleScreenLineCount;

	//! Set once an allocation was refused, and never cleared until the editor is released
	bool hasOutOfMemory;
	//! Whether @ref fuiEditorInit has run on this editor
	bool isInitialized;
} fuiEditor;

/**
* @brief Prepares an editor and gives it an empty document of one empty line.
* @param[out] editor Reference to the editor @ref fuiEditor to prepare.
* @param[in] allocator Reference to the allocator @ref fuiAllocator to take memory from, or null for malloc/free.
* @return Returns true when the editor is ready to use.
*/
fui_api bool fuiEditorInit(fuiEditor *editor, const fuiAllocator *allocator);

/**
* @brief Releases everything an editor holds and leaves it zeroed.
* @param[in,out] editor Reference to the editor @ref fuiEditor to release.
*/
fui_api void fuiEditorRelease(fuiEditor *editor);

/**
* @brief Replaces the whole document with utf-8 text.
* @param[in,out] editor Reference to the editor @ref fuiEditor.
* @param[in] text The utf-8 text to fill it with, or null to empty it.
* @param[in] textLength Length of the text in bytes, pass 0 to measure up to the terminating zero.
* @return Returns true when the text fit.
* @note Carriage returns are kept as they are, and @ref fuiEditorGetEol reports which endings were seen.
*       This is the raw entry point - use @ref fuiEditorLoadFromMemory for text that is not already utf-8.
*/
fui_api bool fuiEditorSetText(fuiEditor *editor, const char *text, const int32_t textLength);

/**
* @brief Replaces the whole document with text in some encoding, converting it to utf-8 on the way in.
* @param[in,out] editor Reference to the editor @ref fuiEditor.
* @param[in] data The bytes to load, byte order mark included when there is one.
* @param[in] dataLength Length of the data in bytes.
* @param[in] encoding Reference to the encoding @ref fuiEditorEncoding to read it with, or null for utf-8.
* @return Returns true when the text was converted and fit.
* @note The encoding is remembered on the editor, so saving converts back to what was loaded.
*/
fui_api bool fuiEditorLoadFromMemory(fuiEditor *editor, const uint8_t *data, const int32_t dataLength, const fuiEditorEncoding *encoding);

/**
* @brief Returns how many bytes the document holds.
* @param[in] editor Reference to the editor @ref fuiEditor.
* @return Returns the length in bytes, which is zero for an empty document.
*/
fui_api int32_t fuiEditorGetTextLength(const fuiEditor *editor);

/**
* @brief Reads one byte of the document.
* @param[in] editor Reference to the editor @ref fuiEditor.
* @param[in] offset The byte offset to read at.
* @return Returns the byte, or zero when the offset is outside the document.
*/
fui_api char fuiEditorGetByte(const fuiEditor *editor, const int32_t offset);

/**
* @brief Copies a range of the document out.
* @param[in] editor Reference to the editor @ref fuiEditor.
* @param[in] offset The byte offset to start at.
* @param[in] byteCount How many bytes to copy, clamped to what is left.
* @param[out] destination Receives the bytes, zero terminated, or null to only ask for the length.
* @param[in] destinationCapacity Size of the destination in bytes, the terminating zero included.
* @return Returns how many bytes the range holds, whether or not it fit.
*/
fui_api int32_t fuiEditorCopyRange(const fuiEditor *editor, const int32_t offset, const int32_t byteCount, char *destination, const int32_t destinationCapacity);

/**
* @brief Copies the whole document out.
* @param[in] editor Reference to the editor @ref fuiEditor.
* @param[out] destination Receives the bytes, zero terminated, or null to only ask for the length.
* @param[in] destinationCapacity Size of the destination in bytes, the terminating zero included.
* @return Returns how many bytes the document holds, whether or not it fit.
*/
fui_api int32_t fuiEditorCopyText(const fuiEditor *editor, char *destination, const int32_t destinationCapacity);

/**
* @brief Moves the hole to the end so the whole document is one contiguous, zero terminated string.
* @param[in,out] editor Reference to the editor @ref fuiEditor.
* @return Returns a pointer into the editor's own memory, valid until the next edit, never null.
* @note This is what a lexer and a search are handed. It costs ONE move of the hole, and leaves it at
*       the end - so doing it every frame on a document nobody is editing costs nothing after the first.
*/
fui_api const char *fuiEditorGetContiguousText(fuiEditor *editor);

/**
* @brief Inserts utf-8 text into the document.
* @param[in,out] editor Reference to the editor @ref fuiEditor.
* @param[in] offset The byte offset to insert at, clamped into the document.
* @param[in] text The utf-8 text to insert.
* @param[in] textLength Length of the text in bytes, pass 0 to measure up to the terminating zero.
* @return Returns true when the text was inserted.
*/
fui_api bool fuiEditorInsert(fuiEditor *editor, const int32_t offset, const char *text, const int32_t textLength);

/**
* @brief Erases a range of the document.
* @param[in,out] editor Reference to the editor @ref fuiEditor.
* @param[in] offset The byte offset to erase at, clamped into the document.
* @param[in] byteCount How many bytes to erase, clamped to what is left.
* @return Returns true when anything was erased.
*/
fui_api bool fuiEditorErase(fuiEditor *editor, const int32_t offset, const int32_t byteCount);

/**
* @brief Returns how many lines the document has.
* @param[in] editor Reference to the editor @ref fuiEditor.
* @return Returns the line count, which is at least one even for an empty document.
* @note A document ending in a line feed has an empty last line, the way every editor shows one.
*/
fui_api int32_t fuiEditorGetLineCount(const fuiEditor *editor);

/**
* @brief Returns the byte offset a line begins at.
* @param[in] editor Reference to the editor @ref fuiEditor.
* @param[in] lineIndex Which line, clamped into the document.
* @return Returns the offset of the line's first byte.
*/
fui_api int32_t fuiEditorGetLineStart(const fuiEditor *editor, const int32_t lineIndex);

/**
* @brief Returns the byte offset a line ends at, the line ending EXCLUDED.
* @param[in] editor Reference to the editor @ref fuiEditor.
* @param[in] lineIndex Which line, clamped into the document.
* @return Returns the offset one past the line's last visible byte.
*/
fui_api int32_t fuiEditorGetLineEnd(const fuiEditor *editor, const int32_t lineIndex);

/**
* @brief Returns how many bytes a line holds, the line ending EXCLUDED.
* @param[in] editor Reference to the editor @ref fuiEditor.
* @param[in] lineIndex Which line, clamped into the document.
* @return Returns the length in bytes.
*/
fui_api int32_t fuiEditorGetLineLength(const fuiEditor *editor, const int32_t lineIndex);

/**
* @brief Copies one line out, the line ending EXCLUDED.
* @param[in] editor Reference to the editor @ref fuiEditor.
* @param[in] lineIndex Which line, clamped into the document.
* @param[out] destination Receives the bytes, zero terminated, or null to only ask for the length.
* @param[in] destinationCapacity Size of the destination in bytes, the terminating zero included.
* @return Returns how many bytes the line holds, whether or not it fit.
*/
fui_api int32_t fuiEditorCopyLine(const fuiEditor *editor, const int32_t lineIndex, char *destination, const int32_t destinationCapacity);

/**
* @brief Returns which line a byte offset falls on.
* @param[in] editor Reference to the editor @ref fuiEditor.
* @param[in] offset The byte offset to look up, clamped into the document.
* @return Returns the line index.
* @note A binary search over the line index, so it costs the logarithm of the line count and not a scan.
*/
fui_api int32_t fuiEditorGetLineOfOffset(const fuiEditor *editor, const int32_t offset);

/**
* @brief Returns which line ending the document arrived with.
* @param[in] editor Reference to the editor @ref fuiEditor.
* @return Returns the line ending @ref fuiEditorEol.
*/
fui_api fuiEditorEol fuiEditorGetEol(const fuiEditor *editor);

/**
* @brief Sets which line ending the document is to be saved with.
* @param[in,out] editor Reference to the editor @ref fuiEditor.
* @param[in] eol The line ending @ref fuiEditorEol to write.
*/
fui_api void fuiEditorSetEol(fuiEditor *editor, const fuiEditorEol eol);

/**
* @brief Steps forward to the start of the next codepoint.
* @param[in] editor Reference to the editor @ref fuiEditor.
* @param[in] offset The byte offset to step from.
* @return Returns the offset of the next codepoint's first byte, or the document length at the end.
*/
fui_api int32_t fuiEditorNextCodepointOffset(const fuiEditor *editor, const int32_t offset);

/**
* @brief Steps back to the start of the previous codepoint.
* @param[in] editor Reference to the editor @ref fuiEditor.
* @param[in] offset The byte offset to step from.
* @return Returns the offset of the previous codepoint's first byte, or zero at the front.
*/
fui_api int32_t fuiEditorPreviousCodepointOffset(const fuiEditor *editor, const int32_t offset);

/**
* @brief Pulls an offset back onto the start of the codepoint it lands inside of.
* @param[in] editor Reference to the editor @ref fuiEditor.
* @param[in] offset The byte offset to snap.
* @return Returns the offset of the enclosing codepoint's first byte.
*/
fui_api int32_t fuiEditorSnapToCodepointStart(const fuiEditor *editor, const int32_t offset);

// ****************************************************************************
//
// > Widget
//
// ****************************************************************************

/**
* @brief Installs a lexer, or takes the current one away.
* @param[in,out] editor Reference to the editor @ref fuiEditor.
* @param[in] lexer Reference to the lexer @ref fuiEditorLexer, or null for none. The caller owns it and everything it points at.
* @note Everything already coloured is thrown away, so swapping lexers costs one full re-colouring.
*/
fui_api void fuiEditorSetLexer(fuiEditor *editor, const fuiEditorLexer *lexer);

/**
* @brief Throws away what has been coloured from a line onwards, so it is worked out again.
* @param[in,out] editor Reference to the editor @ref fuiEditor.
* @param[in] documentLine The first line to doubt, counted from zero.
* @note Every edit does this by itself. It is here for a lexer whose ANSWER changed without the text
*       changing - a keyword list that grew, a preprocessor define that came in from somewhere else.
*/
fui_api void fuiEditorInvalidateStyles(fuiEditor *editor, const int32_t documentLine);

/**
* @brief Installs the decorations, or takes them away.
* @param[in,out] editor Reference to the editor @ref fuiEditor.
* @param[in] decorations Reference to the decorations @ref fuiEditorDecorations, or null for none.
* @note Only the POINTERS are kept. The arrays stay the caller's, and must outlive the editor's next build.
*/
fui_api void fuiEditorSetDecorations(fuiEditor *editor, const fuiEditorDecorations *decorations);

/**
* @struct fuiEditorAction
* @brief What one build of the editor came to. Read it straight after the call, like a return value.
*/
typedef struct fuiEditorAction {
	//! OUT: The editor has the keyboard
	bool isFocused;
	//! OUT: The caret or the selection moved during this build
	bool didMoveCaret;
	//! OUT: The selection was put on the clipboard during this build
	bool didCopy;
	//! OUT: The document was changed by this build
	bool didChange;
} fuiEditorAction;

/**
* @brief Returns the configuration an editor starts life with.
* @return Returns the configuration @ref fuiEditorConfig.
* @note Line numbers, a status line and a current line wash are on; everything else is zero, which is to
*       say every color comes from the theme and every measurement from the theme or from its own default.
*/
fui_api fuiEditorConfig fuiEditorDefaultConfig(void);

/**
* @brief Replaces what an editor draws with.
* @param[in,out] editor Reference to the editor @ref fuiEditor.
* @param[in] config Reference to the configuration @ref fuiEditorConfig, or null to go back to @ref fuiEditorDefaultConfig.
* @note The configuration is copied. The zeroed fields in it are resolved against the theme on the next
*       build rather than here, because a theme belongs to a context and a document does not.
*/
fui_api void fuiEditorSetConfig(fuiEditor *editor, const fuiEditorConfig *config);

/**
* @brief Returns the configuration as the caller gave it, zeros and all.
* @param[in] editor Reference to the editor @ref fuiEditor.
* @return Returns the configuration @ref fuiEditorConfig, or null when there is no editor.
*/
fui_api const fuiEditorConfig *fuiEditorGetConfig(const fuiEditor *editor);

/**
* @brief Draws one editor and lets it be scrolled.
* @param[in,out] context Reference to the context @ref fuiContext.
* @param[in] rect The box the editor sits in, in pixels, its own status line included.
* @param[in] id Identifies the editor's scrollbars across frames.
* @param[in,out] editor Reference to the editor @ref fuiEditor, which is where everything is remembered.
* @return Returns what this build came to @ref fuiEditorAction.
* @note Only the lines that can be seen are laid out and drawn, so a document of a million lines costs
*       what one of twenty costs.
* @note The font the CONTEXT carries is what the text is drawn in. Swap in a monospace face around the
*       call, the way final_ui.h's own widgets are given a face, and swap the old one back afterwards.
*/
fui_api fuiEditorAction fuiTextEditor(fuiContext *context, const fuiRect rect, const char *id, fuiEditor *editor);

/**
* @brief Returns where the caret sits, as a byte offset into the document.
* @param[in] editor Reference to the editor @ref fuiEditor.
* @return Returns the byte offset, always on a codepoint boundary.
*/
fui_api int32_t fuiEditorGetCaretOffset(const fuiEditor *editor);

/**
* @brief Puts the caret somewhere, optionally dragging the selection along with it.
* @param[in,out] editor Reference to the editor @ref fuiEditor.
* @param[in] offset The byte offset, clamped to the document and pulled back onto a codepoint boundary.
* @param[in] extendSelection Keep the anchor where it is, which is what shift and a drag do. False drops the selection.
*/
fui_api void fuiEditorSetCaretOffset(fuiEditor *editor, const int32_t offset, const bool extendSelection);

/**
* @brief Returns which document line the caret sits on.
* @param[in] editor Reference to the editor @ref fuiEditor.
* @return Returns the zero based document line index.
*/
fui_api int32_t fuiEditorGetCaretLine(const fuiEditor *editor);

/**
* @brief Returns how many codepoints of its line stand in front of the caret.
* @param[in] editor Reference to the editor @ref fuiEditor.
* @return Returns the zero based column.
* @note Counted in CODEPOINTS, not in the columns a tab spans - the two differ the moment a line is
*       indented, and which of them a status line should say is the caller's taste rather than this file's.
*/
fui_api int32_t fuiEditorGetCaretColumn(const fuiEditor *editor);

/**
* @brief Puts the caret at the start of a document line and drops the selection.
* @param[in,out] editor Reference to the editor @ref fuiEditor.
* @param[in] documentLine The zero based document line, clamped to what there is.
*/
fui_api void fuiEditorSetCaretLine(fuiEditor *editor, const int32_t documentLine);

/**
* @brief Selects a range, leaving the caret at its second end.
* @param[in,out] editor Reference to the editor @ref fuiEditor.
* @param[in] anchorOffset Where the selection is held down, in bytes.
* @param[in] caretOffset Where it is dragged to, in bytes. The two may be in either order.
*/
fui_api void fuiEditorSetSelection(fuiEditor *editor, const int32_t anchorOffset, const int32_t caretOffset);

/**
* @brief Selects the whole document.
* @param[in,out] editor Reference to the editor @ref fuiEditor.
*/
fui_api void fuiEditorSelectAll(fuiEditor *editor);

/**
* @brief Drops the selection, leaving the caret where it is.
* @param[in,out] editor Reference to the editor @ref fuiEditor.
*/
fui_api void fuiEditorClearSelection(fuiEditor *editor);

/**
* @brief Tests whether anything is selected.
* @param[in] editor Reference to the editor @ref fuiEditor.
* @return Returns true when the selection covers at least one byte.
*/
fui_api bool fuiEditorHasSelection(const fuiEditor *editor);

/**
* @brief Returns the lower end of the selection.
* @param[in] editor Reference to the editor @ref fuiEditor.
* @return Returns the byte offset, which is the caret's own offset when nothing is selected.
*/
fui_api int32_t fuiEditorGetSelectionStart(const fuiEditor *editor);

/**
* @brief Returns the upper end of the selection.
* @param[in] editor Reference to the editor @ref fuiEditor.
* @return Returns the byte offset, which is the caret's own offset when nothing is selected.
*/
fui_api int32_t fuiEditorGetSelectionEnd(const fuiEditor *editor);

/**
* @brief Copies the selected text out.
* @param[in] editor Reference to the editor @ref fuiEditor.
* @param[out] destination Where to write it, or null to ask only how long it is.
* @param[in] destinationCapacity How much room there is, terminator included.
* @return Returns the FULL length in bytes, whether it fitted or not.
* @note Ask once with null, allocate, ask again - the same rule every copying call in this file follows.
*/
fui_api int32_t fuiEditorCopySelection(const fuiEditor *editor, char *destination, const int32_t destinationCapacity);

/**
* @brief Scrolls a document line to the top of the view.
* @param[in,out] editor Reference to the editor @ref fuiEditor.
* @param[in] documentLine The zero based document line, clamped to what there is.
* @note Takes effect on the NEXT build, which is the first moment that knows how tall a line is - that
*       comes from the font the context carries, and a document knows nothing about one.
*/
fui_api void fuiEditorScrollToLine(fuiEditor *editor, const int32_t documentLine);

/**
* @brief Returns which document line was the topmost one the last time the editor was built.
* @param[in] editor Reference to the editor @ref fuiEditor.
* @return Returns the zero based document line index, and zero before the first build.
*/
fui_api int32_t fuiEditorGetFirstVisibleLine(const fuiEditor *editor);

/**
* @brief Returns how many lines fitted in the view the last time the editor was built.
* @param[in] editor Reference to the editor @ref fuiEditor.
* @return Returns the count, and zero before the first build.
*/
fui_api int32_t fuiEditorGetVisibleLineCount(const fuiEditor *editor);

#endif // FUI_TEXTEDITOR_INCLUDE_H

// ****************************************************************************
// ****************************************************************************
//
// > Implementation
//
// ****************************************************************************
// ****************************************************************************
#if (defined(FUI_TEXTEDITOR_IMPLEMENTATION) && !defined(FUI_TEXTEDITOR_IMPLEMENTED)) || (FUI_IS_IDE)
#define FUI_TEXTEDITOR_IMPLEMENTED

#if !defined(FUI_TEXTEDITOR_MEMSET) || !defined(FUI_TEXTEDITOR_MEMCPY) || !defined(FUI_TEXTEDITOR_MEMMOVE) || !defined(FUI_TEXTEDITOR_MEMCHR) || !defined(FUI_TEXTEDITOR_MEMCMP) || !defined(FUI_TEXTEDITOR_STRLEN)
#	include <string.h>
#	if !defined(FUI_TEXTEDITOR_MEMSET)
		//! Memory set - define all five string overrides before including to skip <string.h>
#		define FUI_TEXTEDITOR_MEMSET(destination, value, size) memset(destination, value, size)
#	endif
#	if !defined(FUI_TEXTEDITOR_MEMCPY)
		//! Memory copy for ranges that never overlap
#		define FUI_TEXTEDITOR_MEMCPY(destination, source, size) memcpy(destination, source, size)
#	endif
#	if !defined(FUI_TEXTEDITOR_MEMMOVE)
		//! Memory move for ranges that may overlap, which is what moving the hole does
#		define FUI_TEXTEDITOR_MEMMOVE(destination, source, size) memmove(destination, source, size)
#	endif
#	if !defined(FUI_TEXTEDITOR_MEMCHR)
		//! Memory byte search, which is what finding the next line feed does
#		define FUI_TEXTEDITOR_MEMCHR(pointer, value, size) memchr(pointer, value, size)
#	endif
#	if !defined(FUI_TEXTEDITOR_MEMCMP)
		//! Memory compare, which is what noticing a restyled theme does
#		define FUI_TEXTEDITOR_MEMCMP(left, right, size) memcmp(left, right, size)
#	endif
#	if !defined(FUI_TEXTEDITOR_STRLEN)
		//! String length
#		define FUI_TEXTEDITOR_STRLEN(text) strlen(text)
#	endif
#endif

fui_api const char *fuiEditorGetVersion(void) {
	return(FUI_TEXTEDITOR_VERSION_STRING);
}

// ----------------------------------------------------------------------------
// > Small helpers
// ----------------------------------------------------------------------------

//! The codepoint a malformed or unrepresentable sequence becomes
#define FUI_TEXTEDITOR__REPLACEMENT_CODEPOINT 0xFFFDu

//! What an encoding writes when a codepoint has no representation of its own
#define FUI_TEXTEDITOR__SUBSTITUTE_BYTE '?'

//! How many bytes one utf-8 codepoint can take at most
#define FUI_TEXTEDITOR__MAX_UTF8_BYTES 4

fui_inline int32_t fuiEditor__MinI32(const int32_t a, const int32_t b) {
	return((a < b) ? a : b);
}

fui_inline int32_t fuiEditor__MaxI32(const int32_t a, const int32_t b) {
	return((a > b) ? a : b);
}

fui_inline int32_t fuiEditor__ClampI32(const int32_t value, const int32_t low, const int32_t high) {
	if(value < low) {
		return(low);
	}
	if(value > high) {
		return(high);
	}
	return(value);
}

//! Rounds a wanted capacity up to the next power of two step, so growth is amortized rather than linear
fui_inline int32_t fuiEditor__GrowCapacity(const int32_t currentCapacity, const int32_t wantedCapacity, const int32_t minimumCapacity) {
	const int32_t largestDoublableCapacity = 0x40000000;
	int32_t result = fuiEditor__MaxI32(currentCapacity, minimumCapacity);
	while(result < wantedCapacity) {
		if(result >= largestDoublableCapacity) {
			// Past this point doubling would wrap, so the last step is exactly what was asked for.
			return(wantedCapacity);
		}
		result = result * 2;
	}
	return(result);
}

//! Allocates through the editor's allocator, remembering a refusal on the editor rather than crashing
fui_inline void *fuiEditor__Allocate(fuiEditor *editor, const int32_t size) {
	void *result = editor->allocator.allocate(editor->allocator.userData, (size_t)size);
	if(result == fui_null) {
		editor->hasOutOfMemory = true;
	}
	return(result);
}

//! Releases through the editor's allocator, tolerating a null pointer the way free does
fui_inline void fuiEditor__Release(fuiEditor *editor, void *pointer) {
	if(pointer != fui_null) {
		editor->allocator.release(editor->allocator.userData, pointer);
	}
}

fui_inline bool fuiEditor__IsUtf8Continuation(const char byte) {
	uint8_t unsignedByte = (uint8_t)byte;
	return((unsignedByte & 0xC0u) == 0x80u);
}

// ----------------------------------------------------------------------------
// > Line endings
// ----------------------------------------------------------------------------

fui_api const char *fuiEditorEolGetName(const fuiEditorEol eol) {
	switch(eol) {
		case fuiEditorEol_Lf: return("LF");
		case fuiEditorEol_CrLf: return("CRLF");
		case fuiEditorEol_Cr: return("CR");
		case fuiEditorEol_Mixed: return("Mixed");
		default: return("LF");
	}
}

fui_api const char *fuiEditorEolGetBytes(const fuiEditorEol eol, int32_t *outLength) {
	switch(eol) {
		case fuiEditorEol_CrLf: {
			if(outLength != fui_null) {
				*outLength = 2;
			}
			return("\r\n");
		}
		case fuiEditorEol_Cr: {
			if(outLength != fui_null) {
				*outLength = 1;
			}
			return("\r");
		}
		default: {
			if(outLength != fui_null) {
				*outLength = 1;
			}
			return("\n");
		}
	}
}

/*
	Which endings a utf-8 text uses.

	Counted rather than guessed from the first one found: a file that is mostly CRLF with three stray
	line feeds in it is MIXED, and saying so is the only way a caller can be told that saving it will
	make every line agree.
*/
fui_inline fuiEditorEol fuiEditor__DetectEol(const char *text, const int32_t textLength) {
	bool sawCrLf = false;
	bool sawLoneLf = false;
	bool sawLoneCr = false;

	int32_t scanOffset = 0;
	while(scanOffset < textLength) {
		char currentByte = text[scanOffset];
		if(currentByte == '\r') {
			bool isFollowedByLineFeed = ((scanOffset + 1) < textLength) && (text[scanOffset + 1] == '\n');
			if(isFollowedByLineFeed) {
				sawCrLf = true;
				scanOffset += 2;
				continue;
			}
			sawLoneCr = true;
		} else if(currentByte == '\n') {
			sawLoneLf = true;
		}
		scanOffset += 1;
	}

	int32_t kindCount = 0;
	if(sawCrLf) {
		kindCount += 1;
	}
	if(sawLoneLf) {
		kindCount += 1;
	}
	if(sawLoneCr) {
		kindCount += 1;
	}

	if(kindCount > 1) {
		return(fuiEditorEol_Mixed);
	}
	if(sawCrLf) {
		return(fuiEditorEol_CrLf);
	}
	if(sawLoneCr) {
		return(fuiEditorEol_Cr);
	}
	return(fuiEditorEol_Lf);
}

// ----------------------------------------------------------------------------
// > Encodings
// ----------------------------------------------------------------------------

//! The utf-8 byte order mark, which is legal but carries no information and is dropped on the way in
static const uint8_t fuiEditor__Utf8ByteOrderMark[3] = { 0xEFu, 0xBBu, 0xBFu };

fui_inline int32_t fuiEditor__Utf8GetBomLength(void *userData, const uint8_t *data, const int32_t dataLength) {
	(void)userData;
	if(data == fui_null || dataLength < 3) {
		return(0);
	}
	bool startsWithMark = (data[0] == fuiEditor__Utf8ByteOrderMark[0]) && (data[1] == fuiEditor__Utf8ByteOrderMark[1]) && (data[2] == fuiEditor__Utf8ByteOrderMark[2]);
	if(startsWithMark) {
		return(3);
	}
	return(0);
}

/*
	Writes one encoded codepoint, or only counts it when there is nowhere to put it.

	Every converter below is written once and runs twice - once with a null destination to be told the
	length, once with the buffer to fill it - so the counting and the writing have to agree exactly.
	Sharing this one helper is what makes them agree.
*/
fui_inline int32_t fuiEditor__AppendUtf8(const uint32_t codePoint, char *destination, const int32_t destinationCapacity, const int32_t writeOffset) {
	char encodedBytes[FUI_TEXTEDITOR__MAX_UTF8_BYTES];
	uint32_t encodedLength = fuiEncodeUtf8(codePoint, encodedBytes);
	if(encodedLength == 0) {
		return(0);
	}
	bool thereIsRoom = (destination != fui_null) && ((writeOffset + (int32_t)encodedLength) <= destinationCapacity);
	if(thereIsRoom) {
		FUI_TEXTEDITOR_MEMCPY(&destination[writeOffset], encodedBytes, (size_t)encodedLength);
	}
	return((int32_t)encodedLength);
}

fui_inline int32_t fuiEditor__Utf8ToUtf8(void *userData, const uint8_t *source, const int32_t sourceLength, char *destination, const int32_t destinationCapacity) {
	(void)userData;
	if(source == fui_null || sourceLength <= 0) {
		return(0);
	}

	const char *sourceText = (const char *)source;
	int32_t writtenLength = 0;
	size_t readOffset = 0;
	while(readOffset < (size_t)sourceLength) {
		uint32_t codePoint = fuiDecodeUtf8(sourceText, (size_t)sourceLength, &readOffset);
		int32_t appendedLength = fuiEditor__AppendUtf8(codePoint, destination, destinationCapacity, writtenLength);
		writtenLength += appendedLength;
	}
	return(writtenLength);
}

fui_inline int32_t fuiEditor__Utf8FromUtf8(void *userData, const char *source, const int32_t sourceLength, uint8_t *destination, const int32_t destinationCapacity) {
	(void)userData;
	if(source == fui_null || sourceLength <= 0) {
		return(0);
	}
	bool thereIsRoom = (destination != fui_null) && (sourceLength <= destinationCapacity);
	if(thereIsRoom) {
		FUI_TEXTEDITOR_MEMCPY(destination, source, (size_t)sourceLength);
	}
	return(sourceLength);
}

fui_api fuiEditorEncoding fuiEditorEncodingUtf8(void) {
	fuiEditorEncoding result;
	result.name = "UTF-8";
	result.getBomLength = fuiEditor__Utf8GetBomLength;
	result.toUtf8 = fuiEditor__Utf8ToUtf8;
	result.fromUtf8 = fuiEditor__Utf8FromUtf8;
	result.userData = fui_null;
	return(result);
}

fui_inline int32_t fuiEditor__AsciiGetBomLength(void *userData, const uint8_t *data, const int32_t dataLength) {
	(void)userData;
	(void)data;
	(void)dataLength;
	return(0);
}

fui_inline int32_t fuiEditor__AsciiToUtf8(void *userData, const uint8_t *source, const int32_t sourceLength, char *destination, const int32_t destinationCapacity) {
	(void)userData;
	if(source == fui_null || sourceLength <= 0) {
		return(0);
	}

	const uint8_t highestAsciiByte = 0x7Fu;
	int32_t writtenLength = 0;
	int32_t readOffset = 0;
	while(readOffset < sourceLength) {
		uint8_t currentByte = source[readOffset];
		uint32_t codePoint = (currentByte <= highestAsciiByte) ? (uint32_t)currentByte : (uint32_t)FUI_TEXTEDITOR__REPLACEMENT_CODEPOINT;
		int32_t appendedLength = fuiEditor__AppendUtf8(codePoint, destination, destinationCapacity, writtenLength);
		writtenLength += appendedLength;
		readOffset += 1;
	}
	return(writtenLength);
}

fui_inline int32_t fuiEditor__AsciiFromUtf8(void *userData, const char *source, const int32_t sourceLength, uint8_t *destination, const int32_t destinationCapacity) {
	(void)userData;
	if(source == fui_null || sourceLength <= 0) {
		return(0);
	}

	const uint32_t highestAsciiCodePoint = 0x7Fu;
	int32_t writtenLength = 0;
	size_t readOffset = 0;
	while(readOffset < (size_t)sourceLength) {
		uint32_t codePoint = fuiDecodeUtf8(source, (size_t)sourceLength, &readOffset);
		uint8_t encodedByte = (codePoint <= highestAsciiCodePoint) ? (uint8_t)codePoint : (uint8_t)FUI_TEXTEDITOR__SUBSTITUTE_BYTE;
		bool thereIsRoom = (destination != fui_null) && ((writtenLength + 1) <= destinationCapacity);
		if(thereIsRoom) {
			destination[writtenLength] = encodedByte;
		}
		writtenLength += 1;
	}
	return(writtenLength);
}

fui_api fuiEditorEncoding fuiEditorEncodingAscii(void) {
	fuiEditorEncoding result;
	result.name = "ASCII";
	result.getBomLength = fuiEditor__AsciiGetBomLength;
	result.toUtf8 = fuiEditor__AsciiToUtf8;
	result.fromUtf8 = fuiEditor__AsciiFromUtf8;
	result.userData = fui_null;
	return(result);
}

// ----------------------------------------------------------------------------
// > The line index
// ----------------------------------------------------------------------------

/*
	The line index is a split array: entries in front of its hole are stored as they are, entries behind
	it are stored SHORT by tailDelta. An edit puts the hole right behind the line it happened on and adds
	its byte difference to tailDelta - which moves every line after it, at the price of one addition.

	Physical slot of logical entry i:  i < gapStart ? i : i + (gapEnd - gapStart)
	Value of logical entry i:          starts[physical] + (i < gapStart ? 0 : tailDelta)
*/

fui_inline int32_t fuiEditor__LineIndexCount(const fuiEditorLineIndex *index) {
	int32_t gapSize = index->gapEnd - index->gapStart;
	return(index->capacity - gapSize);
}

fui_inline int32_t fuiEditor__LineIndexGet(const fuiEditorLineIndex *index, const int32_t lineIndex) {
	if(lineIndex < index->gapStart) {
		return(index->starts[lineIndex]);
	}
	int32_t gapSize = index->gapEnd - index->gapStart;
	int32_t physicalSlot = lineIndex + gapSize;
	return(index->starts[physicalSlot] + index->tailDelta);
}

//! The parser state a line STARTS in, out of the same slot its start offset lives in
fui_inline int32_t fuiEditor__LineIndexGetLexerState(const fuiEditorLineIndex *index, const int32_t lineIndex) {
	if(lineIndex < index->gapStart) {
		return(index->lexerStates[lineIndex]);
	}
	int32_t gapSize = index->gapEnd - index->gapStart;
	int32_t physicalSlot = lineIndex + gapSize;
	return(index->lexerStates[physicalSlot]);
}

fui_inline void fuiEditor__LineIndexSetLexerState(fuiEditorLineIndex *index, const int32_t lineIndex, const int32_t state) {
	if(lineIndex < index->gapStart) {
		index->lexerStates[lineIndex] = state;
		return;
	}
	int32_t gapSize = index->gapEnd - index->gapStart;
	int32_t physicalSlot = lineIndex + gapSize;
	index->lexerStates[physicalSlot] = state;
}

//! Grows the slot array so that at least wantedFreeSlots sit in the hole, keeping every entry where it belongs
fui_inline bool fuiEditor__LineIndexReserve(fuiEditor *editor, fuiEditorLineIndex *index, const int32_t wantedFreeSlots) {
	int32_t gapSize = index->gapEnd - index->gapStart;
	if(gapSize >= wantedFreeSlots) {
		return(true);
	}

	int32_t entryCount = index->capacity - gapSize;
	int32_t wantedCapacity = entryCount + wantedFreeSlots + FUI_TEXTEDITOR_MIN_GAP_SLOTS;
	int32_t newCapacity = fuiEditor__GrowCapacity(index->capacity, wantedCapacity, FUI_TEXTEDITOR_MIN_LINE_SLOTS);

	int32_t newByteCount = newCapacity * (int32_t)sizeof(int32_t);
	int32_t *newStarts = (int32_t *)fuiEditor__Allocate(editor, newByteCount);
	if(newStarts == fui_null) {
		return(false);
	}
	int32_t *newLexerStates = (int32_t *)fuiEditor__Allocate(editor, newByteCount);
	if(newLexerStates == fui_null) {
		fuiEditor__Release(editor, newStarts);
		return(false);
	}

	// The entries in front of the hole keep their slots; the ones behind it move to the end of the bigger
	// array, so the hole simply becomes the larger space between them. The lexer states live in the SAME
	// slots as the starts do, which is what keeps a line's state attached to the line through every edit.
	int32_t tailCount = index->capacity - index->gapEnd;
	if(index->gapStart > 0) {
		FUI_TEXTEDITOR_MEMCPY(newStarts, index->starts, (size_t)index->gapStart * sizeof(int32_t));
		FUI_TEXTEDITOR_MEMCPY(newLexerStates, index->lexerStates, (size_t)index->gapStart * sizeof(int32_t));
	}
	if(tailCount > 0) {
		int32_t newTailSlot = newCapacity - tailCount;
		FUI_TEXTEDITOR_MEMCPY(&newStarts[newTailSlot], &index->starts[index->gapEnd], (size_t)tailCount * sizeof(int32_t));
		FUI_TEXTEDITOR_MEMCPY(&newLexerStates[newTailSlot], &index->lexerStates[index->gapEnd], (size_t)tailCount * sizeof(int32_t));
	}

	fuiEditor__Release(editor, index->starts);
	fuiEditor__Release(editor, index->lexerStates);
	index->starts = newStarts;
	index->lexerStates = newLexerStates;
	index->capacity = newCapacity;
	index->gapEnd = newCapacity - tailCount;
	return(true);
}

/*
	Moves the hole so that it begins at a given logical entry.

	Every entry that crosses the hole changes form: one moving forwards was stored short by tailDelta and
	has to be written out in full, one moving backwards is the other way round.
*/
fui_inline void fuiEditor__LineIndexMoveGap(fuiEditorLineIndex *index, const int32_t wantedGapStart) {
	int32_t entryCount = fuiEditor__LineIndexCount(index);
	int32_t targetGapStart = fuiEditor__ClampI32(wantedGapStart, 0, entryCount);

	// The STARTS change form as they cross - one moving forwards was stored short by tailDelta and has to
	// be written out in full, one moving backwards is the other way round. The lexer states just ride
	// along: a parser state is not an offset and nothing about it depends on where the text sits.
	while(index->gapStart < targetGapStart) {
		index->starts[index->gapStart] = index->starts[index->gapEnd] + index->tailDelta;
		index->lexerStates[index->gapStart] = index->lexerStates[index->gapEnd];
		index->gapStart += 1;
		index->gapEnd += 1;
	}
	while(index->gapStart > targetGapStart) {
		index->gapStart -= 1;
		index->gapEnd -= 1;
		index->starts[index->gapEnd] = index->starts[index->gapStart] - index->tailDelta;
		index->lexerStates[index->gapEnd] = index->lexerStates[index->gapStart];
	}
}

//! Which line an offset falls on, by binary search over the starts
fui_inline int32_t fuiEditor__LineIndexLineOfOffset(const fuiEditorLineIndex *index, const int32_t offset) {
	int32_t entryCount = fuiEditor__LineIndexCount(index);
	int32_t low = 0;
	int32_t high = entryCount - 1;
	int32_t result = 0;
	while(low <= high) {
		int32_t middle = low + (high - low) / 2;
		int32_t middleStart = fuiEditor__LineIndexGet(index, middle);
		if(middleStart <= offset) {
			result = middle;
			low = middle + 1;
		} else {
			high = middle - 1;
		}
	}
	return(result);
}

// ----------------------------------------------------------------------------
// > The document
// ----------------------------------------------------------------------------

fui_inline int32_t fuiEditor__DocumentLength(const fuiEditorDocument *document) {
	int32_t gapSize = document->gapEnd - document->gapStart;
	return(document->capacity - gapSize);
}

fui_inline int32_t fuiEditor__DocumentPhysicalOffset(const fuiEditorDocument *document, const int32_t offset) {
	if(offset < document->gapStart) {
		return(offset);
	}
	int32_t gapSize = document->gapEnd - document->gapStart;
	return(offset + gapSize);
}

//! Grows the byte array so that at least wantedFreeBytes sit in the hole, keeping every byte where it belongs
fui_inline bool fuiEditor__DocumentReserve(fuiEditor *editor, fuiEditorDocument *document, const int32_t wantedFreeBytes) {
	// One byte more than the caller asked for, always. The hole is never allowed to close completely,
	// because that is where fuiEditorGetContiguousText puts its terminating zero.
	const int32_t roomForTheTerminator = 1;
	int32_t neededFreeBytes = wantedFreeBytes + roomForTheTerminator;

	int32_t gapSize = document->gapEnd - document->gapStart;
	if(gapSize >= neededFreeBytes) {
		return(true);
	}

	int32_t textLength = document->capacity - gapSize;
	int32_t wantedCapacity = textLength + neededFreeBytes + FUI_TEXTEDITOR_MIN_GAP_BYTES;
	int32_t newCapacity = fuiEditor__GrowCapacity(document->capacity, wantedCapacity, FUI_TEXTEDITOR_MIN_TEXT_BYTES);

	char *newBytes = (char *)fuiEditor__Allocate(editor, newCapacity);
	if(newBytes == fui_null) {
		return(false);
	}

	int32_t tailCount = document->capacity - document->gapEnd;
	if(document->gapStart > 0) {
		FUI_TEXTEDITOR_MEMCPY(newBytes, document->bytes, (size_t)document->gapStart);
	}
	if(tailCount > 0) {
		int32_t newTailOffset = newCapacity - tailCount;
		FUI_TEXTEDITOR_MEMCPY(&newBytes[newTailOffset], &document->bytes[document->gapEnd], (size_t)tailCount);
	}

	fuiEditor__Release(editor, document->bytes);
	document->bytes = newBytes;
	document->capacity = newCapacity;
	document->gapEnd = newCapacity - tailCount;
	return(true);
}

//! Moves the hole so that it begins at a given document offset. This is the one memmove an edit costs
fui_inline void fuiEditor__DocumentMoveGap(fuiEditorDocument *document, const int32_t wantedGapStart) {
	int32_t textLength = fuiEditor__DocumentLength(document);
	int32_t targetGapStart = fuiEditor__ClampI32(wantedGapStart, 0, textLength);

	if(targetGapStart < document->gapStart) {
		int32_t movedCount = document->gapStart - targetGapStart;
		int32_t newGapEnd = document->gapEnd - movedCount;
		FUI_TEXTEDITOR_MEMMOVE(&document->bytes[newGapEnd], &document->bytes[targetGapStart], (size_t)movedCount);
		document->gapStart = targetGapStart;
		document->gapEnd = newGapEnd;
	} else if(targetGapStart > document->gapStart) {
		int32_t movedCount = targetGapStart - document->gapStart;
		FUI_TEXTEDITOR_MEMMOVE(&document->bytes[document->gapStart], &document->bytes[document->gapEnd], (size_t)movedCount);
		document->gapStart = targetGapStart;
		document->gapEnd = document->gapEnd + movedCount;
	}
}

// ----------------------------------------------------------------------------
// > Lifetime
// ----------------------------------------------------------------------------

//! Puts the document back to one empty line, keeping whatever memory is already allocated
fui_inline void fuiEditor__DocumentClear(fuiEditorDocument *document) {
	document->gapStart = 0;
	document->gapEnd = document->capacity;

	document->lines.gapStart = 0;
	document->lines.gapEnd = document->lines.capacity;
	document->lines.tailDelta = 0;

	// An empty document is one empty line, not none - that is the line a caret sits on before anything
	// has been typed, and every line query below counts on entry zero being there.
	if(document->lines.capacity > 0) {
		document->lines.starts[0] = 0;
		// The first line's parser state is the only one that is true by definition: nothing came before it.
		document->lines.lexerStates[0] = 0;
		document->lines.gapStart = 1;
	}
}

fui_api bool fuiEditorInit(fuiEditor *editor, const fuiAllocator *allocator) {
	FUI_TEXTEDITOR_ASSERT(editor != fui_null);
	if(editor == fui_null) {
		return(false);
	}

	FUI_TEXTEDITOR_MEMSET(editor, 0, sizeof(*editor));

	if(allocator != fui_null && allocator->allocate != fui_null && allocator->release != fui_null) {
		editor->allocator = *allocator;
	} else {
		fuiAllocator defaultAllocator = fuiDefaultAllocator();
		editor->allocator = defaultAllocator;
	}

	editor->encoding = fuiEditorEncodingUtf8();
	editor->eol = fuiEditorEol_Lf;
	editor->config = fuiEditorDefaultConfig();

	char *initialBytes = (char *)fuiEditor__Allocate(editor, FUI_TEXTEDITOR_MIN_TEXT_BYTES);
	if(initialBytes == fui_null) {
		return(false);
	}
	int32_t initialSlotBytes = FUI_TEXTEDITOR_MIN_LINE_SLOTS * (int32_t)sizeof(int32_t);
	int32_t *initialStarts = (int32_t *)fuiEditor__Allocate(editor, initialSlotBytes);
	if(initialStarts == fui_null) {
		fuiEditor__Release(editor, initialBytes);
		return(false);
	}
	int32_t *initialLexerStates = (int32_t *)fuiEditor__Allocate(editor, initialSlotBytes);
	if(initialLexerStates == fui_null) {
		fuiEditor__Release(editor, initialBytes);
		fuiEditor__Release(editor, initialStarts);
		return(false);
	}

	editor->document.bytes = initialBytes;
	editor->document.capacity = FUI_TEXTEDITOR_MIN_TEXT_BYTES;
	editor->document.lines.starts = initialStarts;
	editor->document.lines.lexerStates = initialLexerStates;
	editor->document.lines.capacity = FUI_TEXTEDITOR_MIN_LINE_SLOTS;
	fuiEditor__DocumentClear(&editor->document);
	editor->styledUpToLine = 1;

	editor->isInitialized = true;
	return(true);
}

fui_api void fuiEditorRelease(fuiEditor *editor) {
	if(editor == fui_null || !editor->isInitialized) {
		return;
	}
	fuiEditor__Release(editor, editor->document.bytes);
	fuiEditor__Release(editor, editor->document.lines.starts);
	fuiEditor__Release(editor, editor->document.lines.lexerStates);
	fuiEditor__Release(editor, editor->lineScratch);
	fuiEditor__Release(editor, editor->styleScratch);
	FUI_TEXTEDITOR_MEMSET(editor, 0, sizeof(*editor));
}

// ----------------------------------------------------------------------------
// > Reading the document
// ----------------------------------------------------------------------------

fui_api int32_t fuiEditorGetTextLength(const fuiEditor *editor) {
	if(editor == fui_null || !editor->isInitialized) {
		return(0);
	}
	return(fuiEditor__DocumentLength(&editor->document));
}

fui_api char fuiEditorGetByte(const fuiEditor *editor, const int32_t offset) {
	if(editor == fui_null || !editor->isInitialized) {
		return('\0');
	}
	int32_t textLength = fuiEditor__DocumentLength(&editor->document);
	if(offset < 0 || offset >= textLength) {
		return('\0');
	}
	int32_t physicalOffset = fuiEditor__DocumentPhysicalOffset(&editor->document, offset);
	return(editor->document.bytes[physicalOffset]);
}

fui_api int32_t fuiEditorCopyRange(const fuiEditor *editor, const int32_t offset, const int32_t byteCount, char *destination, const int32_t destinationCapacity) {
	if(editor == fui_null || !editor->isInitialized) {
		if(destination != fui_null && destinationCapacity > 0) {
			destination[0] = '\0';
		}
		return(0);
	}

	const fuiEditorDocument *document = &editor->document;
	int32_t textLength = fuiEditor__DocumentLength(document);
	int32_t rangeStart = fuiEditor__ClampI32(offset, 0, textLength);
	int32_t wantedCount = fuiEditor__MaxI32(byteCount, 0);
	int32_t rangeLength = fuiEditor__MinI32(wantedCount, textLength - rangeStart);

	if(destination == fui_null || destinationCapacity <= 0) {
		return(rangeLength);
	}

	int32_t roomForBytes = destinationCapacity - 1;
	int32_t copyLength = fuiEditor__MinI32(rangeLength, roomForBytes);

	// The range may straddle the hole, in which case it is two runs rather than one.
	int32_t frontLength = fuiEditor__ClampI32(document->gapStart - rangeStart, 0, copyLength);
	if(frontLength > 0) {
		FUI_TEXTEDITOR_MEMCPY(destination, &document->bytes[rangeStart], (size_t)frontLength);
	}
	int32_t backLength = copyLength - frontLength;
	if(backLength > 0) {
		int32_t backStart = fuiEditor__MaxI32(rangeStart, document->gapStart);
		int32_t physicalBackStart = fuiEditor__DocumentPhysicalOffset(document, backStart);
		FUI_TEXTEDITOR_MEMCPY(&destination[frontLength], &document->bytes[physicalBackStart], (size_t)backLength);
	}
	destination[copyLength] = '\0';
	return(rangeLength);
}

fui_api int32_t fuiEditorCopyText(const fuiEditor *editor, char *destination, const int32_t destinationCapacity) {
	int32_t textLength = fuiEditorGetTextLength(editor);
	const int32_t fromTheStart = 0;
	return(fuiEditorCopyRange(editor, fromTheStart, textLength, destination, destinationCapacity));
}

fui_api const char *fuiEditorGetContiguousText(fuiEditor *editor) {
	if(editor == fui_null || !editor->isInitialized) {
		return("");
	}
	fuiEditorDocument *document = &editor->document;
	int32_t textLength = fuiEditor__DocumentLength(document);
	fuiEditor__DocumentMoveGap(document, textLength);
	// The reserve above always keeps one byte more than the text needs, so this never writes past the end.
	document->bytes[textLength] = '\0';
	return(document->bytes);
}

// ----------------------------------------------------------------------------
// > Lines
// ----------------------------------------------------------------------------

fui_api int32_t fuiEditorGetLineCount(const fuiEditor *editor) {
	if(editor == fui_null || !editor->isInitialized) {
		return(0);
	}
	return(fuiEditor__LineIndexCount(&editor->document.lines));
}

fui_api int32_t fuiEditorGetLineStart(const fuiEditor *editor, const int32_t lineIndex) {
	int32_t lineCount = fuiEditorGetLineCount(editor);
	if(lineCount == 0) {
		return(0);
	}
	int32_t clampedLine = fuiEditor__ClampI32(lineIndex, 0, lineCount - 1);
	return(fuiEditor__LineIndexGet(&editor->document.lines, clampedLine));
}

fui_api int32_t fuiEditorGetLineEnd(const fuiEditor *editor, const int32_t lineIndex) {
	int32_t lineCount = fuiEditorGetLineCount(editor);
	if(lineCount == 0) {
		return(0);
	}
	int32_t clampedLine = fuiEditor__ClampI32(lineIndex, 0, lineCount - 1);
	int32_t textLength = fuiEditor__DocumentLength(&editor->document);

	bool isTheLastLine = (clampedLine == (lineCount - 1));
	if(isTheLastLine) {
		return(textLength);
	}

	// The next line starts one past the line feed that ended this one, so the visible end is one before it.
	int32_t nextLineStart = fuiEditor__LineIndexGet(&editor->document.lines, clampedLine + 1);
	int32_t lineFeedOffset = nextLineStart - 1;
	bool hasCarriageReturnBeforeIt = false;
	if(lineFeedOffset > 0) {
		char byteBeforeTheLineFeed = fuiEditorGetByte(editor, lineFeedOffset - 1);
		hasCarriageReturnBeforeIt = (byteBeforeTheLineFeed == '\r');
	}
	if(hasCarriageReturnBeforeIt) {
		return(lineFeedOffset - 1);
	}
	return(lineFeedOffset);
}

fui_api int32_t fuiEditorGetLineLength(const fuiEditor *editor, const int32_t lineIndex) {
	int32_t lineStart = fuiEditorGetLineStart(editor, lineIndex);
	int32_t lineEnd = fuiEditorGetLineEnd(editor, lineIndex);
	return(fuiEditor__MaxI32(lineEnd - lineStart, 0));
}

fui_api int32_t fuiEditorCopyLine(const fuiEditor *editor, const int32_t lineIndex, char *destination, const int32_t destinationCapacity) {
	int32_t lineStart = fuiEditorGetLineStart(editor, lineIndex);
	int32_t lineLength = fuiEditorGetLineLength(editor, lineIndex);
	return(fuiEditorCopyRange(editor, lineStart, lineLength, destination, destinationCapacity));
}

fui_api int32_t fuiEditorGetLineOfOffset(const fuiEditor *editor, const int32_t offset) {
	if(editor == fui_null || !editor->isInitialized) {
		return(0);
	}
	int32_t textLength = fuiEditor__DocumentLength(&editor->document);
	int32_t clampedOffset = fuiEditor__ClampI32(offset, 0, textLength);
	return(fuiEditor__LineIndexLineOfOffset(&editor->document.lines, clampedOffset));
}

fui_api fuiEditorEol fuiEditorGetEol(const fuiEditor *editor) {
	if(editor == fui_null) {
		return(fuiEditorEol_Lf);
	}
	return(editor->eol);
}

fui_api void fuiEditorSetEol(fuiEditor *editor, const fuiEditorEol eol) {
	if(editor == fui_null) {
		return;
	}
	editor->eol = eol;
}

// ----------------------------------------------------------------------------
// > Codepoint boundaries
// ----------------------------------------------------------------------------

fui_api int32_t fuiEditorNextCodepointOffset(const fuiEditor *editor, const int32_t offset) {
	int32_t textLength = fuiEditorGetTextLength(editor);
	int32_t walkOffset = fuiEditor__ClampI32(offset, 0, textLength);
	if(walkOffset >= textLength) {
		return(textLength);
	}
	walkOffset += 1;
	while(walkOffset < textLength) {
		char currentByte = fuiEditorGetByte(editor, walkOffset);
		if(!fuiEditor__IsUtf8Continuation(currentByte)) {
			break;
		}
		walkOffset += 1;
	}
	return(walkOffset);
}

fui_api int32_t fuiEditorPreviousCodepointOffset(const fuiEditor *editor, const int32_t offset) {
	int32_t textLength = fuiEditorGetTextLength(editor);
	int32_t walkOffset = fuiEditor__ClampI32(offset, 0, textLength);
	if(walkOffset <= 0) {
		return(0);
	}
	walkOffset -= 1;
	while(walkOffset > 0) {
		char currentByte = fuiEditorGetByte(editor, walkOffset);
		if(!fuiEditor__IsUtf8Continuation(currentByte)) {
			break;
		}
		walkOffset -= 1;
	}
	return(walkOffset);
}

fui_api int32_t fuiEditorSnapToCodepointStart(const fuiEditor *editor, const int32_t offset) {
	int32_t textLength = fuiEditorGetTextLength(editor);
	int32_t walkOffset = fuiEditor__ClampI32(offset, 0, textLength);
	while(walkOffset > 0 && walkOffset < textLength) {
		char currentByte = fuiEditorGetByte(editor, walkOffset);
		if(!fuiEditor__IsUtf8Continuation(currentByte)) {
			break;
		}
		walkOffset -= 1;
	}
	return(walkOffset);
}

// ----------------------------------------------------------------------------
// > Colouring
// ----------------------------------------------------------------------------

/*
	How much of the document is coloured, and when that has to be done again.

	lexerStates[i] is the state a lexer STARTS line i in, and styledUpToLine says how many of them are
	believed: states [0, styledUpToLine) are good, everything above is whatever was left there. Line zero
	is the only one that is true without being worked out - nothing came before it.

	An edit drops the watermark to just behind the line it happened on. Everything ABOVE the watermark is
	not thrown away, because it is what makes the next pass cheap: as soon as a recomputed state comes out
	equal to the one already stored, the rest of the document was already right and does not have to be
	looked at at all. That is what keeps "jump to the end of a big file, then change line three" from
	costing a walk over the whole thing.

	The one case that must NOT converge is a line whose stored state was never written by a lexer - a line
	that has just been inserted, or a document that has just been filled. Those are held below
	lexConvergenceFloor, and no pass may stop before it has got past them.
*/

/*
	Drops everything believed from a line onwards.

	lastUnwrittenLine is the HIGHEST line index whose state slot holds something no lexer ever put there -
	a line that has just appeared. Zero means none of them did. A pass may not stop before it is past that
	line, because a garbage slot that happens to match would look exactly like a state that converged.
*/
static void fuiEditor__InvalidateStylesFrom(fuiEditor *editor, const int32_t firstDoubtfulLine, const int32_t lastUnwrittenLine) {
	int32_t lowestBelievable = fuiEditor__MaxI32(firstDoubtfulLine, 1);
	editor->styledUpToLine = fuiEditor__MinI32(editor->styledUpToLine, lowestBelievable);
	editor->lexConvergenceFloor = fuiEditor__MaxI32(editor->lexConvergenceFloor, lastUnwrittenLine);
}

fui_api void fuiEditorInvalidateStyles(fuiEditor *editor, const int32_t documentLine) {
	if(editor == fui_null || !editor->isInitialized) {
		return;
	}
	const int32_t nothingIsUnwritten = 0;
	fuiEditor__InvalidateStylesFrom(editor, documentLine, nothingIsUnwritten);
}

fui_api void fuiEditorSetLexer(fuiEditor *editor, const fuiEditorLexer *lexer) {
	if(editor == fui_null || !editor->isInitialized) {
		return;
	}
	if(lexer != fui_null) {
		editor->lexer = *lexer;
	} else {
		FUI_TEXTEDITOR_MEMSET(&editor->lexer, 0, sizeof(editor->lexer));
	}

	// A different lexer answers differently everywhere, and nothing the last one left behind can be
	// believed - so every state above line zero counts as never written rather than merely doubtful.
	int32_t lineCount = fuiEditorGetLineCount(editor);
	int32_t lastLine = fuiEditor__MaxI32(lineCount - 1, 0);
	editor->styledUpToLine = 1;
	editor->lexConvergenceFloor = lastLine;
}

fui_api void fuiEditorSetDecorations(fuiEditor *editor, const fuiEditorDecorations *decorations) {
	if(editor == fui_null || !editor->isInitialized) {
		return;
	}
	if(decorations != fui_null) {
		editor->decorations = *decorations;
	} else {
		FUI_TEXTEDITOR_MEMSET(&editor->decorations, 0, sizeof(editor->decorations));
	}
}

//! Makes sure one line's worth of bytes and style bytes fits in the scratch
static bool fuiEditor__EnsureScratch(fuiEditor *editor, const int32_t wantedCapacity) {
	if(editor->scratchCapacity >= wantedCapacity && editor->lineScratch != fui_null) {
		return(true);
	}

	int32_t newCapacity = fuiEditor__GrowCapacity(editor->scratchCapacity, wantedCapacity, FUI_TEXTEDITOR_MIN_TEXT_BYTES);
	char *newLineScratch = (char *)fuiEditor__Allocate(editor, newCapacity);
	if(newLineScratch == fui_null) {
		return(false);
	}
	uint8_t *newStyleScratch = (uint8_t *)fuiEditor__Allocate(editor, newCapacity);
	if(newStyleScratch == fui_null) {
		fuiEditor__Release(editor, newLineScratch);
		return(false);
	}

	fuiEditor__Release(editor, editor->lineScratch);
	fuiEditor__Release(editor, editor->styleScratch);
	editor->lineScratch = newLineScratch;
	editor->styleScratch = newStyleScratch;
	editor->scratchCapacity = newCapacity;
	return(true);
}

/*
	Runs the lexer over one line, leaving its style bytes in the scratch.

	The line is COPIED first, because a lexer wants bytes that lie next to each other and a line may sit
	across the hole. That copy is also what lets a lexer be written against a plain pointer and a length
	instead of against this file's internals.
*/
static int32_t fuiEditor__LexOneLine(fuiEditor *editor, const int32_t lineIndex, const int32_t startState, int32_t *outLineLength) {
	*outLineLength = 0;
	int32_t lineStart = fuiEditorGetLineStart(editor, lineIndex);
	int32_t lineEnd = fuiEditorGetLineEnd(editor, lineIndex);
	int32_t lineLength = fuiEditor__MaxI32(lineEnd - lineStart, 0);

	// One byte more than the line, so an empty line still has somewhere for its terminator to go.
	if(!fuiEditor__EnsureScratch(editor, lineLength + 1)) {
		return(startState);
	}
	(void)fuiEditorCopyRange(editor, lineStart, lineLength, editor->lineScratch, editor->scratchCapacity);
	FUI_TEXTEDITOR_MEMSET(editor->styleScratch, FUI_TEXTEDITOR_STYLE_DEFAULT, (size_t)lineLength);

	fuiEditorLexRequest request;
	FUI_TEXTEDITOR_MEMSET(&request, 0, sizeof(request));
	request.text = editor->lineScratch;
	request.textLength = lineLength;
	request.lineIndex = lineIndex;
	request.startState = startState;
	request.styles = editor->styleScratch;
	request.userData = editor->lexer.userData;

	*outLineLength = lineLength;
	return(editor->lexer.lexLine(&request));
}

//! Works the watermark up to a line, stopping early the moment the states agree again
static void fuiEditor__LexUpToLine(fuiEditor *editor, const int32_t wantedLine) {
	if(editor->lexer.lexLine == fui_null) {
		return;
	}

	int32_t lineCount = fuiEditorGetLineCount(editor);
	if(lineCount <= 0) {
		return;
	}
	int32_t lastLine = lineCount - 1;
	editor->styledUpToLine = fuiEditor__ClampI32(editor->styledUpToLine, 1, lineCount);
	editor->lexConvergenceFloor = fuiEditor__ClampI32(editor->lexConvergenceFloor, 0, lastLine);

	int32_t targetLine = fuiEditor__ClampI32(wantedLine, 0, lastLine);
	int32_t linesLeftThisBuild = FUI_TEXTEDITOR_MAX_LEX_LINES_PER_FRAME;
	fuiEditorLineIndex *index = &editor->document.lines;

	while((editor->styledUpToLine <= targetLine) && (linesLeftThisBuild > 0)) {
		int32_t lastKnownLine = editor->styledUpToLine - 1;
		int32_t startState = fuiEditor__LineIndexGetLexerState(index, lastKnownLine);

		int32_t lineLength = 0;
		int32_t endState = fuiEditor__LexOneLine(editor, lastKnownLine, startState, &lineLength);

		int32_t nextLine = lastKnownLine + 1;
		int32_t previouslyStoredState = fuiEditor__LineIndexGetLexerState(index, nextLine);
		fuiEditor__LineIndexSetLexerState(index, nextLine, endState);
		editor->styledUpToLine = nextLine + 1;
		linesLeftThisBuild -= 1;

		bool isPastEverythingUnwritten = (nextLine > editor->lexConvergenceFloor);
		bool theStateCameOutTheSame = (previouslyStoredState == endState);
		if(isPastEverythingUnwritten && theStateCameOutTheSame) {
			// Everything behind this line was worked out from a state that has not changed, so it is still
			// right. This is the whole reason an edit at the top of a large file is cheap.
			editor->styledUpToLine = lineCount;
			editor->lexConvergenceFloor = 0;
			break;
		}
	}

	if(editor->styledUpToLine > editor->lexConvergenceFloor) {
		editor->lexConvergenceFloor = 0;
	}
}

//! What text of one style is drawn in, and what a style byte outside the table means
fui_inline fuiColor fuiEditor__StyleTextColor(const fuiEditor *editor, const uint8_t style, const fuiColor defaultColor) {
	bool isInTheTable = (editor->lexer.styles != fui_null) && ((int32_t)style < editor->lexer.styleCount);
	if(!isInTheTable) {
		return(defaultColor);
	}
	fuiColor styleColor = editor->lexer.styles[style].color;
	if(styleColor.a <= 0.0f) {
		return(defaultColor);
	}
	return(styleColor);
}

// ----------------------------------------------------------------------------
// > Decorations
// ----------------------------------------------------------------------------

//! The first line decoration that is at or behind a line, by binary search over the sorted array
static int32_t fuiEditor__FirstLineDecorationFrom(const fuiEditorDecorations *decorations, const int32_t wantedLine) {
	int32_t low = 0;
	int32_t high = decorations->lineCount;
	while(low < high) {
		int32_t middle = low + (high - low) / 2;
		if(decorations->lines[middle].line < wantedLine) {
			low = middle + 1;
		} else {
			high = middle;
		}
	}
	return(low);
}

//! The first range decoration that could still reach into an offset, by the same search
static int32_t fuiEditor__FirstRangeDecorationFrom(const fuiEditorDecorations *decorations, const int32_t wantedOffset) {
	int32_t low = 0;
	int32_t high = decorations->rangeCount;
	while(low < high) {
		int32_t middle = low + (high - low) / 2;
		if(decorations->ranges[middle].startOffset < wantedOffset) {
			low = middle + 1;
		} else {
			high = middle;
		}
	}
	// One back, because a range that STARTED before the offset may still cover it. Ranges are documented
	// as not overlapping, so one is enough.
	if(low > 0) {
		return(low - 1);
	}
	return(0);
}

//! What one line's decoration says, or nothing at all when it has none
static const fuiEditorLineDecoration *fuiEditor__LineDecorationAt(const fuiEditorDecorations *decorations, int32_t *inOutCursor, const int32_t wantedLine) {
	int32_t cursor = *inOutCursor;
	while(cursor < decorations->lineCount && decorations->lines[cursor].line < wantedLine) {
		cursor += 1;
	}
	*inOutCursor = cursor;
	if(cursor < decorations->lineCount && decorations->lines[cursor].line == wantedLine) {
		return(&decorations->lines[cursor]);
	}
	return(fui_null);
}

// ----------------------------------------------------------------------------
// > Editing
// ----------------------------------------------------------------------------

fui_api bool fuiEditorInsert(fuiEditor *editor, const int32_t offset, const char *text, const int32_t textLength) {
	if(editor == fui_null || !editor->isInitialized || text == fui_null) {
		return(false);
	}

	int32_t insertedLength = textLength;
	if(insertedLength <= 0) {
		size_t measuredLength = FUI_TEXTEDITOR_STRLEN(text);
		insertedLength = (int32_t)measuredLength;
	}
	if(insertedLength <= 0) {
		return(false);
	}

	fuiEditorDocument *document = &editor->document;
	int32_t documentLength = fuiEditor__DocumentLength(document);
	int32_t insertOffset = fuiEditor__ClampI32(offset, 0, documentLength);

	if(!fuiEditor__DocumentReserve(editor, document, insertedLength)) {
		return(false);
	}

	// How many lines the new text adds decides how much room the line index needs, and it has to be known
	// before either hole is moved - a failed reserve halfway through would leave the two of them disagreeing.
	int32_t addedLineCount = 0;
	int32_t scanOffset = 0;
	while(scanOffset < insertedLength) {
		const char *foundLineFeed = (const char *)FUI_TEXTEDITOR_MEMCHR(&text[scanOffset], '\n', (size_t)(insertedLength - scanOffset));
		if(foundLineFeed == fui_null) {
			break;
		}
		addedLineCount += 1;
		scanOffset = (int32_t)(foundLineFeed - text) + 1;
	}
	if(!fuiEditor__LineIndexReserve(editor, &document->lines, addedLineCount)) {
		return(false);
	}

	fuiEditor__DocumentMoveGap(document, insertOffset);
	FUI_TEXTEDITOR_MEMCPY(&document->bytes[document->gapStart], text, (size_t)insertedLength);
	document->gapStart += insertedLength;

	int32_t insertedOnLine = fuiEditor__LineIndexLineOfOffset(&document->lines, insertOffset);
	fuiEditor__LineIndexMoveGap(&document->lines, insertedOnLine + 1);

	// Everything behind the hole is a line after the edit, so all of it moves by the same amount.
	document->lines.tailDelta += insertedLength;

	// The new lines go in front of the hole, in order, as absolute offsets.
	int32_t newLineScanOffset = 0;
	while(newLineScanOffset < insertedLength) {
		const char *foundLineFeed = (const char *)FUI_TEXTEDITOR_MEMCHR(&text[newLineScanOffset], '\n', (size_t)(insertedLength - newLineScanOffset));
		if(foundLineFeed == fui_null) {
			break;
		}
		int32_t lineFeedIndex = (int32_t)(foundLineFeed - text);
		int32_t newLineStart = insertOffset + lineFeedIndex + 1;
		document->lines.starts[document->lines.gapStart] = newLineStart;
		document->lines.gapStart += 1;
		newLineScanOffset = lineFeedIndex + 1;
	}

	// The line the text landed in has to be looked at again. Only lines that really CAME WITH it have
	// state slots nothing ever wrote - text without a line feed in it adds none.
	int32_t lastUnwrittenLine = (addedLineCount > 0) ? (insertedOnLine + addedLineCount) : 0;
	fuiEditor__InvalidateStylesFrom(editor, insertedOnLine + 1, lastUnwrittenLine);

	editor->version += 1;
	return(true);
}

fui_api bool fuiEditorErase(fuiEditor *editor, const int32_t offset, const int32_t byteCount) {
	if(editor == fui_null || !editor->isInitialized) {
		return(false);
	}

	fuiEditorDocument *document = &editor->document;
	int32_t documentLength = fuiEditor__DocumentLength(document);
	int32_t eraseStart = fuiEditor__ClampI32(offset, 0, documentLength);
	int32_t wantedCount = fuiEditor__MaxI32(byteCount, 0);
	int32_t erasedLength = fuiEditor__MinI32(wantedCount, documentLength - eraseStart);
	if(erasedLength <= 0) {
		return(false);
	}

	int32_t eraseEnd = eraseStart + erasedLength;
	int32_t firstLine = fuiEditor__LineIndexLineOfOffset(&document->lines, eraseStart);
	int32_t lastLine = fuiEditor__LineIndexLineOfOffset(&document->lines, eraseEnd);
	int32_t removedLineCount = lastLine - firstLine;

	fuiEditor__DocumentMoveGap(document, eraseStart);
	document->gapEnd += erasedLength;

	fuiEditor__LineIndexMoveGap(&document->lines, firstLine + 1);

	// The lines that began inside the erased range are dropped by widening the hole over them. Their
	// stored values do not have to be cleared: nothing behind the hole is ever read as an entry again.
	document->lines.gapEnd += removedLineCount;
	document->lines.tailDelta -= erasedLength;

	// Nothing NEW appeared, so the states behind the erase are still the ones a lexer wrote - only the
	// line the erase happened on has to be looked at again.
	const int32_t nothingIsUnwritten = 0;
	fuiEditor__InvalidateStylesFrom(editor, firstLine + 1, nothingIsUnwritten);

	editor->version += 1;
	return(true);
}

// ----------------------------------------------------------------------------
// > Filling the document
// ----------------------------------------------------------------------------

fui_api bool fuiEditorSetText(fuiEditor *editor, const char *text, const int32_t textLength) {
	if(editor == fui_null || !editor->isInitialized) {
		return(false);
	}

	fuiEditor__DocumentClear(&editor->document);
	editor->version += 1;
	editor->caretOffset = 0;
	editor->selectionAnchor = 0;
	editor->scrollX = 0.0f;
	editor->scrollY = 0.0f;
	editor->hasPendingScroll = false;

	// A brand new document has state slots nothing ever wrote, all the way down. What fills it below sets
	// the floor to the line count it ends up with.
	editor->styledUpToLine = 1;
	editor->lexConvergenceFloor = 0;

	if(text == fui_null) {
		editor->eol = fuiEditorEol_Lf;
		return(true);
	}

	int32_t resolvedLength = textLength;
	if(resolvedLength <= 0) {
		size_t measuredLength = FUI_TEXTEDITOR_STRLEN(text);
		resolvedLength = (int32_t)measuredLength;
	}

	editor->eol = fuiEditor__DetectEol(text, resolvedLength);
	if(resolvedLength <= 0) {
		return(true);
	}

	const int32_t atTheStart = 0;
	return(fuiEditorInsert(editor, atTheStart, text, resolvedLength));
}

fui_api bool fuiEditorLoadFromMemory(fuiEditor *editor, const uint8_t *data, const int32_t dataLength, const fuiEditorEncoding *encoding) {
	if(editor == fui_null || !editor->isInitialized) {
		return(false);
	}

	fuiEditorEncoding resolvedEncoding = fuiEditorEncodingUtf8();
	if(encoding != fui_null && encoding->toUtf8 != fui_null && encoding->fromUtf8 != fui_null) {
		resolvedEncoding = *encoding;
	}
	editor->encoding = resolvedEncoding;

	if(data == fui_null || dataLength <= 0) {
		const char *nothingAtAll = fui_null;
		return(fuiEditorSetText(editor, nothingAtAll, 0));
	}

	int32_t byteOrderMarkLength = 0;
	if(resolvedEncoding.getBomLength != fui_null) {
		byteOrderMarkLength = resolvedEncoding.getBomLength(resolvedEncoding.userData, data, dataLength);
	}
	const uint8_t *payload = &data[byteOrderMarkLength];
	int32_t payloadLength = dataLength - byteOrderMarkLength;
	if(payloadLength <= 0) {
		const char *nothingAtAll = fui_null;
		return(fuiEditorSetText(editor, nothingAtAll, 0));
	}

	// Asked once for the length, then once more to fill - which is the contract every converter follows.
	char *noDestinationYet = fui_null;
	const int32_t noCapacityYet = 0;
	int32_t convertedLength = resolvedEncoding.toUtf8(resolvedEncoding.userData, payload, payloadLength, noDestinationYet, noCapacityYet);
	if(convertedLength <= 0) {
		const char *nothingAtAll = fui_null;
		return(fuiEditorSetText(editor, nothingAtAll, 0));
	}

	char *convertedText = (char *)fuiEditor__Allocate(editor, convertedLength);
	if(convertedText == fui_null) {
		return(false);
	}
	(void)resolvedEncoding.toUtf8(resolvedEncoding.userData, payload, payloadLength, convertedText, convertedLength);

	bool didSetText = fuiEditorSetText(editor, convertedText, convertedLength);
	fuiEditor__Release(editor, convertedText);
	return(didSetText);
}

// ----------------------------------------------------------------------------
// > Configuration
// ----------------------------------------------------------------------------

//! How many characters wide one tab stop is when the caller named nothing
#define FUI_TEXTEDITOR__DEFAULT_TAB_SIZE 4

//! How many digits the gutter is wide even for a short document, so its left edge is a straight one
#define FUI_TEXTEDITOR__DEFAULT_GUTTER_MIN_DIGITS 3

//! How thick the hairline between the gutter and the text is
#define FUI_TEXTEDITOR__GUTTER_SEPARATOR_THICKNESS 1.0f

//! How many lines one notch of the wheel moves the view
#define FUI_TEXTEDITOR__WHEEL_LINES 3.0f

//! How long after a press a second one still counts as a double click, in seconds
#define FUI_TEXTEDITOR__MULTI_CLICK_SECONDS 0.4f

//! How far a second press may land from the first and still count as a double click, in pixels
#define FUI_TEXTEDITOR__MULTI_CLICK_SLOP 4.0f

//! How fast a drag past the edge scrolls, in lines per second per line of overshoot
#define FUI_TEXTEDITOR__AUTOSCROLL_LINES_PER_SECOND 8.0f

//! How wide the caret is drawn when the caller named nothing
#define FUI_TEXTEDITOR__DEFAULT_CARET_WIDTH 2.0f

//! How solid the whitespace marks are drawn when the caller named no color
#define FUI_TEXTEDITOR__WHITESPACE_ALPHA 0.45f

//! How wide the dot in a blank is, as a fraction of the blank
#define FUI_TEXTEDITOR__WHITESPACE_DOT_RATIO 0.16f

//! How thick the arrow across a tab is drawn
#define FUI_TEXTEDITOR__WHITESPACE_ARROW_THICKNESS 1.0f

//! How long the two strokes of the arrow head are, as a fraction of the line height
#define FUI_TEXTEDITOR__WHITESPACE_ARROW_HEAD_RATIO 0.2f

//! How far the arrow stays clear of both ends of the tab it spans, as a fraction of a blank
#define FUI_TEXTEDITOR__WHITESPACE_ARROW_INSET_RATIO 0.2f

//! How wide the marker at the left edge of the gutter is drawn
#define FUI_TEXTEDITOR__GUTTER_MARKER_WIDTH 4.0f

//! How far apart two character widths may measure and still count as the same width
#define FUI_TEXTEDITOR__MONOSPACE_TOLERANCE 0.01f

//! How close to a tab stop counts as being ON it, in stops, so that landing on one moves to the next
#define FUI_TEXTEDITOR__TAB_STOP_EPSILON 0.001f

//! How much lighter than the text background the gutter is drawn when the caller named no color
#define FUI_TEXTEDITOR__GUTTER_SHADE 0.06f

//! How solid the wash over the current line is when the caller named no color
#define FUI_TEXTEDITOR__CURRENT_LINE_ALPHA 0.30f

//! What a digit is assumed to be wide, as a fraction of the font height, before one has been measured
#define FUI_TEXTEDITOR__ASSUMED_DIGIT_WIDTH_RATIO 0.6f

//! How long the editor's own status line may get, which is a handful of numbers and names
#define FUI_TEXTEDITOR__MAX_STATUS_TEXT 160

//! How long a line number may get, which is the digits of an int32 and its sign
#define FUI_TEXTEDITOR__MAX_NUMBER_TEXT 16

fui_api fuiEditorConfig fuiEditorDefaultConfig(void) {
	fuiEditorConfig result;
	FUI_TEXTEDITOR_MEMSET(&result, 0, sizeof(result));

	// Everything but the toggles stays at zero, because zero is what "take it from the theme, or take the
	// default named on the field" is spelled as. Only what an editor is EXPECTED to show is turned on here.
	result.toggles.showLineNumbers = true;
	result.toggles.showStatusBar = true;
	result.toggles.highlightCurrentLine = true;
	result.toggles.isInteractive = true;

	// The vertical bar is reserved whether it is needed or not, because a document that is being typed into
	// crosses the "one line more than fits" boundary constantly, and every crossing would shift every line
	// of text sideways. The horizontal one appears only when it is needed: a permanent strip along the
	// bottom of an editor whose lines all fit is a cost paid for a rare case.
	result.toggles.verticalScrollbar = fuiEditorScrollbarMode_Always;
	result.toggles.horizontalScrollbar = fuiEditorScrollbarMode_Auto;
	return(result);
}

fui_api void fuiEditorSetConfig(fuiEditor *editor, const fuiEditorConfig *config) {
	if(editor == fui_null) {
		return;
	}
	if(config != fui_null) {
		editor->config = *config;
	} else {
		editor->config = fuiEditorDefaultConfig();
	}
	editor->hasResolvedConfig = false;
}

fui_api const fuiEditorConfig *fuiEditorGetConfig(const fuiEditor *editor) {
	if(editor == fui_null) {
		return(fui_null);
	}
	return(&editor->config);
}

//! A color the caller left at zero alpha is one they did not name, and the theme answers for it instead
fui_inline fuiColor fuiEditor__ResolveColor(const fuiColor wanted, const fuiColor fallback) {
	if(wanted.a > 0.0f) {
		return(wanted);
	}
	return(fallback);
}

//! Same for a measurement, where zero rather than zero alpha is what "not named" looks like
fui_inline float fuiEditor__ResolveLength(const float wanted, const float fallback) {
	if(wanted > 0.0f) {
		return(wanted);
	}
	return(fallback);
}

//! And for a count
fui_inline int32_t fuiEditor__ResolveCount(const int32_t wanted, const int32_t fallback) {
	if(wanted > 0) {
		return(wanted);
	}
	return(fallback);
}

/*
	Fills every zero in the caller's configuration in from the theme and from the defaults, ONCE.

	The rule everywhere else in final_ui.h is that a zeroed field is worked out where it is used, which is
	right for a description of four fields and wrong for one of forty: forty tests per frame, every frame,
	to arrive at the same forty answers. So the answers are worked out here and kept.

	Kept against the theme they were worked out FROM, so that a context which is restyled between two
	frames is noticed. Comparing the theme costs one memcmp of a few hundred bytes a frame, which is
	nothing beside the alternative - colors that silently stay the old ones until somebody thinks to call
	fuiEditorSetConfig again.
*/
static void fuiEditor__ResolveConfig(fuiEditor *editor, const fuiTheme *theme) {
	bool themeIsTheSameOne = false;
	if(editor->hasResolvedConfig) {
		int themeComparison = FUI_TEXTEDITOR_MEMCMP(&editor->resolvedTheme, theme, sizeof(*theme));
		themeIsTheSameOne = (themeComparison == 0);
	}
	if(themeIsTheSameOne) {
		return;
	}

	fuiEditorConfig resolved = editor->config;

	fuiColor textBackground = theme->widgetTrackColor;
	fuiColor gutterBackground = fuiColorShade(textBackground, FUI_TEXTEDITOR__GUTTER_SHADE);
	fuiColor currentLineWash = fuiColorWithAlpha(theme->menuHighlightColor, FUI_TEXTEDITOR__CURRENT_LINE_ALPHA);

	resolved.colors.background = fuiEditor__ResolveColor(editor->config.colors.background, textBackground);
	resolved.colors.border = fuiEditor__ResolveColor(editor->config.colors.border, theme->panelBorderColor);
	resolved.colors.text = fuiEditor__ResolveColor(editor->config.colors.text, theme->textColor);
	resolved.colors.gutterBackground = fuiEditor__ResolveColor(editor->config.colors.gutterBackground, gutterBackground);
	resolved.colors.gutterText = fuiEditor__ResolveColor(editor->config.colors.gutterText, theme->textMutedColor);
	resolved.colors.gutterCurrentLineText = fuiEditor__ResolveColor(editor->config.colors.gutterCurrentLineText, theme->accentColor);
	resolved.colors.gutterSeparator = fuiEditor__ResolveColor(editor->config.colors.gutterSeparator, theme->treeGuideColor);
	resolved.colors.currentLineBackground = fuiEditor__ResolveColor(editor->config.colors.currentLineBackground, currentLineWash);
	resolved.colors.selectionBackground = fuiEditor__ResolveColor(editor->config.colors.selectionBackground, theme->textSelectionColor);
	resolved.colors.caret = fuiEditor__ResolveColor(editor->config.colors.caret, theme->accentColor);
	fuiColor faintMarkColor = fuiColorWithAlpha(theme->textMutedColor, FUI_TEXTEDITOR__WHITESPACE_ALPHA);
	resolved.colors.whitespace = fuiEditor__ResolveColor(editor->config.colors.whitespace, faintMarkColor);
	resolved.colors.statusBarBackground = fuiEditor__ResolveColor(editor->config.colors.statusBarBackground, theme->widgetColor);
	resolved.colors.statusBarText = fuiEditor__ResolveColor(editor->config.colors.statusBarText, theme->textMutedColor);

	resolved.metrics.fontHeight = fuiEditor__ResolveLength(editor->config.metrics.fontHeight, theme->fontHeight);
	resolved.metrics.lineSpacing = fuiEditor__ResolveLength(editor->config.metrics.lineSpacing, 0.0f);
	resolved.metrics.textPaddingX = fuiEditor__ResolveLength(editor->config.metrics.textPaddingX, theme->widgetPaddingX);
	resolved.metrics.gutterPaddingX = fuiEditor__ResolveLength(editor->config.metrics.gutterPaddingX, theme->widgetPaddingX);
	resolved.metrics.statusBarHeight = fuiEditor__ResolveLength(editor->config.metrics.statusBarHeight, theme->menuItemHeight);
	resolved.metrics.caretWidth = fuiEditor__ResolveLength(editor->config.metrics.caretWidth, FUI_TEXTEDITOR__DEFAULT_CARET_WIDTH);
	resolved.metrics.tabSize = fuiEditor__ResolveCount(editor->config.metrics.tabSize, FUI_TEXTEDITOR__DEFAULT_TAB_SIZE);
	resolved.metrics.gutterMinDigits = fuiEditor__ResolveCount(editor->config.metrics.gutterMinDigits, FUI_TEXTEDITOR__DEFAULT_GUTTER_MIN_DIGITS);

	editor->resolvedConfig = resolved;
	editor->resolvedTheme = *theme;
	editor->hasResolvedConfig = true;
}

// ----------------------------------------------------------------------------
// > Screen lines
// ----------------------------------------------------------------------------

/*
	A SCREEN line is one row of the view; a DOCUMENT line is one line of the text. They are the same thing
	while nothing wraps, and they are told apart from here on anyway - because turning the one into the
	other after the fact would mean touching every line of the widget at once.
*/

//! How many rows the whole document takes up on screen
fui_inline int32_t fuiEditor__GetScreenLineCount(const fuiEditor *editor) {
	int32_t documentLineCount = fuiEditorGetLineCount(editor);
	return(documentLineCount);
}

//! Which document line is drawn on a screen row
fui_inline int32_t fuiEditor__DocumentLineOfScreenLine(const fuiEditor *editor, const int32_t screenLine) {
	int32_t documentLineCount = fuiEditorGetLineCount(editor);
	if(documentLineCount <= 0) {
		return(0);
	}
	return(fuiEditor__ClampI32(screenLine, 0, documentLineCount - 1));
}

//! Which screen row a document line begins on
fui_inline int32_t fuiEditor__ScreenLineOfDocumentLine(const fuiEditor *editor, const int32_t documentLine) {
	int32_t documentLineCount = fuiEditorGetLineCount(editor);
	if(documentLineCount <= 0) {
		return(0);
	}
	return(fuiEditor__ClampI32(documentLine, 0, documentLineCount - 1));
}

//! Whether a screen row is the one its document line STARTS on, which is the only row that gets a number
fui_inline bool fuiEditor__ScreenLineCarriesItsNumber(const fuiEditor *editor, const int32_t screenLine) {
	(void)editor;
	(void)screenLine;
	return(true);
}

/*
	Which screen rows a scroll offset puts inside the view.

	Widened by one row at each end on purpose: the first row is the one the offset lands INSIDE, and the
	last is the one the bottom edge cuts through. Neither is fully visible and both have to be drawn.
*/
static void fuiEditor__VisibleScreenLines(const float scroll, const float viewportHeight, const float lineHeight, const int32_t screenLineCount, int32_t *outFirstScreenLine, int32_t *outEndScreenLine) {
	*outFirstScreenLine = 0;
	*outEndScreenLine = screenLineCount;
	if(lineHeight <= 0.0f) {
		return;
	}

	int32_t firstScreenLine = (int32_t)(scroll / lineHeight);
	firstScreenLine = fuiEditor__ClampI32(firstScreenLine, 0, screenLineCount);

	int32_t rowsThatFit = (int32_t)(viewportHeight / lineHeight) + 2;
	int32_t endScreenLine = firstScreenLine + rowsThatFit;
	if(endScreenLine > screenLineCount) {
		endScreenLine = screenLineCount;
	}

	*outFirstScreenLine = firstScreenLine;
	*outEndScreenLine = endScreenLine;
}

// ----------------------------------------------------------------------------
// > Reaching the bytes
// ----------------------------------------------------------------------------

/*
	Hands back a pointer to as many bytes as lie in ONE unbroken piece of the buffer from offset onwards.

	The hole splits the document in at most two, so any range is at most two of these - which is what lets
	a line be drawn straight out of the buffer instead of being copied into a scratch buffer first, and
	means no line is ever too long to draw.
*/
static const char *fuiEditor__ContiguousRunAt(const fuiEditor *editor, const int32_t offset, const int32_t limit, int32_t *outRunLength) {
	*outRunLength = 0;
	if(editor == fui_null || !editor->isInitialized) {
		return(fui_null);
	}

	const fuiEditorDocument *document = &editor->document;
	int32_t textLength = fuiEditor__DocumentLength(document);
	int32_t runStart = fuiEditor__ClampI32(offset, 0, textLength);
	int32_t runLimit = fuiEditor__ClampI32(limit, runStart, textLength);
	if(runStart >= runLimit) {
		return(fui_null);
	}

	int32_t runEnd = runLimit;
	bool startsInFrontOfTheHole = (runStart < document->gapStart);
	bool endsBehindTheHole = (runLimit > document->gapStart);
	if(startsInFrontOfTheHole && endsBehindTheHole) {
		runEnd = document->gapStart;
	}

	int32_t physicalStart = fuiEditor__DocumentPhysicalOffset(document, runStart);
	*outRunLength = runEnd - runStart;
	return(&document->bytes[physicalStart]);
}

//! How many codepoints a run of utf-8 holds, which is every byte that is not a continuation of the one before it
fui_inline int32_t fuiEditor__CountCodepoints(const char *bytes, const int32_t byteCount) {
	int32_t codepointCount = 0;
	for(int32_t byteIndex = 0; byteIndex < byteCount; ++byteIndex) {
		bool isAContinuationByte = fuiEditor__IsUtf8Continuation(bytes[byteIndex]);
		if(!isAContinuationByte) {
			codepointCount += 1;
		}
	}
	return(codepointCount);
}

// ----------------------------------------------------------------------------
// > Numbers as text
// ----------------------------------------------------------------------------

/*
	The editor writes line numbers and a status line, and that is all the formatting it does. A dependency
	on <stdio.h> for two integers would be a strange price, and final_ui.h's own FUI_VSNPRINTF lives inside
	its implementation block - which this add-on is allowed to be compiled apart from.
*/

//! Writes a number into a buffer, terminator included, and answers how many characters it took
static int32_t fuiEditor__FormatInt(char *destination, const int32_t destinationCapacity, const int32_t value) {
	if(destination == fui_null || destinationCapacity <= 1) {
		return(0);
	}

	char digitsBackwards[FUI_TEXTEDITOR__MAX_NUMBER_TEXT];
	int32_t digitCount = 0;
	bool isNegative = (value < 0);

	// Negated as a wider type, because the most negative int32 has no positive counterpart of its own.
	int64_t remaining = (int64_t)value;
	if(isNegative) {
		remaining = -remaining;
	}
	if(remaining == 0) {
		digitsBackwards[digitCount] = '0';
		digitCount += 1;
	}
	const int32_t decimalBase = 10;
	int32_t maximumDigits = (int32_t)sizeof(digitsBackwards);
	while(remaining > 0 && digitCount < maximumDigits) {
		int32_t digit = (int32_t)(remaining % decimalBase);
		digitsBackwards[digitCount] = (char)('0' + digit);
		digitCount += 1;
		remaining /= decimalBase;
	}

	int32_t writeOffset = 0;
	int32_t roomForCharacters = destinationCapacity - 1;
	if(isNegative && writeOffset < roomForCharacters) {
		destination[writeOffset] = '-';
		writeOffset += 1;
	}
	while(digitCount > 0 && writeOffset < roomForCharacters) {
		digitCount -= 1;
		destination[writeOffset] = digitsBackwards[digitCount];
		writeOffset += 1;
	}
	destination[writeOffset] = '\0';
	return(writeOffset);
}

//! Appends text at a write offset and answers the new one, terminating whatever it managed to write
static int32_t fuiEditor__AppendText(char *destination, const int32_t destinationCapacity, const int32_t writeOffset, const char *text) {
	if(destination == fui_null || destinationCapacity <= 1 || text == fui_null) {
		return(writeOffset);
	}
	int32_t roomForCharacters = destinationCapacity - 1;
	int32_t currentOffset = fuiEditor__ClampI32(writeOffset, 0, roomForCharacters);
	int32_t readOffset = 0;
	while(text[readOffset] != '\0' && currentOffset < roomForCharacters) {
		destination[currentOffset] = text[readOffset];
		currentOffset += 1;
		readOffset += 1;
	}
	destination[currentOffset] = '\0';
	return(currentOffset);
}

//! The same for a number
static int32_t fuiEditor__AppendInt(char *destination, const int32_t destinationCapacity, const int32_t writeOffset, const int32_t value) {
	char numberText[FUI_TEXTEDITOR__MAX_NUMBER_TEXT];
	const int32_t numberCapacity = (int32_t)sizeof(numberText);
	(void)fuiEditor__FormatInt(numberText, numberCapacity, value);
	return(fuiEditor__AppendText(destination, destinationCapacity, writeOffset, numberText));
}

//! How many digits a number is written with, which is what the gutter is sized by
fui_inline int32_t fuiEditor__DigitCount(const int32_t value) {
	int32_t digitCount = 1;
	int32_t remaining = value;
	if(remaining < 0) {
		remaining = -remaining;
	}
	const int32_t decimalBase = 10;
	while(remaining >= decimalBase) {
		remaining /= decimalBase;
		digitCount += 1;
	}
	return(digitCount);
}

// ----------------------------------------------------------------------------
// > Measuring text
// ----------------------------------------------------------------------------

/**
* @struct fuiEditor__Render
* @brief The handful of numbers every line of one build is drawn by, worked out once at the top of it.
*/
typedef struct fuiEditor__Render {
	//! Pixel height the text is drawn at
	float fontHeight;
	//! How far apart two lines are, the caller's extra spacing included
	float lineHeight;
	//! Width of one character, which only means anything when the face is a monospace one
	float characterWidth;
	//! How far apart two tab stops are, in pixels
	float tabWidth;
	//! Width of a digit, which is what the gutter is sized by
	float digitWidth;
	//! Width of a blank, which is what a selected line break is shown as
	float spaceWidth;
	//! Whether every character of the face is the same width, which turns measuring into a multiplication
	bool isMonospace;
} fuiEditor__Render;

//! Measures the face once per build and decides whether it is a monospace one
static fuiEditor__Render fuiEditor__MakeRender(fuiContext *context, const fuiEditorConfig *config) {
	fuiEditor__Render result;
	FUI_TEXTEDITOR_MEMSET(&result, 0, sizeof(result));

	result.fontHeight = config->metrics.fontHeight;

	float fontLineHeight = fuiGetLineHeight(context, result.fontHeight);
	result.lineHeight = fontLineHeight + config->metrics.lineSpacing;

	/*
		A face is a monospace one when its widest and its narrowest character measure the same. That is
		worth asking, because it turns "how wide is this run" from a walk over it into one multiplication -
		which is the difference between a long line of code costing what its length is and costing the
		square of it, once the caret has to be placed inside it.
	*/
	const size_t oneCharacter = 1;
	fuiVec2 wideSize = fuiMeasureText(context, "W", oneCharacter, result.fontHeight);
	fuiVec2 narrowSize = fuiMeasureText(context, "i", oneCharacter, result.fontHeight);
	fuiVec2 spaceSize = fuiMeasureText(context, " ", oneCharacter, result.fontHeight);
	fuiVec2 digitSize = fuiMeasureText(context, "0", oneCharacter, result.fontHeight);

	float widthDifference = wideSize.x - narrowSize.x;
	if(widthDifference < 0.0f) {
		widthDifference = -widthDifference;
	}
	result.isMonospace = (wideSize.x > 0.0f) && (widthDifference <= FUI_TEXTEDITOR__MONOSPACE_TOLERANCE);
	result.characterWidth = wideSize.x;
	result.digitWidth = digitSize.x;
	result.spaceWidth = spaceSize.x;

	// A tab stop is measured in SPACES rather than in the widest character, because that is what a tab stop
	// is taken to be everywhere else. On a monospace face the two are the same number anyway.
	float tabCharacterWidth = spaceSize.x;
	if(tabCharacterWidth <= 0.0f) {
		tabCharacterWidth = result.characterWidth;
	}
	result.tabWidth = tabCharacterWidth * (float)config->metrics.tabSize;
	return(result);
}

//! How wide a run of text is, by multiplication on a monospace face and by measuring on any other
fui_inline float fuiEditor__MeasureRun(fuiContext *context, const fuiEditor__Render *render, const char *bytes, const int32_t byteCount) {
	if(byteCount <= 0) {
		return(0.0f);
	}
	if(render->isMonospace) {
		int32_t codepointCount = fuiEditor__CountCodepoints(bytes, byteCount);
		return((float)codepointCount * render->characterWidth);
	}
	fuiVec2 measured = fuiMeasureText(context, bytes, (size_t)byteCount, render->fontHeight);
	return(measured.x);
}

/*
	How far into the line the pen lands when it hits a tab, which is the next stop counted from the START.

	Counted on the DISTANCE into the line rather than on the pen's own x, and with a hair of tolerance at
	the boundary. Both matter: the pen carries the widget's x, which at a few hundred pixels leaves a float
	too little room to say "exactly one tab along", and a stop that misses its own boundary by a millionth
	is a stop the pen is judged to be just SHORT of - so it is sent to the stop it is already standing on,
	and the second tab of a line moves it nowhere at all.
*/
fui_inline float fuiEditor__NextTabStopDistance(const fuiEditor__Render *render, const float distanceIntoTheLine) {
	if(render->tabWidth <= 0.0f) {
		return(distanceIntoTheLine);
	}
	float stopsAlreadyPassed = distanceIntoTheLine / render->tabWidth;
	int32_t wholeStopsPassed = (int32_t)(stopsAlreadyPassed + FUI_TEXTEDITOR__TAB_STOP_EPSILON);
	if(wholeStopsPassed < 0) {
		wholeStopsPassed = 0;
	}
	return((float)(wholeStopsPassed + 1) * render->tabWidth);
}

// ----------------------------------------------------------------------------
// > Line geometry
// ----------------------------------------------------------------------------

/*
	One line, cut into the pieces that are drawn and measured in one go.

	Two things cut a line up, and both are handled here so that nothing else has to know about either: the
	HOLE, which may sit anywhere inside it, and the TABS, which are not drawn at all and instead jump the
	pen to the next stop. Everything that asks a question about a line - where to draw it, how wide it is,
	how far into it an offset sits, which offset a distance lands on - walks these same pieces, which is
	what keeps the caret standing where the glyphs really are.
*/

//! One piece of a line
typedef struct fuiEditor__LineSegment {
	//! The bytes, pointing straight into the document. Null for a tab, which is not drawn at all
	const char *bytes;
	//! How many bytes there are, and one for a tab
	int32_t byteCount;
	//! The document offset the piece starts at
	int32_t offset;
	//! Whether this piece is a tab, which jumps the pen rather than drawing anything
	bool isTab;
} fuiEditor__LineSegment;

//! Where a walk over a line's pieces has got to
typedef struct fuiEditor__LineCursor {
	//! The document being walked
	const fuiEditor *editor;
	//! The document offset the next piece starts at
	int32_t offset;
	//! Where the line ends
	int32_t lineEnd;
	//! The unbroken piece of buffer the walk is inside of
	const char *runBytes;
	//! The document offset that piece starts at
	int32_t runOffset;
	//! How long it is
	int32_t runLength;
} fuiEditor__LineCursor;

static fuiEditor__LineCursor fuiEditor__BeginLineWalk(const fuiEditor *editor, const int32_t lineStart, const int32_t lineEnd) {
	fuiEditor__LineCursor result;
	FUI_TEXTEDITOR_MEMSET(&result, 0, sizeof(result));
	result.editor = editor;
	result.offset = lineStart;
	result.lineEnd = lineEnd;
	return(result);
}

static bool fuiEditor__NextLineSegment(fuiEditor__LineCursor *cursor, fuiEditor__LineSegment *outSegment) {
	if(cursor->offset >= cursor->lineEnd) {
		return(false);
	}

	int32_t runEnd = cursor->runOffset + cursor->runLength;
	bool theRunIsUsedUp = (cursor->runBytes == fui_null) || (cursor->offset >= runEnd);
	if(theRunIsUsedUp) {
		int32_t runLength = 0;
		const char *runBytes = fuiEditor__ContiguousRunAt(cursor->editor, cursor->offset, cursor->lineEnd, &runLength);
		if(runBytes == fui_null || runLength <= 0) {
			return(false);
		}
		cursor->runBytes = runBytes;
		cursor->runOffset = cursor->offset;
		cursor->runLength = runLength;
	}

	int32_t offsetIntoTheRun = cursor->offset - cursor->runOffset;
	int32_t bytesLeftInTheRun = cursor->runLength - offsetIntoTheRun;
	const char *pieceBytes = &cursor->runBytes[offsetIntoTheRun];

	if(pieceBytes[0] == '\t') {
		outSegment->bytes = fui_null;
		outSegment->byteCount = 1;
		outSegment->offset = cursor->offset;
		outSegment->isTab = true;
		cursor->offset += 1;
		return(true);
	}

	const char *foundTab = (const char *)FUI_TEXTEDITOR_MEMCHR(pieceBytes, '\t', (size_t)bytesLeftInTheRun);
	int32_t plainLength = bytesLeftInTheRun;
	if(foundTab != fui_null) {
		plainLength = (int32_t)(foundTab - pieceBytes);
	}
	outSegment->bytes = pieceBytes;
	outSegment->byteCount = plainLength;
	outSegment->offset = cursor->offset;
	outSegment->isTab = false;
	cursor->offset += plainLength;
	return(true);
}

/**
* @struct fuiEditor__LinePaint
* @brief Everything one line is drawn WITH, worked out before it is walked.
*/
typedef struct fuiEditor__LinePaint {
	//! One style byte per byte of the line, or null when nothing coloured it
	const uint8_t *styles;
	//! How many of them there are
	int32_t styleLength;
	//! What text with nothing said about it is drawn in
	fuiColor defaultColor;
	//! What the dots and arrows are drawn in
	fuiColor whitespaceColor;
	//! Whether blanks and tabs get a mark of their own
	bool showWhitespace;
} fuiEditor__LinePaint;

//! What a lexer said about one byte, or the default for a byte it said nothing about
fui_inline uint8_t fuiEditor__StyleAt(const fuiEditor__LinePaint *paint, const int32_t offsetIntoTheLine) {
	bool thereIsOne = (paint->styles != fui_null) && (offsetIntoTheLine >= 0) && (offsetIntoTheLine < paint->styleLength);
	if(!thereIsOne) {
		return((uint8_t)FUI_TEXTEDITOR_STYLE_DEFAULT);
	}
	return(paint->styles[offsetIntoTheLine]);
}

//! A small square in the middle of every blank of a run, placed by multiplication rather than by measuring
static void fuiEditor__DrawBlankMarks(fuiContext *context, const fuiEditor__Render *render, const float runLeftX, const float lineTopY, const int32_t blankCount, const fuiColor color) {
	float dotSize = render->spaceWidth * FUI_TEXTEDITOR__WHITESPACE_DOT_RATIO;
	if(dotSize <= 0.0f) {
		return;
	}
	float dotTop = lineTopY + (render->lineHeight - dotSize) * 0.5f;
	float dotInset = (render->spaceWidth - dotSize) * 0.5f;
	for(int32_t blankIndex = 0; blankIndex < blankCount; ++blankIndex) {
		float dotLeft = runLeftX + (float)blankIndex * render->spaceWidth + dotInset;
		fuiRect dotRect = fuiRectMake(dotLeft, dotTop, dotSize, dotSize);
		fuiDrawRect(context, dotRect, color);
	}
}

//! An arrow across the whole width a tab spans, which is what says how far it really reached
static void fuiEditor__DrawTabMark(fuiContext *context, const fuiEditor__Render *render, const float tabLeftX, const float tabRightX, const float lineTopY, const fuiColor color) {
	float arrowInset = render->spaceWidth * FUI_TEXTEDITOR__WHITESPACE_ARROW_INSET_RATIO;
	float arrowLeft = tabLeftX + arrowInset;
	float arrowRight = tabRightX - arrowInset;
	if(arrowRight <= arrowLeft) {
		return;
	}

	float arrowY = lineTopY + render->lineHeight * 0.5f;
	float headSize = render->lineHeight * FUI_TEXTEDITOR__WHITESPACE_ARROW_HEAD_RATIO;
	fuiVec2 shaftStart = fuiV2(arrowLeft, arrowY);
	fuiVec2 shaftEnd = fuiV2(arrowRight, arrowY);
	fuiDrawLine(context, shaftStart, shaftEnd, color, FUI_TEXTEDITOR__WHITESPACE_ARROW_THICKNESS);

	fuiVec2 upperHeadStart = fuiV2(arrowRight - headSize, arrowY - headSize);
	fuiVec2 lowerHeadStart = fuiV2(arrowRight - headSize, arrowY + headSize);
	fuiDrawLine(context, upperHeadStart, shaftEnd, color, FUI_TEXTEDITOR__WHITESPACE_ARROW_THICKNESS);
	fuiDrawLine(context, lowerHeadStart, shaftEnd, color, FUI_TEXTEDITOR__WHITESPACE_ARROW_THICKNESS);
}

/*
	Draws one line and answers how wide it came out.

	Every piece the walk hands over is cut again wherever what it is drawn WITH changes: at a style
	boundary, and - when whitespace is being shown - at the edge of a run of blanks, because a run of
	blanks gets its dots placed inside it. Each of those runs is measured as a PREFIX of the piece it
	belongs to rather than on its own, so the widths telescope back to exactly what the whole piece
	measures. That is what keeps the caret, which measures whole pieces, standing where the glyphs are.
*/
static float fuiEditor__DrawLine(fuiContext *context, const fuiEditor *editor, const fuiEditor__Render *render, const int32_t lineStart, const int32_t lineEnd, const float lineLeftX, const float lineTopY, const fuiEditor__LinePaint *paint) {
	float distanceIntoTheLine = 0.0f;
	fuiEditor__LineCursor cursor = fuiEditor__BeginLineWalk(editor, lineStart, lineEnd);
	fuiEditor__LineSegment segment;
	while(fuiEditor__NextLineSegment(&cursor, &segment)) {
		if(segment.isTab) {
			float stopDistance = fuiEditor__NextTabStopDistance(render, distanceIntoTheLine);
			if(paint->showWhitespace) {
				fuiEditor__DrawTabMark(context, render, lineLeftX + distanceIntoTheLine, lineLeftX + stopDistance, lineTopY, paint->whitespaceColor);
			}
			distanceIntoTheLine = stopDistance;
			continue;
		}

		float segmentStartDistance = distanceIntoTheLine;
		float measuredPrefixWidth = 0.0f;
		int32_t runStart = 0;
		while(runStart < segment.byteCount) {
			int32_t styleIndex = (segment.offset - lineStart) + runStart;
			uint8_t runStyle = fuiEditor__StyleAt(paint, styleIndex);
			bool runIsBlanks = paint->showWhitespace && (segment.bytes[runStart] == ' ');

			int32_t runEnd = runStart + 1;
			while(runEnd < segment.byteCount) {
				uint8_t styleHere = fuiEditor__StyleAt(paint, (segment.offset - lineStart) + runEnd);
				bool isABlankHere = paint->showWhitespace && (segment.bytes[runEnd] == ' ');
				if(styleHere != runStyle || isABlankHere != runIsBlanks) {
					break;
				}
				runEnd += 1;
			}

			float prefixWidth = fuiEditor__MeasureRun(context, render, segment.bytes, runEnd);
			float runLeftX = lineLeftX + segmentStartDistance + measuredPrefixWidth;
			int32_t runLength = runEnd - runStart;

			fuiColor runColor = fuiEditor__StyleTextColor(editor, runStyle, paint->defaultColor);
			fuiVec2 runPosition = fuiV2(runLeftX, lineTopY);
			fuiDrawText(context, &segment.bytes[runStart], (size_t)runLength, runPosition, render->fontHeight, runColor);

			if(runIsBlanks) {
				fuiEditor__DrawBlankMarks(context, render, runLeftX, lineTopY, runLength, paint->whitespaceColor);
			}

			measuredPrefixWidth = prefixWidth;
			runStart = runEnd;
		}
		distanceIntoTheLine = segmentStartDistance + measuredPrefixWidth;
	}
	return(distanceIntoTheLine);
}

//! Which ending a line really carries, which is what tells a mixed file apart from a clean one
static fuiEditorEol fuiEditor__LineEndingOf(const fuiEditor *editor, const int32_t documentLine) {
	int32_t lineCount = fuiEditorGetLineCount(editor);
	bool isTheLastLine = (documentLine >= (lineCount - 1));
	if(isTheLastLine) {
		return(fuiEditorEol_Lf);
	}

	int32_t lineEnd = fuiEditorGetLineEnd(editor, documentLine);
	int32_t nextLineStart = fuiEditorGetLineStart(editor, documentLine + 1);
	int32_t endingLength = nextLineStart - lineEnd;
	if(endingLength >= 2) {
		return(fuiEditorEol_CrLf);
	}
	return(fuiEditorEol_Lf);
}

/*
	How far the pen moves for ONE codepoint, the kerning against the one in front of it included.

	final_ui.h exposes measuring and not the kerning table behind it, so the kerning of a pair is reached
	as the difference between measuring the pair and measuring the first of the two. That keeps this in
	step with what a whole run measures to, which is what the pen is actually advanced by - a caret worked
	out without the kerning would drift away from the glyphs across a long line.
*/
fui_inline float fuiEditor__CodepointAdvance(fuiContext *context, const fuiEditor__Render *render, const char *bytes, const int32_t previousStart, const int32_t currentStart, const int32_t currentEnd) {
	if(render->isMonospace) {
		return(render->characterWidth);
	}
	int32_t currentLength = currentEnd - currentStart;
	bool thereIsNothingInFront = (previousStart < 0);
	if(thereIsNothingInFront) {
		fuiVec2 aloneSize = fuiMeasureText(context, &bytes[currentStart], (size_t)currentLength, render->fontHeight);
		return(aloneSize.x);
	}
	fuiVec2 pairSize = fuiMeasureText(context, &bytes[previousStart], (size_t)(currentEnd - previousStart), render->fontHeight);
	fuiVec2 firstSize = fuiMeasureText(context, &bytes[previousStart], (size_t)(currentStart - previousStart), render->fontHeight);
	return(pairSize.x - firstSize.x);
}

//! Which offset inside ONE piece a distance lands on, snapped to the nearer codepoint boundary
static int32_t fuiEditor__OffsetInSegment(fuiContext *context, const fuiEditor__Render *render, const fuiEditor__LineSegment *segment, const float segmentStartDistance, const float wantedDistance) {
	float distance = segmentStartDistance;
	int32_t previousStart = -1;
	int32_t currentStart = 0;
	while(currentStart < segment->byteCount) {
		int32_t currentEnd = currentStart + 1;
		while(currentEnd < segment->byteCount && fuiEditor__IsUtf8Continuation(segment->bytes[currentEnd])) {
			currentEnd += 1;
		}

		// Snapped to the NEARER boundary, so clicking on the left half of a character puts the caret in
		// front of it and clicking on the right half puts it behind - which is what a click means.
		float advance = fuiEditor__CodepointAdvance(context, render, segment->bytes, previousStart, currentStart, currentEnd);
		float middleOfTheCharacter = distance + advance * 0.5f;
		if(wantedDistance < middleOfTheCharacter) {
			return(segment->offset + currentStart);
		}

		distance += advance;
		previousStart = currentStart;
		currentStart = currentEnd;
	}
	return(segment->offset + segment->byteCount);
}

/*
	How far into the line the pen stands at a document offset.

	One measurement per PIECE rather than one per character: every piece the offset lies behind is measured
	whole, and the piece it lies inside is measured once up to it. A line with three tabs in it is four
	measurements however long it is.
*/
static float fuiEditor__DistanceOfOffset(fuiContext *context, const fuiEditor *editor, const fuiEditor__Render *render, const int32_t lineStart, const int32_t lineEnd, const int32_t wantedOffset) {
	if(wantedOffset <= lineStart) {
		return(0.0f);
	}

	float distanceIntoTheLine = 0.0f;
	fuiEditor__LineCursor cursor = fuiEditor__BeginLineWalk(editor, lineStart, lineEnd);
	fuiEditor__LineSegment segment;
	while(fuiEditor__NextLineSegment(&cursor, &segment)) {
		if(segment.isTab) {
			if(wantedOffset <= segment.offset) {
				return(distanceIntoTheLine);
			}
			distanceIntoTheLine = fuiEditor__NextTabStopDistance(render, distanceIntoTheLine);
			continue;
		}

		int32_t segmentEnd = segment.offset + segment.byteCount;
		if(wantedOffset >= segmentEnd) {
			distanceIntoTheLine += fuiEditor__MeasureRun(context, render, segment.bytes, segment.byteCount);
			continue;
		}

		int32_t prefixLength = wantedOffset - segment.offset;
		distanceIntoTheLine += fuiEditor__MeasureRun(context, render, segment.bytes, prefixLength);
		return(distanceIntoTheLine);
	}
	return(distanceIntoTheLine);
}

//! How wide one line is in total
fui_inline float fuiEditor__LineWidth(fuiContext *context, const fuiEditor *editor, const fuiEditor__Render *render, const int32_t lineStart, const int32_t lineEnd) {
	return(fuiEditor__DistanceOfOffset(context, editor, render, lineStart, lineEnd, lineEnd));
}

//! Which document offset a distance into the line lands on
static int32_t fuiEditor__OffsetAtDistance(fuiContext *context, const fuiEditor *editor, const fuiEditor__Render *render, const int32_t lineStart, const int32_t lineEnd, const float wantedDistance) {
	if(wantedDistance <= 0.0f) {
		return(lineStart);
	}

	float distanceIntoTheLine = 0.0f;
	fuiEditor__LineCursor cursor = fuiEditor__BeginLineWalk(editor, lineStart, lineEnd);
	fuiEditor__LineSegment segment;
	while(fuiEditor__NextLineSegment(&cursor, &segment)) {
		if(segment.isTab) {
			// A tab is one character that happens to be as wide as the gap to its stop, so its own middle
			// is what decides whether the caret goes in front of it or behind it.
			float stopDistance = fuiEditor__NextTabStopDistance(render, distanceIntoTheLine);
			float middleOfTheTab = (distanceIntoTheLine + stopDistance) * 0.5f;
			if(wantedDistance < middleOfTheTab) {
				return(segment.offset);
			}
			if(wantedDistance < stopDistance) {
				return(segment.offset + 1);
			}
			distanceIntoTheLine = stopDistance;
			continue;
		}

		float segmentWidth = fuiEditor__MeasureRun(context, render, segment.bytes, segment.byteCount);
		bool theDistanceIsBehindThisPiece = (wantedDistance >= (distanceIntoTheLine + segmentWidth));
		if(theDistanceIsBehindThisPiece) {
			distanceIntoTheLine += segmentWidth;
			continue;
		}
		return(fuiEditor__OffsetInSegment(context, render, &segment, distanceIntoTheLine, wantedDistance));
	}
	return(lineEnd);
}

// ----------------------------------------------------------------------------
// > Caret and selection
// ----------------------------------------------------------------------------

fui_api int32_t fuiEditorGetCaretOffset(const fuiEditor *editor) {
	if(editor == fui_null || !editor->isInitialized) {
		return(0);
	}
	return(editor->caretOffset);
}

fui_api int32_t fuiEditorGetSelectionStart(const fuiEditor *editor) {
	if(editor == fui_null || !editor->isInitialized) {
		return(0);
	}
	return(fuiEditor__MinI32(editor->selectionAnchor, editor->caretOffset));
}

fui_api int32_t fuiEditorGetSelectionEnd(const fuiEditor *editor) {
	if(editor == fui_null || !editor->isInitialized) {
		return(0);
	}
	return(fuiEditor__MaxI32(editor->selectionAnchor, editor->caretOffset));
}

fui_api bool fuiEditorHasSelection(const fuiEditor *editor) {
	int32_t selectionStart = fuiEditorGetSelectionStart(editor);
	int32_t selectionEnd = fuiEditorGetSelectionEnd(editor);
	return(selectionEnd > selectionStart);
}

fui_api void fuiEditorClearSelection(fuiEditor *editor) {
	if(editor == fui_null || !editor->isInitialized) {
		return;
	}
	editor->selectionAnchor = editor->caretOffset;
}

/*
	The one place the caret ever moves.

	Everything that moves it - a key, a click, a drag, the caller - comes through here, so that the three
	things that always go with a move happen once rather than at every call site: the offset is pulled onto
	a codepoint boundary, the anchor follows unless the selection is being extended, and the blink is reset
	to SOLID. A caret that happens to be in its dark half while somebody is typing looks like the keystroke
	was lost.
*/
static void fuiEditor__MoveCaretTo(fuiEditor *editor, const int32_t offset, const bool extendSelection, const bool keepDesiredDistance) {
	int32_t textLength = fuiEditorGetTextLength(editor);
	int32_t clampedOffset = fuiEditor__ClampI32(offset, 0, textLength);
	int32_t snappedOffset = fuiEditorSnapToCodepointStart(editor, clampedOffset);

	editor->caretOffset = snappedOffset;
	if(!extendSelection) {
		editor->selectionAnchor = snappedOffset;
	}
	if(!keepDesiredDistance) {
		editor->hasDesiredDistance = false;
	}
	editor->caretBlinkTime = 0.0f;
}

fui_api void fuiEditorSetCaretOffset(fuiEditor *editor, const int32_t offset, const bool extendSelection) {
	if(editor == fui_null || !editor->isInitialized) {
		return;
	}
	const bool dropTheDesiredDistance = false;
	fuiEditor__MoveCaretTo(editor, offset, extendSelection, dropTheDesiredDistance);
}

fui_api void fuiEditorSetSelection(fuiEditor *editor, const int32_t anchorOffset, const int32_t caretOffset) {
	if(editor == fui_null || !editor->isInitialized) {
		return;
	}
	int32_t textLength = fuiEditorGetTextLength(editor);
	int32_t clampedAnchor = fuiEditor__ClampI32(anchorOffset, 0, textLength);
	editor->selectionAnchor = fuiEditorSnapToCodepointStart(editor, clampedAnchor);

	const bool keepTheAnchorJustSet = true;
	const bool dropTheDesiredDistance = false;
	fuiEditor__MoveCaretTo(editor, caretOffset, keepTheAnchorJustSet, dropTheDesiredDistance);
}

fui_api void fuiEditorSelectAll(fuiEditor *editor) {
	int32_t textLength = fuiEditorGetTextLength(editor);
	const int32_t fromTheStart = 0;
	fuiEditorSetSelection(editor, fromTheStart, textLength);
}

fui_api int32_t fuiEditorCopySelection(const fuiEditor *editor, char *destination, const int32_t destinationCapacity) {
	int32_t selectionStart = fuiEditorGetSelectionStart(editor);
	int32_t selectionEnd = fuiEditorGetSelectionEnd(editor);
	int32_t selectionLength = selectionEnd - selectionStart;
	return(fuiEditorCopyRange(editor, selectionStart, selectionLength, destination, destinationCapacity));
}

fui_api int32_t fuiEditorGetCaretLine(const fuiEditor *editor) {
	if(editor == fui_null || !editor->isInitialized) {
		return(0);
	}
	return(fuiEditorGetLineOfOffset(editor, editor->caretOffset));
}

fui_api int32_t fuiEditorGetCaretColumn(const fuiEditor *editor) {
	if(editor == fui_null || !editor->isInitialized) {
		return(0);
	}
	int32_t caretLine = fuiEditorGetCaretLine(editor);
	int32_t lineStart = fuiEditorGetLineStart(editor, caretLine);

	int32_t column = 0;
	int32_t offset = lineStart;
	while(offset < editor->caretOffset) {
		char currentByte = fuiEditorGetByte(editor, offset);
		if(!fuiEditor__IsUtf8Continuation(currentByte)) {
			column += 1;
		}
		offset += 1;
	}
	return(column);
}

fui_api void fuiEditorSetCaretLine(fuiEditor *editor, const int32_t documentLine) {
	if(editor == fui_null || !editor->isInitialized) {
		return;
	}
	int32_t lineCount = fuiEditorGetLineCount(editor);
	if(lineCount <= 0) {
		fuiEditorSetCaretOffset(editor, 0, false);
		return;
	}
	int32_t clampedLine = fuiEditor__ClampI32(documentLine, 0, lineCount - 1);
	int32_t lineStart = fuiEditorGetLineStart(editor, clampedLine);
	const bool dropTheSelection = false;
	fuiEditorSetCaretOffset(editor, lineStart, dropTheSelection);
}

// ----------------------------------------------------------------------------
// > Words
// ----------------------------------------------------------------------------

//! What kind of character a byte is, which is all a word jump ever has to know
typedef enum fuiEditor__CharClass {
	//! A blank, which is what lies BETWEEN two words
	fuiEditor__CharClass_Space = 0,
	//! A letter, a digit or an underscore, which is what a word is made of
	fuiEditor__CharClass_Word,
	//! Everything else - brackets, operators, punctuation
	fuiEditor__CharClass_Punctuation,
	//! A line feed, which ends the line and is never crossed by one jump
	fuiEditor__CharClass_LineBreak,
} fuiEditor__CharClass;

fui_inline fuiEditor__CharClass fuiEditor__ClassOfByte(const char byte) {
	unsigned char value = (unsigned char)byte;
	if(value == (unsigned char)'\n') {
		return(fuiEditor__CharClass_LineBreak);
	}
	if(value == (unsigned char)' ' || value == (unsigned char)'\t' || value == (unsigned char)'\r') {
		return(fuiEditor__CharClass_Space);
	}

	bool isADigit = (value >= (unsigned char)'0') && (value <= (unsigned char)'9');
	bool isLowerCase = (value >= (unsigned char)'a') && (value <= (unsigned char)'z');
	bool isUpperCase = (value >= (unsigned char)'A') && (value <= (unsigned char)'Z');
	bool isAnUnderscore = (value == (unsigned char)'_');

	// Everything above ascii counts as part of a word. It is a letter far more often than not, and the
	// alternative is a unicode category table that this add-on has no business carrying around.
	bool isAboveAscii = (value >= 0x80u);
	if(isADigit || isLowerCase || isUpperCase || isAnUnderscore || isAboveAscii) {
		return(fuiEditor__CharClass_Word);
	}
	return(fuiEditor__CharClass_Punctuation);
}

fui_inline fuiEditor__CharClass fuiEditor__ClassAt(const fuiEditor *editor, const int32_t offset) {
	char byteThere = fuiEditorGetByte(editor, offset);
	return(fuiEditor__ClassOfByte(byteThere));
}

/*
	Where the next word starts, counted from an offset.

	A jump never crosses a LINE BREAK: standing at the end of a line, one press puts the caret on the next
	line and a second one takes it to the first word there. Running straight through would make a single
	press land somewhere the eye was not following.
*/
static int32_t fuiEditor__NextWordOffset(const fuiEditor *editor, const int32_t offset) {
	int32_t textLength = fuiEditorGetTextLength(editor);
	int32_t current = fuiEditor__ClampI32(offset, 0, textLength);
	if(current >= textLength) {
		return(textLength);
	}

	fuiEditor__CharClass startClass = fuiEditor__ClassAt(editor, current);
	if(startClass == fuiEditor__CharClass_LineBreak) {
		return(current + 1);
	}

	while(current < textLength) {
		fuiEditor__CharClass classHere = fuiEditor__ClassAt(editor, current);
		if(classHere != startClass) {
			break;
		}
		current += 1;
	}
	while(current < textLength) {
		fuiEditor__CharClass classHere = fuiEditor__ClassAt(editor, current);
		if(classHere != fuiEditor__CharClass_Space) {
			break;
		}
		current += 1;
	}
	return(current);
}

//! Where the word in front of an offset starts, by the same rules read backwards
static int32_t fuiEditor__PreviousWordOffset(const fuiEditor *editor, const int32_t offset) {
	int32_t textLength = fuiEditorGetTextLength(editor);
	int32_t current = fuiEditor__ClampI32(offset, 0, textLength);
	if(current <= 0) {
		return(0);
	}

	fuiEditor__CharClass classBehind = fuiEditor__ClassAt(editor, current - 1);
	if(classBehind == fuiEditor__CharClass_LineBreak) {
		return(current - 1);
	}

	while(current > 0) {
		fuiEditor__CharClass classHere = fuiEditor__ClassAt(editor, current - 1);
		if(classHere != fuiEditor__CharClass_Space) {
			break;
		}
		current -= 1;
	}
	if(current <= 0) {
		return(0);
	}

	fuiEditor__CharClass runClass = fuiEditor__ClassAt(editor, current - 1);
	if(runClass == fuiEditor__CharClass_LineBreak) {
		return(current);
	}
	while(current > 0) {
		fuiEditor__CharClass classHere = fuiEditor__ClassAt(editor, current - 1);
		if(classHere != runClass) {
			break;
		}
		current -= 1;
	}
	return(current);
}

//! The run of one character class an offset stands in, which is what a double click selects
static void fuiEditor__WordRangeAt(const fuiEditor *editor, const int32_t offset, int32_t *outStart, int32_t *outEnd) {
	int32_t textLength = fuiEditorGetTextLength(editor);
	int32_t current = fuiEditor__ClampI32(offset, 0, textLength);

	// At the very end of the document, and at the end of a line, there is no character UNDER the caret -
	// so the word that ends there is the one meant.
	bool thereIsNothingUnderIt = (current >= textLength) || (fuiEditor__ClassAt(editor, current) == fuiEditor__CharClass_LineBreak);
	if(thereIsNothingUnderIt && current > 0) {
		current -= 1;
	}
	if(current >= textLength) {
		*outStart = textLength;
		*outEnd = textLength;
		return;
	}

	fuiEditor__CharClass wantedClass = fuiEditor__ClassAt(editor, current);
	if(wantedClass == fuiEditor__CharClass_LineBreak) {
		*outStart = current;
		*outEnd = current;
		return;
	}

	int32_t rangeStart = current;
	while(rangeStart > 0) {
		fuiEditor__CharClass classHere = fuiEditor__ClassAt(editor, rangeStart - 1);
		if(classHere != wantedClass) {
			break;
		}
		rangeStart -= 1;
	}
	int32_t rangeEnd = current;
	while(rangeEnd < textLength) {
		fuiEditor__CharClass classHere = fuiEditor__ClassAt(editor, rangeEnd);
		if(classHere != wantedClass) {
			break;
		}
		rangeEnd += 1;
	}

	*outStart = rangeStart;
	*outEnd = rangeEnd;
}

//! A whole line INCLUDING its ending, which is what a triple click selects and what pasting it back needs
static void fuiEditor__LineRangeAt(const fuiEditor *editor, const int32_t offset, int32_t *outStart, int32_t *outEnd) {
	int32_t documentLine = fuiEditorGetLineOfOffset(editor, offset);
	int32_t lineCount = fuiEditorGetLineCount(editor);
	int32_t textLength = fuiEditorGetTextLength(editor);

	*outStart = fuiEditorGetLineStart(editor, documentLine);
	bool isTheLastLine = (documentLine >= (lineCount - 1));
	if(isTheLastLine) {
		*outEnd = textLength;
		return;
	}
	*outEnd = fuiEditorGetLineStart(editor, documentLine + 1);
}

// ----------------------------------------------------------------------------
// > The view
// ----------------------------------------------------------------------------

fui_api void fuiEditorScrollToLine(fuiEditor *editor, const int32_t documentLine) {
	if(editor == fui_null || !editor->isInitialized) {
		return;
	}
	// Recorded rather than worked out here. The offset is in PIXELS, and how tall a line is comes from the
	// font the CONTEXT carries - which a document knows nothing about. Guessing it from the font height
	// would be wrong by whatever the face's line spacing is, so the next build, which does know, applies it.
	editor->pendingScrollScreenLine = fuiEditor__ScreenLineOfDocumentLine(editor, documentLine);
	editor->hasPendingScroll = true;
}

fui_api int32_t fuiEditorGetFirstVisibleLine(const fuiEditor *editor) {
	if(editor == fui_null) {
		return(0);
	}
	return(fuiEditor__DocumentLineOfScreenLine(editor, editor->firstVisibleScreenLine));
}

fui_api int32_t fuiEditorGetVisibleLineCount(const fuiEditor *editor) {
	if(editor == fui_null) {
		return(0);
	}
	return(editor->visibleScreenLineCount);
}

/**
* @struct fuiEditor__Layout
* @brief Where every part of one build sits, worked out before anything is drawn.
*/
typedef struct fuiEditor__Layout {
	//! The editor without its status line, which is what carries the outline
	fuiRect frameRect;
	//! Inside the outline, before the scrollbars are taken off it
	fuiRect innerRect;
	//! The strip the line numbers are drawn in, zero wide when there are none
	fuiRect gutterRect;
	//! Where the text goes
	fuiRect textRect;
	//! Gutter and text together, which is what the wheel is asked over and what the current line wash covers
	fuiRect bodyRect;
	//! The vertical scrollbar's track
	fuiRect verticalTrackRect;
	//! The horizontal scrollbar's track
	fuiRect horizontalTrackRect;
	//! The editor's own status line, below everything else
	fuiRect statusBarRect;
	//! Whether there is a vertical bar at all
	bool hasVerticalBar;
	//! Whether there is a horizontal bar at all
	bool hasHorizontalBar;
} fuiEditor__Layout;

//! Whether an axis gets a gutter, which is what "Auto" answers differently from one frame to the next
fui_inline bool fuiEditor__ScrollbarIsThere(const fuiEditorScrollbarMode mode, const bool contentOverflows) {
	if(mode == fuiEditorScrollbarMode_Never) {
		return(false);
	}
	if(mode == fuiEditorScrollbarMode_Always) {
		return(true);
	}
	return(contentOverflows);
}

//! How wide the gutter has to be for the longest line number in the document, at a given digit width
fui_inline float fuiEditor__GutterWidthFor(const fuiEditorConfig *config, const int32_t lineCount, const float digitWidth) {
	int32_t widestNumber = fuiEditor__MaxI32(lineCount, 1);
	int32_t digitsOfTheLastLine = fuiEditor__DigitCount(widestNumber);
	int32_t digitCount = fuiEditor__MaxI32(digitsOfTheLastLine, config->metrics.gutterMinDigits);
	float digitsWidth = (float)digitCount * digitWidth;
	float paddingWidth = config->metrics.gutterPaddingX * 2.0f;
	return(digitsWidth + paddingWidth + FUI_TEXTEDITOR__GUTTER_SEPARATOR_THICKNESS);
}

/*
	Cuts the rectangle up.

	The status line comes off first, then the outline, then the VERTICAL bar against the full inner height,
	and only then the horizontal one against what is left of the width. Asking both at once would be
	circular - each takes away the room the other is measured against - so one order has to win. This one
	does, because a document taller than its box is the common case and a wider one is not.
*/
static fuiEditor__Layout fuiEditor__MakeLayout(const fuiRect rect, const fuiEditorConfig *config, const float gutterWidth, const float contentHeight, const float contentWidth, const float borderThickness) {
	fuiEditor__Layout result;
	FUI_TEXTEDITOR_MEMSET(&result, 0, sizeof(result));

	float statusBarHeight = 0.0f;
	if(config->toggles.showStatusBar) {
		statusBarHeight = fuiMinF(config->metrics.statusBarHeight, rect.h);
	}
	float frameHeight = fuiMaxF(rect.h - statusBarHeight, 0.0f);
	result.frameRect = fuiRectMake(rect.x, rect.y, rect.w, frameHeight);
	result.statusBarRect = fuiRectMake(rect.x, rect.y + frameHeight, rect.w, statusBarHeight);

	float innerX = result.frameRect.x + borderThickness;
	float innerY = result.frameRect.y + borderThickness;
	float innerWidth = fuiMaxF(result.frameRect.w - borderThickness * 2.0f, 0.0f);
	float innerHeight = fuiMaxF(result.frameRect.h - borderThickness * 2.0f, 0.0f);
	result.innerRect = fuiRectMake(innerX, innerY, innerWidth, innerHeight);

	float scrollbarThickness = fuiScrollGutterWidth();

	bool contentIsTallerThanTheBox = (contentHeight > innerHeight);
	result.hasVerticalBar = fuiEditor__ScrollbarIsThere(config->toggles.verticalScrollbar, contentIsTallerThanTheBox);
	float verticalBarWidth = result.hasVerticalBar ? scrollbarThickness : 0.0f;

	float widthForTheText = fuiMaxF(innerWidth - gutterWidth - verticalBarWidth, 0.0f);
	bool contentIsWiderThanTheBox = (contentWidth > widthForTheText);
	result.hasHorizontalBar = fuiEditor__ScrollbarIsThere(config->toggles.horizontalScrollbar, contentIsWiderThanTheBox);
	float horizontalBarHeight = result.hasHorizontalBar ? scrollbarThickness : 0.0f;

	float bodyHeight = fuiMaxF(innerHeight - horizontalBarHeight, 0.0f);
	float bodyWidth = fuiMaxF(innerWidth - verticalBarWidth, 0.0f);
	result.bodyRect = fuiRectMake(innerX, innerY, bodyWidth, bodyHeight);

	float cappedGutterWidth = fuiMinF(gutterWidth, bodyWidth);
	result.gutterRect = fuiRectMake(innerX, innerY, cappedGutterWidth, bodyHeight);
	float textLeft = innerX + cappedGutterWidth;
	float textWidth = fuiMaxF(bodyWidth - cappedGutterWidth, 0.0f);
	result.textRect = fuiRectMake(textLeft, innerY, textWidth, bodyHeight);

	result.verticalTrackRect = fuiRectMake(innerX + bodyWidth, innerY, verticalBarWidth, bodyHeight);
	result.horizontalTrackRect = fuiRectMake(innerX, innerY + bodyHeight, bodyWidth, horizontalBarHeight);
	return(result);
}

// ----------------------------------------------------------------------------
// > Input
// ----------------------------------------------------------------------------

//! Which document offset a point on screen lands on
static int32_t fuiEditor__OffsetAtPoint(fuiContext *context, const fuiEditor *editor, const fuiEditor__Render *render, const fuiEditor__Layout *layout, const float scrollX, const float scrollY, const fuiVec2 point) {
	int32_t screenLineCount = fuiEditor__GetScreenLineCount(editor);
	if(screenLineCount <= 0 || render->lineHeight <= 0.0f) {
		return(0);
	}

	float distanceDownTheContent = (point.y - layout->textRect.y) + scrollY;
	int32_t screenLine = (int32_t)(distanceDownTheContent / render->lineHeight);
	if(distanceDownTheContent < 0.0f) {
		screenLine = 0;
	}
	screenLine = fuiEditor__ClampI32(screenLine, 0, screenLineCount - 1);

	int32_t documentLine = fuiEditor__DocumentLineOfScreenLine(editor, screenLine);
	int32_t lineStart = fuiEditorGetLineStart(editor, documentLine);
	int32_t lineEnd = fuiEditorGetLineEnd(editor, documentLine);

	const fuiEditorConfig *config = &editor->resolvedConfig;
	float lineLeftX = layout->textRect.x + config->metrics.textPaddingX - scrollX;
	float distanceIntoTheLine = point.x - lineLeftX;
	return(fuiEditor__OffsetAtDistance(context, editor, render, lineStart, lineEnd, distanceIntoTheLine));
}

/*
	Moves the caret a number of lines, keeping the column it WANTS rather than the one it lands on.

	A caret walked down through a ragged block of code and back up has to come home to where it started.
	That only works if the sideways position is remembered from the last move that was really sideways -
	the short line in the middle would otherwise pull it left and keep it there.
*/
static void fuiEditor__MoveCaretByLines(fuiContext *context, fuiEditor *editor, const fuiEditor__Render *render, const int32_t lineDelta, const bool extendSelection) {
	int32_t caretLine = fuiEditorGetCaretLine(editor);
	if(!editor->hasDesiredDistance) {
		int32_t lineStart = fuiEditorGetLineStart(editor, caretLine);
		int32_t lineEnd = fuiEditorGetLineEnd(editor, caretLine);
		editor->desiredDistance = fuiEditor__DistanceOfOffset(context, editor, render, lineStart, lineEnd, editor->caretOffset);
		editor->hasDesiredDistance = true;
	}

	int32_t lineCount = fuiEditorGetLineCount(editor);
	int32_t wantedLine = fuiEditor__ClampI32(caretLine + lineDelta, 0, lineCount - 1);
	int32_t wantedLineStart = fuiEditorGetLineStart(editor, wantedLine);
	int32_t wantedLineEnd = fuiEditorGetLineEnd(editor, wantedLine);
	int32_t wantedOffset = fuiEditor__OffsetAtDistance(context, editor, render, wantedLineStart, wantedLineEnd, editor->desiredDistance);

	const bool keepTheDesiredDistance = true;
	fuiEditor__MoveCaretTo(editor, wantedOffset, extendSelection, keepTheDesiredDistance);
}

//! Puts the selection on the clipboard, in one allocation of exactly the size it needs
static bool fuiEditor__CopySelectionToClipboard(fuiContext *context, fuiEditor *editor) {
	char *noBufferYet = fui_null;
	const int32_t noCapacityYet = 0;
	int32_t neededLength = fuiEditorCopySelection(editor, noBufferYet, noCapacityYet);
	if(neededLength <= 0) {
		return(false);
	}

	// Sized to the selection rather than to a fixed buffer, so nothing is ever cut in half - least of all
	// in the middle of a codepoint.
	int32_t bufferLength = neededLength + 1;
	char *clipboardText = (char *)fuiEditor__Allocate(editor, bufferLength);
	if(clipboardText == fui_null) {
		return(false);
	}
	(void)fuiEditorCopySelection(editor, clipboardText, bufferLength);
	bool didSet = fuiSetClipboardText(context, clipboardText);
	fuiEditor__Release(editor, clipboardText);
	return(didSet);
}

//! Every key the editor answers to while it has the keyboard
static void fuiEditor__HandleKeyboard(fuiContext *context, fuiEditor *editor, const fuiEditor__Render *render, const int32_t linesPerPage, bool *outDidCopy) {
	bool wantsToExtend = fuiIsShiftDown(context);
	bool wantsToJumpByWord = fuiIsControlDown(context);
	int32_t textLength = fuiEditorGetTextLength(editor);
	int32_t lineCount = fuiEditorGetLineCount(editor);

	if(fuiKeyRepeat(context, fuiKey_Left)) {
		int32_t wantedOffset;
		bool collapsesTheSelection = !wantsToExtend && !wantsToJumpByWord && fuiEditorHasSelection(editor);
		if(wantsToJumpByWord) {
			wantedOffset = fuiEditor__PreviousWordOffset(editor, editor->caretOffset);
		} else if(collapsesTheSelection) {
			// A plain arrow against a selection puts the caret at that END of it rather than one character
			// further, which is what every editor does and what nobody notices until it is missing.
			wantedOffset = fuiEditorGetSelectionStart(editor);
		} else {
			wantedOffset = fuiEditorPreviousCodepointOffset(editor, editor->caretOffset);
		}
		fuiEditor__MoveCaretTo(editor, wantedOffset, wantsToExtend, false);
	}

	if(fuiKeyRepeat(context, fuiKey_Right)) {
		int32_t wantedOffset;
		bool collapsesTheSelection = !wantsToExtend && !wantsToJumpByWord && fuiEditorHasSelection(editor);
		if(wantsToJumpByWord) {
			wantedOffset = fuiEditor__NextWordOffset(editor, editor->caretOffset);
		} else if(collapsesTheSelection) {
			wantedOffset = fuiEditorGetSelectionEnd(editor);
		} else {
			wantedOffset = fuiEditorNextCodepointOffset(editor, editor->caretOffset);
		}
		fuiEditor__MoveCaretTo(editor, wantedOffset, wantsToExtend, false);
	}

	if(fuiKeyRepeat(context, fuiKey_Up)) {
		const int32_t oneLineUp = -1;
		fuiEditor__MoveCaretByLines(context, editor, render, oneLineUp, wantsToExtend);
	}
	if(fuiKeyRepeat(context, fuiKey_Down)) {
		const int32_t oneLineDown = 1;
		fuiEditor__MoveCaretByLines(context, editor, render, oneLineDown, wantsToExtend);
	}
	if(fuiKeyRepeat(context, fuiKey_PageUp)) {
		int32_t pageUp = -linesPerPage;
		fuiEditor__MoveCaretByLines(context, editor, render, pageUp, wantsToExtend);
	}
	if(fuiKeyRepeat(context, fuiKey_PageDown)) {
		fuiEditor__MoveCaretByLines(context, editor, render, linesPerPage, wantsToExtend);
	}

	if(fuiKeyRepeat(context, fuiKey_Home)) {
		int32_t wantedOffset = 0;
		if(!wantsToJumpByWord) {
			int32_t caretLine = fuiEditorGetCaretLine(editor);
			wantedOffset = fuiEditorGetLineStart(editor, caretLine);
		}
		fuiEditor__MoveCaretTo(editor, wantedOffset, wantsToExtend, false);
	}
	if(fuiKeyRepeat(context, fuiKey_End)) {
		int32_t wantedOffset = textLength;
		if(!wantsToJumpByWord) {
			int32_t caretLine = fuiEditorGetCaretLine(editor);
			wantedOffset = fuiEditorGetLineEnd(editor, caretLine);
		}
		fuiEditor__MoveCaretTo(editor, wantedOffset, wantsToExtend, false);
	}

	if(wantsToJumpByWord && fuiKeyWentDown(context, fuiKey_A)) {
		fuiEditorSelectAll(editor);
		editor->caretBlinkTime = 0.0f;
	}
	if(wantsToJumpByWord && fuiKeyWentDown(context, fuiKey_C)) {
		bool didCopy = fuiEditor__CopySelectionToClipboard(context, editor);
		if(didCopy && outDidCopy != fui_null) {
			*outDidCopy = true;
		}
	}

	(void)lineCount;
}

//! The click, the drag, and what a second and a third click in the same place mean
static void fuiEditor__HandleMouse(fuiContext *context, fuiEditor *editor, const fuiEditor__Render *render, const fuiEditor__Layout *layout, const fuiInteraction *interaction, const float frameTime, const float scrollX, float *inOutScrollY) {
	fuiVec2 mousePosition = fuiGetMousePosition(context);

	if(interaction->wasPressed) {
		float movedX = mousePosition.x - editor->lastPressPosition.x;
		float movedY = mousePosition.y - editor->lastPressPosition.y;
		float movedDistanceSquared = movedX * movedX + movedY * movedY;
		bool isNearTheLastPress = (movedDistanceSquared <= (FUI_TEXTEDITOR__MULTI_CLICK_SLOP * FUI_TEXTEDITOR__MULTI_CLICK_SLOP));
		bool isSoonAfterTheLastPress = (editor->timeSinceLastPress <= FUI_TEXTEDITOR__MULTI_CLICK_SECONDS);
		if(isNearTheLastPress && isSoonAfterTheLastPress) {
			editor->pressCount += 1;
		} else {
			editor->pressCount = 1;
		}
		// Round back to a single click after the third, so a fourth one starts over rather than staying
		// on whole lines for as long as somebody keeps clicking.
		if(editor->pressCount > 3) {
			editor->pressCount = 1;
		}
		editor->timeSinceLastPress = 0.0f;
		editor->lastPressPosition = mousePosition;

		int32_t pressedOffset = fuiEditor__OffsetAtPoint(context, editor, render, layout, scrollX, *inOutScrollY, mousePosition);
		bool wantsToExtend = fuiIsShiftDown(context);

		if(editor->pressCount == 2) {
			fuiEditor__WordRangeAt(editor, pressedOffset, &editor->dragAnchorStart, &editor->dragAnchorEnd);
			fuiEditorSetSelection(editor, editor->dragAnchorStart, editor->dragAnchorEnd);
		} else if(editor->pressCount == 3) {
			fuiEditor__LineRangeAt(editor, pressedOffset, &editor->dragAnchorStart, &editor->dragAnchorEnd);
			fuiEditorSetSelection(editor, editor->dragAnchorStart, editor->dragAnchorEnd);
		} else if(wantsToExtend) {
			// Shift and a click reaches out from wherever the selection was already held down.
			editor->dragAnchorStart = editor->selectionAnchor;
			editor->dragAnchorEnd = editor->selectionAnchor;
			fuiEditor__MoveCaretTo(editor, pressedOffset, true, false);
		} else {
			fuiEditor__MoveCaretTo(editor, pressedOffset, false, false);
			editor->dragAnchorStart = editor->caretOffset;
			editor->dragAnchorEnd = editor->caretOffset;
		}
		editor->isDraggingSelection = true;
	}

	if(!interaction->isHeld) {
		editor->isDraggingSelection = false;
		editor->timeSinceLastPress += frameTime;
		return;
	}

	// A drag that has run off the top or the bottom keeps going by itself, faster the further out it is,
	// which is what makes it possible to select more than fits on the screen.
	float topEdge = layout->textRect.y;
	float bottomEdge = layout->textRect.y + layout->textRect.h;
	float overshoot = 0.0f;
	if(mousePosition.y < topEdge) {
		overshoot = mousePosition.y - topEdge;
	} else if(mousePosition.y > bottomEdge) {
		overshoot = mousePosition.y - bottomEdge;
	}
	if(overshoot != 0.0f && render->lineHeight > 0.0f) {
		float overshootInLines = overshoot / render->lineHeight;
		*inOutScrollY += overshootInLines * FUI_TEXTEDITOR__AUTOSCROLL_LINES_PER_SECOND * render->lineHeight * frameTime;
	}

	int32_t draggedOffset = fuiEditor__OffsetAtPoint(context, editor, render, layout, scrollX, *inOutScrollY, mousePosition);

	// A drag that began on a word or on a line stays on whole words or whole lines, and never shrinks
	// past the one it began on.
	if(editor->pressCount == 2 || editor->pressCount == 3) {
		int32_t draggedRangeStart = draggedOffset;
		int32_t draggedRangeEnd = draggedOffset;
		if(editor->pressCount == 2) {
			fuiEditor__WordRangeAt(editor, draggedOffset, &draggedRangeStart, &draggedRangeEnd);
		} else {
			fuiEditor__LineRangeAt(editor, draggedOffset, &draggedRangeStart, &draggedRangeEnd);
		}
		bool isDraggingForwards = (draggedRangeEnd > editor->dragAnchorEnd);
		if(isDraggingForwards) {
			fuiEditorSetSelection(editor, editor->dragAnchorStart, draggedRangeEnd);
		} else {
			fuiEditorSetSelection(editor, editor->dragAnchorEnd, draggedRangeStart);
		}
		return;
	}

	fuiEditor__MoveCaretTo(editor, draggedOffset, true, false);
}

/*
	Brings the caret back into view - but ONLY when it has really moved.

	Doing it unconditionally is the classic way to break the wheel: the view is scrolled away from the
	caret, the next frame drags it straight back, and the document appears to be nailed down. So this runs
	off a comparison against where the caret stood at the top of the build, and the wheel and the
	scrollbars, which move the view without moving the caret, are left alone.
*/
static void fuiEditor__EnsureCaretVisible(fuiContext *context, fuiEditor *editor, const fuiEditor__Render *render, const fuiEditor__Layout *layout, float *inOutScrollX, float *inOutScrollY) {
	const fuiEditorConfig *config = &editor->resolvedConfig;
	int32_t caretLine = fuiEditorGetCaretLine(editor);
	int32_t caretScreenLine = fuiEditor__ScreenLineOfDocumentLine(editor, caretLine);

	float caretTop = (float)caretScreenLine * render->lineHeight;
	float caretBottom = caretTop + render->lineHeight;
	if(caretTop < *inOutScrollY) {
		*inOutScrollY = caretTop;
	} else if(caretBottom > (*inOutScrollY + layout->bodyRect.h)) {
		*inOutScrollY = caretBottom - layout->bodyRect.h;
	}

	int32_t lineStart = fuiEditorGetLineStart(editor, caretLine);
	int32_t lineEnd = fuiEditorGetLineEnd(editor, caretLine);
	float caretDistance = fuiEditor__DistanceOfOffset(context, editor, render, lineStart, lineEnd, editor->caretOffset);
	float caretContentX = config->metrics.textPaddingX + caretDistance;

	// A margin of one character, so the caret is never flush against an edge it is about to cross.
	float margin = render->characterWidth;
	float leftEdge = *inOutScrollX;
	float rightEdge = *inOutScrollX + layout->textRect.w;
	if(caretContentX < (leftEdge + margin)) {
		*inOutScrollX = fuiMaxF(caretContentX - margin, 0.0f);
	} else if(caretContentX > (rightEdge - margin)) {
		*inOutScrollX = caretContentX - layout->textRect.w + margin;
	}
}

// ----------------------------------------------------------------------------
// > The widget
// ----------------------------------------------------------------------------

//! Fills in the editor's own status line - where the caret is, how big the document is, and how it is written
static void fuiEditor__BuildStatusText(const fuiEditor *editor, char *destination, const int32_t destinationCapacity) {
	const char *fieldSeparator = "   |   ";
	int32_t documentLineCount = fuiEditorGetLineCount(editor);
	int32_t caretLine = fuiEditorGetCaretLine(editor);
	int32_t caretColumn = fuiEditorGetCaretColumn(editor);
	int32_t textLength = fuiEditorGetTextLength(editor);
	int32_t selectionStart = fuiEditorGetSelectionStart(editor);
	int32_t selectionEnd = fuiEditorGetSelectionEnd(editor);
	int32_t selectionLength = selectionEnd - selectionStart;
	fuiEditorEol documentEol = fuiEditorGetEol(editor);
	const char *eolName = fuiEditorEolGetName(documentEol);
	const char *encodingName = (editor->encoding.name != fui_null) ? editor->encoding.name : "?";

	// Counted from one, because that is what every other tool that names a line counts from, and a status
	// line is read beside those tools rather than beside this file.
	int32_t writeOffset = 0;
	writeOffset = fuiEditor__AppendText(destination, destinationCapacity, writeOffset, "Ln ");
	writeOffset = fuiEditor__AppendInt(destination, destinationCapacity, writeOffset, caretLine + 1);
	writeOffset = fuiEditor__AppendText(destination, destinationCapacity, writeOffset, ", Col ");
	writeOffset = fuiEditor__AppendInt(destination, destinationCapacity, writeOffset, caretColumn + 1);
	writeOffset = fuiEditor__AppendText(destination, destinationCapacity, writeOffset, fieldSeparator);
	writeOffset = fuiEditor__AppendInt(destination, destinationCapacity, writeOffset, documentLineCount);
	writeOffset = fuiEditor__AppendText(destination, destinationCapacity, writeOffset, " lines");
	if(selectionLength > 0) {
		writeOffset = fuiEditor__AppendText(destination, destinationCapacity, writeOffset, fieldSeparator);
		writeOffset = fuiEditor__AppendInt(destination, destinationCapacity, writeOffset, selectionLength);
		writeOffset = fuiEditor__AppendText(destination, destinationCapacity, writeOffset, " selected");
	}
	writeOffset = fuiEditor__AppendText(destination, destinationCapacity, writeOffset, fieldSeparator);
	writeOffset = fuiEditor__AppendInt(destination, destinationCapacity, writeOffset, textLength);
	writeOffset = fuiEditor__AppendText(destination, destinationCapacity, writeOffset, " bytes");
	writeOffset = fuiEditor__AppendText(destination, destinationCapacity, writeOffset, fieldSeparator);
	writeOffset = fuiEditor__AppendText(destination, destinationCapacity, writeOffset, encodingName);
	writeOffset = fuiEditor__AppendText(destination, destinationCapacity, writeOffset, fieldSeparator);
	writeOffset = fuiEditor__AppendText(destination, destinationCapacity, writeOffset, eolName);
	writeOffset = fuiEditor__AppendText(destination, destinationCapacity, writeOffset, fieldSeparator);
	writeOffset = fuiEditor__AppendText(destination, destinationCapacity, writeOffset, "Tab ");
	writeOffset = fuiEditor__AppendInt(destination, destinationCapacity, writeOffset, editor->resolvedConfig.metrics.tabSize);
	(void)writeOffset;
}

//! Advances the blink and answers whether the caret is in its lit half right now
static bool fuiEditor__AdvanceCaretBlink(fuiEditor *editor, const fuiTheme *theme, const float frameTime) {
	float blinkPeriod = 0.5f;
	if(theme->caretBlinkHz > 0.0f) {
		blinkPeriod = 1.0f / theme->caretBlinkHz;
	}
	float fullCycle = blinkPeriod * 2.0f;

	editor->caretBlinkTime += frameTime;
	if(editor->caretBlinkTime >= fullCycle) {
		// One subtraction is enough for any sane frame. A stall that skipped whole cycles only has to come
		// back ON PHASE, not to remember how many of them it missed.
		editor->caretBlinkTime -= fullCycle;
		if(editor->caretBlinkTime >= fullCycle) {
			editor->caretBlinkTime = 0.0f;
		}
	}
	return(editor->caretBlinkTime < blinkPeriod);
}

fui_api fuiEditorAction fuiTextEditor(fuiContext *context, const fuiRect rect, const char *id, fuiEditor *editor) {
	fuiEditorAction result;
	FUI_TEXTEDITOR_MEMSET(&result, 0, sizeof(result));

	FUI_TEXTEDITOR_ASSERT(context != fui_null && id != fui_null && editor != fui_null);
	if(context == fui_null || id == fui_null || editor == fui_null || !editor->isInitialized) {
		return(result);
	}

	fuiTheme *theme = fuiGetTheme(context);
	if(theme == fui_null) {
		return(result);
	}
	fuiEditor__ResolveConfig(editor, theme);

	const fuiEditorConfig *config = &editor->resolvedConfig;
	fuiEditor__Render render = fuiEditor__MakeRender(context, config);
	if(render.lineHeight <= 0.0f) {
		return(result);
	}

	int32_t caretBeforeThisBuild = editor->caretOffset;
	int32_t anchorBeforeThisBuild = editor->selectionAnchor;
	float frameTime = fuiGetFrameTime(context);

	// An edit throws the widest line away rather than keeping a width that belonged to text which is gone.
	if(editor->widestMeasuredVersion != editor->version) {
		editor->widestMeasuredLineWidth = 0.0f;
		editor->widestMeasuredVersion = editor->version;
	}

	/*
		The caret's own line is measured before anything is laid out.

		It is the one line that has to be reachable sideways whether it has ever been on screen or not: an
		arrow key walking down into a long line has to be able to scroll to the caret, and it can only do
		that if the horizontal range already knows the line is that wide.
	*/
	int32_t caretLineBeforeLayout = fuiEditorGetCaretLine(editor);
	int32_t caretLineStartBeforeLayout = fuiEditorGetLineStart(editor, caretLineBeforeLayout);
	int32_t caretLineEndBeforeLayout = fuiEditorGetLineEnd(editor, caretLineBeforeLayout);
	float caretLineWidth = fuiEditor__LineWidth(context, editor, &render, caretLineStartBeforeLayout, caretLineEndBeforeLayout);
	if(caretLineWidth > editor->widestMeasuredLineWidth) {
		editor->widestMeasuredLineWidth = caretLineWidth;
	}

	int32_t screenLineCount = fuiEditor__GetScreenLineCount(editor);
	float contentHeight = (float)screenLineCount * render.lineHeight;
	float contentWidth = editor->widestMeasuredLineWidth + config->metrics.textPaddingX * 2.0f;

	float gutterWidth = 0.0f;
	if(config->toggles.showLineNumbers) {
		gutterWidth = fuiEditor__GutterWidthFor(config, screenLineCount, render.digitWidth);
	}
	fuiEditor__Layout layout = fuiEditor__MakeLayout(rect, config, gutterWidth, contentHeight, contentWidth, theme->widgetBorderThickness);

	fuiId editorId = fuiGetId(context, id);
	fuiInteraction bodyInteraction = fuiInteract(context, editorId, layout.bodyRect);
	if(config->toggles.isInteractive) {
		fuiRegisterFocusable(context, editorId);
		if(bodyInteraction.wasPressed) {
			fuiSetFocusedId(context, editorId);
		}
	}
	fuiId focusedId = fuiGetFocusedId(context);
	result.isFocused = config->toggles.isInteractive && (focusedId == editorId);

	// A fuiEditorScrollToLine that has been waiting for a line height is answered here, where there is one.
	if(editor->hasPendingScroll) {
		int32_t lastScreenLine = fuiEditor__MaxI32(screenLineCount - 1, 0);
		int32_t pendingScreenLine = fuiEditor__ClampI32(editor->pendingScrollScreenLine, 0, lastScreenLine);
		editor->scrollY = (float)pendingScreenLine * render.lineHeight;
		editor->hasPendingScroll = false;
	}

	// The wheel over the body moves it down, and sideways while shift is held - the same gesture the list
	// view uses, so that scrolling one thing in this library feels like scrolling any other.
	float scrollX = editor->scrollX;
	float scrollY = editor->scrollY;
	float wheelDelta = fuiGetMouseWheelDelta(context);
	if(bodyInteraction.isHovered && wheelDelta != 0.0f) {
		float wheelDistance = wheelDelta * render.lineHeight * FUI_TEXTEDITOR__WHEEL_LINES;
		bool wantsSideways = fuiIsShiftDown(context);
		if(wantsSideways) {
			scrollX -= wheelDistance;
		} else {
			scrollY -= wheelDistance;
		}
	}

	if(config->toggles.isInteractive) {
		fuiEditor__HandleMouse(context, editor, &render, &layout, &bodyInteraction, frameTime, scrollX, &scrollY);
		if(result.isFocused) {
			int32_t linesPerPage = (int32_t)(layout.bodyRect.h / render.lineHeight);
			linesPerPage = fuiEditor__MaxI32(linesPerPage, 1);
			fuiEditor__HandleKeyboard(context, editor, &render, linesPerPage, &result.didCopy);
		}
	}

	bool caretMoved = (editor->caretOffset != caretBeforeThisBuild) || (editor->selectionAnchor != anchorBeforeThisBuild);
	result.didMoveCaret = caretMoved;
	if(caretMoved) {
		fuiEditor__EnsureCaretVisible(context, editor, &render, &layout, &scrollX, &scrollY);
	}

	// Both bars are resolved BEFORE a line is laid out, so the lines are placed from the offsets the frame
	// ends on rather than from ones the wheel is about to change.
	float maxScrollY = fuiMaxF(contentHeight - layout.bodyRect.h, 0.0f);
	float maxScrollX = fuiMaxF(contentWidth - layout.textRect.w, 0.0f);
	fuiPushId(context, id);
	if(layout.hasVerticalBar) {
		scrollY = fuiScrollbarVertical(context, layout.verticalTrackRect, "__editorScrollbarY", scrollY, layout.bodyRect.h, contentHeight);
	} else {
		scrollY = fuiClampF(scrollY, 0.0f, maxScrollY);
	}
	if(layout.hasHorizontalBar) {
		scrollX = fuiScrollbarHorizontal(context, layout.horizontalTrackRect, "__editorScrollbarX", scrollX, layout.textRect.w, contentWidth);
	} else {
		scrollX = fuiClampF(scrollX, 0.0f, maxScrollX);
	}
	fuiPopId(context);
	editor->scrollX = scrollX;
	editor->scrollY = scrollY;

	fuiDrawRect(context, layout.frameRect, config->colors.background);

	int32_t firstScreenLine = 0;
	int32_t endScreenLine = 0;
	fuiEditor__VisibleScreenLines(scrollY, layout.bodyRect.h, render.lineHeight, screenLineCount, &firstScreenLine, &endScreenLine);
	editor->firstVisibleScreenLine = firstScreenLine;
	editor->visibleScreenLineCount = endScreenLine - firstScreenLine;

	// Coloured up to the last line that can be seen, and no further - a lexer is only ever asked for what
	// is about to be drawn. A build that runs out of budget leaves the rest to the next one and draws
	// plain text until it arrives, which is a few frames of plain text rather than one long stall.
	int32_t lastVisibleLine = fuiEditor__DocumentLineOfScreenLine(editor, endScreenLine - 1);
	fuiEditor__LexUpToLine(editor, lastVisibleLine);

	int32_t firstVisibleDocumentLine = fuiEditor__DocumentLineOfScreenLine(editor, firstScreenLine);
	int32_t caretDocumentLine = fuiEditorGetCaretLine(editor);
	int32_t caretScreenLine = fuiEditor__ScreenLineOfDocumentLine(editor, caretDocumentLine);
	int32_t selectionStart = fuiEditorGetSelectionStart(editor);
	int32_t selectionEnd = fuiEditorGetSelectionEnd(editor);
	bool hasSelection = (selectionEnd > selectionStart);
	int32_t documentLineCount = fuiEditorGetLineCount(editor);
	bool hasLineNumbers = config->toggles.showLineNumbers && (layout.gutterRect.w > 0.0f);

	if(hasLineNumbers) {
		fuiDrawRect(context, layout.gutterRect, config->colors.gutterBackground);
	}

	// The wash goes over the gutter as well as the text: a current line that stops at the line number reads
	// as two things beside each other rather than as one line.
	bool caretLineIsVisible = (caretScreenLine >= firstScreenLine) && (caretScreenLine < endScreenLine);
	if(config->toggles.highlightCurrentLine && caretLineIsVisible && !hasSelection) {
		float washTop = layout.bodyRect.y + (float)caretScreenLine * render.lineHeight - scrollY;
		fuiRect washRect = fuiRectMake(layout.bodyRect.x, washTop, layout.bodyRect.w, render.lineHeight);
		fuiPushClip(context, layout.bodyRect);
		fuiDrawRect(context, washRect, config->colors.currentLineBackground);
		fuiPopClip(context);
	}

	if(hasLineNumbers) {
		fuiPushClip(context, layout.gutterRect);
		float numberRightEdge = layout.gutterRect.x + layout.gutterRect.w - config->metrics.gutterPaddingX - FUI_TEXTEDITOR__GUTTER_SEPARATOR_THICKNESS;
		int32_t gutterDecorationCursor = fuiEditor__FirstLineDecorationFrom(&editor->decorations, firstVisibleDocumentLine);
		for(int32_t screenLine = firstScreenLine; screenLine < endScreenLine; ++screenLine) {
			bool carriesItsNumber = fuiEditor__ScreenLineCarriesItsNumber(editor, screenLine);
			if(!carriesItsNumber) {
				continue;
			}

			int32_t documentLine = fuiEditor__DocumentLineOfScreenLine(editor, screenLine);

			// The marker sits at the LEFT edge of the gutter, where a breakpoint or a diff bar belongs -
			// out of the way of the number, which is read.
			const fuiEditorLineDecoration *lineDecoration = fuiEditor__LineDecorationAt(&editor->decorations, &gutterDecorationCursor, documentLine);
			if(lineDecoration != fui_null && lineDecoration->gutterMarker.a > 0.0f) {
				float markerTop = layout.gutterRect.y + (float)screenLine * render.lineHeight - scrollY;
				fuiRect markerRect = fuiRectMake(layout.gutterRect.x, markerTop, FUI_TEXTEDITOR__GUTTER_MARKER_WIDTH, render.lineHeight);
				fuiDrawRect(context, markerRect, lineDecoration->gutterMarker);
			}
			char numberText[FUI_TEXTEDITOR__MAX_NUMBER_TEXT];
			const int32_t numberCapacity = (int32_t)sizeof(numberText);
			int32_t numberLength = fuiEditor__FormatInt(numberText, numberCapacity, documentLine + 1);

			// Right aligned and NOT padded out with blanks or zeroes, so that a jump in the numbers - which
			// is what a folded range or a diff makes - reads as a jump rather than as a change of width.
			fuiVec2 numberSize = fuiMeasureText(context, numberText, (size_t)numberLength, render.fontHeight);
			float numberLeft = numberRightEdge - numberSize.x;
			float numberTop = layout.gutterRect.y + (float)screenLine * render.lineHeight - scrollY;
			bool isTheCaretLine = (screenLine == caretScreenLine);
			fuiColor numberColor = isTheCaretLine ? config->colors.gutterCurrentLineText : config->colors.gutterText;
			fuiVec2 numberPosition = fuiV2(numberLeft, numberTop);
			fuiDrawText(context, numberText, (size_t)numberLength, numberPosition, render.fontHeight, numberColor);
		}
		fuiPopClip(context);

		float separatorX = layout.gutterRect.x + layout.gutterRect.w - FUI_TEXTEDITOR__GUTTER_SEPARATOR_THICKNESS * 0.5f;
		fuiVec2 separatorTop = fuiV2(separatorX, layout.gutterRect.y);
		fuiVec2 separatorBottom = fuiV2(separatorX, layout.gutterRect.y + layout.gutterRect.h);
		fuiDrawLine(context, separatorTop, separatorBottom, config->colors.gutterSeparator, FUI_TEXTEDITOR__GUTTER_SEPARATOR_THICKNESS);
	}

	// Only the lines that can be seen are touched at all. That is the whole reason a document of a million
	// lines costs what one of twenty costs: nothing here is per DOCUMENT line, everything is per VISIBLE one.
	fuiPushClip(context, layout.textRect);
	float lineLeftX = layout.textRect.x + config->metrics.textPaddingX - scrollX;
	float widestLineSoFar = editor->widestMeasuredLineWidth;
	int32_t textDecorationCursor = fuiEditor__FirstLineDecorationFrom(&editor->decorations, firstVisibleDocumentLine);
	for(int32_t screenLine = firstScreenLine; screenLine < endScreenLine; ++screenLine) {
		int32_t documentLine = fuiEditor__DocumentLineOfScreenLine(editor, screenLine);
		int32_t lineStart = fuiEditorGetLineStart(editor, documentLine);
		int32_t lineEnd = fuiEditorGetLineEnd(editor, documentLine);
		float lineTopY = layout.textRect.y + (float)screenLine * render.lineHeight - scrollY;

		// A decoration's wash goes UNDER everything else on the line: it says what the line IS - added,
		// removed, in error - and the caret and the selection are things that happen on top of that.
		const fuiEditorLineDecoration *lineDecoration = fuiEditor__LineDecorationAt(&editor->decorations, &textDecorationCursor, documentLine);
		if(lineDecoration != fui_null && lineDecoration->background.a > 0.0f) {
			fuiRect decorationRect = fuiRectMake(layout.textRect.x, lineTopY, layout.textRect.w, render.lineHeight);
			fuiDrawRect(context, decorationRect, lineDecoration->background);
		}

		if(hasSelection) {
			int32_t washStart = fuiEditor__ClampI32(selectionStart, lineStart, lineEnd);
			int32_t washEnd = fuiEditor__ClampI32(selectionEnd, lineStart, lineEnd);

			// A line whose ENDING is inside the selection gets a blank's worth of wash past its last
			// character, which is how a selected line break is shown at all - it has no glyph of its own.
			bool isTheLastLine = (documentLine >= (documentLineCount - 1));
			bool coversTheLineBreak = !isTheLastLine && (selectionEnd > lineEnd) && (selectionStart <= lineEnd);
			bool thereIsAnythingToWash = (washEnd > washStart) || coversTheLineBreak;
			if(thereIsAnythingToWash) {
				float washStartDistance = fuiEditor__DistanceOfOffset(context, editor, &render, lineStart, lineEnd, washStart);
				float washEndDistance = fuiEditor__DistanceOfOffset(context, editor, &render, lineStart, lineEnd, washEnd);
				float washWidth = washEndDistance - washStartDistance;
				if(coversTheLineBreak) {
					washWidth += render.spaceWidth;
				}
				fuiRect washRect = fuiRectMake(lineLeftX + washStartDistance, lineTopY, washWidth, render.lineHeight);
				fuiDrawRect(context, washRect, config->colors.selectionBackground);
			}
		}

		// The ranges are the decoration that is NOT a whole line - a search hit, a squiggle under one
		// identifier. Sorted, so only the handful that reach into this line are ever looked at.
		if(editor->decorations.ranges != fui_null) {
			int32_t rangeIndex = fuiEditor__FirstRangeDecorationFrom(&editor->decorations, lineStart);
			while(rangeIndex < editor->decorations.rangeCount) {
				const fuiEditorRangeDecoration *range = &editor->decorations.ranges[rangeIndex];
				if(range->startOffset >= lineEnd) {
					break;
				}
				bool reachesThisLine = (range->endOffset > lineStart) && (range->background.a > 0.0f);
				if(reachesThisLine) {
					int32_t washStart = fuiEditor__ClampI32(range->startOffset, lineStart, lineEnd);
					int32_t washEnd = fuiEditor__ClampI32(range->endOffset, lineStart, lineEnd);
					float washStartDistance = fuiEditor__DistanceOfOffset(context, editor, &render, lineStart, lineEnd, washStart);
					float washEndDistance = fuiEditor__DistanceOfOffset(context, editor, &render, lineStart, lineEnd, washEnd);
					fuiRect washRect = fuiRectMake(lineLeftX + washStartDistance, lineTopY, washEndDistance - washStartDistance, render.lineHeight);
					fuiDrawRect(context, washRect, range->background);
				}
				rangeIndex += 1;
			}
		}

		// A line is only coloured when its start state is known. One that the colouring has not reached
		// yet is drawn plain rather than being coloured from a state that was guessed.
		fuiEditor__LinePaint paint;
		FUI_TEXTEDITOR_MEMSET(&paint, 0, sizeof(paint));
		paint.defaultColor = config->colors.text;
		paint.whitespaceColor = config->colors.whitespace;
		paint.showWhitespace = config->toggles.showWhitespace;
		bool thisLineIsColoured = (editor->lexer.lexLine != fui_null) && (documentLine < editor->styledUpToLine);
		if(thisLineIsColoured) {
			int32_t startState = fuiEditor__LineIndexGetLexerState(&editor->document.lines, documentLine);
			int32_t lexedLength = 0;
			(void)fuiEditor__LexOneLine(editor, documentLine, startState, &lexedLength);
			paint.styles = editor->styleScratch;
			paint.styleLength = lexedLength;
		}

		float lineWidth = fuiEditor__DrawLine(context, editor, &render, lineStart, lineEnd, lineLeftX, lineTopY, &paint);
		if(lineWidth > widestLineSoFar) {
			widestLineSoFar = lineWidth;
		}

		// The ending is written AFTER the line, a blank clear of its last character, because that is where
		// it actually is - and it is what makes a file with mixed endings show itself.
		bool hasAnEndingToShow = config->toggles.showLineEndings && (documentLine < (documentLineCount - 1));
		if(hasAnEndingToShow) {
			fuiEditorEol lineEnding = fuiEditor__LineEndingOf(editor, documentLine);
			const char *endingName = fuiEditorEolGetName(lineEnding);
			size_t endingLength = FUI_TEXTEDITOR_STRLEN(endingName);
			fuiVec2 endingPosition = fuiV2(lineLeftX + lineWidth + render.spaceWidth, lineTopY);
			fuiDrawText(context, endingName, endingLength, endingPosition, render.fontHeight, config->colors.whitespace);
		}
	}

	// Drawn last, so it stands on top of the glyph it sits beside rather than under it.
	bool caretIsLitRightNow = fuiEditor__AdvanceCaretBlink(editor, theme, frameTime);
	if(result.isFocused && caretIsLitRightNow && caretLineIsVisible) {
		int32_t caretLineStart = fuiEditorGetLineStart(editor, caretDocumentLine);
		int32_t caretLineEnd = fuiEditorGetLineEnd(editor, caretDocumentLine);
		float caretDistance = fuiEditor__DistanceOfOffset(context, editor, &render, caretLineStart, caretLineEnd, editor->caretOffset);
		float caretTop = layout.textRect.y + (float)caretScreenLine * render.lineHeight - scrollY;
		fuiRect caretRect = fuiRectMake(lineLeftX + caretDistance, caretTop, config->metrics.caretWidth, render.lineHeight);
		fuiDrawRect(context, caretRect, config->colors.caret);
	}
	fuiPopClip(context);

	/*
		The horizontal bar goes by the widest line SEEN so far rather than by the widest line there is.

		Measuring every line of the document would be a walk over the whole of it every frame, and on a
		proportional face that walk is a glyph lookup per character - which is exactly the cost this widget
		exists not to pay. So the range grows as the document is scrolled through, and an edit resets it.
		That is what scintilla does, and for the same reason.
	*/
	editor->widestMeasuredLineWidth = widestLineSoFar;

	if(bodyInteraction.isHovered && config->toggles.isInteractive) {
		fuiSetCursor(context, fuiCursor_Text);
	}

	fuiDrawRectOutline(context, layout.frameRect, config->colors.border, theme->widgetBorderThickness);

	// The square where the two bars meet. Left unpainted it shows whatever is behind the editor through a
	// corner that belongs to neither bar, which reads as a hole in the frame.
	if(layout.hasVerticalBar && layout.hasHorizontalBar) {
		float cornerX = layout.verticalTrackRect.x;
		float cornerY = layout.horizontalTrackRect.y;
		fuiRect cornerRect = fuiRectMake(cornerX, cornerY, layout.verticalTrackRect.w, layout.horizontalTrackRect.h);
		fuiDrawRect(context, cornerRect, theme->widgetTrackColor);
	}

	// Its OWN status line, rather than fuiBeginStatusBar - that one docks against the bottom of the window,
	// and an editor is rarely the whole window.
	if(config->toggles.showStatusBar && layout.statusBarRect.h > 0.0f) {
		char statusText[FUI_TEXTEDITOR__MAX_STATUS_TEXT];
		const int32_t statusCapacity = (int32_t)sizeof(statusText);
		fuiEditor__BuildStatusText(editor, statusText, statusCapacity);
		size_t statusLength = FUI_TEXTEDITOR_STRLEN(statusText);

		fuiDrawRect(context, layout.statusBarRect, config->colors.statusBarBackground);
		fuiDrawRectOutline(context, layout.statusBarRect, config->colors.border, theme->widgetBorderThickness);

		float statusLineHeight = fuiGetLineHeight(context, render.fontHeight);
		float statusTextLeft = layout.statusBarRect.x + config->metrics.textPaddingX;
		float statusTextTop = layout.statusBarRect.y + (layout.statusBarRect.h - statusLineHeight) * 0.5f;
		fuiVec2 statusTextPosition = fuiV2(statusTextLeft, statusTextTop);
		fuiPushClip(context, layout.statusBarRect);
		fuiDrawText(context, statusText, statusLength, statusTextPosition, render.fontHeight, config->colors.statusBarText);
		fuiPopClip(context);
	}

	return(result);
}

#endif // FUI_TEXTEDITOR_IMPLEMENTATION
