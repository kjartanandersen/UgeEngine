workspace "Uge"
	architecture "x64"
	startproject "Uge-Editor"
	
	configurations 
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include Directories relative to root folder (Solution Directory)
IncludeDir = {}
IncludeDir["GLFW"]   	= "Uge/thirdparty/GLFW/include"
IncludeDir["GLAD"]   	= "Uge/thirdparty/GLAD/include"
IncludeDir["IMGUI"]  	= "Uge/thirdparty/imgui"
IncludeDir["GLM"]    	= "Uge/thirdparty/glm"
IncludeDir["STBI"]   	= "Uge/thirdparty/stb_image"
IncludeDir["ENTT"]   	= "Uge/thirdparty/entt/include"
IncludeDir["YAMLCPP"]	= "Uge/thirdparty/yaml-cpp/include"


group "Dependencies"
	include "Uge/thirdparty/GLFW"
	include "Uge/thirdparty/GLAD"
	include "Uge/thirdparty/imgui"
	include "Uge/thirdparty/glm"
	include "Uge/thirdparty/yaml-cpp"

group ""

include "Uge"
include "UgeEditor"
include "Sandbox"


