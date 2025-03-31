-- Based on https://github.com/bluebird75/lua_get_os_name/blob/master/get_os_name.lua
local function getOSArchitecture()
	local raw_os_name, raw_arch_name = '', ''
	if jit and jit.os and jit.arch then
		raw_os_name = jit.os
		raw_arch_name = jit.arch
	else
		if package.config:sub(1,1) == '\\' then
			local env_pfilesx32 = os.getenv('%ProgramFiles(x86)%')
			print("PFiles32: " .. env_pfilesx32)
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
		['windows'] = 'Windows',
		['linux'] = 'Linux',
		['mac'] = 'Mac',
		['darwin'] = 'Mac',
		['^mingw'] = 'Windows',
		['^cygwin'] = 'Windows',
		['bsd$'] = 'BSD',
		['SunOS'] = 'Solaris',
	}
	
	local arch_patterns = {
		['amd64'] = 'x86_64',
		['x86_64'] = 'x86_64',
		['^x86$'] = 'x86',
		['i[%d]86'] = 'x86',
		['Power Macintosh'] = 'powerpc',
		['^arm'] = 'arm',
		['^mips'] = 'mips',
	}

	local os_name, arch_name = 'unknown', 'unknown'
	
	print("Raw Arch Name: "  .. raw_arch_name)

	for pattern, name in pairs(os_patterns) do
		if raw_os_name:match(pattern) then
			os_name = name
			break
		end
	end
	for pattern, name in pairs(arch_patterns) do
		if raw_arch_name:match(pattern) then
			arch_name = name
			break
		end
	end
	return os_name, arch_name
end

local currentOS, currentArchitecture = getOSArchitecture()

print("Detected OS/Arch: " .. currentOS .. "/" .. currentArchitecture)

workspace "demos_final_platform_layer"
	configurations { "Debug", "Release" }
		
	if currentArchitecture == "arm" then
		platforms { "ARM32", "ARM64" }
		defaultplatform "ARM64"
	elseif currentArchitecture == "x86_64" then
		platforms { "X86", "X86_64" }
		defaultplatform "X86_64"
	elseif currentArchitecture == "x86" then
		platforms { "X86" }
		defaultplatform "X86"
	else
		error("Architecture not supported: " .. currentArchitecture)
	end
		
	-- Defaults for every project
	filter {}
		includedirs ({"../", "./additions/", "./dependencies/"})
		targetdir ("./build/%{prj.name}/%{cfg.system}_%{cfg.platform}_%{cfg.buildcfg}")
		objdir ("./immediates/%{prj.name}/%{cfg.system}_%{cfg.platform}_%{cfg.buildcfg}")
		
	-- Link to math library on *nix
	filter { "system:bsd", "system:linux" }
		links { "m" }

	-- Platform to architecture mapping
	filter { "platforms:X86" }
		architecture "x86"
	filter { "platforms:X86_64" }
		architecture "x86_64"
	filter { "platforms:X86_64" }
		architecture "x86_64"
	filter { "platforms:ARM32" }
		architecture "ARM"
	filter { "platforms:ARM64" }
		architecture "ARM64"

	-- Debug / Release
	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"
	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
	
-- Projects sorted into groups
group "ThirdParty"
	include "FPL_ImGui/premake5";

group "Apps"
	include "FPL_FFMpeg/premake5";
	include "FPL_ImageViewer/premake5";

group "Console"
	include "FPL_Console/premake5";
	
group "Audio"
	include "FPL_AudioPlayer/premake5";
	include "FPL_SimpleAudio/premake5";

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
	
