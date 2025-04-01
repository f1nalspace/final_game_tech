project "FPL_DynamicLib_Host"
	kind "SharedLib"
	language "C"
	cdialect "C99"
	files { "fpl_dynamiclib_host.c" }
	targetdir ( "../build/FPL_DynamicLib/%{cfg.platform}_%{cfg.buildcfg}" )