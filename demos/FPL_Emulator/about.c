/*
Name:
	Final Gamebox

	Frontend-Part (About Dialog Implementation)

Author:
	Torsten Spaete

Description:
	This file is part of the frontend of the Final Gamebox project.
*/

#include "about.h"

const char *AboutDialogId = "About-Dialog";

//
// Versions
//
// The core carries its own version number, so that one is read rather than written down twice. The
// libraries under dependencies/ are vendored copies that export no version macro at all, so what their
// own @version tag says is repeated here and has to be carried over whenever one of them is updated.
//

// Version of this frontend, which is versioned on its own rather than with the core it drives
#define ApplicationVersionString "1.1.0"

#define ApplicationAuthorName "Torsten Spaete (alias Finalspace)"

#define ApplicationCopyrightText "Copyright (c) 2024-2026 Torsten Spaete"

#define PlatformLayerVersionString "1.0.0"

#define UserInterfaceVersionString "0.9.2"

#define OpenGLLoaderVersionString "0.4.0"

#define MathVersionString "1.5.0"

#define MemoryVersionString "0.4.0"

#define FontProviderVersionString "0.9.2"

#define STBImageVersionString "2.19"

#define STBTrueTypeVersionString "1.26"

#define MiniZVersionString "3.0.2"

//
// Pages
//
// Every page is one block of plain text, laid out in columns that only line up because the text view
// draws it without wrapping and in the same width for every glyph position it is given.
//

// Filled once on the first build, because it is the only page carrying anything that is not a literal
static char AboutPageText[3072] = { 0 };

static const char *AboutPageTemplate =
"  FINAL GAMEBOX\n"
"  =============\n"
"\n"
"  A Game Boy (DMG) and Game Boy Color (CGB) emulator with a full visual debugger,\n"
"  written in C17 and built on a set of single header libraries.\n"
"\n"
"\n"
"  APPLICATION\n"
"  -----------\n"
"\n"
"    Name             Final Gamebox (FGB)\n"
"    Version          %s\n"
"    Author           %s\n"
"    Copyright        %s\n"
"    License          MIT License\n"
"    Built            %s %s\n"
"\n"
"\n"
"  COMPONENTS\n"
"  ----------\n"
"\n"
"    Game Boy Core    final_game_box.h            %s\n"
"    Platform Layer   final_platform_layer.h      %s\n"
"    User Interface   final_ui.h                  %s\n"
"    OpenGL Loader    final_dynamic_opengl.h      %s\n"
"    Math             final_math.h                %s\n"
"    Memory           final_memory.h              %s\n"
"    Font Provider    fui_font_stbtt.h            %s\n"
"\n"
"    Every one of them is written by the same author and released under the MIT License.\n"
"    The third party libraries are listed on the Libraries page.\n"
"\n"
"\n"
"  ABOUT THE CORE\n"
"  --------------\n"
"\n"
"    The emulated hardware lives in one single header library that knows nothing about\n"
"    windows, files or sound devices - the frontend hands it callbacks for all three.\n"
"    That is what lets the very same core run under this application and under the\n"
"    headless test runner that plays the Blargg, Acid and Mooneye test ROMs.\n"
"\n"
"    It was written for the sake of learning how the machine works, and all knowledge in\n"
"    it comes from public documentation, hardware tests and the Ultimate Game Boy Talk.\n"
"\n"
"\n"
"  KNOWN LIMITATIONS\n"
"  -----------------\n"
"\n"
"    - No Super Game Boy support, an SGB cartridge falls back to DMG or CGB\n"
"    - Save states are unavailable on 32 bit builds, due to pointer alignment\n"
"    - The OAM bug is not emulated\n"
"    - Interrupt timing is not exact, the Blargg interrupt_time test fails\n"
"    - Complex CGB games may still show graphical or audio glitches\n";

static const char *HowToUsePageText =
"  HOW TO USE\n"
"  ==========\n"
"\n"
"\n"
"  LOADING A GAME\n"
"  --------------\n"
"\n"
"    Drag a ROM file onto the window and drop it. That is the whole procedure.\n"
"\n"
"    Accepted files    .gb     Game Boy (DMG) cartridge\n"
"                      .gbc    Game Boy Color (CGB) cartridge\n"
"                      .zip    An archive holding one of the two\n"
"\n"
"    A ROM given on the command line is loaded at startup instead, so the application\n"
"    can be registered as the handler for a ROM file and started by double clicking one.\n"
"\n"
"    The game starts running the moment it is loaded, unless IR-Pause is switched on in\n"
"    the debugger, which makes every freshly loaded game come up paused instead.\n"
"\n"
"\n"
"  THE TWO VIEWS\n"
"  -------------\n"
"\n"
"    F1 switches between them at any time.\n"
"\n"
"    Player view       Nothing but the game, scaled to fill the window.\n"
"    Debugger view     The game in the middle, its state and its controls around it.\n"
"\n"
"\n"
"  THE DEBUGGER VIEW\n"
"  -----------------\n"
"\n"
"    Left column       Log         Everything the core reported, newest at the bottom\n"
"                      Performance Where the frame time is being spent\n"
"                      Disassembly The decoded program, following the program counter\n"
"                      BG-Map      The full 256x256 background with the visible window on it\n"
"\n"
"    Middle column     The cartridge header, the LCD registers, the four sound voices,\n"
"                      the upscaling filter, the emulated display and the state buttons.\n"
"\n"
"    Right column      Run and stepping buttons, the CPU registers and flags, the\n"
"                      switches, and a tab strip with the tiles, the palettes and the\n"
"                      breakpoints.\n"
"\n"
"    The two seams between the columns are draggable, so a column may be widened at the\n"
"    cost of its neighbour.\n"
"\n"
"\n"
"  RUNNING AND STEPPING\n"
"  --------------------\n"
"\n"
"    Pause / Resume    Stops the emulation where it stands, or lets it run on\n"
"    Frame Step        Runs exactly one full PPU frame and stops again\n"
"    Single Step       Runs exactly one CPU instruction\n"
"    Micro Step        Runs one CPU or hardware tick, which is finer than an instruction\n"
"    Reset             Restarts the cartridge and clears the log\n"
"\n"
"    While the emulation is paused the disassembly follows the program counter, so the\n"
"    highlighted line is always the instruction that is about to be executed.\n"
"\n"
"\n"
"  BREAKPOINTS\n"
"  -----------\n"
"\n"
"    The Breakpoints tab in the right column lists every event the core can stop on.\n"
"    Tick one and the emulation halts the next time that event happens.\n"
"\n"
"\n"
"  SAVE STATES\n"
"  -----------\n"
"\n"
"    Save State and Restore State open a grid of six slots, each showing the screen the\n"
"    slot was taken on together with the game and the moment it was saved.\n"
"\n"
"    Arrow keys or D-Pad    Move the selection\n"
"    Return or Start        Save into, or restore from, the selected slot\n"
"    Escape or Close        Leave without touching anything\n"
"\n"
"    A slot is written straight to disk, so it survives the application being closed.\n"
"\n"
"\n"
"  FILES WRITTEN BESIDE THE ROM\n"
"  ----------------------------\n"
"\n"
"    <rom>.eram        Battery backed cartridge RAM, saved and reloaded automatically\n"
"    <rom>.sav0        Save state slot 1\n"
"    ...               ...\n"
"    <rom>.sav5        Save state slot 6\n"
"\n"
"    They sit next to the ROM they belong to and carry its name, so moving a ROM together\n"
"    with its files keeps the saves attached to it.\n"
"\n"
"\n"
"  SOUND\n"
"  -----\n"
"\n"
"    The sound panel in the middle column carries a switch that silences the emulator on\n"
"    the way to the audio device, and a master volume slider beside it. Each of the four\n"
"    voices can be watched there while it plays.\n"
"\n"
"\n"
"  DISPLAY FILTERS\n"
"  ---------------\n"
"\n"
"    The row under the sound panel picks how the 160x144 display is scaled up:\n"
"\n"
"    Nearest           Hard pixels, exactly what the hardware put out\n"
"    Bilinear          Plain smoothing\n"
"    HQ2x, HQ4x        Edge aware pixel art upscalers\n"
"    Bicubic-H/-L      Bicubic smoothing, Hermite or Lagrange\n"
"    CatMullRom        Catmull-Rom smoothing, sharper than bicubic\n"
"\n"
"    A filter that the graphics driver could not compile is shown greyed out.\n"
"\n"
"\n"
"  COLOR PALETTES\n"
"  --------------\n"
"\n"
"    A monochrome game can be shown in any of four palettes, chosen on the Palettes tab:\n"
"    DMG (the green original), MGB (the grey Pocket), SGB and a blue variant. A Game Boy\n"
"    Color game brings its own colors and ignores the choice.\n"
"\n"
"\n"
"  COMMAND LINE\n"
"  ------------\n"
"\n"
"    FPL_Emulator [options] <rom file>\n"
"\n"
"    -t, --trace       Switch CPU instruction tracing on at startup\n"
"    --input <file>    Replay a scripted joypad input file, for automated testing\n"
"\n"
"    The input script format is documented in docs/INPUT_SIMULATOR.md. While a script\n"
"    still has events left to play it owns the joypad and the keyboard is ignored.\n";

static const char *ControlsPageText =
"  CONTROLS\n"
"  ========\n"
"\n"
"\n"
"  GAME BOY JOYPAD\n"
"  ---------------\n"
"\n"
"    Joypad            Keyboard              Gamepad\n"
"    ----------------  --------------------  --------------------------------\n"
"    D-Pad Up          Up Arrow              D-Pad Up or Left Stick Up\n"
"    D-Pad Down        Down Arrow            D-Pad Down or Left Stick Down\n"
"    D-Pad Left        Left Arrow            D-Pad Left or Left Stick Left\n"
"    D-Pad Right       Right Arrow           D-Pad Right or Left Stick Right\n"
"    A                 A                     A Button\n"
"    B                 S                     B Button\n"
"    Start             Return / Enter        Start Button\n"
"    Select            Backspace             Back / Select Button\n"
"\n"
"    The keyboard and every connected gamepad are all wired to the same joypad, and the\n"
"    device that pressed something last is the one that holds it. So picking up a gamepad\n"
"    mid-game just works, and so does putting it back down.\n"
"\n"
"    A gamepad has to be connected before it is noticed. Press any mapped button on it\n"
"    once and it takes over the joypad from there on.\n"
"\n"
"\n"
"  APPLICATION\n"
"  -----------\n"
"\n"
"    F1                Switch between the player view and the debugger view\n"
"    Escape            Close the dialog that is in front\n"
"\n"
"\n"
"  DEBUGGER (only while the debugger view is showing)\n"
"  --------------------------------------------------\n"
"\n"
"    Space             Run a single CPU instruction\n"
"    F                 Run a single full frame\n"
"\n"
"    Everything else the debugger does is on the buttons around the display: Pause,\n"
"    Resume, Frame Step, Single Step, Micro Step and Reset.\n"
"\n"
"\n"
"  STATE DIALOGS (Save State and Restore State)\n"
"  --------------------------------------------\n"
"\n"
"    Arrow keys        Move the slot selection, D-Pad on a gamepad\n"
"    Return            Confirm the selected slot, Start on a gamepad\n"
"    Escape            Close without saving or restoring\n"
"\n"
"\n"
"  MOUSE\n"
"  -----\n"
"\n"
"    Left button       Everything: buttons, checkboxes, tabs, sliders and scrollbars\n"
"    Left drag         The seams between the debugger columns, to widen one of them\n"
"    Wheel             Scrolls the list or the text box under the cursor\n"
"    Shift + Wheel     Scrolls a text box sideways, in this dialog\n"
"\n"
"\n"
"  DRAG AND DROP\n"
"  -------------\n"
"\n"
"    A .gb, .gbc or .zip file dropped anywhere on the window is loaded as a cartridge.\n";

static const char *FeaturesPageText =
"  FEATURES\n"
"  ========\n"
"\n"
"\n"
"  EMULATED HARDWARE\n"
"  -----------------\n"
"\n"
"    CPU               The Z80/8080 like processor with the entire 2 x 256 instruction\n"
"                      set, including the HALT bug and a correct STOP\n"
"    PPU               Pixel-FIFO based renderer for background, window and sprites,\n"
"                      the way the Ultimate Game Boy Talk describes the hardware\n"
"    APU               Four voices: two pulse, one wave/PCM and one LFSR noise\n"
"    MMU               The bus every read and write travels over\n"
"    Timer             DIV and TIMA with their real reload and overflow behaviour\n"
"    Serial            Serial transfer, which some games wait on before they start\n"
"    Interrupts        V-Blank, LCD STAT, Timer, Serial and Joypad\n"
"\n"
"\n"
"  GAME BOY COLOR\n"
"  --------------\n"
"\n"
"    - RGB555 color palette RAM, 2 x 32 colors for backgrounds and sprites\n"
"    - Double speed mode and speed switching\n"
"    - HDMA and GDMA block transfers, chained transfers included\n"
"    - VRAM bank switching and WRAM bank switching\n"
"    - Per-tile attributes: palette, bank, flipping and priority\n"
"\n"
"\n"
"  CARTRIDGES\n"
"  ----------\n"
"\n"
"    ROM only          Cartridges without a memory bank controller\n"
"    MBC1              ROM and RAM banking, both wiring modes\n"
"    MBC2              Built in 512 x 4 bit RAM\n"
"    MBC3              ROM and RAM banking with a real time clock\n"
"    MBC5              Large ROM and RAM banking\n"
"    MBC7              Accelerometer cartridges, with EEPROM\n"
"\n"
"    Battery backed cartridge RAM is loaded when a game starts and written back as the\n"
"    game changes it, so a save inside the game survives the application being closed.\n"
"\n"
"\n"
"  SAVE STATES\n"
"  -----------\n"
"\n"
"    - Six slots per game, each holding the complete machine\n"
"    - A screenshot of the moment it was taken, shown in the slot grid\n"
"    - Written beside the ROM and reloaded the next time that game is opened\n"
"\n"
"\n"
"  FRONTEND\n"
"  --------\n"
"\n"
"    - OpenGL rendering with an immediate mode interface\n"
"    - Drag and drop loading, from a raw file or from inside a zip archive\n"
"    - The DMG boot ROM, switchable, for the original startup animation\n"
"    - Seven display upscaling filters, from hard pixels to Catmull-Rom\n"
"    - Four monochrome palettes for DMG games: DMG, MGB, SGB and blue\n"
"    - Master volume and a sound switch that is independent of the emulated hardware\n"
"    - Scripted joypad input replay, for testing a game without sitting at the keyboard\n"
"\n"
"\n"
"  THREADING\n"
"  ---------\n"
"\n"
"    The emulation, the audio and the drawing each run on their own thread and meet only\n"
"    at ring buffers: one for the finished frames and one for the audio samples. That is\n"
"    what keeps the emulation running at the machine's real speed while the window is\n"
"    being resized, and what keeps the sound from breaking up when a frame takes long.\n"
"\n"
"\n"
"  DEBUGGER\n"
"  --------\n"
"\n"
"    Disassembly       The program decoded from its entry points outwards, following the\n"
"                      program counter as it runs\n"
"    Breakpoints       Stop the emulation on a chosen hardware event\n"
"    Stepping          By instruction, by hardware tick or by whole frame\n"
"    CPU state         Registers, flags, the stack pointer and the interrupt state\n"
"    LCD state         The LCD registers, the current mode and the pixel FIFO\n"
"    Sound state       All four voices with their live parameters, mute markers included\n"
"    Cartridge         Title, cartridge type, core type and the bank counts\n"
"    Tiles             Every tile in video memory, drawn as one sheet\n"
"    Background map    The full 256 x 256 background with the visible window drawn on it\n"
"    Palettes          The monochrome palettes, or all 2 x 32 CGB colors\n"
"    Log               Everything the core reported, with tracing switchable at runtime\n"
"    Performance       Where the time in a frame actually went\n"
"\n"
"\n"
"  TESTING\n"
"  -------\n"
"\n"
"    A second, headless frontend runs the Blargg, Acid and Mooneye test ROM suites\n"
"    against the very same core, comparing serial output, memory mapped results and\n"
"    whole frames by hash against a stored baseline.\n";

static const char *LibrariesPageText =
"  LIBRARIES\n"
"  =========\n"
"\n"
"  Everything this application is built out of is a single header or single file library,\n"
"  compiled straight into it. There is nothing to install and nothing to ship beside it.\n"
"\n"
"\n"
"  BY THE AUTHOR OF THIS APPLICATION\n"
"  ---------------------------------\n"
"\n"
"    final_game_box.h          Torsten Spaete            MIT License\n"
"      The emulated Game Boy: CPU, PPU, APU, MMU and the cartridge controllers.\n"
"\n"
"    final_platform_layer.h    Torsten Spaete            MIT License\n"
"      Window, OpenGL context, audio device, gamepads, files, threads and timing.\n"
"\n"
"    final_ui.h                Torsten Spaete            MIT License\n"
"      The immediate mode interface: panels, tabs, dialogs and every widget in them.\n"
"\n"
"    final_dynamic_opengl.h    Torsten Spaete            MIT License\n"
"      Loads the OpenGL entry points at runtime, so no import library is needed.\n"
"\n"
"    final_math.h              Torsten Spaete            MIT License\n"
"      Vectors, matrices and the color conversions the renderer works in.\n"
"\n"
"    final_memory.h            Torsten Spaete            MIT License\n"
"      The block and arena allocators everything in the frontend is carved out of.\n"
"\n"
"    fui_font_stbtt.h          Torsten Spaete            MIT License\n"
"      Bakes a TrueType font into the glyph atlas final_ui.h draws its text from.\n"
"\n"
"\n"
"  THIRD PARTY\n"
"  -----------\n"
"\n"
"    stb_truetype.h            Sean Barrett / RAD Game Tools\n"
"                              Public Domain, or MIT License, at your choice\n"
"      Rasterizes the TrueType font the interface is drawn with.\n"
"\n"
"    stb_image.h               Sean Barrett\n"
"                              Public Domain, or MIT License, at your choice\n"
"      Decodes the images the application carries built in.\n"
"\n"
"    miniz                     Rich Geldreich and Tenacious Software LLC\n"
"                              Rich Geldreich, Tenacious Software and RAD Game Tools\n"
"                              Valve Software\n"
"                              MIT License\n"
"      Reads a cartridge out of a zip archive without unpacking it to disk first.\n"
"\n"
"\n"
"  THE MIT LICENSE, IN SHORT\n"
"  -------------------------\n"
"\n"
"    Do whatever you like with it, including selling it, as long as the copyright notice\n"
"    and the license text travel with the copy. It comes with no warranty of any kind.\n"
"\n"
"\n"
"  PUBLIC DOMAIN, IN SHORT\n"
"  -----------------------\n"
"\n"
"    No conditions at all. The author has given up every claim to it.\n"
"\n"
"\n"
"  THANKS\n"
"  ------\n"
"\n"
"    The hardware knowledge in the core comes from work that other people published and\n"
"    gave away: the Ultimate Game Boy Talk, the Pan Docs, the Cycle-Accurate Game Boy\n"
"    Docs, the gbops instruction table, and the Blargg, Acid and Mooneye test ROMs that\n"
"    make it possible to tell a correct emulator from one that merely looks correct.\n";

//
// Dialog
//

// How much of the window the dialog takes when it opens, before the user drags it to another size
static const float AboutDialogWidthFactor = 0.8f;
static const float AboutDialogHeightFactor = 0.85f;

// Smallest the dialog opens at, so it stays readable on a small window
static const float AboutDialogMinimumWidth = 640.0f;
static const float AboutDialogMinimumHeight = 420.0f;

static const float AboutDialogCloseButtonWidth = 120.0f;

// How strongly the application icon shows through behind the pages. Turn it down to fade the picture out,
// or to zero to be rid of it altogether
static const float AboutDialogBackdropOpacity = 0.1f;

// How solid a page is drawn over that backdrop. Zero lets the whole picture through and leaves the text
// standing straight on it; one hides the picture behind the page completely. The backdrop above is already
// faint enough to read across, so the pages add nothing on top of it.
static const float AboutDialogPageOpacity = 0.0f;

// Height of the close button as a multiple of one text row
static const float AboutDialogCloseButtonRowCount = 1.5f;

// Height of the page header strip as a multiple of one text row, matching the debugger's own tab strips
static const float AboutDialogTabHeaderRowCount = 1.5f;


static const char *AboutPageNames[AboutPage_Count] = {
	"About",
	"How to Use",
	"Controls",
	"Features",
	"Libraries",
};

// Built on the first call rather than at startup, because it is only ever needed once the dialog is opened
static const char *AboutDialogGetAboutPageText(void) {
	if (AboutPageText[0] == '\0') {
		const char *coreVersionString = fgbGetVersion();
		fplStringFormat(AboutPageText, fplArrayCount(AboutPageText), AboutPageTemplate,
			ApplicationVersionString,
			ApplicationAuthorName,
			ApplicationCopyrightText,
			__DATE__, __TIME__,
			coreVersionString,
			PlatformLayerVersionString,
			UserInterfaceVersionString,
			OpenGLLoaderVersionString,
			MathVersionString,
			MemoryVersionString,
			FontProviderVersionString);
	}
	return AboutPageText;
}

static const char *AboutDialogGetPageText(const AboutPage page) {
	switch (page) {
		case AboutPage_HowToUse:
			return HowToUsePageText;
		case AboutPage_Controls:
			return ControlsPageText;
		case AboutPage_Features:
			return FeaturesPageText;
		case AboutPage_Libraries:
			return LibrariesPageText;
		case AboutPage_About:
		default:
			return AboutDialogGetAboutPageText();
	}
}

// Describes one of the frontend's textures as a picture. Every texture arrives from the image loader upside
// down, so the flip is set here rather than being remembered at each of the places that draw one.
static fuiImageDesc AboutDialogImageFromTexture(const Texture *texture, const float opacity) {
	fuiImageDesc result = fplZeroInit;
	if (texture == fpl_null || !texture->isValid) {
		return result;
	}
	result.texture = (fuiTextureId)texture->id;
	result.textureSize = fuiV2((float)texture->width, (float)texture->height);
	result.uvMin = fuiV2(0.0f, 0.0f);
	result.uvMax = fuiV2(texture->uScale, texture->vScale);
	result.tint = fuiColorRGBA(1.0f, 1.0f, 1.0f, opacity);
	result.scaleMode = FUI_IMAGE_SCALE_LETTERBOX;
	result.flags = FUI_IMAGE_FLIP_V;
	return result;
}

void AboutDialogOpen(fuiContext *ui, AboutDialog *dialog, const AboutPage page) {
	if (ui == fpl_null || dialog == fpl_null) {
		return;
	}
	dialog->selectedPageIndex = (int32_t)page;
	fuiOpenDialog(ui, AboutDialogId);
}

void AboutDialogBuild(fuiContext *ui, AboutDialog *dialog, const Texture *titleIcon, const Texture *backdropIcon, const float windowWidth, const float windowHeight) {
	if (ui == fpl_null || dialog == fpl_null) {
		return;
	}

	const float dialogWidth = fuiMaxF(windowWidth * AboutDialogWidthFactor, AboutDialogMinimumWidth);
	const float dialogHeight = fuiMaxF(windowHeight * AboutDialogHeightFactor, AboutDialogMinimumHeight);

	// The same picture the button that opens this carries, in the title bar the way a desktop window wears
	// its own. The library fits it into the bar and moves the caption along past it, so what belongs here is
	// a bake small enough not to be shrunk to a fifth of its size on the way in.
	const fuiImageDesc titleIconImage = AboutDialogImageFromTexture(titleIcon, 1.0f);

	// The pages are wide tables of text, so this is a dialog worth being able to move and resize
	if (!fuiBeginModalResizableIcon(ui, AboutDialogId, "Final Gamebox - Information", dialogWidth, dialogHeight, &titleIconImage)) {
		fuiEndModal(ui);
		return;
	}

	const fuiTheme *theme = fuiGetTheme(ui);
	const float lineHeight = theme->menuItemHeight;

	const fuiRect content = fuiLayoutRemaining(ui);

	// The application's own icon, laid across the whole dialog before anything is built on top of it. The
	// pages are drawn only partly solid, which is what lets it read as a backdrop rather than as a picture
	// somebody left lying under the text.
	const fuiImageDesc backdropImage = AboutDialogImageFromTexture(backdropIcon, AboutDialogBackdropOpacity);
	fuiImage(ui, content, &backdropImage);

	const float tabHeaderHeight = lineHeight * AboutDialogTabHeaderRowCount;
	const float closeButtonHeight = lineHeight * AboutDialogCloseButtonRowCount;

	// The headers and their page take the whole width of the dialog, the icon being up in the title bar
	const fuiRect tabHeader = fuiRectMake(content.x, content.y, content.w, tabHeaderHeight);

	const float pageY = content.y + tabHeaderHeight;
	const float pageHeight = content.h - tabHeaderHeight - closeButtonHeight - theme->widgetSpacing;
	const fuiRect page = fuiRectMake(content.x, pageY, content.w, pageHeight);

	// Clipped to the strip, so headers wider than a narrowed dialog are cut at its edge rather than
	// spilling over the icon beside them
	fuiPushClip(ui, tabHeader);
	int32_t selectedPageIndex = UITabStrip(ui, tabHeader, "About-TabStrip", AboutPageNames, (int32_t)AboutPage_Count, &dialog->selectedPageIndex);
	fuiPopClip(ui);

	const char *pageText = AboutDialogGetPageText((AboutPage)selectedPageIndex);
	UITextViewState *pageView = dialog->pageViews + selectedPageIndex;
	UITextView(ui, page, AboutPageNames[selectedPageIndex], pageText, pageView, AboutDialogPageOpacity);

	bool shouldClose = false;

	const float closeButtonX = content.x + (content.w - AboutDialogCloseButtonWidth) * 0.5f;
	const float closeButtonY = content.y + content.h - closeButtonHeight;
	if (fuiButton(ui, fuiRectMake(closeButtonX, closeButtonY, AboutDialogCloseButtonWidth, closeButtonHeight), "Close")) {
		shouldClose = true;
	}

	// Escape is taken through the library, so a dialog stacked on another one does not close both on one press
	if (fuiDialogTakeKey(ui, FUI_KEY_ESCAPE)) {
		shouldClose = true;
	}

	fuiEndModal(ui);

	if (shouldClose) {
		fuiCloseDialog(ui, AboutDialogId);
	}
}
