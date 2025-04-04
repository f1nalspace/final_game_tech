project "FPL_NoCRT"
	kind "ConsoleApp"

	language "C"
	cdialect "C99"

	files { "tinycrt.h", "fpl_nocrt.c" }
	
	filter "system:windows"
		links { "kernel32" }
	
		flags { 
			"NoBufferSecurityCheck",
			"NoRuntimeChecks",
			"OmitDefaultLibrary",
		}
	
		linkoptions "/NODEFAULTLIB -STACK:0x100000,0x100000"
		
		buildoptions "-GS- -Gs9999999 -GL-"