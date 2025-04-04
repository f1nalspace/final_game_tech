project "FPL_StaticLib_Client"
	kind "WindowedApp"
	
	language "C"
	cdialect "C99"
	
	files { "fpl_staticlib_client.c" }
	
	targetdir ( "../build/FPL_StaticLib/%{cfg.platform}_%{cfg.buildcfg}" )
	
	links { "FPL_StaticLib_Host" }