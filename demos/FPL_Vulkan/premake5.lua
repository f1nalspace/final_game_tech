project "FPL_Vulkan"
	kind "WindowedApp"
	language "C"
	cdialect "C99"
	includedirs { "../dependencies/vulkan/include/" }
	files { "containers.h", "fpl_vulkan.c" }