#pragma once

#include <final_platform_layer.h>

#include <final_math.h>

#include "types.h"
#include "fonts.h"

static const float FeaturesFontSize = 32.0f;

enum class BlockType {
	None = 0,
	Text,
};

struct TextBlockDefinition {
	const char *text;
	float fontSize;
	HorizontalAlignment textAlign;
};

struct BlockAlignment {
	HorizontalAlignment h;
	VerticalAlignment v;
};

static BlockAlignment MakeAlign(HorizontalAlignment h = HorizontalAlignment::Left, VerticalAlignment v = VerticalAlignment::Top) {
	BlockAlignment result = {};
	result.h = h;
	result.v = v;
	return(result);
}

struct BlockDefinition {
	Vec2f pos;
	Vec2f size;
	BlockType type;
	BlockAlignment contentAlignment;
	union {
		TextBlockDefinition text;
	};
};

static BlockDefinition MakeTextDef(const Vec2f &pos, const Vec2f &size, BlockAlignment contentAlignment, const char *text, const HorizontalAlignment textAlign = HorizontalAlignment::Left, const float fontSize = 0) {
	BlockDefinition result = {};
	result.pos = pos;
	result.size = size;
	result.contentAlignment = contentAlignment;
	result.type = BlockType::Text;
	result.text.text = text;
	result.text.textAlign = textAlign;
	result.text.fontSize = fontSize;
	return(result);
}

constexpr size_t MaxBlockCount = 16;
struct SlideDefinition {
	const char *name;
	BlockDefinition blocks[MaxBlockCount];
	Background background;
	size_t count;
};

template<size_t N>
static SlideDefinition MakeSlideDef(const char *name, BlockDefinition(&blocks)[N], const Background &background) {
	fplAssert(N < MaxBlockCount);
	SlideDefinition result = {};
	result.name = name;
	result.background = background;
	for (size_t i = 0; i < N; ++i) {
		result.blocks[i] = blocks[i];
	}
	result.count = N;
	return(result);
}

struct FontDefinition {
	const char *name;
	float size;
	float lineScale;
	TextStyle style;
};

struct HeaderDefinition {
	FontDefinition font;
	float height;
	const char *leftText;
	const char *centerText;
	const char *rightText;
	Vec2f padding;
};

struct FooterDefinition {
	FontDefinition font;
	float height;
	const char *leftText;
	const char *centerText;
	const char *rightText;
	Vec2f padding;
};

struct PresentationDefinition {
	const SlideDefinition *slides;
	size_t slideCount;
	Vec2f slideSize;
	HeaderDefinition header;
	FooterDefinition footer;
	FontDefinition titleFont;
	FontDefinition normalFont;
	FontDefinition consoleFont;
	float padding;
};

namespace FPLPresentationData {
	static Background DarkBlueBack = MakeBackground(RGBAToLinearRaw(0, 0, 0, 255), RGBAToLinearRaw(15, 13, 80, 255));
	
	// DarkBlue to LightBlue
	static Background Background1 = MakeBackground(RGBAToLinearHex24(0x000000), RGBAToLinearHex24(0x333789));
	static Background Background2 = MakeBackground(RGBAToLinearHex24(0x000000), RGBAToLinearHex24(0x1E5AA3));
	static Background Background3 = MakeBackground(RGBAToLinearHex24(0x000000), RGBAToLinearHex24(0x0483AF));

	// Green to Yellow
	static Background Background4 = MakeBackground(RGBAToLinearHex24(0x000000), RGBAToLinearHex24(0x057F47));
	static Background Background5 = MakeBackground(RGBAToLinearHex24(0x000000), RGBAToLinearHex24(0x7AB30B));
	static Background Background6 = MakeBackground(RGBAToLinearHex24(0x000000), RGBAToLinearHex24(0xF2E500));

	// LightOrange to DarkOrange
	static Background Background7 = MakeBackground(RGBAToLinearHex24(0x000000), RGBAToLinearHex24(0xFDBD00));
	static Background Background8 = MakeBackground(RGBAToLinearHex24(0x000000), RGBAToLinearHex24(0xEE7B00));
	static Background Background9 = MakeBackground(RGBAToLinearHex24(0x000000), RGBAToLinearHex24(0xE54B0A));

	// Red to Purple
	static Background Background10 = MakeBackground(RGBAToLinearHex24(0x000000), RGBAToLinearHex24(0xDC0012));
	static Background Background11 = MakeBackground(RGBAToLinearHex24(0x000000), RGBAToLinearHex24(0xB7006A));
	static Background Background12 = MakeBackground(RGBAToLinearHex24(0x000000), RGBAToLinearHex24(0x59227A));

	static Background Backgrounds[] = {
		Background1,
		Background2,
		Background3,
		Background4,
		Background5,
		Background6,
		Background7,
		Background8,
		Background9,
		Background10,
		Background11,
		Background12,
	};

	static int BackgroundIndex = 0;

	static Background GetBackground() {
		Background result = Backgrounds[BackgroundIndex];
		BackgroundIndex = (BackgroundIndex + 1) % fplArrayCount(Backgrounds);
		return(result);
	}

	static BlockDefinition IntroBlocks[] = {
		MakeTextDef(
			V2f(),V2f(1,1),MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
			"Introducing Final-Platform-Layer (FPL)\n"
			"A lightweight Platform-Abstraction-Library written in C99\n"
		, HorizontalAlignment::Center),
	};
	static const SlideDefinition IntroSlide = MakeSlideDef("Introduction", IntroBlocks, GetBackground());

	static BlockDefinition WhatIsAPALBlocks[] = {
		MakeTextDef(
			V2f(),V2f(1,1),MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
			"A Platform-Abstraction-Library (or short PAL)\n"
			"is a library written in a low-level language, such as C\n"
			"used to access hardware and low-level systems\n"
			"in a platform-independent way.\n"
		, HorizontalAlignment::Center),
	};
	static const SlideDefinition WhatIsAPALSlide = MakeSlideDef("What is a PAL", WhatIsAPALBlocks, GetBackground());

	static BlockDefinition WhatIsFPLBlocks[] = {
		MakeTextDef(
			V2f(),V2f(1,1),MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
			"FPL is an all-purpose/multimedia PAL\n"
			"providing a powerful and easy to use API,\n"
			"for working with low-level and hardware systems\n"
			"written in C99.\n"
		, HorizontalAlignment::Center),
	};
	static const SlideDefinition WhatIsFPLSlide = MakeSlideDef("What is FPL", WhatIsFPLBlocks, GetBackground());

	static BlockDefinition FeaturesOfFPLBlocks[] = {
		MakeTextDef(
			V2f(),V2f(1,1),MakeAlign(HorizontalAlignment::Center),
			"- Platform detection (x86/x64/Arm, Win32/Linux/Unix, etc.)\n"
			"- Compiler detection (MSVC/GCC/Clang/Intel)\n"
			"- Macros (Debug break, Assertions, CPU-Features, Memory etc.)\n"
			"- Window creation and handling (Win32/X11)\n"
			"- Event and input polling (Keyboard/Mouse/Gamepad)\n"
			"- Video initialization and output (Software, OpenGL, etc.)\n"
			"- Asynchronous audio playback (DirectSound, ALSA, etc.)\n"
			"- IO (Console, Paths, Files, Directories, etc.)\n"
			"- Memory handling\n"
			"- Dynamic library loading (.dll/.so)\n"
			"- Multi threading (Atomics, Threads, Mutex, Semaphores, Conditionals, etc.)\n"
			"- Retrieving hardware information\n"
			"- and many more\n"
		, HorizontalAlignment::Left, FeaturesFontSize),
	};
	static const SlideDefinition FeaturesOfFPLSlide = MakeSlideDef("Features of FPL", FeaturesOfFPLBlocks, GetBackground());

	static BlockDefinition MotivationBlocks[] = {
		MakeTextDef(
			V2f(0.0, 0.0),V2f(1.0, 0.25),MakeAlign(HorizontalAlignment::Center),
			"C/C++ has very limited access to the underlying platform,\n"
			"so you have to use third-party libraries to access low level systems\n"
			"or write platform specific codes directly.\n"
		, HorizontalAlignment::Left),
		MakeTextDef(
			V2f(0.0, 0.25),V2f(1.0, 0.75),MakeAlign(HorizontalAlignment::Center),
			"The pre-existing PALs have a lot of issues:\n"
			"- Huge in file count and/or size\n"
			"- Huge in number of translation units\n"
			"- Huge in memory usage and number of allocations\n"
			"- Without configuration and/or build systems you cant compile it\n"
			"- Statically linking is madness or not supported at all\n"
			"- Including the full source is either impossible or extremely cumbersome\n"
			"- It takes forever to compile\n"
			"- No control over the allocated memory, at max you can overwrite malloc/free\n"
			"- Some are built on top of third-party dependencies\n"
			"- Some are heavily bloated\n"
		, HorizontalAlignment::Left, FeaturesFontSize),
	};
	static const SlideDefinition MotivationSlide = MakeSlideDef("Motivation", MotivationBlocks, GetBackground());

	static BlockDefinition WhyBlocks[] = {
		MakeTextDef(
			V2f(0.0, 0.0),V2f(1.0, 0.25),MakeAlign(HorizontalAlignment::Center),
			"I just want to include one header file and thats it.\n"
			"I want it to compile very fast.\n"
			"I dont want it to require any third-party libraries.\n"
		, HorizontalAlignment::Left),
		MakeTextDef(
			V2f(0.0, 0.25),V2f(1.0, 0.75),MakeAlign(HorizontalAlignment::Center),
			"- FPL is designed to require bare minimum linking to the OS (kernel32.lib / libld.so) only\n"
			"- It does not require any dependencies or build-systems to get it running/compiling\n"
			"- It prevents using features from the C-Runtime library, to support lightweight environments\n"
			"- It compiles blazingly fast, even on VC++\n"
			"- It uses a fixed and small memory footprint and handles memory very gracefully\n"
			"- It does not use malloc/free, the memory is allocated/freed using OS system calls directly\n"
			"- No data hiding -> everything is accessible\n"
			"- You decide how to integrate it; not the library\n"
		, HorizontalAlignment::Left, FeaturesFontSize),
	};
	static const SlideDefinition WhySlide = MakeSlideDef("Why FPL", WhyBlocks, GetBackground());

	static BlockDefinition HowItWorksBlocks[] = {
		MakeTextDef(
			V2f(),V2f(1,1),MakeAlign(HorizontalAlignment::Center),
			"- It is written in pure C99 for simplicity and best portability - but is 100%% C++ compatible\n"
			"- It makes heavy use of the pre-compiler to detect compiler/platform/hardware/driver configurations\n"
			"- It uses runtime linking by default, so it does not need any libs to be included\n"
			"- It prevents code-duplication by using sub-platform blocks (Unix vs Linux)\n"
			"- It is stateless, meaning the user does not have to provide any application states\n"
			"- Everything is included in one C file\n"
		, HorizontalAlignment::Left, FeaturesFontSize),

	};
	static const SlideDefinition HowItWorksSlide = MakeSlideDef("How it works", HowItWorksBlocks, GetBackground());

	static BlockDefinition DemosBlocks[] = {
		MakeTextDef(
			V2f(),V2f(1,1),MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
			"Demo-Time!"
		, HorizontalAlignment::Center),

	};
	static const SlideDefinition DemosSlide = MakeSlideDef("Demos!", DemosBlocks, GetBackground());

	static const SlideDefinition Slides[] = {
		IntroSlide,
		WhatIsAPALSlide,
		WhatIsFPLSlide,
		FeaturesOfFPLSlide,
		MotivationSlide,
		WhySlide,
		HowItWorksSlide,
		DemosSlide,
	};

	static const Vec4f ForegroundColor = RGBAToLinearRaw(255, 255, 255, 255);
	static const Vec4f TextShadowColor = RGBAToLinearRaw(1, 84, 164, 200);
	static const Vec2f TextShadowOffset = V2f(2, 1);

	static const TextStyle BasicStyle = {
		/* background */		{},
		/* foreground */		ForegroundColor,
		/* shadowColor */		TextShadowColor,
		/* shadowOffset */		TextShadowOffset,
		/* drawShadow */		true,
	};

	static const TextStyle HeaderStyle = {
		/* background */		{},
		/* foreground */		V4f(1, 1, 1, 1),
		/* shadowColor */		V4f(0, 0, 0, 1),
		/* shadowOffset */		V2f(1, 1),
		/* drawShadow */		true,
	};

	static const HeaderDefinition  Header = {
		/* font */ {FontResources::Arimo.name, 24.0f, 1.15f, FPLPresentationData::HeaderStyle},
		/* height */ 32.0f,
		/* leftText */ "Final-Platform-Layer",
		/* centerText */ "",
		/* rightText */ "",
		/* padding */ V2f(2,2),
	};
	static const FooterDefinition  Footer = {
		/* font */ {FontResources::Arimo.name, 24.0f, 1.15f, FPLPresentationData::HeaderStyle},
		/* height */ 32.0f,
		/* leftText */ "%SLIDE_NAME%",
		/* centerText */ "Copyright (C) 2020 Torsten Spaete",
		/* rightText */ "Page %SLIDE_NUM% of %SLIDE_COUNT%",
		/* padding */ V2f(2,3),
	};
};

static const PresentationDefinition FPLPresentation = {
	/* slides */		FPLPresentationData::Slides,
	/* slideCount */ 	fplArrayCount(FPLPresentationData::Slides),
	/* slideSize */ 	V2f(1280.0f,720.0f),
	/* header */ 		FPLPresentationData::Header,
	/* footer */ 		FPLPresentationData::Footer,
	/* titleFont */ 	{FontResources::Arimo.name, 64.0f, 1.15f, FPLPresentationData::BasicStyle},
	/* normalFont */ 	{FontResources::Arimo.name, 42.0f, 1.15f, FPLPresentationData::BasicStyle},
	/* consoleFont */ 	{FontResources::BitStreamVerySans.name, 36.0f, 1.15f, FPLPresentationData::BasicStyle},
	/* padding */ 	 	20.0f,
};