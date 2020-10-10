#pragma once

#include <final_platform_layer.h>

#include <final_math.h>

#include "types.h"
#include "fonts.h"

static const float FeaturesFontSize = 32.0f;

struct TextBlockDefinition {
	const char* text;
	HorizontalAlignment hAlign;
	VerticalAlignment vAlign;
	float fontSize;

	TextBlockDefinition(const char* text, HorizontalAlignment hAlign, VerticalAlignment vAlign, float fontSize = 0.0f) {
		this->text = text;
		this->hAlign = hAlign;
		this->vAlign = vAlign;
		this->fontSize = fontSize;
	}

	TextBlockDefinition() : TextBlockDefinition(nullptr, HorizontalAlignment::Left, VerticalAlignment::Top) {
	}
};

struct SlideDefinition {
	const char* name;
	TextBlockDefinition items[4];
	size_t count;
};

static SlideDefinition MakeSlideDef(const char* name, const TextBlockDefinition& a) {
	SlideDefinition result = {};
	result.name = name;
	result.items[0] = a;
	result.count = 1;
	return(result);
}

static SlideDefinition MakeSlideDef(const char* name, const TextBlockDefinition& a, const TextBlockDefinition& b) {
	SlideDefinition result = {};
	result.name = name;
	result.items[0] = a;
	result.items[1] = b;
	result.count = 2;
	return(result);
}

struct FontDefinition {
	const char* name;
	float size;
	float lineScale;
	TextStyle style;
};

struct HeaderDefinition {
	FontDefinition font;
	float height;
	const char* leftText;
	const char* centerText;
	const char* rightText;
	Vec2f padding;
};

struct FooterDefinition {
	FontDefinition font;
	float height;
	const char* leftText;
	const char* centerText;
	const char* rightText;
	Vec2f padding;
};

struct PresentationDefinition {
	const SlideDefinition* slides;
	size_t slideCount;
	Vec2f slideSize;
	HeaderDefinition header;
	FooterDefinition footer;
	FontDefinition titleFont;
	FontDefinition normalFont;
	FontDefinition consoleFont;
	Background background;
	float padding;
};

namespace FPLPresentationData {
	static const SlideDefinition IntroSlide = MakeSlideDef(
		"Introduction",
		TextBlockDefinition((
			"Introducing Final-Platform-Layer (FPL)\n"
			"A lightweight Platform-Abstraction-Library written in C99\n"
		), HorizontalAlignment::Center, VerticalAlignment::Middle)
	);

	static const SlideDefinition WhatIsAPALSlide = MakeSlideDef(
		"What is a PAL",
		TextBlockDefinition((
			"A Platform-Abstraction-Library (or short PAL)\n"
			"is a library written in a low-level language, such as C\n"
			"used to access hardware and low-level systems\n"
			"in a platform-independent way.\n"
		), HorizontalAlignment::Center, VerticalAlignment::Middle)
	);

	static const SlideDefinition WhatIsFPLSlide = MakeSlideDef(
		"What is FPL",
		TextBlockDefinition((
			"FPL is an all-purpose/multimedia PAL\n"
			"providing a powerful and easy to use API,\n"
			"for working with low-level and hardware systems\n"
			"written in C99.\n"
			), HorizontalAlignment::Center, VerticalAlignment::Middle)
	);

	static const SlideDefinition FeaturesOfFPLSlide = MakeSlideDef(
		"Features of FPL",
		TextBlockDefinition((
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
		), HorizontalAlignment::Left, VerticalAlignment::Top, FeaturesFontSize)
	);

	static const SlideDefinition MotivationSlide = MakeSlideDef(
		"Motivation",
		TextBlockDefinition((
			"C/C++ has very limited access to the underlying platform,\n"
			"so you have to use third-party libraries to access low level systems\n"
			"or write platform specific codes directly.\n"
		), HorizontalAlignment::Left, VerticalAlignment::Top),
		TextBlockDefinition((
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
		), HorizontalAlignment::Left, VerticalAlignment::Bottom, FeaturesFontSize)
	);

	static const SlideDefinition WhySlide = MakeSlideDef(
		"Why FPL",
		TextBlockDefinition((
			"I just want to include one header file and thats it.\n"
			"I want it to compile very fast - with the full implementation for any platform/compiler i need.\n"
			"I dont want it to require any third-party libraries - not even the C-Runtime.\n"
		), HorizontalAlignment::Left, VerticalAlignment::Top),
		TextBlockDefinition((
			"- FPL is designed to require bare minimum linking to the OS (kernel32.lib / libld.so) only\n"
			"- It does not require any dependencies or build-systems to get it running/compiling\n"
			"- It prevents using features from the C-Runtime library, to support lightweight environments\n"
			"- It compiles blazingly fast, even on VC++\n"
			"- It uses a fixed and small memory footprint and handles memory very gracefully\n"
			"- It does not use malloc/free, the memory is allocated/freed using OS system calls directly\n"
			"- No data hiding -> everything is accessible\n"
			"- You decide how to integrate it; not the library\n"
			), HorizontalAlignment::Left, VerticalAlignment::Bottom, FeaturesFontSize)
	);

	static const SlideDefinition HowItWorks = MakeSlideDef(
		"How it works",
		TextBlockDefinition((
			"- It is written in pure C99 for simplicity and best portability - but is 100%% C++ compatible\n"
			"- It makes heavy use of the pre-compiler to detect compiler/platform/hardware/driver configurations\n"
			"- It uses runtime linking by default, so it does not need any libs to be included\n"
			"- It prevents code-duplication by using sub-platform blocks (Unix vs Linux)\n"
			"- It is stateless, meaning the user does not have to provide any application states\n"
			"- Everything is included in one C file\n"
		), HorizontalAlignment::Left, VerticalAlignment::Top, FeaturesFontSize)
	);

	static const SlideDefinition DemosExclamationMark = MakeSlideDef(
		"Demos!",
		TextBlockDefinition((
			"Demo-Time!"
		), HorizontalAlignment::Center, VerticalAlignment::Middle)
	);

	static const SlideDefinition Slides[] = {
		IntroSlide,
		WhatIsAPALSlide,
		WhatIsFPLSlide,
		FeaturesOfFPLSlide,
		MotivationSlide,
		WhySlide,
		HowItWorks,
		DemosExclamationMark,
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

	static const Background Back = { BackgroundKind::HalfGradientHorizontal, RGBAToLinearRaw(0, 0, 0, 255), RGBAToLinearRaw(15, 13, 80, 255) };

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
	/* background */ 	FPLPresentationData::Back,
	/* padding */ 	 	20.0f,
};