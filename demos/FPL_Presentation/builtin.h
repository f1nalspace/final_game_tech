#pragma once

#include "fonts.h"
#include "presentation.h"

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
}

namespace ImageResources {
	static ImageResource FPLLogo128x128 = ImageResource::CreateFromMemory("FPL Logo 128x128", ptr_fplLogo128x128ImageData, sizeOf_fplLogo128x128ImageData);
	static ImageResource FPLLogo512x512 = ImageResource::CreateFromMemory("FPL Logo 512x512", ptr_fplLogo512x512ImageData, sizeOf_fplLogo512x512ImageData);

	static ImageResource Card_CPU = ImageResource::CreateFromFile("card_cpu.png");
	static ImageResource Card_Memory = ImageResource::CreateFromFile("card_memory.png");
	static ImageResource Card_C_Language = ImageResource::CreateFromFile("card_c_language.png");
	static ImageResource Card_Audio = ImageResource::CreateFromFile("card_audio.png");
	static ImageResource Card_Video = ImageResource::CreateFromFile("card_video.png");
	static ImageResource Card_WindowManagement = ImageResource::CreateFromFile("card_window_management.png");
	static ImageResource Card_Performance = ImageResource::CreateFromFile("card_performance.png");
	static ImageResource Card_NoDependencies = ImageResource::CreateFromFile("card_no_deps.png");
	static ImageResource Card_KeyboardMouse = ImageResource::CreateFromFile("card_keyboard_mouse.png");
	static ImageResource Card_Gamepad = ImageResource::CreateFromFile("card_gamepad.png");

	static ImageResource MagicHat = ImageResource::CreateFromFile("magic_hat.png");
	static ImageResource Arigatou = ImageResource::CreateFromFile("arigatou.png");
	static ImageResource MinimumSource = ImageResource::CreateFromFile("minimum_source.png");

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

	static ImageResource Feature_SingleHeaderFile = ImageResource::CreateFromFile("singleheaderfile.png");
	static ImageResource Feature_RuntimeLinking = ImageResource::CreateFromFile("runtimelinking.png");
	static ImageResource Feature_PreProcessor = ImageResource::CreateFromFile("preprocessor.png");
	static ImageResource Feature_NoCodeDuplication = ImageResource::CreateFromFile("nocodeduplication.png");

	static const ImageResource All[] = {
		FPLLogo128x128,
		FPLLogo512x512,

		Card_CPU,
		Card_Memory,
		Card_C_Language,
		Card_Audio,
		Card_Video,
		Card_WindowManagement,
		Card_Performance,
		Card_NoDependencies,
		Card_KeyboardMouse,
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
		Arigatou,
		MinimumSource,

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

		Feature_SingleHeaderFile,
		Feature_RuntimeLinking,
		Feature_PreProcessor,
		Feature_NoCodeDuplication,
	};
}