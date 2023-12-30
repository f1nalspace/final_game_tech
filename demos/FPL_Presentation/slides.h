#pragma once

#include <final_platform_layer.h>

#include <final_math.h>

#include "types.h"
#include "fonts.h"

static const float CodeFontSize = 24.0f;
static const float FeaturesFontSize = 32.0f;

struct SoundDefinition {
	AudioSourceID id;
	double startTime;
	double length;
};

enum class BlockType {
	None = 0,
	Text,
	Image,
};

struct TextBlockDefinition {
	Vec4f color;
	const char* text;
	float fontSize;
	HorizontalAlignment textAlign;
};

struct ImageBlockDefinition {
	const char* name;
	Vec2f size;
	bool keepAspect;
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
		ImageBlockDefinition image;
	};
};

static BlockDefinition MakeTextDef(const Vec2f& pos, const Vec2f& size, BlockAlignment contentAlignment, const char* text, const HorizontalAlignment textAlign = HorizontalAlignment::Left, const float fontSize = 0, const Vec4f &color = V4f(1,1,1,1)) {
	BlockDefinition result = {};
	result.pos = pos;
	result.size = size;
	result.contentAlignment = contentAlignment;
	result.type = BlockType::Text;
	result.text.text = text;
	result.text.textAlign = textAlign;
	result.text.fontSize = fontSize;
	result.text.color = color;
	return(result);
}

static BlockDefinition MakeImageDef(const Vec2f& pos, const Vec2f& size, BlockAlignment contentAlignment, const char* imageName, const Vec2f& imageSize, const bool keepAspect) {
	BlockDefinition result = {};
	result.pos = pos;
	result.size = size;
	result.contentAlignment = contentAlignment;
	result.type = BlockType::Image;
	result.image.name = imageName;
	result.image.size = imageSize;
	result.image.keepAspect = keepAspect;
	return(result);
}

constexpr size_t MaxBlockCount = 16;
constexpr size_t MaxAudioSoundCount = 4;

struct SlideDefinition {
	const char* name;
	BlockDefinition blocks[MaxBlockCount];
	SoundDefinition sounds[MaxAudioSoundCount];
	BackgroundStyle background;
	Quaternion rotation;
	size_t blockCount;
	size_t soundCount;
};

template<size_t N>
static SlideDefinition MakeSlideDef(const char* name, BlockDefinition(&blocks)[N], const BackgroundStyle &background, const Quaternion &rotation) {
	fplAssert(N < MaxBlockCount);

	SlideDefinition result = {};
	result.name = name;
	result.background = background;
	for(size_t i = 0; i < N; ++i) {
		result.blocks[i] = blocks[i];
	}
	result.rotation = rotation;
	result.blockCount = N;
	return(result);
}

template<size_t NBlocks, size_t NSounds>
static SlideDefinition MakeSlideDef(const char* name, BlockDefinition(&blocks)[NBlocks], SoundDefinition(&sounds)[NSounds], const BackgroundStyle &background, const Quaternion &rotation) {
	fplAssert(NBlocks < MaxBlockCount);
	fplAssert(NSounds < MaxAudioSoundCount);

	SlideDefinition result = {};
	result.name = name;
	result.background = background;
	for(size_t i = 0; i < NBlocks; ++i) {
		result.blocks[i] = blocks[i];
	}
	for(size_t i = 0; i < NSounds; ++i) {
		result.sounds[i] = sounds[i];
	}
	result.rotation = rotation;
	result.blockCount = NBlocks;
	result.soundCount = NSounds;
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
	float padding;
};

namespace FPLPresentationData {
	static BackgroundStyle DarkBlueBack = MakeBackground(RGBAToLinearHex24(0x000000, 255), RGBAToLinearRaw(15, 13, 80, 255));

	// DarkBlue to LightBlue
	static BackgroundStyle Background1 = MakeBackground(RGBAToLinearHex24(0x000000, 255), RGBAToLinearHex24(0x333789, 100));
	static BackgroundStyle Background2 = MakeBackground(RGBAToLinearHex24(0x000000, 255), RGBAToLinearHex24(0x1E5AA3, 100));
	static BackgroundStyle Background3 = MakeBackground(RGBAToLinearHex24(0x000000, 255), RGBAToLinearHex24(0x0483AF, 100));

	// Green to Yellow
	static BackgroundStyle Background4 = MakeBackground(RGBAToLinearHex24(0x000000, 255), RGBAToLinearHex24(0x057F47, 100));
	static BackgroundStyle Background5 = MakeBackground(RGBAToLinearHex24(0x000000, 255), RGBAToLinearHex24(0x7AB30B, 100));
	static BackgroundStyle Background6 = MakeBackground(RGBAToLinearHex24(0x000000, 255), RGBAToLinearHex24(0xF2E500, 100));

	// LightOrange to DarkOrange
	static BackgroundStyle Background7 = MakeBackground(RGBAToLinearHex24(0x000000, 255), RGBAToLinearHex24(0xFDBD00, 100));
	static BackgroundStyle Background8 = MakeBackground(RGBAToLinearHex24(0x000000, 255), RGBAToLinearHex24(0xEE7B00, 100));
	static BackgroundStyle Background9 = MakeBackground(RGBAToLinearHex24(0x000000, 255), RGBAToLinearHex24(0xE54B0A, 100));
	static BackgroundStyle Background10 = MakeBackground(RGBAToLinearHex24(0x000000, 255), RGBAToLinearHex24(0xE54B0A, 200));

	// Red to Purple
	static BackgroundStyle Background11 = MakeBackground(RGBAToLinearHex24(0x000000, 255), RGBAToLinearHex24(0xDC0012, 100));
	static BackgroundStyle Background12 = MakeBackground(RGBAToLinearHex24(0x000000, 255), RGBAToLinearHex24(0xB7006A, 100));
	static BackgroundStyle Background13 = MakeBackground(RGBAToLinearHex24(0x000000, 255), RGBAToLinearHex24(0x59227A, 100));

	static BackgroundStyle Backgrounds[] = {
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
		Background13,
	};

	static int BackgroundIndex = 0;

	static BackgroundStyle GetBackground() {
		BackgroundStyle result = Backgrounds[BackgroundIndex];
		BackgroundIndex = (BackgroundIndex + 1) % fplArrayCount(Backgrounds);
		return(result);
	}

	namespace Intro {
		static const char* Talk = {
			"Hi! I am Torsten"
			"Today, i would like to introduce you to a project of mine that i have been working on for several years"
			"It is called 'Final Platform Layer' or short FPL and is a platform abstraction library for C and C++"
		};

		static BlockDefinition Blocks[] = {
			MakeTextDef(
				V2f(),V2f(1,1),MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				"Introducing Final-Platform-Layer (FPL)\n"
				"A lightweight Platform-Abstraction-Library written in C99\n",
				HorizontalAlignment::Center
			),
		};

		static Quaternion Rot = QuatIdentity();

		static const SlideDefinition Slide = MakeSlideDef("Introduction", Blocks, GetBackground(), Rot);
	};

	namespace WhoAmI {
		static const char* Talk = {
			"I am Torsten Spaete"
			"A professional software engineer with more than 25 years of programming experience"
			""
			"My main focus is data-visualization, software-architecture, multimedia and game programming"
		};

		static BlockDefinition Blocks[] = {
			MakeTextDef(
				V2f(),V2f(1,1),MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				"I am Torsten Spaete\n"
				"A professional software engineer\n"
				"25+ years of programming experience\n",
				HorizontalAlignment::Center
			),
		};

		static Quaternion Rot = QuatFromAngleAxis(DegreesToRadians(15), V3f(1, 1, 1));

		static const SlideDefinition Slide = MakeSlideDef("Who am I", Blocks, GetBackground(), Rot);		
	};

	namespace WhatIsFPL {
		static const char* Talk = {
			"Final-Platform-Layer (or short 'FPL') is a lightweight platform-abstraction library written in C99."
			"With FPL you get access to low-level systems, hardware devices and operating system functions,"
			"in a platform independent and easy-to-use API."
			""
			"For example you can:"
			"- Play audio samples on any audio device"
			"- Create and manage windows, including the initialization of a video graphics api, such as OpenGL, Vulkan"
			"- Query keyboard, mouse and gamepad input"
			"- Execute code in parallel, using several multithreading techniques"
			"- Allocate and free memory directly on the platform, without the use of malloc or free"
			"- Modify and query files & directories"
			"- Query several platform and hardware informations"
			"- and much more"
			""
			"Its main usage is multimedia and game development, but it can be used to write any kind of application."
			"'FPL' is designed to be fast in compile and run time and can be integrated however you like."
		};

		static BlockDefinition Blocks[] = {
			MakeTextDef(
				V2f(),V2f(1,1),MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				"Final-Platform-Layer (or short 'FPL')\n"
				"is a lightweight library written in C99.\n"
				"\n"
				"It provides a powerful and easy to use API,\n"
				"that gives you access to low-level systems, hardware devices,\n"
				"operating system functions and many more.",
				HorizontalAlignment::Left
			),
		};

		static Quaternion Rot = QuatFromAngleAxis(DegreesToRadians(-45), V3f(0, 1, 1));

		static const SlideDefinition Slide = MakeSlideDef("What is FPL", Blocks, GetBackground(), Rot);
	};

	namespace Motivation {
		static const char* Talk = {
			"C/C++ has very limited access to the underlying platform."
			"Even in modern C++, you still don't have direct access to a lot of systems."
			"To access low-level systems, such as audio or video, you either need to use third-party libraries or write platform-specific codes for Win32, Linux, Mac, etc. directly."
			""
			"Of course, there is already a few PALs out there, but they have a lot of issues:"
			"- The source-codes contain dozens of translation units which slows down compilation time enormously"
			"- All are designed to not include the full source within your application and force you either to static or dynamic linked pre-compiled releases"
			"- You have limited or no control over memory allocations"
			"- They have too many dependencies (build-systems, third-party libraries, etc.)"
			"- There are not simple to integrate into your application or development environment"
		};

		static BlockDefinition Blocks[] = {
			MakeTextDef(
				V2f(0.05f, 0.0f),V2f(0.9f, 0.25),MakeAlign(HorizontalAlignment::Left, VerticalAlignment::Top),
				"C/C++ has very limited access to the underlying platform,\n"
				"so you have either use third-party libraries to access the platform or\n"
				"write platform specific codes directly.\n",
				HorizontalAlignment::Left
			),

			MakeTextDef(
				V2f(0.05f, 0.3f),V2f(0.9f, 0.7f),MakeAlign(HorizontalAlignment::Left, VerticalAlignment::Top),
				"The pre-existing PALs have a lot of disavantages:\n"
				"- Very long compile times, due to large number of files\n"
				"- Only static or dynamic linking possible\n"
				"- Not allowed/possible to use the source directly\n"
				"- Limited or no control over the allocated memory\n"
				"- Too many dependencies\n"
				"- Not easy to integrate\n",
				HorizontalAlignment::Left, FeaturesFontSize
			),
		};

		static Quaternion Rot = QuatFromAngleAxis(DegreesToRadians(70), V3f(1, 0.1f, 0.0f));

		static const SlideDefinition Slide = MakeSlideDef("Motivation", Blocks, GetBackground(), Rot);
	};

	namespace Goals {
		static const char* Talk = {
			"That builds up the following goals:"
			"- It should be a single-file library"
			"- It should compile very fast, even with slow hardware or in slow environments"
			"- It should be written in C99, so its highly compatible"
			"- It should not require any third-party dependencies and have bare minimum linking requirements"
			"- It should use a small memory footprint and give the user control over any memory allocations"
			"- It should support runtime and static linking and can be integrated with full source"
			"- It starts with good default settings but can be changed by the user"
		};

		static BlockDefinition Blocks[] = {
			MakeTextDef(
				V2f(0.0, 0.0),V2f(1.0, 1.0),MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Top),
				"- Fast compile times, even in slow environments\n"
				"- Based on C with 100%% C++ compatibility\n"
				"- Bare minimum compile and linking requirements\n"
				"- Small memory footprint\n"
				"- Can be integrated in any way\n"
				"- C-Runtime library should not be required\n"
				"- Features can be disabled, if not needed\n"
				"- Simple and easy to understand API\n"
				"- Configurable with good defaults\n"
				"- Public open source\n",
				HorizontalAlignment::Left, FeaturesFontSize * 1.1f
			),
		};

		static Quaternion Rot = QuatFromAngleAxis(DegreesToRadians(-30), V3f(1, 0, 1.0));

		static const SlideDefinition Slide = MakeSlideDef("Goals of FPL", Blocks, GetBackground(), Rot);
	};

	namespace WhyFPL {
		static const char* Talk = {
			""
		};

		static BlockDefinition Blocks[] = {
			MakeTextDef(
				V2f(0.0, 0.0),V2f(1.0, 1.0),MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Top),
				"- You get access to low-level systems in a nice and clean API\n"
				"- One file containing all the source code\n"
				"- Written in pure C99 for simplicity and best portability\n"
				"- Compiles very fast on all modern C99/C++ compilers\n"
				"- Has bare minimum compile and linking requirements\n"
				"- Uses runtime linking by default, so no libs needs to be included\n"
				"- Allows to control the memory allocations and handles memory very gracefully\n"
				"- Multiple backends for Video/Audio/Input/Window\n"
				"- Supports runtime linking or static linking or even full-source inclusion\n"
				"- MIT-Licensed\n",
				HorizontalAlignment::Left, FeaturesFontSize * 1.1f
			),
		};

		static Quaternion Rot = QuatFromAngleAxis(DegreesToRadians(-60), V3f(0.0f, 0.0f, 1.0f));

		static const SlideDefinition Slide = MakeSlideDef("Why FPL", Blocks, GetBackground(), Rot);
	};

	namespace FeaturesOfFPL {
		static const char* Talk = {
			""
		};

		static BlockDefinition Blocks[] = {
			MakeTextDef(
				V2f(0,0),V2f(1.0f,1.0f),MakeAlign(HorizontalAlignment::Center),
				"\n"
				"- Supports dynamic linking, static linking and full source inclusion\n"
				"- Platform detection (x86/x64/Arm, Win32/Linux/Unix, etc.)\n"
				"- Compiler detection (MSVC/GCC/Clang/Intel)\n"
				"- Macros (debug break, assertions, CPU features, memory etc.)\n"
				"- Window creation and handling (Win32/X11)\n"
				"- Event and input polling (keyboard/mouse/gamepad)\n"
				"- Video initialization and output (software, OpenGL, Vulkan, etc.)\n"
				"- Asynchronous audio playback (DirectSound, ALSA, etc.)\n"
				"- IO (console, paths, files, directories, etc.)\n"
				"- Memory allocation\n"
				"- Dynamic library loading (.dll/.so)\n"
				"- Multi threading (atomics, threads, mutexes, semaphores, conditionals, etc.)\n"
				"- Retrieving hardware information\n"
				"- and many more\n",
				HorizontalAlignment::Left, FeaturesFontSize
			),
		};

		static Quaternion Rot = QuatFromAngleAxis(DegreesToRadians(30), V3f(0.0f, 1.0f, 0.1f));

		static const SlideDefinition Slide = MakeSlideDef("Features of FPL", Blocks, GetBackground(), Rot);
	};

	namespace Magic {
		static const char* Talk = {
			""
		};

		static BlockDefinition Blocks[] = {
			MakeTextDef(
				V2f(0.0f,0.0f),V2f(1.0f,1.0f),MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				"Magic!",
				HorizontalAlignment::Left, FeaturesFontSize * 4
			),			
		};

		static Quaternion Rot = QuatFromAngleAxis(DegreesToRadians(-15), V3f(0.7f, 0.6f, 0.3f));

		static const SlideDefinition Slide = MakeSlideDef("How it works", Blocks, GetBackground(), Rot);
	};

	namespace HowItWorks {
		static const char* Talk = {
			""
		};

		static BlockDefinition Blocks[] = {
			MakeTextDef(
				V2f(0.0f,0.0f),V2f(1.0f,1.0f),MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				"- Single-header-file library, containing the full API and source for every platform\n"
				"\n"
				"- Uses the pre-compiler to detect compiler/platform/hardware/driver setups\n"
				"\n"
				"- Prevents code-duplication by using sub-platforms (Unix vs Linux)\n"
				"\n"
				"- Uses runtime linking by default\n",
				HorizontalAlignment::Left, FeaturesFontSize
			),
		};

		static Quaternion Rot = QuatFromAngleAxis(DegreesToRadians(45), V3f(0.5f, 0.5f, 0.5f));

		static const SlideDefinition Slide = MakeSlideDef("How it actually works", Blocks, GetBackground(), Rot);
	};

	namespace HowToUse {
		static const char* Talk = {
			""
		};

		static BlockDefinition Blocks[] = {
			MakeTextDef(
				V2f(0.0f, 0.1f),V2f(1.0f, 1.0f), MakeAlign(HorizontalAlignment::Left, VerticalAlignment::Top),
				"Using FPL is straight forward\n"
				"\n"
				"- Copy the 'final_platform_layer.h' file into your project\n"
				"\n"
				"- In your main translation unit:\n"
				"- #define FPL_IMPLEMENTATION\n"
				"- #include <final_platform_layer.h>\n"
				"\n"
				"- In your entry point (main):\n"
				"- Create default or custom settings\n"
				"- Initialize FPL\n"
				"- Use any code you want\n"
				"- Release FPL",
				HorizontalAlignment::Left, CodeFontSize, V4f(0.0f, 0.8f, 0.2f, 1.0f)
			),

			MakeImageDef(
				V2f(-0.05f, 0.1f), V2f(1.0f, 1.0f), MakeAlign(HorizontalAlignment::Right, VerticalAlignment::Top),
				"FPL Minimum Source", V2f(0.5f, 1.0f), true
			),
		};

		static Quaternion Rot = QuatFromAngleAxis(DegreesToRadians(75), V3f(0.0f, 1.0f, 0));

		static const SlideDefinition Slide = MakeSlideDef("How to use", Blocks, GetBackground(), Rot);
	};

	namespace Demos {
		static const char* Talk = {
			""
		};

		static BlockDefinition Blocks[] = {
			MakeTextDef(
				V2f(0.0f, 0.3f),V2f(1.0f, 0.1f),MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				"FPL comes with a variety of demos:",
				HorizontalAlignment::Center, FeaturesFontSize
			),
			MakeTextDef(
				V2f(0.0f, 0.4f),V2f(1.0f, 0.1f),MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				"https://github.com/f1nalspace/final_game_tech/tree/master/demos",
				HorizontalAlignment::Center, FeaturesFontSize, V4f(0.0f, 0.8f, 0.2f, 1.0f)
			),
		};

		static Quaternion Rot = QuatFromAngleAxis(DegreesToRadians(75), V3f(0.0f, 1.0f, 0));

		static const SlideDefinition Slide = MakeSlideDef("Demos!", Blocks, GetBackground(), Rot);
	};

	namespace Links {
		static const char* Talk = {
			""
		};

		static BlockDefinition Blocks[] = {
			MakeTextDef(
				V2f(0.25f, 0.0f), V2f(0.5f, 0.1f), MakeAlign(HorizontalAlignment::Left, VerticalAlignment::Middle),
				"Official website (details, documentation):",
				HorizontalAlignment::Left, FeaturesFontSize
			),
			MakeTextDef(
				V2f(0.25f, 0.1f), V2f(0.5f, 0.1f), MakeAlign(HorizontalAlignment::Left, VerticalAlignment::Middle),
				"https://libfpl.org",
				HorizontalAlignment::Left, FeaturesFontSize, V4f(0.0f, 0.8f, 0.2f, 1.0f)
			),
			MakeTextDef(
				V2f(0.25f, 0.3f), V2f(0.5f, 0.1f), MakeAlign(HorizontalAlignment::Left, VerticalAlignment::Middle),
				"Github (sources, issue-tracker, project management):",
				HorizontalAlignment::Left, FeaturesFontSize
			),
			MakeTextDef(
				V2f(0.25f, 0.4f), V2f(0.5f, 0.1f), MakeAlign(HorizontalAlignment::Left, VerticalAlignment::Middle),
				"https://github.com/f1nalspace/final_game_tech",
				HorizontalAlignment::Left, FeaturesFontSize, V4f(0.0f, 0.8f, 0.2f, 1.0f)
			),
			MakeTextDef(
				V2f(0.25f, 0.6f), V2f(0.5f, 0.1f), MakeAlign(HorizontalAlignment::Left, VerticalAlignment::Middle),
				"Handmade-Network (community):",
				HorizontalAlignment::Left, FeaturesFontSize
			),
			MakeTextDef(
				V2f(0.25f, 0.7f), V2f(0.5f, 0.1f), MakeAlign(HorizontalAlignment::Left, VerticalAlignment::Middle),
				"https://fpl.handmade.network",
				HorizontalAlignment::Left, FeaturesFontSize, V4f(0.0f, 0.8f, 0.2f, 1.0f)
			),
		};

		static Quaternion Rot = QuatFromAngleAxis(DegreesToRadians(-45), V3f(1.0f, 0, 0));

		static const SlideDefinition Slide = MakeSlideDef("Links", Blocks, GetBackground(), Rot);
	};

	namespace Thanks {
		static const char* Talk = {
			""
		};

		static BlockDefinition Blocks[] = {
			MakeTextDef(
				V2f(),V2f(1,1),MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				"Thank you for your attention!",
				HorizontalAlignment::Center
			),
		};

		static Quaternion Rot = QuatIdentity();

		static const SlideDefinition Slide = MakeSlideDef("Thanks!", Blocks, GetBackground(), Rot);
	};

	static const SlideDefinition Slides[] = {
		Intro::Slide,
		WhoAmI::Slide,
		WhatIsFPL::Slide,
		Motivation::Slide,
		Goals::Slide,
		WhyFPL::Slide,
		FeaturesOfFPL::Slide,
		Magic::Slide,
		HowItWorks::Slide,
		HowToUse::Slide,
		Demos::Slide,
		Links::Slide,
		Thanks::Slide
	};

	static const Vec4f ForegroundColor = RGBAToLinearRaw(255, 255, 255, 255);
	static const Vec4f TextShadowColor = RGBAToLinearRaw(0, 0, 0, 255);
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
		/* centerText */ "Copyright (C) 2017-2023 Torsten Spaete",
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