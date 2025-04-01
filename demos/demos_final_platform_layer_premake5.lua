-- Based on https://github.com/bluebird75/lua_get_os_name/blob/master/get_os_name.lua
local function getOSArchitecture()
	local raw_os_name, raw_arch_name = '', ''
	if jit and jit.os and jit.arch then
		raw_os_name = jit.os
		raw_arch_name = jit.arch
	else
		if package.config:sub(1,1) == '\\' then
			local env_ARCH = os.getenv('PROCESSOR_ARCHITECTURE')
			local env_OS = os.getenv('OS')
			if env_OS and env_ARCH then
				raw_os_name = env_OS
				raw_arch_name = env_ARCH
			end
		else
			local popen_status, popen_result = pcall(io.popen, "")
			if popen_status then
				popen_result:close()
				raw_os_name = io.popen('uname -s','r'):read('*l')
				raw_arch_name = io.popen('uname -m','r'):read('*l')
			end
		end
	end
	
	if (not raw_os_name) or (not raw_arch_name) then
		raw_os_name = ''
		raw_arch_name = ''
	end

	raw_os_name = (raw_os_name):lower()
	raw_arch_name = (raw_arch_name):lower()

	local os_patterns = {
		['windows'] = {'Windows', 'windows'},
		['linux'] = {'Linux', 'linux'},
		['mac'] = {'Mac', 'mac'},
		['darwin'] = {'Mac', 'mac'},
		['^mingw'] = {'Windows', 'windows'},
		['^cygwin'] = {'Windows', 'windows'},
		['bsd$'] = {'BSD', 'unix'},
		['SunOS'] = {'Solaris', 'unix'},
	}
	
	local arch_patterns = {
		['amd64'] = {'X86_64', 'x64'},
		['x86_64'] = {'X86_64', 'x64'},
		['^x86$'] = {'X86', 'x86'},
		['i[%d]86'] = {'X86', 'x86'},
		['Power Macintosh'] = {'PowerPC', 'powerpc'},
		['arm64'] = {'Arm64', 'arm'},
		['arm32'] = {'Arm32', 'arm'},
		['^arm'] = {'Arm', 'arm'},
		['^mips'] = {'Mips', 'mips'},
	}

	local os_name, os_family, arch_name, arch_family = 'unknown', 'unknown', 'unknown', 'unknown'
	
	for pattern, pair in pairs(os_patterns) do
		if raw_os_name:match(pattern) then
			os_name = pair[1]
			os_family = pair[2]
			break
		end
	end
	for pattern, pair in pairs(arch_patterns) do
		if raw_arch_name:match(pattern) then
			arch_name = pair[1]
			arch_family = pair[2]
			break
		end
	end
	return os_name, os_family, arch_name, arch_family
end

local currentOSName, currentOSFamily, currentArchitectureName, currentArchitectureFamily = getOSArchitecture()

print("Detected OS/Arch: " .. currentOSFamily .. "/" .. currentArchitectureFamily)

workspace "demos_final_platform_layer"
	configurations
	{ 
		"Debug",
		"Release",
	}
	
	flags
    {
        "MultiProcessorCompile",
		"NoPCH",
    }
	
	filter {}
	
	filter "action:vs*"
		platforms 
		{
			"WinX86",
			"WinX64",
		}

	filter "action:gmake*"
		platforms 
		{
			"WinX86",
			"WinX64",
			"LinuxX86",
			"LinuxX64",
			"LinuxARM32",
			"LinuxARM64",
			"UnixX86",
			"UnixX64",
			"UnixARM32",
			"UnixARM64",
		}
		
	-- Defaults for every project
	filter {}
		if currentOSFamily == "windows" then
			defaultplatform "WinX64"
		elseif currentOSFamily == "linux" then
			if currentArchitectureFamily == "arm" then
				if currentArchitectureName == "Arm64" then
					defaultplatform "LinuxARM64"
				else
					defaultplatform "LinuxARM32"
				end
			elseif currentArchitectureFamily == "x64" then
				defaultplatform "LinuxX64"
			elseif currentArchitectureFamily == "x86" then
				defaultplatform "LinuxX86"
			end
		elseif currentOSFamily == "unix" then
			if currentArchitectureFamily == "arm" then
				if currentArchitectureName == "Arm64" then
					defaultplatform "UnixARM64"
				else
					defaultplatform "UnixARM32"
				end
			elseif currentArchitectureFamily == "x64" then
				defaultplatform "UnixX64"
			elseif currentArchitectureFamily == "x86" then
				defaultplatform "UnixX86"
			end
		end
		includedirs ( {"../", "./additions/", "./dependencies/" } )
		targetdir ( "./build/%{prj.name}/%{cfg.platform}_%{cfg.buildcfg}" )
		objdir ( "./immediates/%{prj.name}/%{cfg.platform}_%{cfg.buildcfg}" )
			
	-- Platform mapping
	filter "platforms:WinX86"
        system "windows"
        architecture "x86"
		
	filter "platforms:WinX64"
        system "windows"
        architecture "x86_64"
		
	filter "platforms:LinuxX86"
        system "linux"
        architecture "x86"
        toolset "gcc"

	filter "platforms:LinuxX64"
        system "linux"
        architecture "x86_64"
        toolset "gcc"
		
	filter "platforms:LinuxARM32"
        system "linux"
        architecture "ARM"
        toolset "gcc"

    filter "platforms:LinuxARM64"
        system "linux"
        architecture "ARM64"
        toolset "gcc"
		
	filter "platforms:UnixX86"
        system "bsd"
        architecture "x86"
        toolset "gcc"

	filter "platforms:UnixX64"
        system "bsd"
        architecture "x86_64"
        toolset "gcc"
		
	filter "platforms:UnixARM32"
        system "bsd"
        architecture "ARM"
        toolset "gcc"

    filter "platforms:UnixARM64"
        system "bsd"
        architecture "ARM64"
        toolset "gcc"

	-- Link to math library on *nix
	filter "system:bsd or system:linux"
		links { "m" }
	
	-- Debug / Release
	filter "configurations:Debug"
		defines { "DEBUG" }
		runtime "Debug"
		optimize "Off"
		symbols "On"
		warnings "Default"
		
	filter "configurations:Release"
		defines { "NDEBUG" }
		runtime "Release"
		optimize "On"
		symbols "Off"
		warnings "Default"
	
-- Projects sorted into groups
group "ThirdParty"
	include "FPL_ImGui/premake5";
	include "FPL_MiniAudio/premake5";

group "Apps"
	include "FPL_FFMpeg/premake5";
	include "FPL_ImageViewer/premake5";
	include "FPL_NBodySimulation/premake5";
	include "FPL_Raytracer/premake5";

group "Console"
	include "FPL_Console/premake5";
	
group "Audio"
	include "FPL_AudioPlayer/premake5";
	include "FPL_SimpleAudio/premake5";
	include "FPL_WaveAudio/premake5";

group "Games"
	include "FPL_Crackout/premake5";
	include "FPL_Towadev/premake5";
	include "FPL_GameTemplate/premake5";

group "Graphics"
	include "FPL_OpenGL/premake5";
	include "FPL_Vulkan/premake5";
	include "FPL_Software/premake5";
	
group "Input"
	include "FPL_Input/premake5";

group "Window"
	include "FPL_Window/premake5";
	
group "Test"
	include "FPL_Test/premake5";

group "Compability"
	include "FPL_NoPlatformIncludes/premake5";
	include "FPL_NoRuntimeLinking/premake5";
	include "FPL_DynamicLib_Host/premake5";
	include "FPL_DynamicLib_Client/premake5";
	include "FPL_StaticLib_Host/premake5";
	include "FPL_StaticLib_Client/premake5";
	include "FPL_NoCRT/premake5";
	
