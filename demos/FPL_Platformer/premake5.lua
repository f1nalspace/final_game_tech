project "FPL_Platformer"
	kind "WindowedApp"
	
	language "C++"
	cppdialect "C++11"
	
	files { "fpl_platformer.h", "fpl_platformer.cpp" }
	
	postbuildcommands { "{COPYDIR} %[%{!prj.location}/data] %[%{!cfg.targetdir}/data/]" }	