project "FPL_Emulator"
	kind "WindowedApp"

	language "C"
	cdialect "C99"

	files { "fpl_emulator.c" }

	postbuildcommands { "{COPYDIR} %[%{!prj.location}/roms] %[%{!cfg.targetdir}/roms/]" }

	filter "system:bsd or system:linux"
		links { "m" }