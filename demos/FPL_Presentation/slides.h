#pragma once

#include <final_platform_layer.h>

#include <final_math.h>

#include "types.h"
#include "fonts.h"

static const float CodeFontSize = 17.0f;
static const float FeaturesFontSize = 32.0f;

enum class BlockType {
	None = 0,
	Text,
	Image,
};

struct TextBlockDefinition {
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

static BlockDefinition MakeTextDef(const Vec2f& pos, const Vec2f& size, BlockAlignment contentAlignment, const char* text, const HorizontalAlignment textAlign = HorizontalAlignment::Left, const float fontSize = 0) {
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
struct SlideDefinition {
	const char* name;
	BlockDefinition blocks[MaxBlockCount];
	BackgroundStyle background;
	size_t count;
};

template<size_t N>
static SlideDefinition MakeSlideDef(const char* name, BlockDefinition(&blocks)[N], const BackgroundStyle& background) {
	fplAssert(N < MaxBlockCount);
	SlideDefinition result = {};
	result.name = name;
	result.background = background;
	for(size_t i = 0; i < N; ++i) {
		result.blocks[i] = blocks[i];
	}
	result.count = N;
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
	static BackgroundStyle DarkBlueBack = MakeBackground(RGBAToLinearRaw(0, 0, 0, 255), RGBAToLinearRaw(15, 13, 80, 255));

	// DarkBlue to LightBlue
	static BackgroundStyle Background1 = MakeBackground(RGBAToLinearHex24(0x000000), RGBAToLinearHex24(0x333789));
	static BackgroundStyle Background2 = MakeBackground(RGBAToLinearHex24(0x000000), RGBAToLinearHex24(0x1E5AA3));
	static BackgroundStyle Background3 = MakeBackground(RGBAToLinearHex24(0x000000), RGBAToLinearHex24(0x0483AF));

	// Green to Yellow
	static BackgroundStyle Background4 = MakeBackground(RGBAToLinearHex24(0x000000), RGBAToLinearHex24(0x057F47));
	static BackgroundStyle Background5 = MakeBackground(RGBAToLinearHex24(0x000000), RGBAToLinearHex24(0x7AB30B));
	static BackgroundStyle Background6 = MakeBackground(RGBAToLinearHex24(0x000000), RGBAToLinearHex24(0xF2E500));

	// LightOrange to DarkOrange
	static BackgroundStyle Background7 = MakeBackground(RGBAToLinearHex24(0x000000), RGBAToLinearHex24(0xFDBD00));
	static BackgroundStyle Background8 = MakeBackground(RGBAToLinearHex24(0x000000), RGBAToLinearHex24(0xEE7B00));
	static BackgroundStyle Background9 = MakeBackground(RGBAToLinearHex24(0x000000), RGBAToLinearHex24(0xE54B0A));

	// Red to Purple
	static BackgroundStyle Background10 = MakeBackground(RGBAToLinearHex24(0x000000), RGBAToLinearHex24(0xDC0012));
	static BackgroundStyle Background11 = MakeBackground(RGBAToLinearHex24(0x000000), RGBAToLinearHex24(0xB7006A));
	static BackgroundStyle Background12 = MakeBackground(RGBAToLinearHex24(0x000000), RGBAToLinearHex24(0x59227A));

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
	};

	static int BackgroundIndex = 0;

	static BackgroundStyle GetBackground() {
		BackgroundStyle result = Backgrounds[BackgroundIndex];
		BackgroundIndex = (BackgroundIndex + 1) % fplArrayCount(Backgrounds);
		return(result);
	}

	namespace Intro {
		static const char* Talk = {
			"Hello!"
			"My name is Torsten Spaete. I am a professional software engineer with over 20 years of experience."
			"Today I would like to introduce you to a project I have been working on for a few years now."
			"It's a platform abstraction library written in C99 called 'Final-Platform-Layer'."
		};

		static BlockDefinition Blocks[] = {
			MakeTextDef(
				V2f(),V2f(1,1),MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				"Introducing Final-Platform-Layer (FPL)\n"
				"A lightweight Platform-Abstraction-Library written in C99\n",
				HorizontalAlignment::Center
			),
		};

		static const SlideDefinition Slide = MakeSlideDef("Introduction", Blocks, GetBackground());
	};

	namespace WhatIsAPAL {
		static const char* Talk = {
			"Any operating system has special systems/functions to access certain things,"
			"such as memory, files, threads, etc., or give you access to hardware devices,"
			"such as graphics cards, audio devices, etc."
			""
			"A platform abstraction layer (or short 'PAL') is a development library, written in a low-level language such as 'C',"
			"that gives you access to such systems or functions in a platform-independent way."
			""
			"Such libraries are built so that you can write code once, that runs on every supported platform."
		};

		static BlockDefinition Blocks[] = {
			MakeTextDef(
				V2f(),V2f(1,1),MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				"A platform abstraction layer (or short 'PAL')\n"
				"is a development library,\n"
				"written in a low-level language such as 'C',\n"
				"used to access hardware and low-level systems\n"
				"in a platform-independent way.\n",
				HorizontalAlignment::Left
			),
		};

		static const SlideDefinition Slide = MakeSlideDef("What is a PAL", Blocks, GetBackground());
	};

	namespace WhatIsFPL {
		static const char* Talk = {
			"'Final-Platform-Layer' (or short 'FPL') is an all-purpose platform-abstraction-layer written in C99 which provides a powerful and easy-to-use API,"
			"for working with low-level and hardware systems such as windowing, audio, video, memory, timing, input-systems and many more."
		};

		static BlockDefinition Blocks[] = {
			MakeTextDef(
				V2f(),V2f(1,1),MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				"'Final-Platform-Layer' (or short 'FPL')\n"
				"is an all-purpose/multimedia PAL written in C99,\n"
				"providing a powerful and easy to use API,\n"
				"for working with low-level and hardware systems\n"
				"such as window, audio, video, memory and input-systems, etc.\n",
				HorizontalAlignment::Left
			),
		};

		static const SlideDefinition Slide = MakeSlideDef("What is FPL", Blocks, GetBackground());
	};

	namespace Motivation {
		static BlockDefinition Blocks[] = {
			MakeTextDef(
				V2f(0.0, 0.0),V2f(1.0, 0.25),MakeAlign(HorizontalAlignment::Center),
				"C/C++ has very limited access to the underlying platform,\n"
				"so you have either use third-party libraries to access the platform\n"
				"or write platform specific codes directly.\n",
				HorizontalAlignment::Left
			),

			MakeTextDef(
				V2f(0.0, 0.25),V2f(1.0, 0.75),MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				"The pre-existing PALs have a lot of issues:\n"
				"- Huge in file count and/or size or large compile times\n"
				"- Limited control over the allocated memory\n"
				"- Statically linking is madness or not supported at all\n"
				"- Including the full source is not supported\n"
				"- Some are heavily bloated\n",
				HorizontalAlignment::Left, FeaturesFontSize
			),
		};
		static const SlideDefinition Slide = MakeSlideDef("Motivation", Blocks, GetBackground());
	};

	namespace Goals {
		static BlockDefinition Blocks[] = {
			MakeTextDef(
				V2f(0.0, 0.0),V2f(1.0, 1.0),MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				"- One header file containing all the source code for all platforms\n"
				"\n"
				"- Bare minimum compile and linking requirements\n"
				"\n"
				"- Does not require the C-Runtime library\n"
				"\n"
				"- Uses a fixed and small memory footprint and handles memory very gracefully\n"
				"\n"
				"- Hides no data and let the user decide how to integrate it\n"
				"\n"
				"- Compiles very fast on all modern C99/C++ compilers\n",
				HorizontalAlignment::Left, FeaturesFontSize
			),
		};
		static const SlideDefinition Slide = MakeSlideDef("Goals of FPL", Blocks, GetBackground());
	};

	namespace FeaturesOfFPL1 {
		static BlockDefinition Blocks[] = {
			MakeTextDef(
				V2f(0,0),V2f(1.0f,0.5f),MakeAlign(HorizontalAlignment::Center),
				"- Platform detection (x86/x64/Arm, Win32/Linux/Unix, etc.)\n"
				"- Compiler detection (MSVC/GCC/Clang/Intel)\n"
				"- Macros (debug break, assertions, CPU features, memory etc.)\n"
				"- Window creation and handling (Win32/X11)\n"
				"- Event and input polling (keyboard/mouse/gamepad)\n"
				"- Video initialization and output (software, OpenGL, etc.)\n",
				HorizontalAlignment::Left, FeaturesFontSize
			),
			MakeImageDef(
				V2f(0.0f,0.5f),V2f(1.0f,0.5f),MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				"FPL Features", V2f(1.0f,1.0f), true
			),
		};
		static const SlideDefinition Slide = MakeSlideDef("Features of FPL 1/2", Blocks, GetBackground());
	};

	namespace FeaturesOfFPL2 {
		static BlockDefinition Blocks[] = {
			MakeTextDef(
				V2f(0.0f),V2f(1.0f,0.5f),MakeAlign(HorizontalAlignment::Center),
				"- Asynchronous audio playback (DirectSound, ALSA, etc.)\n"
				"- IO (console, paths, files, directories, etc.)\n"
				"- Memory allocation\n"
				"- Dynamic library loading (.dll/.so)\n"
				"- Multi threading (atomics, threads, mutexes, semaphores, conditionals, etc.)\n"
				"- Retrieving hardware information\n"
				"- and many more\n",
				HorizontalAlignment::Left, FeaturesFontSize
			),

			MakeImageDef(
				V2f(0.0f,0.5f), V2f(1.0f,0.5f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				"FPL Features", V2f(1.0f,1.0f), true
			),
		};

		static const SlideDefinition Slide = MakeSlideDef("Features of FPL 2/2", Blocks, GetBackground());
	};

	namespace HowMagic {
		static BlockDefinition Blocks[] = {
			MakeTextDef(
				V2f(0.0f,0.0f),V2f(1.0f,1.0f),MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				"Magic!",
				HorizontalAlignment::Left, FeaturesFontSize * 4
			),
		};
		static const SlideDefinition Slide = MakeSlideDef("How it works", Blocks, GetBackground());
	};

	namespace HowItWorks {
		static BlockDefinition Blocks[] = {
			MakeTextDef(
				V2f(0.0f,0.0f),V2f(1.0f,0.25f),MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Top),
				"Joke, of course its not magic :P",
				HorizontalAlignment::Left, FeaturesFontSize
			),

			MakeTextDef(
				V2f(0.0f,0.0f),V2f(1.0f,1.0f),MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				"- It is written in pure C99 for simplicity and best portability - but is 100%% C++ compatible\n"
				"- It makes heavy use of the pre-compiler to detect compiler/platform/hardware/driver setups\n"
				"- It uses runtime linking by default, so it does not need any libs to be included\n"
				"- It prevents code-duplication by using sub-platform blocks (Unix vs Linux)\n"
				"- It is stateless, meaning the user does not have to provide any application states\n"
				"- Everything is included in one C file\n",
				HorizontalAlignment::Left, FeaturesFontSize
			),
		};
		static const SlideDefinition Slide = MakeSlideDef("How it really works", Blocks, GetBackground());
	};

	namespace RandomCode {
		static BlockDefinition Blocks[] = {
			MakeTextDef(
				V2f(0.0f,0.0f),V2f(0.5f,1.0f),MakeAlign(HorizontalAlignment::Center),
				"#define FPL_IMPLEMENTATION\n"
				"#include <final_platform_layer.h>\n"
				"\n"
				"int main(int argc, char **argv) {\n"
				"\tif (fplPlatformInit(fplInitFlags_Console, &settings)) {\n"
				"\t\tfplMemoryAllocate(fplMegaBytes(128));\n"
				"\t\tfplDynamicLibraryLoad(\"my_library.so\", &libHandle);\n"
				"\t\tfplThreadCreate(threadProc, userDataForThread);\n"
				"\t\tfplMutexLock(&lockHandle);\n"
				"\t\tfplWriteFileBlock(&fileHandle, &blockData, lengthToWrite);\n"
				"\t\tfplFileCopy(\"my_data_file.dat\", \"my_data_file.dat.backup\");\n"
				"\t\tfplConsoleFormatOut(\"Little endian: %%s\\n\", (fplIsLittleEndian() ? \"yes\" : \"no\");\n"
				"\t\tfplGetAudioDevices(&audioDevices[0], fplArrayCount(audioDevices));\n"
				"\t\tfplGetOperatingSystemInfos(&osInfos);\n"
				"\t\tfplPlatformRelease();\n"
				"\t}\n"
				"}\n"
			, HorizontalAlignment::Left, CodeFontSize),

			MakeTextDef(
				V2f(0.5f,0.0f),V2f(0.5f,1.0f),MakeAlign(HorizontalAlignment::Center),
				"#define FPL_IMPLEMENTATION\n"
				"#include <final_platform_layer.h>\n"
				"\n"
				"int main(int argc, char **argv) {\n"
				"\tif (fplPlatformInit(fplInitFlags_All, &settings)) {\n"
				"\t\twhile (fplWindowUpdate()) {\n"
				"\t\t\tfplEvent ev;\n"
				"\t\t\twhile (fplPollEvent(&ev)) {\n"
				"\t\t\t\tProcessEvent(&ev);\n"
				"\t\t\t}\n"
				"\t\t\tRenderAndUpdateFrame(&frameState);\n"
				"\t\t\tfplVideoFlip();\n"
				"\t\t\tlastFrameTime = fplGetTimeInSecondsHP();\n"
				"\t\t}\n"
				"\t\tfplPlatformRelease();\n"
				"\t}\n"
				"}\n",
				HorizontalAlignment::Left, CodeFontSize
			),
		};
		static const SlideDefinition Slide = MakeSlideDef("Example Code", Blocks, GetBackground());
	};

	namespace Demos {
		static BlockDefinition Blocks[] = {
			MakeTextDef(
				V2f(),V2f(1,1),MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				"Demo-Time!",
				HorizontalAlignment::Center
			),
		};
		static const SlideDefinition Slide = MakeSlideDef("Demos!", Blocks, GetBackground());
	};

	static const SlideDefinition Slides[] = {
		Intro::Slide,
		WhatIsAPAL::Slide,
		WhatIsFPL::Slide,
		Motivation::Slide,
		Goals::Slide,
		FeaturesOfFPL1::Slide,
		FeaturesOfFPL2::Slide,
		HowMagic::Slide,
		HowItWorks::Slide,
		RandomCode::Slide,
		Demos::Slide,
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
		/* centerText */ "Copyright (C) 2021 Torsten Spaete",
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