/*
Name:
	FUI_Editor

Description:
	An interactive demo for final_ui_texteditor.h, on FPL and legacy OpenGL.

	The editor add-on is built over several iterations, and this demo grows with it. What is in right now
	is the document - a gap buffer, a line index that is a split array of its own, and the encoding seam -
	and a widget over it that can be READ: a gutter with line numbers, tab stops, two scrollbars, a status
	line, and a caret that can be moved by the keyboard and the mouse and selected from. Nothing types
	into it yet.

	It fills itself from final_ui.h, because at over fourteen thousand lines that is the largest file to
	hand and the one the add-on has to hold up against.

	Run it with --selftest to get the headless one: no window, no OpenGL, one exit code. That is the mode
	all of this is developed against - a gap buffer is exactly the kind of thing that looks right on screen
	while being wrong across the hole, and a line index is exactly the kind of thing that is off by one
	somewhere in the middle of a file nobody scrolled to.

Requirements:
	- C99 compiler
	- OpenGL 1.1 (fixed function, and only for the windowed mode)

Build (from the repository root):
	gcc -std=c99 demos/FUI_Editor/fui_editor_demo.c -I . -I demos/additions -I demos/dependencies -o fui_editor -lm -ldl
	./fui_editor --selftest

	Or with cmake:  cmake -S demos/FUI_Editor -B build/fui_editor && cmake --build build/fui_editor

License:
	MIT License, Copyright (c) 2017-2026 Torsten Spaete
*/

#define FPL_IMPLEMENTATION
#define FPL_NO_VIDEO_VULKAN
#define FPL_NO_AUDIO
#include <final_platform_layer.h>

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

// stb_truetype's implementation, and the embedded TrueType faces it bakes from.
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>
#include <final_fonts.h>

#define FUI_IMPLEMENTATION
#include <final_ui.h>

#define FUI_TEXTEDITOR_IMPLEMENTATION
#include <final_ui_texteditor.h>

#define FUI_STBTT_IMPLEMENTATION
#include <fui_font_stbtt.h>

#define FUI_GL1_IMPLEMENTATION
#include <fui_backend_gl1.h>

#define FUI_INPUT_FPL_IMPLEMENTATION
#include <fui_input_fpl.h>

#include <stdio.h>
#include <string.h>

#define DEMO_WINDOW_TITLE "final_ui_texteditor.h demo (FPL + OpenGL)"
#define DEMO_WINDOW_WIDTH 1280
#define DEMO_WINDOW_HEIGHT 800
#define DEMO_FONT_PIXEL_HEIGHT 34.0f
#define DEMO_FONT_ATLAS_SIDE 512u

//! Which file the demo fills its document from, relative to the repository root
#define DEMO_SOURCE_FILE_PATH "final_ui.h"

//! Where "Save & verify" writes to. A file of its OWN - the document on screen is this repository's own
//! final_ui.h, and a demo that saves over the file it is showing is a demo nobody runs twice
#define DEMO_SAVE_FILE_PATH "fui_editor_demo_saved.txt"

//! Largest file the demo is willing to read into memory
#define DEMO_MAX_FILE_BYTES (64 * 1024 * 1024)

//! Slot of the proportional face the interface itself is drawn with
#define DEMO_FACE_UI 0

//! Slot of Bitstream Vera Sans Mono, the sibling of the proportional face above
#define DEMO_FACE_VERA_MONO 1

//! Slot of Fira Code
#define DEMO_FACE_FIRA_CODE 2

//! How many faces the demo bakes
#define DEMO_FACE_COUNT 3

// ----------------------------------------------------------------------------
// > Self test
// ----------------------------------------------------------------------------

static int g_checkTotal = 0;
static int g_checkFailed = 0;
static const char *g_checkSection = "";

static void CheckSection(const char *name) {
	g_checkSection = name;
	printf("\n[%s]\n", name);
}

static void CheckImpl(const bool ok, const char *expression, const int line) {
	++g_checkTotal;
	if(ok) {
		printf("  ok   %s\n", expression);
	} else {
		++g_checkFailed;
		printf("  FAIL %s  (%s:%d)\n", expression, g_checkSection, line);
	}
}

#define CHECK(condition) CheckImpl((condition), #condition, __LINE__)
#define CHECK_I(a, b) CheckImpl((long)(a) == (long)(b), #a " == " #b, __LINE__)

//! Compares the whole document against a literal, which is the only check that proves the hole is invisible
static void CheckTextImpl(fuiEditor *editor, const char *expected, const int line) {
	char actual[1024];
	const int32_t wholeDocument = 0;
	int32_t documentLength = fuiEditorGetTextLength(editor);
	(void)fuiEditorCopyRange(editor, wholeDocument, documentLength, actual, (int32_t)sizeof(actual));

	bool isEqual = (strcmp(actual, expected) == 0);
	++g_checkTotal;
	if(isEqual) {
		printf("  ok   text == \"%s\"\n", expected);
	} else {
		++g_checkFailed;
		printf("  FAIL text == \"%s\", got \"%s\"  (%s:%d)\n", expected, actual, g_checkSection, line);
	}

	// The contiguous form has to agree with the piecewise one, or a lexer and a search would see two
	// different documents. Asking for it moves the hole, so this is checked LAST of the two.
	const char *contiguous = fuiEditorGetContiguousText(editor);
	bool contiguousIsEqual = (strcmp(contiguous, expected) == 0);
	++g_checkTotal;
	if(contiguousIsEqual) {
		printf("  ok   contiguous == \"%s\"\n", expected);
	} else {
		++g_checkFailed;
		printf("  FAIL contiguous == \"%s\", got \"%s\"  (%s:%d)\n", expected, contiguous, g_checkSection, line);
	}
}

#define CHECK_TEXT(editor, expected) CheckTextImpl((editor), (expected), __LINE__)

//! Checks one line's content, its start and its length in one go
static void CheckLineImpl(fuiEditor *editor, const int32_t lineIndex, const char *expected, const int32_t expectedStart, const int line) {
	char actual[512];
	int32_t actualLength = fuiEditorCopyLine(editor, lineIndex, actual, (int32_t)sizeof(actual));
	int32_t actualStart = fuiEditorGetLineStart(editor, lineIndex);
	size_t expectedLength = strlen(expected);

	bool isEqual = (strcmp(actual, expected) == 0) && (actualLength == (int32_t)expectedLength) && (actualStart == expectedStart);
	++g_checkTotal;
	if(isEqual) {
		printf("  ok   line %d == \"%s\" at %d\n", (int)lineIndex, expected, (int)expectedStart);
	} else {
		++g_checkFailed;
		printf("  FAIL line %d == \"%s\" at %d, got \"%s\" (length %d) at %d  (%s:%d)\n", (int)lineIndex, expected, (int)expectedStart, actual, (int)actualLength, (int)actualStart, g_checkSection, line);
	}
}

#define CHECK_LINE(editor, lineIndex, expected, expectedStart) CheckLineImpl((editor), (lineIndex), (expected), (expectedStart), __LINE__)

// ----------------------------------------------------------------------------
// > A small C lexer
// ----------------------------------------------------------------------------

/*
	Enough of C to colour this repository, and no more than that.

	It is one function over one line, which is the whole contract a fuiEditorLexer has: it is given the
	state the line starts in and answers the state the next one starts in. Only ONE thing here really
	needs that state - a block comment, which is the only construct in C that survives a line ending.
*/

//! The styles this lexer hands out, which are what its style table is indexed by
typedef enum DemoCStyle {
	DemoCStyle_Default = 0,
	DemoCStyle_Comment,
	DemoCStyle_String,
	DemoCStyle_Number,
	DemoCStyle_Keyword,
	DemoCStyle_Type,
	DemoCStyle_Preprocessor,
	DemoCStyle_Operator,
	DemoCStyle_Count,
} DemoCStyle;

//! What the lexer carries from one line to the next
typedef enum DemoCLexState {
	DemoCLexState_Normal = 0,
	DemoCLexState_InsideBlockComment,
} DemoCLexState;

static const char *g_demoCKeywords[] = {
	"auto", "break", "case", "const", "continue", "default", "do", "else", "enum", "extern",
	"for", "goto", "if", "inline", "register", "restrict", "return", "sizeof", "static",
	"struct", "switch", "typedef", "union", "volatile", "while", "true", "false", "NULL",
};

static const char *g_demoCTypes[] = {
	"bool", "char", "double", "float", "int", "long", "short", "signed", "unsigned", "void",
	"int8_t", "int16_t", "int32_t", "int64_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t",
	"size_t", "wchar_t", "va_list",
};

//! Whether a word is in one of the two tables, compared by length so no terminator is needed
static bool DemoWordIsInTable(const char *word, const int32_t wordLength, const char *const *table, const size_t tableCount) {
	for(size_t entryIndex = 0; entryIndex < tableCount; ++entryIndex) {
		const char *entry = table[entryIndex];
		size_t entryLength = strlen(entry);
		if((int32_t)entryLength != wordLength) {
			continue;
		}
		if(memcmp(entry, word, (size_t)wordLength) == 0) {
			return(true);
		}
	}
	return(false);
}

static bool DemoIsIdentifierStart(const char byte) {
	unsigned char value = (unsigned char)byte;
	bool isLower = (value >= 'a') && (value <= 'z');
	bool isUpper = (value >= 'A') && (value <= 'Z');
	return(isLower || isUpper || value == '_');
}

static bool DemoIsIdentifierPart(const char byte) {
	unsigned char value = (unsigned char)byte;
	bool isDigit = (value >= '0') && (value <= '9');
	return(DemoIsIdentifierStart(byte) || isDigit);
}

static bool DemoIsDigit(const char byte) {
	unsigned char value = (unsigned char)byte;
	return((value >= '0') && (value <= '9'));
}

//! Paints one stretch of the line with one style
static void DemoPaintStyle(uint8_t *styles, const int32_t from, const int32_t to, const DemoCStyle style) {
	for(int32_t offset = from; offset < to; ++offset) {
		styles[offset] = (uint8_t)style;
	}
}

static int32_t DemoLexCLine(fuiEditorLexRequest *request) {
	const char *text = request->text;
	int32_t length = request->textLength;
	uint8_t *styles = request->styles;
	int32_t state = request->startState;
	int32_t offset = 0;

	// A block comment that was already open runs on until it is closed, and takes the rest of the line
	// with it when it is not.
	if(state == (int32_t)DemoCLexState_InsideBlockComment) {
		int32_t commentEnd = offset;
		while(commentEnd < length) {
			bool isTheClosingPair = (text[commentEnd] == '*') && ((commentEnd + 1) < length) && (text[commentEnd + 1] == '/');
			if(isTheClosingPair) {
				commentEnd += 2;
				state = (int32_t)DemoCLexState_Normal;
				break;
			}
			commentEnd += 1;
		}
		DemoPaintStyle(styles, offset, commentEnd, DemoCStyle_Comment);
		offset = commentEnd;
	}

	// A preprocessor line is the whole line, whatever else is on it - except a comment, which is why the
	// scan below keeps running rather than painting to the end here.
	bool isAPreprocessorLine = false;
	int32_t firstNonBlank = offset;
	while(firstNonBlank < length && (text[firstNonBlank] == ' ' || text[firstNonBlank] == '\t')) {
		firstNonBlank += 1;
	}
	if(firstNonBlank < length && text[firstNonBlank] == '#') {
		isAPreprocessorLine = true;
	}

	while(offset < length) {
		char currentByte = text[offset];
		bool hasANextByte = ((offset + 1) < length);
		char nextByte = hasANextByte ? text[offset + 1] : '\0';

		if(currentByte == '/' && nextByte == '/') {
			DemoPaintStyle(styles, offset, length, DemoCStyle_Comment);
			offset = length;
			continue;
		}

		if(currentByte == '/' && nextByte == '*') {
			int32_t commentStart = offset;
			int32_t commentEnd = offset + 2;
			state = (int32_t)DemoCLexState_InsideBlockComment;
			while(commentEnd < length) {
				bool isTheClosingPair = (text[commentEnd] == '*') && ((commentEnd + 1) < length) && (text[commentEnd + 1] == '/');
				if(isTheClosingPair) {
					commentEnd += 2;
					state = (int32_t)DemoCLexState_Normal;
					break;
				}
				commentEnd += 1;
			}
			DemoPaintStyle(styles, commentStart, commentEnd, DemoCStyle_Comment);
			offset = commentEnd;
			continue;
		}

		if(currentByte == '"' || currentByte == '\'') {
			char quote = currentByte;
			int32_t stringStart = offset;
			int32_t stringEnd = offset + 1;
			while(stringEnd < length) {
				if(text[stringEnd] == '\\') {
					stringEnd += 2;
					continue;
				}
				if(text[stringEnd] == quote) {
					stringEnd += 1;
					break;
				}
				stringEnd += 1;
			}
			// A run of escapes at the very end of the line can push this past it.
			if(stringEnd > length) {
				stringEnd = length;
			}
			DemoPaintStyle(styles, stringStart, stringEnd, DemoCStyle_String);
			offset = stringEnd;
			continue;
		}

		if(DemoIsDigit(currentByte)) {
			int32_t numberStart = offset;
			int32_t numberEnd = offset;
			// Deliberately generous: hex digits, the suffixes, the dot and the exponent's sign all just
			// belong to the number. Getting that exactly right is a job for a compiler, not for a colour.
			while(numberEnd < length && (DemoIsIdentifierPart(text[numberEnd]) || text[numberEnd] == '.')) {
				numberEnd += 1;
			}
			DemoPaintStyle(styles, numberStart, numberEnd, DemoCStyle_Number);
			offset = numberEnd;
			continue;
		}

		if(DemoIsIdentifierStart(currentByte)) {
			int32_t wordStart = offset;
			int32_t wordEnd = offset;
			while(wordEnd < length && DemoIsIdentifierPart(text[wordEnd])) {
				wordEnd += 1;
			}
			int32_t wordLength = wordEnd - wordStart;

			DemoCStyle wordStyle = DemoCStyle_Default;
			if(isAPreprocessorLine && wordStart <= (firstNonBlank + 1)) {
				wordStyle = DemoCStyle_Preprocessor;
			} else if(DemoWordIsInTable(&text[wordStart], wordLength, g_demoCKeywords, fplArrayCount(g_demoCKeywords))) {
				wordStyle = DemoCStyle_Keyword;
			} else if(DemoWordIsInTable(&text[wordStart], wordLength, g_demoCTypes, fplArrayCount(g_demoCTypes))) {
				wordStyle = DemoCStyle_Type;
			}
			DemoPaintStyle(styles, wordStart, wordEnd, wordStyle);
			offset = wordEnd;
			continue;
		}

		if(currentByte == '#' && isAPreprocessorLine) {
			styles[offset] = (uint8_t)DemoCStyle_Preprocessor;
			offset += 1;
			continue;
		}

		bool isAnOperator = (currentByte != ' ') && (currentByte != '\t') && !DemoIsIdentifierPart(currentByte);
		if(isAnOperator) {
			styles[offset] = (uint8_t)DemoCStyle_Operator;
		}
		offset += 1;
	}

	return(state);
}

//! What each of those styles is drawn in
static fuiEditorStyleDef g_demoCStyleTable[DemoCStyle_Count];

static void DemoBuildCStyleTable(void) {
	g_demoCStyleTable[DemoCStyle_Default].color = fuiColorRGBA(0.0f, 0.0f, 0.0f, 0.0f);
	g_demoCStyleTable[DemoCStyle_Comment].color = fuiColorRGBA(0.44f, 0.58f, 0.42f, 1.0f);
	g_demoCStyleTable[DemoCStyle_String].color = fuiColorRGBA(0.82f, 0.62f, 0.44f, 1.0f);
	g_demoCStyleTable[DemoCStyle_Number].color = fuiColorRGBA(0.70f, 0.78f, 0.56f, 1.0f);
	g_demoCStyleTable[DemoCStyle_Keyword].color = fuiColorRGBA(0.78f, 0.55f, 0.78f, 1.0f);
	g_demoCStyleTable[DemoCStyle_Type].color = fuiColorRGBA(0.42f, 0.72f, 0.80f, 1.0f);
	g_demoCStyleTable[DemoCStyle_Preprocessor].color = fuiColorRGBA(0.72f, 0.72f, 0.48f, 1.0f);
	g_demoCStyleTable[DemoCStyle_Operator].color = fuiColorRGBA(0.62f, 0.68f, 0.76f, 1.0f);
}

static void SelfTestEmptyDocument(void) {
	CheckSection("empty document");

	fuiEditor editor;
	CHECK(fuiEditorInit(&editor, fpl_null));
	CHECK_I(fuiEditorGetTextLength(&editor), 0);
	// An empty document is ONE empty line, because that is the line a caret sits on before anything is typed.
	CHECK_I(fuiEditorGetLineCount(&editor), 1);
	CHECK_I(fuiEditorGetLineStart(&editor, 0), 0);
	CHECK_I(fuiEditorGetLineLength(&editor, 0), 0);
	CHECK_TEXT(&editor, "");
	fuiEditorRelease(&editor);
}

static void SelfTestLineIndex(void) {
	CheckSection("line index");

	fuiEditor editor;
	fuiEditorInit(&editor, fpl_null);

	CHECK(fuiEditorSetText(&editor, "alpha\nbeta\ngamma", 0));
	CHECK_I(fuiEditorGetTextLength(&editor), 16);
	CHECK_I(fuiEditorGetLineCount(&editor), 3);
	CHECK_LINE(&editor, 0, "alpha", 0);
	CHECK_LINE(&editor, 1, "beta", 6);
	CHECK_LINE(&editor, 2, "gamma", 11);

	CHECK_I(fuiEditorGetLineOfOffset(&editor, 0), 0);
	CHECK_I(fuiEditorGetLineOfOffset(&editor, 5), 0);
	CHECK_I(fuiEditorGetLineOfOffset(&editor, 6), 1);
	CHECK_I(fuiEditorGetLineOfOffset(&editor, 10), 1);
	CHECK_I(fuiEditorGetLineOfOffset(&editor, 11), 2);
	CHECK_I(fuiEditorGetLineOfOffset(&editor, 16), 2);

	// A trailing line feed makes an empty LAST line, the way every editor shows one.
	CHECK(fuiEditorSetText(&editor, "one\ntwo\n", 0));
	CHECK_I(fuiEditorGetLineCount(&editor), 3);
	CHECK_LINE(&editor, 2, "", 8);

	fuiEditorRelease(&editor);
}

static void SelfTestInsert(void) {
	CheckSection("insert");

	fuiEditor editor;
	fuiEditorInit(&editor, fpl_null);
	fuiEditorSetText(&editor, "alpha\ngamma", 0);

	// Inside a line: no line is added, and every line after it moves by the inserted length.
	CHECK(fuiEditorInsert(&editor, 5, "!", 1));
	CHECK_TEXT(&editor, "alpha!\ngamma");
	CHECK_I(fuiEditorGetLineCount(&editor), 2);
	CHECK_LINE(&editor, 1, "gamma", 7);

	// A whole new line in the middle, which is the case the line index's hole exists for.
	CHECK(fuiEditorInsert(&editor, 7, "beta\n", 5));
	CHECK_TEXT(&editor, "alpha!\nbeta\ngamma");
	CHECK_I(fuiEditorGetLineCount(&editor), 3);
	CHECK_LINE(&editor, 0, "alpha!", 0);
	CHECK_LINE(&editor, 1, "beta", 7);
	CHECK_LINE(&editor, 2, "gamma", 12);

	// At the very front, which pushes every remembered offset along.
	CHECK(fuiEditorInsert(&editor, 0, "zero\n", 5));
	CHECK_TEXT(&editor, "zero\nalpha!\nbeta\ngamma");
	CHECK_I(fuiEditorGetLineCount(&editor), 4);
	CHECK_LINE(&editor, 0, "zero", 0);
	CHECK_LINE(&editor, 3, "gamma", 17);

	// At the very end, where there is no line behind the edit at all.
	int32_t endOffset = fuiEditorGetTextLength(&editor);
	CHECK(fuiEditorInsert(&editor, endOffset, "\nomega", 6));
	CHECK_I(fuiEditorGetLineCount(&editor), 5);
	CHECK_LINE(&editor, 4, "omega", 23);

	// Several lines in one insert.
	CHECK(fuiEditorInsert(&editor, 0, "a\nb\nc\n", 6));
	CHECK_I(fuiEditorGetLineCount(&editor), 8);
	CHECK_LINE(&editor, 0, "a", 0);
	CHECK_LINE(&editor, 1, "b", 2);
	CHECK_LINE(&editor, 2, "c", 4);
	CHECK_LINE(&editor, 3, "zero", 6);

	fuiEditorRelease(&editor);
}

static void SelfTestErase(void) {
	CheckSection("erase");

	fuiEditor editor;
	fuiEditorInit(&editor, fpl_null);

	// Inside one line: nothing is removed from the index, only the offsets behind it move.
	fuiEditorSetText(&editor, "alpha\nbeta\ngamma", 0);
	CHECK(fuiEditorErase(&editor, 0, 3));
	CHECK_TEXT(&editor, "ha\nbeta\ngamma");
	CHECK_I(fuiEditorGetLineCount(&editor), 3);
	CHECK_LINE(&editor, 1, "beta", 3);
	CHECK_LINE(&editor, 2, "gamma", 8);

	// Exactly one line feed, which joins two lines into one.
	fuiEditorSetText(&editor, "alpha\nbeta\ngamma", 0);
	CHECK(fuiEditorErase(&editor, 5, 1));
	CHECK_TEXT(&editor, "alphabeta\ngamma");
	CHECK_I(fuiEditorGetLineCount(&editor), 2);
	CHECK_LINE(&editor, 0, "alphabeta", 0);
	CHECK_LINE(&editor, 1, "gamma", 10);

	// A whole line, line feed included.
	fuiEditorSetText(&editor, "alpha\nbeta\ngamma", 0);
	CHECK(fuiEditorErase(&editor, 6, 5));
	CHECK_TEXT(&editor, "alpha\ngamma");
	CHECK_I(fuiEditorGetLineCount(&editor), 2);
	CHECK_LINE(&editor, 1, "gamma", 6);

	// Across several lines at once.
	fuiEditorSetText(&editor, "a\nb\nc\nd\ne", 0);
	CHECK_I(fuiEditorGetLineCount(&editor), 5);
	CHECK(fuiEditorErase(&editor, 2, 4));
	CHECK_TEXT(&editor, "a\nd\ne");
	CHECK_I(fuiEditorGetLineCount(&editor), 3);
	CHECK_LINE(&editor, 1, "d", 2);
	CHECK_LINE(&editor, 2, "e", 4);

	// Everything.
	fuiEditorSetText(&editor, "a\nb\nc", 0);
	int32_t wholeLength = fuiEditorGetTextLength(&editor);
	CHECK(fuiEditorErase(&editor, 0, wholeLength));
	CHECK_TEXT(&editor, "");
	CHECK_I(fuiEditorGetLineCount(&editor), 1);

	// Past the end is clamped, and erasing nothing reports that it did nothing.
	CHECK(!fuiEditorErase(&editor, 0, 0));
	CHECK(!fuiEditorErase(&editor, 99, 5));

	fuiEditorRelease(&editor);
}

/*
	Typing forwards, then backwards, over the same document.

	This is the case a gap buffer is written for and the one that catches it: every step moves the hole,
	and a wrong move shows up as text that is right at one end and shuffled at the other.
*/
static void SelfTestGapMovement(void) {
	CheckSection("gap movement");

	fuiEditor editor;
	fuiEditorInit(&editor, fpl_null);
	fuiEditorSetText(&editor, "0123456789", 0);

	// Insert one character at every position from the front, walking the hole forwards.
	int32_t insertPosition = 0;
	while(insertPosition <= 10) {
		(void)fuiEditorInsert(&editor, insertPosition * 2, ".", 1);
		insertPosition += 1;
	}
	CHECK_TEXT(&editor, ".0.1.2.3.4.5.6.7.8.9.");

	// And remove them again from the back, walking the hole the other way.
	int32_t removePosition = 10;
	while(removePosition >= 0) {
		(void)fuiEditorErase(&editor, removePosition * 2, 1);
		removePosition -= 1;
	}
	CHECK_TEXT(&editor, "0123456789");
	CHECK_I(fuiEditorGetLineCount(&editor), 1);

	fuiEditorRelease(&editor);
}

//! Grows a document past its initial capacity in both dimensions, so both reserves have to move a hole
static void SelfTestGrowth(void) {
	CheckSection("growth");

	fuiEditor editor;
	fuiEditorInit(&editor, fpl_null);

	const int32_t lineCountToBuild = 5000;
	int32_t builtLines = 0;
	while(builtLines < lineCountToBuild) {
		int32_t appendOffset = fuiEditorGetTextLength(&editor);
		(void)fuiEditorInsert(&editor, appendOffset, "line\n", 5);
		builtLines += 1;
	}
	CHECK_I(fuiEditorGetLineCount(&editor), lineCountToBuild + 1);
	CHECK_I(fuiEditorGetTextLength(&editor), lineCountToBuild * 5);
	CHECK_LINE(&editor, 0, "line", 0);
	CHECK_LINE(&editor, lineCountToBuild - 1, "line", (lineCountToBuild - 1) * 5);
	CHECK_LINE(&editor, lineCountToBuild, "", lineCountToBuild * 5);

	// Every line has to answer the offset it was built at, which is what proves tailDelta stayed right
	// through all the reallocations above.
	bool everyLineStartIsRight = true;
	int32_t checkedLine = 0;
	while(checkedLine < lineCountToBuild) {
		int32_t lineStart = fuiEditorGetLineStart(&editor, checkedLine);
		if(lineStart != (checkedLine * 5)) {
			everyLineStartIsRight = false;
			break;
		}
		checkedLine += 1;
	}
	CHECK(everyLineStartIsRight);

	// Cutting the whole middle out has to leave the first and the last line untouched.
	const int32_t keptLineCount = 10;
	int32_t cutStart = keptLineCount * 5;
	int32_t cutLength = (lineCountToBuild - keptLineCount * 2) * 5;
	CHECK(fuiEditorErase(&editor, cutStart, cutLength));
	CHECK_I(fuiEditorGetLineCount(&editor), keptLineCount * 2 + 1);
	CHECK_LINE(&editor, 0, "line", 0);
	CHECK_LINE(&editor, keptLineCount * 2 - 1, "line", (keptLineCount * 2 - 1) * 5);

	fuiEditorRelease(&editor);
}

static void SelfTestLineEndings(void) {
	CheckSection("line endings");

	fuiEditor editor;
	fuiEditorInit(&editor, fpl_null);

	fuiEditorSetText(&editor, "a\nb\n", 0);
	CHECK_I(fuiEditorGetEol(&editor), fuiEditorEol_Lf);

	// A carriage return belongs to the line it ends, so it is not part of what the line SAYS.
	fuiEditorSetText(&editor, "a\r\nb\r\n", 0);
	CHECK_I(fuiEditorGetEol(&editor), fuiEditorEol_CrLf);
	CHECK_I(fuiEditorGetLineCount(&editor), 3);
	CHECK_LINE(&editor, 0, "a", 0);
	CHECK_LINE(&editor, 1, "b", 3);
	CHECK_I(fuiEditorGetLineLength(&editor, 0), 1);

	fuiEditorSetText(&editor, "a\rb\rc", 0);
	CHECK_I(fuiEditorGetEol(&editor), fuiEditorEol_Cr);
	// Only a line feed ends a line, so a text of carriage returns alone is ONE line until it is loaded
	// through an encoding that normalises it.
	CHECK_I(fuiEditorGetLineCount(&editor), 1);

	fuiEditorSetText(&editor, "a\r\nb\nc", 0);
	CHECK_I(fuiEditorGetEol(&editor), fuiEditorEol_Mixed);

	int32_t crlfLength = 0;
	const char *crlfBytes = fuiEditorEolGetBytes(fuiEditorEol_CrLf, &crlfLength);
	CHECK_I(crlfLength, 2);
	CHECK(strcmp(crlfBytes, "\r\n") == 0);
	CHECK(strcmp(fuiEditorEolGetName(fuiEditorEol_CrLf), "CRLF") == 0);

	fuiEditorRelease(&editor);
}

static void SelfTestUtf8(void) {
	CheckSection("utf-8");

	fuiEditor editor;
	fuiEditorInit(&editor, fpl_null);

	// "a" then U+00E4 (two bytes) then U+20AC (three bytes) then U+1F600 (four bytes).
	const char *mixedText = "a\xC3\xA4\xE2\x82\xAC\xF0\x9F\x98\x80";
	fuiEditorSetText(&editor, mixedText, 0);
	CHECK_I(fuiEditorGetTextLength(&editor), 10);

	CHECK_I(fuiEditorNextCodepointOffset(&editor, 0), 1);
	CHECK_I(fuiEditorNextCodepointOffset(&editor, 1), 3);
	CHECK_I(fuiEditorNextCodepointOffset(&editor, 3), 6);
	CHECK_I(fuiEditorNextCodepointOffset(&editor, 6), 10);
	CHECK_I(fuiEditorNextCodepointOffset(&editor, 10), 10);

	CHECK_I(fuiEditorPreviousCodepointOffset(&editor, 10), 6);
	CHECK_I(fuiEditorPreviousCodepointOffset(&editor, 6), 3);
	CHECK_I(fuiEditorPreviousCodepointOffset(&editor, 3), 1);
	CHECK_I(fuiEditorPreviousCodepointOffset(&editor, 1), 0);
	CHECK_I(fuiEditorPreviousCodepointOffset(&editor, 0), 0);

	// Landing in the middle of a sequence pulls back to its first byte.
	CHECK_I(fuiEditorSnapToCodepointStart(&editor, 4), 3);
	CHECK_I(fuiEditorSnapToCodepointStart(&editor, 5), 3);
	CHECK_I(fuiEditorSnapToCodepointStart(&editor, 6), 6);
	CHECK_I(fuiEditorSnapToCodepointStart(&editor, 8), 6);

	fuiEditorRelease(&editor);
}

static void SelfTestEncodings(void) {
	CheckSection("encodings");

	fuiEditor editor;
	fuiEditorInit(&editor, fpl_null);

	// A utf-8 byte order mark carries no information and is dropped on the way in.
	const uint8_t markedBytes[] = { 0xEFu, 0xBBu, 0xBFu, 'h', 'i' };
	fuiEditorEncoding utf8Encoding = fuiEditorEncodingUtf8();
	CHECK(fuiEditorLoadFromMemory(&editor, markedBytes, (int32_t)sizeof(markedBytes), &utf8Encoding));
	CHECK_TEXT(&editor, "hi");

	// A byte with its top bit set is not ascii, so it becomes the replacement character.
	const uint8_t highBytes[] = { 'a', 0xFFu, 'b' };
	fuiEditorEncoding asciiEncoding = fuiEditorEncodingAscii();
	CHECK(fuiEditorLoadFromMemory(&editor, highBytes, (int32_t)sizeof(highBytes), &asciiEncoding));
	CHECK_I(fuiEditorGetTextLength(&editor), 5);
	CHECK_TEXT(&editor, "a\xEF\xBF\xBD" "b");

	// Both converters answer the length when there is nowhere to write, which is what lets a caller size
	// its buffer in one call and fill it in the next.
	const char *euroSign = "\xE2\x82\xAC";
	uint8_t *noDestination = fpl_null;
	const int32_t noCapacity = 0;
	int32_t asciiLength = asciiEncoding.fromUtf8(asciiEncoding.userData, euroSign, 3, noDestination, noCapacity);
	CHECK_I(asciiLength, 1);

	uint8_t asciiOut[4];
	int32_t writtenLength = asciiEncoding.fromUtf8(asciiEncoding.userData, euroSign, 3, asciiOut, (int32_t)sizeof(asciiOut));
	CHECK_I(writtenLength, 1);
	CHECK_I(asciiOut[0], '?');

	fuiEditorRelease(&editor);
}

static bool DemoReadWholeFile(const char *filePath, uint8_t **outData, int32_t *outLength);

/*
	The pieces the widget is built out of, checked where they can be checked without a window.

	Everything here is a pure function of its arguments - how wide a number is written, where the next tab
	stop is, which rows a scroll offset puts in view - which is exactly the part of a widget that is worth
	testing headlessly. What is left over is the drawing, and that is what the demo itself is for.
*/
static void SelfTestViewHelpers(void) {
	CheckSection("view helpers");

	CHECK_I(fuiEditor__DigitCount(0), 1);
	CHECK_I(fuiEditor__DigitCount(9), 1);
	CHECK_I(fuiEditor__DigitCount(10), 2);
	CHECK_I(fuiEditor__DigitCount(999), 3);
	CHECK_I(fuiEditor__DigitCount(14325), 5);

	char numberText[FUI_TEXTEDITOR__MAX_NUMBER_TEXT];
	const int32_t numberCapacity = (int32_t)sizeof(numberText);
	CHECK_I(fuiEditor__FormatInt(numberText, numberCapacity, 0), 1);
	CHECK(strcmp(numberText, "0") == 0);
	CHECK_I(fuiEditor__FormatInt(numberText, numberCapacity, 4711), 4);
	CHECK(strcmp(numberText, "4711") == 0);
	CHECK_I(fuiEditor__FormatInt(numberText, numberCapacity, -12), 3);
	CHECK(strcmp(numberText, "-12") == 0);

	// The most negative int32 has no positive counterpart of its own, which is where a naive negate wraps.
	CHECK_I(fuiEditor__FormatInt(numberText, numberCapacity, -2147483647 - 1), 11);
	CHECK(strcmp(numberText, "-2147483648") == 0);

	// Appending stops at the buffer and terminates what it managed, rather than running past it.
	char shortBuffer[6];
	const int32_t shortCapacity = (int32_t)sizeof(shortBuffer);
	int32_t writeOffset = 0;
	writeOffset = fuiEditor__AppendText(shortBuffer, shortCapacity, writeOffset, "Ln ");
	writeOffset = fuiEditor__AppendInt(shortBuffer, shortCapacity, writeOffset, 123456);
	CHECK_I(writeOffset, 5);
	CHECK(strcmp(shortBuffer, "Ln 12") == 0);

	// A scroll offset of zero starts at the first row, and the range is widened by one row at each end so
	// that the row the top edge lands inside and the one the bottom edge cuts through are both drawn.
	const float lineHeight = 20.0f;
	const float viewportHeight = 100.0f;
	const int32_t screenLineCount = 1000;
	int32_t firstScreenLine = 0;
	int32_t endScreenLine = 0;
	fuiEditor__VisibleScreenLines(0.0f, viewportHeight, lineHeight, screenLineCount, &firstScreenLine, &endScreenLine);
	CHECK_I(firstScreenLine, 0);
	CHECK_I(endScreenLine, 7);

	fuiEditor__VisibleScreenLines(410.0f, viewportHeight, lineHeight, screenLineCount, &firstScreenLine, &endScreenLine);
	CHECK_I(firstScreenLine, 20);
	CHECK_I(endScreenLine, 27);

	// And it never runs past the end of the document, however far the offset has been pushed.
	fuiEditor__VisibleScreenLines(1000000.0f, viewportHeight, lineHeight, screenLineCount, &firstScreenLine, &endScreenLine);
	CHECK_I(firstScreenLine, screenLineCount);
	CHECK_I(endScreenLine, screenLineCount);

	/*
		Tab stops, which is where the arithmetic went wrong once already.

		A pen standing exactly ON a stop has to be sent to the NEXT one, and a pen that measured a
		millionth short of one has to be sent there too - otherwise the second tab of a line puts the pen
		back where it already stood, and a line indented twice draws as though it were indented once.
	*/
	fuiEditor__Render tabRender = fplZeroInit;
	tabRender.tabWidth = 32.0f;
	CHECK(fuiEditor__NextTabStopDistance(&tabRender, 0.0f) == 32.0f);
	CHECK(fuiEditor__NextTabStopDistance(&tabRender, 1.0f) == 32.0f);
	CHECK(fuiEditor__NextTabStopDistance(&tabRender, 32.0f) == 64.0f);
	CHECK(fuiEditor__NextTabStopDistance(&tabRender, 63.9f) == 64.0f);
	CHECK(fuiEditor__NextTabStopDistance(&tabRender, 64.0f) == 96.0f);

	// A hair SHORT of a stop is what a float that has been added up across a line actually looks like.
	float justUnderOneStop = 32.0f - 32.0f * 1.0e-6f;
	CHECK(fuiEditor__NextTabStopDistance(&tabRender, justUnderOneStop) == 64.0f);

	// A face too small to have measured a tab width at all leaves the pen alone rather than dividing by it.
	fuiEditor__Render noTabRender = fplZeroInit;
	CHECK(fuiEditor__NextTabStopDistance(&noTabRender, 17.0f) == 17.0f);

	fuiEditorConfig defaultConfig = fuiEditorDefaultConfig();
	CHECK(defaultConfig.toggles.showLineNumbers);
	CHECK(defaultConfig.toggles.showStatusBar);
	CHECK(defaultConfig.toggles.highlightCurrentLine);
	CHECK(defaultConfig.toggles.verticalScrollbar == fuiEditorScrollbarMode_Always);
	CHECK(defaultConfig.toggles.horizontalScrollbar == fuiEditorScrollbarMode_Auto);

	// Zero is what "the caller named none" is spelled as, in both directions.
	CHECK_I(defaultConfig.metrics.tabSize, 0);
	CHECK(defaultConfig.colors.text.a == 0.0f);
}

/*
	That the bytes can be reached in ONE piece from anywhere, hole or no hole.

	This is what lets a line be drawn straight out of the buffer rather than being copied into a scratch
	buffer that would put a limit on how long a line may be. Every range is at most two runs, and the two
	of them together have to be the same bytes fuiEditorCopyRange answers with.
*/
static void SelfTestContiguousRuns(void) {
	CheckSection("contiguous runs");

	fuiEditor editor;
	fuiEditorInit(&editor, fpl_null);
	fuiEditorSetText(&editor, "one\ntwo\nthree\nfour\n", 0);

	// Puts the hole in the middle of the document rather than at either end of it.
	const int32_t middleOffset = 8;
	fuiEditorInsert(&editor, middleOffset, "X", 1);

	int32_t documentLength = fuiEditorGetTextLength(&editor);
	CHECK(editor.document.gapStart > 0);
	CHECK(editor.document.gapStart < documentLength);

	char reassembled[64];
	int32_t writeOffset = 0;
	int32_t readOffset = 0;
	int32_t runCount = 0;
	while(readOffset < documentLength) {
		int32_t runLength = 0;
		const char *runBytes = fuiEditor__ContiguousRunAt(&editor, readOffset, documentLength, &runLength);
		if(runBytes == fpl_null || runLength <= 0) {
			break;
		}
		memcpy(&reassembled[writeOffset], runBytes, (size_t)runLength);
		writeOffset += runLength;
		readOffset += runLength;
		runCount += 1;
	}
	reassembled[writeOffset] = '\0';

	// Two of them, because a hole in the middle splits the document in exactly two.
	CHECK_I(runCount, 2);
	CHECK_I(writeOffset, documentLength);
	CHECK(strcmp(reassembled, "one\ntwo\nXthree\nfour\n") == 0);

	// A range that stops IN FRONT of the hole is one run, and one that starts behind it is one as well.
	int32_t frontRunLength = 0;
	const char *frontRun = fuiEditor__ContiguousRunAt(&editor, 0, 3, &frontRunLength);
	CHECK(frontRun != fpl_null);
	CHECK_I(frontRunLength, 3);

	int32_t backRunLength = 0;
	const char *backRun = fuiEditor__ContiguousRunAt(&editor, editor.document.gapStart, documentLength, &backRunLength);
	CHECK(backRun != fpl_null);
	CHECK_I(backRunLength, documentLength - editor.document.gapStart);

	// An empty range answers nothing at all rather than a pointer to no bytes.
	int32_t emptyRunLength = 0;
	const char *emptyRun = fuiEditor__ContiguousRunAt(&editor, documentLength, documentLength, &emptyRunLength);
	CHECK(emptyRun == fpl_null);
	CHECK_I(emptyRunLength, 0);

	fuiEditorRelease(&editor);
}

//! The caret, which so far only decides which line is washed and which number is lit
static void SelfTestCaretLine(void) {
	CheckSection("caret line");

	fuiEditor editor;
	fuiEditorInit(&editor, fpl_null);
	fuiEditorSetText(&editor, "alpha\nbeta\ngamma\n", 0);

	CHECK_I(fuiEditorGetCaretLine(&editor), 0);

	fuiEditorSetCaretLine(&editor, 2);
	CHECK_I(fuiEditorGetCaretLine(&editor), 2);
	CHECK_I(editor.caretOffset, fuiEditorGetLineStart(&editor, 2));

	// Out of range in either direction lands on the nearest line there is, rather than nowhere.
	fuiEditorSetCaretLine(&editor, 9999);
	int32_t lastLine = fuiEditorGetLineCount(&editor) - 1;
	CHECK_I(fuiEditorGetCaretLine(&editor), lastLine);

	fuiEditorSetCaretLine(&editor, -5);
	CHECK_I(fuiEditorGetCaretLine(&editor), 0);

	// A new document is a new view, so the caret goes back to the top with it.
	fuiEditorSetCaretLine(&editor, 2);
	fuiEditorSetText(&editor, "one\ntwo\n", 0);
	CHECK_I(fuiEditorGetCaretLine(&editor), 0);
	CHECK(editor.scrollY == 0.0f);

	fuiEditorRelease(&editor);
}

/*
	The acceptance test of this iteration, run headlessly: every line number the gutter would draw has to
	name the line that is really there.

	final_ui.h itself is the document, because it is the largest file to hand and the one the widget has to
	hold up against. The reference is a plain scan of the file for line feeds - the same thing "sed -n Np"
	does, and deliberately nothing the line index is involved in.
*/
static void SelfTestDocumentAgainstFile(void) {
	CheckSection("document against file");

	const char *candidatePaths[] = {
		DEMO_SOURCE_FILE_PATH,
		"../" DEMO_SOURCE_FILE_PATH,
		"../../" DEMO_SOURCE_FILE_PATH,
		"../../../" DEMO_SOURCE_FILE_PATH,
		"../../../../" DEMO_SOURCE_FILE_PATH,
	};

	uint8_t *fileData = fpl_null;
	int32_t fileLength = 0;
	size_t candidateIndex = 0;
	while(candidateIndex < fplArrayCount(candidatePaths) && fileData == fpl_null) {
		(void)DemoReadWholeFile(candidatePaths[candidateIndex], &fileData, &fileLength);
		candidateIndex += 1;
	}
	if(fileData == fpl_null) {
		printf("  skipped, %s was not found from here\n", DEMO_SOURCE_FILE_PATH);
		return;
	}

	fuiEditor editor;
	fuiEditorInit(&editor, fpl_null);
	fuiEditorEncoding utf8Encoding = fuiEditorEncodingUtf8();
	CHECK(fuiEditorLoadFromMemory(&editor, fileData, fileLength, &utf8Encoding));

	// The file is valid utf-8, so nothing was replaced on the way in and byte n of the one is byte n of
	// the other. Everything below rests on that, so it is checked rather than assumed.
	int32_t documentLength = fuiEditorGetTextLength(&editor);
	CHECK_I(documentLength, fileLength);

	int32_t referenceLineCount = 1;
	for(int32_t byteIndex = 0; byteIndex < fileLength; ++byteIndex) {
		if(fileData[byteIndex] == '\n') {
			referenceLineCount += 1;
		}
	}
	int32_t documentLineCount = fuiEditorGetLineCount(&editor);
	CHECK_I(documentLineCount, referenceLineCount);
	CHECK(documentLineCount > 14000);

	// Every line, not a sample of them: a line index that is wrong is usually wrong in one place only.
	int32_t firstWrongStart = -1;
	int32_t firstWrongContent = -1;
	int32_t firstWrongLineOfOffset = -1;
	int32_t referenceLineStart = 0;
	int32_t lineIndex = 0;
	while(lineIndex < documentLineCount && referenceLineStart <= fileLength) {
		int32_t referenceLineEnd = referenceLineStart;
		while(referenceLineEnd < fileLength && fileData[referenceLineEnd] != '\n') {
			referenceLineEnd += 1;
		}

		// A carriage return in front of the line feed belongs to the line it ends and is not part of it.
		int32_t referenceVisibleEnd = referenceLineEnd;
		bool endsWithCarriageReturn = (referenceVisibleEnd > referenceLineStart) && (fileData[referenceVisibleEnd - 1] == '\r');
		if(endsWithCarriageReturn) {
			referenceVisibleEnd -= 1;
		}

		int32_t documentLineStart = fuiEditorGetLineStart(&editor, lineIndex);
		int32_t documentLineEnd = fuiEditorGetLineEnd(&editor, lineIndex);
		if((documentLineStart != referenceLineStart || documentLineEnd != referenceVisibleEnd) && firstWrongStart < 0) {
			firstWrongStart = lineIndex;
		}

		int32_t referenceLineLength = referenceVisibleEnd - referenceLineStart;
		int32_t documentLineLength = documentLineEnd - documentLineStart;
		if(documentLineLength == referenceLineLength && firstWrongContent < 0) {
			for(int32_t byteIndex = 0; byteIndex < referenceLineLength; ++byteIndex) {
				char documentByte = fuiEditorGetByte(&editor, documentLineStart + byteIndex);
				char referenceByte = (char)fileData[referenceLineStart + byteIndex];
				if(documentByte != referenceByte) {
					firstWrongContent = lineIndex;
					break;
				}
			}
		}

		// And the way back: the offset a line begins at has to answer with that same line.
		int32_t lineOfItsOwnStart = fuiEditorGetLineOfOffset(&editor, documentLineStart);
		if(lineOfItsOwnStart != lineIndex && firstWrongLineOfOffset < 0) {
			firstWrongLineOfOffset = lineIndex;
		}

		referenceLineStart = referenceLineEnd + 1;
		lineIndex += 1;
	}

	CHECK_I(lineIndex, documentLineCount);
	CHECK_I(firstWrongStart, -1);
	CHECK_I(firstWrongContent, -1);
	CHECK_I(firstWrongLineOfOffset, -1);

	fuiEditorRelease(&editor);
	free(fileData);
}

//! Advance of every glyph of the stand-in face, in font units where the nominal size is 1.0
#define DEMO_TEST_FACE_ADVANCE 0.5f

//! Distance from one baseline to the next of the stand-in face, in the same units
#define DEMO_TEST_FACE_LINE_HEIGHT 1.25f

//! Pixel height the widget tests draw at, so one line is exactly twenty pixels tall
#define DEMO_TEST_FONT_HEIGHT 16.0f

//! How many lines the document the widget tests are built over has
#define DEMO_TEST_LINE_COUNT 1000

/*
	A face of one size, so that the geometry of a build is arithmetic rather than a measurement.

	Every codepoint is the same width, which is also what makes the widget take its monospace path - and
	that path is worth having under test, since it is the one a code editor actually runs.
*/
static bool TestFaceGetGlyph(void *userData, uint32_t codePoint, fuiGlyph *outGlyph) {
	(void)userData;
	(void)codePoint;
	fuiGlyph glyph = fplZeroInit;
	glyph.advance = DEMO_TEST_FACE_ADVANCE;
	glyph.size = fuiV2(DEMO_TEST_FACE_ADVANCE, 1.0f);
	*outGlyph = glyph;
	return(true);
}

static fuiFont MakeTestFace(void) {
	fuiFont result = fuiZeroFont();
	result.getGlyph = TestFaceGetGlyph;
	result.metrics.ascent = 1.0f;
	result.metrics.descent = 0.25f;
	result.metrics.lineHeight = DEMO_TEST_FACE_LINE_HEIGHT;
	result.metrics.spaceAdvance = DEMO_TEST_FACE_ADVANCE;
	return(result);
}

//! Builds one frame holding nothing but the editor, and answers what it drew
static const fuiDrawData *BuildOneEditorFrame(fuiContext *ui, fuiEditor *editor, const fuiRect rect) {
	fuiInput input = fuiZeroInput();
	input.windowSize = fuiV2i((int32_t)(rect.x + rect.w) + 64, (int32_t)(rect.y + rect.h) + 64);

	fuiBeginFrame(ui, &input, fuiPass_Both);
	(void)fuiTextEditor(ui, rect, "editor", editor);
	fuiEndFrame(ui);
	return(fuiGetDrawData(ui));
}

/*
	The widget itself, built headlessly against a face whose every measurement is known.

	A window is not needed to find out whether the right lines are in view, whether the scroll offset is
	clamped to what there is to scroll, or whether a box too small to hold anything is survived - and those
	are the three things that go wrong when a layout is changed. What is left over is what it LOOKS like,
	and no test answers that one.
*/
static void SelfTestWidgetLayout(void) {
	CheckSection("widget layout");

	fuiFont testFace = MakeTestFace();
	fuiContext ui;
	if(!fuiInit(&ui, &testFace, fpl_null)) {
		CHECK(false);
		return;
	}

	fuiEditor editor;
	fuiEditorInit(&editor, fpl_null);

	// One line per number, so that what is on a line says which line it is.
	const int32_t roomPerLine = 16;
	char documentText[DEMO_TEST_LINE_COUNT * 16];
	int32_t documentLength = 0;
	for(int32_t lineIndex = 0; lineIndex < DEMO_TEST_LINE_COUNT; ++lineIndex) {
		int32_t roomLeft = (int32_t)sizeof(documentText) - documentLength;
		int written = snprintf(&documentText[documentLength], (size_t)roomLeft, "line %d\n", (int)lineIndex);
		if(written <= 0 || written >= roomLeft) {
			break;
		}
		documentLength += written;
	}
	CHECK(documentLength < DEMO_TEST_LINE_COUNT * roomPerLine);
	fuiEditorSetText(&editor, documentText, documentLength);
	CHECK_I(fuiEditorGetLineCount(&editor), DEMO_TEST_LINE_COUNT + 1);

	fuiEditorConfig config = fuiEditorDefaultConfig();
	config.metrics.fontHeight = DEMO_TEST_FONT_HEIGHT;
	config.metrics.textPaddingX = 4.0f;
	config.metrics.gutterPaddingX = 4.0f;
	config.metrics.statusBarHeight = 24.0f;
	config.metrics.tabSize = 4;
	fuiEditorSetConfig(&editor, &config);

	const float editorLeft = 10.0f;
	const float editorTop = 10.0f;
	const float editorWidth = 640.0f;
	const float editorHeight = 424.0f;
	fuiRect editorRect = fuiRectMake(editorLeft, editorTop, editorWidth, editorHeight);

	const fuiDrawData *drawData = BuildOneEditorFrame(&ui, &editor, editorRect);
	CHECK(drawData->commandCount > 0);
	CHECK(drawData->vertexCount > 0);

	// A fresh view starts at the top and holds as many rows as the body has room for, widened by the one
	// the top edge lands inside and the one the bottom edge cuts through.
	CHECK_I(fuiEditorGetFirstVisibleLine(&editor), 0);
	CHECK(fuiEditorGetVisibleLineCount(&editor) > 0);
	CHECK(fuiEditorGetVisibleLineCount(&editor) < DEMO_TEST_LINE_COUNT);

	// Two builds in a row change nothing, which is what says the widget reads its own state back the way
	// it wrote it rather than drifting a little every frame.
	int32_t firstVisibleAfterOneFrame = fuiEditorGetFirstVisibleLine(&editor);
	int32_t visibleCountAfterOneFrame = fuiEditorGetVisibleLineCount(&editor);
	(void)BuildOneEditorFrame(&ui, &editor, editorRect);
	CHECK_I(fuiEditorGetFirstVisibleLine(&editor), firstVisibleAfterOneFrame);
	CHECK_I(fuiEditorGetVisibleLineCount(&editor), visibleCountAfterOneFrame);

	// Somewhere in the middle: the line asked for is the one at the top.
	const int32_t middleLine = 500;
	fuiEditorScrollToLine(&editor, middleLine);
	(void)BuildOneEditorFrame(&ui, &editor, editorRect);
	CHECK_I(fuiEditorGetFirstVisibleLine(&editor), middleLine);

	// Past the end: the offset is clamped so that the LAST line sits at the bottom, rather than the
	// document being scrolled off the top of its own box.
	const int32_t wayPastTheEnd = 999999;
	fuiEditorScrollToLine(&editor, wayPastTheEnd);
	(void)BuildOneEditorFrame(&ui, &editor, editorRect);
	int32_t lineCount = fuiEditorGetLineCount(&editor);
	int32_t firstVisibleLine = fuiEditorGetFirstVisibleLine(&editor);
	int32_t visibleLineCount = fuiEditorGetVisibleLineCount(&editor);
	CHECK_I(firstVisibleLine + visibleLineCount, lineCount);
	CHECK(editor.scrollY > 0.0f);

	// And back to the top, where there is nothing to scroll and the offset is exactly zero.
	fuiEditorScrollToLine(&editor, 0);
	(void)BuildOneEditorFrame(&ui, &editor, editorRect);
	CHECK(editor.scrollY == 0.0f);
	CHECK(editor.scrollX == 0.0f);

	// The widest line SEEN has to have been measured by now, which is what the horizontal bar goes by.
	CHECK(editor.widestMeasuredLineWidth > 0.0f);

	// An edit throws that width away rather than keeping one that belonged to text which is gone.
	fuiEditorInsert(&editor, 0, "x", 1);
	(void)BuildOneEditorFrame(&ui, &editor, editorRect);
	CHECK_I(editor.widestMeasuredVersion, editor.version);

	// Without a gutter and without a status line the same document still lays out, and holds MORE rows,
	// because the status line is no longer taking a strip off the bottom.
	int32_t visibleWithChrome = fuiEditorGetVisibleLineCount(&editor);
	config.toggles.showLineNumbers = false;
	config.toggles.showStatusBar = false;
	config.toggles.highlightCurrentLine = false;
	fuiEditorSetConfig(&editor, &config);
	(void)BuildOneEditorFrame(&ui, &editor, editorRect);
	CHECK(fuiEditorGetVisibleLineCount(&editor) > visibleWithChrome);

	// A box with no room in it at all is survived rather than laid out into the negative.
	fuiRect emptyRect = fuiRectMake(editorLeft, editorTop, 0.0f, 0.0f);
	(void)BuildOneEditorFrame(&ui, &editor, emptyRect);
	CHECK(fuiEditorGetVisibleLineCount(&editor) >= 0);

	// So is one too small to hold even the status line it was asked for.
	config.toggles.showStatusBar = true;
	fuiEditorSetConfig(&editor, &config);
	fuiRect tinyRect = fuiRectMake(editorLeft, editorTop, 12.0f, 6.0f);
	(void)BuildOneEditorFrame(&ui, &editor, tinyRect);
	CHECK(fuiEditorGetVisibleLineCount(&editor) >= 0);

	fuiEditorRelease(&editor);
	fuiRelease(&ui);
}

//! An empty document is still one empty line, and the widget has to draw that line rather than nothing
static void SelfTestWidgetEmptyDocument(void) {
	CheckSection("widget empty document");

	fuiFont testFace = MakeTestFace();
	fuiContext ui;
	if(!fuiInit(&ui, &testFace, fpl_null)) {
		CHECK(false);
		return;
	}

	fuiEditor editor;
	fuiEditorInit(&editor, fpl_null);

	// Asked for by name rather than taken from the theme, because the tab arithmetic below is checked
	// against exactly this height.
	fuiEditorConfig config = fuiEditorDefaultConfig();
	config.metrics.fontHeight = DEMO_TEST_FONT_HEIGHT;
	config.metrics.tabSize = 4;
	fuiEditorSetConfig(&editor, &config);

	fuiRect editorRect = fuiRectMake(0.0f, 0.0f, 320.0f, 200.0f);
	const fuiDrawData *drawData = BuildOneEditorFrame(&ui, &editor, editorRect);
	CHECK(drawData->commandCount > 0);
	CHECK_I(fuiEditorGetLineCount(&editor), 1);
	CHECK_I(fuiEditorGetFirstVisibleLine(&editor), 0);
	CHECK_I(fuiEditorGetVisibleLineCount(&editor), 1);
	CHECK_I(fuiEditorGetCaretLine(&editor), 0);

	// A document of tabs alone is where a line that is cut at every tab stop could come out empty.
	fuiEditorSetText(&editor, "\t\t\tx\n\ty\n", 0);
	(void)BuildOneEditorFrame(&ui, &editor, editorRect);
	CHECK_I(fuiEditorGetLineCount(&editor), 3);

	// Four tab stops of four characters at half the font height, plus the one character behind them.
	float expectedWidestWidth = (3.0f * 4.0f + 1.0f) * DEMO_TEST_FACE_ADVANCE * DEMO_TEST_FONT_HEIGHT;
	float measuredWidestWidth = editor.widestMeasuredLineWidth;
	float widthDifference = measuredWidestWidth - expectedWidestWidth;
	if(widthDifference < 0.0f) {
		widthDifference = -widthDifference;
	}
	CHECK(widthDifference < 0.01f);

	fuiEditorRelease(&editor);
	fuiRelease(&ui);
}

/*
	One editor, one context, one face of known measurements - built headlessly so that a key can be
	pressed and the answer read back without a window anywhere near it.
*/
typedef struct EditorTestHarness {
	fuiFont face;
	fuiContext ui;
	fuiEditor editor;
	fuiInput input;
	fuiRect rect;
	fuiEditorConfig config;
} EditorTestHarness;

/*
	A clipboard of the test's own, so that cutting and pasting can be checked without a window.

	It is also what makes the one case worth having a test for reachable at all: a hook that REFUSES the
	text. FPL's own does exactly that above two kilobytes, and a cut that deleted the selection anyway
	would be a delete with no way back.
*/
#define DEMO_TEST_CLIPBOARD_CAPACITY 4096
static char g_testClipboard[DEMO_TEST_CLIPBOARD_CAPACITY];
static bool g_testClipboardRefusesEverything = false;
static int32_t g_testClipboardSetCount = 0;

static bool TestClipboardGet(void *userData, char *destination, uint32_t maxDestinationLength) {
	(void)userData;
	fplCopyString(g_testClipboard, destination, maxDestinationLength);
	return(true);
}

static bool TestClipboardSet(void *userData, const char *text) {
	(void)userData;
	if(g_testClipboardRefusesEverything) {
		return(false);
	}
	size_t textLength = fplGetStringLength(text);
	if(textLength >= DEMO_TEST_CLIPBOARD_CAPACITY) {
		return(false);
	}
	fplCopyString(text, g_testClipboard, DEMO_TEST_CLIPBOARD_CAPACITY);
	g_testClipboardSetCount += 1;
	return(true);
}

static bool HarnessInit(EditorTestHarness *harness, const char *text, const float editorWidth, const float editorHeight) {
	fplClearStruct(harness);
	harness->face = MakeTestFace();
	if(!fuiInit(&harness->ui, &harness->face, fpl_null)) {
		return(false);
	}
	if(!fuiEditorInit(&harness->editor, fpl_null)) {
		fuiRelease(&harness->ui);
		return(false);
	}

	harness->config = fuiEditorDefaultConfig();
	harness->config.metrics.fontHeight = DEMO_TEST_FONT_HEIGHT;
	harness->config.metrics.textPaddingX = 4.0f;
	harness->config.metrics.gutterPaddingX = 4.0f;
	harness->config.metrics.statusBarHeight = 24.0f;
	harness->config.metrics.tabSize = 4;
	fuiEditorSetConfig(&harness->editor, &harness->config);

	if(text != fpl_null) {
		fuiEditorSetText(&harness->editor, text, 0);
	}

	fuiPlatform testPlatform = fplZeroInit;
	testPlatform.getClipboardText = TestClipboardGet;
	testPlatform.setClipboardText = TestClipboardSet;
	fuiSetPlatform(&harness->ui, &testPlatform);
	g_testClipboard[0] = '\0';
	g_testClipboardRefusesEverything = false;
	g_testClipboardSetCount = 0;

	harness->rect = fuiRectMake(10.0f, 10.0f, editorWidth, editorHeight);
	harness->input = fuiZeroInput();
	harness->input.windowSize = fuiV2i((int32_t)(editorWidth + 64.0f), (int32_t)(editorHeight + 64.0f));
	harness->input.deltaTime = 1.0f / 60.0f;
	harness->input.isActive = true;
	return(true);
}

static void HarnessRelease(EditorTestHarness *harness) {
	fuiEditorRelease(&harness->editor);
	fuiRelease(&harness->ui);
}

//! Builds one frame and then clears every input EDGE, the way a real frame's input would have moved on
static fuiEditorAction HarnessFrame(EditorTestHarness *harness) {
	fuiBeginFrame(&harness->ui, &harness->input, fuiPass_Both);
	fuiEditorAction action = fuiTextEditor(&harness->ui, harness->rect, "editor", &harness->editor);
	fuiEndFrame(&harness->ui);

	harness->input.mouseWheelDelta = 0.0f;
	harness->input.textInputLength = 0;
	for(int32_t keyIndex = 0; keyIndex < (int32_t)fuiKey_Count; ++keyIndex) {
		harness->input.keys[keyIndex].halfTransitionCount = 0;
		harness->input.keys[keyIndex].endedDown = false;
	}
	for(int32_t buttonIndex = 0; buttonIndex < FUI_MOUSE_BUTTON_COUNT; ++buttonIndex) {
		harness->input.mouseButtons[buttonIndex].halfTransitionCount = 0;
		harness->input.mouseButtons[buttonIndex].endedDown = false;
	}
	return(action);
}

//! Presses one key with its modifiers and builds the frame that sees it
static void HarnessPressKey(EditorTestHarness *harness, const fuiKey key, const bool withShift, const bool withControl) {
	harness->input.keys[key].halfTransitionCount = 1;
	harness->input.keys[key].endedDown = true;
	if(withShift) {
		harness->input.keys[fuiKey_LeftShift].halfTransitionCount = 1;
		harness->input.keys[fuiKey_LeftShift].endedDown = true;
	}
	if(withControl) {
		harness->input.keys[fuiKey_LeftControl].halfTransitionCount = 1;
		harness->input.keys[fuiKey_LeftControl].endedDown = true;
	}
	(void)HarnessFrame(harness);
}

//! Gives the editor the keyboard, which a click would otherwise have done
static void HarnessFocusTheEditor(EditorTestHarness *harness) {
	fuiId editorId = fuiGetId(&harness->ui, "editor");
	fuiSetFocusedId(&harness->ui, editorId);
}

//! Types a utf-8 text as ONE frame's worth of codepoints, which is what a fast typist really delivers
static void HarnessTypeText(EditorTestHarness *harness, const char *utf8Text, const bool withControl) {
	size_t textLength = fplGetStringLength(utf8Text);
	size_t readOffset = 0;
	int32_t typedCount = 0;
	while(readOffset < textLength && typedCount < FUI_MAX_TEXT_INPUT) {
		uint32_t codepoint = fuiDecodeUtf8(utf8Text, textLength, &readOffset);
		if(codepoint == 0) {
			break;
		}
		harness->input.textInput[typedCount] = codepoint;
		typedCount += 1;
	}
	harness->input.textInputLength = typedCount;
	if(withControl) {
		harness->input.keys[fuiKey_LeftControl].halfTransitionCount = 1;
		harness->input.keys[fuiKey_LeftControl].endedDown = true;
	}
	(void)HarnessFrame(harness);
}

//! Presses the middle mouse button at a point in the widget, which is what pastes there
static void HarnessClickMiddleAt(EditorTestHarness *harness, const float x, const float y) {
	// Hovering is resolved against the PREVIOUS build, so the pointer has to stand there for a frame
	// before the press can be seen as happening over the editor at all.
	harness->input.mousePosition = fuiV2(x, y);
	(void)HarnessFrame(harness);

	harness->input.mouseButtons[FUI_MOUSE_MIDDLE].halfTransitionCount = 1;
	harness->input.mouseButtons[FUI_MOUSE_MIDDLE].endedDown = true;
	(void)HarnessFrame(harness);
}

/*
	Where a character sits and which character a place belongs to, which have to be each other's inverse.

	Every tab stop in the line is a place where the two could disagree, so the line under test has one at
	the front, one in the middle and text on both sides of it.
*/
static void SelfTestLineGeometry(void) {
	CheckSection("line geometry");

	EditorTestHarness harness;
	if(!HarnessInit(&harness, "ab\tcd", 640.0f, 424.0f)) {
		CHECK(false);
		return;
	}
	(void)HarnessFrame(&harness);

	fuiEditor__Render render = fuiEditor__MakeRender(&harness.ui, &harness.editor.resolvedConfig);
	CHECK(render.isMonospace);

	float characterWidth = render.characterWidth;
	float tabWidth = characterWidth * 4.0f;
	int32_t lineStart = fuiEditorGetLineStart(&harness.editor, 0);
	int32_t lineEnd = fuiEditorGetLineEnd(&harness.editor, 0);
	CHECK_I(lineEnd - lineStart, 5);

	// "ab" is two characters, the tab then jumps to the stop at four, and "cd" follows it.
	float distanceAtA = fuiEditor__DistanceOfOffset(&harness.ui, &harness.editor, &render, lineStart, lineEnd, 0);
	float distanceAtB = fuiEditor__DistanceOfOffset(&harness.ui, &harness.editor, &render, lineStart, lineEnd, 1);
	float distanceAtTab = fuiEditor__DistanceOfOffset(&harness.ui, &harness.editor, &render, lineStart, lineEnd, 2);
	float distanceAtC = fuiEditor__DistanceOfOffset(&harness.ui, &harness.editor, &render, lineStart, lineEnd, 3);
	float distanceAtEnd = fuiEditor__DistanceOfOffset(&harness.ui, &harness.editor, &render, lineStart, lineEnd, 5);
	CHECK(distanceAtA == 0.0f);
	CHECK(distanceAtB == characterWidth);
	CHECK(distanceAtTab == characterWidth * 2.0f);
	CHECK(distanceAtC == tabWidth);
	CHECK(distanceAtEnd == tabWidth + characterWidth * 2.0f);

	// And the way back. A point just inside a character belongs to it; one just past its middle belongs
	// to the next.
	int32_t offsetAtNothing = fuiEditor__OffsetAtDistance(&harness.ui, &harness.editor, &render, lineStart, lineEnd, 0.0f);
	int32_t offsetJustInsideB = fuiEditor__OffsetAtDistance(&harness.ui, &harness.editor, &render, lineStart, lineEnd, characterWidth * 1.1f);
	int32_t offsetPastTheTab = fuiEditor__OffsetAtDistance(&harness.ui, &harness.editor, &render, lineStart, lineEnd, tabWidth + characterWidth * 0.1f);
	int32_t offsetPastTheEnd = fuiEditor__OffsetAtDistance(&harness.ui, &harness.editor, &render, lineStart, lineEnd, tabWidth * 10.0f);
	CHECK_I(offsetAtNothing, 0);
	CHECK_I(offsetJustInsideB, 1);
	CHECK_I(offsetPastTheTab, 3);
	CHECK_I(offsetPastTheEnd, 5);

	// Every boundary of the line has to survive the round trip through both of them.
	int32_t firstWrongRoundTrip = -1;
	for(int32_t offset = lineStart; offset <= lineEnd; ++offset) {
		float distance = fuiEditor__DistanceOfOffset(&harness.ui, &harness.editor, &render, lineStart, lineEnd, offset);
		int32_t backAgain = fuiEditor__OffsetAtDistance(&harness.ui, &harness.editor, &render, lineStart, lineEnd, distance);
		if(backAgain != offset && firstWrongRoundTrip < 0) {
			firstWrongRoundTrip = offset;
		}
	}
	CHECK_I(firstWrongRoundTrip, -1);

	HarnessRelease(&harness);
}

//! What counts as a word, which is what ctrl and an arrow, and a double click, both go by
static void SelfTestWords(void) {
	CheckSection("words");

	fuiEditor editor;
	fuiEditorInit(&editor, fpl_null);
	fuiEditorSetText(&editor, "foo bar_baz  qux(1)\nnext", 0);

	// 0123456789...
	// foo bar_baz  qux(1)
	CHECK_I(fuiEditor__NextWordOffset(&editor, 0), 4);
	CHECK_I(fuiEditor__NextWordOffset(&editor, 4), 13);
	CHECK_I(fuiEditor__NextWordOffset(&editor, 13), 16);
	CHECK_I(fuiEditor__PreviousWordOffset(&editor, 11), 4);
	CHECK_I(fuiEditor__PreviousWordOffset(&editor, 4), 0);
	CHECK_I(fuiEditor__PreviousWordOffset(&editor, 0), 0);

	// Punctuation is a run of its own, so "qux(1)" is four jumps rather than one.
	CHECK_I(fuiEditor__NextWordOffset(&editor, 16), 17);
	CHECK_I(fuiEditor__NextWordOffset(&editor, 17), 18);

	// And a jump never runs over a line break: one press reaches it, the next one crosses it.
	int32_t lineBreakOffset = 19;
	CHECK_I(fuiEditor__NextWordOffset(&editor, 18), lineBreakOffset);
	CHECK_I(fuiEditor__NextWordOffset(&editor, lineBreakOffset), lineBreakOffset + 1);

	int32_t wordStart = 0;
	int32_t wordEnd = 0;
	fuiEditor__WordRangeAt(&editor, 6, &wordStart, &wordEnd);
	CHECK_I(wordStart, 4);
	CHECK_I(wordEnd, 11);

	// The bracket is punctuation, so it is its own run rather than part of the name in front of it.
	fuiEditor__WordRangeAt(&editor, 16, &wordStart, &wordEnd);
	CHECK_I(wordStart, 16);
	CHECK_I(wordEnd, 17);

	// A whole line comes WITH its ending, so pasting it back somewhere puts a line there.
	int32_t lineStart = 0;
	int32_t lineEnd = 0;
	fuiEditor__LineRangeAt(&editor, 5, &lineStart, &lineEnd);
	CHECK_I(lineStart, 0);
	CHECK_I(lineEnd, 20);

	// The last line has no ending to take with it.
	fuiEditor__LineRangeAt(&editor, 21, &lineStart, &lineEnd);
	CHECK_I(lineStart, 20);
	CHECK_I(lineEnd, fuiEditorGetTextLength(&editor));

	fuiEditorRelease(&editor);
}

//! The selection itself, without a widget anywhere near it
static void SelfTestSelection(void) {
	CheckSection("selection");

	fuiEditor editor;
	fuiEditorInit(&editor, fpl_null);
	fuiEditorSetText(&editor, "alpha\nbeta\ngamma", 0);

	CHECK(!fuiEditorHasSelection(&editor));
	CHECK_I(fuiEditorGetSelectionStart(&editor), 0);
	CHECK_I(fuiEditorGetSelectionEnd(&editor), 0);

	fuiEditorSetSelection(&editor, 6, 10);
	CHECK(fuiEditorHasSelection(&editor));
	CHECK_I(fuiEditorGetSelectionStart(&editor), 6);
	CHECK_I(fuiEditorGetSelectionEnd(&editor), 10);
	CHECK_I(fuiEditorGetCaretOffset(&editor), 10);

	char selectedText[32];
	int32_t selectedLength = fuiEditorCopySelection(&editor, selectedText, (int32_t)sizeof(selectedText));
	CHECK_I(selectedLength, 4);
	CHECK(strcmp(selectedText, "beta") == 0);

	// Held down at the far end and dragged back is the same selection, and the caret is at the near end.
	fuiEditorSetSelection(&editor, 10, 6);
	CHECK_I(fuiEditorGetSelectionStart(&editor), 6);
	CHECK_I(fuiEditorGetSelectionEnd(&editor), 10);
	CHECK_I(fuiEditorGetCaretOffset(&editor), 6);

	fuiEditorClearSelection(&editor);
	CHECK(!fuiEditorHasSelection(&editor));
	CHECK_I(fuiEditorGetCaretOffset(&editor), 6);

	fuiEditorSelectAll(&editor);
	CHECK_I(fuiEditorGetSelectionStart(&editor), 0);
	CHECK_I(fuiEditorGetSelectionEnd(&editor), fuiEditorGetTextLength(&editor));

	// Asking with no buffer answers the full length, which is what sizes one.
	char *noBuffer = fpl_null;
	int32_t neededLength = fuiEditorCopySelection(&editor, noBuffer, 0);
	CHECK_I(neededLength, fuiEditorGetTextLength(&editor));

	// The caret lands on a codepoint boundary however badly it is aimed.
	fuiEditorSetText(&editor, "a\xC3\xA4" "b", 0);
	fuiEditorSetCaretOffset(&editor, 2, false);
	CHECK_I(fuiEditorGetCaretOffset(&editor), 1);
	CHECK_I(fuiEditorGetCaretColumn(&editor), 1);
	fuiEditorSetCaretOffset(&editor, 3, false);
	CHECK_I(fuiEditorGetCaretOffset(&editor), 3);
	CHECK_I(fuiEditorGetCaretColumn(&editor), 2);

	fuiEditorRelease(&editor);
}

/*
	That the scrollbar is still THERE when the frame is finished.

	It was not, for three iterations: the background covers the whole frame and was drawn after the bars,
	so both of them were painted over the moment they were drawn. Nothing about the layout was wrong, the
	bars really were built, and every check that counted geometry passed - which is why this one goes by
	the ORDER the geometry was emitted in instead.

	The thumb is the last thing the bar draws and it carries the widget colour; the editor's background
	carries the track colour and nothing else in this build does. So the last vertex of the track colour
	has to come BEFORE the last vertex of the thumb's. The status line is switched off because it carries
	the widget colour too, and it is drawn after everything.
*/
static void SelfTestScrollbarSurvivesTheBackground(void) {
	CheckSection("scrollbar is not painted over");

	EditorTestHarness harness;
	if(!HarnessInit(&harness, fpl_null, 640.0f, 424.0f)) {
		CHECK(false);
		return;
	}

	const int32_t lineCount = 400;
	static char documentText[400 * 16];
	int32_t documentLength = 0;
	for(int32_t lineIndex = 0; lineIndex < lineCount; ++lineIndex) {
		int32_t roomLeft = (int32_t)sizeof(documentText) - documentLength;
		int written = snprintf(&documentText[documentLength], (size_t)roomLeft, "line %d\n", (int)lineIndex);
		if(written <= 0 || written >= roomLeft) {
			break;
		}
		documentLength += written;
	}
	fuiEditorSetText(&harness.editor, documentText, documentLength);

	harness.config.toggles.showStatusBar = false;
	harness.config.toggles.verticalScrollbar = fuiEditorScrollbarMode_Always;
	harness.config.toggles.horizontalScrollbar = fuiEditorScrollbarMode_Never;
	fuiEditorSetConfig(&harness.editor, &harness.config);

	(void)HarnessFrame(&harness);
	const fuiDrawData *drawData = fuiGetDrawData(&harness.ui);

	fuiTheme *theme = fuiGetTheme(&harness.ui);
	uint32_t trackColor = fuiPackColor(theme->widgetTrackColor);
	uint32_t thumbColor = fuiPackColor(theme->widgetColor);

	int32_t lastTrackColouredVertex = -1;
	int32_t lastThumbColouredVertex = -1;
	for(uint32_t vertexIndex = 0; vertexIndex < drawData->vertexCount; ++vertexIndex) {
		uint32_t vertexColor = drawData->vertices[vertexIndex].color;
		if(vertexColor == trackColor) {
			lastTrackColouredVertex = (int32_t)vertexIndex;
		}
		if(vertexColor == thumbColor) {
			lastThumbColouredVertex = (int32_t)vertexIndex;
		}
	}

	CHECK(lastTrackColouredVertex >= 0);
	CHECK(lastThumbColouredVertex >= 0);
	CHECK(lastThumbColouredVertex > lastTrackColouredVertex);

	HarnessRelease(&harness);
}

//! Every key the editor answers to, pressed against a document whose lines say which line they are
static void SelfTestKeyboard(void) {
	CheckSection("keyboard");

	EditorTestHarness harness;
	if(!HarnessInit(&harness, "aaaaaaaaaa\nbb\ncccccccccc\ndddd", 640.0f, 424.0f)) {
		CHECK(false);
		return;
	}
	(void)HarnessFrame(&harness);
	HarnessFocusTheEditor(&harness);

	const bool noShift = false;
	const bool noControl = false;
	const bool withShift = true;
	const bool withControl = true;

	HarnessPressKey(&harness, fuiKey_Down, noShift, noControl);
	CHECK_I(fuiEditorGetCaretLine(&harness.editor), 1);
	HarnessPressKey(&harness, fuiKey_Down, noShift, noControl);
	CHECK_I(fuiEditorGetCaretLine(&harness.editor), 2);
	HarnessPressKey(&harness, fuiKey_Up, noShift, noControl);
	CHECK_I(fuiEditorGetCaretLine(&harness.editor), 1);

	HarnessPressKey(&harness, fuiKey_End, noShift, noControl);
	CHECK_I(fuiEditorGetCaretColumn(&harness.editor), 2);
	HarnessPressKey(&harness, fuiKey_Home, noShift, noControl);
	CHECK_I(fuiEditorGetCaretColumn(&harness.editor), 0);

	HarnessPressKey(&harness, fuiKey_End, noShift, withControl);
	CHECK_I(fuiEditorGetCaretOffset(&harness.editor), fuiEditorGetTextLength(&harness.editor));
	HarnessPressKey(&harness, fuiKey_Home, noShift, withControl);
	CHECK_I(fuiEditorGetCaretOffset(&harness.editor), 0);

	/*
		The column a caret WANTS.

		Walked off the end of a long line, down across a short one and on, it has to come back out at the
		column it started in. A caret that only remembered where it landed would be stuck at the short
		line's width from there on - which is the single most noticeable thing an editor can get wrong.
	*/
	HarnessPressKey(&harness, fuiKey_End, noShift, noControl);
	CHECK_I(fuiEditorGetCaretColumn(&harness.editor), 10);
	HarnessPressKey(&harness, fuiKey_Down, noShift, noControl);
	CHECK_I(fuiEditorGetCaretColumn(&harness.editor), 2);
	HarnessPressKey(&harness, fuiKey_Down, noShift, noControl);
	CHECK_I(fuiEditorGetCaretColumn(&harness.editor), 10);

	// Shift drags the selection along, and a plain arrow drops it and lands on its END rather than one
	// character past it.
	HarnessPressKey(&harness, fuiKey_Home, noShift, withControl);
	HarnessPressKey(&harness, fuiKey_Right, withShift, noControl);
	HarnessPressKey(&harness, fuiKey_Right, withShift, noControl);
	CHECK(fuiEditorHasSelection(&harness.editor));
	CHECK_I(fuiEditorGetSelectionStart(&harness.editor), 0);
	CHECK_I(fuiEditorGetSelectionEnd(&harness.editor), 2);
	HarnessPressKey(&harness, fuiKey_Right, noShift, noControl);
	CHECK(!fuiEditorHasSelection(&harness.editor));
	CHECK_I(fuiEditorGetCaretOffset(&harness.editor), 2);

	HarnessPressKey(&harness, fuiKey_A, noShift, withControl);
	CHECK_I(fuiEditorGetSelectionStart(&harness.editor), 0);
	CHECK_I(fuiEditorGetSelectionEnd(&harness.editor), fuiEditorGetTextLength(&harness.editor));

	/*
		That the caret is really EMITTED, which is the one thing a screenshot of an unfocused editor cannot
		show. An editor without the keyboard draws no caret; the same editor with it draws one more quad.
	*/
	fuiSetFocusedId(&harness.ui, FUI_ID_NONE);
	(void)HarnessFrame(&harness);
	const fuiDrawData *unfocusedDrawData = fuiGetDrawData(&harness.ui);
	uint32_t vertexCountWithoutCaret = unfocusedDrawData->vertexCount;

	HarnessFocusTheEditor(&harness);
	harness.editor.caretBlinkTime = 0.0f;
	(void)HarnessFrame(&harness);
	const fuiDrawData *focusedDrawData = fuiGetDrawData(&harness.ui);
	uint32_t vertexCountWithCaret = focusedDrawData->vertexCount;
	CHECK(vertexCountWithCaret > vertexCountWithoutCaret);

	/*
		The blink, which has to be LIT the moment the caret moves.

		A caret that happens to be in its dark half while somebody is typing looks like the keystroke was
		lost, so every move resets the phase rather than letting it run on.
	*/
	fuiTheme *theme = fuiGetTheme(&harness.ui);
	float blinkPeriod = 1.0f / theme->caretBlinkHz;
	const float noTimeAtAll = 0.0f;
	harness.editor.caretBlinkTime = 0.0f;
	CHECK(fuiEditor__AdvanceCaretBlink(&harness.editor, theme, noTimeAtAll));
	CHECK(!fuiEditor__AdvanceCaretBlink(&harness.editor, theme, blinkPeriod));
	CHECK(fuiEditor__AdvanceCaretBlink(&harness.editor, theme, blinkPeriod));

	// A frame that ate a whole stall comes back ON PHASE rather than running the cycles it missed.
	harness.editor.caretBlinkTime = 0.0f;
	const float aVeryLongStall = 60.0f;
	(void)fuiEditor__AdvanceCaretBlink(&harness.editor, theme, aVeryLongStall);
	CHECK(harness.editor.caretBlinkTime < (blinkPeriod * 2.0f));

	harness.editor.caretBlinkTime = blinkPeriod * 1.5f;
	fuiEditorSetCaretOffset(&harness.editor, 3, false);
	CHECK(harness.editor.caretBlinkTime == 0.0f);
	CHECK(fuiEditor__AdvanceCaretBlink(&harness.editor, theme, noTimeAtAll));

	// A key that moves nothing reports nothing, which is what the caller's didMoveCaret is for.
	fuiEditorSetCaretOffset(&harness.editor, 0, false);
	fuiEditorAction quietAction = HarnessFrame(&harness);
	CHECK(!quietAction.didMoveCaret);
	fuiEditorAction movingAction;
	harness.input.keys[fuiKey_Down].halfTransitionCount = 1;
	harness.input.keys[fuiKey_Down].endedDown = true;
	movingAction = HarnessFrame(&harness);
	CHECK(movingAction.didMoveCaret);

	HarnessRelease(&harness);
}

/*
	The wheel against the caret.

	Scrolling away from the caret and then doing nothing has to LEAVE the view where it was put. Bringing
	the caret back into view unconditionally is the classic way to nail a document down: the wheel moves
	it, the next frame drags it back, and it looks like the wheel is broken.
*/
static void SelfTestWheelDoesNotFightTheCaret(void) {
	CheckSection("wheel against caret");

	EditorTestHarness harness;
	const int32_t lineCount = 400;
	char documentText[400 * 16];
	int32_t documentLength = 0;
	for(int32_t lineIndex = 0; lineIndex < lineCount; ++lineIndex) {
		int32_t roomLeft = (int32_t)sizeof(documentText) - documentLength;
		int written = snprintf(&documentText[documentLength], (size_t)roomLeft, "line %d\n", (int)lineIndex);
		if(written <= 0 || written >= roomLeft) {
			break;
		}
		documentLength += written;
	}
	if(!HarnessInit(&harness, fpl_null, 640.0f, 424.0f)) {
		CHECK(false);
		return;
	}
	fuiEditorSetText(&harness.editor, documentText, documentLength);

	// Twice, because hovering is resolved against the PREVIOUS build - the first frame is what makes the
	// editor the thing under the cursor.
	harness.input.mousePosition = fuiV2(harness.rect.x + 200.0f, harness.rect.y + 100.0f);
	(void)HarnessFrame(&harness);
	(void)HarnessFrame(&harness);
	CHECK(harness.editor.scrollY == 0.0f);

	const float threeNotchesBackwards = -3.0f;
	harness.input.mouseWheelDelta = threeNotchesBackwards;
	(void)HarnessFrame(&harness);
	float scrollAfterTheWheel = harness.editor.scrollY;
	CHECK(scrollAfterTheWheel > 0.0f);

	// And now nothing at all happens for three frames.
	(void)HarnessFrame(&harness);
	(void)HarnessFrame(&harness);
	(void)HarnessFrame(&harness);
	CHECK(harness.editor.scrollY == scrollAfterTheWheel);
	CHECK_I(fuiEditorGetCaretOffset(&harness.editor), 0);

	// A key that DOES move the caret pulls the view back to it, which is the other half of the same rule.
	HarnessFocusTheEditor(&harness);
	HarnessPressKey(&harness, fuiKey_Down, false, false);
	CHECK(harness.editor.scrollY < scrollAfterTheWheel);

	HarnessRelease(&harness);
}

/*
	The acceptance test of this iteration: select everything and copy it out.

	final_ui.h itself is the document, and what comes back has to be the file byte for byte. Everything the
	editor does to text - the hole it is stored around, the line index over it, the offsets the selection
	is made of - is wrong somewhere if this is off by a single byte.
*/
static void SelfTestCopyAgainstFile(void) {
	CheckSection("copy against file");

	const char *candidatePaths[] = {
		DEMO_SOURCE_FILE_PATH,
		"../" DEMO_SOURCE_FILE_PATH,
		"../../" DEMO_SOURCE_FILE_PATH,
		"../../../" DEMO_SOURCE_FILE_PATH,
		"../../../../" DEMO_SOURCE_FILE_PATH,
	};

	uint8_t *fileData = fpl_null;
	int32_t fileLength = 0;
	size_t candidateIndex = 0;
	while(candidateIndex < fplArrayCount(candidatePaths) && fileData == fpl_null) {
		(void)DemoReadWholeFile(candidatePaths[candidateIndex], &fileData, &fileLength);
		candidateIndex += 1;
	}
	if(fileData == fpl_null) {
		printf("  skipped, %s was not found from here\n", DEMO_SOURCE_FILE_PATH);
		return;
	}

	fuiEditor editor;
	fuiEditorInit(&editor, fpl_null);
	fuiEditorEncoding utf8Encoding = fuiEditorEncodingUtf8();
	CHECK(fuiEditorLoadFromMemory(&editor, fileData, fileLength, &utf8Encoding));

	fuiEditorSelectAll(&editor);
	CHECK_I(fuiEditorGetSelectionStart(&editor), 0);
	CHECK_I(fuiEditorGetSelectionEnd(&editor), fileLength);

	char *noBuffer = fpl_null;
	int32_t neededLength = fuiEditorCopySelection(&editor, noBuffer, 0);
	CHECK_I(neededLength, fileLength);

	char *copiedText = (char *)malloc((size_t)neededLength + 1);
	if(copiedText != fpl_null) {
		int32_t copiedLength = fuiEditorCopySelection(&editor, copiedText, neededLength + 1);
		CHECK_I(copiedLength, fileLength);
		CHECK(memcmp(copiedText, fileData, (size_t)fileLength) == 0);
		CHECK(copiedText[fileLength] == '\0');
		free(copiedText);
	} else {
		CHECK(false);
	}

	// And one selection that does NOT start at zero, so the offsets are exercised rather than the length.
	const int32_t someWayIn = 100000;
	const int32_t someLength = 50000;
	fuiEditorSetSelection(&editor, someWayIn, someWayIn + someLength);
	char *middleText = (char *)malloc((size_t)someLength + 1);
	if(middleText != fpl_null) {
		int32_t middleLength = fuiEditorCopySelection(&editor, middleText, someLength + 1);
		CHECK_I(middleLength, someLength);
		CHECK(memcmp(middleText, &fileData[someWayIn], (size_t)someLength) == 0);
		free(middleText);
	} else {
		CHECK(false);
	}

	fuiEditorRelease(&editor);
	free(fileData);
}

//! How many lines the lexer under test was asked about, which is the whole point of the checks below
static int32_t g_lexCallCount = 0;

//! The C lexer with a counter around it, so it can be asked how much work a change really cost
static int32_t CountingLexLine(fuiEditorLexRequest *request) {
	g_lexCallCount += 1;
	return(DemoLexCLine(request));
}

/*
	The incremental colouring, and the one number that says whether it is incremental at all.

	Colouring a document once costs one call per line - there is no way around that. What matters is the
	SECOND time: a change near the top of a large file has to cost two calls and not fourteen thousand,
	and that only works if a recomputed state that comes out equal to the stored one is taken as proof
	that everything behind it is still right.
*/
static void SelfTestIncrementalColouring(void) {
	CheckSection("incremental colouring");

	const int32_t lineCount = 2000;
	static char documentText[2000 * 24];
	int32_t documentLength = 0;
	for(int32_t lineIndex = 0; lineIndex < lineCount; ++lineIndex) {
		int32_t roomLeft = (int32_t)sizeof(documentText) - documentLength;
		int written = snprintf(&documentText[documentLength], (size_t)roomLeft, "int value%d = %d;\n", (int)lineIndex, (int)lineIndex);
		if(written <= 0 || written >= roomLeft) {
			break;
		}
		documentLength += written;
	}

	fuiEditor editor;
	fuiEditorInit(&editor, fpl_null);
	fuiEditorSetText(&editor, documentText, documentLength);

	DemoBuildCStyleTable();
	fuiEditorLexer lexer = fplZeroInit;
	lexer.lexLine = CountingLexLine;
	lexer.styles = g_demoCStyleTable;
	lexer.styleCount = (int32_t)DemoCStyle_Count;
	fuiEditorSetLexer(&editor, &lexer);

	int32_t documentLineCount = fuiEditorGetLineCount(&editor);
	int32_t lastLine = documentLineCount - 1;
	CHECK_I(editor.styledUpToLine, 1);

	// Colouring only reaches as far as it is asked to.
	g_lexCallCount = 0;
	fuiEditor__LexUpToLine(&editor, 9);
	CHECK_I(g_lexCallCount, 9);
	CHECK_I(editor.styledUpToLine, 10);

	// And then the whole way, which is one call per line and no more.
	g_lexCallCount = 0;
	fuiEditor__LexUpToLine(&editor, lastLine);
	CHECK_I(g_lexCallCount, lastLine - 9);
	CHECK_I(editor.styledUpToLine, documentLineCount);

	// Asking again when nothing has changed costs nothing at all.
	g_lexCallCount = 0;
	fuiEditor__LexUpToLine(&editor, lastLine);
	CHECK_I(g_lexCallCount, 0);

	/*
		The case this whole scheme exists for: a change at the TOP of the document while the view is at the
		bottom of it. The state coming out of line two is unchanged, so line three onwards is already right.
	*/
	int32_t earlyLineEnd = fuiEditorGetLineEnd(&editor, 2);
	fuiEditorInsert(&editor, earlyLineEnd, " // touched", 0);
	CHECK_I(editor.styledUpToLine, 3);

	g_lexCallCount = 0;
	fuiEditor__LexUpToLine(&editor, lastLine);
	CHECK(g_lexCallCount <= 2);
	CHECK_I(editor.styledUpToLine, fuiEditorGetLineCount(&editor));

	/*
		And the other half of the same rule: a change that really DOES alter the state may not stop early.
		An unclosed block comment puts every line behind it into a comment, and every one of them has to
		be looked at. It goes on a line of its own, clear of the one that just got a // comment - a block
		comment opened behind a line comment is not opened at all.
	*/
	int32_t commentLineEnd = fuiEditorGetLineEnd(&editor, 5);
	fuiEditorInsert(&editor, commentLineEnd, " /* opened", 0);

	g_lexCallCount = 0;
	fuiEditor__LexUpToLine(&editor, lastLine);
	CHECK(g_lexCallCount > (lineCount / 2));
	CHECK_I(editor.styledUpToLine, fuiEditorGetLineCount(&editor));

	// The state really did travel: a line far below now starts inside the comment.
	int32_t stateFarBelow = fuiEditor__LineIndexGetLexerState(&editor.document.lines, 1500);
	CHECK_I(stateFarBelow, (int32_t)DemoCLexState_InsideBlockComment);

	// Closing it again lets everything behind the close come back out of the comment.
	int32_t closingLineEnd = fuiEditorGetLineEnd(&editor, 7);
	fuiEditorInsert(&editor, closingLineEnd, " */", 0);
	fuiEditor__LexUpToLine(&editor, lastLine);
	int32_t stateAfterTheClose = fuiEditor__LineIndexGetLexerState(&editor.document.lines, 1500);
	CHECK_I(stateAfterTheClose, (int32_t)DemoCLexState_Normal);

	// Taking the lexer away leaves nothing believed, so installing another one starts from scratch.
	fuiEditorSetLexer(&editor, fpl_null);
	CHECK_I(editor.styledUpToLine, 1);

	fuiEditorRelease(&editor);
}

//! That a line's state slot really does travel with the line through an insert
static void SelfTestLexerStatesFollowTheirLines(void) {
	CheckSection("lexer states follow lines");

	fuiEditor editor;
	fuiEditorInit(&editor, fpl_null);
	fuiEditorSetText(&editor, "a\nb\nc\nd\ne\n", 0);

	fuiEditorLineIndex *index = &editor.document.lines;
	for(int32_t lineIndex = 0; lineIndex < fuiEditorGetLineCount(&editor); ++lineIndex) {
		fuiEditor__LineIndexSetLexerState(index, lineIndex, 100 + lineIndex);
	}

	/*
		A whole line inserted in the middle pushes every state BEHIND it along with its line.

		The line the insert lands in keeps its own slot: inserting "X\n" at the start of line two splits
		that line, the head of the split ("X") stays in slot two, and the tail (the old "c") gets a fresh
		slot after it. So slot two keeps 102 and slot three is the new one - which is exactly why the pass
		below is not allowed to converge on it.
	*/
	// Cleared first, as a document that has been coloured all the way through would leave it, so that what
	// the insert puts there is the insert's own doing and not what filling the document left behind.
	editor.lexConvergenceFloor = 0;
	int32_t thirdLineStart = fuiEditorGetLineStart(&editor, 2);
	fuiEditorInsert(&editor, thirdLineStart, "X\n", 0);
	CHECK_I(fuiEditorGetLineCount(&editor), 7);
	CHECK_I(fuiEditor__LineIndexGetLexerState(index, 0), 100);
	CHECK_I(fuiEditor__LineIndexGetLexerState(index, 1), 101);
	CHECK_I(fuiEditor__LineIndexGetLexerState(index, 2), 102);
	CHECK_I(fuiEditor__LineIndexGetLexerState(index, 4), 103);
	CHECK_I(fuiEditor__LineIndexGetLexerState(index, 5), 104);

	// And the new slot is held below the floor, so nothing may stop on whatever it happens to hold.
	CHECK_I(editor.lexConvergenceFloor, 3);

	// An erase takes the slot back out again and everything behind it moves back.
	fuiEditorErase(&editor, thirdLineStart, 2);
	CHECK_I(fuiEditorGetLineCount(&editor), 6);
	CHECK_I(fuiEditor__LineIndexGetLexerState(index, 2), 102);
	CHECK_I(fuiEditor__LineIndexGetLexerState(index, 3), 103);

	fuiEditorRelease(&editor);
}

//! The two searches the decoration layer is looked up through
static void SelfTestDecorationLookup(void) {
	CheckSection("decoration lookup");

	fuiEditorLineDecoration lineDecorations[4];
	FUI_TEXTEDITOR_MEMSET(lineDecorations, 0, sizeof(lineDecorations));
	lineDecorations[0].line = 3;
	lineDecorations[1].line = 10;
	lineDecorations[2].line = 11;
	lineDecorations[3].line = 40;

	fuiEditorRangeDecoration rangeDecorations[3];
	FUI_TEXTEDITOR_MEMSET(rangeDecorations, 0, sizeof(rangeDecorations));
	rangeDecorations[0].startOffset = 10;
	rangeDecorations[0].endOffset = 20;
	rangeDecorations[1].startOffset = 100;
	rangeDecorations[1].endOffset = 110;
	rangeDecorations[2].startOffset = 300;
	rangeDecorations[2].endOffset = 305;

	fuiEditorDecorations decorations = fplZeroInit;
	decorations.lines = lineDecorations;
	decorations.lineCount = (int32_t)fplArrayCount(lineDecorations);
	decorations.ranges = rangeDecorations;
	decorations.rangeCount = (int32_t)fplArrayCount(rangeDecorations);

	CHECK_I(fuiEditor__FirstLineDecorationFrom(&decorations, 0), 0);
	CHECK_I(fuiEditor__FirstLineDecorationFrom(&decorations, 3), 0);
	CHECK_I(fuiEditor__FirstLineDecorationFrom(&decorations, 4), 1);
	CHECK_I(fuiEditor__FirstLineDecorationFrom(&decorations, 11), 2);
	CHECK_I(fuiEditor__FirstLineDecorationFrom(&decorations, 41), 4);

	// The cursor walks forward and answers only for the line it was asked about.
	int32_t cursor = 0;
	const fuiEditorLineDecoration *found = fuiEditor__LineDecorationAt(&decorations, &cursor, 3);
	CHECK(found != fpl_null);
	found = fuiEditor__LineDecorationAt(&decorations, &cursor, 4);
	CHECK(found == fpl_null);
	found = fuiEditor__LineDecorationAt(&decorations, &cursor, 11);
	CHECK(found != fpl_null);
	CHECK_I(found->line, 11);

	// A range that STARTED before the offset may still reach into it, so the search steps one back.
	CHECK_I(fuiEditor__FirstRangeDecorationFrom(&decorations, 0), 0);
	CHECK_I(fuiEditor__FirstRangeDecorationFrom(&decorations, 15), 0);
	CHECK_I(fuiEditor__FirstRangeDecorationFrom(&decorations, 105), 1);
	CHECK_I(fuiEditor__FirstRangeDecorationFrom(&decorations, 400), 2);
}

//! Which ending a line really carries, which is what the CR/LF marks are drawn from
static void SelfTestLineEndingsOfLines(void) {
	CheckSection("endings of lines");

	fuiEditor editor;
	fuiEditorInit(&editor, fpl_null);
	fuiEditorSetText(&editor, "unix\nwindows\r\nunix again\nlast", 0);

	CHECK(fuiEditor__LineEndingOf(&editor, 0) == fuiEditorEol_Lf);
	CHECK(fuiEditor__LineEndingOf(&editor, 1) == fuiEditorEol_CrLf);
	CHECK(fuiEditor__LineEndingOf(&editor, 2) == fuiEditorEol_Lf);

	// A carriage return belongs to the line it ends and is not part of what the line SAYS.
	CHECK_I(fuiEditorGetLineLength(&editor, 1), 7);
	CHECK(fuiEditorGetEol(&editor) == fuiEditorEol_Mixed);

	fuiEditorRelease(&editor);
}

// ----------------------------------------------------------------------------
// > Writing
// ----------------------------------------------------------------------------

/*
	What a burst of typing costs.

	Everything the frame delivered has to become ONE insert. Checking the resulting text would pass either
	way - what says it is one edit is the document's VERSION, which every insert and every erase bumps
	exactly once. The same rule found the bug in the colouring watermark last iteration: check the cost,
	not only the result.
*/
static void SelfTestTyping(void) {
	CheckSection("typing");

	EditorTestHarness harness;
	if(!HarnessInit(&harness, "ab\ncd", 640.0f, 424.0f)) {
		CHECK(false);
		return;
	}
	(void)HarnessFrame(&harness);
	HarnessFocusTheEditor(&harness);

	const bool noControl = false;
	const bool withControl = true;
	const bool dropTheSelection = false;

	fuiEditorSetCaretOffset(&harness.editor, 1, dropTheSelection);
	int32_t versionBeforeTyping = harness.editor.version;
	HarnessTypeText(&harness, "XY", noControl);
	CHECK_TEXT(&harness.editor, "aXYb\ncd");
	CHECK_I(fuiEditorGetCaretOffset(&harness.editor), 3);
	CHECK_I(harness.editor.version - versionBeforeTyping, 1);
	CHECK(fuiEditorIsModified(&harness.editor));

	// A chord is a keystroke, not typing. Control and v is a paste and must not leave a "v" behind.
	int32_t versionBeforeTheChord = harness.editor.version;
	HarnessTypeText(&harness, "Z", withControl);
	CHECK_I(harness.editor.version, versionBeforeTheChord);
	CHECK_TEXT(&harness.editor, "aXYb\ncd");

	// Typing over a selection replaces it, in one edit rather than a delete and an insert.
	fuiEditorSetSelection(&harness.editor, 1, 4);
	int32_t versionBeforeTheReplacement = harness.editor.version;
	HarnessTypeText(&harness, "Q", noControl);
	CHECK_TEXT(&harness.editor, "aQ\ncd");
	CHECK_I(fuiEditorGetCaretOffset(&harness.editor), 2);
	CHECK(!fuiEditorHasSelection(&harness.editor));
	CHECK_I(harness.editor.version - versionBeforeTheReplacement, 2);

	// Multi-byte characters go in whole. A caret that landed inside one would be the end of everything.
	fuiEditorSetCaretOffset(&harness.editor, 0, dropTheSelection);
	HarnessTypeText(&harness, "\xc3\xa4", noControl);
	CHECK_TEXT(&harness.editor, "\xc3\xa4" "aQ\ncd");
	CHECK_I(fuiEditorGetCaretOffset(&harness.editor), 2);

	// fuiEditorClearModified is what saving puts the flag back with, and nothing else clears it.
	fuiEditorClearModified(&harness.editor);
	CHECK(!fuiEditorIsModified(&harness.editor));

	HarnessRelease(&harness);
}

static void SelfTestEnterBackspaceDelete(void) {
	CheckSection("enter, backspace and delete");

	EditorTestHarness harness;
	if(!HarnessInit(&harness, "abcd", 640.0f, 424.0f)) {
		CHECK(false);
		return;
	}
	(void)HarnessFrame(&harness);
	HarnessFocusTheEditor(&harness);

	const bool noShift = false;
	const bool withShift = true;
	const bool noControl = false;
	const bool dropTheSelection = false;

	fuiEditorSetCaretOffset(&harness.editor, 2, dropTheSelection);
	HarnessPressKey(&harness, fuiKey_Return, noShift, noControl);
	CHECK_TEXT(&harness.editor, "ab\ncd");
	CHECK_I(fuiEditorGetLineCount(&harness.editor), 2);
	CHECK_I(fuiEditorGetCaretOffset(&harness.editor), 3);

	HarnessPressKey(&harness, fuiKey_Backspace, noShift, noControl);
	CHECK_TEXT(&harness.editor, "abcd");
	CHECK_I(fuiEditorGetCaretOffset(&harness.editor), 2);

	HarnessPressKey(&harness, fuiKey_Delete, noShift, noControl);
	CHECK_TEXT(&harness.editor, "abd");

	// Neither end of the document has anything to remove, and neither may wrap around to the other one.
	fuiEditorSetCaretOffset(&harness.editor, 0, dropTheSelection);
	int32_t versionAtTheFront = harness.editor.version;
	HarnessPressKey(&harness, fuiKey_Backspace, noShift, noControl);
	CHECK_I(harness.editor.version, versionAtTheFront);
	int32_t documentLength = fuiEditorGetTextLength(&harness.editor);
	fuiEditorSetCaretOffset(&harness.editor, documentLength, dropTheSelection);
	int32_t versionAtTheBack = harness.editor.version;
	HarnessPressKey(&harness, fuiKey_Delete, noShift, noControl);
	CHECK_I(harness.editor.version, versionAtTheBack);

	// Both of them take the SELECTION when there is one, rather than the character beside the caret.
	fuiEditorSetSelection(&harness.editor, 0, 2);
	HarnessPressKey(&harness, fuiKey_Backspace, noShift, noControl);
	CHECK_TEXT(&harness.editor, "d");
	fuiEditorSetText(&harness.editor, "abcd", 0);
	fuiEditorSetSelection(&harness.editor, 1, 3);
	HarnessPressKey(&harness, fuiKey_Delete, noShift, noControl);
	CHECK_TEXT(&harness.editor, "ad");

	/*
		A carriage return and the line feed behind it are ONE ending.

		Taking the feed alone would leave the return standing at the end of the joined line: a character
		nothing draws, nothing selects and nobody can find, in a file that looks exactly right.
	*/
	fuiEditorSetText(&harness.editor, "ab\r\ncd", 0);
	CHECK(fuiEditorGetEol(&harness.editor) == fuiEditorEol_CrLf);
	fuiEditorSetCaretOffset(&harness.editor, 4, dropTheSelection);
	HarnessPressKey(&harness, fuiKey_Backspace, noShift, noControl);
	CHECK_TEXT(&harness.editor, "abcd");
	CHECK_I(fuiEditorGetCaretOffset(&harness.editor), 2);

	fuiEditorSetText(&harness.editor, "ab\r\ncd", 0);
	fuiEditorSetCaretOffset(&harness.editor, 2, dropTheSelection);
	HarnessPressKey(&harness, fuiKey_Delete, noShift, noControl);
	CHECK_TEXT(&harness.editor, "abcd");

	// And enter writes back what the document arrived with, so a crlf file stays crlf rather than mixed.
	fuiEditorSetText(&harness.editor, "ab\r\ncd", 0);
	fuiEditorSetCaretOffset(&harness.editor, 1, dropTheSelection);
	HarnessPressKey(&harness, fuiKey_Return, noShift, noControl);
	CHECK_TEXT(&harness.editor, "a\r\nb\r\ncd");
	CHECK_I(fuiEditorGetLineCount(&harness.editor), 3);
	CHECK(fuiEditorGetEol(&harness.editor) == fuiEditorEol_CrLf);

	(void)withShift;
	HarnessRelease(&harness);
}

static void SelfTestOverwriteMode(void) {
	CheckSection("overwrite mode");

	EditorTestHarness harness;
	if(!HarnessInit(&harness, "abc\ndef", 640.0f, 424.0f)) {
		CHECK(false);
		return;
	}
	(void)HarnessFrame(&harness);
	HarnessFocusTheEditor(&harness);

	const bool noShift = false;
	const bool noControl = false;
	const bool dropTheSelection = false;

	CHECK(!fuiEditorIsOverwriting(&harness.editor));
	HarnessPressKey(&harness, fuiKey_Insert, noShift, noControl);
	CHECK(fuiEditorIsOverwriting(&harness.editor));

	fuiEditorSetCaretOffset(&harness.editor, 0, dropTheSelection);
	HarnessTypeText(&harness, "X", noControl);
	CHECK_TEXT(&harness.editor, "Xbc\ndef");
	CHECK_I(fuiEditorGetCaretOffset(&harness.editor), 1);

	// Two characters at once eat two, which is what a paste of two would have to do as well.
	HarnessTypeText(&harness, "YZ", noControl);
	CHECK_TEXT(&harness.editor, "XYZ\ndef");

	/*
		At the end of a line there is nothing to overwrite, so it INSERTS.

		Eating the break would join the line to the one below it, and joining two lines is not what
		replacing a character means in any editor anybody has used.
	*/
	CHECK_I(fuiEditorGetCaretOffset(&harness.editor), 3);
	HarnessTypeText(&harness, "W", noControl);
	CHECK_TEXT(&harness.editor, "XYZW\ndef");
	CHECK_I(fuiEditorGetLineCount(&harness.editor), 2);

	// And a break TYPED while overwriting still splits, rather than replacing a character with a newline.
	fuiEditorSetCaretOffset(&harness.editor, 1, dropTheSelection);
	HarnessPressKey(&harness, fuiKey_Return, noShift, noControl);
	CHECK_TEXT(&harness.editor, "X\nYZW\ndef");

	HarnessPressKey(&harness, fuiKey_Insert, noShift, noControl);
	CHECK(!fuiEditorIsOverwriting(&harness.editor));
	fuiEditorSetCaretOffset(&harness.editor, 0, dropTheSelection);
	HarnessTypeText(&harness, "Q", noControl);
	CHECK_TEXT(&harness.editor, "QX\nYZW\ndef");

	HarnessRelease(&harness);
}

static void SelfTestCutPasteAndLines(void) {
	CheckSection("cut, paste and the line commands");

	EditorTestHarness harness;
	if(!HarnessInit(&harness, "one\ntwo\nthree", 640.0f, 424.0f)) {
		CHECK(false);
		return;
	}
	(void)HarnessFrame(&harness);
	HarnessFocusTheEditor(&harness);

	const bool noShift = false;
	const bool noControl = false;
	const bool withControl = true;

	// Ctrl+x on a selection: the clipboard gets it, the document loses it.
	fuiEditorSetSelection(&harness.editor, 4, 7);
	HarnessPressKey(&harness, fuiKey_X, noShift, withControl);
	CHECK_TEXT(&harness.editor, "one\n\nthree");
	CHECK(strcmp(g_testClipboard, "two") == 0);

	// And ctrl+v puts it back where the caret was left.
	HarnessPressKey(&harness, fuiKey_V, noShift, withControl);
	CHECK_TEXT(&harness.editor, "one\ntwo\nthree");
	CHECK_I(fuiEditorGetCaretOffset(&harness.editor), 7);

	/*
		Ctrl+x with NOTHING selected takes the whole line, its ending included.

		Without the ending it would be a way to blank a line rather than a way to move one, and pasting it
		back somewhere else would run it into whatever is already there.
	*/
	fuiEditorSetCaretLine(&harness.editor, 1);
	HarnessPressKey(&harness, fuiKey_X, noShift, withControl);
	CHECK_TEXT(&harness.editor, "one\nthree");
	CHECK(strcmp(g_testClipboard, "two\n") == 0);

	// Ctrl+d on the LAST line takes the ending in front of it - there is none behind it to take.
	fuiEditorSetCaretLine(&harness.editor, 1);
	HarnessPressKey(&harness, fuiKey_D, noShift, withControl);
	CHECK_TEXT(&harness.editor, "one");
	CHECK_I(fuiEditorGetLineCount(&harness.editor), 1);

	/*
		A cut whose COPY failed must not delete anything.

		FPL's own clipboard hook refuses above two kilobytes, and there is no undo stack to catch this
		until the next iteration - so a cut that went ahead anyway would be a delete with no way back.
	*/
	fuiEditorSetText(&harness.editor, "keep me", 0);
	fuiEditorSelectAll(&harness.editor);
	g_testClipboardRefusesEverything = true;
	int32_t versionBeforeTheRefusedCut = harness.editor.version;
	HarnessPressKey(&harness, fuiKey_X, noShift, withControl);
	CHECK_I(harness.editor.version, versionBeforeTheRefusedCut);
	CHECK_TEXT(&harness.editor, "keep me");
	g_testClipboardRefusesEverything = false;

	// Shift and insert is the other spelling of paste, and shift and delete of cut.
	fuiEditorSetText(&harness.editor, "abc", 0);
	fplCopyString("!", g_testClipboard, DEMO_TEST_CLIPBOARD_CAPACITY);
	fuiEditorSetCaretOffset(&harness.editor, 3, false);
	HarnessPressKey(&harness, fuiKey_Insert, true, noControl);
	CHECK_TEXT(&harness.editor, "abc!");
	fuiEditorSetSelection(&harness.editor, 0, 3);
	HarnessPressKey(&harness, fuiKey_Delete, true, noControl);
	CHECK_TEXT(&harness.editor, "!");
	CHECK(strcmp(g_testClipboard, "abc") == 0);

	(void)noShift;
	HarnessRelease(&harness);
}

/*
	The middle mouse button pastes where it is CLICKED, not where the caret was.

	That is what it does everywhere on x11, and it is the reason final_ui.h needed fuiMouseButtonWentDown:
	fuiInteract answers for the left button and for nothing else.
*/
static void SelfTestMiddleButtonPaste(void) {
	CheckSection("middle button paste");

	EditorTestHarness harness;
	if(!HarnessInit(&harness, "aaaaaaaaaa\nbbbbbbbbbb\ncccccccccc", 640.0f, 424.0f)) {
		CHECK(false);
		return;
	}
	(void)HarnessFrame(&harness);
	HarnessFocusTheEditor(&harness);

	fplCopyString("PASTED", g_testClipboard, DEMO_TEST_CLIPBOARD_CAPACITY);

	// The caret is parked at the very end, so a paste that landed AT the caret rather than at the pointer
	// would show up on the last line instead of the first.
	int32_t documentLength = fuiEditorGetTextLength(&harness.editor);
	fuiEditorSetCaretOffset(&harness.editor, documentLength, false);

	int32_t lengthOfTheFirstLineBefore = fuiEditorGetLineLength(&harness.editor, 0);
	float insideTheFirstLineX = harness.rect.x + 200.0f;
	float insideTheFirstLineY = harness.rect.y + DEMO_TEST_FONT_HEIGHT * 0.5f;
	HarnessClickMiddleAt(&harness, insideTheFirstLineX, insideTheFirstLineY);

	int32_t lengthOfTheFirstLineAfter = fuiEditorGetLineLength(&harness.editor, 0);
	CHECK_I(lengthOfTheFirstLineAfter - lengthOfTheFirstLineBefore, 6);
	CHECK_I(fuiEditorGetCaretLine(&harness.editor), 0);
	CHECK_I(fuiEditorGetLineCount(&harness.editor), 3);

	HarnessRelease(&harness);
}

/*
	One toggle, and it has to hold every writing branch there is.

	Reading is not writing, so ctrl+c goes on working - and neither is the PROGRAM writing, so
	fuiEditorInsert goes on working too. A read-only editor a caller cannot fill is a view onto nothing.
*/
static void SelfTestReadOnly(void) {
	CheckSection("read only");

	EditorTestHarness harness;
	if(!HarnessInit(&harness, "one\ntwo\nthree", 640.0f, 424.0f)) {
		CHECK(false);
		return;
	}
	harness.config.toggles.isReadOnly = true;
	fuiEditorSetConfig(&harness.editor, &harness.config);
	(void)HarnessFrame(&harness);
	HarnessFocusTheEditor(&harness);

	const bool noShift = false;
	const bool noControl = false;
	const bool withControl = true;

	CHECK(fuiEditorIsReadOnly(&harness.editor));
	fplCopyString("nope", g_testClipboard, DEMO_TEST_CLIPBOARD_CAPACITY);
	fuiEditorSetCaretOffset(&harness.editor, 2, false);

	int32_t versionBefore = harness.editor.version;
	HarnessTypeText(&harness, "X", noControl);
	HarnessPressKey(&harness, fuiKey_Return, noShift, noControl);
	HarnessPressKey(&harness, fuiKey_Backspace, noShift, noControl);
	HarnessPressKey(&harness, fuiKey_Delete, noShift, noControl);
	HarnessPressKey(&harness, fuiKey_V, noShift, withControl);
	HarnessPressKey(&harness, fuiKey_X, noShift, withControl);
	HarnessPressKey(&harness, fuiKey_D, noShift, withControl);
	CHECK_I(harness.editor.version, versionBefore);
	CHECK_TEXT(&harness.editor, "one\ntwo\nthree");
	CHECK(!fuiEditorIsModified(&harness.editor));

	// The api a user reaches through is shut as well, and every one of them says so rather than lying.
	CHECK(!fuiEditorInsertAtCaret(&harness.editor, "X", 1));
	CHECK(!fuiEditorInsertLineBreak(&harness.editor));
	CHECK(!fuiEditorDeleteBackward(&harness.editor));
	CHECK(!fuiEditorDeleteForward(&harness.editor));
	CHECK(!fuiEditorDeleteLine(&harness.editor, 0));
	fuiEditorSelectAll(&harness.editor);
	CHECK(!fuiEditorDeleteSelection(&harness.editor));
	CHECK_I(harness.editor.version, versionBefore);

	// Copying is reading, so it still works.
	g_testClipboardSetCount = 0;
	HarnessPressKey(&harness, fuiKey_C, noShift, withControl);
	CHECK_I(g_testClipboardSetCount, 1);

	// And the PROGRAM may still fill it, which is what makes a read-only view usable at all.
	CHECK(fuiEditorInsert(&harness.editor, 0, "filled ", 7));
	CHECK(harness.editor.version != versionBefore);

	HarnessRelease(&harness);
}

/*
	An edit moves the caret and the selection that stood behind it.

	Without this a caller that inserts a line at the top of a document would have to know that the caret it
	left on line five hundred is now on line five hundred and one - and every caller would get it wrong in
	the same way.
*/
static void SelfTestEditsMoveTheCaret(void) {
	CheckSection("an edit moves the caret");

	fuiEditor editor;
	if(!fuiEditorInit(&editor, fpl_null)) {
		CHECK(false);
		return;
	}
	const bool dropTheSelection = false;

	fuiEditorSetText(&editor, "0123456789", 0);
	CHECK_I(fuiEditorGetCaretOffset(&editor), 0);

	fuiEditorSetCaretOffset(&editor, 5, dropTheSelection);
	CHECK(fuiEditorInsert(&editor, 2, "abc", 3));
	CHECK_I(fuiEditorGetCaretOffset(&editor), 8);

	// An insert BEHIND the caret leaves it exactly where it was.
	CHECK(fuiEditorInsert(&editor, 10, "z", 1));
	CHECK_I(fuiEditorGetCaretOffset(&editor), 8);

	// An erase in front of it pulls it back by what went away.
	CHECK(fuiEditorErase(&editor, 0, 3));
	CHECK_I(fuiEditorGetCaretOffset(&editor), 5);

	// And an erase THROUGH it collapses it onto where the erase happened.
	CHECK(fuiEditorErase(&editor, 4, 3));
	CHECK_I(fuiEditorGetCaretOffset(&editor), 4);

	// The selection is two positions and both of them move.
	fuiEditorSetText(&editor, "0123456789", 0);
	fuiEditorSetSelection(&editor, 2, 6);
	CHECK(fuiEditorInsert(&editor, 0, "AB", 2));
	CHECK_I(fuiEditorGetSelectionStart(&editor), 4);
	CHECK_I(fuiEditorGetSelectionEnd(&editor), 8);

	// A load is not an edit: the caret goes back to the front rather than being carried to the end by the
	// insert that filled the document.
	fuiEditorSetText(&editor, "a much longer document than the one before it", 0);
	CHECK_I(fuiEditorGetCaretOffset(&editor), 0);
	CHECK(!fuiEditorHasSelection(&editor));

	fuiEditorRelease(&editor);
}

static int32_t g_changeCallCount = 0;
static fuiEditorChange g_lastChange;

static void TestOnChange(fuiEditor *editor, const fuiEditorChange *change, void *userData) {
	(void)editor;
	(void)userData;
	g_changeCallCount += 1;
	g_lastChange = *change;
}

static void SelfTestChangeCallback(void) {
	CheckSection("the change callback");

	fuiEditor editor;
	if(!fuiEditorInit(&editor, fpl_null)) {
		CHECK(false);
		return;
	}

	fuiEditorConfig config = fuiEditorDefaultConfig();
	config.callbacks.onChange = TestOnChange;
	fuiEditorSetConfig(&editor, &config);

	// Filling the document is not a CHANGE to it: the caller is the one who did it, and being told about
	// one's own load is noise at best and a recursion at worst.
	g_changeCallCount = 0;
	fuiEditorSetText(&editor, "one\ntwo\nthree", 0);
	CHECK_I(g_changeCallCount, 0);

	fuiEditorSetCaretOffset(&editor, 4, false);
	CHECK(fuiEditorInsertAtCaret(&editor, "ab", 2));
	CHECK_I(g_changeCallCount, 1);
	CHECK_I(g_lastChange.offset, 4);
	CHECK_I(g_lastChange.removedBytes, 0);
	CHECK_I(g_lastChange.insertedBytes, 2);
	CHECK_I(g_lastChange.firstLine, 1);
	CHECK_I(g_lastChange.lineCountDelta, 0);

	// A text with a break in it says how many lines came with it, so a caller keeping something per line
	// knows how far to shift it.
	CHECK(fuiEditorInsertAtCaret(&editor, "x\ny\nz", 5));
	CHECK_I(g_changeCallCount, 2);
	CHECK_I(g_lastChange.lineCountDelta, 2);

	CHECK(fuiEditorErase(&editor, 0, 4));
	CHECK_I(g_changeCallCount, 3);
	CHECK_I(g_lastChange.offset, 0);
	CHECK_I(g_lastChange.removedBytes, 4);
	CHECK_I(g_lastChange.insertedBytes, 0);
	CHECK_I(g_lastChange.lineCountDelta, -1);

	fuiEditorRelease(&editor);
}

//! A generator with a fixed seed, so a failure in the run below is reproducible rather than a story
static uint32_t TestNextRandom(uint32_t *state) {
	*state = (*state) * 1664525u + 1013904223u;
	return(*state);
}

/*
	The acceptance criterion of this iteration, without a window: edit, and let the bytes prove it.

	A long run of inserts and deletes at pseudo random places is applied to the editor and to a plain
	malloc'd buffer at the same time, and the two are compared byte for byte at the end. The gap buffer,
	the split line index, the shared tail offset and the caret bookkeeping all have to agree with a
	reference implementation that is too dumb to be wrong - and the line index is checked separately
	against a raw scan for line feeds, because a document whose BYTES are right and whose LINES are not
	looks perfect until something asks it for line nine thousand.
*/
static void SelfTestEditsAgainstAPlainBuffer(void) {
	CheckSection("edits against a plain buffer");

	const char *candidatePaths[] = {
		DEMO_SOURCE_FILE_PATH,
		"../" DEMO_SOURCE_FILE_PATH,
		"../../" DEMO_SOURCE_FILE_PATH,
		"../../../" DEMO_SOURCE_FILE_PATH,
		"../../../../" DEMO_SOURCE_FILE_PATH,
	};

	uint8_t *fileData = fpl_null;
	int32_t fileLength = 0;
	size_t candidateIndex = 0;
	while(candidateIndex < fplArrayCount(candidatePaths) && fileData == fpl_null) {
		(void)DemoReadWholeFile(candidatePaths[candidateIndex], &fileData, &fileLength);
		candidateIndex += 1;
	}
	if(fileData == fpl_null) {
		printf("  skipped, %s was not found from here\n", DEMO_SOURCE_FILE_PATH);
		return;
	}

	fuiEditor editor;
	if(!fuiEditorInit(&editor, fpl_null)) {
		free(fileData);
		CHECK(false);
		return;
	}
	fuiEditorSetText(&editor, (const char *)fileData, fileLength);

	const int32_t stepCount = 400;
	const int32_t longestInsert = 16;
	const int32_t longestErase = 24;
	int32_t mirrorCapacity = fileLength + stepCount * longestInsert + longestInsert;
	char *mirror = (char *)malloc((size_t)mirrorCapacity);
	if(mirror == fpl_null) {
		fuiEditorRelease(&editor);
		free(fileData);
		CHECK(false);
		return;
	}
	memcpy(mirror, fileData, (size_t)fileLength);
	int32_t mirrorLength = fileLength;
	free(fileData);

	const char *insertTexts[] = { "x", "hello", "\n", "ab\ncd", "  \t", "\xc3\xa4\xc3\xb6", "// note\n", "}\n\n" };
	uint32_t randomState = 0x13579BDFu;
	bool everyStepAgreed = true;

	for(int32_t stepIndex = 0; stepIndex < stepCount && everyStepAgreed; ++stepIndex) {
		int32_t documentLength = fuiEditorGetTextLength(&editor);
		uint32_t placeRoll = TestNextRandom(&randomState);
		int32_t rawOffset = (int32_t)(placeRoll % (uint32_t)(documentLength + 1));
		int32_t offset = fuiEditorSnapToCodepointStart(&editor, rawOffset);

		bool wantsToInsert = ((placeRoll & 0x10000u) != 0u);
		if(wantsToInsert) {
			uint32_t textRoll = TestNextRandom(&randomState);
			const char *insertText = insertTexts[textRoll % fplArrayCount(insertTexts)];
			int32_t insertLength = (int32_t)strlen(insertText);

			fuiEditorSetCaretOffset(&editor, offset, false);
			everyStepAgreed = fuiEditorInsertAtCaret(&editor, insertText, insertLength);

			int32_t bytesBehindTheEdit = mirrorLength - offset;
			memmove(&mirror[offset + insertLength], &mirror[offset], (size_t)bytesBehindTheEdit);
			memcpy(&mirror[offset], insertText, (size_t)insertLength);
			mirrorLength += insertLength;

			// The caret has to come out BEHIND what was written, or typing would run backwards.
			int32_t caretAfterTheInsert = fuiEditorGetCaretOffset(&editor);
			everyStepAgreed = everyStepAgreed && (caretAfterTheInsert == (offset + insertLength));
		} else {
			uint32_t lengthRoll = TestNextRandom(&randomState);
			int32_t wantedEnd = offset + (int32_t)(lengthRoll % (uint32_t)longestErase) + 1;
			if(wantedEnd > documentLength) {
				wantedEnd = documentLength;
			}
			int32_t eraseEnd = fuiEditorSnapToCodepointStart(&editor, wantedEnd);
			if(eraseEnd <= offset) {
				continue;
			}

			fuiEditorSetSelection(&editor, offset, eraseEnd);
			everyStepAgreed = fuiEditorDeleteSelection(&editor);

			int32_t bytesBehindTheErase = mirrorLength - eraseEnd;
			memmove(&mirror[offset], &mirror[eraseEnd], (size_t)bytesBehindTheErase);
			mirrorLength -= (eraseEnd - offset);

			// And the caret collapses onto where the erase happened, whichever end it was dragged from.
			int32_t caretAfterTheErase = fuiEditorGetCaretOffset(&editor);
			everyStepAgreed = everyStepAgreed && (caretAfterTheErase == offset);
		}
	}
	CHECK(everyStepAgreed);

	int32_t finalLength = fuiEditorGetTextLength(&editor);
	CHECK_I(finalLength, mirrorLength);

	if(finalLength == mirrorLength) {
		const char *documentText = fuiEditorGetContiguousText(&editor);
		int comparison = memcmp(documentText, mirror, (size_t)mirrorLength);
		CHECK_I(comparison, 0);
	}

	// The line index, against a raw scan for line feeds. Bytes being right says nothing about lines.
	int32_t expectedLineCount = 1;
	for(int32_t byteIndex = 0; byteIndex < mirrorLength; ++byteIndex) {
		if(mirror[byteIndex] == '\n') {
			expectedLineCount += 1;
		}
	}
	CHECK_I(fuiEditorGetLineCount(&editor), expectedLineCount);

	bool everyLineStartMatches = true;
	int32_t expectedLineIndex = 0;
	int32_t expectedLineStart = 0;
	for(int32_t byteIndex = 0; byteIndex <= mirrorLength && everyLineStartMatches; ++byteIndex) {
		bool isTheEndOfALine = (byteIndex == mirrorLength) || (mirror[byteIndex] == '\n');
		if(!isTheEndOfALine) {
			continue;
		}
		int32_t reportedStart = fuiEditorGetLineStart(&editor, expectedLineIndex);
		int32_t reportedLine = fuiEditorGetLineOfOffset(&editor, expectedLineStart);
		everyLineStartMatches = (reportedStart == expectedLineStart) && (reportedLine == expectedLineIndex);
		expectedLineIndex += 1;
		expectedLineStart = byteIndex + 1;
	}
	CHECK(everyLineStartMatches);

	free(mirror);
	fuiEditorRelease(&editor);
}

static int RunSelfTest(void) {
	printf("final_ui_texteditor.h v%s self test\n", fuiEditorGetVersion());

	SelfTestEmptyDocument();
	SelfTestLineIndex();
	SelfTestInsert();
	SelfTestErase();
	SelfTestGapMovement();
	SelfTestGrowth();
	SelfTestLineEndings();
	SelfTestUtf8();
	SelfTestEncodings();
	SelfTestViewHelpers();
	SelfTestContiguousRuns();
	SelfTestCaretLine();
	SelfTestDocumentAgainstFile();
	SelfTestWidgetLayout();
	SelfTestWidgetEmptyDocument();
	SelfTestLineGeometry();
	SelfTestWords();
	SelfTestSelection();
	SelfTestScrollbarSurvivesTheBackground();
	SelfTestKeyboard();
	SelfTestWheelDoesNotFightTheCaret();
	SelfTestCopyAgainstFile();
	SelfTestLexerStatesFollowTheirLines();
	SelfTestIncrementalColouring();
	SelfTestDecorationLookup();
	SelfTestLineEndingsOfLines();
	SelfTestTyping();
	SelfTestEnterBackspaceDelete();
	SelfTestOverwriteMode();
	SelfTestCutPasteAndLines();
	SelfTestMiddleButtonPaste();
	SelfTestReadOnly();
	SelfTestEditsMoveTheCaret();
	SelfTestChangeCallback();
	SelfTestEditsAgainstAPlainBuffer();

	printf("\n%d checks, %d failed\n", g_checkTotal, g_checkFailed);
	return((g_checkFailed == 0) ? 0 : 1);
}

// ----------------------------------------------------------------------------
// > Demo
// ----------------------------------------------------------------------------

//! The monospace faces the demo carries, both embedded in final_fonts.h
typedef enum EditorDemoMonoFace {
	//! Bitstream Vera Sans Mono, the sibling of the proportional face the rest of the demos use
	EditorDemoMonoFace_VeraMono = 0,
	//! Fira Code, whose ligatures never fire here because the baker maps codepoints and does not shape
	EditorDemoMonoFace_FiraCode,
	//! How many faces there are
	EditorDemoMonoFace_Count,
} EditorDemoMonoFace;

typedef struct EditorDemoState {
	//! The document, which is all there is of the editor so far
	fuiEditor editor;
	//! Where the document was read from, or a note saying it was not found
	char sourceDescription[256];
	//! The proportional face the interface itself is drawn with
	const fuiFont *uiFont;
	//! The monospace faces, and what to call them
	const fuiFont *monoFonts[EditorDemoMonoFace_Count];
	const char *monoFontNames[EditorDemoMonoFace_Count];
	//! Which one the document is shown in
	EditorDemoMonoFace activeMonoFace;
	//! What the editor is configured with, edited in place by the toolbar and pushed on every change
	fuiEditorConfig editorConfig;
	//! What the last copy came to, since what reaches the SYSTEM clipboard is up to the platform
	char copyDescription[192];
	//! What the last change to the document was, as onChange reported it
	char editDescription[192];
	//! What the last save came to, and whether what was written read back identical
	char saveDescription[256];
	//! Whether the C lexer is installed
	bool useLexer;
	//! Whether the changed lines are handed over as decorations
	bool showChangedLines;

	//! The file exactly as it was read, which is what a changed line is changed AGAINST
	uint8_t *baselineData;
	//! How long it is
	int32_t baselineLength;
	//! Where every one of its lines begins
	int32_t *baselineLineStarts;
	//! How many there are
	int32_t baselineLineCount;
	//! One entry per line that no longer matches, sorted by line because that is what the editor searches
	fuiEditorLineDecoration *changedLines;
	//! How many of them are filled in
	int32_t changedLineCount;
	//! Which document version they were worked out from, so they are only rebuilt when something changed
	int32_t decoratedVersion;
	//! Whether the loop keeps going
	bool isRunning;
} EditorDemoState;

// ----------------------------------------------------------------------------
// > Changed lines
// ----------------------------------------------------------------------------

/*
	The other colouring layer, which carries no state at all.

	The demo keeps the file exactly as it was read and compares the document against it LINE BY LINE, at
	the same index. That is not a diff - a diff would have to find out which lines moved - and it is not
	meant to be one: the point here is that the editor takes the answer as an array and knows nothing
	about how it was arrived at. A real diff is the caller's business.

	It is rebuilt only when the document's version says something changed, because walking every line of a
	large file is exactly the cost the editor itself refuses to pay once a frame.
*/

//! Fill of a line that no longer matches the file it was read from
#define DEMO_CHANGED_LINE_BACKGROUND fuiColorRGBA(0.55f, 0.42f, 0.20f, 0.28f)

//! And of its marker in the gutter
#define DEMO_CHANGED_LINE_MARKER fuiColorRGBA(0.85f, 0.65f, 0.25f, 1.0f)

//! How many changed lines the demo is willing to mark
#define DEMO_MAX_CHANGED_LINES 4096

static void DemoReleaseBaseline(EditorDemoState *demo) {
	free(demo->baselineData);
	free(demo->baselineLineStarts);
	free(demo->changedLines);
	demo->baselineData = fpl_null;
	demo->baselineLineStarts = fpl_null;
	demo->changedLines = fpl_null;
	demo->baselineLength = 0;
	demo->baselineLineCount = 0;
	demo->changedLineCount = 0;
}

//! Keeps the file as it was read, plus where every one of its lines begins
static void DemoTakeBaseline(EditorDemoState *demo, const uint8_t *fileData, const int32_t fileLength) {
	DemoReleaseBaseline(demo);

	demo->baselineData = (uint8_t *)malloc((size_t)fileLength);
	if(demo->baselineData == fpl_null) {
		return;
	}
	memcpy(demo->baselineData, fileData, (size_t)fileLength);
	demo->baselineLength = fileLength;

	int32_t lineCount = 1;
	for(int32_t byteIndex = 0; byteIndex < fileLength; ++byteIndex) {
		if(fileData[byteIndex] == '\n') {
			lineCount += 1;
		}
	}
	demo->baselineLineStarts = (int32_t *)malloc((size_t)lineCount * sizeof(int32_t));
	if(demo->baselineLineStarts == fpl_null) {
		DemoReleaseBaseline(demo);
		return;
	}

	int32_t lineIndex = 0;
	demo->baselineLineStarts[lineIndex++] = 0;
	for(int32_t byteIndex = 0; byteIndex < fileLength; ++byteIndex) {
		if(fileData[byteIndex] == '\n' && lineIndex < lineCount) {
			demo->baselineLineStarts[lineIndex++] = byteIndex + 1;
		}
	}
	demo->baselineLineCount = lineCount;
	demo->changedLines = (fuiEditorLineDecoration *)malloc((size_t)DEMO_MAX_CHANGED_LINES * sizeof(fuiEditorLineDecoration));
}

//! One baseline line, without its ending, the way fuiEditorGetLineEnd answers for the document
static void DemoBaselineLine(const EditorDemoState *demo, const int32_t lineIndex, int32_t *outStart, int32_t *outLength) {
	*outStart = 0;
	*outLength = 0;
	if(lineIndex < 0 || lineIndex >= demo->baselineLineCount) {
		return;
	}

	int32_t lineStart = demo->baselineLineStarts[lineIndex];
	bool isTheLastLine = (lineIndex >= (demo->baselineLineCount - 1));
	int32_t lineEnd = isTheLastLine ? demo->baselineLength : (demo->baselineLineStarts[lineIndex + 1] - 1);
	if(lineEnd > lineStart && demo->baselineData[lineEnd - 1] == '\r') {
		lineEnd -= 1;
	}

	*outStart = lineStart;
	*outLength = lineEnd - lineStart;
}

static void DemoRebuildChangedLines(EditorDemoState *demo) {
	demo->changedLineCount = 0;
	demo->decoratedVersion = demo->editor.version;
	if(demo->changedLines == fpl_null || demo->baselineData == fpl_null) {
		return;
	}

	int32_t documentLineCount = fuiEditorGetLineCount(&demo->editor);
	int32_t lineCount = (documentLineCount > demo->baselineLineCount) ? documentLineCount : demo->baselineLineCount;
	for(int32_t lineIndex = 0; lineIndex < lineCount; ++lineIndex) {
		if(demo->changedLineCount >= DEMO_MAX_CHANGED_LINES) {
			break;
		}

		int32_t baselineStart = 0;
		int32_t baselineLength = 0;
		DemoBaselineLine(demo, lineIndex, &baselineStart, &baselineLength);

		bool documentHasIt = (lineIndex < documentLineCount);
		int32_t documentStart = documentHasIt ? fuiEditorGetLineStart(&demo->editor, lineIndex) : 0;
		int32_t documentLength = documentHasIt ? fuiEditorGetLineLength(&demo->editor, lineIndex) : -1;

		bool isTheSame = (documentLength == baselineLength);
		if(isTheSame) {
			for(int32_t byteIndex = 0; byteIndex < baselineLength; ++byteIndex) {
				char documentByte = fuiEditorGetByte(&demo->editor, documentStart + byteIndex);
				char baselineByte = (char)demo->baselineData[baselineStart + byteIndex];
				if(documentByte != baselineByte) {
					isTheSame = false;
					break;
				}
			}
		}
		if(isTheSame) {
			continue;
		}

		fuiEditorLineDecoration *decoration = &demo->changedLines[demo->changedLineCount];
		decoration->line = lineIndex;
		decoration->background = DEMO_CHANGED_LINE_BACKGROUND;
		decoration->gutterMarker = DEMO_CHANGED_LINE_MARKER;
		demo->changedLineCount += 1;
	}
}

//! Hands the array over, or takes it away again. The editor only ever holds the pointer
static void DemoApplyDecorations(EditorDemoState *demo) {
	if(!demo->showChangedLines) {
		fuiEditorSetDecorations(&demo->editor, fpl_null);
		return;
	}
	if(demo->decoratedVersion != demo->editor.version) {
		DemoRebuildChangedLines(demo);
	}

	fuiEditorDecorations decorations = fplZeroInit;
	decorations.lines = demo->changedLines;
	decorations.lineCount = demo->changedLineCount;
	fuiEditorSetDecorations(&demo->editor, &decorations);
}

/*
	Reads a whole file into memory.

	The demo fills its document from a file rather than a literal, because a line index is only worth
	anything on something the size of a real source file - and the largest one to hand is the library
	this add-on sits next to.
*/
static bool DemoReadWholeFile(const char *filePath, uint8_t **outData, int32_t *outLength) {
	*outData = fpl_null;
	*outLength = 0;

	fplFileHandle fileHandle;
	if(!fplFileOpenBinary(filePath, &fileHandle)) {
		return(false);
	}

	uint32_t fileSize = fplFileGetSizeFromHandle32(&fileHandle);
	if(fileSize == 0 || fileSize > (uint32_t)DEMO_MAX_FILE_BYTES) {
		fplFileClose(&fileHandle);
		return(false);
	}

	uint8_t *fileData = (uint8_t *)malloc(fileSize);
	if(fileData == fpl_null) {
		fplFileClose(&fileHandle);
		return(false);
	}

	uint32_t readCount = fplFileReadBlock32(&fileHandle, fileSize, fileData, fileSize);
	fplFileClose(&fileHandle);
	if(readCount != fileSize) {
		free(fileData);
		return(false);
	}

	*outData = fileData;
	*outLength = (int32_t)fileSize;
	return(true);
}

//! Defined below, beside the saving it keeps the description for
static void DemoOnEditorChange(fuiEditor *editor, const fuiEditorChange *change, void *userData);

static void DemoInit(EditorDemoState *demo) {
	fplClearStruct(demo);
	demo->isRunning = true;
	fuiEditorInit(&demo->editor, fpl_null);

	// Started from the defaults and then edited by the toolbar, which is what a caller who wants to change
	// one thing does: take the defaults, change the one field, hand the whole thing back.
	demo->editorConfig = fuiEditorDefaultConfig();
	demo->editorConfig.callbacks.onChange = DemoOnEditorChange;
	demo->editorConfig.callbacks.userData = demo;
	fuiEditorSetConfig(&demo->editor, &demo->editorConfig);

	DemoBuildCStyleTable();
	demo->useLexer = true;
	demo->decoratedVersion = -1;
	fplCopyString("Click, drag, double click, arrows, Ctrl+A, Ctrl+C", demo->copyDescription, fplArrayCount(demo->copyDescription));
	fplCopyString("Type into it - there is no undo yet, so what is typed cannot be taken back", demo->editDescription, fplArrayCount(demo->editDescription));
	fplCopyString("Not saved yet", demo->saveDescription, fplArrayCount(demo->saveDescription));

	// Tried from the working directory and from one level up, so running it out of the build folder and
	// out of the repository root both find something.
	const char *candidatePaths[] = {
		DEMO_SOURCE_FILE_PATH,
		"../" DEMO_SOURCE_FILE_PATH,
		"../../" DEMO_SOURCE_FILE_PATH,
		"../../../" DEMO_SOURCE_FILE_PATH,
		"../../../../" DEMO_SOURCE_FILE_PATH,
	};

	size_t candidateIndex = 0;
	while(candidateIndex < fplArrayCount(candidatePaths)) {
		const char *candidatePath = candidatePaths[candidateIndex];
		uint8_t *fileData = fpl_null;
		int32_t fileLength = 0;
		if(DemoReadWholeFile(candidatePath, &fileData, &fileLength)) {
			fuiEditorEncoding utf8Encoding = fuiEditorEncodingUtf8();
			fuiEditorLoadFromMemory(&demo->editor, fileData, fileLength, &utf8Encoding);
			DemoTakeBaseline(demo, fileData, fileLength);
			free(fileData);
			fplStringFormat(demo->sourceDescription, fplArrayCount(demo->sourceDescription), "Loaded %s (%d bytes)", candidatePath, (int)fileLength);
			return;
		}
		candidateIndex += 1;
	}

	fuiEditorSetText(&demo->editor, "Nothing was loaded.\n\nRun the demo from the repository root so it finds " DEMO_SOURCE_FILE_PATH ".\n", 0);
	fplCopyString("Not found: " DEMO_SOURCE_FILE_PATH, demo->sourceDescription, fplArrayCount(demo->sourceDescription));
}

static void DemoRelease(EditorDemoState *demo) {
	fuiEditorRelease(&demo->editor);
	DemoReleaseBaseline(demo);
}

//! Installs the C lexer, or takes it away again
static void DemoApplyLexer(EditorDemoState *demo) {
	if(!demo->useLexer) {
		fuiEditorSetLexer(&demo->editor, fpl_null);
		return;
	}

	fuiEditorLexer lexer = fplZeroInit;
	lexer.lexLine = DemoLexCLine;
	lexer.styles = g_demoCStyleTable;
	lexer.styleCount = (int32_t)DemoCStyle_Count;
	fuiEditorSetLexer(&demo->editor, &lexer);
}

/*
	Changes a line near the TOP of the document, which is the case the incremental colouring exists for.

	Scroll to the end of a large file, press this, and everything has to stay where it is: the watermark
	drops to line three, the next build re-colours from there, and the state it arrives at for line four
	is the one already stored - so it stops after two lines rather than walking fourteen thousand.
*/
static void DemoChangeAnEarlyLine(EditorDemoState *demo) {
	const int32_t theLineToChange = 2;
	int32_t lineCount = fuiEditorGetLineCount(&demo->editor);
	if(lineCount <= theLineToChange) {
		return;
	}

	// Written at the END of the line so nothing about its indentation moves, and with no line feed in it,
	// so the lines below keep their numbers and the positional comparison stays meaningful.
	//
	// And with no comment characters in it either. A closing block comment marker here would close the
	// block comment that final_ui.h's own header sits inside of - correct C, and a thoroughly confusing
	// thing for a demo to do to itself. This very comment had to be rewritten for the same reason.
	const char *marker = "   <-- changed by the demo";
	int32_t lineEnd = fuiEditorGetLineEnd(&demo->editor, theLineToChange);
	(void)fuiEditorInsert(&demo->editor, lineEnd, marker, 0);
	demo->showChangedLines = true;
}

/*
	What onChange is for, shown by using it.

	The editor says where the change was, how many bytes went and came and how many lines appeared - so a
	caller that keeps something ALONGSIDE the document, a baseline or an outline or a diff, can bring it up
	to date without walking the document to find out what happened. The demo only writes a sentence with it,
	but a sentence is enough to show that everything which changes the document arrives here: a key, a
	paste, the middle mouse button and the toolbar's own fuiEditorInsert alike.
*/
static void DemoOnEditorChange(fuiEditor *editor, const fuiEditorChange *change, void *userData) {
	EditorDemoState *demo = (EditorDemoState *)userData;
	int32_t caretLine = fuiEditorGetCaretLine(editor);
	fplStringFormat(demo->editDescription, fplArrayCount(demo->editDescription), "Line %d: -%d +%d bytes, %+d lines, caret now on line %d", (int)(change->firstLine + 1), (int)change->removedBytes, (int)change->insertedBytes, (int)change->lineCountDelta, (int)(caretLine + 1));
}

static bool DemoWriteWholeFile(const char *filePath, const void *data, const int32_t dataLength) {
	fplFileHandle fileHandle;
	if(!fplFileCreateBinary(filePath, &fileHandle)) {
		return(false);
	}
	uint32_t writtenCount = fplFileWriteBlock32(&fileHandle, data, (uint32_t)dataLength);
	fplFileClose(&fileHandle);
	return(writtenCount == (uint32_t)dataLength);
}

/*
	The acceptance criterion of this iteration, as a button.

	"It saved" and "what came out is what was in there" are two different claims, and only the second one is
	worth making - so what was written is read straight back in and compared byte for byte. It goes to a
	file of its OWN rather than back over the source: the document on screen is this repository's own
	final_ui.h, and a demo that overwrites the file it is showing is a demo nobody runs twice.
*/
static void DemoSaveAndVerify(EditorDemoState *demo) {
	int32_t documentLength = fuiEditorGetTextLength(&demo->editor);
	if(documentLength <= 0) {
		fplCopyString("Nothing to save - the document is empty", demo->saveDescription, fplArrayCount(demo->saveDescription));
		return;
	}

	const char *documentText = fuiEditorGetContiguousText(&demo->editor);
	if(!DemoWriteWholeFile(DEMO_SAVE_FILE_PATH, documentText, documentLength)) {
		fplStringFormat(demo->saveDescription, fplArrayCount(demo->saveDescription), "Could not write %s", DEMO_SAVE_FILE_PATH);
		return;
	}

	uint8_t *savedData = fpl_null;
	int32_t savedLength = 0;
	if(!DemoReadWholeFile(DEMO_SAVE_FILE_PATH, &savedData, &savedLength)) {
		fplStringFormat(demo->saveDescription, fplArrayCount(demo->saveDescription), "Wrote %s but could not read it back", DEMO_SAVE_FILE_PATH);
		return;
	}

	bool lengthsMatch = (savedLength == documentLength);
	bool bytesMatch = false;
	if(lengthsMatch) {
		int comparison = memcmp(savedData, documentText, (size_t)documentLength);
		bytesMatch = (comparison == 0);
	}
	free(savedData);

	if(!lengthsMatch || !bytesMatch) {
		fplStringFormat(demo->saveDescription, fplArrayCount(demo->saveDescription), "MISMATCH: wrote %d bytes, read back %d", (int)documentLength, (int)savedLength);
		return;
	}

	fuiEditorClearModified(&demo->editor);
	fplStringFormat(demo->saveDescription, fplArrayCount(demo->saveDescription), "Saved %d bytes to %s and read them back identical", (int)documentLength, DEMO_SAVE_FILE_PATH);
}

/*
	The clipboard hook the demo installs INSTEAD of handing fuiFplSetClipboardText straight over.

	fplSetClipboardText copies into a fixed buffer of FPL_MAX_BUFFER_LENGTH bytes through fplCopyString,
	and fplCopyString writes NOTHING AT ALL when the text does not fit - it answers zero and returns. The
	selection owner is then taken anyway and serves those zero bytes, so what comes out the other end is
	not a shortened clipboard but an EMPTY one, and whatever was in it beforehand is gone with it. The
	call even reports success, because taking the ownership did work.

	So anything that does not fit is refused HERE rather than handed over. Every path through the editor
	goes through this one hook - the Copy button and ctrl+c alike - which is exactly what the hook is for.
*/
static bool DemoSetClipboardText(void *userData, const char *text) {
	EditorDemoState *demo = (EditorDemoState *)userData;
	size_t textLength = fplGetStringLength(text);

	// One less than the buffer, because the terminator has to fit in it as well.
	const size_t platformClipboardLimit = FPL_MAX_BUFFER_LENGTH - 1;
	if(textLength > platformClipboardLimit) {
		fplStringFormat(demo->copyDescription, fplArrayCount(demo->copyDescription), "%d bytes exceeds the platform clipboard (%d) - not copied", (int)textLength, (int)platformClipboardLimit);
		return(false);
	}

	bool didSet = fplSetClipboardText(text);
	if(didSet) {
		fplStringFormat(demo->copyDescription, fplArrayCount(demo->copyDescription), "Copied %d bytes", (int)textLength);
	} else {
		fplStringFormat(demo->copyDescription, fplArrayCount(demo->copyDescription), "The platform refused %d bytes", (int)textLength);
	}
	return(didSet);
}

//! Hands the selection to that hook, which is the same thing ctrl+c inside the editor does
static void DemoCopySelection(fuiContext *ui, EditorDemoState *demo) {
	char *noBuffer = fpl_null;
	const int32_t noCapacity = 0;
	int32_t selectionLength = fuiEditorCopySelection(&demo->editor, noBuffer, noCapacity);
	if(selectionLength <= 0) {
		fplCopyString("Nothing selected", demo->copyDescription, fplArrayCount(demo->copyDescription));
		return;
	}

	int32_t bufferLength = selectionLength + 1;
	char *copiedText = (char *)malloc((size_t)bufferLength);
	if(copiedText == fpl_null) {
		fplCopyString("Out of memory", demo->copyDescription, fplArrayCount(demo->copyDescription));
		return;
	}
	(void)fuiEditorCopySelection(&demo->editor, copiedText, bufferLength);
	(void)fuiSetClipboardText(ui, copiedText);
	free(copiedText);
}

/*
	What there is to show at this iteration.

	The editor is read only so far - it draws, it scrolls, the caret moves and the selection can be copied
	out, and nothing types into it yet. So the panel is a toolbar over one fuiTextEditor: the toolbar is
	there to prove that the configuration really is the whole of what a caller decides about an editor, and
	the editor below it is there to be read through final_ui.h, which is the largest file to hand and the
	one this add-on has to hold up against.
*/
static void BuildUserInterface(fuiContext *ui, EditorDemoState *demo) {
	const float panelPadding = 16.0f;
	const float rowHeight = 30.0f;
	const float rowSpacing = 6.0f;
	const float buttonWidth = 96.0f;
	const float wideButtonWidth = 340.0f;
	const float toggleWidth = 150.0f;
	const int32_t bigJumpLines = 1000;
	const int32_t smallestTabSize = 1;
	const int32_t largestTabSize = 16;

	const fuiDrawData *drawData = fuiGetDrawData(ui);
	float windowWidth = (float)drawData->windowSize.x;
	float windowHeight = (float)drawData->windowSize.y;
	float panelWidth = windowWidth - panelPadding * 2.0f;
	float panelHeight = windowHeight - panelPadding * 2.0f;

	if(!fuiBeginPanel(ui, "Document", fuiDock_None, panelPadding, panelPadding, panelWidth, panelHeight)) {
		return;
	}

	fuiRect sourceRow = fuiLayoutSlot(ui, rowHeight);
	fuiLabel(ui, sourceRow, demo->sourceDescription);

	bool configurationChanged = false;
	int32_t documentLineCount = fuiEditorGetLineCount(&demo->editor);
	int32_t caretLine = fuiEditorGetCaretLine(&demo->editor);

	// One row of buttons: which face the code is set in, and where in the document to look.
	fuiRect navigationRow = fuiLayoutSlot(ui, rowHeight);
	fuiBeginStackAt(ui, "navigation", fuiAxis_Horizontal, navigationRow, rowSpacing);
	{
		const char *activeMonoName = demo->monoFontNames[demo->activeMonoFace];
		char faceButtonLabel[128];
		fplStringFormat(faceButtonLabel, fplArrayCount(faceButtonLabel), "Monospace face: %s", activeMonoName);

		fuiRect faceButtonRect = fuiLayoutSlot(ui, wideButtonWidth);
		if(fuiButton(ui, faceButtonRect, faceButtonLabel)) {
			int32_t nextFace = ((int32_t)demo->activeMonoFace + 1) % (int32_t)EditorDemoMonoFace_Count;
			demo->activeMonoFace = (EditorDemoMonoFace)nextFace;
		}

		fuiRect topButtonRect = fuiLayoutSlot(ui, buttonWidth);
		if(fuiButton(ui, topButtonRect, "Top")) {
			const int32_t firstLine = 0;
			fuiEditorSetCaretLine(&demo->editor, firstLine);
			fuiEditorScrollToLine(&demo->editor, firstLine);
		}

		fuiRect backButtonRect = fuiLayoutSlot(ui, buttonWidth);
		if(fuiButtonRepeat(ui, backButtonRect, "-1000")) {
			int32_t wantedLine = caretLine - bigJumpLines;
			fuiEditorSetCaretLine(&demo->editor, wantedLine);
			fuiEditorScrollToLine(&demo->editor, wantedLine);
		}

		fuiRect forwardButtonRect = fuiLayoutSlot(ui, buttonWidth);
		if(fuiButtonRepeat(ui, forwardButtonRect, "+1000")) {
			int32_t wantedLine = caretLine + bigJumpLines;
			fuiEditorSetCaretLine(&demo->editor, wantedLine);
			fuiEditorScrollToLine(&demo->editor, wantedLine);
		}

		fuiRect bottomButtonRect = fuiLayoutSlot(ui, buttonWidth);
		if(fuiButton(ui, bottomButtonRect, "Bottom")) {
			int32_t lastLine = documentLineCount - 1;
			fuiEditorSetCaretLine(&demo->editor, lastLine);
			fuiEditorScrollToLine(&demo->editor, lastLine);
		}
	}
	fuiEndStack(ui);

	// And one row of what the configuration can be asked for. Every one of these writes into the caller's
	// own fuiEditorConfig and hands the whole struct back, which is the only way an editor is configured.
	fuiRect toggleRow = fuiLayoutSlot(ui, rowHeight);
	fuiBeginStackAt(ui, "toggles", fuiAxis_Horizontal, toggleRow, rowSpacing);
	{
		fuiRect lineNumbersRect = fuiLayoutSlot(ui, toggleWidth);
		if(fuiCheckbox(ui, lineNumbersRect, "Line numbers", &demo->editorConfig.toggles.showLineNumbers)) {
			configurationChanged = true;
		}

		fuiRect statusBarRect = fuiLayoutSlot(ui, toggleWidth);
		if(fuiCheckbox(ui, statusBarRect, "Status bar", &demo->editorConfig.toggles.showStatusBar)) {
			configurationChanged = true;
		}

		fuiRect currentLineRect = fuiLayoutSlot(ui, toggleWidth);
		if(fuiCheckbox(ui, currentLineRect, "Current line", &demo->editorConfig.toggles.highlightCurrentLine)) {
			configurationChanged = true;
		}

		int32_t shownTabSize = (demo->editorConfig.metrics.tabSize > 0) ? demo->editorConfig.metrics.tabSize : 4;
		char tabButtonLabel[64];
		fplStringFormat(tabButtonLabel, fplArrayCount(tabButtonLabel), "Tab width: %d", (int)shownTabSize);

		fuiRect tabDownRect = fuiLayoutSlot(ui, buttonWidth);
		if(fuiButtonRepeat(ui, tabDownRect, "Tab -")) {
			int32_t wantedTabSize = shownTabSize - 1;
			if(wantedTabSize >= smallestTabSize) {
				demo->editorConfig.metrics.tabSize = wantedTabSize;
				configurationChanged = true;
			}
		}

		fuiRect tabLabelRect = fuiLayoutSlot(ui, toggleWidth);
		fuiLabel(ui, tabLabelRect, tabButtonLabel);

		fuiRect tabUpRect = fuiLayoutSlot(ui, buttonWidth);
		if(fuiButtonRepeat(ui, tabUpRect, "Tab +")) {
			int32_t wantedTabSize = shownTabSize + 1;
			if(wantedTabSize <= largestTabSize) {
				demo->editorConfig.metrics.tabSize = wantedTabSize;
				configurationChanged = true;
			}
		}
	}
	fuiEndStack(ui);

	fuiRect selectionRow = fuiLayoutSlot(ui, rowHeight);
	fuiBeginStackAt(ui, "selection", fuiAxis_Horizontal, selectionRow, rowSpacing);
	{
		fuiRect interactiveRect = fuiLayoutSlot(ui, toggleWidth);
		if(fuiCheckbox(ui, interactiveRect, "Interactive", &demo->editorConfig.toggles.isInteractive)) {
			configurationChanged = true;
		}

		fuiRect selectAllRect = fuiLayoutSlot(ui, buttonWidth + buttonWidth / 2.0f);
		if(fuiButton(ui, selectAllRect, "Select all")) {
			fuiEditorSelectAll(&demo->editor);
		}

		fuiRect copyRect = fuiLayoutSlot(ui, buttonWidth);
		if(fuiButton(ui, copyRect, "Copy")) {
			DemoCopySelection(ui, demo);
		}

		fuiRect noteRect = fuiLayoutRemaining(ui);
		fuiLabel(ui, noteRect, demo->copyDescription);
	}
	fuiEndStack(ui);

	// What this iteration added: the document can be written to now. Read only is a CONFIGURATION toggle
	// like every other one here, and it is the one gate every writing branch in the editor goes through.
	fuiRect editingRow = fuiLayoutSlot(ui, rowHeight);
	fuiBeginStackAt(ui, "editing", fuiAxis_Horizontal, editingRow, rowSpacing);
	{
		fuiRect readOnlyRect = fuiLayoutSlot(ui, toggleWidth);
		if(fuiCheckbox(ui, readOnlyRect, "Read only", &demo->editorConfig.toggles.isReadOnly)) {
			configurationChanged = true;
		}

		fuiRect saveRect = fuiLayoutSlot(ui, wideButtonWidth / 2.0f);
		if(fuiButton(ui, saveRect, "Save & verify")) {
			DemoSaveAndVerify(demo);
		}

		fuiRect saveNoteRect = fuiLayoutRemaining(ui);
		fuiLabel(ui, saveNoteRect, demo->saveDescription);
	}
	fuiEndStack(ui);

	fuiRect changeRow = fuiLayoutSlot(ui, rowHeight);
	fuiLabel(ui, changeRow, demo->editDescription);

	fuiRect colouringRow = fuiLayoutSlot(ui, rowHeight);
	fuiBeginStackAt(ui, "colouring", fuiAxis_Horizontal, colouringRow, rowSpacing);
	{
		fuiRect lexerRect = fuiLayoutSlot(ui, toggleWidth);
		if(fuiCheckbox(ui, lexerRect, "C lexer", &demo->useLexer)) {
			DemoApplyLexer(demo);
		}

		fuiRect whitespaceRect = fuiLayoutSlot(ui, toggleWidth);
		if(fuiCheckbox(ui, whitespaceRect, "Whitespace", &demo->editorConfig.toggles.showWhitespace)) {
			configurationChanged = true;
		}

		fuiRect endingsRect = fuiLayoutSlot(ui, toggleWidth);
		if(fuiCheckbox(ui, endingsRect, "Line endings", &demo->editorConfig.toggles.showLineEndings)) {
			configurationChanged = true;
		}

		fuiRect changedRect = fuiLayoutSlot(ui, toggleWidth + toggleWidth / 4.0f);
		(void)fuiCheckbox(ui, changedRect, "Changed lines", &demo->showChangedLines);

		fuiRect editRect = fuiLayoutSlot(ui, wideButtonWidth / 2.0f);
		if(fuiButton(ui, editRect, "Change line 3")) {
			DemoChangeAnEarlyLine(demo);
		}

		fuiRect countRect = fuiLayoutRemaining(ui);
		char changedText[96];
		fplStringFormat(changedText, fplArrayCount(changedText), "%d changed", (int)demo->changedLineCount);
		fuiLabel(ui, countRect, changedText);
	}
	fuiEndStack(ui);

	if(configurationChanged) {
		fuiEditorSetConfig(&demo->editor, &demo->editorConfig);
	}
	DemoApplyDecorations(demo);

	fuiRect separatorRow = fuiLayoutSlot(ui, rowHeight);
	fuiSeparator(ui, separatorRow);

	// final_ui.h holds ONE font per context, so a widget that wants a different face swaps it in for the
	// length of its own build and puts the old one back. Code wants a monospace face; the panel around it
	// does not.
	fuiRect editorRect = fuiLayoutRemaining(ui);
	const fuiFont *monoFont = demo->monoFonts[demo->activeMonoFace];
	fuiSetFont(ui, monoFont);
	fuiEditorAction action = fuiTextEditor(ui, editorRect, "source", &demo->editor);
	fuiSetFont(ui, demo->uiFont);
	(void)action;

	fuiEndPanel(ui);
}

// ----------------------------------------------------------------------------

static void DemoReleaseFaces(fuiStbttFont *bakedFonts, uint32_t *atlasTextures, const int bakedFaceCount) {
	for(int faceIndex = 0; faceIndex < bakedFaceCount; ++faceIndex) {
		if(atlasTextures[faceIndex] != 0) {
			fuiGL1DeleteTexture(atlasTextures[faceIndex]);
		}
		fuiStbttFontRelease(&bakedFonts[faceIndex]);
	}
}

int main(int argc, char **argv) {
	for(int argumentIndex = 1; argumentIndex < argc; ++argumentIndex) {
		bool isTheSelfTestFlag = (strcmp(argv[argumentIndex], "--selftest") == 0);
		if(isTheSelfTestFlag) {
			// No window and no OpenGL: the document does not need either, and a test that opens a window
			// cannot run where a test is most wanted.
			if(!fplPlatformInit(fplInitFlags_None, fpl_null)) {
				fprintf(stderr, "failed to initialize the platform\n");
				return 1;
			}
			int selfTestResult = RunSelfTest();
			fplPlatformRelease();
			return selfTestResult;
		}
	}

	fplSettings settings = fplZeroInit;
	fplSetDefaultSettings(&settings);
	fplCopyString(DEMO_WINDOW_TITLE, settings.window.title, fplArrayCount(settings.window.title));
	settings.window.windowSize.width = DEMO_WINDOW_WIDTH;
	settings.window.windowSize.height = DEMO_WINDOW_HEIGHT;
	settings.video.backend = fplVideoBackendType_OpenGL;
	settings.video.graphics.opengl.compatibilityFlags = fplOpenGLCompatibilityFlags_Legacy;
	settings.video.isVSync = true;

	if(!fplPlatformInit(fplInitFlags_Window | fplInitFlags_Video, &settings)) {
		fprintf(stderr, "failed to initialize the platform\n");
		return 1;
	}
	if(!fglLoadOpenGL(true)) {
		fprintf(stderr, "failed to load OpenGL\n");
		fplPlatformRelease();
		return 1;
	}

	// Three faces: the proportional one the interface is drawn with, and the two monospace ones the
	// document can be shown in. All three are embedded in final_fonts.h, so there is still nothing to
	// find at runtime.
	fuiStbttBakeSettings bakeSettings = fuiStbttDefaultBakeSettings();
	bakeSettings.pixelHeight = DEMO_FONT_PIXEL_HEIGHT;
	bakeSettings.atlasWidth = DEMO_FONT_ATLAS_SIDE;
	bakeSettings.atlasHeight = DEMO_FONT_ATLAS_SIDE;

	const uint8_t *faceData[DEMO_FACE_COUNT];
	faceData[DEMO_FACE_UI] = ptr_fontBitstreamVeraRegular;
	faceData[DEMO_FACE_VERA_MONO] = ptr_fontBitstreamVeraMonoRegular;
	faceData[DEMO_FACE_FIRA_CODE] = ptr_fontFiraCodeRegular;

	fuiStbttFont bakedFonts[DEMO_FACE_COUNT];
	uint32_t atlasTextures[DEMO_FACE_COUNT];
	fuiFont fonts[DEMO_FACE_COUNT];
	int bakedFaceCount = 0;
	bool everyFaceIsReady = true;
	for(int faceIndex = 0; faceIndex < DEMO_FACE_COUNT; ++faceIndex) {
		atlasTextures[faceIndex] = 0;
		if(!fuiStbttFontBake(&bakedFonts[faceIndex], faceData[faceIndex], &bakeSettings)) {
			fprintf(stderr, "failed to bake font %d\n", faceIndex);
			everyFaceIsReady = false;
			break;
		}
		bakedFaceCount = faceIndex + 1;
		const fuiStbttFont *justBaked = &bakedFonts[faceIndex];
		if(!fuiGL1UploadFontAtlas(justBaked->atlasPixels, justBaked->atlasWidth, justBaked->atlasHeight, &atlasTextures[faceIndex])) {
			fprintf(stderr, "failed to upload atlas %d\n", faceIndex);
			everyFaceIsReady = false;
			break;
		}
		fonts[faceIndex] = fuiStbttFontToFuiFont(&bakedFonts[faceIndex], (fuiTextureId)atlasTextures[faceIndex]);
	}
	if(!everyFaceIsReady) {
		DemoReleaseFaces(bakedFonts, atlasTextures, bakedFaceCount);
		fglUnloadOpenGL();
		fplPlatformRelease();
		return 1;
	}

	fuiContext ui;
	if(!fuiInit(&ui, &fonts[DEMO_FACE_UI], fpl_null)) {
		fprintf(stderr, "failed to initialize the user interface\n");
		DemoReleaseFaces(bakedFonts, atlasTextures, bakedFaceCount);
		fglUnloadOpenGL();
		fplPlatformRelease();
		return 1;
	}

	EditorDemoState demo;
	DemoInit(&demo);
	demo.uiFont = &fonts[DEMO_FACE_UI];
	demo.monoFonts[EditorDemoMonoFace_VeraMono] = &fonts[DEMO_FACE_VERA_MONO];
	demo.monoFonts[EditorDemoMonoFace_FiraCode] = &fonts[DEMO_FACE_FIRA_CODE];
	demo.monoFontNames[EditorDemoMonoFace_VeraMono] = "Bitstream Vera Sans Mono";
	demo.monoFontNames[EditorDemoMonoFace_FiraCode] = "Fira Code";
	DemoApplyLexer(&demo);

	// The demo's own clipboard hook rather than fuiFplSetClipboardText, so that every copy - the button
	// and ctrl+c alike - goes through the size check. It needs the demo state, so it is installed here
	// rather than before the state exists.
	fuiPlatform platform = fplZeroInit;
	platform.getClipboardText = fuiFplGetClipboardText;
	platform.setClipboardText = DemoSetClipboardText;
	platform.userData = &demo;
	fuiSetPlatform(&ui, &platform);

	fuiFplInput bridge;
	fuiFplInputInit(&bridge);

	while(demo.isRunning && fplWindowUpdate()) {
		fuiFplInputPumpEvents(&bridge);
		fuiFplInputBuild(&bridge);

		fuiBeginFrame(&ui, &bridge.input, fuiPass_Both);
		BuildUserInterface(&ui, &demo);
		fuiEndFrame(&ui);

		glClearColor(0.09f, 0.09f, 0.11f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		fuiGL1Render(fuiGetDrawData(&ui));

		fplVideoFlip();
	}

	DemoRelease(&demo);
	fuiRelease(&ui);
	DemoReleaseFaces(bakedFonts, atlasTextures, bakedFaceCount);
	fglUnloadOpenGL();
	fplPlatformRelease();
	return 0;
}
