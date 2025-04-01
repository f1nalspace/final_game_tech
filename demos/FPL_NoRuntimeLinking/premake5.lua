project "FPL_NoRuntimeLinking"
	kind "WindowedApp"
	
	language "C++"
	cppdialect "C++11"
	
	includedirs { "../dependencies/vulkan/include/" }
	
	files { "fpl_noruntimelinking.cpp" }
	
	filter { "system:windows" }
		libdirs { "../dependencies/vulkan/lib/%{cfg.platform}/" }
		links { "opengl32", "dsound", "xinput", "vulkan-1" }
		
	filter { "system:bsd", "system:linux" }
		local openGLLibPath = os.findlib("GL")
		local vulkanLibPath = os.findlib("vulkan")
		local alsaLibPath = os.findlib("asound")
		local x11LibPath = os.findlib("X11")
		libdirs { openGLLibPath, vulkanLibPath, alsaLibPath, x11LibPath }
		links { "m", "pthread", "GL", "vulkan", "asound", "X11" }