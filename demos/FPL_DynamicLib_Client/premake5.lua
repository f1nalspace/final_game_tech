project "FPL_DynamicLib_Client"
	kind "WindowedApp"
	language "C"
	cdialect "C99"
	files { "fpl_dynamiclib_client.c" }
	targetdir ( "../build/FPL_DynamicLib/%{cfg.platform}_%{cfg.buildcfg}" )
	libdirs ( "../build/FPL_DynamicLib/%{cfg.platform}_%{cfg.buildcfg}" )
	links { "FPL_DynamicLib_Host" }