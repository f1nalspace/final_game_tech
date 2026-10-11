project "FPL_NoRuntimeLinking"
	kind "WindowedApp"
	
	language "C++"
	cppdialect "C++11"
	
	includedirs { "../dependencies/vulkan/include/" }
	
	files { "fpl_noruntimelinking.cpp" }
	
	filter "system:windows"
		libdirs { "../dependencies/vulkan/lib/%{cfg.platform}/" }
		links { "opengl32", "dsound", "xinput", "vulkan-1" }
		
	filter "system:bsd or system:linux"
		local openGLLibPath = os.findlib("GL")
		local vulkanLibPath = os.findlib("vulkan")
		local alsaLibPath = os.findlib("asound")
		local x11LibPath = os.findlib("X11")
		local pulseLibPath = os.findlib("pulse")
		libdirs { openGLLibPath, vulkanLibPath, alsaLibPath, x11LibPath }
		links { "m", "pthread", "GL", "vulkan", "asound", "X11", "Xrandr", "Xinerama", "Xi" }
		-- FPL enables the PulseAudio backend by itself when the headers are installed, then the library is needed as well
		if pulseLibPath then
			links { "pulse" }
		end