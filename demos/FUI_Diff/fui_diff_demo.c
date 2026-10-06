/*
Name:
	FUI_Diff

Description:
	A diff viewer for final_ui_texteditor.h, on FPL and legacy OpenGL.

	It shows the same difference in two ways, and the point of the demo is that BOTH of them are the plain
	editor widget with nothing but decorations hung on it - no diff mode, no second widget, no special case
	anywhere in the add-on.

	  Unified   One editor holding both texts woven together. A line that went is washed red, a line that
	            came is washed green, and there are no plus or minus signs anywhere - what a line is, is
	            said by its colour and by the two columns of line numbers beside it, the way Gittyup says
	            it.

	  Side by side   Two editors with EXACTLY as many rows as each other. Where one side has a line the
	            other has not, the other side gets a filler row that stands for a line that is not there -
	            drawn in a style of its own, and carrying no line number, because numbering a line that
	            does not exist is worse than leaving the space blank.

	The difference itself is Myers' algorithm over whole lines, in its linear space form, so a diff of two
	versions of a file of fourteen thousand lines costs the edits rather than the file.

	Run it with --selftest to get the headless one: no window, no OpenGL, one exit code. The diff is checked
	against a brute force edit distance over thousands of random pairs - the script has to REBUILD the new
	text from the old one, and it has to be as short as the shortest one there is.

Usage:
	FUI_Diff                     the built in sample, which is small enough to read at a glance
	FUI_Diff <old> <new>         two files of your own
	FUI_Diff --side-by-side      start in the two pane view rather than the unified one
	FUI_Diff --selftest          the headless checks

Requirements:
	- C99 compiler
	- OpenGL 1.1 (fixed function, and only for the windowed mode)

Build (from the repository root):
	gcc -std=c99 demos/FUI_Diff/fui_diff_demo.c -I . -I demos/additions -I demos/dependencies -o fui_diff -lm -ldl
	./fui_diff --selftest

	Or with cmake:  cmake -S demos/FUI_Diff -B build/fui_diff && cmake --build build/fui_diff

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
#include <stdlib.h>
#include <string.h>

#define DEMO_WINDOW_TITLE "final_ui_texteditor.h diff demo (FPL + OpenGL)"
#define DEMO_WINDOW_WIDTH 1600
#define DEMO_WINDOW_HEIGHT 900
#define DEMO_FONT_PIXEL_HEIGHT 30.0f
#define DEMO_FONT_ATLAS_SIDE 512u

//! Which file the big case diffs against an edited copy of itself, relative to the repository root
#define DEMO_SOURCE_FILE_PATH "final_ui.h"

//! Largest file the demo is willing to read into memory
#define DEMO_MAX_FILE_BYTES (64 * 1024 * 1024)

//! Slot of the proportional face the interface itself is drawn with
#define DEMO_FACE_UI 0

//! Slot of the monospace face the two texts are shown in
#define DEMO_FACE_MONO 1

//! How many faces the demo bakes
#define DEMO_FACE_COUNT 2

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

//! Counted but only PRINTED when it fails, for the checks that run thousands of times over random input
static void CheckQuietImpl(const bool ok, const char *expression, const int line) {
	++g_checkTotal;
	if(!ok) {
		++g_checkFailed;
		printf("  FAIL %s  (%s:%d)\n", expression, g_checkSection, line);
	}
}

#define CHECK(condition) CheckImpl((condition), #condition, __LINE__)
#define CHECK_I(a, b) CheckImpl((long)(a) == (long)(b), #a " == " #b, __LINE__)
#define CHECK_QUIET(condition) CheckQuietImpl((condition), #condition, __LINE__)

// ----------------------------------------------------------------------------
// > A text as its lines
// ----------------------------------------------------------------------------

/*
	A text, cut into lines once, with a hash of every line beside it.

	A diff compares lines thousands of times over, and comparing them as bytes every time is what makes a
	naive diff slow rather than the algorithm. The hash answers "different" in one compare for almost every
	pair; the bytes are only looked at when it answers "maybe the same".
*/
typedef struct DiffText {
	//! The whole text, as one allocation this owns
	char *bytes;
	//! How many bytes there are
	int32_t byteCount;
	//! Where every line begins
	int32_t *lineStarts;
	//! How long every line is, its ending NOT counted
	int32_t *lineLengths;
	//! A hash of every line's bytes
	uint64_t *lineHashes;
	//! How many lines there are
	int32_t lineCount;
} DiffText;

//! FNV-1a, which is short, has no table and spreads well enough for what it is asked here
static uint64_t DiffHashBytes(const char *bytes, const int32_t byteCount) {
	const uint64_t offsetBasis = 1469598103934665603ull;
	const uint64_t prime = 1099511628211ull;
	uint64_t hash = offsetBasis;
	for(int32_t byteIndex = 0; byteIndex < byteCount; ++byteIndex) {
		hash = hash ^ (uint64_t)(uint8_t)bytes[byteIndex];
		hash = hash * prime;
	}
	return(hash);
}

static void DiffTextRelease(DiffText *text) {
	free(text->bytes);
	free(text->lineStarts);
	free(text->lineLengths);
	free(text->lineHashes);
	memset(text, 0, sizeof(*text));
}

/*
	Cuts a text into lines the same way the editor does: a LINE FEED ends a line, and a carriage return in
	front of one belongs to the line it ends rather than being part of it.

	A text that ends in a line feed has an empty last line, which is what every editor shows and what makes
	"the file grew a line at the end" come out as one added line rather than as a change to the last one.
*/
static bool DiffTextInit(DiffText *text, const char *bytes, const int32_t byteCount) {
	memset(text, 0, sizeof(*text));
	if(bytes == fpl_null || byteCount < 0) {
		return(false);
	}

	text->bytes = (char *)malloc((size_t)byteCount + 1);
	if(text->bytes == fpl_null) {
		return(false);
	}
	if(byteCount > 0) {
		memcpy(text->bytes, bytes, (size_t)byteCount);
	}
	text->bytes[byteCount] = '\0';
	text->byteCount = byteCount;

	int32_t lineCount = 1;
	for(int32_t byteIndex = 0; byteIndex < byteCount; ++byteIndex) {
		if(bytes[byteIndex] == '\n') {
			lineCount += 1;
		}
	}

	text->lineStarts = (int32_t *)malloc(sizeof(int32_t) * (size_t)lineCount);
	text->lineLengths = (int32_t *)malloc(sizeof(int32_t) * (size_t)lineCount);
	text->lineHashes = (uint64_t *)malloc(sizeof(uint64_t) * (size_t)lineCount);
	if(text->lineStarts == fpl_null || text->lineLengths == fpl_null || text->lineHashes == fpl_null) {
		DiffTextRelease(text);
		return(false);
	}
	text->lineCount = lineCount;

	int32_t lineIndex = 0;
	int32_t lineStart = 0;
	while(lineIndex < lineCount) {
		int32_t lineEnd = lineStart;
		while(lineEnd < byteCount && text->bytes[lineEnd] != '\n') {
			lineEnd += 1;
		}

		int32_t visibleEnd = lineEnd;
		bool endsWithCarriageReturn = (visibleEnd > lineStart) && (text->bytes[visibleEnd - 1] == '\r');
		if(endsWithCarriageReturn) {
			visibleEnd -= 1;
		}

		text->lineStarts[lineIndex] = lineStart;
		text->lineLengths[lineIndex] = visibleEnd - lineStart;
		text->lineHashes[lineIndex] = DiffHashBytes(&text->bytes[lineStart], visibleEnd - lineStart);

		lineStart = lineEnd + 1;
		lineIndex += 1;
	}
	return(true);
}

static const char *DiffTextGetLine(const DiffText *text, const int32_t lineIndex, int32_t *outLength) {
	if(lineIndex < 0 || lineIndex >= text->lineCount) {
		*outLength = 0;
		return("");
	}
	*outLength = text->lineLengths[lineIndex];
	int32_t lineStart = text->lineStarts[lineIndex];
	return(&text->bytes[lineStart]);
}

static bool DiffLinesAreEqual(const DiffText *oldText, const int32_t oldLine, const DiffText *newText, const int32_t newLine) {
	if(oldText->lineHashes[oldLine] != newText->lineHashes[newLine]) {
		return(false);
	}
	int32_t oldLength = oldText->lineLengths[oldLine];
	int32_t newLength = newText->lineLengths[newLine];
	if(oldLength != newLength) {
		return(false);
	}
	const char *oldBytes = &oldText->bytes[oldText->lineStarts[oldLine]];
	const char *newBytes = &newText->bytes[newText->lineStarts[newLine]];
	int comparison = memcmp(oldBytes, newBytes, (size_t)oldLength);
	return(comparison == 0);
}

// ----------------------------------------------------------------------------
// > The difference itself
// ----------------------------------------------------------------------------

/*
	Myers' algorithm over whole lines, in the linear space form.

	The greedy form of it keeps every step it took so that it can walk back out of them, which costs the
	SQUARE of the edit distance in memory - fine for two versions of a file that differ in twenty lines,
	hopeless for two files that differ everywhere. The form below never keeps a trace at all: it finds the
	one snake that sits halfway along the shortest script, recurses on what lies either side of it, and so
	needs two arrays the size of the two texts and nothing else.

	Everything the two texts have in common at the front and at the back is taken off before any of that
	happens. Taking the FRONT off is more than a shortcut: what is left then begins with a line the two do
	not share, so the snake in the middle of it cannot be the whole of it and the two halves either side
	are certainly smaller. Taking the back off is the shortcut - it keeps the search small, and nothing
	below depends on it.
*/

//! What a diff says about one line
typedef enum DiffEditKind {
	//! In both texts, unchanged
	DiffEditKind_Context = 0,
	//! In the old text only
	DiffEditKind_Removed,
	//! In the new text only
	DiffEditKind_Added,
} DiffEditKind;

typedef struct DiffEdit {
	//! What happened to this line
	DiffEditKind kind;
	//! Which line of the old text it is, or -1 when it is not in the old text
	int32_t oldLine;
	//! Which line of the new text it is, or -1 when it is not in the new text
	int32_t newLine;
} DiffEdit;

typedef struct DiffScript {
	//! The edits, in the order they read
	DiffEdit *edits;
	//! How many there are
	int32_t count;
	//! How many are allocated
	int32_t capacity;
	//! Set once an allocation was refused, so a short answer is never mistaken for a small difference
	bool hasOutOfMemory;
	//! How many times the safety net below had to catch a split that made no progress. It is meant to stay
	//! at zero forever, and the checks look at it rather than trusting that it does
	int32_t fallbackCount;
} DiffScript;

//! A run of lines the two texts share, which is what the middle of a shortest script looks like
typedef struct DiffSnake {
	int32_t startOldLine;
	int32_t startNewLine;
	int32_t endOldLine;
	int32_t endNewLine;
} DiffSnake;

typedef struct DiffWorkspace {
	//! How far the forward search has got on each diagonal
	int32_t *forward;
	//! And the backward one, on diagonals counted from the far end
	int32_t *backward;
	//! What is added to a diagonal to index the two arrays with it, since diagonals go negative
	int32_t offset;
	//! How many slots each of them has
	int32_t capacity;
} DiffWorkspace;

typedef struct DiffContext {
	const DiffText *oldText;
	const DiffText *newText;
	DiffWorkspace work;
	DiffScript *script;
} DiffContext;

static void DiffScriptRelease(DiffScript *script) {
	free(script->edits);
	memset(script, 0, sizeof(*script));
}

static void DiffScriptAppend(DiffScript *script, const DiffEditKind kind, const int32_t oldLine, const int32_t newLine) {
	if(script->count >= script->capacity) {
		int32_t wantedCapacity = (script->capacity > 0) ? (script->capacity * 2) : 256;
		DiffEdit *grown = (DiffEdit *)realloc(script->edits, sizeof(DiffEdit) * (size_t)wantedCapacity);
		if(grown == fpl_null) {
			script->hasOutOfMemory = true;
			return;
		}
		script->edits = grown;
		script->capacity = wantedCapacity;
	}

	DiffEdit *edit = &script->edits[script->count];
	edit->kind = kind;
	edit->oldLine = oldLine;
	edit->newLine = newLine;
	script->count += 1;
}

/*
	Finds the snake in the middle of the shortest script between the two runs.

	Two searches run towards each other, one from each end, one step of edit distance at a time. The moment
	the two overlap on some diagonal, the piece where they met IS in the middle of some shortest script -
	and everything before and after it is a smaller problem of the same shape.

	The forward search works in the runs' own coordinates; the backward one counts from the far end, so its
	diagonal k stands for the forward diagonal delta - k. That is where every one of the four index
	expressions below comes from, and it is the whole trick.
*/
static bool DiffFindMiddleSnake(DiffContext *diff, const int32_t oldStart, const int32_t oldCount, const int32_t newStart, const int32_t newCount, DiffSnake *outSnake) {
	int32_t *forward = diff->work.forward;
	int32_t *backward = diff->work.backward;
	int32_t offset = diff->work.offset;

	int32_t delta = oldCount - newCount;
	bool deltaIsOdd = (delta & 1) != 0;
	int32_t furthestDistance = (oldCount + newCount + 1) / 2;

	forward[offset + 1] = 0;
	backward[offset + 1] = 0;

	for(int32_t distance = 0; distance <= furthestDistance; ++distance) {
		for(int32_t diagonal = -distance; diagonal <= distance; diagonal += 2) {
			int32_t slot = offset + diagonal;
			int32_t oldOffset = 0;
			bool takesTheStepDown = (diagonal == -distance) || ((diagonal != distance) && (forward[slot - 1] < forward[slot + 1]));
			if(takesTheStepDown) {
				oldOffset = forward[slot + 1];
			} else {
				oldOffset = forward[slot - 1] + 1;
			}
			int32_t newOffset = oldOffset - diagonal;

			int32_t snakeOldStart = oldOffset;
			int32_t snakeNewStart = newOffset;
			while(oldOffset < oldCount && newOffset < newCount && DiffLinesAreEqual(diff->oldText, oldStart + oldOffset, diff->newText, newStart + newOffset)) {
				oldOffset += 1;
				newOffset += 1;
			}
			forward[slot] = oldOffset;

			// Only a run of odd length can have its two halves meet during the FORWARD step, because the
			// two searches are then one step out of phase with each other.
			if(deltaIsOdd) {
				int32_t backwardDiagonal = delta - diagonal;
				bool theBackwardSearchHasBeenThere = (backwardDiagonal >= -(distance - 1)) && (backwardDiagonal <= (distance - 1));
				if(theBackwardSearchHasBeenThere && ((forward[slot] + backward[offset + backwardDiagonal]) >= oldCount)) {
					outSnake->startOldLine = oldStart + snakeOldStart;
					outSnake->startNewLine = newStart + snakeNewStart;
					outSnake->endOldLine = oldStart + oldOffset;
					outSnake->endNewLine = newStart + newOffset;
					return(true);
				}
			}
		}

		for(int32_t diagonal = -distance; diagonal <= distance; diagonal += 2) {
			int32_t slot = offset + diagonal;
			int32_t oldOffset = 0;
			bool takesTheStepDown = (diagonal == -distance) || ((diagonal != distance) && (backward[slot - 1] < backward[slot + 1]));
			if(takesTheStepDown) {
				oldOffset = backward[slot + 1];
			} else {
				oldOffset = backward[slot - 1] + 1;
			}
			int32_t newOffset = oldOffset - diagonal;

			int32_t snakeOldStart = oldOffset;
			int32_t snakeNewStart = newOffset;
			while(oldOffset < oldCount && newOffset < newCount && DiffLinesAreEqual(diff->oldText, oldStart + oldCount - 1 - oldOffset, diff->newText, newStart + newCount - 1 - newOffset)) {
				oldOffset += 1;
				newOffset += 1;
			}
			backward[slot] = oldOffset;

			// And only a run of even length can have them meet during the backward step.
			if(!deltaIsOdd) {
				int32_t forwardDiagonal = delta - diagonal;
				bool theForwardSearchHasBeenThere = (forwardDiagonal >= -distance) && (forwardDiagonal <= distance);
				if(theForwardSearchHasBeenThere && ((backward[slot] + forward[offset + forwardDiagonal]) >= oldCount)) {
					// Back into the runs' own coordinates, which is where everything else works.
					outSnake->startOldLine = oldStart + oldCount - oldOffset;
					outSnake->startNewLine = newStart + newCount - newOffset;
					outSnake->endOldLine = oldStart + oldCount - snakeOldStart;
					outSnake->endNewLine = newStart + newCount - snakeNewStart;
					return(true);
				}
			}
		}
	}
	return(false);
}

static void DiffWalk(DiffContext *diff, int32_t oldStart, int32_t oldCount, int32_t newStart, int32_t newCount) {
	// Everything the two share at the front comes out as it is found; what they share at the back is
	// counted here and written after the middle, because that is the order it reads in.
	while(oldCount > 0 && newCount > 0 && DiffLinesAreEqual(diff->oldText, oldStart, diff->newText, newStart)) {
		DiffScriptAppend(diff->script, DiffEditKind_Context, oldStart, newStart);
		oldStart += 1;
		newStart += 1;
		oldCount -= 1;
		newCount -= 1;
	}

	int32_t sharedTailCount = 0;
	while((oldCount > sharedTailCount) && (newCount > sharedTailCount) && DiffLinesAreEqual(diff->oldText, oldStart + oldCount - 1 - sharedTailCount, diff->newText, newStart + newCount - 1 - sharedTailCount)) {
		sharedTailCount += 1;
	}

	int32_t middleOldCount = oldCount - sharedTailCount;
	int32_t middleNewCount = newCount - sharedTailCount;

	if(middleOldCount == 0) {
		for(int32_t lineOffset = 0; lineOffset < middleNewCount; ++lineOffset) {
			DiffScriptAppend(diff->script, DiffEditKind_Added, -1, newStart + lineOffset);
		}
	} else if(middleNewCount == 0) {
		for(int32_t lineOffset = 0; lineOffset < middleOldCount; ++lineOffset) {
			DiffScriptAppend(diff->script, DiffEditKind_Removed, oldStart + lineOffset, -1);
		}
	} else {
		DiffSnake snake;
		bool foundTheMiddle = DiffFindMiddleSnake(diff, oldStart, middleOldCount, newStart, middleNewCount, &snake);

		/*
			The safety net. What is left after the shared ends are off begins and finishes with lines the
			two texts do NOT share, so the snake in the middle of it can never be the whole of it - and if
			it ever were, the two halves below would be the whole problem again and this would not return.

			It has never fired. The checks look at fallbackCount rather than taking that on trust, because
			a net nobody can see going off is worse than no net at all.
		*/
		bool bothHalvesAreSmaller = foundTheMiddle;
		if(foundTheMiddle) {
			bool theHeadIsTheWholeThing = (snake.startOldLine >= (oldStart + middleOldCount)) && (snake.startNewLine >= (newStart + middleNewCount));
			bool theTailIsTheWholeThing = (snake.endOldLine <= oldStart) && (snake.endNewLine <= newStart);
			bothHalvesAreSmaller = !theHeadIsTheWholeThing && !theTailIsTheWholeThing;
		}

		if(bothHalvesAreSmaller) {
			DiffWalk(diff, oldStart, snake.startOldLine - oldStart, newStart, snake.startNewLine - newStart);
			int32_t snakeLength = snake.endOldLine - snake.startOldLine;
			for(int32_t lineOffset = 0; lineOffset < snakeLength; ++lineOffset) {
				DiffScriptAppend(diff->script, DiffEditKind_Context, snake.startOldLine + lineOffset, snake.startNewLine + lineOffset);
			}
			int32_t tailOldCount = (oldStart + middleOldCount) - snake.endOldLine;
			int32_t tailNewCount = (newStart + middleNewCount) - snake.endNewLine;
			DiffWalk(diff, snake.endOldLine, tailOldCount, snake.endNewLine, tailNewCount);
		} else {
			diff->script->fallbackCount += 1;
			for(int32_t lineOffset = 0; lineOffset < middleOldCount; ++lineOffset) {
				DiffScriptAppend(diff->script, DiffEditKind_Removed, oldStart + lineOffset, -1);
			}
			for(int32_t lineOffset = 0; lineOffset < middleNewCount; ++lineOffset) {
				DiffScriptAppend(diff->script, DiffEditKind_Added, -1, newStart + lineOffset);
			}
		}
	}

	for(int32_t lineOffset = 0; lineOffset < sharedTailCount; ++lineOffset) {
		DiffScriptAppend(diff->script, DiffEditKind_Context, oldStart + middleOldCount + lineOffset, newStart + middleNewCount + lineOffset);
	}
}

/*
	Puts every removal of a hunk in front of every addition of it.

	Myers hands back a shortest script and says nothing about the order of the steps inside one, so a run
	of changes can come out interleaved. That is a perfectly good answer and an unreadable diff: what makes
	a change legible is seeing what went and then seeing what came, and it is also what pairs a removal
	with the addition that replaced it - which is what the side by side view is built out of.
*/
static void DiffGroupHunks(DiffScript *script) {
	int32_t editIndex = 0;
	while(editIndex < script->count) {
		if(script->edits[editIndex].kind == DiffEditKind_Context) {
			editIndex += 1;
			continue;
		}

		int32_t hunkStart = editIndex;
		while(editIndex < script->count && script->edits[editIndex].kind != DiffEditKind_Context) {
			editIndex += 1;
		}
		int32_t hunkEnd = editIndex;

		// A stable partition, written out rather than sorted: the removals keep their order among
		// themselves and so do the additions, and that order is the one the two texts are in.
		int32_t writeIndex = hunkStart;
		for(int32_t readIndex = hunkStart; readIndex < hunkEnd; ++readIndex) {
			if(script->edits[readIndex].kind == DiffEditKind_Removed) {
				DiffEdit moved = script->edits[readIndex];
				for(int32_t shiftIndex = readIndex; shiftIndex > writeIndex; --shiftIndex) {
					script->edits[shiftIndex] = script->edits[shiftIndex - 1];
				}
				script->edits[writeIndex] = moved;
				writeIndex += 1;
			}
		}
	}
}

static bool DiffCompute(const DiffText *oldText, const DiffText *newText, DiffScript *outScript) {
	memset(outScript, 0, sizeof(*outScript));

	DiffContext diff;
	diff.oldText = oldText;
	diff.newText = newText;
	diff.script = outScript;

	// Room for every diagonal there can be, in both directions, plus the one slot either side that the
	// step reads before it is written.
	int32_t totalLineCount = oldText->lineCount + newText->lineCount;
	diff.work.offset = totalLineCount + 3;
	diff.work.capacity = totalLineCount * 2 + 8;
	diff.work.forward = (int32_t *)malloc(sizeof(int32_t) * (size_t)diff.work.capacity);
	diff.work.backward = (int32_t *)malloc(sizeof(int32_t) * (size_t)diff.work.capacity);
	if(diff.work.forward == fpl_null || diff.work.backward == fpl_null) {
		free(diff.work.forward);
		free(diff.work.backward);
		outScript->hasOutOfMemory = true;
		return(false);
	}

	DiffWalk(&diff, 0, oldText->lineCount, 0, newText->lineCount);
	free(diff.work.forward);
	free(diff.work.backward);

	if(outScript->hasOutOfMemory) {
		return(false);
	}
	DiffGroupHunks(outScript);
	return(true);
}

// ----------------------------------------------------------------------------
// > The difference as rows on a screen
// ----------------------------------------------------------------------------

//! What one row of either view is
typedef enum DiffRowKind {
	//! In both texts, unchanged
	DiffRowKind_Context = 0,
	//! A line that went
	DiffRowKind_Removed,
	//! A line that came
	DiffRowKind_Added,
	//! Not a line at all - it stands opposite one the other side has and this one has not
	DiffRowKind_Filler,
	//! The band that stands for the run of unchanged lines that is not being shown
	DiffRowKind_Skipped,
	//! How many kinds there are
	DiffRowKind_Count,
} DiffRowKind;

typedef struct DiffViewRow {
	//! What this row is
	DiffRowKind kind;
	//! Its bytes, pointing into one of the two texts, or null for a row that has none
	const char *text;
	//! How many bytes those are
	int32_t textLength;
	//! What goes in the first column of the gutter, or -1 for blanks
	int32_t gutterFirst;
	//! And in the second one. Always -1 in the side by side view, which has one column
	int32_t gutterSecond;
	//! Which line of the old text this is, or -1
	int32_t oldLine;
	//! Which line of the new text this is, or -1
	int32_t newLine;
	//! The line of the OTHER text that this one replaced or was replaced by, or -1 when it stands alone.
	//! What it is for is marking the piece of the line that really changed
	int32_t partnerLine;
} DiffViewRow;

typedef struct DiffViewRows {
	DiffViewRow *rows;
	int32_t count;
	int32_t capacity;
	bool hasOutOfMemory;
} DiffViewRows;

static void DiffViewRowsRelease(DiffViewRows *rows) {
	free(rows->rows);
	memset(rows, 0, sizeof(*rows));
}

static DiffViewRow *DiffViewRowsAppend(DiffViewRows *rows) {
	if(rows->count >= rows->capacity) {
		int32_t wantedCapacity = (rows->capacity > 0) ? (rows->capacity * 2) : 256;
		DiffViewRow *grown = (DiffViewRow *)realloc(rows->rows, sizeof(DiffViewRow) * (size_t)wantedCapacity);
		if(grown == fpl_null) {
			rows->hasOutOfMemory = true;
			return(fpl_null);
		}
		rows->rows = grown;
		rows->capacity = wantedCapacity;
	}

	DiffViewRow *row = &rows->rows[rows->count];
	memset(row, 0, sizeof(*row));
	row->gutterFirst = -1;
	row->gutterSecond = -1;
	row->oldLine = -1;
	row->newLine = -1;
	row->partnerLine = -1;
	rows->count += 1;
	return(row);
}

/*
	Where two lines really differ, as a byte range on each of them.

	What the two share at the front and at the back is not part of the change, and marking a whole line
	because one word in it moved is the difference between a diff that is read and one that is squinted at.
	Both ends are pulled back onto a character boundary, because half a character washed is a smear.
*/
static void DiffFindChangedSpan(const char *oldBytes, const int32_t oldLength, const char *newBytes, const int32_t newLength, int32_t *outOldStart, int32_t *outOldEnd, int32_t *outNewStart, int32_t *outNewEnd) {
	int32_t shorterLength = (oldLength < newLength) ? oldLength : newLength;

	int32_t sharedHead = 0;
	while(sharedHead < shorterLength && oldBytes[sharedHead] == newBytes[sharedHead]) {
		sharedHead += 1;
	}
	while(sharedHead > 0 && (((uint8_t)oldBytes[sharedHead] & 0xC0u) == 0x80u)) {
		sharedHead -= 1;
	}

	int32_t sharedTail = 0;
	int32_t roomForTail = shorterLength - sharedHead;
	while(sharedTail < roomForTail && oldBytes[oldLength - 1 - sharedTail] == newBytes[newLength - 1 - sharedTail]) {
		sharedTail += 1;
	}
	while(sharedTail > 0 && (((uint8_t)oldBytes[oldLength - sharedTail] & 0xC0u) == 0x80u)) {
		sharedTail -= 1;
	}

	*outOldStart = sharedHead;
	*outOldEnd = oldLength - sharedTail;
	*outNewStart = sharedHead;
	*outNewEnd = newLength - sharedTail;
}

//! Which edits of a hunk are its removals and which its additions, given that they have been grouped
static void DiffFindHunkHalves(const DiffScript *script, const int32_t hunkStart, int32_t *outRemovedCount, int32_t *outAddedCount, int32_t *outHunkEnd) {
	int32_t editIndex = hunkStart;
	while(editIndex < script->count && script->edits[editIndex].kind == DiffEditKind_Removed) {
		editIndex += 1;
	}
	*outRemovedCount = editIndex - hunkStart;
	int32_t addedStart = editIndex;
	while(editIndex < script->count && script->edits[editIndex].kind == DiffEditKind_Added) {
		editIndex += 1;
	}
	*outAddedCount = editIndex - addedStart;
	*outHunkEnd = editIndex;
}

/*
	Everything woven into one column, which is the unified view.

	The two numbers beside a row are what say which of the three it is - a row with only a left number went,
	a row with only a right one came, a row with both stayed - so there is nothing to put a plus or a minus
	in front of.
*/
static bool DiffBuildUnifiedRows(const DiffScript *script, const DiffText *oldText, const DiffText *newText, DiffViewRows *outRows) {
	memset(outRows, 0, sizeof(*outRows));

	int32_t editIndex = 0;
	while(editIndex < script->count) {
		const DiffEdit *edit = &script->edits[editIndex];
		if(edit->kind == DiffEditKind_Context) {
			DiffViewRow *row = DiffViewRowsAppend(outRows);
			if(row == fpl_null) {
				return(false);
			}
			row->kind = DiffRowKind_Context;
			row->text = DiffTextGetLine(newText, edit->newLine, &row->textLength);
			row->gutterFirst = edit->oldLine;
			row->gutterSecond = edit->newLine;
			row->oldLine = edit->oldLine;
			row->newLine = edit->newLine;
			editIndex += 1;
			continue;
		}

		int32_t removedCount = 0;
		int32_t addedCount = 0;
		int32_t hunkEnd = 0;
		DiffFindHunkHalves(script, editIndex, &removedCount, &addedCount, &hunkEnd);
		int32_t removedStart = editIndex;
		int32_t addedStart = editIndex + removedCount;

		for(int32_t halfIndex = 0; halfIndex < removedCount; ++halfIndex) {
			const DiffEdit *removedEdit = &script->edits[removedStart + halfIndex];
			DiffViewRow *row = DiffViewRowsAppend(outRows);
			if(row == fpl_null) {
				return(false);
			}
			row->kind = DiffRowKind_Removed;
			row->text = DiffTextGetLine(oldText, removedEdit->oldLine, &row->textLength);
			row->gutterFirst = removedEdit->oldLine;
			row->oldLine = removedEdit->oldLine;
			if(halfIndex < addedCount) {
				row->partnerLine = script->edits[addedStart + halfIndex].newLine;
			}
		}

		for(int32_t halfIndex = 0; halfIndex < addedCount; ++halfIndex) {
			const DiffEdit *addedEdit = &script->edits[addedStart + halfIndex];
			DiffViewRow *row = DiffViewRowsAppend(outRows);
			if(row == fpl_null) {
				return(false);
			}
			row->kind = DiffRowKind_Added;
			row->text = DiffTextGetLine(newText, addedEdit->newLine, &row->textLength);
			row->gutterSecond = addedEdit->newLine;
			row->newLine = addedEdit->newLine;
			if(halfIndex < removedCount) {
				row->partnerLine = script->edits[removedStart + halfIndex].oldLine;
			}
		}

		editIndex = hunkEnd;
	}
	return(!outRows->hasOutOfMemory);
}

/*
	The same difference as two columns that are exactly as long as each other.

	Where a hunk took three lines away and put five back, the three are set beside the first three of the
	five and the other two get a filler beside them. That is the whole of the "same number of rows on both
	sides" rule, and it is what lets the two panes be scrolled as one thing.

	BOTH numbers go in the gutter of BOTH panes - the line the row was and the line it became - so the two
	gutters read the same all the way down. Each pane showing only its own file's number is what a split
	view usually does, and it means the two columns of numbers drift apart the moment anything is inserted
	above; two identical gutters say instead, on either side, where a row sits in each of the two files.
*/
static bool DiffBuildSideRows(const DiffScript *script, const DiffText *oldText, const DiffText *newText, DiffViewRows *outLeftRows, DiffViewRows *outRightRows) {
	memset(outLeftRows, 0, sizeof(*outLeftRows));
	memset(outRightRows, 0, sizeof(*outRightRows));

	int32_t editIndex = 0;
	while(editIndex < script->count) {
		const DiffEdit *edit = &script->edits[editIndex];
		if(edit->kind == DiffEditKind_Context) {
			DiffViewRow *leftRow = DiffViewRowsAppend(outLeftRows);
			DiffViewRow *rightRow = DiffViewRowsAppend(outRightRows);
			if(leftRow == fpl_null || rightRow == fpl_null) {
				return(false);
			}
			leftRow->kind = DiffRowKind_Context;
			leftRow->text = DiffTextGetLine(oldText, edit->oldLine, &leftRow->textLength);
			leftRow->oldLine = edit->oldLine;

			rightRow->kind = DiffRowKind_Context;
			rightRow->text = DiffTextGetLine(newText, edit->newLine, &rightRow->textLength);
			rightRow->newLine = edit->newLine;

			leftRow->gutterFirst = edit->oldLine;
			leftRow->gutterSecond = edit->newLine;
			rightRow->gutterFirst = edit->oldLine;
			rightRow->gutterSecond = edit->newLine;
			editIndex += 1;
			continue;
		}

		int32_t removedCount = 0;
		int32_t addedCount = 0;
		int32_t hunkEnd = 0;
		DiffFindHunkHalves(script, editIndex, &removedCount, &addedCount, &hunkEnd);
		int32_t removedStart = editIndex;
		int32_t addedStart = editIndex + removedCount;
		int32_t pairCount = (removedCount > addedCount) ? removedCount : addedCount;

		for(int32_t pairIndex = 0; pairIndex < pairCount; ++pairIndex) {
			DiffViewRow *leftRow = DiffViewRowsAppend(outLeftRows);
			DiffViewRow *rightRow = DiffViewRowsAppend(outRightRows);
			if(leftRow == fpl_null || rightRow == fpl_null) {
				return(false);
			}

			if(pairIndex < removedCount) {
				const DiffEdit *removedEdit = &script->edits[removedStart + pairIndex];
				leftRow->kind = DiffRowKind_Removed;
				leftRow->text = DiffTextGetLine(oldText, removedEdit->oldLine, &leftRow->textLength);
				leftRow->oldLine = removedEdit->oldLine;
			} else {
				leftRow->kind = DiffRowKind_Filler;
			}

			if(pairIndex < addedCount) {
				const DiffEdit *addedEdit = &script->edits[addedStart + pairIndex];
				rightRow->kind = DiffRowKind_Added;
				rightRow->text = DiffTextGetLine(newText, addedEdit->newLine, &rightRow->textLength);
				rightRow->newLine = addedEdit->newLine;
			} else {
				rightRow->kind = DiffRowKind_Filler;
			}

			// One row is one PAIR, so the two numbers of that pair are what both gutters say - and a half
			// of it that is not there leaves its column blank on both sides rather than only on one.
			leftRow->gutterFirst = leftRow->oldLine;
			leftRow->gutterSecond = rightRow->newLine;
			rightRow->gutterFirst = leftRow->oldLine;
			rightRow->gutterSecond = rightRow->newLine;

			// Only a row that has a line on BOTH sides is a change to a line rather than a line coming or
			// going, and only a change to a line has a piece of it worth marking.
			bool isAChangedPair = (pairIndex < removedCount) && (pairIndex < addedCount);
			if(isAChangedPair) {
				leftRow->partnerLine = rightRow->newLine;
				rightRow->partnerLine = leftRow->oldLine;
			}
		}

		editIndex = hunkEnd;
	}
	return(!outLeftRows->hasOutOfMemory && !outRightRows->hasOutOfMemory);
}

/*
	Drops the runs of unchanged lines nobody is reading, leaving a band where they were.

	Without this a diff of two versions of a fourteen thousand line file is fourteen thousand rows of which
	nine are interesting. The band carries no line number, so the numbers on either side of it JUMP - which
	is what says how much is not being shown, without a word of text having to say it.

	Runs the same way over one row array or over two, and a pair of side by side arrays has to be given both
	at once: a row is only skippable when NEITHER side has anything to say about it, or the two columns
	would stop lining up.
*/
static bool DiffCollapseContext(DiffViewRows *firstRows, DiffViewRows *secondRows, const int32_t contextLineCount) {
	int32_t rowCount = firstRows->count;
	if(secondRows != fpl_null && secondRows->count != rowCount) {
		return(false);
	}
	if(contextLineCount < 0 || rowCount == 0) {
		return(true);
	}

	bool *isWorthShowing = (bool *)calloc((size_t)rowCount, sizeof(bool));
	if(isWorthShowing == fpl_null) {
		return(false);
	}

	for(int32_t rowIndex = 0; rowIndex < rowCount; ++rowIndex) {
		bool isAChange = firstRows->rows[rowIndex].kind != DiffRowKind_Context;
		if(!isAChange && secondRows != fpl_null) {
			isAChange = secondRows->rows[rowIndex].kind != DiffRowKind_Context;
		}
		if(!isAChange) {
			continue;
		}

		int32_t firstNeighbour = rowIndex - contextLineCount;
		int32_t lastNeighbour = rowIndex + contextLineCount;
		if(firstNeighbour < 0) {
			firstNeighbour = 0;
		}
		if(lastNeighbour > (rowCount - 1)) {
			lastNeighbour = rowCount - 1;
		}
		for(int32_t neighbourIndex = firstNeighbour; neighbourIndex <= lastNeighbour; ++neighbourIndex) {
			isWorthShowing[neighbourIndex] = true;
		}
	}

	int32_t writeIndex = 0;
	int32_t readIndex = 0;
	while(readIndex < rowCount) {
		if(isWorthShowing[readIndex]) {
			firstRows->rows[writeIndex] = firstRows->rows[readIndex];
			if(secondRows != fpl_null) {
				secondRows->rows[writeIndex] = secondRows->rows[readIndex];
			}
			writeIndex += 1;
			readIndex += 1;
			continue;
		}

		while(readIndex < rowCount && !isWorthShowing[readIndex]) {
			readIndex += 1;
		}

		DiffViewRow skippedRow;
		memset(&skippedRow, 0, sizeof(skippedRow));
		skippedRow.kind = DiffRowKind_Skipped;
		skippedRow.gutterFirst = -1;
		skippedRow.gutterSecond = -1;
		skippedRow.oldLine = -1;
		skippedRow.newLine = -1;
		skippedRow.partnerLine = -1;
		firstRows->rows[writeIndex] = skippedRow;
		if(secondRows != fpl_null) {
			secondRows->rows[writeIndex] = skippedRow;
		}
		writeIndex += 1;
	}

	firstRows->count = writeIndex;
	if(secondRows != fpl_null) {
		secondRows->count = writeIndex;
	}
	free(isWorthShowing);
	return(true);
}

// ----------------------------------------------------------------------------
// > One pane
// ----------------------------------------------------------------------------

/*
	What a pane paints its rows with.

	Everything here is a WASH rather than a text colour, because a diff has to be readable under a syntax
	highlighter rather than instead of one - Gittyup colours the line and leaves the code alone, and so does
	this. The washes are translucent for the same reason.
*/
typedef struct DiffPalette {
	//! Behind a line that went
	fuiColor removedBackground;
	//! Behind a line that came
	fuiColor addedBackground;
	//! Behind the piece of a changed line that really differs, on top of the wash for the whole line
	fuiColor removedInsideBackground;
	//! And on the line that replaced it
	fuiColor addedInsideBackground;
	//! Behind a row that stands for a line the other side has and this one has not
	fuiColor fillerBackground;
	//! Behind the band that stands for the unchanged lines that are not being shown
	fuiColor skippedBackground;
	//! The bar at the left edge of the gutter beside a line that went, when those are switched on
	fuiColor removedGutterMarker;
	//! And beside one that came
	fuiColor addedGutterMarker;
} DiffPalette;

//! Red for what went and green for what came, which is what a unified diff has looked like since diff(1)
static DiffPalette DiffUnifiedPalette(void) {
	DiffPalette palette;
	palette.removedBackground = fuiColorRGBA(0.55f, 0.16f, 0.19f, 0.30f);
	palette.addedBackground = fuiColorRGBA(0.16f, 0.48f, 0.24f, 0.30f);
	palette.removedInsideBackground = fuiColorRGBA(0.78f, 0.20f, 0.24f, 0.38f);
	palette.addedInsideBackground = fuiColorRGBA(0.20f, 0.70f, 0.32f, 0.38f);
	palette.fillerBackground = fuiColorRGBA(0.46f, 0.49f, 0.58f, 0.18f);
	palette.skippedBackground = fuiColorRGBA(0.40f, 0.46f, 0.62f, 0.34f);
	palette.removedGutterMarker = fuiColorRGBA(0.72f, 0.24f, 0.28f, 0.90f);
	palette.addedGutterMarker = fuiColorRGBA(0.26f, 0.66f, 0.34f, 0.90f);
	return(palette);
}

//! Blue on the left for what was there and green on the right for what is there now, which is the side by
//! side reading: the two panes are two VERSIONS rather than two halves of one change
static DiffPalette DiffSidePalette(void) {
	DiffPalette palette = DiffUnifiedPalette();
	palette.removedBackground = fuiColorRGBA(0.18f, 0.34f, 0.62f, 0.30f);
	palette.removedInsideBackground = fuiColorRGBA(0.26f, 0.50f, 0.90f, 0.40f);
	palette.removedGutterMarker = fuiColorRGBA(0.30f, 0.52f, 0.86f, 0.90f);
	return(palette);
}

typedef struct DiffPane {
	//! The document this pane shows, which is built out of rows rather than read from anywhere
	fuiEditor editor;
	//! What it is drawn with, edited by the toolbar and pushed back whenever anything changes
	fuiEditorConfig config;
	//! One entry per row that is not plain context, in row order - which is the order sorted by line
	fuiEditorLineDecoration *lineDecorations;
	int32_t lineDecorationCount;
	//! One entry per piece of a changed line, in row order, which is also sorted by offset
	fuiEditorRangeDecoration *rangeDecorations;
	int32_t rangeDecorationCount;
	//! What stands in the first gutter column of each row, or -1 for blanks
	int32_t *gutterFirst;
	//! And in the second one. Null when the pane has only one column
	int32_t *gutterSecond;
	//! How many rows there are, which is how long all four arrays above are
	int32_t rowCount;
	//! How many characters wide one gutter column is written, so the two line up under each other
	int32_t gutterColumnWidth;
	//! Whether fuiEditorInit has run on the editor in here
	bool isReady;
} DiffPane;

//! How many characters a number takes to write
static int32_t DiffDigitCount(const int32_t value) {
	int32_t digitCount = 1;
	int32_t remaining = value;
	while(remaining >= 10) {
		remaining /= 10;
		digitCount += 1;
	}
	return(digitCount);
}

//! Writes one right aligned number into a column of blanks, or leaves the whole column blank
static int32_t DiffWriteNumberColumn(char *destination, const int32_t destinationCapacity, const int32_t writeOffset, const int32_t value, const int32_t columnWidth) {
	int32_t written = 0;
	char digits[16];
	int32_t digitCount = 0;
	if(value >= 0) {
		int32_t remaining = value;
		do {
			digits[digitCount] = (char)('0' + (remaining % 10));
			digitCount += 1;
			remaining /= 10;
		} while(remaining > 0 && digitCount < (int32_t)sizeof(digits));
	}

	int32_t padCount = columnWidth - digitCount;
	for(int32_t padIndex = 0; padIndex < padCount; ++padIndex) {
		if((writeOffset + written) < destinationCapacity) {
			destination[writeOffset + written] = ' ';
		}
		written += 1;
	}
	for(int32_t digitIndex = digitCount - 1; digitIndex >= 0; --digitIndex) {
		if((writeOffset + written) < destinationCapacity) {
			destination[writeOffset + written] = digits[digitIndex];
		}
		written += 1;
	}
	return(written);
}

/*
	What stands in the gutter beside one row.

	Counted from one, the way every other tool that names a line counts. A row with nothing in either column
	answers zero, and the editor then draws no number at all - which is what a filler row and the band of
	skipped lines both want, because numbering a line that is not there is worse than a blank space.
*/
static int32_t DiffFormatGutterText(fuiEditor *editor, const int32_t documentLine, char *destination, const int32_t destinationCapacity, void *userData) {
	(void)editor;
	DiffPane *pane = (DiffPane *)userData;
	if(pane == fpl_null || documentLine < 0 || documentLine >= pane->rowCount) {
		return(0);
	}

	int32_t firstNumber = pane->gutterFirst[documentLine];
	int32_t secondNumber = -1;
	if(pane->gutterSecond != fpl_null) {
		secondNumber = pane->gutterSecond[documentLine];
	}
	bool hasNothingToSay = (firstNumber < 0) && (secondNumber < 0);
	if(hasNothingToSay) {
		return(0);
	}

	int32_t writeOffset = 0;
	int32_t firstValue = (firstNumber >= 0) ? (firstNumber + 1) : -1;
	writeOffset += DiffWriteNumberColumn(destination, destinationCapacity, writeOffset, firstValue, pane->gutterColumnWidth);
	if(pane->gutterSecond != fpl_null) {
		if(writeOffset < destinationCapacity) {
			destination[writeOffset] = ' ';
		}
		writeOffset += 1;
		int32_t secondValue = (secondNumber >= 0) ? (secondNumber + 1) : -1;
		writeOffset += DiffWriteNumberColumn(destination, destinationCapacity, writeOffset, secondValue, pane->gutterColumnWidth);
	}
	return(writeOffset);
}

static void DiffPaneRelease(DiffPane *pane) {
	if(pane->isReady) {
		fuiEditorRelease(&pane->editor);
	}
	free(pane->lineDecorations);
	free(pane->rangeDecorations);
	free(pane->gutterFirst);
	free(pane->gutterSecond);
	memset(pane, 0, sizeof(*pane));
}

static void DiffPaneReleaseRows(DiffPane *pane) {
	free(pane->lineDecorations);
	free(pane->rangeDecorations);
	free(pane->gutterFirst);
	free(pane->gutterSecond);
	pane->lineDecorations = fpl_null;
	pane->rangeDecorations = fpl_null;
	pane->gutterFirst = fpl_null;
	pane->gutterSecond = fpl_null;
	pane->lineDecorationCount = 0;
	pane->rangeDecorationCount = 0;
	pane->rowCount = 0;
}

//! Which wash a row is painted with, and which bar it gets in the gutter when those are on
static void DiffPickRowColors(const DiffRowKind kind, const DiffPalette *palette, const bool showsGutterBars, fuiColor *outBackground, fuiColor *outGutterMarker) {
	fuiColor nothing = fuiColorRGBA(0.0f, 0.0f, 0.0f, 0.0f);
	*outBackground = nothing;
	*outGutterMarker = nothing;
	switch(kind) {
		case DiffRowKind_Removed: {
			*outBackground = palette->removedBackground;
			if(showsGutterBars) {
				*outGutterMarker = palette->removedGutterMarker;
			}
		} break;
		case DiffRowKind_Added: {
			*outBackground = palette->addedBackground;
			if(showsGutterBars) {
				*outGutterMarker = palette->addedGutterMarker;
			}
		} break;
		case DiffRowKind_Filler: {
			*outBackground = palette->fillerBackground;
		} break;
		case DiffRowKind_Skipped: {
			*outBackground = palette->skippedBackground;
		} break;
		default: break;
	}
}

/*
	Turns a list of rows into a document, a set of decorations and a gutter.

	The document is built as ONE buffer and handed over in one call rather than being written into line by
	line: an editor filled a line at a time would record every one of them on its undo stack, and a view
	that is rebuilt whenever a checkbox is clicked has nothing to undo.
*/
static bool DiffPaneFill(DiffPane *pane, const DiffViewRows *rows, const DiffText *oldText, const DiffText *newText, const DiffPalette *palette, const bool marksInsideLines, const bool showsGutterBars, const bool hasTwoGutterColumns) {
	DiffPaneReleaseRows(pane);

	int32_t rowCount = rows->count;
	pane->rowCount = rowCount;
	pane->gutterColumnWidth = DiffDigitCount((oldText->lineCount > newText->lineCount) ? oldText->lineCount : newText->lineCount);

	int32_t documentLength = 0;
	for(int32_t rowIndex = 0; rowIndex < rowCount; ++rowIndex) {
		documentLength += rows->rows[rowIndex].textLength;
		bool needsALineFeed = rowIndex < (rowCount - 1);
		if(needsALineFeed) {
			documentLength += 1;
		}
	}

	char *documentText = (char *)malloc((size_t)documentLength + 1);
	pane->gutterFirst = (int32_t *)malloc(sizeof(int32_t) * (size_t)(rowCount + 1));
	if(hasTwoGutterColumns) {
		pane->gutterSecond = (int32_t *)malloc(sizeof(int32_t) * (size_t)(rowCount + 1));
	}
	pane->lineDecorations = (fuiEditorLineDecoration *)malloc(sizeof(fuiEditorLineDecoration) * (size_t)(rowCount + 1));
	pane->rangeDecorations = (fuiEditorRangeDecoration *)malloc(sizeof(fuiEditorRangeDecoration) * (size_t)(rowCount + 1));
	bool everyAllocationWorked = (documentText != fpl_null) && (pane->gutterFirst != fpl_null) && (pane->lineDecorations != fpl_null) && (pane->rangeDecorations != fpl_null);
	if(hasTwoGutterColumns && pane->gutterSecond == fpl_null) {
		everyAllocationWorked = false;
	}
	if(!everyAllocationWorked) {
		free(documentText);
		DiffPaneReleaseRows(pane);
		return(false);
	}

	int32_t writeOffset = 0;
	for(int32_t rowIndex = 0; rowIndex < rowCount; ++rowIndex) {
		const DiffViewRow *row = &rows->rows[rowIndex];
		int32_t rowStartOffset = writeOffset;
		if(row->textLength > 0) {
			memcpy(&documentText[writeOffset], row->text, (size_t)row->textLength);
			writeOffset += row->textLength;
		}
		bool needsALineFeed = rowIndex < (rowCount - 1);
		if(needsALineFeed) {
			documentText[writeOffset] = '\n';
			writeOffset += 1;
		}

		pane->gutterFirst[rowIndex] = row->gutterFirst;
		if(pane->gutterSecond != fpl_null) {
			pane->gutterSecond[rowIndex] = row->gutterSecond;
		}

		fuiColor rowBackground;
		fuiColor rowGutterMarker;
		DiffPickRowColors(row->kind, palette, showsGutterBars, &rowBackground, &rowGutterMarker);
		bool paintsAnything = (rowBackground.a > 0.0f) || (rowGutterMarker.a > 0.0f);
		if(paintsAnything) {
			fuiEditorLineDecoration *decoration = &pane->lineDecorations[pane->lineDecorationCount];
			decoration->line = rowIndex;
			decoration->background = rowBackground;
			decoration->gutterMarker = rowGutterMarker;
			pane->lineDecorationCount += 1;
		}

		// The piece of a changed line that really differs, which only a row with a partner on the other
		// side has at all - a line that simply came or went changed all the way through.
		bool marksItsChange = marksInsideLines && (row->partnerLine >= 0) && (row->textLength > 0);
		if(marksItsChange) {
			const DiffText *partnerText = (row->kind == DiffRowKind_Removed) ? newText : oldText;
			int32_t partnerLength = 0;
			const char *partnerBytes = DiffTextGetLine(partnerText, row->partnerLine, &partnerLength);

			int32_t ownStart = 0;
			int32_t ownEnd = 0;
			int32_t partnerStart = 0;
			int32_t partnerEnd = 0;
			DiffFindChangedSpan(row->text, row->textLength, partnerBytes, partnerLength, &ownStart, &ownEnd, &partnerStart, &partnerEnd);
			if(ownEnd > ownStart) {
				fuiEditorRangeDecoration *decoration = &pane->rangeDecorations[pane->rangeDecorationCount];
				decoration->startOffset = rowStartOffset + ownStart;
				decoration->endOffset = rowStartOffset + ownEnd;
				decoration->background = (row->kind == DiffRowKind_Removed) ? palette->removedInsideBackground : palette->addedInsideBackground;
				pane->rangeDecorationCount += 1;
			}
		}
	}
	documentText[documentLength] = '\0';

	bool didSetText = fuiEditorSetText(&pane->editor, documentText, documentLength);
	free(documentText);

	// Wide enough for both columns and the blank between them, since the editor cannot ask every line what
	// it is going to write without walking the whole document to find out.
	int32_t gutterCharacterCount = pane->gutterColumnWidth;
	if(hasTwoGutterColumns) {
		gutterCharacterCount = pane->gutterColumnWidth * 2 + 1;
	}
	pane->config.metrics.gutterMinDigits = gutterCharacterCount;
	pane->config.callbacks.formatGutterText = DiffFormatGutterText;
	pane->config.callbacks.userData = pane;
	fuiEditorSetConfig(&pane->editor, &pane->config);

	fuiEditorDecorations decorations = fplZeroInit;
	decorations.lines = pane->lineDecorations;
	decorations.lineCount = pane->lineDecorationCount;
	decorations.ranges = pane->rangeDecorations;
	decorations.rangeCount = pane->rangeDecorationCount;
	fuiEditorSetDecorations(&pane->editor, &decorations);
	return(didSetText);
}

// ----------------------------------------------------------------------------
// > What the checks are run against
// ----------------------------------------------------------------------------

//! A small pseudo random source of its own, so the checks are the same run on any machine on any day
typedef struct DiffRandom {
	uint64_t state;
} DiffRandom;

static uint32_t DiffRandomNext(DiffRandom *random) {
	random->state = random->state * 6364136223846793005ull + 1442695040888963407ull;
	uint32_t folded = (uint32_t)(random->state >> 33);
	return(folded);
}

static int32_t DiffRandomBelow(DiffRandom *random, const int32_t bound) {
	if(bound <= 0) {
		return(0);
	}
	uint32_t drawn = DiffRandomNext(random);
	return((int32_t)(drawn % (uint32_t)bound));
}

/*
	The shortest number of lines that have to go and come to turn one text into the other, worked out the
	slow and obvious way.

	This is the whole reason the checks mean anything. Myers is short, and every short thing that is wrong
	is wrong in a way that still LOOKS like a diff - a script that rebuilds the new text but takes four
	steps where three would do is a diff nobody would notice was bad. So the answer is held against a table
	that cannot be clever: every cell is the best of the three that reach it, and nothing else.
*/
static int32_t DiffBruteForceDistance(const DiffText *oldText, const DiffText *newText) {
	int32_t oldCount = oldText->lineCount;
	int32_t newCount = newText->lineCount;
	int32_t rowLength = newCount + 1;
	int32_t *table = (int32_t *)malloc(sizeof(int32_t) * (size_t)((oldCount + 1) * rowLength));
	if(table == fpl_null) {
		return(-1);
	}

	for(int32_t newIndex = 0; newIndex <= newCount; ++newIndex) {
		table[newIndex] = newIndex;
	}
	for(int32_t oldIndex = 1; oldIndex <= oldCount; ++oldIndex) {
		table[oldIndex * rowLength] = oldIndex;
		for(int32_t newIndex = 1; newIndex <= newCount; ++newIndex) {
			int32_t best = table[(oldIndex - 1) * rowLength + newIndex] + 1;
			int32_t fromTheLeft = table[oldIndex * rowLength + (newIndex - 1)] + 1;
			if(fromTheLeft < best) {
				best = fromTheLeft;
			}
			if(DiffLinesAreEqual(oldText, oldIndex - 1, newText, newIndex - 1)) {
				int32_t diagonal = table[(oldIndex - 1) * rowLength + (newIndex - 1)];
				if(diagonal < best) {
					best = diagonal;
				}
			}
			table[oldIndex * rowLength + newIndex] = best;
		}
	}

	int32_t distance = table[oldCount * rowLength + newCount];
	free(table);
	return(distance);
}

//! Walks a script and rebuilds both texts out of it, which is the one thing a diff must never get wrong
static bool DiffScriptRebuildsBothTexts(const DiffScript *script, const DiffText *oldText, const DiffText *newText) {
	int32_t expectedOldLine = 0;
	int32_t expectedNewLine = 0;
	for(int32_t editIndex = 0; editIndex < script->count; ++editIndex) {
		const DiffEdit *edit = &script->edits[editIndex];
		if(edit->kind == DiffEditKind_Context || edit->kind == DiffEditKind_Removed) {
			if(edit->oldLine != expectedOldLine) {
				return(false);
			}
			expectedOldLine += 1;
		}
		if(edit->kind == DiffEditKind_Context || edit->kind == DiffEditKind_Added) {
			if(edit->newLine != expectedNewLine) {
				return(false);
			}
			expectedNewLine += 1;
		}
		if(edit->kind == DiffEditKind_Context && !DiffLinesAreEqual(oldText, edit->oldLine, newText, edit->newLine)) {
			return(false);
		}
	}
	return((expectedOldLine == oldText->lineCount) && (expectedNewLine == newText->lineCount));
}

/*
	Whether every hunk reads as its removals and then its additions.

	Myers hands back A shortest script and says nothing about the order of the steps inside one, so this is
	not something the algorithm gives - it is something DiffGroupHunks makes, and the side by side view
	depends on it completely: it pairs the n-th removal of a hunk with the n-th addition of it, and over an
	interleaved hunk it would pair the wrong lines with each other and call the rest separate hunks.
*/
static bool DiffScriptHasGroupedHunks(const DiffScript *script) {
	bool sawAnAdditionInThisHunk = false;
	for(int32_t editIndex = 0; editIndex < script->count; ++editIndex) {
		DiffEditKind kind = script->edits[editIndex].kind;
		if(kind == DiffEditKind_Context) {
			sawAnAdditionInThisHunk = false;
			continue;
		}
		if(kind == DiffEditKind_Added) {
			sawAnAdditionInThisHunk = true;
			continue;
		}
		if(sawAnAdditionInThisHunk) {
			return(false);
		}
	}
	return(true);
}

static int32_t DiffCountChanges(const DiffScript *script) {
	int32_t changeCount = 0;
	for(int32_t editIndex = 0; editIndex < script->count; ++editIndex) {
		if(script->edits[editIndex].kind != DiffEditKind_Context) {
			changeCount += 1;
		}
	}
	return(changeCount);
}

//! Joins a list of one character lines into a text, which is what the random checks are built out of
static void DiffBuildRandomText(DiffText *text, DiffRandom *random, const int32_t lineCount, const int32_t alphabetSize) {
	char joined[64];
	int32_t writeOffset = 0;
	for(int32_t lineIndex = 0; lineIndex < lineCount; ++lineIndex) {
		if(lineIndex > 0) {
			joined[writeOffset] = '\n';
			writeOffset += 1;
		}
		joined[writeOffset] = (char)('a' + DiffRandomBelow(random, alphabetSize));
		writeOffset += 1;
	}
	DiffTextInit(text, joined, writeOffset);
}

static void SelfTestCuttingIntoLines(void) {
	CheckSection("cutting a text into lines");

	DiffText text;
	CHECK(DiffTextInit(&text, "alpha\nbeta\ngamma", 16));
	CHECK_I(text.lineCount, 3);
	int32_t lineLength = 0;
	const char *secondLine = DiffTextGetLine(&text, 1, &lineLength);
	CHECK_I(lineLength, 4);
	CHECK(memcmp(secondLine, "beta", 4) == 0);
	DiffTextRelease(&text);

	// A text that ends in a line feed has an empty last line, the way every editor shows one - so a file
	// that grew a line at the end reads as one line added rather than as a change to the last one.
	CHECK(DiffTextInit(&text, "one\ntwo\n", 8));
	CHECK_I(text.lineCount, 3);
	CHECK_I(text.lineLengths[2], 0);
	DiffTextRelease(&text);

	// A carriage return in front of the line feed belongs to the line it ends, exactly as in the editor,
	// so two files that differ only in their line endings do not read as differing in every line.
	DiffText windowsText;
	DiffText unixText;
	CHECK(DiffTextInit(&windowsText, "one\r\ntwo\r\n", 10));
	CHECK(DiffTextInit(&unixText, "one\ntwo\n", 8));
	CHECK_I(windowsText.lineCount, unixText.lineCount);
	CHECK(DiffLinesAreEqual(&windowsText, 0, &unixText, 0));
	CHECK(DiffLinesAreEqual(&windowsText, 1, &unixText, 1));
	DiffTextRelease(&windowsText);
	DiffTextRelease(&unixText);

	CHECK(DiffTextInit(&text, "", 0));
	CHECK_I(text.lineCount, 1);
	CHECK_I(text.lineLengths[0], 0);
	DiffTextRelease(&text);
}

static void SelfTestDifferenceOverTheObvious(void) {
	CheckSection("a difference over what is obvious");

	DiffText oldText;
	DiffText newText;
	DiffScript script;

	// Nothing changed at all.
	CHECK(DiffTextInit(&oldText, "a\nb\nc", 5));
	CHECK(DiffTextInit(&newText, "a\nb\nc", 5));
	CHECK(DiffCompute(&oldText, &newText, &script));
	CHECK_I(script.count, 3);
	CHECK_I(DiffCountChanges(&script), 0);
	CHECK(DiffScriptRebuildsBothTexts(&script, &oldText, &newText));
	DiffScriptRelease(&script);
	DiffTextRelease(&oldText);
	DiffTextRelease(&newText);

	// One line put in the middle.
	CHECK(DiffTextInit(&oldText, "a\nc", 3));
	CHECK(DiffTextInit(&newText, "a\nb\nc", 5));
	CHECK(DiffCompute(&oldText, &newText, &script));
	CHECK_I(script.count, 3);
	CHECK_I(script.edits[1].kind, DiffEditKind_Added);
	CHECK_I(script.edits[1].newLine, 1);
	CHECK(DiffScriptRebuildsBothTexts(&script, &oldText, &newText));
	DiffScriptRelease(&script);
	DiffTextRelease(&oldText);
	DiffTextRelease(&newText);

	// One line taken out of the middle.
	CHECK(DiffTextInit(&oldText, "a\nb\nc", 5));
	CHECK(DiffTextInit(&newText, "a\nc", 3));
	CHECK(DiffCompute(&oldText, &newText, &script));
	CHECK_I(script.count, 3);
	CHECK_I(script.edits[1].kind, DiffEditKind_Removed);
	CHECK_I(script.edits[1].oldLine, 1);
	CHECK(DiffScriptRebuildsBothTexts(&script, &oldText, &newText));
	DiffScriptRelease(&script);
	DiffTextRelease(&oldText);
	DiffTextRelease(&newText);

	// One line replaced, which is a removal and an addition standing next to each other - and in THAT
	// order, because that is what the grouping is for.
	CHECK(DiffTextInit(&oldText, "a\nb\nc", 5));
	CHECK(DiffTextInit(&newText, "a\nx\nc", 5));
	CHECK(DiffCompute(&oldText, &newText, &script));
	CHECK_I(script.count, 4);
	CHECK_I(script.edits[1].kind, DiffEditKind_Removed);
	CHECK_I(script.edits[2].kind, DiffEditKind_Added);
	CHECK(DiffScriptRebuildsBothTexts(&script, &oldText, &newText));
	DiffScriptRelease(&script);
	DiffTextRelease(&oldText);
	DiffTextRelease(&newText);

	// Two texts with nothing whatever in common, which is the case a shortest script is most free to
	// interleave - so it is also the one that says whether the grouping really happened.
	CHECK(DiffTextInit(&oldText, "a\nb", 3));
	CHECK(DiffTextInit(&newText, "x\ny\nz", 5));
	CHECK(DiffCompute(&oldText, &newText, &script));
	CHECK_I(DiffCountChanges(&script), 5);
	CHECK_I(script.edits[0].kind, DiffEditKind_Removed);
	CHECK_I(script.edits[1].kind, DiffEditKind_Removed);
	CHECK_I(script.edits[2].kind, DiffEditKind_Added);
	CHECK(DiffScriptHasGroupedHunks(&script));
	CHECK(DiffScriptRebuildsBothTexts(&script, &oldText, &newText));
	DiffScriptRelease(&script);
	DiffTextRelease(&oldText);
	DiffTextRelease(&newText);

	// An empty text on one side, which is what a file that was just created or just emptied looks like.
	CHECK(DiffTextInit(&oldText, "", 0));
	CHECK(DiffTextInit(&newText, "a\nb\nc", 5));
	CHECK(DiffCompute(&oldText, &newText, &script));
	CHECK(DiffScriptRebuildsBothTexts(&script, &oldText, &newText));
	DiffScriptRelease(&script);
	DiffTextRelease(&oldText);
	DiffTextRelease(&newText);
}

/*
	Thousands of random pairs, against a table that cannot be clever.

	Three things are asked of every one of them: the script rebuilds both texts, it is exactly as short as
	the shortest one there is, and the safety net inside the recursion did not have to catch anything.
*/
static void SelfTestDifferenceAgainstBruteForce(void) {
	CheckSection("as short as the shortest there is");

	DiffRandom random;
	random.state = 0x9E3779B97F4A7C15ull;

	const int32_t pairCount = 3000;
	const int32_t longestText = 12;
	int32_t rebuiltCount = 0;
	int32_t shortestCount = 0;
	int32_t groupedCount = 0;
	int32_t netCaughtCount = 0;
	for(int32_t pairIndex = 0; pairIndex < pairCount; ++pairIndex) {
		int32_t alphabetSize = 2 + DiffRandomBelow(&random, 3);
		int32_t oldLineCount = 1 + DiffRandomBelow(&random, longestText);
		int32_t newLineCount = 1 + DiffRandomBelow(&random, longestText);

		DiffText oldText;
		DiffText newText;
		DiffBuildRandomText(&oldText, &random, oldLineCount, alphabetSize);
		DiffBuildRandomText(&newText, &random, newLineCount, alphabetSize);

		DiffScript script;
		if(DiffCompute(&oldText, &newText, &script)) {
			if(DiffScriptRebuildsBothTexts(&script, &oldText, &newText)) {
				rebuiltCount += 1;
			}
			int32_t bestDistance = DiffBruteForceDistance(&oldText, &newText);
			if(DiffCountChanges(&script) == bestDistance) {
				shortestCount += 1;
			}
			if(DiffScriptHasGroupedHunks(&script)) {
				groupedCount += 1;
			}
			netCaughtCount += script.fallbackCount;
		}
		DiffScriptRelease(&script);
		DiffTextRelease(&oldText);
		DiffTextRelease(&newText);
	}

	CHECK_I(rebuiltCount, pairCount);
	CHECK_I(shortestCount, pairCount);
	CHECK_I(groupedCount, pairCount);
	CHECK_I(netCaughtCount, 0);
}

static void SelfTestRowsOfBothViews(void) {
	CheckSection("the rows the two views are made of");

	DiffText oldText;
	DiffText newText;
	DiffScript script;
	CHECK(DiffTextInit(&oldText, "keep\ngone1\ngone2\nsame\nold", 25));
	CHECK(DiffTextInit(&newText, "keep\nsame\nnew\nplus", 18));
	CHECK(DiffCompute(&oldText, &newText, &script));

	DiffViewRows unifiedRows;
	CHECK(DiffBuildUnifiedRows(&script, &oldText, &newText, &unifiedRows));

	// Every row of the unified view holds a line of one of the two texts, and every line of both texts is
	// held by exactly one row of it - which is what "woven together" has to mean.
	int32_t oldLinesShown = 0;
	int32_t newLinesShown = 0;
	for(int32_t rowIndex = 0; rowIndex < unifiedRows.count; ++rowIndex) {
		const DiffViewRow *row = &unifiedRows.rows[rowIndex];
		if(row->oldLine >= 0) {
			oldLinesShown += 1;
		}
		if(row->newLine >= 0) {
			newLinesShown += 1;
		}
		CHECK_QUIET((row->oldLine >= 0) || (row->newLine >= 0));
	}
	CHECK_I(oldLinesShown, oldText.lineCount);
	CHECK_I(newLinesShown, newText.lineCount);

	DiffViewRows leftRows;
	DiffViewRows rightRows;
	CHECK(DiffBuildSideRows(&script, &oldText, &newText, &leftRows, &rightRows));
	CHECK_I(leftRows.count, rightRows.count);
	CHECK(leftRows.count > 0);

	// The rule the whole side by side view rests on: a row is a line on one side and a line or a filler on
	// the other, never a filler on both, and the two columns never get out of step.
	int32_t leftLinesShown = 0;
	int32_t rightLinesShown = 0;
	int32_t fillerRowCount = 0;
	for(int32_t rowIndex = 0; rowIndex < leftRows.count; ++rowIndex) {
		const DiffViewRow *leftRow = &leftRows.rows[rowIndex];
		const DiffViewRow *rightRow = &rightRows.rows[rowIndex];
		if(leftRow->oldLine >= 0) {
			leftLinesShown += 1;
		}
		if(rightRow->newLine >= 0) {
			rightLinesShown += 1;
		}
		bool bothAreFillers = (leftRow->kind == DiffRowKind_Filler) && (rightRow->kind == DiffRowKind_Filler);
		CHECK_QUIET(!bothAreFillers);
		if(leftRow->kind == DiffRowKind_Filler || rightRow->kind == DiffRowKind_Filler) {
			fillerRowCount += 1;
		}
	}
	CHECK_I(leftLinesShown, oldText.lineCount);
	CHECK_I(rightLinesShown, newText.lineCount);
	CHECK(fillerRowCount > 0);

	/*
		Both numbers stand in both gutters, so the two read the same all the way down - and a filler leaves
		the column of the file it stands IN blank while the other column still says which line it is
		standing opposite. A blank on both sides would say nothing at all.
	*/
	for(int32_t rowIndex = 0; rowIndex < leftRows.count; ++rowIndex) {
		const DiffViewRow *leftRow = &leftRows.rows[rowIndex];
		const DiffViewRow *rightRow = &rightRows.rows[rowIndex];
		CHECK_QUIET(leftRow->gutterFirst == rightRow->gutterFirst);
		CHECK_QUIET(leftRow->gutterSecond == rightRow->gutterSecond);
		if(leftRow->kind == DiffRowKind_Filler) {
			CHECK_QUIET(leftRow->gutterFirst < 0);
			CHECK_QUIET(leftRow->gutterSecond >= 0);
		}
		if(rightRow->kind == DiffRowKind_Filler) {
			CHECK_QUIET(rightRow->gutterSecond < 0);
			CHECK_QUIET(rightRow->gutterFirst >= 0);
		}
	}

	DiffViewRowsRelease(&unifiedRows);
	DiffViewRowsRelease(&leftRows);
	DiffViewRowsRelease(&rightRows);
	DiffScriptRelease(&script);
	DiffTextRelease(&oldText);
	DiffTextRelease(&newText);
}

static void SelfTestSideRowsStayInStep(void) {
	CheckSection("both sides keep the same number of rows");

	DiffRandom random;
	random.state = 0xD1B54A32D192ED03ull;

	const int32_t pairCount = 600;
	const int32_t longestText = 14;
	const int32_t contextLineCount = 2;
	int32_t inStepCount = 0;
	int32_t inStepAfterCollapsingCount = 0;
	for(int32_t pairIndex = 0; pairIndex < pairCount; ++pairIndex) {
		int32_t alphabetSize = 2 + DiffRandomBelow(&random, 3);
		DiffText oldText;
		DiffText newText;
		DiffBuildRandomText(&oldText, &random, 1 + DiffRandomBelow(&random, longestText), alphabetSize);
		DiffBuildRandomText(&newText, &random, 1 + DiffRandomBelow(&random, longestText), alphabetSize);

		DiffScript script;
		DiffViewRows leftRows;
		DiffViewRows rightRows;
		if(DiffCompute(&oldText, &newText, &script) && DiffBuildSideRows(&script, &oldText, &newText, &leftRows, &rightRows)) {
			if(leftRows.count == rightRows.count) {
				inStepCount += 1;
			}
			if(DiffCollapseContext(&leftRows, &rightRows, contextLineCount) && (leftRows.count == rightRows.count)) {
				inStepAfterCollapsingCount += 1;
			}
			DiffViewRowsRelease(&leftRows);
			DiffViewRowsRelease(&rightRows);
		}
		DiffScriptRelease(&script);
		DiffTextRelease(&oldText);
		DiffTextRelease(&newText);
	}

	CHECK_I(inStepCount, pairCount);
	CHECK_I(inStepAfterCollapsingCount, pairCount);
}

static void SelfTestCollapsingContext(void) {
	CheckSection("collapsing the lines nobody reads");

	DiffText oldText;
	DiffText newText;
	DiffScript script;
	CHECK(DiffTextInit(&oldText, "1\n2\n3\n4\n5\n6\n7\n8\n9\nX", 20));
	CHECK(DiffTextInit(&newText, "1\n2\n3\n4\n5\n6\n7\n8\n9\nY", 20));
	CHECK(DiffCompute(&oldText, &newText, &script));

	DiffViewRows rows;
	CHECK(DiffBuildUnifiedRows(&script, &oldText, &newText, &rows));
	CHECK_I(rows.count, 11);

	// Two lines of context on either side of the one change, and one band standing for the seven that are
	// left out. The band carries no number, so the jump in the numbers says how much is missing.
	const int32_t contextLineCount = 2;
	const DiffViewRows *noSecondSide = fpl_null;
	CHECK(DiffCollapseContext(&rows, (DiffViewRows *)noSecondSide, contextLineCount));
	CHECK_I(rows.count, 5);
	CHECK_I(rows.rows[0].kind, DiffRowKind_Skipped);
	CHECK_I(rows.rows[0].gutterFirst, -1);
	CHECK_I(rows.rows[1].kind, DiffRowKind_Context);
	CHECK_I(rows.rows[2].kind, DiffRowKind_Context);
	CHECK_I(rows.rows[3].kind, DiffRowKind_Removed);
	CHECK_I(rows.rows[4].kind, DiffRowKind_Added);

	DiffViewRowsRelease(&rows);
	DiffScriptRelease(&script);
	DiffTextRelease(&oldText);
	DiffTextRelease(&newText);
}

static void SelfTestChangedSpanInsideALine(void) {
	CheckSection("the piece of a line that changed");

	int32_t oldStart = 0;
	int32_t oldEnd = 0;
	int32_t newStart = 0;
	int32_t newEnd = 0;

	const char *oldLine = "int value = 10;";
	const char *newLine = "int value = 42;";
	DiffFindChangedSpan(oldLine, 15, newLine, 15, &oldStart, &oldEnd, &newStart, &newEnd);
	CHECK_I(oldStart, 12);
	CHECK_I(oldEnd, 14);
	CHECK_I(newStart, 12);
	CHECK_I(newEnd, 14);

	// A line that only grew at the end has an empty span on the side that did not.
	DiffFindChangedSpan("abc", 3, "abcdef", 6, &oldStart, &oldEnd, &newStart, &newEnd);
	CHECK_I(oldStart, 3);
	CHECK_I(oldEnd, 3);
	CHECK_I(newStart, 3);
	CHECK_I(newEnd, 6);

	// Two lines that are the same have no span at all, which is what keeps a wash off a line that only
	// MOVED rather than changed.
	DiffFindChangedSpan("same", 4, "same", 4, &oldStart, &oldEnd, &newStart, &newEnd);
	CHECK_I(oldStart, 4);
	CHECK_I(oldEnd, 4);

	/*
		Both ends are pulled back onto a character boundary. These two differ in the SECOND byte of a two
		byte character - a span that started there would wash half a glyph, and the editor measures a
		prefix that ends inside a character as a prefix that is not there.
	*/
	const char *oldUmlaut = "x\xC3\xA4y";
	const char *newUmlaut = "x\xC3\xB6y";
	DiffFindChangedSpan(oldUmlaut, 4, newUmlaut, 4, &oldStart, &oldEnd, &newStart, &newEnd);
	CHECK_I(oldStart, 1);
	CHECK_I(oldEnd, 3);
	CHECK_I(newStart, 1);
	CHECK_I(newEnd, 3);
}

// ----------------------------------------------------------------------------
// > Where the two texts come from
// ----------------------------------------------------------------------------

//! The built in pair, small enough that every case in it can be seen at once
static const char *DemoSampleOldText =
	"// A tiny scene graph, before\n"
	"#include \"scene.h\"\n"
	"\n"
	"void SceneInit(Scene *scene) {\n"
	"\tscene->nodeCount = 0;\n"
	"\tscene->capacity = 16;\n"
	"\tscene->nodes = malloc(sizeof(Node) * 16);\n"
	"}\n"
	"\n"
	"Node *SceneAddNode(Scene *scene, const char *name) {\n"
	"\tif(scene->nodeCount == scene->capacity) {\n"
	"\t\treturn NULL;\n"
	"\t}\n"
	"\tNode *node = &scene->nodes[scene->nodeCount++];\n"
	"\tnode->name = name;\n"
	"\tnode->parent = NULL;\n"
	"\treturn node;\n"
	"}\n"
	"\n"
	"void SceneRelease(Scene *scene) {\n"
	"\tfree(scene->nodes);\n"
	"}\n";

static const char *DemoSampleNewText =
	"// A tiny scene graph, after\n"
	"#include \"scene.h\"\n"
	"#include <assert.h>\n"
	"\n"
	"void SceneInit(Scene *scene, int capacity) {\n"
	"\tassert(capacity > 0);\n"
	"\tscene->nodeCount = 0;\n"
	"\tscene->capacity = capacity;\n"
	"\tscene->nodes = malloc(sizeof(Node) * capacity);\n"
	"}\n"
	"\n"
	"Node *SceneAddNode(Scene *scene, const char *name) {\n"
	"\tif(scene->nodeCount == scene->capacity) {\n"
	"\t\tSceneGrow(scene);\n"
	"\t}\n"
	"\tNode *node = &scene->nodes[scene->nodeCount++];\n"
	"\tnode->name = name;\n"
	"\tnode->parent = NULL;\n"
	"\tnode->flags = NodeFlags_Dirty;\n"
	"\treturn node;\n"
	"}\n"
	"\n"
	"void SceneRelease(Scene *scene) {\n"
	"\tfree(scene->nodes);\n"
	"\tscene->nodes = NULL;\n"
	"}\n";

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

/*
	Finds final_ui.h from wherever the working directory happens to be.

	The build puts the binary anywhere between the repository root and four levels under it depending on
	the generator, and the big case reads the repository's own source.
*/
static bool DemoReadSourceFile(uint8_t **outData, int32_t *outLength, const char **outPath) {
	static const char *candidatePaths[] = {
		DEMO_SOURCE_FILE_PATH,
		"../" DEMO_SOURCE_FILE_PATH,
		"../../" DEMO_SOURCE_FILE_PATH,
		"../../../" DEMO_SOURCE_FILE_PATH,
		"../../../../" DEMO_SOURCE_FILE_PATH,
	};

	*outData = fpl_null;
	*outLength = 0;
	*outPath = DEMO_SOURCE_FILE_PATH;
	size_t candidateIndex = 0;
	while(candidateIndex < fplArrayCount(candidatePaths)) {
		if(DemoReadWholeFile(candidatePaths[candidateIndex], outData, outLength)) {
			*outPath = candidatePaths[candidateIndex];
			return(true);
		}
		candidateIndex += 1;
	}
	return(false);
}

/*
	Makes a second version of a text by writing into it in a handful of places.

	The point of the big case is a diff over fourteen thousand lines that changes nine of them, because
	that is the shape a diff really has - and it is also the shape that says whether an algorithm costs the
	file or costs the change.
*/
static bool DemoBuildEditedCopy(const DiffText *sourceText, DiffText *outEditedText) {
	int32_t lineCount = sourceText->lineCount;
	if(lineCount < 200) {
		return(false);
	}

	// Written as line numbers so the edits land in the same places whatever the file grows to.
	const int32_t deletedFirstLine = lineCount / 4;
	const int32_t deletedLineCount = 3;
	const int32_t rewrittenLine = lineCount / 2;
	const int32_t insertedBeforeLine = (lineCount * 3) / 4;
	const char *insertedLines[] = {
		"// A line that was not in the file before.",
		"// And a second one right behind it.",
	};

	int32_t wantedCapacity = sourceText->byteCount + 4096;
	char *editedBytes = (char *)malloc((size_t)wantedCapacity);
	if(editedBytes == fpl_null) {
		return(false);
	}

	int32_t writeOffset = 0;
	for(int32_t lineIndex = 0; lineIndex < lineCount; ++lineIndex) {
		bool isDeleted = (lineIndex >= deletedFirstLine) && (lineIndex < (deletedFirstLine + deletedLineCount));
		if(isDeleted) {
			continue;
		}

		if(lineIndex == insertedBeforeLine) {
			for(size_t insertedIndex = 0; insertedIndex < fplArrayCount(insertedLines); ++insertedIndex) {
				int32_t insertedLength = (int32_t)strlen(insertedLines[insertedIndex]);
				memcpy(&editedBytes[writeOffset], insertedLines[insertedIndex], (size_t)insertedLength);
				writeOffset += insertedLength;
				editedBytes[writeOffset] = '\n';
				writeOffset += 1;
			}
		}

		int32_t lineLength = 0;
		const char *lineBytes = DiffTextGetLine(sourceText, lineIndex, &lineLength);
		memcpy(&editedBytes[writeOffset], lineBytes, (size_t)lineLength);
		writeOffset += lineLength;

		// One line that is CHANGED rather than added or removed, so the two halves of it can be set beside
		// each other and the piece that really differs marked inside them.
		if(lineIndex == rewrittenLine) {
			const char *addedTail = "   // and something written at the end of this one";
			int32_t addedLength = (int32_t)strlen(addedTail);
			memcpy(&editedBytes[writeOffset], addedTail, (size_t)addedLength);
			writeOffset += addedLength;
		}

		bool needsALineFeed = lineIndex < (lineCount - 1);
		if(needsALineFeed) {
			editedBytes[writeOffset] = '\n';
			writeOffset += 1;
		}
	}

	bool didInit = DiffTextInit(outEditedText, editedBytes, writeOffset);
	free(editedBytes);
	return(didInit);
}

static void SelfTestARealFileAgainstAnEditedCopy(void) {
	CheckSection("a real file against an edited copy");

	uint8_t *fileData = fpl_null;
	int32_t fileLength = 0;
	const char *filePath = fpl_null;
	if(!DemoReadSourceFile(&fileData, &fileLength, &filePath)) {
		printf("  skipped, %s was not found from here\n", DEMO_SOURCE_FILE_PATH);
		return;
	}

	DiffText oldText;
	DiffText newText;
	CHECK(DiffTextInit(&oldText, (const char *)fileData, fileLength));
	free(fileData);
	CHECK(oldText.lineCount > 14000);
	CHECK(DemoBuildEditedCopy(&oldText, &newText));

	DiffScript script;
	CHECK(DiffCompute(&oldText, &newText, &script));
	CHECK(DiffScriptRebuildsBothTexts(&script, &oldText, &newText));
	CHECK(DiffScriptHasGroupedHunks(&script));
	CHECK_I(script.fallbackCount, 0);

	// Three lines out, two in, and one rewritten - which is a removal and an addition of its own. Seven
	// changed lines out of fourteen thousand, and not one more. A diff that walked the whole FILE rather
	// than the change would still be correct here; one that is not minimal would not.
	CHECK_I(DiffCountChanges(&script), 7);

	DiffViewRows leftRows;
	DiffViewRows rightRows;
	CHECK(DiffBuildSideRows(&script, &oldText, &newText, &leftRows, &rightRows));
	CHECK_I(leftRows.count, rightRows.count);

	// Fourteen thousand rows down to a couple of dozen, which is what makes a diff of a big file readable.
	const int32_t contextLineCount = 3;
	CHECK(DiffCollapseContext(&leftRows, &rightRows, contextLineCount));
	CHECK_I(leftRows.count, rightRows.count);
	CHECK(leftRows.count < 40);
	CHECK(leftRows.count > 10);

	DiffViewRowsRelease(&leftRows);
	DiffViewRowsRelease(&rightRows);
	DiffScriptRelease(&script);
	DiffTextRelease(&oldText);
	DiffTextRelease(&newText);
}

// ----------------------------------------------------------------------------
// > Demo
// ----------------------------------------------------------------------------

//! Which of the two ways of showing the same difference is on screen
typedef enum DiffViewMode {
	//! One editor holding both texts woven together
	DiffViewMode_Unified = 0,
	//! Two editors with exactly as many rows as each other
	DiffViewMode_SideBySide,
	//! How many there are
	DiffViewMode_Count,
} DiffViewMode;

typedef struct DiffDemoState {
	//! The two texts, as lines
	DiffText oldText;
	DiffText newText;
	//! Whether there are two texts at all
	bool hasTexts;
	//! What the difference between them is
	DiffScript script;

	//! The one editor of the unified view
	DiffPane unifiedPane;
	//! And the two of the side by side one
	DiffPane leftPane;
	DiffPane rightPane;

	//! Which view is on screen
	DiffViewMode mode;
	//! How many unchanged lines are kept around a change, or -1 for all of them
	int32_t contextLineCount;
	//! Whether the piece of a changed line that really differs is marked inside it
	bool marksInsideLines;
	//! Whether a bar is drawn at the left edge of the gutter beside a changed line
	bool showsGutterBars;
	//! Whether the side by side view uses red and green as well, rather than blue for what was there
	bool usesRedAndGreenOnBothSides;
	//! Whether blanks and tabs are drawn
	bool showsWhitespace;
	//! Whether the line the caret is on is washed, which fights the diff colours and is off by default
	bool highlightsCurrentLine;

	//! Where the two texts came from
	char sourceDescription[512];
	//! What the difference came to
	char summaryDescription[256];

	//! The proportional face the interface is drawn with
	const fuiFont *uiFont;
	//! The monospace face the two texts are shown in
	const fuiFont *monoFont;
	//! Whether the window is still open
	bool isRunning;
} DiffDemoState;

static fuiEditorConfig DemoMakePaneConfig(const DiffDemoState *demo) {
	fuiEditorConfig config = fuiEditorDefaultConfig();

	// A diff is READ, never written into - and the widget's own read only toggle is the one gate every
	// writing branch of it goes through, so there is nothing else to switch off.
	config.toggles.isReadOnly = true;
	config.toggles.isInteractive = true;
	config.toggles.showLineNumbers = true;
	config.toggles.showStatusBar = false;
	config.toggles.highlightCurrentLine = demo->highlightsCurrentLine;
	config.toggles.showWhitespace = demo->showsWhitespace;
	config.toggles.showLineEndings = false;

	// Searching a diff is worth having; replacing in one is not, and neither is jumping to a line number
	// that is a ROW of the view rather than a line of either file.
	config.toggles.canFind = true;
	config.toggles.canReplace = false;
	config.toggles.canGoToLine = false;
	return(config);
}

static void DemoApplyPaneConfig(DiffDemoState *demo) {
	fuiEditorConfig config = DemoMakePaneConfig(demo);

	DiffPane *panes[3];
	panes[0] = &demo->unifiedPane;
	panes[1] = &demo->leftPane;
	panes[2] = &demo->rightPane;
	for(size_t paneIndex = 0; paneIndex < fplArrayCount(panes); ++paneIndex) {
		DiffPane *pane = panes[paneIndex];

		// Everything about the LOOK comes from here; the two fields that describe the pane's own gutter are
		// the pane's and are left where DiffPaneFill put them.
		int32_t keptGutterWidth = pane->config.metrics.gutterMinDigits;
		config.metrics.gutterMinDigits = keptGutterWidth;
		config.callbacks.formatGutterText = DiffFormatGutterText;
		config.callbacks.userData = pane;
		pane->config = config;
		fuiEditorSetConfig(&pane->editor, &pane->config);
	}
}

/*
	Builds both views out of the one difference.

	Both of them are built whichever is on screen, because switching between them is a button and a view
	that is built when it is asked for would rebuild a fourteen thousand line document under the click.
*/
static void DemoRebuildViews(DiffDemoState *demo) {
	if(!demo->hasTexts) {
		return;
	}

	DiffPalette unifiedPalette = DiffUnifiedPalette();
	DiffPalette sidePalette = demo->usesRedAndGreenOnBothSides ? DiffUnifiedPalette() : DiffSidePalette();

	DiffViewRows unifiedRows;
	if(DiffBuildUnifiedRows(&demo->script, &demo->oldText, &demo->newText, &unifiedRows)) {
		const DiffViewRows *noSecondSide = fpl_null;
		(void)DiffCollapseContext(&unifiedRows, (DiffViewRows *)noSecondSide, demo->contextLineCount);
		const bool hasTwoGutterColumns = true;
		(void)DiffPaneFill(&demo->unifiedPane, &unifiedRows, &demo->oldText, &demo->newText, &unifiedPalette, demo->marksInsideLines, demo->showsGutterBars, hasTwoGutterColumns);
	}
	DiffViewRowsRelease(&unifiedRows);

	DiffViewRows leftRows;
	DiffViewRows rightRows;
	if(DiffBuildSideRows(&demo->script, &demo->oldText, &demo->newText, &leftRows, &rightRows)) {
		(void)DiffCollapseContext(&leftRows, &rightRows, demo->contextLineCount);

		// Both columns in both panes, so the two gutters read the same all the way down rather than
		// drifting apart the moment something is inserted above.
		const bool hasTwoGutterColumns = true;
		(void)DiffPaneFill(&demo->leftPane, &leftRows, &demo->oldText, &demo->newText, &sidePalette, demo->marksInsideLines, demo->showsGutterBars, hasTwoGutterColumns);
		(void)DiffPaneFill(&demo->rightPane, &rightRows, &demo->oldText, &demo->newText, &sidePalette, demo->marksInsideLines, demo->showsGutterBars, hasTwoGutterColumns);
	}
	DiffViewRowsRelease(&leftRows);
	DiffViewRowsRelease(&rightRows);

	DemoApplyPaneConfig(demo);

	int32_t removedCount = 0;
	int32_t addedCount = 0;
	for(int32_t editIndex = 0; editIndex < demo->script.count; ++editIndex) {
		if(demo->script.edits[editIndex].kind == DiffEditKind_Removed) {
			removedCount += 1;
		} else if(demo->script.edits[editIndex].kind == DiffEditKind_Added) {
			addedCount += 1;
		}
	}
	fplStringFormat(demo->summaryDescription, fplArrayCount(demo->summaryDescription), "%d lines gone, %d lines come, over %d and %d lines - %d rows in the unified view, %d in each pane of the other", (int)removedCount, (int)addedCount, (int)demo->oldText.lineCount, (int)demo->newText.lineCount, (int)demo->unifiedPane.rowCount, (int)demo->leftPane.rowCount);
}

/*
	Takes on two texts and works out the difference between them.

	The two are cut into their lines BEFORE the old pair is let go, because one of the callers hands this
	the very texts it is already holding: swapping them round is a call with the two the other way about,
	and letting go first would read from what had just been freed.
*/
static void DemoSetTexts(DiffDemoState *demo, const char *oldBytes, const int32_t oldLength, const char *newBytes, const int32_t newLength, const char *description) {
	DiffText cutOldText;
	DiffText cutNewText;
	bool bothWereCut = DiffTextInit(&cutOldText, oldBytes, oldLength) && DiffTextInit(&cutNewText, newBytes, newLength);

	DiffScriptRelease(&demo->script);
	DiffTextRelease(&demo->oldText);
	DiffTextRelease(&demo->newText);
	demo->hasTexts = false;

	if(!bothWereCut) {
		DiffTextRelease(&cutOldText);
		DiffTextRelease(&cutNewText);
		fplCopyString("Could not read the two texts", demo->sourceDescription, fplArrayCount(demo->sourceDescription));
		return;
	}
	demo->oldText = cutOldText;
	demo->newText = cutNewText;
	if(!DiffCompute(&demo->oldText, &demo->newText, &demo->script)) {
		fplCopyString("Ran out of memory while working out the difference", demo->sourceDescription, fplArrayCount(demo->sourceDescription));
		return;
	}

	demo->hasTexts = true;
	fplCopyString(description, demo->sourceDescription, fplArrayCount(demo->sourceDescription));
	DemoRebuildViews(demo);
}

static void DemoLoadTheSample(DiffDemoState *demo) {
	int32_t oldLength = (int32_t)strlen(DemoSampleOldText);
	int32_t newLength = (int32_t)strlen(DemoSampleNewText);
	DemoSetTexts(demo, DemoSampleOldText, oldLength, DemoSampleNewText, newLength, "The built in sample: a small scene graph, before and after");
}

static void DemoLoadTheBigOne(DiffDemoState *demo) {
	uint8_t *fileData = fpl_null;
	int32_t fileLength = 0;
	const char *filePath = fpl_null;
	if(!DemoReadSourceFile(&fileData, &fileLength, &filePath)) {
		fplCopyString("Not found: " DEMO_SOURCE_FILE_PATH " - run the demo from the repository root", demo->sourceDescription, fplArrayCount(demo->sourceDescription));
		return;
	}

	DiffText sourceText;
	if(!DiffTextInit(&sourceText, (const char *)fileData, fileLength)) {
		free(fileData);
		return;
	}
	free(fileData);

	DiffText editedText;
	if(!DemoBuildEditedCopy(&sourceText, &editedText)) {
		DiffTextRelease(&sourceText);
		return;
	}

	char description[512];
	fplStringFormat(description, fplArrayCount(description), "%s (%d lines) against a copy with three lines taken out, two put in and one written over", filePath, (int)sourceText.lineCount);
	DemoSetTexts(demo, sourceText.bytes, sourceText.byteCount, editedText.bytes, editedText.byteCount, description);

	DiffTextRelease(&sourceText);
	DiffTextRelease(&editedText);
}

static bool DemoLoadTwoFiles(DiffDemoState *demo, const char *oldPath, const char *newPath) {
	uint8_t *oldData = fpl_null;
	int32_t oldLength = 0;
	uint8_t *newData = fpl_null;
	int32_t newLength = 0;
	bool bothWereRead = DemoReadWholeFile(oldPath, &oldData, &oldLength) && DemoReadWholeFile(newPath, &newData, &newLength);
	if(!bothWereRead) {
		free(oldData);
		free(newData);
		fplStringFormat(demo->sourceDescription, fplArrayCount(demo->sourceDescription), "Could not read both of %s and %s", oldPath, newPath);
		return(false);
	}

	char description[512];
	fplStringFormat(description, fplArrayCount(description), "%s against %s", oldPath, newPath);
	DemoSetTexts(demo, (const char *)oldData, oldLength, (const char *)newData, newLength, description);
	free(oldData);
	free(newData);
	return(true);
}

static bool DemoInit(DiffDemoState *demo) {
	memset(demo, 0, sizeof(*demo));
	demo->isRunning = true;
	demo->mode = DiffViewMode_Unified;
	demo->contextLineCount = 3;
	demo->marksInsideLines = true;
	demo->showsGutterBars = false;
	demo->usesRedAndGreenOnBothSides = false;
	demo->showsWhitespace = false;
	demo->highlightsCurrentLine = false;

	bool everyPaneIsReady = fuiEditorInit(&demo->unifiedPane.editor, fpl_null) && fuiEditorInit(&demo->leftPane.editor, fpl_null) && fuiEditorInit(&demo->rightPane.editor, fpl_null);
	demo->unifiedPane.isReady = true;
	demo->leftPane.isReady = true;
	demo->rightPane.isReady = true;
	if(!everyPaneIsReady) {
		return(false);
	}

	fuiEditorConfig config = DemoMakePaneConfig(demo);
	demo->unifiedPane.config = config;
	demo->leftPane.config = config;
	demo->rightPane.config = config;
	return(true);
}

static void DemoRelease(DiffDemoState *demo) {
	DiffPaneRelease(&demo->unifiedPane);
	DiffPaneRelease(&demo->leftPane);
	DiffPaneRelease(&demo->rightPane);
	DiffScriptRelease(&demo->script);
	DiffTextRelease(&demo->oldText);
	DiffTextRelease(&demo->newText);
}

/*
	Keeps the two panes of the side by side view on the same row.

	Whichever of the two MOVED this frame is the one that says where both of them are - and it is read
	after the build rather than before it, because that is when the wheel, the scrollbar and the keyboard
	have all had their say. The other one catches up on the next frame, which at sixty of them a second is
	not a thing anybody sees.
*/
static void DemoKeepThePanesTogether(DiffDemoState *demo, const float leftBeforeX, const float leftBeforeY, const float rightBeforeX, const float rightBeforeY) {
	float leftAfterX = 0.0f;
	float leftAfterY = 0.0f;
	float rightAfterX = 0.0f;
	float rightAfterY = 0.0f;
	fuiEditorGetScrollOffset(&demo->leftPane.editor, &leftAfterX, &leftAfterY);
	fuiEditorGetScrollOffset(&demo->rightPane.editor, &rightAfterX, &rightAfterY);

	bool theLeftOneMoved = (leftAfterX != leftBeforeX) || (leftAfterY != leftBeforeY);
	bool theRightOneMoved = (rightAfterX != rightBeforeX) || (rightAfterY != rightBeforeY);
	if(theLeftOneMoved) {
		fuiEditorSetScrollOffset(&demo->rightPane.editor, leftAfterX, leftAfterY);
	} else if(theRightOneMoved) {
		fuiEditorSetScrollOffset(&demo->leftPane.editor, rightAfterX, rightAfterY);
	}
}

/*
	The whole interface: a few rows of buttons, and then the view.

	Both views are the plain editor widget with decorations hung on it and a gutter callback in front of
	its line numbers. There is no diff mode in the add-on and there is no second widget - which is the one
	thing this demo is here to show.
*/
static void BuildUserInterface(fuiContext *ui, DiffDemoState *demo) {
	const float panelPadding = 16.0f;
	const float rowHeight = 30.0f;
	const float rowSpacing = 6.0f;
	const float buttonWidth = 120.0f;
	const float wideButtonWidth = 300.0f;
	const float toggleWidth = 200.0f;
	const float paneGap = 10.0f;
	const int32_t mostContextLines = 20;

	const fuiDrawData *drawData = fuiGetDrawData(ui);
	float windowWidth = (float)drawData->windowSize.x;
	float windowHeight = (float)drawData->windowSize.y;
	float panelWidth = windowWidth - panelPadding * 2.0f;
	float panelHeight = windowHeight - panelPadding * 2.0f;

	if(!fuiBeginPanel(ui, "Difference", fuiDock_None, panelPadding, panelPadding, panelWidth, panelHeight)) {
		return;
	}

	fuiRect sourceRow = fuiLayoutSlot(ui, rowHeight);
	fuiLabel(ui, sourceRow, demo->sourceDescription);

	fuiRect summaryRow = fuiLayoutSlot(ui, rowHeight);
	fuiLabel(ui, summaryRow, demo->summaryDescription);

	bool viewsNeedRebuilding = false;
	bool configurationChanged = false;

	fuiRect modeRow = fuiLayoutSlot(ui, rowHeight);
	fuiBeginStackAt(ui, "mode", fuiAxis_Horizontal, modeRow, rowSpacing);
	{
		const char *modeName = (demo->mode == DiffViewMode_Unified) ? "Unified, one editor" : "Side by side, two editors";
		char modeButtonLabel[128];
		fplStringFormat(modeButtonLabel, fplArrayCount(modeButtonLabel), "View: %s", modeName);

		fuiRect modeRect = fuiLayoutSlot(ui, wideButtonWidth);
		if(fuiButton(ui, modeRect, modeButtonLabel)) {
			int32_t nextMode = ((int32_t)demo->mode + 1) % (int32_t)DiffViewMode_Count;
			demo->mode = (DiffViewMode)nextMode;
		}

		fuiRect sampleRect = fuiLayoutSlot(ui, wideButtonWidth / 1.5f);
		if(fuiButton(ui, sampleRect, "Load the sample")) {
			DemoLoadTheSample(demo);
		}

		fuiRect bigRect = fuiLayoutSlot(ui, wideButtonWidth);
		if(fuiButton(ui, bigRect, "Load " DEMO_SOURCE_FILE_PATH " against an edited copy")) {
			DemoLoadTheBigOne(demo);
		}

		fuiRect swapRect = fuiLayoutSlot(ui, wideButtonWidth / 1.5f);
		if(fuiButton(ui, swapRect, "Swap the two")) {
			DemoSetTexts(demo, demo->newText.bytes, demo->newText.byteCount, demo->oldText.bytes, demo->oldText.byteCount, "The same two texts, the other way round");
		}
	}
	fuiEndStack(ui);

	fuiRect toggleRow = fuiLayoutSlot(ui, rowHeight);
	fuiBeginStackAt(ui, "toggles", fuiAxis_Horizontal, toggleRow, rowSpacing);
	{
		char contextButtonLabel[64];
		if(demo->contextLineCount < 0) {
			fplCopyString("Context: all", contextButtonLabel, fplArrayCount(contextButtonLabel));
		} else {
			fplStringFormat(contextButtonLabel, fplArrayCount(contextButtonLabel), "Context: %d lines", (int)demo->contextLineCount);
		}

		fuiRect contextDownRect = fuiLayoutSlot(ui, buttonWidth / 1.5f);
		if(fuiButtonRepeat(ui, contextDownRect, "Less")) {
			if(demo->contextLineCount > 0) {
				demo->contextLineCount -= 1;
				viewsNeedRebuilding = true;
			}
		}

		fuiRect contextLabelRect = fuiLayoutSlot(ui, buttonWidth + rowSpacing);
		fuiLabel(ui, contextLabelRect, contextButtonLabel);

		fuiRect contextUpRect = fuiLayoutSlot(ui, buttonWidth / 1.5f);
		if(fuiButtonRepeat(ui, contextUpRect, "More")) {
			if(demo->contextLineCount >= 0 && demo->contextLineCount < mostContextLines) {
				demo->contextLineCount += 1;
				viewsNeedRebuilding = true;
			}
		}

		fuiRect allContextRect = fuiLayoutSlot(ui, toggleWidth);
		bool showsEveryLine = demo->contextLineCount < 0;
		if(fuiCheckbox(ui, allContextRect, "Every line", &showsEveryLine)) {
			demo->contextLineCount = showsEveryLine ? -1 : 3;
			viewsNeedRebuilding = true;
		}

		fuiRect insideRect = fuiLayoutSlot(ui, toggleWidth + toggleWidth / 2.0f);
		if(fuiCheckbox(ui, insideRect, "Mark what changed inside a line", &demo->marksInsideLines)) {
			viewsNeedRebuilding = true;
		}

		fuiRect gutterBarRect = fuiLayoutSlot(ui, toggleWidth);
		if(fuiCheckbox(ui, gutterBarRect, "Gutter bars", &demo->showsGutterBars)) {
			viewsNeedRebuilding = true;
		}
	}
	fuiEndStack(ui);

	fuiRect lookRow = fuiLayoutSlot(ui, rowHeight);
	fuiBeginStackAt(ui, "look", fuiAxis_Horizontal, lookRow, rowSpacing);
	{
		fuiRect redGreenRect = fuiLayoutSlot(ui, toggleWidth + toggleWidth / 2.0f);
		if(fuiCheckbox(ui, redGreenRect, "Red and green on both sides", &demo->usesRedAndGreenOnBothSides)) {
			viewsNeedRebuilding = true;
		}

		fuiRect whitespaceRect = fuiLayoutSlot(ui, toggleWidth);
		if(fuiCheckbox(ui, whitespaceRect, "Whitespace", &demo->showsWhitespace)) {
			configurationChanged = true;
		}

		fuiRect currentLineRect = fuiLayoutSlot(ui, toggleWidth);
		if(fuiCheckbox(ui, currentLineRect, "Current line", &demo->highlightsCurrentLine)) {
			configurationChanged = true;
		}

		fuiRect noteRect = fuiLayoutRemaining(ui);
		const char *noteText = "Blue was there, green is there now, grey is not a line at all";
		if(demo->mode == DiffViewMode_Unified) {
			noteText = "Red went, green came - the two number columns say which";
		} else if(demo->usesRedAndGreenOnBothSides) {
			noteText = "Red went, green came, grey is not a line at all";
		}
		fuiLabel(ui, noteRect, noteText);
	}
	fuiEndStack(ui);

	if(viewsNeedRebuilding) {
		DemoRebuildViews(demo);
	} else if(configurationChanged) {
		DemoApplyPaneConfig(demo);
	}

	fuiRect viewRect = fuiLayoutRemaining(ui);
	fuiSetFont(ui, demo->monoFont);
	if(demo->mode == DiffViewMode_Unified) {
		(void)fuiTextEditor(ui, viewRect, "unified", &demo->unifiedPane.editor);
	} else {
		float halfWidth = (viewRect.w - paneGap) * 0.5f;
		fuiRect leftRect = fuiRectMake(viewRect.x, viewRect.y, halfWidth, viewRect.h);
		fuiRect rightRect = fuiRectMake(viewRect.x + halfWidth + paneGap, viewRect.y, viewRect.w - halfWidth - paneGap, viewRect.h);

		float leftBeforeX = 0.0f;
		float leftBeforeY = 0.0f;
		float rightBeforeX = 0.0f;
		float rightBeforeY = 0.0f;
		fuiEditorGetScrollOffset(&demo->leftPane.editor, &leftBeforeX, &leftBeforeY);
		fuiEditorGetScrollOffset(&demo->rightPane.editor, &rightBeforeX, &rightBeforeY);

		(void)fuiTextEditor(ui, leftRect, "before", &demo->leftPane.editor);
		(void)fuiTextEditor(ui, rightRect, "after", &demo->rightPane.editor);

		DemoKeepThePanesTogether(demo, leftBeforeX, leftBeforeY, rightBeforeX, rightBeforeY);
	}
	fuiSetFont(ui, demo->uiFont);

	fuiEndPanel(ui);
}

/*
	The two panes, checked as a pair rather than one at a time.

	Everything the side by side view promises is a relation BETWEEN the two: the same number of rows, the
	same width of gutter so the two columns of code line up, a number on one side wherever the other has a
	filler, and never a filler on both. None of that can be seen by looking at one pane.
*/
static void SelfTestTheTwoPanesAgree(void) {
	CheckSection("the two panes agree with each other");

	DiffDemoState demo;
	CHECK(DemoInit(&demo));
	DemoLoadTheSample(&demo);
	CHECK(demo.hasTexts);

	CHECK_I(demo.leftPane.rowCount, demo.rightPane.rowCount);
	CHECK_I(fuiEditorGetLineCount(&demo.leftPane.editor), fuiEditorGetLineCount(&demo.rightPane.editor));
	CHECK_I(demo.leftPane.gutterColumnWidth, demo.rightPane.gutterColumnWidth);

	/*
		The gutters of the two panes have to come out BYTE FOR BYTE the same, row by row.

		This is the check the whole thing turns on. Each pane numbering only its own file reads perfectly
		well one pane at a time and is wrong the moment the two are set beside each other: the two columns
		drift apart by however much was inserted above, and the same row then carries two different numbers.
		Only comparing the two rendered gutters against each other can see that.
	*/
	int32_t rowsWhoseGuttersDiffer = 0;
	int32_t rowsWithNoNumberAtAll = 0;
	int32_t rowsOfTheWrongWidth = 0;
	int32_t expectedGutterWidth = demo.leftPane.gutterColumnWidth * 2 + 1;
	for(int32_t rowIndex = 0; rowIndex < demo.leftPane.rowCount; ++rowIndex) {
		char leftText[FUI_TEXTEDITOR__MAX_GUTTER_TEXT];
		char rightText[FUI_TEXTEDITOR__MAX_GUTTER_TEXT];
		int32_t leftLength = DiffFormatGutterText(&demo.leftPane.editor, rowIndex, leftText, (int32_t)sizeof(leftText), &demo.leftPane);
		int32_t rightLength = DiffFormatGutterText(&demo.rightPane.editor, rowIndex, rightText, (int32_t)sizeof(rightText), &demo.rightPane);

		bool theTwoAgree = (leftLength == rightLength) && (memcmp(leftText, rightText, (size_t)leftLength) == 0);
		if(!theTwoAgree) {
			rowsWhoseGuttersDiffer += 1;
		}
		if(leftLength == 0) {
			rowsWithNoNumberAtAll += 1;
		} else if(leftLength != expectedGutterWidth) {
			rowsOfTheWrongWidth += 1;
		}
	}
	CHECK_I(rowsWhoseGuttersDiffer, 0);
	CHECK_I(rowsOfTheWrongWidth, 0);

	// The only rows without a number at all are the bands standing for lines that are not shown, and the
	// sample is small enough to be shown whole.
	CHECK_I(rowsWithNoNumberAtAll, 0);

	// Every line of each text is shown once, and the numbers of each file climb by one every time they
	// appear - which is what says the first column really is the old file and the second one the new.
	int32_t expectedOldLine = 0;
	int32_t expectedNewLine = 0;
	int32_t rowsOutOfOrder = 0;
	for(int32_t rowIndex = 0; rowIndex < demo.leftPane.rowCount; ++rowIndex) {
		int32_t oldNumber = demo.leftPane.gutterFirst[rowIndex];
		int32_t newNumber = demo.leftPane.gutterSecond[rowIndex];
		if(oldNumber >= 0) {
			if(oldNumber != expectedOldLine) {
				rowsOutOfOrder += 1;
			}
			expectedOldLine += 1;
		}
		if(newNumber >= 0) {
			if(newNumber != expectedNewLine) {
				rowsOutOfOrder += 1;
			}
			expectedNewLine += 1;
		}
	}
	CHECK_I(rowsOutOfOrder, 0);
	CHECK_I(expectedOldLine, demo.oldText.lineCount);
	CHECK_I(expectedNewLine, demo.newText.lineCount);

	DemoRelease(&demo);
}

/*
	Swapping the two texts round, which hands the load the very texts it is already holding.

	This is the one call that would go wrong if the old pair were let go before the new one was cut, and
	what it would go wrong with is a read of memory that had just been freed - so it is worth a check of
	its own, and it is worth running the suite under a sanitizer.
*/
static void SelfTestSwappingTheTwoTexts(void) {
	CheckSection("swapping the two texts round");

	DiffDemoState demo;
	CHECK(DemoInit(&demo));
	DemoLoadTheSample(&demo);
	int32_t rowCountBefore = demo.leftPane.rowCount;
	int32_t oldLineCountBefore = demo.oldText.lineCount;
	int32_t newLineCountBefore = demo.newText.lineCount;

	DemoSetTexts(&demo, demo.newText.bytes, demo.newText.byteCount, demo.oldText.bytes, demo.oldText.byteCount, "the other way round");
	CHECK(demo.hasTexts);
	CHECK_I(demo.oldText.lineCount, newLineCountBefore);
	CHECK_I(demo.newText.lineCount, oldLineCountBefore);
	CHECK_I(demo.leftPane.rowCount, demo.rightPane.rowCount);
	CHECK_I(demo.leftPane.rowCount, rowCountBefore);

	// What was added one way round is taken away the other, and the same number of rows says so.
	int32_t sampleNewLength = (int32_t)strlen(DemoSampleNewText);
	CHECK_I(demo.oldText.byteCount, sampleNewLength);
	CHECK_I(memcmp(demo.oldText.bytes, DemoSampleNewText, (size_t)sampleNewLength), 0);

	DemoRelease(&demo);
}

static int RunSelfTest(void) {
	printf("FUI_Diff self test, over final_ui_texteditor.h v%s\n", fuiEditorGetVersion());

	SelfTestCuttingIntoLines();
	SelfTestDifferenceOverTheObvious();
	SelfTestDifferenceAgainstBruteForce();
	SelfTestRowsOfBothViews();
	SelfTestSideRowsStayInStep();
	SelfTestCollapsingContext();
	SelfTestChangedSpanInsideALine();
	SelfTestARealFileAgainstAnEditedCopy();
	SelfTestTheTwoPanesAgree();
	SelfTestSwappingTheTwoTexts();

	printf("\n%d checks, %d failed\n", g_checkTotal, g_checkFailed);
	return((g_checkFailed == 0) ? 0 : 1);
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
	const char *oldFilePath = fpl_null;
	const char *newFilePath = fpl_null;
	bool startsSideBySide = false;
	for(int argumentIndex = 1; argumentIndex < argc; ++argumentIndex) {
		const char *argument = argv[argumentIndex];
		if(strcmp(argument, "--side-by-side") == 0) {
			startsSideBySide = true;
			continue;
		}
		if(strcmp(argument, "--selftest") == 0) {
			// No window and no OpenGL: a difference needs neither, and a check that opens a window cannot
			// run where a check is most wanted.
			if(!fplPlatformInit(fplInitFlags_None, fpl_null)) {
				fprintf(stderr, "failed to initialize the platform\n");
				return 1;
			}
			int selfTestResult = RunSelfTest();
			fplPlatformRelease();
			return selfTestResult;
		}
		if(oldFilePath == fpl_null) {
			oldFilePath = argument;
		} else if(newFilePath == fpl_null) {
			newFilePath = argument;
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

	// Two faces, both embedded in final_fonts.h: the proportional one the interface is drawn with, and the
	// monospace one the two texts are shown in - which is what makes the two gutter columns line up.
	fuiStbttBakeSettings bakeSettings = fuiStbttDefaultBakeSettings();
	bakeSettings.pixelHeight = DEMO_FONT_PIXEL_HEIGHT;
	bakeSettings.atlasWidth = DEMO_FONT_ATLAS_SIDE;
	bakeSettings.atlasHeight = DEMO_FONT_ATLAS_SIDE;

	const uint8_t *faceData[DEMO_FACE_COUNT];
	faceData[DEMO_FACE_UI] = ptr_fontBitstreamVeraRegular;
	faceData[DEMO_FACE_MONO] = ptr_fontBitstreamVeraMonoRegular;

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

	DiffDemoState demo;
	if(!DemoInit(&demo)) {
		fprintf(stderr, "failed to prepare the three editors\n");
		DemoRelease(&demo);
		fuiRelease(&ui);
		DemoReleaseFaces(bakedFonts, atlasTextures, bakedFaceCount);
		fglUnloadOpenGL();
		fplPlatformRelease();
		return 1;
	}
	demo.uiFont = &fonts[DEMO_FACE_UI];
	demo.monoFont = &fonts[DEMO_FACE_MONO];
	if(startsSideBySide) {
		demo.mode = DiffViewMode_SideBySide;
	}

	bool loadedFromTheCommandLine = false;
	if(oldFilePath != fpl_null && newFilePath != fpl_null) {
		loadedFromTheCommandLine = DemoLoadTwoFiles(&demo, oldFilePath, newFilePath);
	}
	if(!loadedFromTheCommandLine) {
		DemoLoadTheSample(&demo);
	}

	fuiPlatform platform = fplZeroInit;
	platform.getClipboardText = fuiFplGetClipboardText;
	platform.setClipboardText = fuiFplSetClipboardText;
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
