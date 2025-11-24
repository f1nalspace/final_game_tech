project "FPL_GameTemplate"
	kind "WindowedApp"
	
	language "C"
	cdialect "C99"
	
	files { 
		"fpl_gametemplate.h", 
		"fpl_gametemplate.c", 
		"../../final_dynamic_opengl.h",
		"../../final_memory.h",
		"../../final_platform_layer.h",
		"../dependencies/stb/stb_image.h",
		"../dependencies/stb/stb_truetype.h",
		"../additions/final_audio.h",
		"../additions/final_audiosystem.h",
		"../additions/final_audioconversion.h",
		"../additions/final_assets.h",
		"../additions/final_fonts.h",
		"../additions/final_fontloader.h",
		"../additions/final_game.h",
		"../additions/final_gameplatform.h",
		"../additions/final_geometry.h",
		"../additions/final_math.h",
		"../additions/final_random.h",
		"../additions/final_render.h",
		"../additions/final_opengl_render.h",
		"../additions/final_utils.h",
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

	group "Dependencies"
		vpaths { 
			["Dependencies"] = {
				"../../final_dynamic_opengl.h",
				"../../final_memory.h",
				"../../final_platform_layer.h",
			},
			["ThirdParty"] = {
				"../dependencies/stb/stb_image.h",
				"../dependencies/stb/stb_truetype.h",
			},
			["Additions"] = {
				"../additions/final_audio.h",
				"../additions/final_audiosystem.h",
				"../additions/final_audioconversion.h",
				"../additions/final_assets.h",
				"../additions/final_fonts.h",
				"../additions/final_fontloader.h",
				"../additions/final_game.h",
				"../additions/final_gameplatform.h",
				"../additions/final_geometry.h",
				"../additions/final_math.h",
				"../additions/final_random.h",
				"../additions/final_render.h",
				"../additions/final_opengl_render.h",
				"../additions/final_utils.h",
			},
		}
	group "" -- end of "Dependencies"
		
	postbuildcommands { "{COPYDIR} %[%{!prj.location}/data] %[%{!cfg.targetdir}/data/]" }