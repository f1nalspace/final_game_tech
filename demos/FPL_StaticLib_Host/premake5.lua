project "FPL_StaticLib_Host"
	kind "StaticLib"
	language "C"
	cdialect "C99"
	files { "final_platform_layer.c" }
	targetdir ( "../build/FPL_StaticLib/%{cfg.platform}_%{cfg.buildcfg}" )