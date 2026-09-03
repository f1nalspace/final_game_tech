/*
Name:
	FUI_Editor

Description:
	An interactive demo for final_ui_texteditor.h, on FPL and legacy OpenGL.

	The editor add-on is built over several iterations, and this demo grows with it. What is in right
	now is the FOUNDATION: the document itself - a gap buffer, a line index that is a split array of its
	own, and the encoding seam - with no widget on top of it yet. So the window reports what the document
	knows about itself rather than drawing it, and the interesting part of this file is the self test.

	Run it with --selftest to get the headless one: no window, no OpenGL, one exit code. That is the mode
	the document is developed against, because a gap buffer is exactly the kind of thing that looks right
	on screen while being wrong across the hole.

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

//! Largest file the demo is willing to read into memory
#define DEMO_MAX_FILE_BYTES (64 * 1024 * 1024)

//! How many document lines the placeholder preview reads back, until the real widget replaces it
#define DEMO_PREVIEW_LINES 400

//! How much text those lines are allowed to add up to
#define DEMO_PREVIEW_BYTES (64 * 1024)

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
	//! Whether the loop keeps going
	bool isRunning;
} EditorDemoState;

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

static void DemoInit(EditorDemoState *demo) {
	fplClearStruct(demo);
	demo->isRunning = true;
	fuiEditorInit(&demo->editor, fpl_null);

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
}

/*
	What there is to show at this iteration.

	There is no editor widget yet, so the panel reports what the document knows about itself and shows
	its first lines through final_ui.h's own read-only text view. Every line of this goes away in the
	next iteration, when fuiTextEditor draws the document itself.
*/
static void BuildUserInterface(fuiContext *ui, EditorDemoState *demo) {
	const float panelPadding = 16.0f;
	const float rowHeight = 30.0f;

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

	char statisticsText[256];
	int32_t documentLength = fuiEditorGetTextLength(&demo->editor);
	int32_t documentLineCount = fuiEditorGetLineCount(&demo->editor);
	fuiEditorEol documentEol = fuiEditorGetEol(&demo->editor);
	const char *eolName = fuiEditorEolGetName(documentEol);
	fplStringFormat(statisticsText, fplArrayCount(statisticsText), "%d bytes, %d lines, %s, %s", (int)documentLength, (int)documentLineCount, demo->editor.encoding.name, eolName);

	fuiRect statisticsRow = fuiLayoutSlot(ui, rowHeight);
	fuiLabel(ui, statisticsRow, statisticsText);

	fuiRect versionRow = fuiLayoutSlot(ui, rowHeight);
	char versionText[128];
	fplStringFormat(versionText, fplArrayCount(versionText), "final_ui.h v%s, final_ui_texteditor.h v%s", fuiGetVersion(), fuiEditorGetVersion());
	fuiLabel(ui, versionRow, versionText);

	fuiRect separatorRow = fuiLayoutSlot(ui, rowHeight);
	fuiSeparator(ui, separatorRow);

	// Switching the face is the same move the editor widget will make in the next iteration, so wiring
	// it up now is the first test of that path rather than a demo convenience.
	const char *activeMonoName = demo->monoFontNames[demo->activeMonoFace];
	char faceButtonLabel[128];
	fplStringFormat(faceButtonLabel, fplArrayCount(faceButtonLabel), "Monospace face: %s", activeMonoName);

	fuiRect faceButtonRow = fuiLayoutSlot(ui, rowHeight);
	if(fuiButton(ui, faceButtonRow, faceButtonLabel)) {
		int32_t nextFace = ((int32_t)demo->activeMonoFace + 1) % (int32_t)EditorDemoMonoFace_Count;
		demo->activeMonoFace = (EditorDemoMonoFace)nextFace;
	}

	fuiRect noteRow = fuiLayoutSlot(ui, rowHeight);
	fuiLabel(ui, noteRow, "No widget yet - this is the document, read back line by line:");

	// Read the lines back out one at a time, which is the only way to prove from the outside that the
	// line index and the hole agree with each other. Static rather than automatic because it is far too
	// big for a stack frame, and there is exactly one of these panels.
	static char previewText[DEMO_PREVIEW_BYTES];
	int32_t previewLength = 0;
	int32_t previewLine = 0;
	int32_t linesToShow = (documentLineCount < DEMO_PREVIEW_LINES) ? documentLineCount : DEMO_PREVIEW_LINES;
	while(previewLine < linesToShow) {
		char lineText[256];
		(void)fuiEditorCopyLine(&demo->editor, previewLine, lineText, (int32_t)sizeof(lineText));
		int32_t roomLeft = (int32_t)sizeof(previewText) - previewLength;
		int written = snprintf(&previewText[previewLength], (size_t)roomLeft, "%5d  %s\n", (int)(previewLine + 1), lineText);
		if(written <= 0 || written >= roomLeft) {
			break;
		}
		previewLength += written;
		previewLine += 1;
	}
	previewText[previewLength] = '\0';

	fuiRect previewRect = fuiLayoutRemaining(ui);
	const bool isMultiline = true;
	const bool wrapsLongLines = false;

	// final_ui.h holds ONE font per context, so a widget that wants a different face swaps it for the
	// length of its own build and puts the old one back. Code wants a monospace face; the panel around
	// it does not.
	const fuiFont *monoFont = demo->monoFonts[demo->activeMonoFace];
	fuiSetFont(ui, monoFont);
	fuiTextView(ui, previewRect, "preview", previewText, isMultiline, wrapsLongLines);
	fuiSetFont(ui, demo->uiFont);

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

	fuiPlatform platform = fplZeroInit;
	platform.getClipboardText = fuiFplGetClipboardText;
	platform.setClipboardText = fuiFplSetClipboardText;
	fuiSetPlatform(&ui, &platform);

	EditorDemoState demo;
	DemoInit(&demo);
	demo.uiFont = &fonts[DEMO_FACE_UI];
	demo.monoFonts[EditorDemoMonoFace_VeraMono] = &fonts[DEMO_FACE_VERA_MONO];
	demo.monoFonts[EditorDemoMonoFace_FiraCode] = &fonts[DEMO_FACE_FIRA_CODE];
	demo.monoFontNames[EditorDemoMonoFace_VeraMono] = "Bitstream Vera Sans Mono";
	demo.monoFontNames[EditorDemoMonoFace_FiraCode] = "Fira Code";

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
