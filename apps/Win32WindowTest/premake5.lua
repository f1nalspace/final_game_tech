project "Win32WindowTest"
	kind "WindowedApp"

	language "C++"
	cppdialect "C++11"

	includedirs { "../../" }

	files { "main.cpp" }

	filter "system:windows"
		links { "user32", "gdi32" }
