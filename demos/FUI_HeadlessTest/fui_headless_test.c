/*
Name:
	FUI_HeadlessTest

Description:
	Tests of final_ui.h without a window, a GPU or a font file.

	Every test builds frames from scripted input - a press here, a key there, some typed text - and checks what the widgets made of it.
	The input is scripted the way a hand would give it: the mouse rests over a widget for a frame before it presses, and a click into a field is followed by Ctrl+A before typing, because the click puts the caret where it lands.
	The font is a fake one of fixed width, which is all that layout and caret placement need.
	Each test prints PASS or FAIL with what it expected and what came instead, and the program exits with 1 when one failed.

Requirements:
	- C99 compiler

Build (from the repository root):
	gcc -std=c99 demos/FUI_HeadlessTest/fui_headless_test.c -I . -o fui_headless_test -lm
	./fui_headless_test

	Or with cmake:  cmake -S demos/FUI_HeadlessTest -B build/fui_headless_test && cmake --build build/fui_headless_test

License:
	MIT License, Copyright (c) 2017-2026 Torsten Spaete
*/

#define FUI_IMPLEMENTATION
#include <final_ui.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// ----------------------------------------------------------------------------
// > Results
// ----------------------------------------------------------------------------

//! What a running test knows about itself
typedef struct TestResult {
	const char *name;
	int32_t failureCount;
} TestResult;

static void TestExpect(TestResult *result, const bool condition, const char *format, ...) {
	if(condition) {
		return;
	}
	result->failureCount += 1;
	printf("FAIL %s: ", result->name);
	va_list arguments;
	va_start(arguments, format);
	vprintf(format, arguments);
	va_end(arguments);
	printf("\n");
}

// ----------------------------------------------------------------------------
// > The host: a fake font and scripted input
// ----------------------------------------------------------------------------

//! Every glyph of the fake font is this wide, in font units
static const float testGlyphAdvance = 0.5f;
static const float testFontAscent = 0.8f;
static const float testFontDescent = 0.2f;
static const float testFontLineHeight = 1.2f;

static const int32_t testWindowWidth = 400;
static const int32_t testWindowHeight = 200;
static const float testFrameSeconds = 1.0f / 60.0f;

//! One press and release of a key inside one frame is two half transitions
static const int32_t testTapTransitionCount = 2;

static bool TestFontGetGlyph(void *userData, uint32_t codePoint, fuiGlyph *outGlyph) {
	(void)userData;
	(void)codePoint;
	fuiGlyph glyph;
	memset(&glyph, 0, sizeof(glyph));
	glyph.size = fuiV2(testGlyphAdvance, testFontAscent + testFontDescent);
	glyph.advance = testGlyphAdvance;
	*outGlyph = glyph;
	return(true);
}

//! Which passes a host builds each frame
typedef enum TestPasses {
	//! One build that interacts and draws, what most hosts do
	TestPasses_One = 0,
	//! An interacting build followed by a drawing one, for a host that has to know where the mouse is wanted before it draws
	TestPasses_Two,
} TestPasses;

//! A context with its font, and the state of the mouse button carried from one frame to the next
typedef struct TestHost {
	fuiContext ui;
	fuiFont font;
	TestPasses passes;
	bool leftWasDown;
} TestHost;

//! What happens in one frame
typedef struct TestFrame {
	fuiVec2 mousePosition;
	bool leftIsDown;
	//! A key pressed and released inside the frame, fuiKey_None for none
	fuiKey tappedKey;
	//! Whether the left control key is held through the frame
	bool controlIsHeld;
	//! Text typed in the frame, null for none
	const char *typedText;
} TestFrame;

static bool TestHostInit(TestHost *host, const TestPasses passes) {
	memset(host, 0, sizeof(*host));
	host->font = fuiZeroFont();
	host->font.getGlyph = TestFontGetGlyph;
	host->font.metrics.ascent = testFontAscent;
	host->font.metrics.descent = testFontDescent;
	host->font.metrics.lineHeight = testFontLineHeight;
	host->font.metrics.spaceAdvance = testGlyphAdvance;
	host->passes = passes;
	const fuiAllocator *defaultAllocator = fui_null;
	bool result = fuiInit(&host->ui, &host->font, defaultAllocator);
	return(result);
}

static void TestHostRelease(TestHost *host) {
	fuiRelease(&host->ui);
}

static fuiInput TestBuildInput(TestHost *host, const TestFrame *frame) {
	fuiInput input = fuiZeroInput();
	input.mousePosition = frame->mousePosition;
	input.windowSize.x = testWindowWidth;
	input.windowSize.y = testWindowHeight;
	input.deltaTime = testFrameSeconds;
	input.isActive = true;

	bool leftChanged = (frame->leftIsDown != host->leftWasDown);
	input.mouseButtons[FUI_MOUSE_LEFT].endedDown = frame->leftIsDown;
	input.mouseButtons[FUI_MOUSE_LEFT].halfTransitionCount = leftChanged ? 1 : 0;
	host->leftWasDown = frame->leftIsDown;

	input.keys[fuiKey_LeftControl].endedDown = frame->controlIsHeld;

	if(frame->tappedKey != fuiKey_None) {
		input.keys[frame->tappedKey].endedDown = false;
		input.keys[frame->tappedKey].halfTransitionCount = testTapTransitionCount;
	}

	if(frame->typedText != fui_null) {
		size_t typedLength = strlen(frame->typedText);
		for(size_t typedIndex = 0; (typedIndex < typedLength) && (typedIndex < (size_t)FUI_MAX_TEXT_INPUT); ++typedIndex) {
			input.textInput[typedIndex] = (uint32_t)(unsigned char)frame->typedText[typedIndex];
			input.textInputLength += 1;
		}
	}
	return(input);
}

// ----------------------------------------------------------------------------
// > A form of two numeric fields
// ----------------------------------------------------------------------------

static const int32_t testSmallestValue = 1;
static const int32_t testLargestValue = 16384;
static const int32_t testStepSize = 1;
static const int32_t testFirstStartValue = 100;
static const int32_t testSecondStartValue = 200;

static const float testFieldX = 10.0f;
static const float testFieldWidth = 200.0f;
static const float testFieldHeight = 28.0f;
static const float testFirstFieldY = 10.0f;
static const float testSecondFieldY = 50.0f;

//! The two fields and what the form holds, built one after the other, the first above the second
typedef struct TestForm {
	int32_t first;
	int32_t second;
	//! False builds the second field alone, as if the first one had gone away
	bool firstIsBuilt;
} TestForm;

static void TestFormInit(TestForm *form) {
	form->first = testFirstStartValue;
	form->second = testSecondStartValue;
	form->firstIsBuilt = true;
}

//! The middle of a field, which is its text and not one of its step buttons
static fuiVec2 TestFieldCenter(const float fieldY) {
	float centerX = testFieldX + testFieldWidth * 0.5f;
	float centerY = fieldY + testFieldHeight * 0.5f;
	fuiVec2 result = fuiV2(centerX, centerY);
	return(result);
}

static void TestBuildForm(TestHost *host, TestForm *form, const fuiPass pass, const fuiInput *input) {
	fuiContext *ui = &host->ui;
	fuiBeginFrame(ui, input, pass);
	if(form->firstIsBuilt) {
		fuiRect firstRect = fuiRectMake(testFieldX, testFirstFieldY, testFieldWidth, testFieldHeight);
		(void)fuiInputInt(ui, firstRect, "first", &form->first, testSmallestValue, testLargestValue, testStepSize);
	}
	fuiRect secondRect = fuiRectMake(testFieldX, testSecondFieldY, testFieldWidth, testFieldHeight);
	(void)fuiInputInt(ui, secondRect, "second", &form->second, testSmallestValue, testLargestValue, testStepSize);
	fuiEndFrame(ui);
}

static void TestRunFrame(TestHost *host, TestForm *form, const TestFrame *frame) {
	fuiInput input = TestBuildInput(host, frame);
	if(host->passes == TestPasses_Two) {
		TestBuildForm(host, form, fuiPass_Interact, &input);
		TestBuildForm(host, form, fuiPass_Draw, &input);
	} else {
		TestBuildForm(host, form, fuiPass_Both, &input);
	}
}

//! A frame in which nothing happens, with the mouse resting where it is
static void TestIdle(TestHost *host, TestForm *form, const fuiVec2 mousePosition) {
	TestFrame frame;
	memset(&frame, 0, sizeof(frame));
	frame.mousePosition = mousePosition;
	TestRunFrame(host, form, &frame);
}

//! The mouse comes to rest over the position for a frame, as a real one does, because hovering is resolved against the frame before. Then a press in one frame and the release in the next
static void TestClick(TestHost *host, TestForm *form, const fuiVec2 position) {
	TestIdle(host, form, position);
	TestFrame frame;
	memset(&frame, 0, sizeof(frame));
	frame.mousePosition = position;
	frame.leftIsDown = true;
	TestRunFrame(host, form, &frame);
	frame.leftIsDown = false;
	TestRunFrame(host, form, &frame);
}

static void TestType(TestHost *host, TestForm *form, const fuiVec2 mousePosition, const char *text) {
	TestFrame frame;
	memset(&frame, 0, sizeof(frame));
	frame.mousePosition = mousePosition;
	frame.typedText = text;
	TestRunFrame(host, form, &frame);
}

//! Ctrl+A, which selects all of a text field, so what is typed next replaces it
static void TestSelectAll(TestHost *host, TestForm *form, const fuiVec2 mousePosition) {
	TestFrame frame;
	memset(&frame, 0, sizeof(frame));
	frame.mousePosition = mousePosition;
	frame.controlIsHeld = true;
	frame.tappedKey = fuiKey_A;
	TestRunFrame(host, form, &frame);
}

static void TestTapKey(TestHost *host, TestForm *form, const fuiVec2 mousePosition, const fuiKey key) {
	TestFrame frame;
	memset(&frame, 0, sizeof(frame));
	frame.mousePosition = mousePosition;
	frame.tappedKey = key;
	TestRunFrame(host, form, &frame);
}

// ----------------------------------------------------------------------------
// > Numeric fields
// ----------------------------------------------------------------------------

//! Tab out of a field that is typed into reads the number back and gives the keyboard to the next field
static void TestTabMovesOnFromATypedNumber(TestResult *result) {
	const int32_t typedFirst = 1234;
	const int32_t typedSecond = 77;
	TestHost host;
	bool hostIsReady = TestHostInit(&host, TestPasses_One);
	TestExpect(result, hostIsReady, "expected the context to come up");
	if(!hostIsReady) {
		return;
	}
	TestForm form;
	TestFormInit(&form);
	fuiVec2 firstCenter = TestFieldCenter(testFirstFieldY);

	TestClick(&host, &form, firstCenter);
	TestSelectAll(&host, &form, firstCenter);
	TestType(&host, &form, firstCenter, "1234");
	TestTapKey(&host, &form, firstCenter, fuiKey_Tab);
	TestIdle(&host, &form, firstCenter);
	TestExpect(result, form.first == typedFirst, "expected tab to read back %d into the first field, it holds %d", typedFirst, form.first);

	// The keyboard is in the second field now, so typing and enter end up there.
	TestType(&host, &form, firstCenter, "77");
	TestTapKey(&host, &form, firstCenter, fuiKey_Return);
	TestIdle(&host, &form, firstCenter);
	TestExpect(result, form.second == typedSecond, "expected the typing after tab to land in the second field as %d, it holds %d and the first holds %d", typedSecond, form.second, form.first);
	TestHostRelease(&host);
}

//! A click from a field that is typed into straight into the field built AFTER it reads the number back
static void TestClickIntoALaterFieldKeepsTheNumber(TestResult *result, const TestPasses passes) {
	TestHost host;
	bool hostIsReady = TestHostInit(&host, passes);
	TestExpect(result, hostIsReady, "expected the context to come up");
	if(!hostIsReady) {
		return;
	}
	TestForm form;
	TestFormInit(&form);
	fuiVec2 firstCenter = TestFieldCenter(testFirstFieldY);
	fuiVec2 secondCenter = TestFieldCenter(testSecondFieldY);

	// More than the range holds, so the number comes back clamped.
	TestClick(&host, &form, firstCenter);
	TestSelectAll(&host, &form, firstCenter);
	TestType(&host, &form, firstCenter, "99999");
	TestClick(&host, &form, secondCenter);
	TestIdle(&host, &form, secondCenter);
	TestExpect(result, form.first == testLargestValue, "expected the first field to read back 99999 clamped to %d, it holds %d", testLargestValue, form.first);
	bool secondIsTypedInto = fuiIsTypingANumber(&host.ui);
	TestExpect(result, secondIsTypedInto && (form.second == testSecondStartValue), "expected the second field to be typed into and still hold %d, it holds %d", testSecondStartValue, form.second);
	TestHostRelease(&host);
}

//! A click from a field that is typed into straight into the field built BEFORE it reads the number back
static void TestClickIntoAnEarlierFieldKeepsTheNumber(TestResult *result) {
	const int32_t typedSecond = 4321;
	TestHost host;
	bool hostIsReady = TestHostInit(&host, TestPasses_One);
	TestExpect(result, hostIsReady, "expected the context to come up");
	if(!hostIsReady) {
		return;
	}
	TestForm form;
	TestFormInit(&form);
	fuiVec2 firstCenter = TestFieldCenter(testFirstFieldY);
	fuiVec2 secondCenter = TestFieldCenter(testSecondFieldY);

	TestClick(&host, &form, secondCenter);
	TestSelectAll(&host, &form, secondCenter);
	TestType(&host, &form, secondCenter, "4321");
	TestClick(&host, &form, firstCenter);
	TestIdle(&host, &form, firstCenter);
	TestExpect(result, form.second == typedSecond, "expected the second field to read back %d, it holds %d", typedSecond, form.second);
	TestExpect(result, form.first == testFirstStartValue, "expected the first field to still hold %d, it holds %d", testFirstStartValue, form.first);
	TestHostRelease(&host);
}

//! A field that is not built again right after its edit was taken over never reads the text back, not even when it comes back later
static void TestPutAsideTextExpires(TestResult *result) {
	TestHost host;
	bool hostIsReady = TestHostInit(&host, TestPasses_One);
	TestExpect(result, hostIsReady, "expected the context to come up");
	if(!hostIsReady) {
		return;
	}
	TestForm form;
	TestFormInit(&form);
	fuiVec2 firstCenter = TestFieldCenter(testFirstFieldY);
	fuiVec2 secondCenter = TestFieldCenter(testSecondFieldY);

	TestClick(&host, &form, firstCenter);
	TestSelectAll(&host, &form, firstCenter);
	TestType(&host, &form, firstCenter, "555");

	// The press takes the edit over while the first field is still built, and with the release the first field is gone.
	TestIdle(&host, &form, secondCenter);
	TestFrame press;
	memset(&press, 0, sizeof(press));
	press.mousePosition = secondCenter;
	press.leftIsDown = true;
	TestRunFrame(&host, &form, &press);
	form.firstIsBuilt = false;
	TestIdle(&host, &form, secondCenter);
	TestIdle(&host, &form, secondCenter);

	form.firstIsBuilt = true;
	TestIdle(&host, &form, secondCenter);
	TestIdle(&host, &form, secondCenter);
	TestExpect(result, form.first == testFirstStartValue, "expected the first field to keep %d after it was gone for a frame, it holds %d", testFirstStartValue, form.first);
	TestHostRelease(&host);
}

// ----------------------------------------------------------------------------
// > Main
// ----------------------------------------------------------------------------

//! One test and its name
typedef struct TestEntry {
	const char *name;
	void (*run)(TestResult *result);
} TestEntry;

static void TestClickIntoALaterFieldOnePass(TestResult *result) {
	TestClickIntoALaterFieldKeepsTheNumber(result, TestPasses_One);
}

static void TestClickIntoALaterFieldTwoPasses(TestResult *result) {
	TestClickIntoALaterFieldKeepsTheNumber(result, TestPasses_Two);
}

static const TestEntry testEntries[] = {
	{ "numeric/tab-moves-on-from-a-typed-number", TestTabMovesOnFromATypedNumber },
	{ "numeric/click-into-a-later-field-keeps-the-number", TestClickIntoALaterFieldOnePass },
	{ "numeric/click-into-a-later-field-keeps-the-number-two-passes", TestClickIntoALaterFieldTwoPasses },
	{ "numeric/click-into-an-earlier-field-keeps-the-number", TestClickIntoAnEarlierFieldKeepsTheNumber },
	{ "numeric/put-aside-text-expires", TestPutAsideTextExpires },
};

int main(int argc, char **argv) {
	(void)argc;
	(void)argv;
	const size_t testCount = sizeof(testEntries) / sizeof(testEntries[0]);
	int32_t failedTestCount = 0;
	for(size_t testIndex = 0; testIndex < testCount; ++testIndex) {
		const TestEntry *entry = &testEntries[testIndex];
		TestResult result;
		result.name = entry->name;
		result.failureCount = 0;
		entry->run(&result);
		if(result.failureCount == 0) {
			printf("PASS %s\n", entry->name);
		} else {
			failedTestCount += 1;
		}
	}
	int32_t passedTestCount = (int32_t)testCount - failedTestCount;
	printf("\n%d passed, %d failed\n", passedTestCount, failedTestCount);
	int exitCode = (failedTestCount == 0) ? 0 : 1;
	return(exitCode);
}
