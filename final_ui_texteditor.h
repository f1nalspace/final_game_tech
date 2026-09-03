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

Status: under construction. See the changelog for what is in.

-------------------------------------------------------------------------------
	Getting started
-------------------------------------------------------------------------------

- Include final_ui.h BEFORE this file. It is a hard prerequisite.
- Define FUI_TEXTEDITOR_IMPLEMENTATION in exactly ONE translation unit before including this file.
- Create one fuiEditor per document with fuiEditorInit(), pass fui_null as allocator to use malloc/free.
- Fill it with fuiEditorSetText() or fuiEditorLoadFromMemory().
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

fuiEditor editor;
fuiEditorInit(&editor, fui_null);            // fui_null = default malloc/free
fuiEditorSetText(&editor, sourceCode, 0);    // 0 = measure up to the terminating zero

// ... once the widget lands in a later iteration, per frame:
// fuiEditorAction action = fuiTextEditor(&ui, rect, "source", &editor);

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
FUI_TEXTEDITOR_STRLEN(s)        Override string length (defaults to strlen).
FUI_TEXTEDITOR_MIN_TEXT_BYTES   Smallest byte capacity a document is ever allocated at (default 4096).
FUI_TEXTEDITOR_MIN_LINE_SLOTS   Smallest number of line slots the line index is ever allocated at (default 256).
FUI_TEXTEDITOR_MIN_GAP_BYTES    How much room an insert leaves behind for the next one (default 1024).
FUI_TEXTEDITOR_MIN_GAP_SLOTS    How many line slots an insert leaves behind for the next one (default 64).

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
	@version v0.1.0
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
#define FUI_TEXTEDITOR_VERSION_MINOR 1
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

#if !defined(FUI_TEXTEDITOR_MEMSET) || !defined(FUI_TEXTEDITOR_MEMCPY) || !defined(FUI_TEXTEDITOR_MEMMOVE) || !defined(FUI_TEXTEDITOR_MEMCHR) || !defined(FUI_TEXTEDITOR_STRLEN)
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

	// The entries in front of the hole keep their slots; the ones behind it move to the end of the bigger
	// array, so the hole simply becomes the larger space between them.
	int32_t tailCount = index->capacity - index->gapEnd;
	if(index->gapStart > 0) {
		FUI_TEXTEDITOR_MEMCPY(newStarts, index->starts, (size_t)index->gapStart * sizeof(int32_t));
	}
	if(tailCount > 0) {
		int32_t newTailSlot = newCapacity - tailCount;
		FUI_TEXTEDITOR_MEMCPY(&newStarts[newTailSlot], &index->starts[index->gapEnd], (size_t)tailCount * sizeof(int32_t));
	}

	fuiEditor__Release(editor, index->starts);
	index->starts = newStarts;
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

	while(index->gapStart < targetGapStart) {
		index->starts[index->gapStart] = index->starts[index->gapEnd] + index->tailDelta;
		index->gapStart += 1;
		index->gapEnd += 1;
	}
	while(index->gapStart > targetGapStart) {
		index->gapStart -= 1;
		index->gapEnd -= 1;
		index->starts[index->gapEnd] = index->starts[index->gapStart] - index->tailDelta;
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

	editor->document.bytes = initialBytes;
	editor->document.capacity = FUI_TEXTEDITOR_MIN_TEXT_BYTES;
	editor->document.lines.starts = initialStarts;
	editor->document.lines.capacity = FUI_TEXTEDITOR_MIN_LINE_SLOTS;
	fuiEditor__DocumentClear(&editor->document);

	editor->isInitialized = true;
	return(true);
}

fui_api void fuiEditorRelease(fuiEditor *editor) {
	if(editor == fui_null || !editor->isInitialized) {
		return;
	}
	fuiEditor__Release(editor, editor->document.bytes);
	fuiEditor__Release(editor, editor->document.lines.starts);
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

#endif // FUI_TEXTEDITOR_IMPLEMENTATION
