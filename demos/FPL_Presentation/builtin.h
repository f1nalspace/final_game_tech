#pragma once

#include "fonts.h"
#include "presentation.h"

namespace FontResources {
    static const Resource Debug = Resource::CreateFromMemory("Debug", ptr_bitstreamVerySansFontData, sizeOf_bitstreamVerySansFontData);
    static const Resource Arimo = Resource::CreateFromMemory("Arimo", ptr_arimoRegularFontData, sizeOf_arimoRegularFontData);
    static const Resource SulphurPoint = Resource::CreateFromMemory("Sulphur Point", ptr_sulphurPointRegularData, sizeOf_sulphurPointRegularData);
    static const Resource BitStreamVerySans = Resource::CreateFromMemory("Bitstream Vera Sans", ptr_bitstreamVerySansFontData, sizeOf_bitstreamVerySansFontData);
};

namespace SoundResources {
    static const Resource Intro1 = Resource::CreateFromFile("fpl_intro1.mp3");
    static const Resource Intro2 = Resource::CreateFromFile("fpl_intro2.mp3");
    static const Resource WhoAmi1 = Resource::CreateFromFile("fpl_whoami1.mp3");
    static const Resource What1 = Resource::CreateFromFile("fpl_what1.mp3");
    static const Resource What2 = Resource::CreateFromFile("fpl_what2.mp3");
    static const Resource Motivation = Resource::CreateFromFile("fpl_motivation.mp3");

    static const Resource All[] = {
        Intro1,
        Intro2,
        WhoAmi1,
        What1,
        What2,
        Motivation,
    };
}

namespace ImageResources {
	static Resource FPLLogo128x128 = Resource::CreateFromMemory("FPL Logo 128x128", ptr_fplLogo128x128ImageData, sizeOf_fplLogo128x128ImageData);
	static Resource FPLLogo512x512 = Resource::CreateFromMemory("FPL Logo 512x512", ptr_fplLogo512x512ImageData, sizeOf_fplLogo512x512ImageData);

	static Resource Card_CPU = Resource::CreateFromFile("card_cpu.png");
	static Resource Card_Memory = Resource::CreateFromFile("card_memory.png");
	static Resource Card_C_Language = Resource::CreateFromFile("card_c_language.png");
	static Resource Card_Audio = Resource::CreateFromFile("card_audio.png");
	static Resource Card_Video = Resource::CreateFromFile("card_video.png");
	static Resource Card_WindowManagement = Resource::CreateFromFile("card_window_management.png");
	static Resource Card_Performance = Resource::CreateFromFile("card_performance.png");
	static Resource Card_NoDependencies = Resource::CreateFromFile("card_no_deps.png");
	static Resource Card_KeyboardMouse = Resource::CreateFromFile("card_keyboard_mouse.png");
	static Resource Card_Gamepad = Resource::CreateFromFile("card_gamepad.png");

	static Resource MagicHat = Resource::CreateFromFile("magic_hat.png");
	static Resource Arigatou = Resource::CreateFromFile("arigatou.png");
	static Resource MinimumSource = Resource::CreateFromFile("minimum_source.png");

	static Resource DataVisualization = Resource::CreateFromFile("data-visualization.png");
	static Resource GameDev = Resource::CreateFromFile("game_dev.png");
	static Resource MultimediaDev = Resource::CreateFromFile("multimedia_dev.png");
	static Resource SimulationDev = Resource::CreateFromFile("sim_dev.png");

	static Resource Code = Resource::CreateFromFile("code.png");

	static Resource Vendor_FreeBSD = Resource::CreateFromFile("vendor-freebsd.png");
	static Resource Vendor_Linux = Resource::CreateFromFile("vendor-linux.png");
	static Resource Vendor_Windows = Resource::CreateFromFile("vendor-windows.png");
	static Resource Vendor_Raspberry = Resource::CreateFromFile("vendor-raspberry_pi.png");
	static Resource Vendor_OpenSource = Resource::CreateFromFile("vendor-open_source.png");
	static Resource Vendor_OpenGL = Resource::CreateFromFile("vendor-opengl.png");
	static Resource Vendor_Vulkan = Resource::CreateFromFile("vendor-vulkan.png");
	static Resource Vendor_DirectX = Resource::CreateFromFile("vendor-directx.png");
	static Resource Vendor_Alsa = Resource::CreateFromFile("vendor-alsa.png");
	static Resource Vendor_XLib = Resource::CreateFromFile("vendor-xlib.png");

	static Resource Demo_ImGUI = Resource::CreateFromFile("demo_imgui.png");
	static Resource Demo_FFMPEG = Resource::CreateFromFile("demo_ffmpeg.png");
	static Resource Demo_NBodySimulation = Resource::CreateFromFile("demo_nbodysim.png");
	static Resource Demo_Audio = Resource::CreateFromFile("demo_audio.png");
	static Resource Demo_Input = Resource::CreateFromFile("demo_input.png");
	static Resource Demo_Crackout = Resource::CreateFromFile("demo_crackout.png");
	static Resource Demo_OpenGL = Resource::CreateFromFile("demo_opengl.png");
	static Resource Demo_Raytracer = Resource::CreateFromFile("demo_raytracer.png");

	static Resource Feature_SingleHeaderFile = Resource::CreateFromFile("singleheaderfile.png");
	static Resource Feature_RuntimeLinking = Resource::CreateFromFile("runtimelinking.png");
	static Resource Feature_PreProcessor = Resource::CreateFromFile("preprocessor.png");
	static Resource Feature_NoCodeDuplication = Resource::CreateFromFile("nocodeduplication.png");

	static const Resource All[] = {
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