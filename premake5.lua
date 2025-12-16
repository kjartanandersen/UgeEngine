workspace "Uge"
	architecture "x64"
	startproject "Sandbox"
	
	configurations 
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include Directories relative to root folder (Solution Directory)
IncludeDir = {}
IncludeDir["GLFW"] = "Uge/thirdparty/GLFW/include"
IncludeDir["GLAD"] = "Uge/thirdparty/GLAD/include"
IncludeDir["IMGUI"] = "Uge/thirdparty/imgui"


group "Dependencies"
	include "Uge/thirdparty/GLFW"
	include "Uge/thirdparty/GLAD"
	include "Uge/thirdparty/imgui"

group ""

include "Uge"
include "Sandbox"

