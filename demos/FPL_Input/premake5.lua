project "FPL_Input"
	kind "WindowedApp"
	
	language "C++"
	cppdialect "C++11"
	
	files { 
		"fpl_input.cpp", 
		"../../final_platform_layer.h",
		"../../final_dynamic_opengl.h",
		"../dependencies/stb/stb_image.h",
		"../dependencies/stb/stb_truetype.h",
		"../additions/final_fonts.h",
	}
	
	filter { "files:../../*.h" }
		flags { "ExcludeFromBuild" }
	filter {}

	filter { "files:../dependencies/stb/*.h" }
		flags { "ExcludeFromBuild" }
	filter {}

	filter { "files:../additions/*.h" }
		flags { "ExcludeFromBuild" }
	filter {}

	vpaths { 
		["Dependencies"] = {
			"../../final_platform_layer.h",
			"../../final_dynamic_opengl.h",
		},
		["ThirdParty"] = {
			"../dependencies/stb/stb_image.h",
			"../dependencies/stb/stb_truetype.h",
		},
		["Additions"] = {
			"../additions/final_fonts.h",
		},
	}
		
	postbuildcommands { "{COPYDIR} %[%{!prj.location}/data] %[%{!cfg.targetdir}/data/]" }
