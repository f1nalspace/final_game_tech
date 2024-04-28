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
	static const SoundResource Motivation = SoundResource::CreateFromFile("fpl_motivation.mp3");

	static const SoundResource All[] = {
		Intro1,
		Intro2,
		WhoAmi1,
		What1,
		What2,
		Motivation,
	};
};

namespace ImageResources {
	static ImageResource FPLLogo128x128 = ImageResource::CreateFromMemory(ptr_fplLogo128x128ImageData, "FPL Logo 128x128", sizeOf_fplLogo128x128ImageData);
	static ImageResource FPLLogo512x512 = ImageResource::CreateFromMemory(ptr_fplLogo512x512ImageData, "FPL Logo 512x512", sizeOf_fplLogo512x512ImageData);

	static ImageResource MinimumSource = ImageResource::CreateFromFile("minimum_source.png");

	static ImageResource Card_CPU = ImageResource::CreateFromFile("card_cpu.png");
	static ImageResource Card_C_Language = ImageResource::CreateFromFile("card_c_language.png");
	static ImageResource Card_Audio = ImageResource::CreateFromFile("card_audio.png");
	static ImageResource Card_Video = ImageResource::CreateFromFile("card_video.png");
	static ImageResource Card_WindowManagement = ImageResource::CreateFromFile("card_window_management.png");
	static ImageResource Card_Performance = ImageResource::CreateFromFile("card_performance.png");
	static ImageResource Card_NoDependencies = ImageResource::CreateFromFile("card_no_deps.png");
	static ImageResource Card_Mouse = ImageResource::CreateFromFile("card_mouse.png");
	static ImageResource Card_Keyboard = ImageResource::CreateFromFile("card_keyboard.png");
	static ImageResource Card_Gamepad = ImageResource::CreateFromFile("card_gamepad.png");

	static ImageResource MagicHat = ImageResource::CreateFromFile("magic_hat.png");

	static ImageResource DataVisualization = ImageResource::CreateFromFile("data-visualization.png");
	static ImageResource GameDev = ImageResource::CreateFromFile("game_dev.png");
	static ImageResource MultimediaDev = ImageResource::CreateFromFile("multimedia_dev.png");
	static ImageResource SimulationDev = ImageResource::CreateFromFile("sim_dev.png");

	static ImageResource Code = ImageResource::CreateFromFile("code.png");

	static ImageResource Vendor_FreeBSD = ImageResource::CreateFromFile("vendor-freebsd.png");
	static ImageResource Vendor_Linux = ImageResource::CreateFromFile("vendor-linux.png");
	static ImageResource Vendor_Windows = ImageResource::CreateFromFile("vendor-windows.png");
	static ImageResource Vendor_Raspberry = ImageResource::CreateFromFile("vendor-raspberry_pi.png");
	static ImageResource Vendor_OpenSource = ImageResource::CreateFromFile("vendor-open_source.png");
	static ImageResource Vendor_OpenGL = ImageResource::CreateFromFile("vendor-opengl.png");
	static ImageResource Vendor_Vulkan = ImageResource::CreateFromFile("vendor-vulkan.png");
	static ImageResource Vendor_DirectX = ImageResource::CreateFromFile("vendor-directx.png");
	static ImageResource Vendor_Alsa = ImageResource::CreateFromFile("vendor-alsa.png");
	static ImageResource Vendor_XLib = ImageResource::CreateFromFile("vendor-xlib.png");

	static ImageResource Demo_ImGUI = ImageResource::CreateFromFile("demo_imgui.png");
	static ImageResource Demo_FFMPEG = ImageResource::CreateFromFile("demo_ffmpeg.png");
	static ImageResource Demo_NBodySimulation = ImageResource::CreateFromFile("demo_nbodysim.png");
	static ImageResource Demo_Audio = ImageResource::CreateFromFile("demo_audio.png");
	static ImageResource Demo_Input = ImageResource::CreateFromFile("demo_input.png");
	static ImageResource Demo_Crackout = ImageResource::CreateFromFile("demo_crackout.png");
	static ImageResource Demo_OpenGL = ImageResource::CreateFromFile("demo_opengl.png");
	static ImageResource Demo_Raytracer = ImageResource::CreateFromFile("demo_raytracer.png");

	static const ImageResource All[] = {
		FPLLogo128x128,
		FPLLogo512x512,
		
		MinimumSource,

		Card_CPU,
		Card_C_Language,
		Card_Audio,
		Card_Video,
		Card_WindowManagement,
		Card_Performance,
		Card_NoDependencies,
		Card_Mouse,
		Card_Keyboard,
		Card_Gamepad,

		Vendor_FreeBSD,
		Vendor_Linux,
		Vendor_Windows,
		Vendor_Raspberry,
		Vendor_OpenSource,
		Vendor_OpenGL,
		Vendor_Vulkan,
		Vendor_DirectX,
		Vendor_Alsa,
		Vendor_XLib,

		MagicHat,
		DataVisualization,
		GameDev,
		MultimediaDev,
		SimulationDev,
		Code,

		Demo_ImGUI,
		Demo_FFMPEG,
		Demo_NBodySimulation,
		Demo_Audio,
		Demo_Input,
		Demo_Crackout,
		Demo_OpenGL,
		Demo_Raytracer,
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

	// Purple to Brown
	static BackgroundStyle Background14 = MakeBackground(RGBAToLinearHex24(0x000000, 255), RGBAToLinearHex24(0x237b75, 100));
	static BackgroundStyle Background15 = MakeBackground(RGBAToLinearHex24(0x000000, 255), RGBAToLinearHex24(0x6d7b23, 100));
	static BackgroundStyle Background16 = MakeBackground(RGBAToLinearHex24(0x000000, 255), RGBAToLinearHex24(0x7b5223, 100));

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
		Background14,
		Background15,
		Background16,
	};

	static int BackgroundIndex = 0;

	static BackgroundStyle GetBackground() {
		BackgroundStyle result = Backgrounds[BackgroundIndex];
		BackgroundIndex = (BackgroundIndex + 1) % fplArrayCount(Backgrounds);
		return(result);
	}

	namespace Intro {
		static const char *Talk = {
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
		static const char *Talk = {
			"I am Torsten Spaete"
			"A professional software engineer with more than 25 years of programming experience"
			"My main focus is data-visualization, software-architecture, multimedia and game development"
		};

		static BlockDefinition Blocks[] = {
			MakeImageDef(
				V2f(0.0f, 0.0f), V2f(0.45f, 0.45f), MakeAlign(HorizontalAlignment::Left, VerticalAlignment::Top),
				&ImageResources::DataVisualization, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 0.5f)
			),
			MakeImageDef(
				V2f(0.55f, 0.0f), V2f(0.45f, 0.45f), MakeAlign(HorizontalAlignment::Right, VerticalAlignment::Top),
				&ImageResources::MultimediaDev, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 0.5f)
			),
			MakeImageDef(
				V2f(0.0f, 0.50f), V2f(0.45f, 0.45f), MakeAlign(HorizontalAlignment::Left, VerticalAlignment::Bottom),
				&ImageResources::SimulationDev, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 0.5f)
			),
			MakeImageDef(
				V2f(0.55f, 0.50f), V2f(0.45f, 0.45f), MakeAlign(HorizontalAlignment::Right, VerticalAlignment::Bottom),
				&ImageResources::GameDev, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 0.5f)
			),

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
		static const char *Talk = {
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
			MakeImageDef(
				V2f(0.0f, -0.1f), V2f(1.0f, 1.0f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				&ImageResources::Code, V2f(1.0f, 1.2f), false, V4f(1.0f, 1.0f, 1.0f, 0.05f)
			),

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
			MakeSoundDef(SoundResources::What1, 1.0),
			MakeSoundDef(SoundResources::What2, 17.0),
		};

		static Quaternion Rot = QuatFromAngleAxis(DegreesToRadians(-45), V3f(0, 1, 1));

		static const SlideDefinition Slide = MakeSlideDef("What is FPL", Blocks, Sounds, GetBackground(), Rot, 32.0);
	};

	namespace Motivation {
		static const char *Talk = {
			"C/C++ has very limited access to the underlying platform."
			"Even in modern C++, you still don't have direct access to a lot of systems."
			"To access low-level systems, such as audio or video or windowing systems, you either need to use third-party libraries"
			"or write platform-specific codes directly for every platform you want to support."
		};

		static BlockDefinition Blocks[] = {
			MakeTextDef(
				V2f(0.0f, 0.0f),V2f(1.0f, 1.0f),MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				"C/C++ has very limited access to the underlying platform,\n"
				"so you have either use third-party libraries or\n"
				"write platform specific codes directly.\n",
				HorizontalAlignment::Left
			),
		};

		static SoundDefinition Sounds[] = {
			MakeSoundDef(SoundResources::Motivation, 2.0),
		};

		static Quaternion Rot = QuatFromAngleAxis(DegreesToRadians(70), V3f(1, 0.1f, 0.0f));

		static const SlideDefinition Slide = MakeSlideDef("Motivation", Blocks, Sounds, GetBackground(), Rot, 31.0);
	};

	namespace Features {
		static const char *Talk = {
			"- The core features of FPL are:"
			""
			"- Is is written in pure C99 for simplicity and best portability"
			"- No build systems or thirdparty libraries required, it works out-of-the box"
			"- Extraordinary fast compile times, due to the single-header-file style."
			"- It gives you control of the allocated memory and handles memory very gracefully."
			"- Support for many compilers and platforms, including a lot of utilities and macros to work with them."
			"- Creating and managing a window that initialize a graphics API such as OpenGL or Vulkan or even direct drawing of pixels."
			"- Handle and process keyboard, mouse, gamepad devices through events or polling."
			"- Playing back audio samples asynchronously."
			"- Let the user decide how to integrate it, not force it in any way."
			"- So it supports dynamic and static linking and full-source inclusion with private and external integration."
			"- It also provides primitives for working with multithreaded code efficiently."
			"- In addition it contains functions to work with consoles, files, directories."
		};

		static BlockDefinition Blocks[] = {
			/*MakeTextDef(
				V2f(0,0),V2f(1.0f,1.0f),MakeAlign(HorizontalAlignment::Center),
				"- Is is written in pure C99 for simplicity and best portability.\n"
				"- No build systems or thirdparty libraries required, it works out-of-the box.\n"
				"- Extraordinary fast compile times.\n"
				"- Control of the memory allocations and handles memory very gracefully.\n"
				"- Support for many compilers and platforms.\n"
				"- Creating and managing a window with support for many video backends.\n"
				"- Handling and processing Keyboard, Mouse, Gamepad devices through events or polling.\n"
				"- Asynchronous audio playback implemented for many audio backends.\n"
				"- Let the user decide how to integrate it.\n"
				"- Supports dynamic linking, static linking and full-source inclusion.\n"
				"- Provides several multi threading primitives.\n"
				"- Support for working with IO (console, files, directories, etc.).\n",
				HorizontalAlignment::Left, FeaturesFontSize
			),*/

			MakeImageDef(
				V2f(0.0f, 0.1f), V2f(0.2f, 0.2f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Top),
				&ImageResources::Card_CPU, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 1.0f)
			),
			MakeImageDef(
				V2f(0.2f, 0.1f), V2f(0.2f, 0.2f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Top),
				&ImageResources::Card_C_Language, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 1.0f)
			),
			MakeImageDef(
				V2f(0.4f, 0.1f), V2f(0.2f, 0.2f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Top),
				&ImageResources::Card_Performance, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 1.0f)
			),
			MakeImageDef(
				V2f(0.6f, 0.1f), V2f(0.2f, 0.2f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Top),
				&ImageResources::Card_NoDependencies, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 1.0f)
			),
			MakeImageDef(
				V2f(0.8f, 0.1f), V2f(0.2f, 0.2f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Top),
				&ImageResources::Card_WindowManagement, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 1.0f)
			),

			MakeImageDef(
				V2f(0.0f, 0.5f), V2f(0.2f, 0.2f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Top),
				&ImageResources::Card_Video, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 1.0f)
			),
			MakeImageDef(
				V2f(0.2f, 0.5f), V2f(0.2f, 0.2f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Top),
				&ImageResources::Card_Audio, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 1.0f)
			),
			MakeImageDef(
				V2f(0.4f, 0.5f), V2f(0.2f, 0.2f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Top),
				&ImageResources::Card_Keyboard, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 1.0f)
			),
			MakeImageDef(
				V2f(0.6f, 0.5f), V2f(0.2f, 0.2f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Top),
				&ImageResources::Card_Mouse, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 1.0f)
			),
			MakeImageDef(
				V2f(0.8f, 0.5f), V2f(0.2f, 0.2f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Top),
				&ImageResources::Card_Gamepad, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 1.0f)
			),

			MakeImageDef(
				V2f(0.0f, 0.85f), V2f(0.1f, 0.1f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Bottom),
				&ImageResources::Vendor_Windows, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 0.5f)
			),
			MakeImageDef(
				V2f(0.1f, 0.85f), V2f(0.1f, 0.1f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Bottom),
				&ImageResources::Vendor_Linux, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 0.5f)
			),
			MakeImageDef(
				V2f(0.2f, 0.85f), V2f(0.1f, 0.1f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Bottom),
				&ImageResources::Vendor_Raspberry, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 0.5f)
			),
			MakeImageDef(
				V2f(0.3f, 0.85f), V2f(0.1f, 0.1f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Bottom),
				&ImageResources::Vendor_FreeBSD, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 0.5f)
			),
			MakeImageDef(
				V2f(0.4f, 0.85f), V2f(0.1f, 0.1f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Bottom),
				&ImageResources::Vendor_OpenGL, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 0.5f)
			),
			MakeImageDef(
				V2f(0.5f, 0.85f), V2f(0.1f, 0.1f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Bottom),
				&ImageResources::Vendor_Vulkan, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 0.5f)
			),
			MakeImageDef(
				V2f(0.6f, 0.85f), V2f(0.1f, 0.1f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Bottom),
				&ImageResources::Vendor_DirectX, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 0.5f)
			),
			MakeImageDef(
				V2f(0.7f, 0.85f), V2f(0.1f, 0.1f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Bottom),
				&ImageResources::Vendor_Alsa, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 0.5f)
			),
			MakeImageDef(
				V2f(0.8f, 0.85f), V2f(0.1f, 0.1f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Bottom),
				&ImageResources::Vendor_XLib, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 0.5f)
			),
			MakeImageDef(
				V2f(0.9f, 0.85f), V2f(0.1f, 0.1f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Bottom),
				&ImageResources::Vendor_OpenSource, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 0.5f)
			),
		};

		static Quaternion Rot = QuatFromAngleAxis(DegreesToRadians(30), V3f(0.0f, 1.0f, 0.1f));

		static const SlideDefinition Slide = MakeSlideDef("Features of FPL", Blocks, GetBackground(), Rot);
	};

	namespace Magic {
		static const char *Talk = {
			"How does FPL work?"
			""
			"It works with Magic!"
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
		static const char *Talk = {
			"Unfortunatly it does not with magic."
			""
			"It is a single-header-file-library, so it contains the full API and source for every platform"
			"It makes heavy use of the pre-compiler to detect compiler/platform/hardware/driver setups"
			"It prevents code-duplication by using sub-platforms"
			"It uses runtime linking by default, so everything is automatically loaded on demand"
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
		static const char *Talk = {
			""
		};

		static BlockDefinition Blocks[] = {
			MakeImageDef(
				V2f(0.075f, 0.0f), V2f(0.85f, 1.0f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				&ImageResources::MinimumSource, V2f(1.0f, 1.0f), false, V4f(1.0f, 1.0f, 1.0f, 0.85f)
			),
		};

		static Quaternion Rot = QuatFromAngleAxis(DegreesToRadians(75), V3f(0.0f, 1.0f, 0));

		static const SlideDefinition Slide = MakeSlideDef("How to use", Blocks, GetBackground(), Rot);
	};

	namespace Demos {
		static const char *Talk = {
			""
		};

		static BlockDefinition Blocks[] = {
			MakeImageDef(
				V2f(0.0f, 0.0f), V2f(0.25f, 0.25f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				&ImageResources::Demo_ImGUI, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 0.65f)
			),
			MakeImageDef(
				V2f(0.25f, 0.0f), V2f(0.25f, 0.25f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				&ImageResources::Demo_FFMPEG, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 0.65f)
			),
			MakeImageDef(
				V2f(0.5f, 0.0f), V2f(0.25f, 0.25f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				&ImageResources::Demo_Audio, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 0.65f)
			),
			MakeImageDef(
				V2f(0.75f, 0.0f), V2f(0.25f, 0.25f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				&ImageResources::Demo_Input, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 0.65f)
			),
			MakeImageDef(
				V2f(0.0f, 0.6f), V2f(0.25f, 0.25f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				&ImageResources::Demo_OpenGL, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 0.65f)
			),
			MakeImageDef(
				V2f(0.25f, 0.6f), V2f(0.25f, 0.25f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				&ImageResources::Demo_Crackout, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 0.65f)
			),
			MakeImageDef(
				V2f(0.5f, 0.6f), V2f(0.25f, 0.25f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				&ImageResources::Demo_Raytracer, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 0.65f)
			),
			MakeImageDef(
				V2f(0.75f, 0.6f), V2f(0.25f, 0.25f), MakeAlign(HorizontalAlignment::Center, VerticalAlignment::Middle),
				&ImageResources::Demo_NBodySimulation, V2f(1.0f, 1.0f), true, V4f(1.0f, 1.0f, 1.0f, 0.65f)
			),

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
		static const char *Talk = {
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
		static const char *Talk = {
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
		Features::Slide,
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