project "FPL_ImGuiMultiWindow"
	kind "WindowedApp"
	
	language "C++"
	cppdialect "C++11"
	
	includedirs { "../dependencies/imgui_docking/", "../dependencies/imgui_docking/backends/" }
	
	files {
		"../dependencies/imgui_docking/imgui.cpp",
		"../dependencies/imgui_docking/imgui_demo.cpp",
		"../dependencies/imgui_docking/imgui_draw.cpp",
		"../dependencies/imgui_docking/imgui_tables.cpp",
		"../dependencies/imgui_docking/imgui_widgets.cpp",
		"../dependencies/imgui_docking/backends/imgui_impl_opengl2.cpp",
		"fpl_imgui_multiwindow.cpp",
	}
	
	vpaths { 
		["Dependencies"] = {
			"../../final_platform_layer.h",
		},
		["ThirdParty"] = {
			"../dependencies/imgui_docking/imgui.cpp",
			"../dependencies/imgui_docking/imgui_demo.cpp",
			"../dependencies/imgui_docking/imgui_draw.cpp",
			"../dependencies/imgui_docking/imgui_tables.cpp",
			"../dependencies/imgui_docking/imgui_widgets.cpp",
			"../dependencies/imgui_docking/backends/imgui_impl_opengl2.cpp",
		},
	}

	filter "system:windows"
		links { "opengl32" }
	
	filter "system:bsd or system:linux"
		local openGLLibPath = os.findlib("GL")
		libdirs { openGLLibPath }
		links { "GL" }
