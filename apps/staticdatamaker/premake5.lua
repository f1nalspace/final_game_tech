project "staticdatamaker"
	kind "ConsoleApp"

	language "C"
	cdialect "C99"

	includedirs { "../../" }

	files { "main.c" }

	filter "system:linux"
		links { "m" }
