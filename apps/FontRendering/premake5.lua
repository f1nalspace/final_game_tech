project "FontRendering"
	kind "ConsoleApp"

	language "C++"
	cppdialect "C++11"

	includedirs { "../../", "../../demos/additions", "../../demos/dependencies" }

	files {
		"main.cpp",
	}

	filter { "files:**.h" }
		buildaction "None"
	filter {}

	filter "system:linux"
		links { "m", "GL" }
