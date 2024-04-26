#pragma once

#include "presentation.h"

static const float CodeFontSize = 24.0f;
static const float FeaturesFontSize = 32.0f;

namespace FontResources {
	static FontResource Debug = { ptr_bitstreamVerySansFontData, sizeOf_bitstreamVerySansFontData, "Debug" };
	static FontResource Arimo = { ptr_arimoRegularFontData, sizeOf_arimoRegularFontData, "Arimo" };
	static FontResource SulphurPoint = { ptr_sulphurPointRegularData, sizeOf_sulphurPointRegularData, "Sulphur Point" };
	static FontResource BitStreamVerySans = { ptr_bitstreamVerySansFontData, sizeOf_bitstreamVerySansFontData, "Bitstream Vera Sans" };
};

namespace SoundResources {
	static const SoundResource Intro1 = SoundResource::CreateFromFile("fpl_intro1.mp3");
	static const SoundResource Intro2 = SoundResource::CreateFromFile("fpl_intro2.mp3");
	static const SoundResource WhoAmi1 = SoundResource::CreateFromFile("fpl_whoami1.mp3");
	static const SoundResource What1 = SoundResource::CreateFromFile("fpl_what1.mp3");
	static const SoundResource What2 = SoundResource::CreateFromFile("fpl_what2.mp3");
	static const SoundResource Motivation1 = SoundResource::CreateFromFile("fpl_motivation1.mp3");
	static const SoundResource Motivation2 = SoundResource::CreateFromFile("fpl_motivation2.mp3");

	static const SoundResource All[] = {
		Intro1,
		Intro2,
		WhoAmi1,
		What1,
		What2,
		Motivation1,
		Motivation2,
	};
};

namespace ImageResources {
	static ImageResource FPLLogo128x128 = ImageResource::CreateFromMemory(ptr_fplLogo128x128ImageData, "FPL Logo 128x128", sizeOf_fplLogo128x128ImageData);
	static ImageResource FPLLogo512x512 = ImageResource::CreateFromMemory(ptr_fplLogo512x512ImageData, "FPL Logo 512x512", sizeOf_fplLogo512x512ImageData);
	static ImageResource FPLMinimumSource = ImageResource::CreateFromMemory(ptr_minimumSourceImageData, "FPL Minimum Source", sizeOf_minimumSourceImageData);

	static ImageResource Card_SingleHeaderFile = ImageResource::CreateFromFile("card_single_header_file.png");
	static ImageResource Card_C99 = ImageResource::CreateFromFile("card_c99.png");
	static ImageResource Card_Fast = ImageResource::CreateFromFile("card_fast.png");
	static ImageResource Card_NoDeps = ImageResource::CreateFromFile("card_no_deps.png");

	static ImageResource Card_EasyToUse = ImageResource::CreateFromFile("card_easytouse.png");
	static ImageResource Card_Lightweight = ImageResource::CreateFromFile("card_lightweight.png");
	static ImageResource Card_Memory = ImageResource::CreateFromFile("card_memory.png");
	static ImageResource Card_Cpp_Compatible = ImageResource::CreateFromFile("card_cpp_compatible.png");

	static ImageResource Card_CleanApi = ImageResource::CreateFromFile("card_clean_api.png");
	static ImageResource Card_FullSource = ImageResource::CreateFromFile("card_full_source.png");
	static ImageResource Card_RuntimeLinking = ImageResource::CreateFromFile("card_runtime_linking.png");
	static ImageResource Card_OpenSource = ImageResource::CreateFromFile("card_open_source.png");

	static ImageResource MagicHat = ImageResource::CreateFromFile("magic_hat.png");

	static const ImageResource All[] = {
		FPLLogo128x128,
		FPLLogo512x512,
		FPLMinimumSource,

		Card_SingleHeaderFile,
		Card_C99,
		Card_Fast,
		Card_NoDeps,

		Card_EasyToUse,
		Card_Lightweight,
		Card_Memory,
		Card_Cpp_Compatible,

		Card_CleanApi,
		Card_FullSource,
		Card_OpenSource,
		Card_RuntimeLinking,

		MagicHat,
	};
}

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
				V2f(0.0, 0.0), V2f(1.0, 1.0),MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				"Introducing Final-Platform-Layer (FPL)\n"
				"A lightweight Platform-Abstraction-Library written in C99\n",
				HorizontalAlignment::Center
			),
		};

		static SoundDefinition Sounds[] = {
			MakeSoundDef(SoundResources::Intro1, 1.0),
			MakeSoundDef(SoundResources::Intro2, 10.0),
		};

		static Quaternion Rot = QuatIdentity();

		static const SlideDefinition Slide = MakeSlideDef("Introduction", Blocks, Sounds, GetBackground(), Rot, 18.5);
	};

	namespace WhoAmI {
		static const char* Talk = {
			"I am Torsten Spaete"
			"A professional software engineer with more than 25 years of programming experience"
			"My main focus is data-visualization, software-architecture, multimedia and game development"
		};

		static BlockDefinition Blocks[] = {
			MakeTextDef(
				V2f(0.0f, 0.0f), V2f(1.0f, 1.0f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				"I am Torsten Spaete\n"
				"A professional software engineer\n"
				"25+ years of programming experience\n",
				HorizontalAlignment::Center
			),
		};

		static SoundDefinition Sounds[] = {
			MakeSoundDef(SoundResources::WhoAmi1, 2.0),
		};

		static Quaternion Rot = QuatFromAngleAxis(DegreesToRadians(15), V3f(1, 1, 1));

		static const SlideDefinition Slide = MakeSlideDef("Who am I", Blocks, Sounds, GetBackground(), Rot, 20.5);		
	};

	namespace WhatIsFPL {
		static const char* Talk = {
			"Final-Platform-Layer (or short 'FPL') is a lightweight platform-abstraction library written in C99,"
			"that gives you access to low-level systems, hardware devices and operating system functions,"
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
			"Its main usage is multimedia and game development, but it can be used to write any kind of software."
			"'FPL' is designed to be fast in compile and run time and can be integrated however you like."
		};

		static BlockDefinition Blocks[] = {
			MakeTextDef(
				V2f(0.0f, 0.0f), V2f(1.0f, 1.0f),MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				"Final-Platform-Layer (or short 'FPL')\n"
				"is a lightweight library written in C99.\n"
				"\n"
				"It provides a powerful and easy to use API,\n"
				"that gives you access to low-level systems, hardware devices,\n"
				"operating system functions and many more.",
				HorizontalAlignment::Left
			),
		};

		static SoundDefinition Sounds[] = {
			MakeSoundDef(SoundResources::What1, 2.0),
			MakeSoundDef(SoundResources::What2, 20.0),
		};

		static Quaternion Rot = QuatFromAngleAxis(DegreesToRadians(-45), V3f(0, 1, 1));

		static const SlideDefinition Slide = MakeSlideDef("What is FPL", Blocks, Sounds, GetBackground(), Rot, 19.0);
	};

	namespace Motivation {
		static const char* Talk = {
			"C/C++ has very limited access to the underlying platform."
			"Even in modern C++, you still don't have direct access to a lot of systems."
			"To access low-level systems, such as audio or video, you either need to use third-party libraries or write platform-specific codes for Win32, Linux, Mac, etc. directly."
			""
			"Existing solutions was not working for me, due to the several reasons:"
			"- They was not compatible with either my compiler/linker/runtime configuration"
			"- They was hard to integrate into my own applications"
			"- They had no support for controlling the memory allocations"
			"- They had very long compile times"
			"- They required build-systems or other dependencies"
			"- They had no support for including the source directly"
			"- There was no suitable license"
		};

		static BlockDefinition Blocks[] = {
			MakeTextDef(
				V2f(0.0f, 0.0f),V2f(1.0f, 1.0f),MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				"C/C++ has very limited access to the underlying platform,\n"
				"so you have either use third-party libraries or\n"
				"write platform specific codes directly.\n"
				"\n"
				"Existing solutions was not working for me due to several reasons.\n",
				HorizontalAlignment::Left
			),
		};

		static SoundDefinition Sounds[] = {
			MakeSoundDef(SoundResources::Motivation1, 2.0),
			MakeSoundDef(SoundResources::Motivation2, 31.0),
		};

		static Quaternion Rot = QuatFromAngleAxis(DegreesToRadians(70), V3f(1, 0.1f, 0.0f));

		static const SlideDefinition Slide = MakeSlideDef("Motivation", Blocks, Sounds, GetBackground(), Rot);
	};

	namespace WhyFPL {
		static const char* Talk = {
			"Why do you want to use FPL?"
			""

			"- Everything is contained in one C-Header file (single-header-file)"

			"- Is is written in pure C99 for simplicity and best portability"

			"- It compiles blazingly fast on all modern C99/C++ compilers"

			"- It requires bare minimum compile and linking requirements"
			"- It does not require any build systems"
			"- It does not require the C - RunTime library"

			"- It is lightweight, has clean API and is easy to use"

			"- It allows to control the memory allocations and handles memory very gracefully\n"

			"- It is 100% C++ compatible\n"

			"- It can be integrated however you like: static linked, dynamic linked or with full-source"

			"- It uses runtime linking by default\n"

			"- It is MIT-Licensed"
		};

		static BlockDefinition Blocks[] = {
			MakeImageDef(
				V2f(0.0f, 0.05f), V2f(0.25f, 0.25f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				&ImageResources::Card_SingleHeaderFile, V2f(1.0f, 1.0f), true),
				
			MakeImageDef(
				V2f(0.25f, 0.05f), V2f(0.25f, 0.25f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				&ImageResources::Card_C99, V2f(1.0f, 1.0f), true),

			MakeImageDef(
				V2f(0.5f, 0.05f), V2f(0.25f, 0.25f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				&ImageResources::Card_Fast, V2f(1.0f, 1.0f), true),

			MakeImageDef(
				V2f(0.75f, 0.05f), V2f(0.25f, 0.25f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				&ImageResources::Card_NoDeps, V2f(1.0f, 1.0f), true),

			MakeImageDef(
				V2f(0.0f, 0.375f), V2f(0.25f, 0.25f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				&ImageResources::Card_EasyToUse, V2f(1.0f, 1.0f), true),

			MakeImageDef(
				V2f(0.25f, 0.375f), V2f(0.25f, 0.25f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				&ImageResources::Card_Lightweight, V2f(1.0f, 1.0f), true),

			MakeImageDef(
				V2f(0.5f, 0.375f), V2f(0.25f, 0.25f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				&ImageResources::Card_Memory, V2f(1.0f, 1.0f), true),

			MakeImageDef(
				V2f(0.75f, 0.375f), V2f(0.25f, 0.25f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				&ImageResources::Card_Cpp_Compatible, V2f(1.0f, 1.0f), true),

			MakeImageDef(
				V2f(0.0f, 0.7f), V2f(0.25f, 0.25f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				&ImageResources::Card_CleanApi, V2f(1.0f, 1.0f), true),

			MakeImageDef(
				V2f(0.25f, 0.7f), V2f(0.25f, 0.25f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				&ImageResources::Card_FullSource, V2f(1.0f, 1.0f), true),

			MakeImageDef(
				V2f(0.5f, 0.7f), V2f(0.25f, 0.25f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				&ImageResources::Card_RuntimeLinking, V2f(1.0f, 1.0f), true),

			MakeImageDef(
				V2f(0.75f, 0.7f), V2f(0.25f, 0.25f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				&ImageResources::Card_OpenSource, V2f(1.0f, 1.0f), true),
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
			MakeImageDef(
				V2f(-0.15f, 0.0f), V2f(1.0f, 1.0f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				&ImageResources::MagicHat, V2f(0.75f, 0.75f), true),

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
				&ImageResources::FPLMinimumSource, V2f(0.5f, 1.0f), true
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
		/* centerText */ "Copyright (C) 2017-2024 Torsten Spaete",
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