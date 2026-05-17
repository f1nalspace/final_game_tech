project "OpenGLExtParser"
	kind "ConsoleApp"

	language "C++"
	cppdialect "C++11"

	includedirs { "../../" }

	files { "main.cpp" }

	filter "system:linux"
		links { "m" }
