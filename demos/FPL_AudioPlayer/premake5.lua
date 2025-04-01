project "FPL_AudioPlayer"
	kind "WindowedApp"
	
	language "C"
	cdialect "C99"
	
	files { "fpl_audioplayer.c" }
	
	filter "system:windows"
		links { "opengl32" }
	
	filter "system:bsd or system:linux"
		local openGLLibPath = os.findlib("GL")
		libdirs { openGLLibPath }
		links { "m", "GL" }