include "./thirdparty/premake/premake_customization/solution_items.lua"
include "Dependencies.lua"

workspace "Uge"
	architecture "x64"
	startproject "Uge-Editor"
	
	configurations 
	{
		"Debug",
		"Release",
		"Dist"
	}

		solution_items
	{
		".editorconfig"
	}

	multiprocessorcompile ("on")

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"


group "Dependencies"
	include "thirdparty/premake"
	include "Uge/thirdparty/GLFW"
	include "Uge/thirdparty/GLAD"
	include "Uge/thirdparty/imgui"
	include "Uge/thirdparty/glm"
	include "Uge/thirdparty/assimp"
	include "Uge/thirdparty/yaml-cpp"
	include "Uge/thirdparty/JoltPhysics"

group ""

group "Core"
	include "Uge"
	include "Uge-ScriptCore"
group ""

group "Tools"
	include "UgeEditor"
group ""

group "Misc"
	include "Sandbox"
group ""

