project "FPL_Presentation"
	kind "WindowedApp"
	
	language "C++"
	cppdialect "C++11"
	
	files { "../../final_platform_layer.h", "fonts.h", "images.h", "types.h", "presentation.h", "slides.h", "fpl_presentation.cpp" }
	
	postbuildcommands { "{COPYDIR} %[%{!prj.location}/data] %[%{!cfg.targetdir}/data/]" }