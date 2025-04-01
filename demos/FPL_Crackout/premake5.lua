project "FPL_Crackout"
	kind "WindowedApp"
	
	language "C++"
	cppdialect "C++11"
	
	files { "fpl_crackout.cpp" }
	
	postbuildcommands { "{COPYDIR} %[%{!prj.location}/data] %[%{!cfg.targetdir}/data/]" }	