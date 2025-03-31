project "FPL_ImageViewer"
	kind "WindowedApp"
	language "C++"
	cppdialect "C++11"
	defines { "_CRT_SECURE_NO_DEPRECATE" }
	files {
		"version.h",
		"shadersources.h",
		"logging.h",
		"imageresources.h",
		"fpl_imageviewer.cpp",
	}