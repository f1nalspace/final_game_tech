project "FPL_Input"
	kind "WindowedApp"
	
	language "C++"
	cppdialect "C++11"
	
	files { "fpl_input.cpp" }
	
	postbuildcommands { "{COPYDIR} %[%{!prj.location}/data] %[%{!cfg.targetdir}/data/]" }		