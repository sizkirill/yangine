newoption
{
	trigger = "vsversion",
	description = "Specify the Visual Studio version",
	default = "2019",
	allowed = {
		{ "2019", "Visual Studio 2019"},
		{ "2017", "Visual Studio 2017"}
	}
}

newoption
{
	trigger = "vspath",
	description = "Path to Visual Studio",
	default = 'C:\\"Program Files (x86)"\\"Microsoft Visual Studio"\\'
}

newoption
{
	trigger = "vstype",
	description = "Community / Enterprise / Professional",
	allowed = {
		{"Community", "Visual Studio Community"},
		{"Enterprise", "Visual Studio Enterprise"},
		{"Professional", "Visual Studio Professional"}
	}
}

newoption
{
	trigger = "buildconfig",
	description = "Specify the build configurations",
	default = "Both",
	allowed = {
		{"Debug", "Debug"},
		{"Release", "Release"},
		{"Both", "Release and Debug"}
	}
}

newoption
{
	trigger = "buildplatform",
	description = "Specify the build platform",
	default = "Both",
	allowed = {
		{"x86", "x86"},
		{"x64", "x64"},
		{"Both", "x86 and x64"}
	}
}

local BuildSettings = 
{
	vsversion = _OPTIONS["vsversion"] or "2019",
	vspath = _OPTIONS["vspath"] or 'C:\\"Program Files (x86)"\\"Microsoft Visual Studio"\\',
	vstype = _OPTIONS["vstype"] or 'Enterprise',
	buildconfigs = {},
	buildplatforms = {}
}

if _OPTIONS["buildplatform"] == "Both" then
	BuildSettings.buildplatforms = {"x86", "x64"} else
	BuildSettings.buildplatforms = {_OPTIONS["buildplatform"]} end
	
if _OPTIONS["buildconfig"] == "Both" then
	BuildSettings.buildconfigs = {"Release", "Debug"} else
	BuildSettings.buildconfigs = {_OPTIONS["buildconfig"]} end

newaction {
	trigger = "build",
	description = "Build the project",
	execute = function()
		local initVS = BuildSettings.vspath .. BuildSettings.vsversion .. '\\' .. BuildSettings.vstype .. '\\VC\\Auxiliary\\Build\\vcvars32.bat'
		local buildCommandTemplate = 'msbuild "Yangine.sln" /p:Platform=%s /p:Configuration=%s /m'
		local buildCommand = ""
		
		for k,config in ipairs(BuildSettings.buildconfigs) do
			for j, platform in ipairs(BuildSettings.buildplatforms) do
				buildCommand = buildCommand .. string.format(buildCommandTemplate, platform, config) .. " && "
			end
		end
		
		buildCommand = buildCommand:sub(1, #buildCommand - 4)
		os.execute('premake5 vs' .. BuildSettings.vsversion .. ' && ' .. initVS .. ' && ' .. buildCommand)
	end
}

workspace "Yangine"
	startproject "Engine"
	configurations {"Debug", "Release"}
	platforms {"x86", "x64"}
	language "C++"
	cppdialect "C++17"
	
project "Engine"
	kind "StaticLib"
	location "Yangine"
	includedirs {"Yangine/Source", "Toolset/Include"}
	files {"Yangine/Source/**.h", "Yangine/Source/**.cpp"}
	
	filter {"configurations:Debug", "platforms:x86"}
		architecture "x86"
		targetdir "Yangine/Binaries/Debug_x86"
		
	filter {"configurations:Release", "platforms:x86"}
		architecture "x86"
		targetdir "Yangine/Binaries/Release_x86"
		
	filter {"configurations:Debug", "platforms:x64"}
		architecture "x86_64"
		targetdir "Yangine/Binaries/Debug_x64"
		
	filter {"configurations:Release", "platforms:x64"}
		architecture "x86_64"
		targetdir "Yangine/Binaries/Release_x64"
	
	filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
		optimize "Full"

	