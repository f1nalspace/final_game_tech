project "FPL_FFMpeg"
	kind "WindowedApp"
	
	language "C++"
	cppdialect "C++11"
	
	includedirs { "../dependencies/ffmpeg/include/"  }
	
	files {
		"defines.h",
		"ffmpeg.h",
		"mpmc_queue.h",
		"shaders.h",
		"utils.h",
		"fpl_ffmpeg.cpp",
	}
	
	filter { "system:windows", "architecture:x86_64" }
		postbuildcommands { "{COPYDIR} %[%{!wks.location}/dependencies/ffmpeg/bin/win-x64/*.dll] %[%{!cfg.targetdir}/]" }