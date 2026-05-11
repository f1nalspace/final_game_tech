project "FontRendering"
	kind "ConsoleApp"

	language "C++"
	cppdialect "C++11"

	includedirs { "../../" }

	files {
		"main.cpp",
		"stb_truetype.h",
	}

	filter { "files:**.h" }
		buildaction "None"
	filter {}

	filter "system:linux"
		links { "m", "GL" }
