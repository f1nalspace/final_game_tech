project "FPL_Emulator"
	kind "WindowedApp"

	language "C"
	cdialect "C99"

	files { "fpl_emulator.c" }

	-- The boot ROM dump is not ours to hand on, so this demo builds without it and does not ship bootrom.h
	defines { "NO_BOOTROM" }

	postbuildcommands { "{COPYDIR} %[%{!prj.location}/roms] %[%{!cfg.targetdir}/roms/]" }

	filter "system:bsd or system:linux"
		links { "m" }