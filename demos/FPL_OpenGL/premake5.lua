project "FPL_OpenGL"
	kind "WindowedApp"
	
	language "C"
	cdialect "C99"
	
	files { "../../final_platform_layer.h", "fpl_opengl.c" }
	
	filter "system:windows"
		links { "opengl32" }
		
	filter "system:bsd or system:linux"
		local openGLLibPath = os.findlib("GL")
		libdirs { openGLLibPath }
		links { "m", "GL" }