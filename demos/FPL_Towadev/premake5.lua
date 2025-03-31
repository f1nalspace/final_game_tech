project "FPL_Towadev"
	kind "WindowedApp"
	language "C++"
	cppdialect "C++11"
	files { "fpl_towadev.h", "fpl_towadev.cpp" }
	postbuildcommands { "{COPYDIR} %[%{!prj.location}/data] %[%{!cfg.targetdir}/data/]" }