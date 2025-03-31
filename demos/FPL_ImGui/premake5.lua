project "FPL_ImGui"
	kind "WindowedApp"
	language "C++"
	cppdialect "C++11"
	includedirs { "../dependencies/imgui/include/"  }
	files {
		"../dependencies/imgui/include/imgui/imgui.cpp",
		"../dependencies/imgui/include/imgui/imgui_demo.cpp",
		"../dependencies/imgui/include/imgui/imgui_draw.cpp",
		"fpl_imgui.cpp",
	}
	filter { "system:windows" }
		links { "opengl32" }
	filter { "system:bsd", "system:linux" }
		links { "gl" }